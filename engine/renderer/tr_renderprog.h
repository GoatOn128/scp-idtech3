#ifndef TR_RENDERPROG_H
#define TR_RENDERPROG_H

// Backend-only API. All GL objects are created and used on the render thread.
enum screenProgram_t {
	SCREEN_PARTICLE_PROGRAM,
	SCREEN_DECAL_PROGRAM,
	SCREEN_PROGRAM_COUNT
};

void R_InitRenderProgramDiagnostics( void );
void R_ShutdownRenderPrograms( void );
void RB_CaptureScreenDepth( void );
qboolean RB_BeginScreenPass( screenProgram_t program );
void RB_EndScreenPass( void );
void RB_ScreenSoftness( float value );
void RB_ScreenDecalProjection( const vec3_t origin, const vec3_t right, const vec3_t up, const vec3_t normal );
qboolean RB_ScreenEntityVisible( const refEntity_t *entity );
void RB_BindScreenMaterial( const refEntity_t *entity );
void RB_DrawScreenParticles( void );
void RB_DrawScreenDecals( void );

#endif
