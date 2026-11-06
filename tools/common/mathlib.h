#ifndef __MATHLIB__
#define __MATHLIB__

// mathlib.h

#include <math.h>

using vec_t = float;

#define	SIDE_FRONT		0
#define	SIDE_ON			2
#define	SIDE_BACK		1
#ifdef SIDE_CROSS
#undef SIDE_CROSS
#endif
#define	SIDE_CROSS		-2

#define	Q_PI	3.14159265358979323846
#ifdef DEG2RAD
#undef DEG2RAD
#endif
#define DEG2RAD( a ) ( ( (a) * Q_PI ) / 180.0F )
#ifdef RAD2DEG
#undef RAD2DEG
#endif
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / Q_PI )

#ifdef idQ3MapPlaneTypeForNormal
#undef idQ3MapPlaneTypeForNormal
#endif

#define	EQUAL_EPSILON	0.001

// plane types are used to speed some tests
// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2
#define	PLANE_NON_AXIAL	3

qboolean idQ3MapVectorCompare( const idVec3 &v1, const idVec3 &v2 );

#define DotProduct(x,y) (x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define VectorSubtract(a,b,c) {c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];}
#define VectorAdd(a,b,c) {c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];}
#define VectorCopy(a,b) {b[0]=a[0];b[1]=a[1];b[2]=a[2];}
#define VectorScale(a,b,c) {c[0]=b*a[0];c[1]=b*a[1];c[2]=b*a[2];}
#define VectorClear(x) {x[0] = x[1] = x[2] = 0;}
#define	VectorNegate(x) {x[0]=-x[0];x[1]=-x[1];x[2]=-x[2];}
void Vec10Copy( const vec_t *in, vec_t *out );

vec_t idQ3MapQ_rint (vec_t in);
vec_t _DotProduct (const idVec3 &v1, const idVec3 &v2);
void _VectorSubtract (const idVec3 &va, const idVec3 &vb, idVec3 &out);
void _VectorAdd (const idVec3 &va, const idVec3 &vb, idVec3 &out);
void _VectorCopy (const idVec3 &in, idVec3 &out);
void _VectorScale (const idVec3 &v, vec_t scale, idVec3 &out);

double idQ3MapVectorLength( const idVec3 &v );

void idQ3MapVectorMA( const idVec3 &va, double scale, const idVec3 &vb, idVec3 &vc );

void idQ3MapCrossProduct( const idVec3 &v1, const idVec3 &v2, idVec3 &cross );
vec_t idQ3MapVectorNormalize( const idVec3 &in, idVec3 &out );
vec_t idQ3MapColorNormalize( const idVec3 &in, idVec3 &out );
void idQ3MapVectorInverse (idVec3 &v);

void idQ3MapClearBounds (idVec3 &mins, idVec3 &maxs);
void idQ3MapAddPointToBounds( const idVec3 &v, idVec3 &mins, idVec3 &maxs );

qboolean idQ3MapPlaneFromPoints( idVec4 &plane, const idVec3 &a, const idVec3 &b, const idVec3 &c );

void idQ3MapNormalToLatLong( const idVec3 &normal, byte bytes[2] );

int	idQ3MapPlaneTypeForNormal (const idVec3 &normal);

void idQ3MapRotatePointAroundVector( idVec3 &dst, const idVec3 &dir, const idVec3 &point,
							 float degrees );
#endif
