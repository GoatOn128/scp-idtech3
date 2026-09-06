// this include must remain at the top of every FXxxxx.CPP file
#include "FxWolf.h"

#if !defined(FX_SCHEDULER_H_INC)
	#include "FxScheduler.h"
#endif

vmCvar_t fx_debug;
vmCvar_t fx_freeze;

extern void CG_StartShakeCamera( float intensity, int time, vec3_t origin, float radius );

static void FX_RenderPrimitive(FxRefEntity *ent);

// Stuff for the FxHelper
//------------------------------------------------------
void SFxHelper::Init()
{
	mTime = 0;
	mOldTime = 0;
	mTimeFrozen = false;
	mFrameTime = 0;
	mFloatFrameTime = 0;
	trap_Cvar_Register(&fx_debug, "fx_debug", "0", 0);
	trap_Cvar_Register(&fx_freeze, "fx_freeze", "0", CVAR_CHEAT);
}

//------------------------------------------------------
void SFxHelper::Print( const char *msg, ... )
{
#ifndef FINAL_BUILD

	va_list		argptr;
	char		text[1024];

	va_start( argptr, msg );
	vsnprintf( text, sizeof(text), msg, argptr );
	va_end( argptr );
 
	CG_Printf( "%s", text );

#endif
}

//------------------------------------------------------
void SFxHelper::AdjustTime( int frameTime )
{
    trap_Cvar_Update(&fx_debug);
    trap_Cvar_Update(&fx_freeze);
    mOldTime = mTime;
    mTimeFrozen = fx_freeze.integer || cg_paused.integer;
    mFrameTime = mTimeFrozen ? 0 : (frameTime > 0 ? frameTime : 0);
    mFloatFrameTime = mFrameTime * 0.001f;
    mTime += mFrameTime;
}

//------------------------------------------------------
int SFxHelper::OpenFile( const char *file, fileHandle_t *fh, int mode )
{
//	char path[256];

//	sprintf( path, "%s/%s", FX_FILE_PATH, file );
	return trap_FS_FOpenFile( file, fh, FS_READ );
}

//------------------------------------------------------
int SFxHelper::ReadFile( void *data, int len, fileHandle_t fh )
{
	trap_FS_Read( data, len, fh );
	return len;
}

//------------------------------------------------------
void SFxHelper::CloseFile( fileHandle_t fh )
{
	trap_FS_FCloseFile( fh );
}

//------------------------------------------------------
void SFxHelper::PlaySound( vec3_t org, int entityNum, int entchannel, int sfxHandle )
{
	trap_S_StartSound( org, entityNum, entchannel, sfxHandle );
}

//------------------------------------------------------
void SFxHelper::Trace( trace_t *tr, vec3_t start, vec3_t min, vec3_t max, 
						vec3_t end, int skipEntNum, int flags )
{
	CG_Trace( tr, start, min, max, end, skipEntNum, flags );
}

//------------------------------------------------------
void SFxHelper::AddFxToScene( FxRefEntity *ent )
{
	FX_RenderPrimitive( ent );
}

//------------------------------------------------------
int SFxHelper::RegisterShader( const char *shader )
{
	return trap_R_RegisterShader( shader );
}

//------------------------------------------------------
int SFxHelper::RegisterSound( const char *sound )
{
	return trap_S_RegisterSound( sound );
}

//------------------------------------------------------
int SFxHelper::RegisterModel( const char *model )
{
	return trap_R_RegisterModel( model );
}

//------------------------------------------------------
void SFxHelper::AddLightToScene( vec3_t org, float radius, float red, float green, float blue )
{
	trap_R_AddLightToScene( org, radius, red, green, blue, 0 );
}

//------------------------------------------------------
void SFxHelper::AddPolyToScene( int shader, int count, polyVert_t *verts )
{
	trap_R_AddPolyToScene( shader, count, verts );
}

//------------------------------------------------------
void SFxHelper::CameraShake( vec3_t origin, float intensity, int radius, int time )
{
	CG_StartShakeCamera( intensity, time, origin, radius );
}

static void FX_Quad( const FxRefEntity *ent, const vec3_t a, const vec3_t b,
					 const vec3_t c, const vec3_t d ) {
	polyVert_t verts[4];
	const float *points[4] = { a, b, c, d };
	const float st[4][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };
	for ( int i = 0; i < 4; i++ ) {
		VectorCopy( points[i], verts[i].xyz );
		verts[i].st[0] = st[i][0];
		verts[i].st[1] = st[i][1];
		memcpy( verts[i].modulate, ent->shaderRGBA, 4 );
	}
	trap_R_AddPolyToScene( ent->customShader, 4, verts );
}

static void FX_Line( const FxRefEntity *ent, const vec3_t start, const vec3_t end,
					 float startWidth, float endWidth ) {
	vec3_t direction, view, side, a, b, c, d;
	VectorSubtract( end, start, direction );
	if ( VectorNormalize( direction ) == 0 ) {
		return;
	}
	VectorSubtract( cg.refdef.vieworg, start, view );
	CrossProduct( direction, view, side );
	if ( VectorNormalize( side ) == 0 ) {
		PerpendicularVector( side, direction );
	}
	VectorMA( start, startWidth, side, a );
	VectorMA( start, -startWidth, side, b );
	VectorMA( end, -endWidth, side, c );
	VectorMA( end, endWidth, side, d );
	FX_Quad( ent, a, b, c, d );
}

static void FX_RenderPrimitive( FxRefEntity *ent ) {
	if ( ent->reType == RT_LINE ) {
		FX_Line( ent, ent->origin, ent->oldorigin, ent->radius, ent->radius );
	} else if ( ent->reType == RT_ELECTRICITY ) {
		vec3_t previous, point, direction, side, up, end;
		VectorCopy( ent->oldorigin, end );
		if ( ent->renderfx & RF_GROW ) {
			float life = ent->angles[1];
			float fraction = life > 0 ? 1.0f - ( ent->endTime - cg.time ) / life : 1.0f;
			fraction = fraction < 0 ? 0 : ( fraction > 1 ? 1 : fraction );
			VectorSubtract( end, ent->origin, direction );
			VectorMA( ent->origin, fraction, direction, end );
		}
		VectorSubtract( end, ent->origin, direction );
		float length = VectorNormalize( direction );
		if ( length == 0 ) {
			return;
		}
		PerpendicularVector( side, direction );
		CrossProduct( direction, side, up );
		VectorCopy( ent->origin, previous );
		int seed = ent->frame + theFxHelper.mTime / 50;
		for ( int i = 1; i <= 16; i++ ) {
			float fraction = i / 16.0f;
			VectorMA( ent->origin, length * fraction, direction, point );
			if ( i < 16 ) {
				float chaos = ent->angles[0] * length * 0.05f;
				VectorMA( point, Q_crandom( &seed ) * chaos, side, point );
				VectorMA( point, Q_crandom( &seed ) * chaos, up, point );
			}
			float width = ent->radius * ( ( ent->renderfx & RF_TAPERED ) ? 1.0f - fraction : 1.0f );
			FX_Line( ent, previous, point, width, width );
			if ( ( ent->renderfx & RF_FORKED ) && i == 8 ) {
				vec3_t branch;
				VectorMA( point, length * 0.2f, side, branch );
				VectorMA( branch, length * 0.2f, direction, branch );
				FX_Line( ent, point, branch, width * 0.5f, 0 );
			}
			VectorCopy( point, previous );
		}
	} else if ( ent->reType == RT_ORIENTED_QUAD ) {
		vec3_t right, up, a, b, c, d;
		PerpendicularVector( up, ent->axis[0] );
		RotatePointAroundVector( right, ent->axis[0], up, ent->rotation );
		CrossProduct( ent->axis[0], right, up );
		VectorMA( ent->origin, ent->radius, right, a );
		VectorMA( a, ent->radius, up, a );
		VectorMA( ent->origin, -ent->radius, right, b );
		VectorMA( b, ent->radius, up, b );
		VectorMA( ent->origin, -ent->radius, right, c );
		VectorMA( c, -ent->radius, up, c );
		VectorMA( ent->origin, ent->radius, right, d );
		VectorMA( d, -ent->radius, up, d );
		FX_Quad( ent, a, b, c, d );
	} else if ( ent->reType == RT_CYLINDER ) {
		vec3_t side, up, radial[2], a, b, c, d;
		PerpendicularVector( side, ent->axis[0] );
		CrossProduct( ent->axis[0], side, up );
		for ( int i = 0; i < 16; i++ ) {
			for ( int j = 0; j < 2; j++ ) {
				float angle = ( i + j ) * 2.0f * M_PI / 16;
				VectorScale( side, cosf( angle ), radial[j] );
				VectorMA( radial[j], sinf( angle ), up, radial[j] );
			}
			VectorMA( ent->origin, ent->radius, radial[0], a );
			VectorMA( ent->origin, ent->radius, radial[1], b );
			VectorMA( ent->oldorigin, ent->backlerp, radial[1], c );
			VectorMA( ent->oldorigin, ent->backlerp, radial[0], d );
			FX_Quad( ent, a, b, c, d );
		}
	} else if ( ent->reType == RT_SPRITE && ent->aspectRatio > 0 && ent->aspectRatio != 1 ) {
        vec3_t right, up, a, b, c, d;
        RotatePointAroundVector( right, cg.refdef.viewaxis[0], cg.refdef.viewaxis[1], ent->rotation );
        CrossProduct( cg.refdef.viewaxis[0], right, up );
        float height = ent->radius * ent->aspectRatio;
        VectorMA( ent->origin, ent->radius, right, a ); VectorMA( a, height, up, a );
        VectorMA( ent->origin, -ent->radius, right, b ); VectorMA( b, height, up, b );
        VectorMA( ent->origin, -ent->radius, right, c ); VectorMA( c, -height, up, c );
        VectorMA( ent->origin, ent->radius, right, d ); VectorMA( d, -height, up, d );
        FX_Quad( ent, a, b, c, d );
    } else {
        refEntity_t renderEntity = *ent;
		renderEntity.renderfx &= ~( RF_FORKED | RF_TAPERED | RF_GROW );
		trap_R_AddRefEntityToScene( &renderEntity );
	}
}
