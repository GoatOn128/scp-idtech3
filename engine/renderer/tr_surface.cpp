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

// tr_surf.c
#include "tr_local.h"

/*

  THIS ENTIRE FILE IS BACK END

backEnd.currentEntity will be valid.

Tess_Begin has already been called for the surface's shader.

The modelview matrix will be set.

It is safe to actually issue drawing commands here if you don't want to
use the shader system.
*/


//============================================================================


/*
==============
RB_CheckOverflow
==============
*/
void RB_CheckOverflow( int verts, int indexes ) {
	if ( tess.numVertexes + verts < SHADER_MAX_VERTEXES
		 && tess.numIndexes + indexes < SHADER_MAX_INDEXES ) {
		return;
	}

	RB_EndSurface();

	if ( verts >= SHADER_MAX_VERTEXES ) {
		ri.Error( ERR_DROP, "RB_CheckOverflow: verts > MAX (%d > %d)", verts, SHADER_MAX_VERTEXES );
	}
	if ( indexes >= SHADER_MAX_INDEXES ) {
		ri.Error( ERR_DROP, "RB_CheckOverflow: indices > MAX (%d > %d)", indexes, SHADER_MAX_INDEXES );
	}

	RB_BeginSurface( tess.shader, tess.fogNum );
}

/*
==============
RB_AddQuadStampFadingCornersExt

  Creates a sprite with the center at colors[3] alpha, and the corners all 0 alpha
==============
*/
void RB_AddQuadStampFadingCornersExt( idVec3 origin, idVec3 left, idVec3 up, byte *color, float s1, float t1, float s2, float t2 ) {
	idVec3 normal;
	int ndx;
	byte lColor[4];

	RB_CHECKOVERFLOW( 5, 12 );

	ndx = tess.numVertexes;

	// triangle indexes for a simple quad
	tess.indexes[ tess.numIndexes + 0 ] = ndx + 0;
	tess.indexes[ tess.numIndexes + 1 ] = ndx + 1;
	tess.indexes[ tess.numIndexes + 2 ] = ndx + 4;

	tess.indexes[ tess.numIndexes + 3 ] = ndx + 1;
	tess.indexes[ tess.numIndexes + 4 ] = ndx + 2;
	tess.indexes[ tess.numIndexes + 5 ] = ndx + 4;

	tess.indexes[ tess.numIndexes + 6 ] = ndx + 2;
	tess.indexes[ tess.numIndexes + 7 ] = ndx + 3;
	tess.indexes[ tess.numIndexes + 8 ] = ndx + 4;

	tess.indexes[ tess.numIndexes + 9 ] = ndx + 3;
	tess.indexes[ tess.numIndexes + 10] = ndx + 0;
	tess.indexes[ tess.numIndexes + 11] = ndx + 4;

	tess.xyz[ndx][0] = origin[0] + left[0] + up[0];
	tess.xyz[ndx][1] = origin[1] + left[1] + up[1];
	tess.xyz[ndx][2] = origin[2] + left[2] + up[2];

	tess.xyz[ndx + 1][0] = origin[0] - left[0] + up[0];
	tess.xyz[ndx + 1][1] = origin[1] - left[1] + up[1];
	tess.xyz[ndx + 1][2] = origin[2] - left[2] + up[2];

	tess.xyz[ndx + 2][0] = origin[0] - left[0] - up[0];
	tess.xyz[ndx + 2][1] = origin[1] - left[1] - up[1];
	tess.xyz[ndx + 2][2] = origin[2] - left[2] - up[2];

	tess.xyz[ndx + 3][0] = origin[0] + left[0] - up[0];
	tess.xyz[ndx + 3][1] = origin[1] + left[1] - up[1];
	tess.xyz[ndx + 3][2] = origin[2] + left[2] - up[2];

	tess.xyz[ndx + 4][0] = origin[0];
	tess.xyz[ndx + 4][1] = origin[1];
	tess.xyz[ndx + 4][2] = origin[2];


	// constant normal all the way around
	normal = vec3_origin - backEnd.viewParms.orent.axis[0];

	tess.normal[ndx][0] = tess.normal[ndx + 1][0] = tess.normal[ndx + 2][0] = tess.normal[ndx + 3][0] = tess.normal[ndx + 4][0] = normal[0];
	tess.normal[ndx][1] = tess.normal[ndx + 1][1] = tess.normal[ndx + 2][1] = tess.normal[ndx + 3][1] = tess.normal[ndx + 4][1] = normal[1];
	tess.normal[ndx][2] = tess.normal[ndx + 1][2] = tess.normal[ndx + 2][2] = tess.normal[ndx + 3][2] = tess.normal[ndx + 4][2] = normal[2];

	// standard square texture coordinates
	tess.texCoords[ndx][0][0] = tess.texCoords[ndx][1][0] = s1;
	tess.texCoords[ndx][0][1] = tess.texCoords[ndx][1][1] = t1;

	tess.texCoords[ndx + 1][0][0] = tess.texCoords[ndx + 1][1][0] = s2;
	tess.texCoords[ndx + 1][0][1] = tess.texCoords[ndx + 1][1][1] = t1;

	tess.texCoords[ndx + 2][0][0] = tess.texCoords[ndx + 2][1][0] = s2;
	tess.texCoords[ndx + 2][0][1] = tess.texCoords[ndx + 2][1][1] = t2;

	tess.texCoords[ndx + 3][0][0] = tess.texCoords[ndx + 3][1][0] = s1;
	tess.texCoords[ndx + 3][0][1] = tess.texCoords[ndx + 3][1][1] = t2;

	tess.texCoords[ndx + 4][0][0] = tess.texCoords[ndx + 4][1][0] = ( s1 + s2 ) / 2.0;
	tess.texCoords[ndx + 4][0][1] = tess.texCoords[ndx + 4][1][1] = ( t1 + t2 ) / 2.0;

	// center uses full alpha
	*( unsigned int * ) &tess.vertexColors[ndx + 4] =
		*( unsigned int * )color;

	// fade around edges
	memcpy( lColor, color, sizeof( byte ) * 4 );
	lColor[3] = 0;
	*( unsigned int * ) &tess.vertexColors[ndx] =
		*( unsigned int * ) &tess.vertexColors[ndx + 1] =
			*( unsigned int * ) &tess.vertexColors[ndx + 2] =
				*( unsigned int * ) &tess.vertexColors[ndx + 3] =
					*( unsigned int * )lColor;


	tess.numVertexes += 5;
	tess.numIndexes += 12;
}

/*
==============
RB_AddQuadStampExt
==============
*/
void RB_AddQuadStampExt( idVec3 origin, idVec3 left, idVec3 up, byte *color, float s1, float t1, float s2, float t2 ) {
	idVec3 normal;
	int ndx;

	RB_CHECKOVERFLOW( 4, 6 );

	ndx = tess.numVertexes;

	// triangle indexes for a simple quad
	tess.indexes[ tess.numIndexes ] = ndx;
	tess.indexes[ tess.numIndexes + 1 ] = ndx + 1;
	tess.indexes[ tess.numIndexes + 2 ] = ndx + 3;

	tess.indexes[ tess.numIndexes + 3 ] = ndx + 3;
	tess.indexes[ tess.numIndexes + 4 ] = ndx + 1;
	tess.indexes[ tess.numIndexes + 5 ] = ndx + 2;

	tess.xyz[ndx][0] = origin[0] + left[0] + up[0];
	tess.xyz[ndx][1] = origin[1] + left[1] + up[1];
	tess.xyz[ndx][2] = origin[2] + left[2] + up[2];

	tess.xyz[ndx + 1][0] = origin[0] - left[0] + up[0];
	tess.xyz[ndx + 1][1] = origin[1] - left[1] + up[1];
	tess.xyz[ndx + 1][2] = origin[2] - left[2] + up[2];

	tess.xyz[ndx + 2][0] = origin[0] - left[0] - up[0];
	tess.xyz[ndx + 2][1] = origin[1] - left[1] - up[1];
	tess.xyz[ndx + 2][2] = origin[2] - left[2] - up[2];

	tess.xyz[ndx + 3][0] = origin[0] + left[0] - up[0];
	tess.xyz[ndx + 3][1] = origin[1] + left[1] - up[1];
	tess.xyz[ndx + 3][2] = origin[2] + left[2] - up[2];


	// constant normal all the way around
	normal = vec3_origin - backEnd.viewParms.orent.axis[0];

	tess.normal[ndx][0] = tess.normal[ndx + 1][0] = tess.normal[ndx + 2][0] = tess.normal[ndx + 3][0] = normal[0];
	tess.normal[ndx][1] = tess.normal[ndx + 1][1] = tess.normal[ndx + 2][1] = tess.normal[ndx + 3][1] = normal[1];
	tess.normal[ndx][2] = tess.normal[ndx + 1][2] = tess.normal[ndx + 2][2] = tess.normal[ndx + 3][2] = normal[2];

	// standard square texture coordinates
	tess.texCoords[ndx][0][0] = tess.texCoords[ndx][1][0] = s1;
	tess.texCoords[ndx][0][1] = tess.texCoords[ndx][1][1] = t1;

	tess.texCoords[ndx + 1][0][0] = tess.texCoords[ndx + 1][1][0] = s2;
	tess.texCoords[ndx + 1][0][1] = tess.texCoords[ndx + 1][1][1] = t1;

	tess.texCoords[ndx + 2][0][0] = tess.texCoords[ndx + 2][1][0] = s2;
	tess.texCoords[ndx + 2][0][1] = tess.texCoords[ndx + 2][1][1] = t2;

	tess.texCoords[ndx + 3][0][0] = tess.texCoords[ndx + 3][1][0] = s1;
	tess.texCoords[ndx + 3][0][1] = tess.texCoords[ndx + 3][1][1] = t2;

	// constant color all the way around
	// should this be identity and let the shader specify from entity?
	*( unsigned int * ) &tess.vertexColors[ndx] =
		*( unsigned int * ) &tess.vertexColors[ndx + 1] =
			*( unsigned int * ) &tess.vertexColors[ndx + 2] =
				*( unsigned int * ) &tess.vertexColors[ndx + 3] =
					*( unsigned int * )color;


	tess.numVertexes += 4;
	tess.numIndexes += 6;
}

/*
==============
RB_AddQuadStamp
==============
*/
void RB_AddQuadStamp( idVec3 origin, idVec3 left, idVec3 up, byte *color ) {
	RB_AddQuadStampExt( origin, left, up, color, 0, 0, 1, 1 );
}

/*
==============
RB_SurfaceSplash
==============
*/
static void RB_SurfaceSplash( void ) {
	idVec3 left, up;
	float radius;

	// calculate the xyz locations for the four corners
	radius = backEnd.currentEntity->e.radius;

	left.Set( left, -radius, 0, 0 );
	up.Set( up, 0, radius, 0 );
	if ( backEnd.viewParms.isMirror ) {
		left = vec3_origin - left;
	}

	RB_AddQuadStamp( backEnd.currentEntity->e.origin, left, up, backEnd.currentEntity->e.shaderRGBA );
}

/*
==============
RB_SurfaceSprite
==============
*/
static void RB_SurfaceSprite( void ) {
	idVec3 left, up;
	float radius;

	// calculate the xyz locations for the four corners
	radius = backEnd.currentEntity->e.radius;
	if ( backEnd.currentEntity->e.rotation == 0 ) {
		left = backEnd.viewParms.orent.axis[1] * radius;
		up = backEnd.viewParms.orent.axis[2] * radius;
	} else {
		float s, c;
		float ang;

		ang = M_PI * backEnd.currentEntity->e.rotation / 180;
		s = sin( ang );
		c = cos( ang );

		left = backEnd.viewParms.orent.axis[1] * c * radius;
		left = left + backEnd.viewParms.orent.axis[2] * -s * radius;

		up = backEnd.viewParms.orent.axis[2] * c * radius;
		up = up + backEnd.viewParms.orent.axis[1] * s * radius;
	}
	if ( backEnd.viewParms.isMirror ) {
		left = vec3_origin - left;
	}

	RB_AddQuadStamp( backEnd.currentEntity->e.origin, left, up, backEnd.currentEntity->e.shaderRGBA );
}


/*
=============
RB_SurfacePolychain
=============
*/
void RB_SurfacePolychain( srfPoly_t *p ) {
	int i;
	int numv;

	RB_CHECKOVERFLOW( p->numVerts, 3 * ( p->numVerts - 2 ) );

	// fan triangles into the tess array
	numv = tess.numVertexes;
	for ( i = 0; i < p->numVerts; i++ ) {
		tess.xyz[numv] = p->verts[i].xyz;
		tess.texCoords[numv][0][0] = p->verts[i].st[0];
		tess.texCoords[numv][0][1] = p->verts[i].st[1];
		*(int *)&tess.vertexColors[numv] = *(int *)p->verts[ i ].modulate;

		numv++;
	}

	// generate fan indexes into the tess array
	for ( i = 0; i < p->numVerts - 2; i++ ) {
		tess.indexes[tess.numIndexes + 0] = tess.numVertexes;
		tess.indexes[tess.numIndexes + 1] = tess.numVertexes + i + 1;
		tess.indexes[tess.numIndexes + 2] = tess.numVertexes + i + 2;
		tess.numIndexes += 3;
	}

	tess.numVertexes = numv;
}


/*
=============
RB_SurfaceTriangles
=============
*/
void RB_SurfaceTriangles( srfTriangles_t *srf ) {
	int i;
	drawVert_t  *dv;
	float       *xyz, *normal, *texCoords;
	byte        *color;
	int dlightBits;
	qboolean needsNormal;

	dlightBits = srf->dlightBits[backEnd.smpFrame];
	tess.dlightBits |= dlightBits;

	RB_CHECKOVERFLOW( srf->numVerts, srf->numIndexes );

	for ( i = 0 ; i < srf->numIndexes ; i += 3 ) {
		tess.indexes[ tess.numIndexes + i + 0 ] = tess.numVertexes + srf->indexes[ i + 0 ];
		tess.indexes[ tess.numIndexes + i + 1 ] = tess.numVertexes + srf->indexes[ i + 1 ];
		tess.indexes[ tess.numIndexes + i + 2 ] = tess.numVertexes + srf->indexes[ i + 2 ];
	}
	tess.numIndexes += srf->numIndexes;

	dv = srf->verts;
	xyz = tess.xyz[ tess.numVertexes ];
	normal = tess.normal[ tess.numVertexes ];
	texCoords = tess.texCoords[ tess.numVertexes ][0];
	color = tess.vertexColors[ tess.numVertexes ];
	needsNormal = tess.shader->needsNormal;

	for ( i = 0 ; i < srf->numVerts ; i++, dv++, xyz += 4, normal += 4, texCoords += 4, color += 4 ) {
		xyz[0] = dv->xyz[0];
		xyz[1] = dv->xyz[1];
		xyz[2] = dv->xyz[2];

		if ( needsNormal ) {
			normal[0] = dv->normal[0];
			normal[1] = dv->normal[1];
			normal[2] = dv->normal[2];
		}

		texCoords[0] = dv->st[0];
		texCoords[1] = dv->st[1];

		texCoords[2] = dv->lightmap[0];
		texCoords[3] = dv->lightmap[1];

		*(int *)color = *(int *)dv->color;
	}

	for ( i = 0 ; i < srf->numVerts ; i++ ) {
		tess.vertexDlightBits[ tess.numVertexes + i] = dlightBits;
	}

	tess.numVertexes += srf->numVerts;
}



/*
==============
RB_SurfaceBeam
==============
*/
void RB_SurfaceBeam( void ) {
#define NUM_BEAM_SEGS 6
	refEntity_t *e;
	int i;
	idVec3 perpvec;
	idVec3 direction, normalized_direction;
	idVec3 start_points[NUM_BEAM_SEGS], end_points[NUM_BEAM_SEGS];
	idVec3 oldorigin, origin;

	e = &backEnd.currentEntity->e;

	oldorigin[0] = e->oldorigin[0];
	oldorigin[1] = e->oldorigin[1];
	oldorigin[2] = e->oldorigin[2];

	origin[0] = e->origin[0];
	origin[1] = e->origin[1];
	origin[2] = e->origin[2];

	normalized_direction[0] = direction[0] = oldorigin[0] - origin[0];
	normalized_direction[1] = direction[1] = oldorigin[1] - origin[1];
	normalized_direction[2] = direction[2] = oldorigin[2] - origin[2];

	if ( normalized_direction.Normalize() == 0 ) {
		return;
	}

	perpvec.PerpendicularVector( normalized_direction );

	perpvec = perpvec * 4;

	for ( i = 0; i < NUM_BEAM_SEGS ; i++ )
	{
		start_points[i].RotatePointAroundVector( normalized_direction, perpvec, ( 360.0 / NUM_BEAM_SEGS ) * i );
//		start_points[i] = start_points[i] + origin;
		end_points[i] = start_points[i] + direction;
	}

	GL_Bind( tr.whiteImage );

	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );

	qglColor3f( 1, 0, 0 );

	qglBegin( GL_TRIANGLE_STRIP );
	for ( i = 0; i <= NUM_BEAM_SEGS; i++ ) {
		qglVertex3fv( start_points[ i % NUM_BEAM_SEGS] );
		qglVertex3fv( end_points[ i % NUM_BEAM_SEGS] );
	}
	qglEnd();
}

//================================================================================

static void DoRailCore( const idVec3 start, const idVec3 end, const idVec3 up, float len, float spanWidth ) {
	float spanWidth2;
	int vbase;
	float t = len / 256.0f;

	vbase = tess.numVertexes;

	spanWidth2 = -spanWidth;

	// FIXME: use quad stamp?
	tess.xyz[tess.numVertexes] = start + up * spanWidth;
	tess.texCoords[tess.numVertexes][0][0] = 0;
	tess.texCoords[tess.numVertexes][0][1] = 0;
	tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0] * 0.25;
	tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1] * 0.25;
	tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2] * 0.25;
	tess.numVertexes++;

	tess.xyz[tess.numVertexes] = start + up * spanWidth2;
	tess.texCoords[tess.numVertexes][0][0] = 0;
	tess.texCoords[tess.numVertexes][0][1] = 1;
	tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0];
	tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1];
	tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2];
	tess.numVertexes++;

	tess.xyz[tess.numVertexes] = end + up * spanWidth;

	tess.texCoords[tess.numVertexes][0][0] = t;
	tess.texCoords[tess.numVertexes][0][1] = 0;
	tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0];
	tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1];
	tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2];
	tess.numVertexes++;

	tess.xyz[tess.numVertexes] = end + up * spanWidth2;
	tess.texCoords[tess.numVertexes][0][0] = t;
	tess.texCoords[tess.numVertexes][0][1] = 1;
	tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0];
	tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1];
	tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2];
	tess.numVertexes++;

	tess.indexes[tess.numIndexes++] = vbase;
	tess.indexes[tess.numIndexes++] = vbase + 1;
	tess.indexes[tess.numIndexes++] = vbase + 2;

	tess.indexes[tess.numIndexes++] = vbase + 2;
	tess.indexes[tess.numIndexes++] = vbase + 1;
	tess.indexes[tess.numIndexes++] = vbase + 3;
}

static void DoRailDiscs( int numSegs, const idVec3 start, const idVec3 dir, const idVec3 right, const idVec3 up ) {
	int i;
	idVec3 pos[4];
	idVec3 v;
	int spanWidth = r_railWidth->integer;
	float c, s;
	float scale;

	if ( numSegs > 1 ) {
		numSegs--;
	}
	if ( !numSegs ) {
		return;
	}

	scale = 0.25;

	for ( i = 0; i < 4; i++ )
	{
		c = cos( DEG2RAD( 45 + i * 90 ) );
		s = sin( DEG2RAD( 45 + i * 90 ) );
		v[0] = ( right[0] * c + up[0] * s ) * scale * spanWidth;
		v[1] = ( right[1] * c + up[1] * s ) * scale * spanWidth;
		v[2] = ( right[2] * c + up[2] * s ) * scale * spanWidth;
		pos[i] = start + v;

		if ( numSegs > 1 ) {
			// offset by 1 segment if we're doing a long distance shot
			pos[i] = pos[i] + dir;
		}
	}

	for ( i = 0; i < numSegs; i++ )
	{
		int j;

		RB_CHECKOVERFLOW( 4, 6 );

		for ( j = 0; j < 4; j++ )
		{
			tess.xyz[tess.numVertexes] = pos[j];
			tess.texCoords[tess.numVertexes][0][0] = ( j < 2 );
			tess.texCoords[tess.numVertexes][0][1] = ( j && j != 3 );
			tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0];
			tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1];
			tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2];
			tess.numVertexes++;

			pos[j] = pos[j] + dir;
		}

		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 0;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 1;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 3;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 3;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 1;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 2;
	}
}

/*
** RB_SurfaceRailRinges
*/
void RB_SurfaceRailRings( void ) {
	refEntity_t *e;
	int numSegs;
	int len;
	idVec3 vec;
	idVec3 right, up;
	idVec3 start, end;

	e = &backEnd.currentEntity->e;

	start = e->oldorigin;
	end = e->origin;

	// compute variables
	vec = end - start;
	len = vec.Normalize();
	MakeNormalVectors( vec, right, up );
	numSegs = ( len ) / r_railSegmentLength->value;
	if ( numSegs <= 0 ) {
		numSegs = 1;
	}

	vec = vec * r_railSegmentLength->value;

	DoRailDiscs( numSegs, start, vec, right, up );
}

/*
** RB_SurfaceRailCore
*/
void RB_SurfaceRailCore( void ) {
	refEntity_t *e;
	int len;
	idVec3 right;
	idVec3 vec;
	idVec3 start, end;
	idVec3 v1, v2;

	e = &backEnd.currentEntity->e;

	start = e->oldorigin;
	end = e->origin;

	vec = end - start;
	len = vec.Normalize();

	// compute side vector
	v1 = start - backEnd.viewParms.orent.origin;
	v1.Normalize();
	v2 = end - backEnd.viewParms.orent.origin;
	v2.Normalize();
	right.Cross( v1, v2 );
	right.Normalize();

	DoRailCore( start, end, right, len, r_railCoreWidth->integer );
}

/*
** RB_SurfaceLightningBolt
*/
void RB_SurfaceLightningBolt( void ) {
	refEntity_t *e;
	int len;
	idVec3 right;
	idVec3 vec;
	idVec3 start, end;
	idVec3 v1, v2;
	int i;

	e = &backEnd.currentEntity->e;

	end = e->oldorigin;
	start = e->origin;

	// compute variables
	vec = end - start;
	len = vec.Normalize();

	// compute side vector
	v1 = start - backEnd.viewParms.orent.origin;
	v1.Normalize();
	v2 = end - backEnd.viewParms.orent.origin;
	v2.Normalize();
	right.Cross( v1, v2 );
	right.Normalize();

	for ( i = 0 ; i < 4 ; i++ ) {
		idVec3 temp;

		DoRailCore( start, end, right, len, 8 );
		temp.RotatePointAroundVector( vec, right, 45 );
		right = temp;
	}
}

/*
** VectorArrayNormalize
*
* The inputs to this routing seem to always be close to length = 1.0 (about 0.6 to 2.0)
* This means that we don't have to worry about zero length or enormously long vectors.
*/
static void VectorArrayNormalize( idVec4 *normals, unsigned int count ) {
	// the inputs are always close to length 1.0 (about 0.6 to 2.0)
	// so we don't have to worry about zero length or enormously long vectors
	for ( unsigned int i = 0; i < count; i++ ) {
		idVec3 n;
		n[0] = normals[i][0];
		n[1] = normals[i][1];
		n[2] = normals[i][2];
		n.Normalize();
		normals[i][0] = n[0];
		normals[i][1] = n[1];
		normals[i][2] = n[2];
	}
}



/*
** LerpMeshVertexes
*/
static void LerpMeshVertexes( md3Surface_t *surf, float backlerp ) {
	short   *oldXyz, *newXyz, *oldNormals, *newNormals;
	float   *outXyz, *outNormal;
	float oldXyzScale, newXyzScale;
	float oldNormalScale, newNormalScale;
	int vertNum;
	unsigned lat, lng;
	int numVerts;

	outXyz = tess.xyz[tess.numVertexes];
	outNormal = tess.normal[tess.numVertexes];

	newXyz = ( short * )( (byte *)surf + surf->ofsXyzNormals )
			 + ( backEnd.currentEntity->e.frame * surf->numVerts * 4 );
	newNormals = newXyz + 3;

	newXyzScale = MD3_XYZ_SCALE * ( 1.0 - backlerp );
	newNormalScale = 1.0 - backlerp;

	numVerts = surf->numVerts;

	if ( backlerp == 0 ) {
		//
		// just copy the vertexes
		//
		for ( vertNum = 0 ; vertNum < numVerts ; vertNum++,
			  newXyz += 4, newNormals += 4,
			  outXyz += 4, outNormal += 4 )
		{

			outXyz[0] = newXyz[0] * newXyzScale;
			outXyz[1] = newXyz[1] * newXyzScale;
			outXyz[2] = newXyz[2] * newXyzScale;

			lat = ( newNormals[0] >> 8 ) & 0xff;
			lng = ( newNormals[0] & 0xff );
			lat *= ( FUNCTABLE_SIZE / 256 );
			lng *= ( FUNCTABLE_SIZE / 256 );

			// decode X as cos( lat ) * sin( long )
			// decode Y as sin( lat ) * sin( long )
			// decode Z as cos( long )

			outNormal[0] = tr.sinTable[( lat + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK] * tr.sinTable[lng];
			outNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
			outNormal[2] = tr.sinTable[( lng + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK];
		}
	} else {
		//
		// interpolate and copy the vertex and normal
		//
		oldXyz = ( short * )( (byte *)surf + surf->ofsXyzNormals )
				 + ( backEnd.currentEntity->e.oldframe * surf->numVerts * 4 );
		oldNormals = oldXyz + 3;

		oldXyzScale = MD3_XYZ_SCALE * backlerp;
		oldNormalScale = backlerp;

		for ( vertNum = 0 ; vertNum < numVerts ; vertNum++,
			  oldXyz += 4, newXyz += 4, oldNormals += 4, newNormals += 4,
			  outXyz += 4, outNormal += 4 )
		{
			idVec3 uncompressedOldNormal, uncompressedNewNormal;

			// interpolate the xyz
			outXyz[0] = oldXyz[0] * oldXyzScale + newXyz[0] * newXyzScale;
			outXyz[1] = oldXyz[1] * oldXyzScale + newXyz[1] * newXyzScale;
			outXyz[2] = oldXyz[2] * oldXyzScale + newXyz[2] * newXyzScale;

			// FIXME: interpolate lat/long instead?
			lat = ( newNormals[0] >> 8 ) & 0xff;
			lng = ( newNormals[0] & 0xff );
			lat *= 4;
			lng *= 4;
			uncompressedNewNormal[0] = tr.sinTable[( lat + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK] * tr.sinTable[lng];
			uncompressedNewNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
			uncompressedNewNormal[2] = tr.sinTable[( lng + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK];

			lat = ( oldNormals[0] >> 8 ) & 0xff;
			lng = ( oldNormals[0] & 0xff );
			lat *= 4;
			lng *= 4;

			uncompressedOldNormal[0] = tr.sinTable[( lat + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK] * tr.sinTable[lng];
			uncompressedOldNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
			uncompressedOldNormal[2] = tr.sinTable[( lng + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK];

			outNormal[0] = uncompressedOldNormal[0] * oldNormalScale + uncompressedNewNormal[0] * newNormalScale;
			outNormal[1] = uncompressedOldNormal[1] * oldNormalScale + uncompressedNewNormal[1] * newNormalScale;
			outNormal[2] = uncompressedOldNormal[2] * oldNormalScale + uncompressedNewNormal[2] * newNormalScale;

//			outNormal.Normalize();
		}
		VectorArrayNormalize( (idVec4 *)tess.normal[tess.numVertexes], numVerts );
	}
}

/*
=============
RB_SurfaceMesh
=============
*/
void RB_SurfaceMesh( md3Surface_t *surface ) {
	int j;
	float backlerp;
	int             *triangles;
	float           *texCoords;
	int indexes;
	int Bob, Doug;
	int numVerts;

	// RF, check for REFLAG_HANDONLY
	if ( backEnd.currentEntity->e.reFlags & REFLAG_ONLYHAND ) {
		if ( !strstr( surface->name, "hand" ) ) {
			return;
		}
	}

	if (  backEnd.currentEntity->e.oldframe == backEnd.currentEntity->e.frame ) {
		backlerp = 0;
	} else  {
		backlerp = backEnd.currentEntity->e.backlerp;
	}

	RB_CHECKOVERFLOW( surface->numVerts, surface->numTriangles * 3 );

	LerpMeshVertexes( surface, backlerp );

	triangles = ( int * )( (byte *)surface + surface->ofsTriangles );
	indexes = surface->numTriangles * 3;
	Bob = tess.numIndexes;
	Doug = tess.numVertexes;
	for ( j = 0 ; j < indexes ; j++ ) {
		tess.indexes[Bob + j] = Doug + triangles[j];
	}
	tess.numIndexes += indexes;

	texCoords = ( float * )( (byte *)surface + surface->ofsSt );

	numVerts = surface->numVerts;
	for ( j = 0; j < numVerts; j++ ) {
		tess.texCoords[Doug + j][0][0] = texCoords[j * 2 + 0];
		tess.texCoords[Doug + j][0][1] = texCoords[j * 2 + 1];
		// FIXME: fill in lightmapST for completeness?
	}

	tess.numVertexes += surface->numVerts;

}

/*
** R_LatLongToNormal
*/
void R_LatLongToNormal( idVec3 outNormal, short latLong ) {
	unsigned lat, lng;

	lat = ( latLong >> 8 ) & 0xff;
	lng = ( latLong & 0xff );
	lat *= ( FUNCTABLE_SIZE / 256 );
	lng *= ( FUNCTABLE_SIZE / 256 );

	// decode X as cos( lat ) * sin( long )
	// decode Y as sin( lat ) * sin( long )
	// decode Z as cos( long )

	outNormal[0] = tr.sinTable[( lat + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK] * tr.sinTable[lng];
	outNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
	outNormal[2] = tr.sinTable[( lng + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK];
}

// Ridah
/*
** LerpCMeshVertexes
*/
static void LerpCMeshVertexes( mdcSurface_t *surf, float backlerp ) {
	short   *oldXyz, *newXyz, *oldNormals, *newNormals;
	float   *outXyz, *outNormal;
	float oldXyzScale, newXyzScale;
	float oldNormalScale, newNormalScale;
	int vertNum;
	unsigned lat, lng;
	int numVerts;

	int oldBase, newBase;
	short   *oldComp = NULL, *newComp = NULL; // TTimo: init
	mdcXyzCompressed_t *oldXyzComp = NULL, *newXyzComp = NULL; // TTimo: init
	idVec3 oldOfsVec, newOfsVec;

	qboolean hasComp;

	outXyz = tess.xyz[tess.numVertexes];
	outNormal = tess.normal[tess.numVertexes];

	newBase = (int)*( ( short * )( (byte *)surf + surf->ofsFrameBaseFrames ) + backEnd.currentEntity->e.frame );
	newXyz = ( short * )( (byte *)surf + surf->ofsXyzNormals )
			 + ( newBase * surf->numVerts * 4 );
	newNormals = newXyz + 3;

	hasComp = ( surf->numCompFrames > 0 );
	if ( hasComp ) {
		newComp = ( ( short * )( (byte *)surf + surf->ofsFrameCompFrames ) + backEnd.currentEntity->e.frame );
		if ( *newComp >= 0 ) {
			newXyzComp = ( mdcXyzCompressed_t * )( (byte *)surf + surf->ofsXyzCompressed )
						 + ( *newComp * surf->numVerts );
		}
	}

	newXyzScale = MD3_XYZ_SCALE * ( 1.0 - backlerp );
	newNormalScale = 1.0 - backlerp;

	numVerts = surf->numVerts;

	if ( backlerp == 0 ) {
		//
		// just copy the vertexes
		//
		for ( vertNum = 0 ; vertNum < numVerts ; vertNum++,
			  newXyz += 4, newNormals += 4,
			  outXyz += 4, outNormal += 4 )
		{

			outXyz[0] = newXyz[0] * newXyzScale;
			outXyz[1] = newXyz[1] * newXyzScale;
			outXyz[2] = newXyz[2] * newXyzScale;

			// add the compressed ofsVec
			if ( hasComp && *newComp >= 0 ) {
				R_MDC_DecodeXyzCompressed( newXyzComp->ofsVec, newOfsVec, outNormal );
				newXyzComp++;
				outXyz = outXyz + newOfsVec;
			} else {
				lat = ( newNormals[0] >> 8 ) & 0xff;
				lng = ( newNormals[0] & 0xff );
				lat *= 4;
				lng *= 4;

				outNormal[0] = tr.sinTable[( lat + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK] * tr.sinTable[lng];
				outNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
				outNormal[2] = tr.sinTable[( lng + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK];
			}
		}
	} else {
		//
		// interpolate and copy the vertex and normal
		//
		oldBase = (int)*( ( short * )( (byte *)surf + surf->ofsFrameBaseFrames ) + backEnd.currentEntity->e.oldframe );
		oldXyz = ( short * )( (byte *)surf + surf->ofsXyzNormals )
				 + ( oldBase * surf->numVerts * 4 );
		oldNormals = oldXyz + 3;

		if ( hasComp ) {
			oldComp = ( ( short * )( (byte *)surf + surf->ofsFrameCompFrames ) + backEnd.currentEntity->e.oldframe );
			if ( *oldComp >= 0 ) {
				oldXyzComp = ( mdcXyzCompressed_t * )( (byte *)surf + surf->ofsXyzCompressed )
							 + ( *oldComp * surf->numVerts );
			}
		}

		oldXyzScale = MD3_XYZ_SCALE * backlerp;
		oldNormalScale = backlerp;

		for ( vertNum = 0 ; vertNum < numVerts ; vertNum++,
			  oldXyz += 4, newXyz += 4, oldNormals += 4, newNormals += 4,
			  outXyz += 4, outNormal += 4 )
		{
			idVec3 uncompressedOldNormal, uncompressedNewNormal;

			// interpolate the xyz
			outXyz[0] = oldXyz[0] * oldXyzScale + newXyz[0] * newXyzScale;
			outXyz[1] = oldXyz[1] * oldXyzScale + newXyz[1] * newXyzScale;
			outXyz[2] = oldXyz[2] * oldXyzScale + newXyz[2] * newXyzScale;

			// add the compressed ofsVec
			if ( hasComp && *newComp >= 0 ) {
				R_MDC_DecodeXyzCompressed( newXyzComp->ofsVec, newOfsVec, uncompressedNewNormal );
				newXyzComp++;
				outXyz = outXyz + newOfsVec * 1.0 - backlerp;
			} else {
				lat = ( newNormals[0] >> 8 ) & 0xff;
				lng = ( newNormals[0] & 0xff );
				lat *= 4;
				lng *= 4;

				uncompressedNewNormal[0] = tr.sinTable[( lat + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK] * tr.sinTable[lng];
				uncompressedNewNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
				uncompressedNewNormal[2] = tr.sinTable[( lng + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK];
			}

			if ( hasComp && *oldComp >= 0 ) {
				R_MDC_DecodeXyzCompressed( oldXyzComp->ofsVec, oldOfsVec, uncompressedOldNormal );
				oldXyzComp++;
				outXyz = outXyz + oldOfsVec * backlerp;
			} else {
				lat = ( oldNormals[0] >> 8 ) & 0xff;
				lng = ( oldNormals[0] & 0xff );
				lat *= 4;
				lng *= 4;

				uncompressedOldNormal[0] = tr.sinTable[( lat + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK] * tr.sinTable[lng];
				uncompressedOldNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
				uncompressedOldNormal[2] = tr.sinTable[( lng + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK];
			}

			outNormal[0] = uncompressedOldNormal[0] * oldNormalScale + uncompressedNewNormal[0] * newNormalScale;
			outNormal[1] = uncompressedOldNormal[1] * oldNormalScale + uncompressedNewNormal[1] * newNormalScale;
			outNormal[2] = uncompressedOldNormal[2] * oldNormalScale + uncompressedNewNormal[2] * newNormalScale;

			outNormal.Normalize();
		}
	}
}

/*
=============
RB_SurfaceCMesh
=============
*/
void RB_SurfaceCMesh( mdcSurface_t *surface ) {
	int j;
	float backlerp;
	int             *triangles;
	float           *texCoords;
	int indexes;
	int Bob, Doug;
	int numVerts;

	// RF, check for REFLAG_HANDONLY
	if ( backEnd.currentEntity->e.reFlags & REFLAG_ONLYHAND ) {
		if ( !strstr( surface->name, "hand" ) ) {
			return;
		}
	}

	if (  backEnd.currentEntity->e.oldframe == backEnd.currentEntity->e.frame ) {
		backlerp = 0;
	} else  {
		backlerp = backEnd.currentEntity->e.backlerp;
	}

	RB_CHECKOVERFLOW( surface->numVerts, surface->numTriangles * 3 );

	LerpCMeshVertexes( surface, backlerp );

	triangles = ( int * )( (byte *)surface + surface->ofsTriangles );
	indexes = surface->numTriangles * 3;
	Bob = tess.numIndexes;
	Doug = tess.numVertexes;
	for ( j = 0 ; j < indexes ; j++ ) {
		tess.indexes[Bob + j] = Doug + triangles[j];
	}
	tess.numIndexes += indexes;

	texCoords = ( float * )( (byte *)surface + surface->ofsSt );

	numVerts = surface->numVerts;
	for ( j = 0; j < numVerts; j++ ) {
		tess.texCoords[Doug + j][0][0] = texCoords[j * 2 + 0];
		tess.texCoords[Doug + j][0][1] = texCoords[j * 2 + 1];
		// FIXME: fill in lightmapST for completeness?
	}

	tess.numVertexes += surface->numVerts;
}
// done.

/*
==============
RB_SurfaceFace
==============
*/
void RB_SurfaceFace( srfSurfaceFace_t *surf ) {
	int i;
	unsigned    *indices, *tessIndexes;
	float       *v;
	float       *normal;
	int ndx;
	int Bob;
	int numPoints;
	int dlightBits;

	RB_CHECKOVERFLOW( surf->numPoints, surf->numIndices );

	dlightBits = surf->dlightBits[backEnd.smpFrame];
	tess.dlightBits |= dlightBits;

	indices = ( unsigned * )( ( ( char  * ) surf ) + surf->ofsIndices );

	Bob = tess.numVertexes;
	tessIndexes = tess.indexes + tess.numIndexes;
	for ( i = surf->numIndices - 1 ; i >= 0  ; i-- ) {
		tessIndexes[i] = indices[i] + Bob;
	}

	tess.numIndexes += surf->numIndices;

	v = surf->points[0];

	ndx = tess.numVertexes;

	numPoints = surf->numPoints;

	if ( tess.shader->needsNormal ) {
		normal = surf->plane.normal;
		for ( i = 0, ndx = tess.numVertexes; i < numPoints; i++, ndx++ ) {
			tess.normal[ndx] = normal;
		}
	}

	for ( i = 0, v = surf->points[0], ndx = tess.numVertexes; i < numPoints; i++, v += VERTEXSIZE, ndx++ ) {
		tess.xyz[ndx] = v;
		tess.texCoords[ndx][0][0] = v[3];
		tess.texCoords[ndx][0][1] = v[4];
		tess.texCoords[ndx][1][0] = v[5];
		tess.texCoords[ndx][1][1] = v[6];
		*( unsigned int * ) &tess.vertexColors[ndx] = *( unsigned int * ) &v[7];
		tess.vertexDlightBits[ndx] = dlightBits;
	}


	tess.numVertexes += surf->numPoints;
}


static float    LodErrorForVolume( idVec3 local, float radius ) {
	idVec3 world;
	float d;

	// never let it go negative
	if ( r_lodCurveError->value < 0 ) {
		return 0;
	}

	world[0] = local[0] * backEnd.orent.axis[0][0] + local[1] * backEnd.orent.axis[1][0] +
			   local[2] * backEnd.orent.axis[2][0] + backEnd.orent.origin[0];
	world[1] = local[0] * backEnd.orent.axis[0][1] + local[1] * backEnd.orent.axis[1][1] +
			   local[2] * backEnd.orent.axis[2][1] + backEnd.orent.origin[1];
	world[2] = local[0] * backEnd.orent.axis[0][2] + local[1] * backEnd.orent.axis[1][2] +
			   local[2] * backEnd.orent.axis[2][2] + backEnd.orent.origin[2];

	world = world - backEnd.viewParms.orent.origin;
	d = ( world ) * ( backEnd.viewParms.orent.axis[0] );

	if ( d < 0 ) {
		d = -d;
	}
	d -= radius;
	if ( d < 1 ) {
		d = 1;
	}

	return r_lodCurveError->value / d;
}

/*
=============
RB_SurfaceGrid

Just copy the grid of points and triangulate
=============
*/
void RB_SurfaceGrid( srfGridMesh_t *cv ) {
	int i, j;
	float   *xyz;
	float   *texCoords;
	float   *normal;
	unsigned char *color;
	drawVert_t  *dv;
	int rows, irows, vrows;
	int used;
	int widthTable[MAX_GRID_SIZE];
	int heightTable[MAX_GRID_SIZE];
	float lodError;
	int lodWidth, lodHeight;
	int numVertexes;
	int dlightBits;
	int     *vDlightBits;
	qboolean needsNormal;

	dlightBits = cv->dlightBits[backEnd.smpFrame];
	tess.dlightBits |= dlightBits;

	// determine the allowable discrepance
	lodError = LodErrorForVolume( cv->lodOrigin, cv->lodRadius );

	// determine which rows and columns of the subdivision
	// we are actually going to use
	widthTable[0] = 0;
	lodWidth = 1;
	for ( i = 1 ; i < cv->width - 1 ; i++ ) {
		if ( cv->widthLodError[i] <= lodError ) {
			widthTable[lodWidth] = i;
			lodWidth++;
		}
	}
	widthTable[lodWidth] = cv->width - 1;
	lodWidth++;

	heightTable[0] = 0;
	lodHeight = 1;
	for ( i = 1 ; i < cv->height - 1 ; i++ ) {
		if ( cv->heightLodError[i] <= lodError ) {
			heightTable[lodHeight] = i;
			lodHeight++;
		}
	}
	heightTable[lodHeight] = cv->height - 1;
	lodHeight++;


	// very large grids may have more points or indexes than can be fit
	// in the tess structure, so we may have to issue it in multiple passes

	used = 0;
	rows = 0;
	while ( used < lodHeight - 1 ) {
		// see how many rows of both verts and indexes we can add without overflowing
		do {
			vrows = ( SHADER_MAX_VERTEXES - tess.numVertexes ) / lodWidth;
			irows = ( SHADER_MAX_INDEXES - tess.numIndexes ) / ( lodWidth * 6 );

			// if we don't have enough space for at least one strip, flush the buffer
			if ( vrows < 2 || irows < 1 ) {
				RB_EndSurface();
				RB_BeginSurface( tess.shader, tess.fogNum );
			} else {
				break;
			}
		} while ( 1 );

		rows = irows;
		if ( vrows < irows + 1 ) {
			rows = vrows - 1;
		}
		if ( used + rows > lodHeight ) {
			rows = lodHeight - used;
		}

		numVertexes = tess.numVertexes;

		xyz = tess.xyz[numVertexes];
		normal = tess.normal[numVertexes];
		texCoords = tess.texCoords[numVertexes][0];
		color = ( unsigned char * ) &tess.vertexColors[numVertexes];
		vDlightBits = &tess.vertexDlightBits[numVertexes];
		needsNormal = tess.shader->needsNormal;

		for ( i = 0 ; i < rows ; i++ ) {
			for ( j = 0 ; j < lodWidth ; j++ ) {
				dv = cv->verts + heightTable[ used + i ] * cv->width
					 + widthTable[ j ];

				xyz[0] = dv->xyz[0];
				xyz[1] = dv->xyz[1];
				xyz[2] = dv->xyz[2];
				texCoords[0] = dv->st[0];
				texCoords[1] = dv->st[1];
				texCoords[2] = dv->lightmap[0];
				texCoords[3] = dv->lightmap[1];
				if ( needsNormal ) {
					normal[0] = dv->normal[0];
					normal[1] = dv->normal[1];
					normal[2] = dv->normal[2];
				}
				*( unsigned int * ) color = *( unsigned int * ) dv->color;
				*vDlightBits++ = dlightBits;
				xyz += 4;
				normal += 4;
				texCoords += 4;
				color += 4;
			}
		}


		// add the indexes
		{
			int numIndexes;
			int w, h;

			h = rows - 1;
			w = lodWidth - 1;
			numIndexes = tess.numIndexes;
			for ( i = 0 ; i < h ; i++ ) {
				for ( j = 0 ; j < w ; j++ ) {
					int v1, v2, v3, v4;

					// vertex order to be reckognized as tristrips
					v1 = numVertexes + i * lodWidth + j + 1;
					v2 = v1 - 1;
					v3 = v2 + lodWidth;
					v4 = v3 + 1;

					tess.indexes[numIndexes] = v2;
					tess.indexes[numIndexes + 1] = v3;
					tess.indexes[numIndexes + 2] = v1;

					tess.indexes[numIndexes + 3] = v1;
					tess.indexes[numIndexes + 4] = v3;
					tess.indexes[numIndexes + 5] = v4;
					numIndexes += 6;
				}
			}

			tess.numIndexes = numIndexes;
		}

		tess.numVertexes += rows * lodWidth;

		used += rows - 1;
	}
}


/*
===========================================================================

NULL MODEL

===========================================================================
*/

/*
===================
RB_SurfaceAxis

Draws x/y/z lines from the origin for orientation debugging
===================
*/
void RB_SurfaceAxis( void ) {
	GL_Bind( tr.whiteImage );
	qglLineWidth( 3 );
	qglBegin( GL_LINES );
	qglColor3f( 1,0,0 );
	qglVertex3f( 0,0,0 );
	qglVertex3f( 16,0,0 );
	qglColor3f( 0,1,0 );
	qglVertex3f( 0,0,0 );
	qglVertex3f( 0,16,0 );
	qglColor3f( 0,0,1 );
	qglVertex3f( 0,0,0 );
	qglVertex3f( 0,0,16 );
	qglEnd();
	qglLineWidth( 1 );
}

//===========================================================================

/*
====================
RB_SurfaceEntity

Entities that have a single procedurally generated surface
====================
*/
void RB_SurfaceEntity( surfaceType_t *surfType ) {
	switch ( backEnd.currentEntity->e.reType ) {
	case RT_SPLASH:
		RB_SurfaceSplash();
		break;
	case RT_SPRITE:
		RB_SurfaceSprite();
		break;
	case RT_BEAM:
		RB_SurfaceBeam();
		break;
	case RT_RAIL_CORE:
		RB_SurfaceRailCore();
		break;
	case RT_RAIL_RINGS:
		RB_SurfaceRailRings();
		break;
	case RT_LIGHTNING:
		RB_SurfaceLightningBolt();
		break;
	default:
		RB_SurfaceAxis();
		break;
	}
	return;
}

void RB_SurfaceBad( surfaceType_t *surfType ) {
	ri.Printf( PRINT_ALL, "Bad surface tesselated.\n" );
}

#if 0

void RB_SurfaceFlare( srfFlare_t *surf ) {
	idVec3 left, up;
	float radius;
	byte color[4];
	idVec3 dir;
	idVec3 origin;
	float d;

	// calculate the xyz locations for the four corners
	radius = 30;
	left = backEnd.viewParms.orent.axis[1] * radius;
	up = backEnd.viewParms.orent.axis[2] * radius;
	if ( backEnd.viewParms.isMirror ) {
		left = vec3_origin - left;
	}

	color[0] = color[1] = color[2] = color[3] = 255;

	origin = surf->origin + surf->normal * 3;
	dir = origin - backEnd.viewParms.orent.origin;
	dir.Normalize();
	origin = origin + dir * r_ignore->value;

	d = -( dir ) * ( surf->normal );
	if ( d < 0 ) {
		return;
	}
#if 0
	color[0] *= d;
	color[1] *= d;
	color[2] *= d;
#endif

	RB_AddQuadStamp( origin, left, up, color );
}

#else

void RB_SurfaceFlare( srfFlare_t *surf ) {
#if 0
	idVec3 left, up;
	byte color[4];

	color[0] = surf->color[0] * 255;
	color[1] = surf->color[1] * 255;
	color[2] = surf->color[2] * 255;
	color[3] = 255;

	left.Zero();
	up.Zero();

	left[0] = r_ignore->value;

	up[1] = r_ignore->value;

	RB_AddQuadStampExt( surf->origin, left, up, color, 0, 0, 1, 1 );
#endif
}

#endif



void RB_SurfaceDisplayList( srfDisplayList_t *surf ) {
	// all apropriate state must be set in RB_BeginSurface
	// this isn't implemented yet...
	qglCallList( surf->listNum );
}

void RB_SurfaceSkip( void *surf ) {
}


void( *rb_surfaceTable[SF_NUM_SURFACE_TYPES] ) ( void * ) = {
	( void( * ) ( void* ) )RB_SurfaceBad,          // SF_BAD,
	( void( * ) ( void* ) )RB_SurfaceSkip,         // SF_SKIP,
	( void( * ) ( void* ) )RB_SurfaceFace,         // SF_FACE,
	( void( * ) ( void* ) )RB_SurfaceGrid,         // SF_GRID,
	( void( * ) ( void* ) )RB_SurfaceTriangles,    // SF_TRIANGLES,
	( void( * ) ( void* ) )RB_SurfacePolychain,    // SF_POLY,
	( void( * ) ( void* ) )RB_SurfaceMesh,         // SF_MD3,
	( void( * ) ( void* ) )RB_SurfaceCMesh,        // SF_MDC,
	( void( * ) ( void* ) )RB_SurfaceAnim,         // SF_MDS,
	( void( * ) ( void* ) )RB_SurfaceFlare,        // SF_FLARE,
	( void( * ) ( void* ) )RB_SurfaceEntity,       // SF_ENTITY
	( void( * ) ( void* ) )RB_SurfaceDisplayList   // SF_DISPLAY_LIST
};
