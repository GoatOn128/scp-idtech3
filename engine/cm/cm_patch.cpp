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


#include "cm_local.h"
#include "cm_patch.h"

/*

This file does not reference any globals, and has these entry points:

void CM_ClearLevelPatches( void );
struct patchCollide_s	*CM_GeneratePatchCollide( int width, int height, const idVec3 *points );
void CM_TraceThroughPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc );
qboolean CM_PositionTestInPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc );
void CM_DrawDebugSurface( void (*drawPoly)(int color, int numPoints, flaot *points) );


WARNING: this may misbehave with meshes that have rows or columns that only
degenerate a few triangles.  Completely degenerate rows and columns are handled
properly.
*/

/*
#define	MAX_FACETS			1024
#define	MAX_PATCH_PLANES	2048

typedef struct {
	float	plane[4];
	int		signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
} patchPlane_t;

typedef struct {
	int			surfacePlane;
	int			numBorders;		// 3 or four + 6 axial bevels + 4 or 3 * 4 edge bevels
	int			borderPlanes[4+6+16];
	int			borderInward[4+6+16];
	qboolean	borderNoAdjust[4+6+16];
} facet_t;

typedef struct patchCollide_s {
	idVec3	bounds[2];
	int		numPlanes;			// surface planes plus edge planes
	patchPlane_t	*planes;
	int		numFacets;
	facet_t	*facets;
} patchCollide_t;


#define	MAX_GRID_SIZE	129

typedef struct {
	int			width;
	int			height;
	qboolean	wrapWidth;
	qboolean	wrapHeight;
	idVec3	points[MAX_GRID_SIZE][MAX_GRID_SIZE];	// [width][height]
} cGrid_t;

#define	SUBDIVIDE_DISTANCE	16	//4	// never more than this units away from curve
#define	PLANE_TRI_EPSILON	0.1
#define	WRAP_POINT_EPSILON	0.1
*/

int c_totalPatchBlocks;
int c_totalPatchSurfaces;
int c_totalPatchEdges;

static const patchCollide_t *debugPatchCollide;
static const facet_t        *debugFacet;
static qboolean debugBlock;
static idVec3 debugBlockPoints[4];

/*
=================
CM_ClearLevelPatches
=================
*/
void CM_ClearLevelPatches( void ) {
	debugPatchCollide = NULL;
	debugFacet = NULL;
}

/*
=================
CM_SignbitsForNormal
=================
*/
static int CM_SignbitsForNormal( idVec3 normal ) {
	int bits, j;

	bits = 0;
	for ( j = 0 ; j < 3 ; j++ ) {
		if ( normal[j] < 0 ) {
			bits |= 1 << j;
		}
	}
	return bits;
}

/*
=====================
CM_PlaneFromPoints

Returns false if the triangle is degenrate.
The normal will point out of the clock for clockwise ordered points
=====================
*/
static qboolean CM_PlaneFromPoints( idPlane &plane, const idVec3 &a, const idVec3 &b, const idVec3 &c ) {
	idVec3 d1, d2;

	d1 = b - a;
	d2 = c - a;
	plane.Normal().Cross( d2, d1 );
	if ( plane.Normal().Normalize() == 0.0f ) {
		return qfalse;
	}

	plane.SetDist( a * plane.Normal() );
	return qtrue;
}


/*
================================================================================

GRID SUBDIVISION

================================================================================
*/

/*
=================
CM_NeedsSubdivision

Returns true if the given quadratic curve is not flat enough for our
collision detection purposes
=================
*/
static qboolean CM_NeedsSubdivision( idVec3 a, idVec3 b, idVec3 c ) {
	idVec3 cmid;
	idVec3 lmid;
	idVec3 delta;
	float dist;
	int i;

	// calculate the linear midpoint
	for ( i = 0 ; i < 3 ; i++ ) {
		lmid[i] = 0.5 * ( a[i] + c[i] );
	}

	// calculate the exact curve midpoint
	for ( i = 0 ; i < 3 ; i++ ) {
		cmid[i] = 0.5 * ( 0.5 * ( a[i] + b[i] ) + 0.5 * ( b[i] + c[i] ) );
	}

	// see if the curve is far enough away from the linear mid
	delta = cmid - lmid;
	dist = delta.Length();

	return dist >= SUBDIVIDE_DISTANCE;
}

/*
===============
CM_Subdivide

a, b, and c are control points.
the subdivided sequence will be: a, out1, out2, out3, c
===============
*/
static void CM_Subdivide( const idVec3 &a, const idVec3 &b, const idVec3 &c, idVec3 &out1, idVec3 &out2, idVec3 &out3 ) {
	int i;

	for ( i = 0 ; i < 3 ; i++ ) {
		out1[i] = 0.5 * ( a[i] + b[i] );
		out3[i] = 0.5 * ( b[i] + c[i] );
		out2[i] = 0.5 * ( out1[i] + out3[i] );
	}
}

/*
=================
CM_TransposeGrid

Swaps the rows and columns in place
=================
*/
static void CM_TransposeGrid( cGrid_t *grid ) {
	int i, j, l;
	idVec3 temp;
	qboolean tempWrap;

	if ( grid->width > grid->height ) {
		for ( i = 0 ; i < grid->height ; i++ ) {
			for ( j = i + 1 ; j < grid->width ; j++ ) {
				if ( j < grid->height ) {
					// swap the value
					temp = grid->points[i][j];
					grid->points[i][j] = grid->points[j][i];
					grid->points[j][i] = temp;
				} else {
					// just copy
					grid->points[i][j] = grid->points[j][i];
				}
			}
		}
	} else {
		for ( i = 0 ; i < grid->width ; i++ ) {
			for ( j = i + 1 ; j < grid->height ; j++ ) {
				if ( j < grid->width ) {
					// swap the value
					temp = grid->points[j][i];
					grid->points[j][i] = grid->points[i][j];
					grid->points[i][j] = temp;
				} else {
					// just copy
					grid->points[j][i] = grid->points[i][j];
				}
			}
		}
	}

	l = grid->width;
	grid->width = grid->height;
	grid->height = l;

	tempWrap = grid->wrapWidth;
	grid->wrapWidth = grid->wrapHeight;
	grid->wrapHeight = tempWrap;
}

/*
===================
CM_SetGridWrapWidth

If the left and right columns are exactly equal, set grid->wrapWidth qtrue
===================
*/
static void CM_SetGridWrapWidth( cGrid_t *grid ) {
	int i, j;
	float d;

	for ( i = 0 ; i < grid->height ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			d = grid->points[0][i][j] - grid->points[grid->width - 1][i][j];
			if ( d < -WRAP_POINT_EPSILON || d > WRAP_POINT_EPSILON ) {
				break;
			}
		}
		if ( j != 3 ) {
			break;
		}
	}
	if ( i == grid->height ) {
		grid->wrapWidth = qtrue;
	} else {
		grid->wrapWidth = qfalse;
	}
}

/*
=================
CM_SubdivideGridColumns

Adds columns as necessary to the grid until
all the aproximating points are within SUBDIVIDE_DISTANCE
from the true curve
=================
*/
static void CM_SubdivideGridColumns( cGrid_t *grid ) {
	int i, j, k;

	for ( i = 0 ; i < grid->width - 2 ;  ) {
		// grid->points[i][x] is an interpolating control point
		// grid->points[i+1][x] is an aproximating control point
		// grid->points[i+2][x] is an interpolating control point

		//
		// first see if we can collapse the aproximating collumn away
		//
		for ( j = 0 ; j < grid->height ; j++ ) {
			if ( CM_NeedsSubdivision( grid->points[i][j], grid->points[i + 1][j], grid->points[i + 2][j] ) ) {
				break;
			}
		}
		if ( j == grid->height ) {
			// all of the points were close enough to the linear midpoints
			// that we can collapse the entire column away
			for ( j = 0 ; j < grid->height ; j++ ) {
				// remove the column
				for ( k = i + 2 ; k < grid->width ; k++ ) {
					grid->points[k - 1][j] = grid->points[k][j];
				}
			}

			grid->width--;

			// go to the next curve segment
			i++;
			continue;
		}

		//
		// we need to subdivide the curve
		//
		for ( j = 0 ; j < grid->height ; j++ ) {
			idVec3 prev, mid, next;

			// save the control points now
			prev = grid->points[i][j];
			mid = grid->points[i + 1][j];
			next = grid->points[i + 2][j];

			// make room for two additional columns in the grid
			// columns i+1 will be replaced, column i+2 will become i+4
			// i+1, i+2, and i+3 will be generated
			for ( k = grid->width - 1 ; k > i + 1 ; k-- ) {
				grid->points[k + 2][j] = grid->points[k][j];
			}

			// generate the subdivided points
			CM_Subdivide( prev, mid, next, grid->points[i + 1][j], grid->points[i + 2][j], grid->points[i + 3][j] );
		}

		grid->width += 2;

		// the new aproximating point at i+1 may need to be removed
		// or subdivided farther, so don't advance i
	}
}

/*
======================
CM_ComparePoints
======================
*/
#define POINT_EPSILON   0.1
static qboolean CM_ComparePoints( const idVec3 &a, const idVec3 &b ) {
	float d;

	d = a[0] - b[0];
	if ( d < -POINT_EPSILON || d > POINT_EPSILON ) {
		return qfalse;
	}
	d = a[1] - b[1];
	if ( d < -POINT_EPSILON || d > POINT_EPSILON ) {
		return qfalse;
	}
	d = a[2] - b[2];
	if ( d < -POINT_EPSILON || d > POINT_EPSILON ) {
		return qfalse;
	}
	return qtrue;
}

/*
=================
CM_RemoveDegenerateColumns

If there are any identical columns, remove them
=================
*/
static void CM_RemoveDegenerateColumns( cGrid_t *grid ) {
	int i, j, k;

	for ( i = 0 ; i < grid->width - 1 ; i++ ) {
		for ( j = 0 ; j < grid->height ; j++ ) {
			if ( !CM_ComparePoints( grid->points[i][j], grid->points[i + 1][j] ) ) {
				break;
			}
		}

		if ( j != grid->height ) {
			continue;   // not degenerate
		}

		for ( j = 0 ; j < grid->height ; j++ ) {
			// remove the column
			for ( k = i + 2 ; k < grid->width ; k++ ) {
				grid->points[k - 1][j] = grid->points[k][j];
			}
		}
		grid->width--;

		// check against the next column
		i--;
	}
}

/*
================================================================================

PATCH COLLIDE GENERATION

================================================================================
*/

static int numPlanes;
static patchPlane_t planes[MAX_PATCH_PLANES];

static int numFacets;
static facet_t facets[MAX_PATCH_PLANES];          //maybe MAX_FACETS ??

#define NORMAL_EPSILON  0.0001
#define DIST_EPSILON    0.02

/*
==================
CM_PlaneEqual
==================
*/
int CM_PlaneEqual( patchPlane_t *p, const idPlane &plane, int *flipped ) {
	const idPlane invplane = -plane;

	if (
		fabs( p->plane.Normal()[0] - plane.Normal()[0] ) < NORMAL_EPSILON
		&& fabs( p->plane.Normal()[1] - plane.Normal()[1] ) < NORMAL_EPSILON
		&& fabs( p->plane.Normal()[2] - plane.Normal()[2] ) < NORMAL_EPSILON
		&& fabs( p->plane.Dist() - plane.Dist() ) < DIST_EPSILON ) {
		*flipped = qfalse;
		return qtrue;
	}

	if (
		fabs( p->plane.Normal()[0] - invplane.Normal()[0] ) < NORMAL_EPSILON
		&& fabs( p->plane.Normal()[1] - invplane.Normal()[1] ) < NORMAL_EPSILON
		&& fabs( p->plane.Normal()[2] - invplane.Normal()[2] ) < NORMAL_EPSILON
		&& fabs( p->plane.Dist() - invplane.Dist() ) < DIST_EPSILON ) {
		*flipped = qtrue;
		return qtrue;
	}

	return qfalse;
}

/*
==================
CM_SnapVector
==================
*/
void CM_SnapVector( idVec3 &normal ) {
	int i;

	for ( i = 0 ; i < 3 ; i++ )
	{
		if ( fabs( normal[i] - 1 ) < NORMAL_EPSILON ) {
			normal.Zero();
			normal[i] = 1;
			break;
		}
		if ( fabs( normal[i] - -1 ) < NORMAL_EPSILON ) {
			normal.Zero();
			normal[i] = -1;
			break;
		}
	}
}

/*
==================
CM_FindPlane2
==================
*/
int CM_FindPlane2( const idPlane &plane, int *flipped ) {
	int i;

	// see if the points are close enough to an existing plane
	for ( i = 0 ; i < numPlanes ; i++ ) {
		if ( CM_PlaneEqual( &planes[i], plane, flipped ) ) {
			return i;
		}
	}

	// add a new plane
	if ( numPlanes == MAX_PATCH_PLANES ) {
		Com_Error( ERR_DROP, "MAX_PATCH_PLANES" );
	}

	planes[numPlanes].plane = plane;
	planes[numPlanes].signbits = CM_SignbitsForNormal( plane.Normal() );

	numPlanes++;

	*flipped = qfalse;

	return numPlanes - 1;
}

/*
==================
CM_FindPlane
==================
*/
static int CM_FindPlane( const idVec3 &p1, const idVec3 &p2, const idVec3 &p3 ) {
	idPlane plane;
	int i;
	float d;

	if ( !CM_PlaneFromPoints( plane, p1, p2, p3 ) ) {
		return -1;
	}

	// see if the points are close enough to an existing plane
	for ( i = 0 ; i < numPlanes ; i++ ) {
		if ( plane.Normal() * planes[i].plane.Normal() < 0 ) {
			continue;   // allow backwards planes?
		}

		d = p1 * planes[i].plane.Normal() - planes[i].plane.Dist();
		if ( d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON ) {
			continue;
		}

		d = p2 * planes[i].plane.Normal() - planes[i].plane.Dist();
		if ( d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON ) {
			continue;
		}

		d = p3 * planes[i].plane.Normal() - planes[i].plane.Dist();
		if ( d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON ) {
			continue;
		}

		// found it
		return i;
	}

	// add a new plane
	if ( numPlanes == MAX_PATCH_PLANES ) {
		Com_Error( ERR_DROP, "MAX_PATCH_PLANES" );
	}

	planes[numPlanes].plane = plane;
	planes[numPlanes].signbits = CM_SignbitsForNormal( plane.Normal() );

	numPlanes++;

	return numPlanes - 1;
}

/*
==================
CM_PointOnPlaneSide
==================
*/
static int CM_PointOnPlaneSide( const idVec3 &p, int planeNum ) {
	const idPlane *plane;
	float d;

	if ( planeNum == -1 ) {
		return SIDE_ON;
	}
	plane = &planes[ planeNum ].plane;

	d = p * plane->Normal() - plane->Dist();

	if ( d > PLANE_TRI_EPSILON ) {
		return SIDE_FRONT;
	}

	if ( d < -PLANE_TRI_EPSILON ) {
		return SIDE_BACK;
	}

	return SIDE_ON;
}

/*
==================
CM_GridPlane
==================
*/
static int  CM_GridPlane( int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], int i, int j, int tri ) {
	int p;

	p = gridPlanes[i][j][tri];
	if ( p != -1 ) {
		return p;
	}
	p = gridPlanes[i][j][!tri];
	if ( p != -1 ) {
		return p;
	}

	// should never happen
	Com_Printf( "WARNING: CM_GridPlane unresolvable\n" );
	return -1;
}

/*
==================
CM_EdgePlaneNum
==================
*/
static int CM_EdgePlaneNum( cGrid_t *grid, int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], int i, int j, int k ) {
	const idVec3 *p1, *p2;
	idVec3 up;
	int p;

	switch ( k ) {
	case 0: // top border
		p1 = &grid->points[i][j];
		p2 = &grid->points[i + 1][j];
		p = CM_GridPlane( gridPlanes, i, j, 0 );
		up = *p1 + planes[ p ].plane.Normal() * 4;
		return CM_FindPlane( *p1, *p2, up );

	case 2: // bottom border
		p1 = &grid->points[i][j + 1];
		p2 = &grid->points[i + 1][j + 1];
		p = CM_GridPlane( gridPlanes, i, j, 1 );
		up = *p1 + planes[ p ].plane.Normal() * 4;
		return CM_FindPlane( *p2, *p1, up );

	case 3: // left border
		p1 = &grid->points[i][j];
		p2 = &grid->points[i][j + 1];
		p = CM_GridPlane( gridPlanes, i, j, 1 );
		up = *p1 + planes[ p ].plane.Normal() * 4;
		return CM_FindPlane( *p2, *p1, up );

	case 1: // right border
		p1 = &grid->points[i + 1][j];
		p2 = &grid->points[i + 1][j + 1];
		p = CM_GridPlane( gridPlanes, i, j, 0 );
		up = *p1 + planes[ p ].plane.Normal() * 4;
		return CM_FindPlane( *p1, *p2, up );

	case 4: // diagonal out of triangle 0
		p1 = &grid->points[i + 1][j + 1];
		p2 = &grid->points[i][j];
		p = CM_GridPlane( gridPlanes, i, j, 0 );
		up = *p1 + planes[ p ].plane.Normal() * 4;
		return CM_FindPlane( *p1, *p2, up );

	case 5: // diagonal out of triangle 1
		p1 = &grid->points[i][j];
		p2 = &grid->points[i + 1][j + 1];
		p = CM_GridPlane( gridPlanes, i, j, 1 );
		up = *p1 + planes[ p ].plane.Normal() * 4;
		return CM_FindPlane( *p1, *p2, up );

	}

	Com_Error( ERR_DROP, "CM_EdgePlaneNum: bad k" );
	return -1;
}

/*
===================
CM_SetBorderInward
===================
*/
static void CM_SetBorderInward( facet_t *facet, cGrid_t *grid, int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2],
								int i, int j, int which ) {
	int k, l;
	const idVec3 *points[4];
	int numPoints;

	switch ( which ) {
	case - 1:
		points[0] = &grid->points[i][j];
		points[1] = &grid->points[i + 1][j];
		points[2] = &grid->points[i + 1][j + 1];
		points[3] = &grid->points[i][j + 1];
		numPoints = 4;
		break;
	case 0:
		points[0] = &grid->points[i][j];
		points[1] = &grid->points[i + 1][j];
		points[2] = &grid->points[i + 1][j + 1];
		numPoints = 3;
		break;
	case 1:
		points[0] = &grid->points[i + 1][j + 1];
		points[1] = &grid->points[i][j + 1];
		points[2] = &grid->points[i][j];
		numPoints = 3;
		break;
	default:
		Com_Error( ERR_FATAL, "CM_SetBorderInward: bad parameter" );
		numPoints = 0;
		break;
	}

	for ( k = 0 ; k < facet->numBorders ; k++ ) {
		int front, back;

		front = 0;
		back = 0;

		for ( l = 0 ; l < numPoints ; l++ ) {
			int side;

			side = CM_PointOnPlaneSide( *points[l], facet->borderPlanes[k] );
			if ( side == SIDE_FRONT ) {
				front++;
			}
			if ( side == SIDE_BACK ) {
				back++;
			}
		}

		if ( front && !back ) {
			facet->borderInward[k] = qtrue;
		} else if ( back && !front ) {
			facet->borderInward[k] = qfalse;
		} else if ( !front && !back ) {
			// flat side border
			facet->borderPlanes[k] = -1;
		} else {
			// bisecting side border
			Com_DPrintf( "WARNING: CM_SetBorderInward: mixed plane sides\n" );
			facet->borderInward[k] = qfalse;
			if ( !debugBlock ) {
				debugBlock = qtrue;
				debugBlockPoints[0] = grid->points[i][j];
				debugBlockPoints[1] = grid->points[i + 1][j];
				debugBlockPoints[2] = grid->points[i + 1][j + 1];
				debugBlockPoints[3] = grid->points[i][j + 1];
			}
		}
	}
}

/*
==================
CM_ValidateFacet

If the facet isn't bounded by its borders, we screwed up.
==================
*/
static qboolean CM_ValidateFacet( facet_t *facet ) {
	idPlane plane;
	int j;
	winding_t   *w;
	idVec3 bounds[2];

	if ( facet->surfacePlane == -1 ) {
		return qfalse;
	}

	plane = planes[ facet->surfacePlane ].plane;
	w = BaseWindingForPlane( plane.Normal(), plane.Dist() );
	for ( j = 0 ; j < facet->numBorders && w ; j++ ) {
		if ( facet->borderPlanes[j] == -1 ) {
			return qfalse;
		}
		plane = planes[ facet->borderPlanes[j] ].plane;
		if ( !facet->borderInward[j] ) {
			plane = -plane;
		}
		ChopWindingInPlace( &w, plane.Normal(), plane.Dist(), 0.1f );
	}

	if ( !w ) {
		return qfalse;      // winding was completely chopped away
	}

	// see if the facet is unreasonably large
	WindingBounds( w, bounds[0], bounds[1] );
	FreeWinding( w );

	for ( j = 0 ; j < 3 ; j++ ) {
		if ( bounds[1][j] - bounds[0][j] > MAX_MAP_BOUNDS ) {
			return qfalse;      // we must be missing a plane
		}
		if ( bounds[0][j] >= MAX_MAP_BOUNDS ) {
			return qfalse;
		}
		if ( bounds[1][j] <= -MAX_MAP_BOUNDS ) {
			return qfalse;
		}
	}
	return qtrue;       // winding is fine
}

/*
==================
CM_AddFacetBevels
==================
*/
void CM_AddFacetBevels( facet_t *facet ) {

	int i, j, k, l;
	int axis, dir, order, flipped;
	idPlane plane, newplane;
	float d;
	winding_t *w, *w2;
	idVec3 mins, maxs, vec, vec2;

	plane = planes[ facet->surfacePlane ].plane;

	w = BaseWindingForPlane( plane.Normal(), plane.Dist() );
	for ( j = 0 ; j < facet->numBorders && w ; j++ ) {
		if ( facet->borderPlanes[j] == facet->surfacePlane ) {
			continue;
		}
		plane = planes[ facet->borderPlanes[j] ].plane;

		if ( !facet->borderInward[j] ) {
			plane = -plane;
		}

		ChopWindingInPlace( &w, plane.Normal(), plane.Dist(), 0.1f );
	}
	if ( !w ) {
		return;
	}

	WindingBounds( w, mins, maxs );

	// add the axial planes
	order = 0;
	for ( axis = 0 ; axis < 3 ; axis++ )
	{
		for ( dir = -1 ; dir <= 1 ; dir += 2, order++ )
		{
			plane.Normal().Zero();
			plane.Normal()[axis] = dir;
			if ( dir == 1 ) {
				plane.SetDist( maxs[axis] );
			} else {
				plane.SetDist( -mins[axis] );
			}
			//if it's the surface plane
			if ( CM_PlaneEqual( &planes[facet->surfacePlane], plane, &flipped ) ) {
				continue;
			}
			// see if the plane is allready present
			for ( i = 0 ; i < facet->numBorders ; i++ ) {
				if ( CM_PlaneEqual( &planes[facet->borderPlanes[i]], plane, &flipped ) ) {
					break;
				}
			}

			if ( i == facet->numBorders ) {
				if ( facet->numBorders > 4 + 6 + 16 ) {
					Com_Printf( "ERROR: too many bevels\n" );
				}
				facet->borderPlanes[facet->numBorders] = CM_FindPlane2( plane, &flipped );
				facet->borderNoAdjust[facet->numBorders] = 0;
				facet->borderInward[facet->numBorders] = flipped;
				facet->numBorders++;
			}
		}
	}
	//
	// add the edge bevels
	//
	// test the non-axial plane edges
	for ( j = 0 ; j < w->numpoints ; j++ )
	{
		k = ( j + 1 ) % w->numpoints;
		vec = w->p[j] - w->p[k];
		//if it's a degenerate edge
		if ( vec.Normalize() < 0.5 ) {
			continue;
		}
		CM_SnapVector( vec );
		for ( k = 0; k < 3 ; k++ )
			if ( vec[k] == -1 || vec[k] == 1 ) {
				break;
			}           // axial
		if ( k < 3 ) {
			continue;   // only test non-axial edges

		}
		// try the six possible slanted axials from this edge
		for ( axis = 0 ; axis < 3 ; axis++ )
		{
			for ( dir = -1 ; dir <= 1 ; dir += 2 )
			{
				// construct a plane
				vec2.Zero();
				vec2[axis] = dir;
				plane.Normal().Cross( vec, vec2 );
				if ( plane.Normal().Normalize() < 0.5 ) {
					continue;
				}
				plane.SetDist( w->p[j] * plane.Normal() );

				// if all the points of the facet winding are
				// behind this plane, it is a proper edge bevel
				for ( l = 0 ; l < w->numpoints ; l++ )
				{
					d = w->p[l] * plane.Normal() - plane.Dist();
					if ( d > 0.1 ) {
						break;  // point in front
					}
				}
				if ( l < w->numpoints ) {
					continue;
				}

				//if it's the surface plane
				if ( CM_PlaneEqual( &planes[facet->surfacePlane], plane, &flipped ) ) {
					continue;
				}
				// see if the plane is allready present
				for ( i = 0 ; i < facet->numBorders ; i++ ) {
					if ( CM_PlaneEqual( &planes[facet->borderPlanes[i]], plane, &flipped ) ) {
						break;
					}
				}

				if ( i == facet->numBorders ) {
					if ( facet->numBorders > 4 + 6 + 16 ) {
						Com_Printf( "ERROR: too many bevels\n" );
					}
					facet->borderPlanes[facet->numBorders] = CM_FindPlane2( plane, &flipped );

					for ( k = 0 ; k < facet->numBorders ; k++ ) {
						if ( facet->borderPlanes[facet->numBorders] ==
							 facet->borderPlanes[k] ) {
							Com_Printf( "WARNING: bevel plane already used\n" );
						}
					}

					facet->borderNoAdjust[facet->numBorders] = 0;
					facet->borderInward[facet->numBorders] = flipped;
					//
					w2 = CopyWinding( w );
					newplane = planes[facet->borderPlanes[facet->numBorders]].plane;
					if ( !facet->borderInward[facet->numBorders] ) {
						newplane = -newplane;
					} //end if
					ChopWindingInPlace( &w2, newplane.Normal(), newplane.Dist(), 0.1f );
					if ( !w2 ) {
						Com_DPrintf( "WARNING: CM_AddFacetBevels... invalid bevel\n" );
						continue;
					} else {
						FreeWinding( w2 );
					}
					//
					facet->numBorders++;
					//already got a bevel
//					break;
				}
			}
		}
	}
	FreeWinding( w );

#ifndef BSPC
	//add opposite plane
	facet->borderPlanes[facet->numBorders] = facet->surfacePlane;
	facet->borderNoAdjust[facet->numBorders] = 0;
	facet->borderInward[facet->numBorders] = qtrue;
	facet->numBorders++;
#endif //BSPC

}

typedef enum {
	EN_TOP,
	EN_RIGHT,
	EN_BOTTOM,
	EN_LEFT
} edgeName_t;

/*
==================
CM_PatchCollideFromGrid
==================
*/
static void CM_PatchCollideFromGrid( cGrid_t *grid, patchCollide_t *pf ) {
	int i, j;
	const idVec3   *p1, *p2, *p3;
	static int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2];
	facet_t         *facet;
	int borders[4];
	int noAdjust[4];

	numPlanes = 0;
	numFacets = 0;

	// find the planes for each triangle of the grid
	for ( i = 0 ; i < grid->width - 1 ; i++ ) {
		for ( j = 0 ; j < grid->height - 1 ; j++ ) {
			p1 = &grid->points[i][j];
			p2 = &grid->points[i + 1][j];
			p3 = &grid->points[i + 1][j + 1];
			gridPlanes[i][j][0] = CM_FindPlane( *p1, *p2, *p3 );

			p1 = &grid->points[i + 1][j + 1];
			p2 = &grid->points[i][j + 1];
			p3 = &grid->points[i][j];
			gridPlanes[i][j][1] = CM_FindPlane( *p1, *p2, *p3 );
		}
	}

	// create the borders for each facet
	for ( i = 0 ; i < grid->width - 1 ; i++ ) {
		for ( j = 0 ; j < grid->height - 1 ; j++ ) {

			borders[EN_TOP] = -1;
			if ( j > 0 ) {
				borders[EN_TOP] = gridPlanes[i][j - 1][1];
			} else if ( grid->wrapHeight ) {
				borders[EN_TOP] = gridPlanes[i][grid->height - 2][1];
			}
			noAdjust[EN_TOP] = ( borders[EN_TOP] == gridPlanes[i][j][0] );
			if ( borders[EN_TOP] == -1 || noAdjust[EN_TOP] ) {
				borders[EN_TOP] = CM_EdgePlaneNum( grid, gridPlanes, i, j, 0 );
			}

			borders[EN_BOTTOM] = -1;
			if ( j < grid->height - 2 ) {
				borders[EN_BOTTOM] = gridPlanes[i][j + 1][0];
			} else if ( grid->wrapHeight ) {
				borders[EN_BOTTOM] = gridPlanes[i][0][0];
			}
			noAdjust[EN_BOTTOM] = ( borders[EN_BOTTOM] == gridPlanes[i][j][1] );
			if ( borders[EN_BOTTOM] == -1 || noAdjust[EN_BOTTOM] ) {
				borders[EN_BOTTOM] = CM_EdgePlaneNum( grid, gridPlanes, i, j, 2 );
			}

			borders[EN_LEFT] = -1;
			if ( i > 0 ) {
				borders[EN_LEFT] = gridPlanes[i - 1][j][0];
			} else if ( grid->wrapWidth ) {
				borders[EN_LEFT] = gridPlanes[grid->width - 2][j][0];
			}
			noAdjust[EN_LEFT] = ( borders[EN_LEFT] == gridPlanes[i][j][1] );
			if ( borders[EN_LEFT] == -1 || noAdjust[EN_LEFT] ) {
				borders[EN_LEFT] = CM_EdgePlaneNum( grid, gridPlanes, i, j, 3 );
			}

			borders[EN_RIGHT] = -1;
			if ( i < grid->width - 2 ) {
				borders[EN_RIGHT] = gridPlanes[i + 1][j][1];
			} else if ( grid->wrapWidth ) {
				borders[EN_RIGHT] = gridPlanes[0][j][1];
			}
			noAdjust[EN_RIGHT] = ( borders[EN_RIGHT] == gridPlanes[i][j][0] );
			if ( borders[EN_RIGHT] == -1 || noAdjust[EN_RIGHT] ) {
				borders[EN_RIGHT] = CM_EdgePlaneNum( grid, gridPlanes, i, j, 1 );
			}

			if ( numFacets == MAX_FACETS ) {
				Com_Error( ERR_DROP, "MAX_FACETS" );
			}
			facet = &facets[numFacets];
			memset( facet, 0, sizeof( *facet ) );

			if ( gridPlanes[i][j][0] == gridPlanes[i][j][1] ) {
				if ( gridPlanes[i][j][0] == -1 ) {
					continue;       // degenrate
				}
				facet->surfacePlane = gridPlanes[i][j][0];
				facet->numBorders = 4;
				facet->borderPlanes[0] = borders[EN_TOP];
				facet->borderNoAdjust[0] = noAdjust[EN_TOP];
				facet->borderPlanes[1] = borders[EN_RIGHT];
				facet->borderNoAdjust[1] = noAdjust[EN_RIGHT];
				facet->borderPlanes[2] = borders[EN_BOTTOM];
				facet->borderNoAdjust[2] = noAdjust[EN_BOTTOM];
				facet->borderPlanes[3] = borders[EN_LEFT];
				facet->borderNoAdjust[3] = noAdjust[EN_LEFT];
				CM_SetBorderInward( facet, grid, gridPlanes, i, j, -1 );
				if ( CM_ValidateFacet( facet ) ) {
					CM_AddFacetBevels( facet );
					numFacets++;
				}
			} else {
				// two seperate triangles
				facet->surfacePlane = gridPlanes[i][j][0];
				facet->numBorders = 3;
				facet->borderPlanes[0] = borders[EN_TOP];
				facet->borderNoAdjust[0] = noAdjust[EN_TOP];
				facet->borderPlanes[1] = borders[EN_RIGHT];
				facet->borderNoAdjust[1] = noAdjust[EN_RIGHT];
				facet->borderPlanes[2] = gridPlanes[i][j][1];
				if ( facet->borderPlanes[2] == -1 ) {
					facet->borderPlanes[2] = borders[EN_BOTTOM];
					if ( facet->borderPlanes[2] == -1 ) {
						facet->borderPlanes[2] = CM_EdgePlaneNum( grid, gridPlanes, i, j, 4 );
					}
				}
				CM_SetBorderInward( facet, grid, gridPlanes, i, j, 0 );
				if ( CM_ValidateFacet( facet ) ) {
					CM_AddFacetBevels( facet );
					numFacets++;
				}

				if ( numFacets == MAX_FACETS ) {
					Com_Error( ERR_DROP, "MAX_FACETS" );
				}
				facet = &facets[numFacets];
				memset( facet, 0, sizeof( *facet ) );

				facet->surfacePlane = gridPlanes[i][j][1];
				facet->numBorders = 3;
				facet->borderPlanes[0] = borders[EN_BOTTOM];
				facet->borderNoAdjust[0] = noAdjust[EN_BOTTOM];
				facet->borderPlanes[1] = borders[EN_LEFT];
				facet->borderNoAdjust[1] = noAdjust[EN_LEFT];
				facet->borderPlanes[2] = gridPlanes[i][j][0];
				if ( facet->borderPlanes[2] == -1 ) {
					facet->borderPlanes[2] = borders[EN_TOP];
					if ( facet->borderPlanes[2] == -1 ) {
						facet->borderPlanes[2] = CM_EdgePlaneNum( grid, gridPlanes, i, j, 5 );
					}
				}
				CM_SetBorderInward( facet, grid, gridPlanes, i, j, 1 );
				if ( CM_ValidateFacet( facet ) ) {
					CM_AddFacetBevels( facet );
					numFacets++;
				}
			}
		}
	}

	// copy the results out
	pf->numPlanes = numPlanes;
	pf->numFacets = numFacets;
	pf->facets = (facet_t *) Mem_ClearedAlloc( numFacets * sizeof( *pf->facets ), TAG_IDLIB );
	memcpy( pf->facets, facets, numFacets * sizeof( *pf->facets ) );
	pf->planes = (patchPlane_t *) Mem_ClearedAlloc( numPlanes * sizeof( *pf->planes ), TAG_IDLIB );
	memcpy( pf->planes, planes, numPlanes * sizeof( *pf->planes ) );
}


static void CM_AddPointToBounds( const idVec3 &v, idVec3 &mins, idVec3 &maxs ) {
	int i;

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( v[i] < mins[i] ) {
				mins[i] = v[i];
			}
		if ( v[i] > maxs[i] ) {
				maxs[i] = v[i];
			}
	}
}

/*
===================
CM_GeneratePatchCollide

Creates an internal structure that will be used to perform
collision detection with a patch mesh.

Points is packed as concatenated rows.
===================
*/
struct patchCollide_s   *CM_GeneratePatchCollide( int width, int height, idVec3 *points ) {
	patchCollide_t  *pf;
	static cGrid_t grid;
	int i, j;

	if ( width <= 2 || height <= 2 || !points ) {
		Com_Error( ERR_DROP, "CM_GeneratePatchFacets: bad parameters: (%i, %i, %p)",
				   width, height, points );
	}

	if ( !( width & 1 ) || !( height & 1 ) ) {
		Com_Error( ERR_DROP, "CM_GeneratePatchFacets: even sizes are invalid for quadratic meshes" );
	}

	if ( width > MAX_GRID_SIZE || height > MAX_GRID_SIZE ) {
		Com_Error( ERR_DROP, "CM_GeneratePatchFacets: source is > MAX_GRID_SIZE" );
	}

	// build a grid
	grid.width = width;
	grid.height = height;
	grid.wrapWidth = qfalse;
	grid.wrapHeight = qfalse;
	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < height ; j++ ) {
			grid.points[i][j] = points[j * width + i];
		}
	}

	// subdivide the grid
	CM_SetGridWrapWidth( &grid );
	CM_SubdivideGridColumns( &grid );
	CM_RemoveDegenerateColumns( &grid );

	CM_TransposeGrid( &grid );

	CM_SetGridWrapWidth( &grid );
	CM_SubdivideGridColumns( &grid );
	CM_RemoveDegenerateColumns( &grid );

	// we now have a grid of points exactly on the curve
	// the aproximate surface defined by these points will be
	// collided against
	pf = (patchCollide_t *) Mem_ClearedAlloc( sizeof( *pf ), TAG_IDLIB );
	pf->bounds[0] = idVec3( 99999, 99999, 99999 );
	pf->bounds[1] = idVec3( -99999, -99999, -99999 );
	for ( i = 0 ; i < grid.width ; i++ ) {
		for ( j = 0 ; j < grid.height ; j++ ) {
			CM_AddPointToBounds( grid.points[i][j], pf->bounds[0], pf->bounds[1] );
		}
	}

	c_totalPatchBlocks += ( grid.width - 1 ) * ( grid.height - 1 );

	// generate a bsp tree for the surface
	CM_PatchCollideFromGrid( &grid, pf );

	// expand by one unit for epsilon purposes
	pf->bounds[0][0] -= 1;
	pf->bounds[0][1] -= 1;
	pf->bounds[0][2] -= 1;

	pf->bounds[1][0] += 1;
	pf->bounds[1][1] += 1;
	pf->bounds[1][2] += 1;

	return pf;
}

/*
================================================================================

TRACE TESTING

================================================================================
*/

/*
====================
CM_TracePointThroughPatchCollide

  special case for point traces because the patch collide "brushes" have no volume
====================
*/
void CM_TracePointThroughPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc ) {
	qboolean frontFacing[MAX_PATCH_PLANES];
	float intersection[MAX_PATCH_PLANES];
	float intersect;
	const patchPlane_t  *patchPlanes;
	const facet_t   *facet;
	int i, j, k;
	float offset;
	float d1, d2;
#ifndef BSPC
	static cvar_t *cv;
#endif //BSPC

#ifndef BSPC
	if ( !cm_playerCurveClip->integer || !tw->isPoint ) {
		return;
	}
#endif

	// determine the trace's relationship to all patchPlanes
	patchPlanes = pc->planes;
	for ( i = 0 ; i < pc->numPlanes ; i++, patchPlanes++ ) {
		offset = tw->offsets[ patchPlanes->signbits ] * patchPlanes->plane.Normal();
		d1 = tw->start * patchPlanes->plane.Normal() - patchPlanes->plane.Dist() + offset;
		d2 = tw->end * patchPlanes->plane.Normal() - patchPlanes->plane.Dist() + offset;
		if ( d1 <= 0 ) {
			frontFacing[i] = qfalse;
		} else {
			frontFacing[i] = qtrue;
		}
		if ( d1 == d2 ) {
			intersection[i] = 99999;
		} else {
			intersection[i] = d1 / ( d1 - d2 );
			if ( intersection[i] <= 0 ) {
				intersection[i] = 99999;
			}
		}
	}


	// see if any of the surface patchPlanes are intersected
	facet = pc->facets;
	for ( i = 0 ; i < pc->numFacets ; i++, facet++ ) {
		if ( !frontFacing[facet->surfacePlane] ) {
			continue;
		}
		intersect = intersection[facet->surfacePlane];
		if ( intersect < 0 ) {
			continue;       // surface is behind the starting point
		}
		if ( intersect > tw->trace.fraction ) {
			continue;       // already hit something closer
		}
		for ( j = 0 ; j < facet->numBorders ; j++ ) {
			k = facet->borderPlanes[j];
			if ( frontFacing[k] ^ facet->borderInward[j] ) {
				if ( intersection[k] > intersect ) {
					break;
				}
			} else {
				if ( intersection[k] < intersect ) {
					break;
				}
			}
		}
		if ( j == facet->numBorders ) {
			// we hit this facet
#ifndef BSPC
			if ( !cv ) {
				cv = Cvar_Get( "r_debugSurfaceUpdate", "1", 0 );
			}
			if ( cv->integer ) {
				debugPatchCollide = pc;
				debugFacet = facet;
			}
#endif //BSPC
			patchPlanes = &pc->planes[facet->surfacePlane];

			// calculate intersection with a slight pushoff
			offset = tw->offsets[ patchPlanes->signbits ] * patchPlanes->plane.Normal();
			d1 = tw->start * patchPlanes->plane.Normal() - patchPlanes->plane.Dist() + offset;
			d2 = tw->end * patchPlanes->plane.Normal() - patchPlanes->plane.Dist() + offset;
			tw->trace.fraction = ( d1 - SURFACE_CLIP_EPSILON ) / ( d1 - d2 );

			if ( tw->trace.fraction < 0 ) {
				tw->trace.fraction = 0;
			}

			tw->trace.plane.normal = patchPlanes->plane.Normal();
			tw->trace.plane.dist = patchPlanes->plane.Dist();
		}
	}
}

/*
====================
CM_CheckFacetPlane
====================
*/
int CM_CheckFacetPlane( const idPlane &plane, const idVec3 &start, const idVec3 &end, float *enterFrac, float *leaveFrac, int *hit ) {
	float d1, d2, f;

	*hit = qfalse;

	d1 = start * plane.Normal() - plane.Dist();
	d2 = end * plane.Normal() - plane.Dist();

	// if completely in front of face, no intersection with the entire facet
	if ( d1 > 0 && ( d2 >= SURFACE_CLIP_EPSILON || d2 >= d1 )  ) {
		return qfalse;
	}

	// if it doesn't cross the plane, the plane isn't relevent
	if ( d1 <= 0 && d2 <= 0 ) {
		return qtrue;
	}

	// crosses face
	if ( d1 > d2 ) {  // enter
		f = ( d1 - SURFACE_CLIP_EPSILON ) / ( d1 - d2 );
		if ( f < 0 ) {
			f = 0;
		}
		//always favor previous plane hits and thus also the surface plane hit
		if ( f > *enterFrac ) {
			*enterFrac = f;
			*hit = qtrue;
		}
	} else {    // leave
		f = ( d1 + SURFACE_CLIP_EPSILON ) / ( d1 - d2 );
		if ( f > 1 ) {
			f = 1;
		}
		if ( f < *leaveFrac ) {
			*leaveFrac = f;
		}
	}
	return qtrue;
}

/*
====================
CM_TraceThroughPatchCollide
====================
*/
void CM_TraceThroughPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc ) {
	int i, j, hit, hitnum;
	float offset, enterFrac, leaveFrac, t;
	patchPlane_t *patchPlanes;
	facet_t *facet;
	idPlane plane, bestplane;
	idVec3 startp, endp;
#ifndef BSPC
	static cvar_t *cv;
#endif //BSPC

	if ( tw->isPoint ) {
		CM_TracePointThroughPatchCollide( tw, pc );
		return;
	}

	facet = pc->facets;
	for ( i = 0 ; i < pc->numFacets ; i++, facet++ ) {
		enterFrac = -1.0;
		leaveFrac = 1.0;
		hitnum = -1;
		//
		patchPlanes = &pc->planes[ facet->surfacePlane ];
		plane = patchPlanes->plane;
		if ( tw->sphere.use ) {
			// adjust the plane distance apropriately for radius
			plane.SetDist( plane.Dist() + tw->sphere.radius );

			// find the closest point on the capsule to the plane
			t = plane.Normal() * tw->sphere.offset;
			if ( t > 0.0f ) {
				startp = tw->start - tw->sphere.offset;
				endp = tw->end - tw->sphere.offset;
			} else {
				startp = tw->start + tw->sphere.offset;
				endp = tw->end + tw->sphere.offset;
			}
		} else {
			offset = tw->offsets[ patchPlanes->signbits ] * plane.Normal();
			plane.SetDist( plane.Dist() - offset );
			startp = tw->start;
			endp = tw->end;
		}

		if ( !CM_CheckFacetPlane( plane, startp, endp, &enterFrac, &leaveFrac, &hit ) ) {
			continue;
		}
		if ( hit ) {
			bestplane = plane;
		}

		for ( j = 0; j < facet->numBorders; j++ ) {
			patchPlanes = &pc->planes[ facet->borderPlanes[j] ];
			if ( facet->borderInward[j] ) {
				plane = -patchPlanes->plane;
			} else {
				plane = patchPlanes->plane;
			}
			if ( tw->sphere.use ) {
				// adjust the plane distance apropriately for radius
				plane.SetDist( plane.Dist() + tw->sphere.radius );

				// find the closest point on the capsule to the plane
				t = plane.Normal() * tw->sphere.offset;
				if ( t > 0.0f ) {
					startp = tw->start - tw->sphere.offset;
					endp = tw->end - tw->sphere.offset;
				} else {
					startp = tw->start + tw->sphere.offset;
					endp = tw->end + tw->sphere.offset;
				}
			} else {
				// NOTE: this works even though the plane might be flipped because the bbox is centered
				offset = tw->offsets[ patchPlanes->signbits ] * plane.Normal();
				plane.SetDist( plane.Dist() + fabs( offset ) );
				startp = tw->start;
				endp = tw->end;
			}

			if ( !CM_CheckFacetPlane( plane, startp, endp, &enterFrac, &leaveFrac, &hit ) ) {
				break;
			}
			if ( hit ) {
				hitnum = j;
				bestplane = plane;
			}
		}
		if ( j < facet->numBorders ) {
			continue;
		}
		//never clip against the back side
		if ( hitnum == facet->numBorders - 1 ) {
			continue;
		}

		if ( enterFrac < leaveFrac && enterFrac >= 0 ) {
			if ( enterFrac < tw->trace.fraction ) {
				if ( enterFrac < 0 ) {
					enterFrac = 0;
				}
#ifndef BSPC
				if ( !cv ) {
					cv = Cvar_Get( "r_debugSurfaceUpdate", "1", 0 );
				}
				if ( cv && cv->integer ) {
					debugPatchCollide = pc;
					debugFacet = facet;
				}
#endif //BSPC

				tw->trace.fraction = enterFrac;
				tw->trace.plane.normal = bestplane.Normal();
				tw->trace.plane.dist = bestplane.Dist();
			}
		}
	}
}


/*
=======================================================================

POSITION TEST

=======================================================================
*/

/*
====================
CM_PositionTestInPatchCollide
====================
*/
qboolean CM_PositionTestInPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc ) {
	int i, j;
	float offset, t;
	patchPlane_t *patchPlanes;
	facet_t *facet;
	idPlane plane;
	idVec3 startp;

	if ( tw->isPoint ) {
		return qfalse;
	}
	//
	facet = pc->facets;
	for ( i = 0 ; i < pc->numFacets ; i++, facet++ ) {
		patchPlanes = &pc->planes[ facet->surfacePlane ];
		plane = patchPlanes->plane;
		if ( tw->sphere.use ) {
			// adjust the plane distance apropriately for radius
			plane.SetDist( plane.Dist() + tw->sphere.radius );

			// find the closest point on the capsule to the plane
			t = plane.Normal() * tw->sphere.offset;
			if ( t > 0 ) {
				startp = tw->start - tw->sphere.offset;
			} else {
				startp = tw->start + tw->sphere.offset;
			}
		} else {
			offset = tw->offsets[ patchPlanes->signbits ] * plane.Normal();
			plane.SetDist( plane.Dist() - offset );
			startp = tw->start;
		}

		if ( plane.Normal() * startp - plane.Dist() > 0.0f ) {
			continue;
		}

		for ( j = 0; j < facet->numBorders; j++ ) {
			patchPlanes = &pc->planes[ facet->borderPlanes[j] ];
			if ( facet->borderInward[j] ) {
				plane = -patchPlanes->plane;
			} else {
				plane = patchPlanes->plane;
			}
			if ( tw->sphere.use ) {
				// adjust the plane distance apropriately for radius
				plane.SetDist( plane.Dist() + tw->sphere.radius );

				// find the closest point on the capsule to the plane
				t = plane.Normal() * tw->sphere.offset;
				if ( t > 0.0f ) {
					startp = tw->start - tw->sphere.offset;
				} else {
					startp = tw->start + tw->sphere.offset;
				}
			} else {
				// NOTE: this works even though the plane might be flipped because the bbox is centered
				offset = tw->offsets[ patchPlanes->signbits ] * plane.Normal();
				plane.SetDist( plane.Dist() + fabs( offset ) );
				startp = tw->start;
			}

			if ( plane.Normal() * startp - plane.Dist() > 0.0f ) {
				break;
			}
		}
		if ( j < facet->numBorders ) {
			continue;
		}
		// inside this patch facet
		return qtrue;
	}
	return qfalse;
}

/*
=======================================================================

DEBUGGING

=======================================================================
*/


/*
==================
CM_DrawDebugSurface

Called from the renderer
==================
*/
#ifndef BSPC
void BotDrawDebugPolygons( void ( *drawPoly )( int color, int numPoints, float *points ), int value );
#endif

void CM_DrawDebugSurface( void ( *drawPoly )( int color, int numPoints, float *points ) ) {
	static cvar_t   *cv;
#ifndef BSPC
	static cvar_t   *cv2;
#endif
	const patchCollide_t    *pc;
	facet_t         *facet;
	winding_t       *w;
	int i, j, k, n;
	int curplanenum, planenum, curinward, inward;
	idPlane plane;
	idVec3 mins( -15, -15, -28 ), maxs( 15, 15, 28 );
	//idVec3 mins = {0, 0, 0}, maxs = {0, 0, 0};
	idVec3 v1, v2;

#ifndef BSPC
	if ( !cv2 ) {
		cv2 = Cvar_Get( "r_debugSurface", "0", 0 );
	}

	if ( cv2->integer != 1 ) {
		BotDrawDebugPolygons( drawPoly, cv2->integer );
		return;
	}
#endif

	if ( !debugPatchCollide ) {
		return;
	}

#ifndef BSPC
	if ( !cv ) {
		cv = Cvar_Get( "cm_debugSize", "2", 0 );
	}
#endif
	pc = debugPatchCollide;

	for ( i = 0, facet = pc->facets ; i < pc->numFacets ; i++, facet++ ) {

		for ( k = 0 ; k < facet->numBorders + 1; k++ ) {
			//
			if ( k < facet->numBorders ) {
				planenum = facet->borderPlanes[k];
				inward = facet->borderInward[k];
			} else {
				planenum = facet->surfacePlane;
				inward = qfalse;
				//continue;
			}

			plane = pc->planes[ planenum ].plane;

			//planenum = facet->surfacePlane;
			if ( inward ) {
				plane = -plane;
			}

			plane.SetDist( plane.Dist() + cv->value );
			//*
			for ( n = 0; n < 3; n++ )
			{
				if ( plane.Normal()[n] > 0 ) {
					v1[n] = maxs[n];
				} else { v1[n] = mins[n];}
			} //end for
			v2 = -plane.Normal();
			plane.SetDist( plane.Dist() + fabs( v1 * v2 ) );
			//*/

			w = BaseWindingForPlane( plane.Normal(), plane.Dist() );
			for ( j = 0 ; j < facet->numBorders + 1 && w; j++ ) {
				//
				if ( j < facet->numBorders ) {
					curplanenum = facet->borderPlanes[j];
					curinward = facet->borderInward[j];
				} else {
					curplanenum = facet->surfacePlane;
					curinward = qfalse;
					//continue;
				}
				//
				if ( curplanenum == planenum ) {
					continue;
				}

				plane = pc->planes[ curplanenum ].plane;
				if ( !curinward ) {
					plane = -plane;
				}
				//			if ( !facet->borderNoAdjust[j] ) {
				plane.SetDist( plane.Dist() - cv->value );
				//			}
				for ( n = 0; n < 3; n++ )
				{
					if ( plane.Normal()[n] > 0 ) {
						v1[n] = maxs[n];
					} else { v1[n] = mins[n];}
				} //end for
				v2 = -plane.Normal();
				plane.SetDist( plane.Dist() - fabs( v1 * v2 ) );

				ChopWindingInPlace( &w, plane.Normal(), plane.Dist(), 0.1f );
			}
			if ( w ) {
				if ( facet == debugFacet ) {
					drawPoly( 4, w->numpoints, w->p[0].ToFloatPtr() );
					//Com_Printf("blue facet has %d border planes\n", facet->numBorders);
				} else {
					drawPoly( 1, w->numpoints, w->p[0].ToFloatPtr() );
				}
				FreeWinding( w );
			} else {
				Com_Printf( "winding chopped away by border planes\n" );
			}
		}
	}

	// draw the debug block
	{
		idVec3 v[3];

		v[0] = debugBlockPoints[0];
		v[1] = debugBlockPoints[1];
		v[2] = debugBlockPoints[2];
		drawPoly( 2, 3, v[0].ToFloatPtr() );

		v[0] = debugBlockPoints[2];
		v[1] = debugBlockPoints[3];
		v[2] = debugBlockPoints[0];
		drawPoly( 2, 3, v[0].ToFloatPtr() );
	}

#if 0
	idVec3 v[4];

	v[0][0] = pc->bounds[1][0];
	v[0][1] = pc->bounds[1][1];
	v[0][2] = pc->bounds[1][2];

	v[1][0] = pc->bounds[1][0];
	v[1][1] = pc->bounds[0][1];
	v[1][2] = pc->bounds[1][2];

	v[2][0] = pc->bounds[0][0];
	v[2][1] = pc->bounds[0][1];
	v[2][2] = pc->bounds[1][2];

	v[3][0] = pc->bounds[0][0];
	v[3][1] = pc->bounds[1][1];
	v[3][2] = pc->bounds[1][2];

	drawPoly( 4, v[0] );
#endif
}

