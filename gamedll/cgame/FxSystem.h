
#include "FxWolf.h"

#ifndef FX_SYSTEM_H_INC
#define FX_SYSTEM_H_INC


#define irand	Q_irand
#define flrand	Q_flrand

extern vmCvar_t	fx_debug;
extern vmCvar_t	fx_freeze;


extern void	CG_CalcEntityLerpPositions( centity_t * );	


struct SFxHelper
{
	int		mTime;
	int		mOldTime;
	int		mFrameTime;
	float	mFloatFrameTime;
	bool	mTimeFrozen;

	void	Init();
	void	AdjustTime( int time );

	// These functions are wrapped and used by the fx system in case it makes things a bit more portable
	void	Print( const char *msg, ... );

	// File handling
	int		OpenFile( const char *path, fileHandle_t *fh, int mode );
	int		ReadFile( void *data, int len, fileHandle_t fh );
	void	CloseFile( fileHandle_t fh );

	// Sound
	void	PlaySound( idVec3 &origin, int entityNum, int entchannel, sfxHandle_t sfx );
	int		RegisterSound( const char *sound );

	// Physics/collision
	void	Trace( trace_t *tr, idVec3 &start, idVec3 &min, idVec3 &max, idVec3 &end, int skipEntNum, int flags );

	void	AddFxToScene( FxRefEntity *ent );
	void	AddLightToScene( idVec3 &org, float radius, float red, float green, float blue );

	int		RegisterShader( const char *shader );
	int		RegisterModel( const char *model );

	void	AddPolyToScene( int shader, int count, polyVert_t *verts );

	void	CameraShake( idVec3 &origin, float intensity, int radius, int time );
};

extern SFxHelper	theFxHelper;

#endif // FX_SYSTEM_H_INC
