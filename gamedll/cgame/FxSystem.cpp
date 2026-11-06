// this include must remain at the top of every FXxxxx.CPP file
#include "FxWolf.h"

#if !defined(FX_SCHEDULER_H_INC)
	#include "FxScheduler.h"
#endif

vmCvar_t fx_debug;
vmCvar_t fx_freeze;

extern void CG_StartShakeCamera( float intensity, int time, idVec3 &origin, float radius );

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
void SFxHelper::PlaySound( idVec3 &org, int entityNum, int entchannel, int sfxHandle )
{
	trap_S_StartSound( &org, entityNum, entchannel, sfxHandle );
}

//------------------------------------------------------
void SFxHelper::Trace( trace_t *tr, idVec3 &start, idVec3 &min, idVec3 &max, 
						idVec3 &end, int skipEntNum, int flags )
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
void SFxHelper::AddLightToScene( idVec3 &org, float radius, float red, float green, float blue )
{
	trap_R_AddLightToScene( org, radius, red, green, blue, 0 );
}

//------------------------------------------------------
void SFxHelper::AddPolyToScene( int shader, int count, polyVert_t *verts )
{
	trap_R_AddPolyToScene( shader, count, verts );
}

//------------------------------------------------------
void SFxHelper::CameraShake( idVec3 &origin, float intensity, int radius, int time )
{
	CG_StartShakeCamera( intensity, time, origin, radius );
}

static void FX_Quad( const FxRefEntity *ent, const idVec3 &a, const idVec3 &b,
					 const idVec3 &c, const idVec3 &d ) {
	polyVert_t verts[4];
	const idVec3 *points[4] = { &a, &b, &c, &d };
	const float st[4][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };
	for ( int i = 0; i < 4; i++ ) {
		verts[i].xyz = *points[i];
		verts[i].st[0] = st[i][0];
		verts[i].st[1] = st[i][1];
		memcpy( verts[i].modulate, ent->shaderRGBA, 4 );
	}
	trap_R_AddPolyToScene( ent->customShader, 4, verts );
}

static void FX_Line( const FxRefEntity *ent, const idVec3 &start, const idVec3 &end,
					 float startWidth, float endWidth ) {
	idVec3 direction, view, side, a, b, c, d;
	direction = end - start;
	if ( direction.Normalize() == 0 ) {
		return;
	}
	view = cg.refdef.vieworg - start;
	side = direction.Cross( view );
	if ( side.Normalize() == 0 ) {
		side.PerpendicularVector( direction );
	}
	a = start + side * startWidth;
	b = start + side * ( -startWidth );
	c = end + side * ( -endWidth );
	d = end + side * endWidth;
	FX_Quad( ent, a, b, c, d );
}

static void FX_RenderPrimitive( FxRefEntity *ent ) {
	if ( ent->reType == RT_LINE ) {
		FX_Line( ent, ent->origin, ent->oldorigin, ent->radius, ent->radius );
	} else if ( ent->reType == RT_ELECTRICITY ) {
		idVec3 previous, point, direction, side, up, end;
		end = ent->oldorigin;
		if ( ent->renderfx & RF_GROW ) {
			float life = ent->angles[1];
			float fraction = life > 0 ? 1.0f - ( ent->endTime - cg.time ) / life : 1.0f;
			fraction = fraction < 0 ? 0 : ( fraction > 1 ? 1 : fraction );
			direction = end - ent->origin;
			end = ent->origin + direction * fraction;
		}
		direction = end - ent->origin;
		float length = direction.Normalize();
		if ( length == 0 ) {
			return;
		}
		side.PerpendicularVector( direction );
		up = direction.Cross( side );
		previous = ent->origin;
		int seed = ent->frame + theFxHelper.mTime / 50;
		for ( int i = 1; i <= 16; i++ ) {
			float fraction = i / 16.0f;
			point = ent->origin + direction * ( length * fraction );
			if ( i < 16 ) {
				float chaos = ent->angles[0] * length * 0.05f;
				point = point + side * ( Q_crandom( &seed ) * chaos );
				point = point + up * ( Q_crandom( &seed ) * chaos );
			}
			float width = ent->radius * ( ( ent->renderfx & RF_TAPERED ) ? 1.0f - fraction : 1.0f );
			FX_Line( ent, previous, point, width, width );
			if ( ( ent->renderfx & RF_FORKED ) && i == 8 ) {
				idVec3 branch;
				branch = point + side * ( length * 0.2f );
				branch = branch + direction * ( length * 0.2f );
				FX_Line( ent, point, branch, width * 0.5f, 0 );
			}
			previous = point;
		}
	} else if ( ent->reType == RT_ORIENTED_QUAD ) {
		idVec3 right, up, a, b, c, d;
		up.PerpendicularVector( ent->axis[0] );
		right.RotatePointAroundVector( ent->axis[0], up, ent->rotation );
		up = ent->axis[0].Cross( right );
		a = ent->origin + right * ent->radius;
		a = a + up * ent->radius;
		b = ent->origin + right * ( -ent->radius );
		b = b + up * ent->radius;
		c = ent->origin + right * ( -ent->radius );
		c = c + up * ( -ent->radius );
		d = ent->origin + right * ent->radius;
		d = d + up * ( -ent->radius );
		FX_Quad( ent, a, b, c, d );
	} else if ( ent->reType == RT_CYLINDER ) {
		idVec3 side, up, radial[2], a, b, c, d;
		side.PerpendicularVector( ent->axis[0] );
		up = ent->axis[0].Cross( side );
		for ( int i = 0; i < 16; i++ ) {
			for ( int j = 0; j < 2; j++ ) {
				float angle = ( i + j ) * 2.0f * M_PI / 16;
				radial[j] = side * ( cosf( angle ) );
				radial[j] = radial[j] + up * ( sinf( angle ) );
			}
			a = ent->origin + radial[0] * ent->radius;
			b = ent->origin + radial[1] * ent->radius;
			c = ent->oldorigin + radial[1] * ent->backlerp;
			d = ent->oldorigin + radial[0] * ent->backlerp;
			FX_Quad( ent, a, b, c, d );
		}
	} else if ( ent->reType == RT_SPRITE && ent->aspectRatio > 0 && ent->aspectRatio != 1 ) {
        idVec3 right, up, a, b, c, d;
        right.RotatePointAroundVector( cg.refdef.viewaxis[0], cg.refdef.viewaxis[1], ent->rotation );
        up = cg.refdef.viewaxis[0].Cross( right );
        float height = ent->radius * ent->aspectRatio;
        a = ent->origin + right * ent->radius; a = a + up * height;
        b = ent->origin + right * ( -ent->radius ); b = b + up * height;
        c = ent->origin + right * ( -ent->radius ); c = c + up * ( -height );
        d = ent->origin + right * ent->radius; d = d + up * ( -height );
        FX_Quad( ent, a, b, c, d );
    } else {
        refEntity_t renderEntity = *ent;
		renderEntity.renderfx &= ~( RF_FORKED | RF_TAPERED | RF_GROW );
		trap_R_AddRefEntityToScene( &renderEntity );
	}
}
