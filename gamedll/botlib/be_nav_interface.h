/*
===========================================================================

IceWolf GPL Source Code
Copyright (C) 2026 Justin Marshall (justinmarshall20@gmail.com and git.retrodamage.com)

This file is part of the IceWolf.

IceWolf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

IceWolf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with IceWolf.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#pragma once

#ifndef MAX_EPAIRKEY
#define MAX_EPAIRKEY 128
#endif

int AAS_Setup( void );
void AAS_Shutdown( void );
int AAS_LoadMap( const char *mapname );
int AAS_StartFrame( float time );
int AAS_Initialized( void );
int AAS_Loaded( void );
float AAS_Time( void );
void AAS_SetCurrentWorld( int index );
char *AAS_ModelFromIndex( int index );
int AAS_IndexFromModel( char *modelname );

int AAS_UpdateEntity( int entnum, bot_entitystate_t *state );
void AAS_EntityInfo( int entnum, aas_entityinfo_t *info );
int AAS_NextEntity( int entnum );
int AAS_EntityType( int entnum );
int AAS_EntityModelindex( int entnum );
int AAS_EntityModelNum( int entnum );
int AAS_OriginOfEntityWithModelNum( int modelnum, idVec3 &origin );
int AAS_IsEntityInArea( int entnumIgnore, int entnumIgnore2, int areanum );

bsp_trace_t AAS_Trace( const idVec3 &start, const idVec3 *mins, const idVec3 *maxs, const idVec3 &end, int passent, int contentmask );
int AAS_PointContents( const idVec3 &point );
qboolean AAS_inPVS( const idVec3 &p1, const idVec3 &p2 );
qboolean AAS_AreasConnected( int area1, int area2 );
void AAS_BSPModelMinsMaxsOrigin( int modelnum, const idVec3 &angles, idVec3 &mins, idVec3 &maxs, idVec3 *origin );
int AAS_NextBSPEntity( int ent );
int AAS_ValueForBSPEpairKey( int ent, char *key, char *value, int size );
int AAS_VectorForBSPEpairKey( int ent, char *key, idVec3 &v );
int AAS_FloatForBSPEpairKey( int ent, char *key, float *value );
int AAS_IntForBSPEpairKey( int ent, char *key, int *value );

void AAS_PresenceTypeBoundingBox( int presencetype, idVec3 &mins, idVec3 &maxs );
int AAS_PointAreaNum( const idVec3 &point );
int AAS_TraceAreas( const idVec3 &start, const idVec3 &end, int *areas, idVec3 *points, int maxareas );
int AAS_AreaCluster( int areanum );
int AAS_AreaPresenceType( int areanum );
int AAS_PointPresenceType( const idVec3 &point );
int AAS_AreaReachability( int areanum );
int AAS_BestReachableArea( const idVec3 &origin, const idVec3 &mins, const idVec3 &maxs, idVec3 &goalorigin );
int AAS_NextModelReachability( int num, int modelnum );
int AAS_AreaLava( int areanum );
int AAS_AreaSlime( int areanum );
int AAS_AreaJumpPad( int areanum );
int AAS_AreaDoNotEnter( int areanum );
int AAS_AreaDoNotEnterLarge( int areanum );

int AAS_TravelFlagForType( int traveltype );
int AAS_AreaContentsTravelFlag( int areanum );
int AAS_NextAreaReachability( int areanum, int reachnum );
void AAS_ReachabilityFromNum( int num, aas_reachability_t *reach );
int AAS_AreaRouteToGoalArea( int areanum, const idVec3 &origin, int goalareanum, int travelflags, int *traveltime, int *reachnum );
int AAS_AreaTravelTimeToGoalArea( int areanum, const idVec3 &origin, int goalareanum, int travelflags );
int AAS_AreaTravelTimeToGoalAreaCheckLoop( int areanum, const idVec3 &origin, int goalareanum, int travelflags, int loopareanum );
unsigned short AAS_AreaTravelTime( int areanum, const idVec3 &start, const idVec3 &end );
int AAS_RandomGoalArea( int areanum, int travelflags, int *goalareanum, idVec3 &goalorigin );

int AAS_OnGround( const idVec3 &origin, int presencetype, int passent );
int AAS_Swimming( const idVec3 &origin );
void AAS_JumpReachRunStart( aas_reachability_t *reach, idVec3 &runstart );
int AAS_AgainstLadder( const idVec3 &origin, int ms_areanum );
int AAS_HorizontalVelocityForJump( float zvel, const idVec3 &start, const idVec3 &end, float *velocity );
int AAS_DropToFloor( idVec3 &origin, const idVec3 &mins, const idVec3 &maxs );
int AAS_PredictClientMovement( aas_clientmove_t *move, int entnum, const idVec3 &origin, int presencetype, int onground,
	const idVec3 &velocity, const idVec3 &cmdmove, int cmdframes, int maxframes, float frametime,
	int stopevent, int stopareanum, int visualize );
aas_trace_t AAS_TraceClientBBox( const idVec3 &start, const idVec3 &end, int presencetype, int passent );

void AAS_SetAASBlockingEntity( const idVec3 &absmin, const idVec3 &absmax, qboolean blocking );
void AAS_RT_ShowRoute( const idVec3 &srcpos, int srcnum, int destnum );
qboolean AAS_RT_GetHidePos( const idVec3 &srcpos, int srcnum, int srcarea, const idVec3 &destpos, int destnum, int destarea, idVec3 &returnPos );
int AAS_FindAttackSpotWithinRange( int srcnum, int rangenum, int enemynum, float rangedist, int travelflags, float *outpos );
qboolean AAS_GetRouteFirstVisPos( const idVec3 &srcpos, const idVec3 &destpos, int travelflags, idVec3 &retpos );

void AAS_ClearShownDebugLines( void );
void AAS_ClearShownPolygons( void );
void AAS_DebugLine( const idVec3 &start, const idVec3 &end, int color );
void AAS_DrawCross( const idVec3 &origin, float size, int color );
void AAS_DrawPermanentCross( const idVec3 &origin, float size, int color );
void AAS_DrawPlaneCross( const idVec3 &point, const idVec3 &normal, float dist, int type, int color );
void AAS_ShowBoundingBox( const idVec3 &origin, const idVec3 &mins, const idVec3 &maxs );
void AAS_ShowArea( int areanum, int groundfacesonly );
void AAS_ShowAreaPolygons( int areanum, int color, int groundfacesonly );
void AAS_ShowReachability( aas_reachability_t *reach );
void AAS_ShowReachableAreas( int areanum );
void AAS_PrintTravelType( int traveltype );
