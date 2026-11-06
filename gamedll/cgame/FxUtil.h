
#if !defined(FX_PRIMITIVES_H_INC)
	#include "FxPrimitives.h"
#endif

#ifndef FX_UTIL_H_INC
#define FX_UTIL_H_INC


void FX_GroupMessage( int group, int action, const idVec3 &value );
void FX_AddPrimitive( CEffect **effect, int lifetime );
bool	FX_Free( void );	// ditches all active effects;
int		FX_Init( void );	// called in CG_Init to purge the fx system.
void	FX_Add( void );		// called every cgame frame to add all fx into the scene.
void	FX_Stop( void );	// ditches all active effects without touching the templates.

bool	FX_ActiveFx(void);	// returns whether there are any active or scheduled effects


CParticle *FX_AddParticle( const idVec3 &org, const idVec3 &vel, const idVec3 &accel, 
							float size1, float size2, float sizeParm, 
							float alpha1, float alpha2, float alphaParm, 
							const idVec3 &rgb1, const idVec3 &rgb2, float rgbParm,
							float rotation, float rotationDelta,
							const idVec3 &min, const idVec3 &max, float elasticity, 
							int deathID, int impactID,
							int killTime, qhandle_t shader, int flags );

CParticle *FX_AddParticle( int clientID, const idVec3 &org, const idVec3 &vel, const idVec3 &accel, float gravity,
							float size1, float size2, float sizeParm,
							float alpha1, float alpha2, float alphaParm,
							const idVec3 &rgb1, const idVec3 &rgb2, float rgbParm,
							float rotation, float rotationDelta,
							int killTime, qhandle_t shader, int flags );

CLine *FX_AddLine( idVec3 &start, idVec3 &end, 
							float size1, float size2, float sizeParm,
							float alpha1, float alpha2, float alphaParm,
							idVec3 &rgb1, idVec3 &rgb2, float rgbParm,
							int killTime, qhandle_t shader, int flags );

CLine *FX_AddLine( int clientID, idVec3 &start, float size1, float size2, float sizeParm,
									float alpha1, float alpha2, float alphaParm,
									idVec3 &sRGB, idVec3 &eRGB, float rgbParm,
									int killTime, qhandle_t shader, int impactFX_id, int flags );

CElectricity *FX_AddElectricity( idVec3 &start, idVec3 &end, float size1, float size2, float sizeParm,
							float alpha1, float alpha2, float alphaParm,
							idVec3 &sRGB, idVec3 &eRGB, float rgbParm,
							float chaos, int killTime, qhandle_t shader, int flags );

CTail *FX_AddTail( idVec3 &org, idVec3 &vel, idVec3 &accel, 
							float size1, float size2, float sizeParm, 
							float length1, float length2, float lengthParm,
							float alpha1, float alpha2, float alphaParm,
							idVec3 &rgb1, idVec3 &rgb2, float rgbParm,
							idVec3 &min, idVec3 &max, float elasticity, 
							int deathID, int impactID,
							int killTime, qhandle_t shader, int flags );

CTail *FX_AddTail( int clientID, idVec3 &org, idVec3 &vel, idVec3 &accel, 
							float size1, float size2, float sizeParm, 
							float length1, float length2, float lengthParm,
							float alpha1, float alpha2, float alphaParm,
							idVec3 &sRGB, idVec3 &eRGB, float rgbParm,
							idVec3 &min, idVec3 &max, float elasticity, 
							int deathID, int impactID,
							int killTime, qhandle_t shader, int flags );

CCylinder *FX_AddCylinder( idVec3 &start, idVec3 &normal, 
							float size1s, float size1e, float size1Parm,
							float size2s, float size2e, float size2Parm,
							float length1, float length2, float lengthParm,
							float alpha1, float alpha2, float alphaParm,
							idVec3 &rgb1, idVec3 &rgb2, float rgbParm,
							int killTime, qhandle_t shader, int flags );

CEmitter *FX_AddEmitter( idVec3 &org, idVec3 &vel, idVec3 &accel,
							float size1, float size2, float sizeParm,
							float alpha1, float alpha2, float alphaParm,
							idVec3 &rgb1, idVec3 &rgb2, float rgbParm,
							idVec3 &angs, idVec3 &deltaAngs,
							idVec3 &min, idVec3 &max, float elasticity, 
							int deathID, int impactID, int emitterID,
							float density, float variance,
							int killTime, qhandle_t model, int flags );

CLight *FX_AddLight( idVec3 &org, float size1, float size2, float sizeParm,
							idVec3 &rgb1, idVec3 &rgb2, float rgbParm,
							int killTime, int flags );

COrientedParticle *FX_AddOrientedParticle( idVec3 &org, idVec3 &norm, idVec3 &vel, idVec3 &accel,
							float size1, float size2, float sizeParm,
							float alpha1, float alpha2, float alphaParm,
							idVec3 &rgb1, idVec3 &rgb2, float rgbParm,
							float rotation, float rotationDelta,
							idVec3 &min, idVec3 &max, float bounce,
							int deathID, int impactID,
							int killTime, qhandle_t shader, int flags );

CPoly *FX_AddPoly( idVec3 *verts, idVec2 *st, int numVerts,
							idVec3 &vel, idVec3 &accel,
							float alpha1, float alpha2, float alphaParm,
							idVec3 &rgb1, idVec3 &rgb2, float rgbParm,
							idVec3 &rotationDelta, float bounce, int motionDelay,
							int killTime, qhandle_t shader, int flags );

CFlash *FX_AddFlash( idVec3 &origin, idVec3 &sRGB, idVec3 &eRGB, float rgbParm,
						int life, qhandle_t shader, int flags );


// Convenience functions for programmatic effects.
void FX_AddSprite( idVec3 &origin, idVec3 &vel, idVec3 &accel, 
							float scale, float dscale, 
							float sAlpha, float eAlpha, 
							float rotation, float bounce, 
							int life, qhandle_t shader, int flags = 0 );

void FX_AddSprite( idVec3 &origin, idVec3 &vel, idVec3 &accel, 
							float scale, float dscale, 
							float sAlpha, float eAlpha, 
							idVec3 &sRGB, idVec3 &eRGB, 
							float rotation, float bounce, 
							int life, qhandle_t shader, int flags = 0 );

void FX_AddLine( idVec3 &start, idVec3 &end, float stScale, 
							float width, float dwidth, 
							float sAlpha, float eAlpha, 
							int life, qhandle_t shader, int flags = 0 );

void FX_AddLine( idVec3 &start, idVec3 &end, float stScale, 
							float width, float dwidth, 
							float sAlpha, float eAlpha, 
							idVec3 &sRGB, idVec3 &eRGB, 
							int life, qhandle_t shader, int flags = 0 );

void FX_AddQuad( idVec3 &origin, idVec3 &normal, 
						idVec3 &vel, idVec3 &accel, 
						float sradius, float eradius, 
						float salpha, float ealpha, 
						idVec3 &sRGB, idVec3 &eRGB, 
						float rotation, int life, qhandle_t shader, int flags = 0 );

CBezier *FX_AddBezier( const idVec3 &start, const idVec3 &end, 
						const idVec3 &control1, const idVec3 &control1Vel,
						const idVec3 &control2, const idVec3 &control2Vel,
						float size1, float size2, float sizeParm,
						float alpha1, float alpha2, float alphaParm,
						const idVec3 &sRGB, const idVec3 &eRGB, const float rgbParm,
						int killTime, qhandle_t shader, int flags = 0 );


#endif //FX_UTIL_H_INC