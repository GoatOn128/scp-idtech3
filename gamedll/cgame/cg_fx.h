#pragma once

void CG_FxInit( void );
void CG_FxShutdown( void );
void CG_FxReset( void );
void CG_FxBeginFrame( void );
void CG_FxDraw( void );
void CG_FxPlay( const char *name, const idVec3 &origin, const idVec3 &direction );
void CG_FxTest_f( void );

// Values supplied by the game or map. An .efx primitive must explicitly bind
// each value it uses; unbound appearance remains entirely controlled by data.
struct FxParameters {
	idVec3 velocity;
	float life, size, sizeEnd, alpha, scale, fadeTime, speed, acceleration;
	float heightMin, heightMax, spread;
	float pathLength = 0, aspect = -1, verticalSign = 1;
	idVec3 spreadAxes;
	qhandle_t shader;
	int group;
	FxParameters() : life(-1), size(-1), sizeEnd(-1), alpha(-1), scale(1),
		fadeTime(-1), speed(0), acceleration(0), heightMin(0), heightMax(0), spread(0), spreadAxes(0, 0, 0), shader(0), group(-1) {
		velocity.Zero();
	}
};
void CG_FxPlayParameters( const char *name, const idVec3 &origin, const idVec3 &direction, const FxParameters &parameters );

// Authored settings for continuous Wolf controllers, loaded by the FX parser.
float CG_FxValue( const char *effect, const char *property );
qhandle_t CG_FxMaterial( const char *effect, const char *property );
struct FxTrailInputs {
	int time = -1;
	float life = -1, alpha = -1, alphaEnd = -1, width = -1, widthEnd = -1;
	float ratio = -1, speed = -1;
	int shader = 0, flags = -1, mapping = -1;
	idVec3 color = idVec3( -1, -1, -1 );
	idVec3 colorEnd = idVec3( -1, -1, -1 );
	FxTrailInputs &Time( int value ) { time = value; return *this; }
	FxTrailInputs &Life( float value ) { life = value; return *this; }
	FxTrailInputs &Alpha( float value ) { alpha = value; return *this; }
	FxTrailInputs &AlphaEnd( float value ) { alphaEnd = value; return *this; }
	FxTrailInputs &Width( float value ) { width = value; return *this; }
	FxTrailInputs &WidthEnd( float value ) { widthEnd = value; return *this; }
	FxTrailInputs &Ratio( float value ) { ratio = value; return *this; }
	FxTrailInputs &Speed( float value ) { speed = value; return *this; }
	FxTrailInputs &Shader( int value ) { shader = value; return *this; }
	FxTrailInputs &Flags( int value ) { flags = value; return *this; }
	FxTrailInputs &Mapping( int value ) { mapping = value; return *this; }
	FxTrailInputs &Color( const idVec3 &value ) { color = value; return *this; }
	FxTrailInputs &ColorEnd( const idVec3 &value ) { colorEnd = value; return *this; }
};
int CG_FxTrail( const char *name, int head, const idVec3 &position, const FxTrailInputs &inputs = FxTrailInputs() );

int CG_FxInt( const char *effect, const char *property );
qhandle_t CG_FxModel( const char *effect, const char *property );
