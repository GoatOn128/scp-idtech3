#include "../precompiled.h"

#include "FxScheduler.h"
#include "cg_fx.h"

static int fxLastFrameTime = -1;

void CG_FxInit( void ) {
    FX_Init();
    fileHandle_t file = 0;
    int length = theFxHelper.OpenFile( "effects/wolf/manifest.txt", &file, FS_READ );
    if ( length > 0 && length < 16384 ) {
        idList<char> buffer;
        buffer.SetNum( length + 1 );
        buffer[length] = '\0';
        theFxHelper.ReadFile( buffer.Ptr(), length, file );
        idLexer lexer;
        lexer.LoadMemory( buffer.Ptr(), length, "effects/wolf/manifest.txt" );
        idToken token;
        while ( lexer.ReadToken( &token ) ) {
            theFxScheduler.RegisterEffect( token.c_str() );
        }
    } else {
        CG_Printf( "FX: missing or invalid effects/wolf/manifest.txt\n" );
    }
    if ( file ) { theFxHelper.CloseFile( file ); }
	fxLastFrameTime = -1;
}

void CG_FxShutdown( void ) {
	FX_Free();
	fxLastFrameTime = -1;
}

void CG_FxReset( void ) {
	FX_Stop();
	fxLastFrameTime = -1;
}

void CG_FxBeginFrame( void ) {
	if ( fxLastFrameTime > cg.time ) {
		CG_FxReset();
	}
	int delta = fxLastFrameTime < 0 ? 1 : cg.time - fxLastFrameTime;
	theFxHelper.AdjustTime( delta );
	fxLastFrameTime = cg.time;
}

void CG_FxDraw( void ) {
	cg.viewFade = 0.0f;
	CG_FxBeginFlames();
	theFxScheduler.AddScheduledEffects();
	FX_Add();
	CG_UpdateFlamethrowerSounds();
}

void CG_FxPlay( const char *name, const idVec3 &origin, const idVec3 &direction ) {
	int handle = theFxScheduler.RegisterEffect( name );
	if ( handle ) {
		idVec3 position, forward;
		position = origin;
		forward = direction;
		if ( forward.Normalize() == 0 ) {
			forward.Set( 0, 0, 1 );
		}
		theFxScheduler.PlayEffect( handle, position, forward );
	}
}

void CG_FxTest_f( void ) {
	if ( !cg.snap || ( trap_Argc() != 2 && trap_Argc() != 3 ) ) {
		CG_Printf( "usage: fxplay <effect name under effects/> [surface]\n" );
		return;
	}
	idVec3 origin, direction;
	direction = cg.refdef.viewaxis[0];
	origin = cg.refdef.vieworg + direction * 96;
	if ( trap_Argc() == 3 ) {
		if ( idStr::Icmp( CG_Argv( 2 ), "surface" ) ) {
			CG_Printf( "usage: fxplay <effect name under effects/> [surface]\n" );
			return;
		}
		trace_t trace;
		origin = cg.refdef.vieworg + direction * 8192;
		CG_Trace( &trace, cg.refdef.vieworg, NULL, NULL, origin, cg.snap->ps.clientNum, MASK_SOLID );
		if ( trace.fraction == 1 || trace.startsolid || trace.allsolid || ( trace.surfaceFlags & SURF_SKY ) ) {
			CG_Printf( "FX: aim at a solid surface\n" );
			return;
		}
		origin = trace.endpos;
		direction = trace.plane.normal;
	}
	CG_FxPlay( CG_Argv( 1 ), origin, direction );
}

static bool FX_Binds( const idStr &bindings, const char *name ) {
	idStr padded( " " );
	padded += bindings;
	padded += " ";
	idStr needle( " " );
	needle += name;
	needle += " ";
	return padded.Find( needle.c_str() ) != -1;
}

static void FX_OffsetRange( CFxRange &range, float offset ) {
	range.SetRange( range.GetMin() + offset, range.GetMax() + offset );
}

static void FX_ScaleRange( CFxRange &range, float scale ) {
	float first = range.GetMin() * scale;
	float last = range.GetMax() * scale;
	range.SetRange( first < last ? first : last, first < last ? last : first );
}

void CG_FxPlayParameters( const char *name, const idVec3 &origin, const idVec3 &direction, const FxParameters &p ) {
	if ( theFxHelper.mTimeFrozen ) { return; }
	int registered = theFxScheduler.RegisterEffect( name );
	if ( !registered ) { return; }
	int handle = 0;
	SEffectTemplate *effect = theFxScheduler.GetEffectCopy( registered, &handle );
	if ( !effect || !handle ) { return; }
	for ( int i = 0; i < effect->mPrimitiveCount; i++ ) {
		CPrimitiveTemplate &primitive = *effect->mPrimitives[i];
		const idStr &bindings = primitive.mBindings;
		if ( FX_Binds( bindings, "life" ) && p.life > 0 ) { primitive.mLife.SetRange( p.life, p.life ); }
		if ( FX_Binds( bindings, "size" ) && p.size >= 0 ) {
			FX_ScaleRange( primitive.mSizeStart, p.size );
			FX_ScaleRange( primitive.mSizeEnd, p.size );
		}
		if ( FX_Binds( bindings, "sizeEnd" ) && p.sizeEnd >= 0 ) { primitive.mSizeEnd.SetRange( p.sizeEnd, p.sizeEnd ); }
		if ( FX_Binds( bindings, "scale" ) ) {
			FX_ScaleRange( primitive.mSizeStart, p.scale );
			FX_ScaleRange( primitive.mSizeEnd, p.scale );
		}
		if ( FX_Binds( bindings, "alpha" ) && p.alpha >= 0 ) {
			FX_ScaleRange( primitive.mAlphaStart, p.alpha );
			FX_ScaleRange( primitive.mAlphaEnd, p.alpha );
		}
		if ( FX_Binds( bindings, "shader" ) && p.shader ) { primitive.mMediaHandles.SetHandle( p.shader ); }
		if ( FX_Binds( bindings, "aspect" ) && p.aspect > 0 ) { primitive.mAspect = p.aspect; }
		if ( FX_Binds( bindings, "pathLength" ) && primitive.mCountSpacing > 0 ) {
			float count = idMath::Ceil( p.pathLength / primitive.mCountSpacing );
			count = count < 1 ? 1 : ( count > MAX_EFFECTS ? MAX_EFFECTS : count );
			primitive.mSpawnCount.SetRange( count, count );
		}
		if ( FX_Binds( bindings, "spread" ) ) {
			primitive.mOrigin1X.SetRange( -p.spreadAxes[0], p.spreadAxes[0] );
			primitive.mOrigin1Y.SetRange( -p.spreadAxes[1], p.spreadAxes[1] );
			primitive.mOrigin1Z.SetRange( -p.spreadAxes[2], p.spreadAxes[2] );
		}
		if ( FX_Binds( bindings, "speedScale" ) ) { FX_ScaleRange( primitive.mVelZ, p.speed ); }
		if ( p.size >= 0 ) {
			FX_ScaleRange( primitive.mVelX, 1 + primitive.mSizeVelocity[0] * (p.size - 1) );
			FX_ScaleRange( primitive.mVelY, 1 + primitive.mSizeVelocity[1] * (p.size - 1) );
			FX_ScaleRange( primitive.mVelZ, 1 + primitive.mSizeVelocity[2] * (p.size - 1) );
		}
		if ( FX_Binds( bindings, "velocity" ) ) {
			FX_OffsetRange( primitive.mVelX, p.velocity[0] * primitive.mVelocityScale[0] );
			FX_OffsetRange( primitive.mVelY, p.velocity[1] * primitive.mVelocityScale[1] );
			FX_OffsetRange( primitive.mVelZ, p.velocity[2] * primitive.mVelocityScale[2] );
		}
		if ( FX_Binds( bindings, "verticalSpeed" ) ) { FX_OffsetRange( primitive.mVelZ, p.speed ); }
		if ( FX_Binds( bindings, "speed" ) ) { FX_OffsetRange( primitive.mVelX, p.speed ); }
		if ( FX_Binds( bindings, "acceleration" ) ) { FX_OffsetRange( primitive.mAccelX, p.acceleration ); }
		if ( FX_Binds( bindings, "fadeTime" ) && p.fadeTime >= 0 && primitive.mLife.GetMin() > 0 ) {
			float percent = 100 * p.fadeTime / primitive.mLife.GetMin();
			primitive.mAlphaParm.SetRange( percent, percent );
		}
		if ( FX_Binds( bindings, "volume" ) ) {
			primitive.mHeightMin = p.heightMin;
			primitive.mHeightMax = p.heightMax;
			primitive.mOrigin1X.SetRange( -p.spread, p.spread );
			primitive.mOrigin1Y.SetRange( -p.spread, p.spread );
			primitive.mOrigin1Z.SetRange( p.heightMin - origin[2], p.heightMax - origin[2] );
		}
		if ( FX_Binds( bindings, "verticalSign" ) ) { FX_ScaleRange( primitive.mVelZ, p.verticalSign ); }
		primitive.mGroup = p.group;
	}
	idVec3 position, forward;
	position = origin;
	forward = direction;
	if ( forward.Normalize() == 0 ) { forward.Set( 0, 0, 1 ); }
	theFxScheduler.PlayEffect( handle, position, forward );
}

static const FxControllerProperty &CG_FxProperty( const char *effect, const char *property ) {
	const CPrimitiveTemplate *definition = theFxScheduler.Definition( effect );
	if ( definition ) {
		const FxControllerProperty *found = nullptr;
		if ( definition->mProperties.Get( property, &found ) ) { return *found; }
	}
	CG_Error( "FX: missing required property %s in %s", property, effect );
	static FxControllerProperty empty;
	return empty;
}

float CG_FxValue( const char *effect, const char *property ) {
	const FxControllerProperty &value = CG_FxProperty(effect, property);
	if ( !value.numeric ) { CG_Error( "FX: invalid numeric property %s in %s", property, effect ); }
	return value.range.GetVal();
}

qhandle_t CG_FxMaterial( const char *effect, const char *property ) {
	const FxControllerProperty &value = CG_FxProperty(effect, property);
	if ( !value.shader ) { value.shader = trap_R_RegisterShader(value.text.c_str()); }
	return value.shader;
}

int CG_FxInt( const char *effect, const char *property ) {
	return static_cast<int>(CG_FxValue(effect, property));
}
qhandle_t CG_FxModel( const char *effect, const char *property ) {
	const FxControllerProperty &value = CG_FxProperty(effect, property);
	if ( !value.model ) { value.model = trap_R_RegisterModel(value.text.c_str()); }
	return value.model;
}
