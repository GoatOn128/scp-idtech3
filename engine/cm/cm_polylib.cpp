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


// this is only used for visualization tools in cm_ debug functions


#include "cm_local.h"


// counters are only bumped when running single threaded,
// because they are an awefull coherence problem
int c_active_windings;
int c_peak_windings;
int c_winding_allocs;
int c_winding_points;

void pw( winding_t *w ) {
	int i;
	for ( i = 0 ; i < w->numpoints ; i++ )
		printf( "(%5.1f, %5.1f, %5.1f)\n",w->p[i][0], w->p[i][1],w->p[i][2] );
}


/*
=============
AllocWinding
=============
*/
winding_t   *AllocWinding( int points ) {
	winding_t   *w;
	int s;

	c_winding_allocs++;
	c_winding_points += points;
	c_active_windings++;
	if ( c_active_windings > c_peak_windings ) {
		c_peak_windings = c_active_windings;
	}

	s = sizeof( float ) * 3 * points + sizeof( int );
	w = (winding_t *) Mem_ClearedAlloc( s, TAG_IDLIB );
	memset( w, 0, s );
	return w;
}

void FreeWinding( winding_t *w ) {
	if ( *(unsigned *)w == 0xdeaddead ) {
		Com_Error( ERR_FATAL, "FreeWinding: freed a freed winding" );
	}
	*(unsigned *)w = 0xdeaddead;

	c_active_windings--;
	Mem_Free( w );
}

/*
============
RemoveColinearPoints
============
*/
int c_removed;

void    RemoveColinearPoints( winding_t *w ) {
	int i, j, k;
	idVec3 v1, v2;
	int nump;
	idVec3 p[MAX_POINTS_ON_WINDING];

	nump = 0;
	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		j = ( i + 1 ) % w->numpoints;
		k = ( i + w->numpoints - 1 ) % w->numpoints;
		( v1 = w->p[j] - w->p[i] );
		( v2 = w->p[i] - w->p[k] );
		( (v1 = v1), v1.Normalize() );
		( (v2 = v2), v2.Normalize() );
		if ( ( v1 * v2 ) < 0.999 ) {
			( p[nump] = w->p[i] );
			nump++;
		}
	}

	if ( nump == w->numpoints ) {
		return;
	}

	c_removed += w->numpoints - nump;
	w->numpoints = nump;
	memcpy( w->p, p, nump * sizeof( p[0] ) );
}

/*
============
WindingPlane
============
*/
void WindingPlane( winding_t *w, idVec3 normal, float *dist ) {
	idVec3 v1, v2;

	( v1 = w->p[1] - w->p[0] );
	( v2 = w->p[2] - w->p[0] );
	( normal.Cross( v2, v1 ) );
	( (normal = normal), normal.Normalize() );
	*dist = ( w->p[0] * normal );

}

/*
=============
WindingArea
=============
*/
float   WindingArea( winding_t *w ) {
	int i;
	idVec3 d1, d2, cross;
	float total;

	total = 0;
	for ( i = 2 ; i < w->numpoints ; i++ )
	{
		( d1 = w->p[i - 1] - w->p[0] );
		( d2 = w->p[i] - w->p[0] );
		( cross.Cross( d1, d2 ) );
		total += 0.5 * ( cross.Length() );
	}
	return total;
}

/*
=============
WindingBounds
=============
*/
void    WindingBounds( winding_t *w, idVec3 mins, idVec3 maxs ) {
	float v;
	int i,j;

	mins[0] = mins[1] = mins[2] = MAX_MAP_BOUNDS;
	maxs[0] = maxs[1] = maxs[2] = -MAX_MAP_BOUNDS;

	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		for ( j = 0 ; j < 3 ; j++ )
		{
			v = w->p[i][j];
			if ( v < mins[j] ) {
				mins[j] = v;
			}
			if ( v > maxs[j] ) {
				maxs[j] = v;
			}
		}
	}
}

/*
=============
WindingCenter
=============
*/
void    WindingCenter( winding_t *w, idVec3 center ) {
	int i;
	float scale;

	( center = vec3_origin );
	for ( i = 0 ; i < w->numpoints ; i++ )
		( center = w->p[i] + center );

	scale = 1.0 / w->numpoints;
	( center = center * scale );
}

/*
=================
BaseWindingForPlane
=================
*/
winding_t *BaseWindingForPlane( idVec3 normal, float dist ) {
	int i, x;
	float max, v;
	idVec3 org, vright, vup;
	winding_t   *w;

// find the major axis

	max = -MAX_MAP_BOUNDS;
	x = -1;
	for ( i = 0 ; i < 3; i++ )
	{
		v = fabs( normal[i] );
		if ( v > max ) {
			x = i;
			max = v;
		}
	}
	if ( x == -1 ) {
		Com_Error( ERR_DROP, "BaseWindingForPlane: no axis found" );
	}

	( vup = vec3_origin );
	switch ( x )
	{
	case 0:
	case 1:
		vup[2] = 1;
		break;
	case 2:
		vup[0] = 1;
		break;
	}

	v = ( vup * normal );
	( vup = vup + normal * -v );
	( (vup = vup), vup.Normalize() );

	( org = normal * dist );

	( vright.Cross( vup, normal ) );

	( vup = vup * MAX_MAP_BOUNDS );
	( vright = vright * MAX_MAP_BOUNDS );

// project a really big	axis aligned box onto the plane
	w = AllocWinding( 4 );

	( w->p[0] = org - vright );
	( w->p[0] = w->p[0] + vup );

	( w->p[1] = org + vright );
	( w->p[1] = w->p[1] + vup );

	( w->p[2] = org + vright );
	( w->p[2] = w->p[2] - vup );

	( w->p[3] = org - vright );
	( w->p[3] = w->p[3] - vup );

	w->numpoints = 4;

	return w;
}

/*
==================
CopyWinding
==================
*/
winding_t   *CopyWinding( winding_t *w ) {
	int size;
	winding_t   *c;

	c = AllocWinding( w->numpoints );
	size = (int)(intptr_t)( (winding_t *)0 )->p[w->numpoints];
	memcpy( c, w, size );
	return c;
}

/*
==================
ReverseWinding
==================
*/
winding_t   *ReverseWinding( winding_t *w ) {
	int i;
	winding_t   *c;

	c = AllocWinding( w->numpoints );
	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		( c->p[i] = w->p[w->numpoints - 1 - i] );
	}
	c->numpoints = w->numpoints;
	return c;
}


/*
=============
ClipWindingEpsilon
=============
*/
void    ClipWindingEpsilon( winding_t *in, idVec3 normal, float dist,
							float epsilon, winding_t **front, winding_t **back ) {
	float dists[MAX_POINTS_ON_WINDING + 4];
	int sides[MAX_POINTS_ON_WINDING + 4];
	int counts[3];
	static float dot;           // VC 4.2 optimizer bug if not static
	int i, j;
	float   *p1, *p2;
	idVec3 mid;
	winding_t   *f, *b;
	int maxpts;

	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	for ( i = 0 ; i < in->numpoints ; i++ )
	{
		dot = ( in->p[i] * normal );
		dot -= dist;
		dists[i] = dot;
		if ( dot > epsilon ) {
			sides[i] = SIDE_FRONT;
		} else if ( dot < -epsilon ) {
			sides[i] = SIDE_BACK;
		} else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	*front = *back = NULL;

	if ( !counts[0] ) {
		*back = CopyWinding( in );
		return;
	}
	if ( !counts[1] ) {
		*front = CopyWinding( in );
		return;
	}

	maxpts = in->numpoints + 4;   // cant use counts[0]+2 because
								  // of fp grouping errors

	*front = f = AllocWinding( maxpts );
	*back = b = AllocWinding( maxpts );

	for ( i = 0 ; i < in->numpoints ; i++ )
	{
		p1 = in->p[i];

		if ( sides[i] == SIDE_ON ) {
			( f->p[f->numpoints] = p1 );
			f->numpoints++;
			( b->p[b->numpoints] = p1 );
			b->numpoints++;
			continue;
		}

		if ( sides[i] == SIDE_FRONT ) {
			( f->p[f->numpoints] = p1 );
			f->numpoints++;
		}
		if ( sides[i] == SIDE_BACK ) {
			( b->p[b->numpoints] = p1 );
			b->numpoints++;
		}

		if ( sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i] ) {
			continue;
		}

		// generate a split point
		p2 = in->p[( i + 1 ) % in->numpoints];

		dot = dists[i] / ( dists[i] - dists[i + 1] );
		for ( j = 0 ; j < 3 ; j++ )
		{   // avoid round off error when possible
			if ( normal[j] == 1 ) {
				mid[j] = dist;
			} else if ( normal[j] == -1 ) {
				mid[j] = -dist;
			} else {
				mid[j] = p1[j] + dot * ( p2[j] - p1[j] );
			}
		}

		( f->p[f->numpoints] = mid );
		f->numpoints++;
		( b->p[b->numpoints] = mid );
		b->numpoints++;
	}

	if ( f->numpoints > maxpts || b->numpoints > maxpts ) {
		Com_Error( ERR_DROP, "ClipWinding: points exceeded estimate" );
	}
	if ( f->numpoints > MAX_POINTS_ON_WINDING || b->numpoints > MAX_POINTS_ON_WINDING ) {
		Com_Error( ERR_DROP, "ClipWinding: MAX_POINTS_ON_WINDING" );
	}
}


/*
=============
ChopWindingInPlace
=============
*/
void ChopWindingInPlace( winding_t **inout, idVec3 normal, float dist, float epsilon ) {
	winding_t   *in;
	float dists[MAX_POINTS_ON_WINDING + 4];
	int sides[MAX_POINTS_ON_WINDING + 4];
	int counts[3];
	static float dot;           // VC 4.2 optimizer bug if not static
	int i, j;
	float   *p1, *p2;
	idVec3 mid;
	winding_t   *f;
	int maxpts;

	in = *inout;
	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	for ( i = 0 ; i < in->numpoints ; i++ )
	{
		dot = ( in->p[i] * normal );
		dot -= dist;
		dists[i] = dot;
		if ( dot > epsilon ) {
			sides[i] = SIDE_FRONT;
		} else if ( dot < -epsilon ) {
			sides[i] = SIDE_BACK;
		} else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	if ( !counts[0] ) {
		FreeWinding( in );
		*inout = NULL;
		return;
	}
	if ( !counts[1] ) {
		return;     // inout stays the same

	}
	maxpts = in->numpoints + 4;   // cant use counts[0]+2 because
								  // of fp grouping errors

	f = AllocWinding( maxpts );

	for ( i = 0 ; i < in->numpoints ; i++ )
	{
		p1 = in->p[i];

		if ( sides[i] == SIDE_ON ) {
			( f->p[f->numpoints] = p1 );
			f->numpoints++;
			continue;
		}

		if ( sides[i] == SIDE_FRONT ) {
			( f->p[f->numpoints] = p1 );
			f->numpoints++;
		}

		if ( sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i] ) {
			continue;
		}

		// generate a split point
		p2 = in->p[( i + 1 ) % in->numpoints];

		dot = dists[i] / ( dists[i] - dists[i + 1] );
		for ( j = 0 ; j < 3 ; j++ )
		{   // avoid round off error when possible
			if ( normal[j] == 1 ) {
				mid[j] = dist;
			} else if ( normal[j] == -1 ) {
				mid[j] = -dist;
			} else {
				mid[j] = p1[j] + dot * ( p2[j] - p1[j] );
			}
		}

		( f->p[f->numpoints] = mid );
		f->numpoints++;
	}

	if ( f->numpoints > maxpts ) {
		Com_Error( ERR_DROP, "ClipWinding: points exceeded estimate" );
	}
	if ( f->numpoints > MAX_POINTS_ON_WINDING ) {
		Com_Error( ERR_DROP, "ClipWinding: MAX_POINTS_ON_WINDING" );
	}

	FreeWinding( in );
	*inout = f;
}


/*
=================
ChopWinding

Returns the fragment of in that is on the front side
of the cliping plane.  The original is freed.
=================
*/
winding_t   *ChopWinding( winding_t *in, idVec3 normal, float dist ) {
	winding_t   *f, *b;

	ClipWindingEpsilon( in, normal, dist, ON_EPSILON, &f, &b );
	FreeWinding( in );
	if ( b ) {
		FreeWinding( b );
	}
	return f;
}


/*
=================
CheckWinding

=================
*/
void CheckWinding( winding_t *w ) {
	int i, j;
	float   *p1, *p2;
	float d, edgedist;
	idVec3 dir, edgenormal, facenormal;
	float area;
	float facedist;

	if ( w->numpoints < 3 ) {
		Com_Error( ERR_DROP, "CheckWinding: %i points",w->numpoints );
	}

	area = WindingArea( w );
	if ( area < 1 ) {
		Com_Error( ERR_DROP, "CheckWinding: %f area", area );
	}

	WindingPlane( w, facenormal, &facedist );

	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		p1 = w->p[i];

		for ( j = 0 ; j < 3 ; j++ )
			if ( p1[j] > MAX_MAP_BOUNDS || p1[j] < -MAX_MAP_BOUNDS ) {
				Com_Error( ERR_DROP, "CheckFace: BUGUS_RANGE: %f",p1[j] );
			}

		j = i + 1 == w->numpoints ? 0 : i + 1;

		// check the point is on the face plane
		d = ( p1 * facenormal ) - facedist;
		if ( d < -ON_EPSILON || d > ON_EPSILON ) {
			Com_Error( ERR_DROP, "CheckWinding: point off plane" );
		}

		// check the edge isnt degenerate
		p2 = w->p[j];
		( dir = p2 - p1 );

		if ( ( dir.Length() ) < ON_EPSILON ) {
			Com_Error( ERR_DROP, "CheckWinding: degenerate edge" );
		}

		( edgenormal.Cross( facenormal, dir ) );
		( (edgenormal = edgenormal), edgenormal.Normalize() );
		edgedist = ( p1 * edgenormal );
		edgedist += ON_EPSILON;

		// all other points must be on front side
		for ( j = 0 ; j < w->numpoints ; j++ )
		{
			if ( j == i ) {
				continue;
			}
			d = ( w->p[j] * edgenormal );
			if ( d > edgedist ) {
				Com_Error( ERR_DROP, "CheckWinding: non-convex" );
			}
		}
	}
}


/*
============
WindingOnPlaneSide
============
*/
int     WindingOnPlaneSide( winding_t *w, idVec3 normal, float dist ) {
	qboolean front, back;
	int i;
	float d;

	front = qfalse;
	back = qfalse;
	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		d = ( w->p[i] * normal ) - dist;
		if ( d < -ON_EPSILON ) {
			if ( front ) {
				return SIDE_CROSS;
			}
			back = qtrue;
			continue;
		}
		if ( d > ON_EPSILON ) {
			if ( back ) {
				return SIDE_CROSS;
			}
			front = qtrue;
			continue;
		}
	}

	if ( back ) {
		return SIDE_BACK;
	}
	if ( front ) {
		return SIDE_FRONT;
	}
	return SIDE_ON;
}


/*
=================
AddWindingToConvexHull

Both w and *hull are on the same plane
=================
*/
#define MAX_HULL_POINTS     128
void    AddWindingToConvexHull( winding_t *w, winding_t **hull, idVec3 normal ) {
	int i, j, k;
	float       *p, *copy;
	idVec3 dir;
	float d;
	int numHullPoints, numNew;
	idVec3 hullPoints[MAX_HULL_POINTS];
	idVec3 newHullPoints[MAX_HULL_POINTS];
	idVec3 hullDirs[MAX_HULL_POINTS];
	qboolean hullSide[MAX_HULL_POINTS];
	qboolean outside;

	if ( !*hull ) {
		*hull = CopyWinding( w );
		return;
	}

	numHullPoints = ( *hull )->numpoints;
	memcpy( hullPoints, ( *hull )->p, numHullPoints * sizeof( idVec3 ) );

	for ( i = 0 ; i < w->numpoints ; i++ ) {
		p = w->p[i];

		// calculate hull side vectors
		for ( j = 0 ; j < numHullPoints ; j++ ) {
			k = ( j + 1 ) % numHullPoints;

			( dir = hullPoints[k] - hullPoints[j] );
			( (dir = dir), dir.Normalize() );
			( hullDirs[j].Cross( normal, dir ) );
		}

		outside = qfalse;
		for ( j = 0 ; j < numHullPoints ; j++ ) {
			( dir = p - hullPoints[j] );
			d = ( dir * hullDirs[j] );
			if ( d >= ON_EPSILON ) {
				outside = qtrue;
			}
			if ( d >= -ON_EPSILON ) {
				hullSide[j] = qtrue;
			} else {
				hullSide[j] = qfalse;
			}
		}

		// if the point is effectively inside, do nothing
		if ( !outside ) {
			continue;
		}

		// find the back side to front side transition
		for ( j = 0 ; j < numHullPoints ; j++ ) {
			if ( !hullSide[ j % numHullPoints ] && hullSide[ ( j + 1 ) % numHullPoints ] ) {
				break;
			}
		}
		if ( j == numHullPoints ) {
			continue;
		}

		// insert the point here
		( newHullPoints[0] = p );
		numNew = 1;

		// copy over all points that aren't double fronts
		j = ( j + 1 ) % numHullPoints;
		for ( k = 0 ; k < numHullPoints ; k++ ) {
			if ( hullSide[ ( j + k ) % numHullPoints ] && hullSide[ ( j + k + 1 ) % numHullPoints ] ) {
				continue;
			}
			copy = hullPoints[ ( j + k + 1 ) % numHullPoints ];
			( newHullPoints[numNew] = copy );
			numNew++;
		}

		numHullPoints = numNew;
		memcpy( hullPoints, newHullPoints, numHullPoints * sizeof( idVec3 ) );
	}

	FreeWinding( *hull );
	w = AllocWinding( numHullPoints );
	w->numpoints = numHullPoints;
	*hull = w;
	memcpy( w->p, hullPoints, numHullPoints * sizeof( idVec3 ) );
}


