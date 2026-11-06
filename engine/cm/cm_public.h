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


#include "../qcommon/qfiles.h"

#include "../renderer/tr_types.h"

void        CM_LoadMap( const char *name, qboolean clientload, int *checksum );
clipHandle_t CM_InlineModel( int index );       // 0 = world, 1 + are bmodels
clipHandle_t CM_TempBoxModel( const idVec3 mins, const idVec3 maxs, int capsule );

void        CM_ModelBounds( clipHandle_t model, idVec3 &mins, idVec3 &maxs );

int         CM_NumClusters( void );
int         CM_NumInlineModels( void );
char        *CM_EntityString( void );

// returns an ORed contents mask
int         CM_PointContents( const idVec3 p, clipHandle_t model );
int         CM_TransformedPointContents( const idVec3 p, clipHandle_t model, const idVec3 origin, const idVec3 angles );

void        CM_BoxTrace( trace_t *results, const idVec3 start, const idVec3 end,
						 const idVec3 mins, const idVec3 maxs,
						 clipHandle_t model, int brushmask, int capsule );
void        CM_TransformedBoxTrace( trace_t *results, const idVec3 start, const idVec3 end,
									const idVec3 mins, const idVec3 maxs,
									clipHandle_t model, int brushmask,
									const idVec3 origin, const idVec3 angles, int capsule );

byte        *CM_ClusterPVS( int cluster );

int         CM_PointLeafnum( const idVec3 p );

// only returns non-solid leafs
// overflow if return listsize and if *lastLeaf != list[listsize-1]
int         CM_BoxLeafnums( const idVec3 mins, const idVec3 maxs, int *list,
							int listsize, int *lastLeaf );

int         CM_LeafCluster( int leafnum );
int         CM_LeafArea( int leafnum );

void        CM_AdjustAreaPortalState( int area1, int area2, qboolean open );
qboolean    CM_AreasConnected( int area1, int area2 );

int         CM_WriteAreaBits( byte *buffer, int area );

// cm_tag.c
int         CM_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex );


// cm_marks.c
int CM_MarkFragments( int numPoints, const idVec3 *points, const idVec3 &projection,
					  int maxPoints, idVec5 *pointBuffer, int maxFragments, markFragment_t *fragmentBuffer );

// cm_patch.c
void CM_DrawDebugSurface( void ( *drawPoly )( int color, int numPoints, float *points ) );
