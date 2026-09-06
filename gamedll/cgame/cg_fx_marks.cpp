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

// Projected marks are FX primitives; there is no separate mark allocator or frame loop.
#include "FxScheduler.h"
#include "cg_fx.h"

class CWolfDecal : public CEffect {
    polyVert_t vertices[MAX_VERTS_ON_POLY];
    byte color[4];
    int count, life;
    qhandle_t shader;
    qboolean alphaFade;
    float fadeScale, fadeIn, energyStart, energyLife;
public:
    CWolfDecal( qhandle_t material, const polyVert_t *verts, int numVerts, qboolean fadeAlpha, int duration ) :
        count( numVerts ), life( duration > 0 ? duration : 1 ), shader( material ), alphaFade( fadeAlpha ) {
        memcpy( vertices, verts, count * sizeof( vertices[0] ) );
        memcpy( color, verts[0].modulate, sizeof( color ) );
        const char *profile = material == cgs.media.energyMarkShader ? "wolf/decal_energy" :
            (material == cgs.media.zombieSpiritWallShader ? "wolf/decal_spirit" : "wolf/decal_default");
        fadeScale = CG_FxValue(profile, "fadeScale");
        fadeIn = CG_FxValue(profile, "fadeIn");
        energyStart = CG_FxValue(profile, "energyStart");
        energyLife = CG_FxValue(profile, "energyLife");
        SetFlags( 0 );
    }
    bool Update() override {
        if ( !cg_markTime.integer ) { return true; }
        float age = theFxHelper.mTime - mTimeStart;
        float fade = fadeScale * ( 1 - age / life );
        fade = fade < 0 ? 0 : ( fade > 1 ? 1 : fade );
        float rgbFade = alphaFade ? 1 : fade;
        if ( energyLife > 0 ) {
            float energy = energyStart * ( 1 - age / energyLife );
            rgbFade *= energy < 0 ? 0 : ( energy > 1 ? 1 : energy );
        }
        if ( fadeIn > 0 ) {
            rgbFade *= age < fadeIn ? age / fadeIn : 1;
        }
        for ( int i = 0; i < count; i++ ) {
            for ( int j = 0; j < 3; j++ ) { vertices[i].modulate[j] = static_cast<byte>( color[j] * rgbFade ); }
            vertices[i].modulate[3] = static_cast<byte>( color[3] * ( alphaFade ? fade : 1 ) );
        }
        theFxHelper.AddPolyToScene( shader, count, vertices );
        return true;
    }
};

#define MAX_MARK_FRAGMENTS  384
#define MAX_MARK_POINTS     1024
//#define	MAX_MARK_FRAGMENTS	128
//#define	MAX_MARK_POINTS		384

// these are ignored now for the most part
//#define	MARK_TOTAL_TIME		20000	// (SA) made this a cvar: cg_markTime  (we could cap the time or remove marks quicker if too long a time starts to cause new marks to not appear)
#define MARK_FADE_TIME      10000

void CG_ImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir,
					float orientation, float red, float green, float blue, float alpha,
					qboolean alphaFade, float radius, qboolean temporary, int duration ) {
	vec3_t axis[3];
	float texCoordScale;
	vec3_t originalPoints[4];
	byte colors[4];
	int i, j;
	int numFragments;
	markFragment_t markFragments[MAX_MARK_FRAGMENTS], *mf;
	vec5_t markPoints[MAX_MARK_POINTS];             // Ridah, made it vec5_t so it includes S/T
	vec3_t projection;
	int multMaxFragments = 1;

	if ( !cg_markTime.integer ) {
		return;
	}

	if ( radius <= 0 ) {
		// just ignore it, don't error out
		return;
//		CG_Error( "CG_ImpactMark called with <= 0 radius" );
	}

	// Ridah, if no duration, use the default
	if ( duration < 0 ) {
		if ( duration == -2 ) {
			multMaxFragments = -1;  // use original mapping
		}

//		duration = MARK_TOTAL_TIME;
		duration = cg_markTime.integer;
	}

	// create the texture axis
	VectorNormalize2( dir, axis[0] );
	PerpendicularVector( axis[1], axis[0] );
	RotatePointAroundVector( axis[2], axis[0], axis[1], orientation );
	CrossProduct( axis[0], axis[2], axis[1] );

	texCoordScale = 0.5 * 1.0 / radius;

	// create the full polygon
	for ( i = 0 ; i < 3 ; i++ ) {
		originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
		originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
		originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
		originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
	}

	// get the fragments
	//VectorScale( dir, -20, projection );
	VectorScale( dir, radius * 2, projection );
	numFragments = trap_CM_MarkFragments( (int)orientation, originalPoints,
										  projection, MAX_MARK_POINTS, (float *)&markPoints[0],
										  MAX_MARK_FRAGMENTS * multMaxFragments, markFragments );

	colors[0] = red * 255;
	colors[1] = green * 255;
	colors[2] = blue * 255;
	colors[3] = alpha * 255;

	for ( i = 0, mf = markFragments ; i < numFragments ; i++, mf++ ) {
		polyVert_t  *v;
		polyVert_t verts[MAX_VERTS_ON_POLY];

		qboolean hasST;

		// we have an upper limit on the complexity of polygons
		// that we store persistantly
        hasST = mf->numPoints < 0 ? qtrue : qfalse;
        if ( hasST ) { mf->numPoints = -mf->numPoints; }
        if ( mf->numPoints > MAX_VERTS_ON_POLY ) { mf->numPoints = MAX_VERTS_ON_POLY; }
        if ( mf->numPoints < 3 || mf->firstPoint < 0 || mf->firstPoint + mf->numPoints > MAX_MARK_POINTS ) { continue; }
		for ( j = 0, v = verts ; j < mf->numPoints ; j++, v++ ) {
			vec3_t delta;

			VectorCopy( markPoints[mf->firstPoint + j], v->xyz );

			if ( !hasST ) {
				VectorSubtract( v->xyz, origin, delta );
				v->st[0] = 0.5 + DotProduct( delta, axis[1] ) * texCoordScale;
				v->st[1] = 0.5 + DotProduct( delta, axis[2] ) * texCoordScale;
			} else {
				v->st[0] = markPoints[mf->firstPoint + j][3];
				v->st[1] = markPoints[mf->firstPoint + j][4];
			}

			*(int *)v->modulate = *(int *)colors;
		}

		// if it is a temporary (shadow) mark, add it immediately and forget about it
		if ( temporary ) {
			trap_R_AddPolyToScene( markShader, mf->numPoints, verts );
			continue;
		}

        CEffect *effect = new CWolfDecal( markShader, verts, mf->numPoints, alphaFade, duration );
        FX_AddPrimitive( &effect, duration );
    }
}
