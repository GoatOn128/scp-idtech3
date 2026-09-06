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


#include "tr_local.h"
#include "tr_renderprog.h"
#include <algorithm>
#include <cmath>
#include <vector>

struct screenParticle_t {
	const refEntity_t *entity;
	float depth;
};

/*
====================
RB_DrawScreenParticles

The simulation remains in cgame's FX pool. This pass only draws the submitted
billboards, sorted back to front, and fades intersections against opaque depth.
====================
*/
void RB_DrawScreenParticles( void ) {
	std::vector<screenParticle_t> particles;
	for ( int i = 0; i < backEnd.refdef.num_entities; i++ ) {
		const refEntity_t *entity = &backEnd.refdef.entities[i].e;
		if ( entity->reType != RT_SCREEN_PARTICLE || !RB_ScreenEntityVisible( entity ) ) { continue; }
		vec3_t delta;
		VectorSubtract( entity->origin, backEnd.viewParms.or.origin, delta );
		float depth = DotProduct( delta, backEnd.viewParms.or.axis[0] );
		if ( depth <= 0 ) { continue; }
		particles.push_back( { entity, depth } );
	}
	if ( particles.empty() || !RB_BeginScreenPass( SCREEN_PARTICLE_PROGRAM ) ) { return; }
	std::stable_sort( particles.begin(), particles.end(), []( const screenParticle_t &a, const screenParticle_t &b ) {
		return a.depth > b.depth;
	} );
	const float st[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
	const float signs[4][2] = { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } };
	for ( size_t i = 0; i < particles.size(); i++ ) {
		const refEntity_t *entity = particles[i].entity;
		float aspect = entity->scale[0];
		if ( !std::isfinite( aspect ) || aspect <= 0 || !std::isfinite( entity->rotation ) ) { continue; }
		float angle = DEG2RAD( entity->rotation );
		float cosine = cosf( angle ), sine = sinf( angle );
		vec3_t right, up;
		for ( int j = 0; j < 3; j++ ) {
			right[j] = -backEnd.viewParms.or.axis[1][j] * cosine + backEnd.viewParms.or.axis[2][j] * sine;
			up[j] = backEnd.viewParms.or.axis[1][j] * sine + backEnd.viewParms.or.axis[2][j] * cosine;
		}
		float softness = entity->shaderTexCoord[0];
		if ( !std::isfinite( softness ) || softness <= 0 ) { softness = 1; }
		RB_ScreenSoftness( softness );
		RB_BindScreenMaterial( entity );
		qglBegin( GL_QUADS );
		for ( int corner = 0; corner < 4; corner++ ) {
			vec3_t point;
			VectorMA( entity->origin, signs[corner][0] * entity->radius, right, point );
			VectorMA( point, signs[corner][1] * entity->radius * aspect, up, point );
			qglTexCoord2fv( st[corner] );
			qglVertex3fv( point );
		}
		qglEnd();
	}
	RB_EndScreenPass();
}
