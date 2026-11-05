/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// q_math.c -- stateless support routines that are included in each code module
// The implementations sit on idlib (idMath, idVec3, idMat3); the angle and axis
// helpers keep the engine's Q3 conventions, which differ from idlib's.
#include "idlib/precompiled.h"

// idlib's Str.h redirects the standard string routines; this file still
// talks to the C library directly, so undo the redirects here
#undef strcmp
#undef strncmp
#undef stricmp
#undef _stricmp
#undef strcasecmp
#undef strnicmp
#undef _strnicmp
#undef _memicmp
#undef snprintf
#undef _snprintf
#undef vsnprintf
#undef _vsnprintf
// idlib also predefines the console color macros and the angle shortcuts
// with Doom 3 values; q_shared.h redefines them with the engine's values,
// which must win here
#undef S_COLOR_DEFAULT
#undef S_COLOR_RED
#undef S_COLOR_GREEN
#undef S_COLOR_YELLOW
#undef S_COLOR_BLUE
#undef S_COLOR_CYAN
#undef S_COLOR_ORANGE
#undef S_COLOR_WHITE
#undef S_COLOR_GRAY
#undef S_COLOR_BLACK
#undef ANGLE2SHORT
#undef SHORT2ANGLE
#include "q_shared.h"


vec3_t vec3_origin = {0,0,0};
vec3_t axisDefault[3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };


vec4_t colorBlack  = {0, 0, 0, 1};
vec4_t colorRed    = {1, 0, 0, 1};
vec4_t colorGreen  = {0, 1, 0, 1};
vec4_t colorBlue   = {0, 0, 1, 1};
vec4_t colorYellow = {1, 1, 0, 1};
vec4_t colorMagenta = {1, 0, 1, 1};
vec4_t colorCyan   = {0, 1, 1, 1};
vec4_t colorWhite  = {1, 1, 1, 1};
vec4_t colorLtGrey = {0.75, 0.75, 0.75, 1};
vec4_t colorMdGrey = {0.5, 0.5, 0.5, 1};
vec4_t colorDkGrey = {0.25, 0.25, 0.25, 1};

vec4_t g_color_table[8] =
{
	{0.0, 0.0, 0.0, 1.0},
	{1.0, 0.0, 0.0, 1.0},
	{0.0, 1.0, 0.0, 1.0},
	{1.0, 1.0, 0.0, 1.0},
	{0.0, 0.0, 1.0, 1.0},
	{0.0, 1.0, 1.0, 1.0},
	{1.0, 0.0, 1.0, 1.0},
	{1.0, 1.0, 1.0, 1.0},
};


vec3_t bytedirs[NUMVERTEXNORMALS] =
{
	{-0.525731, 0.000000, 0.850651}, {-0.442863, 0.238856, 0.864188},
	{-0.295242, 0.000000, 0.955423}, {-0.309017, 0.500000, 0.809017},
	{-0.162460, 0.262866, 0.951056}, {0.000000, 0.000000, 1.000000},
	{0.000000, 0.850651, 0.525731}, {-0.147621, 0.716567, 0.681718},
	{0.147621, 0.716567, 0.681718}, {0.000000, 0.525731, 0.850651},
	{0.309017, 0.500000, 0.809017}, {0.525731, 0.000000, 0.850651},
	{0.295242, 0.000000, 0.955423}, {0.442863, 0.238856, 0.864188},
	{0.162460, 0.262866, 0.951056}, {-0.681718, 0.147621, 0.716567},
	{-0.809017, 0.309017, 0.500000},{-0.587785, 0.425325, 0.688191},
	{-0.850651, 0.525731, 0.000000},{-0.864188, 0.442863, 0.238856},
	{-0.716567, 0.681718, 0.147621},{-0.688191, 0.587785, 0.425325},
	{-0.500000, 0.809017, 0.309017}, {-0.238856, 0.864188, 0.442863},
	{-0.425325, 0.688191, 0.587785}, {-0.716567, 0.681718, -0.147621},
	{-0.500000, 0.809017, -0.309017}, {-0.525731, 0.850651, 0.000000},
	{0.000000, 0.850651, -0.525731}, {-0.238856, 0.864188, -0.442863},
	{0.000000, 0.955423, -0.295242}, {-0.262866, 0.951056, -0.162460},
	{0.000000, 1.000000, 0.000000}, {0.000000, 0.955423, 0.295242},
	{-0.262866, 0.951056, 0.162460}, {0.238856, 0.864188, 0.442863},
	{0.262866, 0.951056, 0.162460}, {0.500000, 0.809017, 0.309017},
	{0.238856, 0.864188, -0.442863},{0.262866, 0.951056, -0.162460},
	{0.500000, 0.809017, -0.309017},{0.850651, 0.525731, 0.000000},
	{0.716567, 0.681718, 0.147621}, {0.716567, 0.681718, -0.147621},
	{0.525731, 0.850651, 0.000000}, {0.425325, 0.688191, 0.587785},
	{0.864188, 0.442863, 0.238856}, {0.688191, 0.587785, 0.425325},
	{0.809017, 0.309017, 0.500000}, {0.681718, 0.147621, 0.716567},
	{0.587785, 0.425325, 0.688191}, {0.955423, 0.295242, 0.000000},
	{1.000000, 0.000000, 0.000000}, {0.951056, 0.162460, 0.262866},
	{0.850651, -0.525731, 0.000000},{0.955423, -0.295242, 0.000000},
	{0.864188, -0.442863, 0.238856}, {0.951056, -0.162460, 0.262866},
	{0.809017, -0.309017, 0.500000}, {0.681718, -0.147621, 0.716567},
	{0.850651, 0.000000, 0.525731}, {0.864188, 0.442863, -0.238856},
	{0.809017, 0.309017, -0.500000}, {0.951056, 0.162460, -0.262866},
	{0.525731, 0.000000, -0.850651}, {0.681718, 0.147621, -0.716567},
	{0.681718, -0.147621, -0.716567},{0.850651, 0.000000, -0.525731},
	{0.809017, -0.309017, -0.500000}, {0.864188, -0.442863, -0.238856},
	{0.951056, -0.162460, -0.262866}, {0.147621, 0.716567, -0.681718},
	{0.309017, 0.500000, -0.809017}, {0.425325, 0.688191, -0.587785},
	{0.442863, 0.238856, -0.864188}, {0.587785, 0.425325, -0.688191},
	{0.688191, 0.587785, -0.425325}, {-0.147621, 0.716567, -0.681718},
	{-0.309017, 0.500000, -0.809017}, {0.000000, 0.525731, -0.850651},
	{-0.525731, 0.000000, -0.850651}, {-0.442863, 0.238856, -0.864188},
	{-0.295242, 0.000000, -0.955423}, {-0.162460, 0.262866, -0.951056},
	{0.000000, 0.000000, -1.000000}, {0.295242, 0.000000, -0.955423},
	{0.162460, 0.262866, -0.951056}, {-0.442863, -0.238856, -0.864188},
	{-0.309017, -0.500000, -0.809017}, {-0.162460, -0.262866, -0.951056},
	{0.000000, -0.850651, -0.525731}, {-0.147621, -0.716567, -0.681718},
	{0.147621, -0.716567, -0.681718}, {0.000000, -0.525731, -0.850651},
	{0.309017, -0.500000, -0.809017}, {0.442863, -0.238856, -0.864188},
	{0.162460, -0.262866, -0.951056}, {0.238856, -0.864188, -0.442863},
	{0.500000, -0.809017, -0.309017}, {0.425325, -0.688191, -0.587785},
	{0.716567, -0.681718, -0.147621}, {0.688191, -0.587785, -0.425325},
	{0.587785, -0.425325, -0.688191}, {0.000000, -0.955423, -0.295242},
	{0.000000, -1.000000, 0.000000}, {0.262866, -0.951056, -0.162460},
	{0.000000, -0.850651, 0.525731}, {0.000000, -0.955423, 0.295242},
	{0.238856, -0.864188, 0.442863}, {0.262866, -0.951056, 0.162460},
	{0.500000, -0.809017, 0.309017}, {0.716567, -0.681718, 0.147621},
	{0.525731, -0.850651, 0.000000}, {-0.238856, -0.864188, -0.442863},
	{-0.500000, -0.809017, -0.309017}, {-0.262866, -0.951056, -0.162460},
	{-0.850651, -0.525731, 0.000000}, {-0.716567, -0.681718, -0.147621},
	{-0.716567, -0.681718, 0.147621}, {-0.525731, -0.850651, 0.000000},
	{-0.500000, -0.809017, 0.309017}, {-0.238856, -0.864188, 0.442863},
	{-0.262866, -0.951056, 0.162460}, {-0.864188, -0.442863, 0.238856},
	{-0.809017, -0.309017, 0.500000}, {-0.688191, -0.587785, 0.425325},
	{-0.681718, -0.147621, 0.716567}, {-0.442863, -0.238856, 0.864188},
	{-0.587785, -0.425325, 0.688191}, {-0.309017, -0.500000, 0.809017},
	{-0.147621, -0.716567, 0.681718}, {-0.425325, -0.688191, 0.587785},
	{-0.162460, -0.262866, 0.951056}, {0.442863, -0.238856, 0.864188},
	{0.162460, -0.262866, 0.951056}, {0.309017, -0.500000, 0.809017},
	{0.147621, -0.716567, 0.681718}, {0.000000, -0.525731, 0.850651},
	{0.425325, -0.688191, 0.587785}, {0.587785, -0.425325, 0.688191},
	{0.688191, -0.587785, 0.425325}, {-0.955423, 0.295242, 0.000000},
	{-0.951056, 0.162460, 0.262866}, {-1.000000, 0.000000, 0.000000},
	{-0.850651, 0.000000, 0.525731}, {-0.955423, -0.295242, 0.000000},
	{-0.951056, -0.162460, 0.262866}, {-0.864188, 0.442863, -0.238856},
	{-0.951056, 0.162460, -0.262866}, {-0.809017, 0.309017, -0.500000},
	{-0.864188, -0.442863, -0.238856}, {-0.951056, -0.162460, -0.262866},
	{-0.809017, -0.309017, -0.500000}, {-0.681718, 0.147621, -0.716567},
	{-0.681718, -0.147621, -0.716567}, {-0.850651, 0.000000, -0.525731},
	{-0.688191, 0.587785, -0.425325}, {-0.587785, 0.425325, -0.688191},
	{-0.425325, 0.688191, -0.587785}, {-0.425325, -0.688191, -0.587785},
	{-0.587785, -0.425325, -0.688191}, {-0.688191, -0.587785, -0.425325}
};

//==============================================================

int     Q_rand( int *seed ) {
	*seed = ( 69069 * *seed + 1 );
	return *seed;
}

float   Q_random( int *seed ) {
	return ( Q_rand( seed ) & 0xffff ) / (float)0x10000;
}

float   Q_crandom( int *seed ) {
	return 2.0 * ( Q_random( seed ) - 0.5 );
}


//=======================================================

signed char ClampChar( int i ) {
	return idMath::ClampChar( i );
}

signed short ClampShort( int i ) {
	return idMath::ClampShort( i );
}


// this isn't a real cheap function to call!
int DirToByte( vec3_t dir ) {
	if ( !dir ) {
		return 0;
	}

	const idVec3 & d = *( const idVec3 * ) dir;
	float bestd = 0.0F;
	int best = 0;
	for ( int i = 0 ; i < NUMVERTEXNORMALS ; i++ ) {
		float dot = d * ( *( const idVec3 * ) bytedirs[i] );
		if ( dot > bestd ) {
			bestd = dot;
			best = i;
		}
	}

	return best;
}

void ByteToDir( int b, vec3_t dir ) {
	if ( b < 0 || b >= NUMVERTEXNORMALS ) {
		*( idVec3 * ) dir = *( const idVec3 * ) vec3_origin;
		return;
	}
	*( idVec3 * ) dir = *( const idVec3 * ) bytedirs[b];
}


unsigned ColorBytes3( float r, float g, float b ) {
	unsigned i;

	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;

	return i;
}

unsigned ColorBytes4( float r, float g, float b, float a ) {
	unsigned i;

	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;
	( (byte *)&i )[3] = a * 255;

	return i;
}

float NormalizeColor( const vec3_t in, vec3_t out ) {
	float max;

	max = in[0];
	if ( in[1] > max ) {
		max = in[1];
	}
	if ( in[2] > max ) {
		max = in[2];
	}

	if ( !max ) {
		VectorClear( out );
	} else {
		out[0] = in[0] / max;
		out[1] = in[1] / max;
		out[2] = in[2] / max;
	}
	return max;
}


/*
=====================
PlaneFromPoints

Returns false if the triangle is degenrate.
The normal will point out of the clock for clockwise ordered points
=====================
*/
qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c ) {
	idVec3 d1 = *( const idVec3 * ) b - *( const idVec3 * ) a;
	idVec3 d2 = *( const idVec3 * ) c - *( const idVec3 * ) a;

	idVec3 & n = *( idVec3 * ) plane;
	n = d2.Cross( d1 );
	if ( n.Normalize() == 0.0F ) {
		return qfalse;
	}

	plane[3] = *( const idVec3 * ) a * n;
	return qtrue;
}

/*
===============
RotatePointAroundVector

This is not implemented very well...
===============
*/
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point,
							  float degrees ) {
	const idVec3 & vf = *( const idVec3 * ) dir;
	idVec3 vr, vup;

	PerpendicularVector( vr.ToFloatPtr(), dir );
	vup = vr.Cross( vf );

	// idMat3( a, b, c ) sets the rows; the engine's basis matrix has
	// vr, vup and vf as its columns
	idMat3 im( vr, vup, vf );
	idMat3 m = im.Transpose();

	float rad = DEG2RAD( degrees );
	idMat3 zrot( cos( rad ), sin( rad ), 0.0F,
				 -sin( rad ), cos( rad ), 0.0F,
				 0.0F, 0.0F, 1.0F );

	idMat3 rot = m * zrot * im;

	// the engine multiplies the matrix rows by the point, idMat3
	// multiplies by its columns
	*( idVec3 * ) dst = rot.Transpose() * ( *( const idVec3 * ) point );
}

/*
===============
RotateAroundDirection
===============
*/
void RotateAroundDirection( vec3_t axis[3], float yaw ) {

	// create an arbitrary axis[1]
	PerpendicularVector( axis[1], axis[0] );

	// rotate it around axis[0] by yaw
	if ( yaw ) {
		vec3_t temp;

		VectorCopy( axis[1], temp );
		RotatePointAroundVector( axis[1], axis[0], temp, yaw );
	}

	// cross to get axis[2]
	CrossProduct( axis[0], axis[1], axis[2] );
}



void vectoangles( const vec3_t value1, vec3_t angles ) {
	float forward;
	float yaw, pitch;

	if ( value1[1] == 0 && value1[0] == 0 ) {
		yaw = 0;
		if ( value1[2] > 0 ) {
			pitch = 90;
		} else {
			pitch = 270;
		}
	} else {
		if ( value1[0] ) {
			yaw = ( atan2( value1[1], value1[0] ) * 180 / M_PI );
		} else if ( value1[1] > 0 )   {
			yaw = 90;
		} else {
			yaw = 270;
		}
		if ( yaw < 0 ) {
			yaw += 360;
		}

		forward = sqrt( value1[0] * value1[0] + value1[1] * value1[1] );
		pitch = ( atan2( value1[2], forward ) * 180 / M_PI );
		if ( pitch < 0 ) {
			pitch += 360;
		}
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}


/*
=================
AnglesToAxis
=================
*/
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] ) {
	vec3_t right;

	// angle vectors returns "right" instead of "y axis"
	AngleVectors( angles, axis[0], right, axis[2] );
	VectorSubtract( vec3_origin, right, axis[1] );
}

void AxisClear( vec3_t axis[3] ) {
	*( idMat3 * ) axis = idMat3( 1, 0, 0, 0, 1, 0, 0, 0, 1 );
}

void AxisCopy( vec3_t in[3], vec3_t out[3] ) {
	*( idMat3 * ) out = *( const idMat3 * ) in;
}

void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal ) {
	const idVec3 & n = *( const idVec3 * ) normal;
	const idVec3 & pt = *( const idVec3 * ) p;

	float inv_denom = 1.0F / ( n * n );
	float d = ( n * pt ) * inv_denom;
	idVec3 nscaled = n * inv_denom;

	*( idVec3 * ) dst = pt - nscaled * d;
}

/*
================
MakeNormalVectors

Given a normalized forward vector, create two
other perpendicular vectors
================
*/
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up ) {
	const idVec3 & f = *( const idVec3 * ) forward;
	idVec3 r( f[2], -f[0], f[1] );	// rotate and negate: not colinear with f

	float d = r * f;
	r -= f * d;
	r.Normalize();

	*( idVec3 * ) right = r;
	*( idVec3 * ) up = r.Cross( f );
}


void VectorRotate( vec3_t in, vec3_t matrix[3], vec3_t out ) {
	// the engine multiplies the matrix rows by the vector, idMat3
	// multiplies by its columns
	*( idVec3 * ) out = ( *( idMat3 * ) matrix ).Transpose() * ( *( const idVec3 * ) in );
}

//============================================================================

/*
** float q_rsqrt( float number )
*/
float Q_rsqrt( float number ) {
	return idMath::InvSqrt( number );
}

float Q_fabs( float f ) {
	return idMath::Fabs( f );
}

//============================================================

/*
===============
LerpAngle

===============
*/
float LerpAngle( float from, float to, float frac ) {
	float a;

	if ( to - from > 180 ) {
		to -= 360;
	}
	if ( to - from < -180 ) {
		to += 360;
	}
	a = from + frac * ( to - from );

	return a;
}


/*
=================
AngleSubtract

Always returns a value from -180 to 180
=================
*/
float   AngleSubtract( float a1, float a2 ) {
	float a;

	a = a1 - a2;
	while ( a > 180 ) {
		a -= 360;
	}
	while ( a < -180 ) {
		a += 360;
	}
	return a;
}


void AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 ) {
	v3[0] = AngleSubtract( v1[0], v2[0] );
	v3[1] = AngleSubtract( v1[1], v2[1] );
	v3[2] = AngleSubtract( v1[2], v2[2] );
}


float   AngleMod( float a ) {
	a = ( 360.0 / 65536 ) * ( (int)( a * ( 65536 / 360.0 ) ) & 65535 );
	return a;
}


/*
=================
AngleNormalize360

returns angle normalized to the range [0 <= angle < 360]
=================
*/
float AngleNormalize360( float angle ) {
	return idMath::AngleNormalize360( angle );
}


/*
=================
AngleNormalize180

returns angle normalized to the range [-180 < angle <= 180]
=================
*/
float AngleNormalize180( float angle ) {
	return idMath::AngleNormalize180( angle );
}


/*
=================
AngleDelta

returns the normalized delta from angle1 to angle2
=================
*/
float AngleDelta( float angle1, float angle2 ) {
	return idMath::AngleDelta( angle1, angle2 );
}


//============================================================


/*
=================
SetPlaneSignbits
=================
*/
void SetPlaneSignbits( cplane_t *out ) {
	int bits, j;

	// for fast box on planeside test
	bits = 0;
	for ( j = 0 ; j < 3 ; j++ ) {
		if ( out->normal[j] < 0 ) {
			bits |= 1 << j;
		}
	}
	out->signbits = bits;
}


/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2

// this is the slow, general version
int BoxOnPlaneSide2 (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	int		i;
	float	dist1, dist2;
	int		sides;
	vec3_t	corners[2];

	for (i=0 ; i<3 ; i++)
	{
		if (p->normal[i] < 0)
		{
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		}
		else
		{
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}
	dist1 = DotProduct (p->normal, corners[0]) - p->dist;
	dist2 = DotProduct (p->normal, corners[1]) - p->dist;
	sides = 0;
	if (dist1 >= 0)
		sides = 1;
	if (dist2 < 0)
		sides |= 2;

	return sides;
}

==================
*/
int BoxOnPlaneSide( vec3_t emins, vec3_t emaxs, struct cplane_s *p ) {
	float dist1, dist2;
	int sides;

// fast axial cases
	if ( p->type < 3 ) {
		if ( p->dist <= emins[p->type] ) {
			return 1;
		}
		if ( p->dist >= emaxs[p->type] ) {
			return 2;
		}
		return 3;
	}

// general case
	switch ( p->signbits )
	{
	case 0:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		break;
	case 1:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		break;
	case 2:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		break;
	case 3:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		break;
	case 4:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;      // shut up compiler
		break;
	}

	sides = 0;
	if ( dist1 >= p->dist ) {
		sides = 1;
	}
	if ( dist2 < p->dist ) {
		sides |= 2;
	}

	return sides;
}


/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds( const vec3_t mins, const vec3_t maxs ) {
	idVec3 corner;

	for ( int i = 0 ; i < 3 ; i++ ) {
		float a = idMath::Fabs( mins[i] );
		float b = idMath::Fabs( maxs[i] );
		corner[i] = a > b ? a : b;
	}

	return corner.Length();
}


void ClearBounds( vec3_t mins, vec3_t maxs ) {
	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;
}

void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs ) {
	if ( v[0] < mins[0] ) {
		mins[0] = v[0];
	}
	if ( v[0] > maxs[0] ) {
		maxs[0] = v[0];
	}

	if ( v[1] < mins[1] ) {
		mins[1] = v[1];
	}
	if ( v[1] > maxs[1] ) {
		maxs[1] = v[1];
	}

	if ( v[2] < mins[2] ) {
		mins[2] = v[2];
	}
	if ( v[2] > maxs[2] ) {
		maxs[2] = v[2];
	}
}


int VectorCompare( const vec3_t v1, const vec3_t v2 ) {
	return ( *( const idVec3 * ) v1 == *( const idVec3 * ) v2 ) ? 1 : 0;
}


vec_t VectorNormalize( vec3_t v ) {
	return ( *( idVec3 * ) v ).Normalize();
}

//
// fast vector normalize routine that does not check to make sure
// that length != 0, nor does it return length
//
void VectorNormalizeFast( vec3_t v ) {
	( *( idVec3 * ) v ).NormalizeFast();
}

vec_t VectorNormalize2( const vec3_t v, vec3_t out ) {
	idVec3 n = *( const idVec3 * ) v;
	float length = n.Normalize();
	*( idVec3 * ) out = n;
	return length;
}

void _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc ) {
	*( idVec3 * ) vecc = *( const idVec3 * ) veca + *( const idVec3 * ) vecb * scale;
}


vec_t _DotProduct( const vec3_t v1, const vec3_t v2 ) {
	return ( *( const idVec3 * ) v1 ) * ( *( const idVec3 * ) v2 );
}

void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out ) {
	*( idVec3 * ) out = *( const idVec3 * ) veca - *( const idVec3 * ) vecb;
}

void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out ) {
	*( idVec3 * ) out = *( const idVec3 * ) veca + *( const idVec3 * ) vecb;
}

void _VectorCopy( const vec3_t in, vec3_t out ) {
	*( idVec3 * ) out = *( const idVec3 * ) in;
}

void _VectorScale( const vec3_t in, vec_t scale, vec3_t out ) {
	*( idVec3 * ) out = *( const idVec3 * ) in * scale;
}

void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross ) {
	*( idVec3 * ) cross = ( *( const idVec3 * ) v1 ).Cross( *( const idVec3 * ) v2 );
}

vec_t VectorLength( const vec3_t v ) {
	return ( *( const idVec3 * ) v ).Length();
}

vec_t VectorLengthSquared( const vec3_t v ) {
	return ( *( const idVec3 * ) v ).LengthSqr();
}

vec_t Distance( const vec3_t p1, const vec3_t p2 ) {
	return ( *( const idVec3 * ) p2 - *( const idVec3 * ) p1 ).Length();
}

vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 ) {
	return ( *( const idVec3 * ) p2 - *( const idVec3 * ) p1 ).LengthSqr();
}


void VectorInverse( vec3_t v ) {
	*( idVec3 * ) v = -( *( const idVec3 * ) v );
}

void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out ) {
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
	out[3] = in[3] * scale;
}


int Q_log2( int val ) {
	int answer;

	answer = 0;
	while ( ( val >>= 1 ) != 0 ) {
		answer++;
	}
	return answer;
}



/*
=================
PlaneTypeForNormal
=================
*/
/*
int	PlaneTypeForNormal (vec3_t normal) {
	if ( normal[0] == 1.0 )
		return PLANE_X;
	if ( normal[1] == 1.0 )
		return PLANE_Y;
	if ( normal[2] == 1.0 )
		return PLANE_Z;

	return PLANE_NON_AXIAL;
}
*/


/*
================
MatrixMultiply
================
*/
void MatrixMultiply( float in1[3][3], float in2[3][3], float out[3][3] ) {
	*( idMat3 * ) out = ( *( const idMat3 * ) in1 ) * ( *( const idMat3 * ) in2 );
}


void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up ) {
	float angle;
	static float sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[YAW] * ( M_PI * 2 / 360 );
	sy = sin( angle );
	cy = cos( angle );
	angle = angles[PITCH] * ( M_PI * 2 / 360 );
	sp = sin( angle );
	cp = cos( angle );
	angle = angles[ROLL] * ( M_PI * 2 / 360 );
	sr = sin( angle );
	cr = cos( angle );

	if ( forward ) {
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}
	if ( right ) {
		right[0] = ( -1 * sr * sp * cy + - 1 * cr * -sy );
		right[1] = ( -1 * sr * sp * sy + - 1 * cr * cy );
		right[2] = -1 * sr * cp;
	}
	if ( up ) {
		up[0] = ( cr * sp * cy + - sr * -sy );
		up[1] = ( cr * sp * sy + - sr * cy );
		up[2] = cr * cp;
	}
}

/*
** assumes "src" is normalized
*/
void PerpendicularVector( vec3_t dst, const vec3_t src ) {
	// find the smallest magnitude axially aligned vector
	int pos = 0;
	float minelem = 1.0F;
	for ( int i = 0; i < 3; i++ ) {
		if ( idMath::Fabs( src[i] ) < minelem ) {
			pos = i;
			minelem = idMath::Fabs( src[i] );
		}
	}

	idVec3 tempvec( 0.0F, 0.0F, 0.0F );
	tempvec[pos] = 1.0F;

	// project the point onto the plane defined by src and normalize
	ProjectPointOnPlane( dst, tempvec.ToFloatPtr(), src );
	VectorNormalize( dst );
}

// Ridah
/*
=================
GetPerpendicularViewVector

  Used to find an "up" vector for drawing a sprite so that it always faces the view as best as possible
=================
*/
void GetPerpendicularViewVector( const vec3_t point, const vec3_t p1, const vec3_t p2, vec3_t up ) {
	idVec3 v1 = *( const idVec3 * ) point - *( const idVec3 * ) p1;
	idVec3 v2 = *( const idVec3 * ) point - *( const idVec3 * ) p2;
	v1.Normalize();
	v2.Normalize();

	idVec3 & upvec = *( idVec3 * ) up;
	upvec = v1.Cross( v2 );
	upvec.Normalize();
}

/*
================
ProjectPointOntoVector
================
*/
void ProjectPointOntoVector( vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj ) {
	idVec3 pVec = *( const idVec3 * ) point - *( const idVec3 * ) vStart;
	idVec3 vec = *( const idVec3 * ) vEnd - *( const idVec3 * ) vStart;
	vec.Normalize();
	// project onto the directional vector for this segment
	*( idVec3 * ) vProj = *( const idVec3 * ) vStart + vec * ( pVec * vec );
}

float vectoyaw( const vec3_t vec ) {
	float yaw;

	if ( vec[YAW] == 0 && vec[PITCH] == 0 ) {
		yaw = 0;
	} else {
		if ( vec[PITCH] ) {
			yaw = ( atan2( vec[YAW], vec[PITCH] ) * 180 / M_PI );
		} else if ( vec[YAW] > 0 ) {
			yaw = 90;
		} else {
			yaw = 270;
		}
		if ( yaw < 0 ) {
			yaw += 360;
		}
	}

	return yaw;
}

/*
=================
AxisToAngles

  Used to convert the MD3 tag axis to MDC tag angles, which are much smaller

  This doesn't have to be fast, since it's only used for conversion in utils, try to avoid
  using this during gameplay
=================
*/
void AxisToAngles( vec3_t axis[3], vec3_t angles ) {
	vec3_t right, roll_angles, tvec;

	// first get the pitch and yaw from the forward vector
	vectoangles( axis[0], angles );

	// now get the roll from the right vector
	VectorCopy( axis[1], right );
	// get the angle difference between the tmpAxis[2] and axis[2] after they have been reverse-rotated
	RotatePointAroundVector( tvec, axisDefault[2], right, -angles[YAW] );
	RotatePointAroundVector( right, axisDefault[1], tvec, -angles[PITCH] );
	// now find the angles, the PITCH is effectively our ROLL
	vectoangles( right, roll_angles );
	roll_angles[PITCH] = AngleNormalize180( roll_angles[PITCH] );
	// if the yaw is more than 90 degrees difference, we should adjust the pitch
	if ( DotProduct( right, axisDefault[1] ) < 0 ) {
		if ( roll_angles[PITCH] < 0 ) {
			roll_angles[PITCH] = -90 + ( -90 - roll_angles[PITCH] );
		} else {
			roll_angles[PITCH] =  90 + ( 90 - roll_angles[PITCH] );
		}
	}

	angles[ROLL] = -roll_angles[PITCH];
}

float VectorDistance( vec3_t v1, vec3_t v2 ) {
	return ( *( const idVec3 * ) v2 - *( const idVec3 * ) v1 ).Length();
}
// done.
