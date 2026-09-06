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
#include <cmath>

/*
====================
RB_ScreenDecalBounds

Project the volume corners to bound fragment work. A volume crossing the near
plane uses the viewport; the shader still clips every pixel to the volume.
====================
*/
static qboolean RB_ScreenDecalBounds( const refEntity_t *entity, const vec3_t right,
		const vec3_t up, const vec3_t normal, float aspect, float depth, float *bounds ) {
	bounds[0] = bounds[1] = 1;
	bounds[2] = bounds[3] = -1;
	for ( int corner = 0; corner < 8; corner++ ) {
		vec3_t point;
		vec4_t eye, clip;
		VectorMA( entity->origin, ( corner & 1 ? 1 : -1 ) * entity->radius, right, point );
		VectorMA( point, ( corner & 2 ? 1 : -1 ) * entity->radius * aspect, up, point );
		VectorMA( point, ( corner & 4 ? 1 : -1 ) * depth, normal, point );
		R_TransformModelToClip( point, backEnd.viewParms.world.modelMatrix, backEnd.viewParms.projectionMatrix, eye, clip );
		if ( clip[3] <= 0 || clip[2] <= -clip[3] ) {
			bounds[0] = bounds[1] = -1;
			bounds[2] = bounds[3] = 1;
			return qtrue;
		}
		for ( int j = 0; j < 2; j++ ) {
			float value = clip[j] / clip[3];
			if ( value < bounds[j] ) { bounds[j] = value; }
			if ( value > bounds[j + 2] ) { bounds[j + 2] = value; }
		}
	}
	for ( int j = 0; j < 2; j++ ) {
		if ( bounds[j] > 1 || bounds[j + 2] < -1 ) { return qfalse; }
		if ( bounds[j] < -1 ) { bounds[j] = -1; }
		if ( bounds[j + 2] > 1 ) { bounds[j + 2] = 1; }
	}
	return qtrue;
}

/*
====================
RB_DrawScreenDecals

Reconstruct visible receiver positions from opaque depth. No BSP mark fragment
allocation or CPU clipping is required, and camera-inside volumes remain valid.
====================
*/
void RB_DrawScreenDecals( void ) {
	qboolean started = qfalse;
	for ( int i = 0; i < backEnd.refdef.num_entities; i++ ) {
		const refEntity_t *entity = &backEnd.refdef.entities[i].e;
		if ( entity->reType != RT_SCREEN_DECAL || !RB_ScreenEntityVisible( entity ) ) { continue; }
		float depth = entity->shaderTexCoord[1], aspect = entity->scale[0];
		if ( !std::isfinite( depth ) || depth <= 0 || !std::isfinite( aspect ) || aspect <= 0 || !std::isfinite( entity->rotation ) ) { continue; }
		vec3_t normal, right, up, rotated;
		VectorCopy( entity->axis[0], normal );
		float length = VectorNormalize( normal );
		if ( !std::isfinite( length ) || length <= 0 ) { continue; }
		PerpendicularVector( right, normal );
		RotatePointAroundVector( rotated, normal, right, entity->rotation );
		VectorCopy( rotated, right );
		CrossProduct( normal, right, up );
		float bounds[4];
		if ( !RB_ScreenDecalBounds( entity, right, up, normal, aspect, depth, bounds ) ) { continue; }
		if ( !started ) {
			if ( !RB_BeginScreenPass( SCREEN_DECAL_PROGRAM ) ) { return; }
			started = qtrue;
		}
		VectorScale( right, 1.0f / entity->radius, right );
		VectorScale( up, 1.0f / entity->radius * aspect, up );
		VectorScale( normal, 1.0f / depth, normal );
		RB_ScreenDecalProjection( entity->origin, right, up, normal );
		RB_BindScreenMaterial( entity );
		qglBegin( GL_QUADS );
		qglVertex2f( bounds[0], bounds[1] );
		qglVertex2f( bounds[2], bounds[1] );
		qglVertex2f( bounds[2], bounds[3] );
		qglVertex2f( bounds[0], bounds[3] );
		qglEnd();
	}
	if ( started ) { RB_EndScreenPass(); }
}
