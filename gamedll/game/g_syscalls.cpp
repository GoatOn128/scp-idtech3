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
#include "g_local.h"

// Native module bindings supplied through the typed game import table.

static const gameImport_t *gameImports;

void G_SetImports( const gameImport_t *imports ) {
	gameImports = imports;
}


void trap_Printf( const char *fmt ) {
	gameImports->Com_Printf( fmt );
}

void trap_Error( const char *fmt ) {
	gameImports->Com_Error( fmt );
}

void trap_Endgame( void ) {
	gameImports->Com_ErrorEndGame(  );
}

int trap_Milliseconds( void ) {
	return gameImports->Sys_Milliseconds(  );
}
int trap_Argc( void ) {
	return gameImports->Cmd_Argc(  );
}

void trap_Argv( int n, char *buffer, int bufferLength ) {
	gameImports->Cmd_ArgvBuffer( n, buffer, bufferLength );
}

int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return gameImports->FS_FOpenFileByMode( qpath, f, mode );
}

void trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	gameImports->FS_Read( buffer, len, f );
}

int trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	return gameImports->FS_Write( buffer, len, f );
}

int trap_FS_Rename( const char *from, const char *to ) {
	return gameImports->FS_Rename( from, to );
}

void trap_FS_FCloseFile( fileHandle_t f ) {
	gameImports->FS_FCloseFile( f );
}

void trap_FS_CopyFile( char *from, char *to ) {
	gameImports->FS_CopyFileOS( from, to );
}

int trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize ) {
	return gameImports->FS_GetFileList( path, extension, listbuf, bufsize );
}

void trap_SendConsoleCommand( int exec_when, const char *text ) {
	gameImports->Cbuf_ExecuteText( exec_when, text );
}

void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	gameImports->Cvar_Register( cvar, var_name, value, flags );
}

void trap_Cvar_Update( vmCvar_t *cvar ) {
	gameImports->Cvar_Update( cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	gameImports->Cvar_Set( var_name, value );
}

int trap_Cvar_VariableIntegerValue( const char *var_name ) {
	return gameImports->Cvar_VariableIntegerValue( var_name );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	gameImports->Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}


void trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						  playerState_t *clients, int sizeofGClient ) {
	gameImports->SV_LocateGameData( (sharedEntity_t *)gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient );
}

void trap_DropClient( int clientNum, const char *reason ) {
	gameImports->SV_GameDropClient( clientNum, reason );
}

void trap_SendServerCommand( int clientNum, const char *text ) {
	gameImports->SV_GameSendServerCommand( clientNum, text );
}

void trap_SetConfigstring( int num, const char *string ) {
	gameImports->SV_SetConfigstring( num, string );
}

void trap_GetConfigstring( int num, char *buffer, int bufferSize ) {
	gameImports->SV_GetConfigstring( num, buffer, bufferSize );
}

void trap_GetUserinfo( int num, char *buffer, int bufferSize ) {
	gameImports->SV_GetUserinfo( num, buffer, bufferSize );
}

void trap_SetUserinfo( int num, const char *buffer ) {
	gameImports->SV_SetUserinfo( num, buffer );
}

void trap_GetServerinfo( char *buffer, int bufferSize ) {
	gameImports->SV_GetServerinfo( buffer, bufferSize );
}

void trap_SetBrushModel( gentity_t *ent, const char *name ) {
	gameImports->SV_SetBrushModel( ent, name );
}

void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	gameImports->SV_Trace( results, start, mins, maxs, end, passEntityNum, contentmask );
}

void trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	gameImports->SV_TraceCapsule( results, start, mins, maxs, end, passEntityNum, contentmask );
}

int trap_PointContents( const vec3_t point, int passEntityNum ) {
	return gameImports->SV_PointContents( point, passEntityNum );
}


qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 ) {
	return gameImports->SV_inPVS( p1, p2 );
}

qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {
	return gameImports->SV_inPVSIgnorePortals( p1, p2 );
}

void trap_AdjustAreaPortalState( gentity_t *ent, qboolean open ) {
	gameImports->SV_AdjustAreaPortalState( ent, open );
}

qboolean trap_AreasConnected( int area1, int area2 ) {
	return gameImports->CM_AreasConnected( area1, area2 );
}

void trap_LinkEntity( gentity_t *ent ) {
	gameImports->SV_LinkEntity( ent );
}

void trap_UnlinkEntity( gentity_t *ent ) {
	gameImports->SV_UnlinkEntity( ent );
}


int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *list, int maxcount ) {
	return gameImports->SV_AreaEntities( mins, maxs, list, maxcount );
}

qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return gameImports->SV_EntityContact( mins, maxs, ent );
}

qboolean trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return gameImports->SV_EntityContactCapsule( mins, maxs, ent );
}

int trap_BotAllocateClient( void ) {
	return gameImports->SV_BotAllocateClient(  );
}

void trap_BotFreeClient( int clientNum ) {
	gameImports->SV_BotFreeClient( clientNum );
}

void trap_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	gameImports->SV_GetUsercmd( clientNum, cmd );
}

qboolean trap_GetEntityToken( char *buffer, int bufferSize ) {
	return gameImports->SV_GetEntityToken( buffer, bufferSize );
}

int trap_DebugPolygonCreate( int color, int numPoints, vec3_t *points ) {
	return gameImports->BotImport_DebugPolygonCreate( color, numPoints, points );
}

void trap_DebugPolygonDelete( int id ) {
	gameImports->BotImport_DebugPolygonDelete( id );
}

int trap_RealTime( qtime_t *qtime ) {
	return gameImports->Com_RealTime( qtime );
}

void trap_SnapVector( float *v ) {
	gameImports->Sys_SnapVector( v );
}

qboolean trap_GetTag( int clientNum, char *tagName, orientation_t *or ) {
	return gameImports->SV_GetTag( clientNum, tagName, or );
}

// BotLib traps start here
int trap_BotLibSetup( void ) {
	return gameImports->SV_BotLibSetup(  );
}

int trap_BotLibShutdown( void ) {
	return gameImports->SV_BotLibShutdown(  );
}

int trap_BotLibVarSet( char *var_name, char *value ) {
	return gameImports->BotLibVarSet( var_name, value );
}

int trap_BotLibVarGet( char *var_name, char *value, int size ) {
	return gameImports->BotLibVarGet( var_name, value, size );
}

int trap_BotLibDefine( char *string ) {
	return gameImports->PC_AddGlobalDefine( string );
}

int trap_BotLibStartFrame( float time ) {
	return gameImports->BotLibStartFrame( time );
}

int trap_BotLibLoadMap( const char *mapname ) {
	return gameImports->BotLibLoadMap( mapname );
}

int trap_BotLibUpdateEntity( int ent, struct bot_entitystate_s *bue ) {
	return gameImports->BotLibUpdateEntity( ent, bue );
}

int trap_BotLibTest( int parm0, char *parm1, vec3_t parm2, vec3_t parm3 ) {
	return gameImports->Test( parm0, parm1, parm2, parm3 );
}

int trap_BotGetSnapshotEntity( int clientNum, int sequence ) {
	return gameImports->SV_BotGetSnapshotEntity( clientNum, sequence );
}

int trap_BotGetServerCommand( int clientNum, char *message, int size ) {
	return gameImports->SV_BotGetConsoleMessage( clientNum, message, size );
}

void trap_BotUserCommand( int clientNum, usercmd_t *ucmd ) {
	gameImports->SV_ClientThink( clientNum, ucmd );
}

void trap_AAS_EntityInfo( int entnum, struct aas_entityinfo_s *info ) {
	gameImports->AAS_EntityInfo( entnum, info );
}

int trap_AAS_Initialized( void ) {
	return gameImports->AAS_Initialized(  );
}

void trap_AAS_PresenceTypeBoundingBox( int presencetype, vec3_t mins, vec3_t maxs ) {
	gameImports->AAS_PresenceTypeBoundingBox( presencetype, mins, maxs );
}

float trap_AAS_Time( void ) {
	return gameImports->AAS_Time(  );
}

// Ridah, multiple AAS files
void trap_AAS_SetCurrentWorld( int index ) {
	gameImports->AAS_SetCurrentWorld( index );
}
// done.

int trap_AAS_PointAreaNum( vec3_t point ) {
	return gameImports->AAS_PointAreaNum( point );
}

int trap_AAS_TraceAreas( vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas ) {
	return gameImports->AAS_TraceAreas( start, end, areas, points, maxareas );
}

int trap_AAS_PointContents( vec3_t point ) {
	return gameImports->AAS_PointContents( point );
}

int trap_AAS_NextBSPEntity( int ent ) {
	return gameImports->AAS_NextBSPEntity( ent );
}

int trap_AAS_ValueForBSPEpairKey( int ent, char *key, char *value, int size ) {
	return gameImports->AAS_ValueForBSPEpairKey( ent, key, value, size );
}

int trap_AAS_VectorForBSPEpairKey( int ent, char *key, vec3_t v ) {
	return gameImports->AAS_VectorForBSPEpairKey( ent, key, v );
}

int trap_AAS_FloatForBSPEpairKey( int ent, char *key, float *value ) {
	return gameImports->AAS_FloatForBSPEpairKey( ent, key, value );
}

int trap_AAS_IntForBSPEpairKey( int ent, char *key, int *value ) {
	return gameImports->AAS_IntForBSPEpairKey( ent, key, value );
}

int trap_AAS_AreaReachability( int areanum ) {
	return gameImports->AAS_AreaReachability( areanum );
}

int trap_AAS_AreaTravelTimeToGoalArea( int areanum, vec3_t origin, int goalareanum, int travelflags ) {
	return gameImports->AAS_AreaTravelTimeToGoalArea( areanum, origin, goalareanum, travelflags );
}

int trap_AAS_Swimming( vec3_t origin ) {
	return gameImports->AAS_Swimming( origin );
}

int trap_AAS_PredictClientMovement( struct aas_clientmove_s *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize ) {
	return gameImports->AAS_PredictClientMovement( move, entnum, origin, presencetype, onground, velocity, cmdmove, cmdframes, maxframes, frametime, stopevent, stopareanum, visualize );
}

// Ridah, route-tables
void trap_AAS_RT_ShowRoute( vec3_t srcpos, int srcnum, int destnum ) {
	gameImports->AAS_RT_ShowRoute( srcpos, srcnum, destnum );
}

qboolean trap_AAS_RT_GetHidePos( vec3_t srcpos, int srcnum, int srcarea, vec3_t destpos, int destnum, int destarea, vec3_t returnPos ) {
	return gameImports->AAS_RT_GetHidePos( srcpos, srcnum, srcarea, destpos, destnum, destarea, returnPos );
}

int trap_AAS_FindAttackSpotWithinRange( int srcnum, int rangenum, int enemynum, float rangedist, int travelflags, float *outpos ) {
	return gameImports->AAS_FindAttackSpotWithinRange( srcnum, rangenum, enemynum, rangedist, travelflags, outpos );
}

qboolean trap_AAS_GetRouteFirstVisPos( vec3_t srcpos, vec3_t destpos, int travelflags, vec3_t retpos ) {
	return gameImports->AAS_GetRouteFirstVisPos( srcpos, destpos, travelflags, retpos );
}

void trap_AAS_SetAASBlockingEntity( vec3_t absmin, vec3_t absmax, qboolean blocking ) {
	gameImports->AAS_SetAASBlockingEntity( absmin, absmax, blocking );
}
// done.

void trap_EA_Say( int client, char *str ) {
	gameImports->EA_Say( client, str );
}

void trap_EA_SayTeam( int client, char *str ) {
	gameImports->EA_SayTeam( client, str );
}

void trap_EA_UseItem( int client, char *it ) {
	gameImports->EA_UseItem( client, it );
}

void trap_EA_DropItem( int client, char *it ) {
	gameImports->EA_DropItem( client, it );
}

void trap_EA_UseInv( int client, char *inv ) {
	gameImports->EA_UseInv( client, inv );
}

void trap_EA_DropInv( int client, char *inv ) {
	gameImports->EA_DropInv( client, inv );
}

void trap_EA_Gesture( int client ) {
	gameImports->EA_Gesture( client );
}

void trap_EA_Command( int client, char *command ) {
	gameImports->EA_Command( client, command );
}

void trap_EA_SelectWeapon( int client, int weapon ) {
	gameImports->EA_SelectWeapon( client, weapon );
}

void trap_EA_Talk( int client ) {
	gameImports->EA_Talk( client );
}

void trap_EA_Attack( int client ) {
	gameImports->EA_Attack( client );
}

void trap_EA_Reload( int client ) {
	gameImports->EA_Reload( client );
}

void trap_EA_Use( int client ) {
	gameImports->EA_Use( client );
}

void trap_EA_Respawn( int client ) {
	gameImports->EA_Respawn( client );
}

void trap_EA_Jump( int client ) {
	gameImports->EA_Jump( client );
}

void trap_EA_DelayedJump( int client ) {
	gameImports->EA_DelayedJump( client );
}

void trap_EA_Crouch( int client ) {
	gameImports->EA_Crouch( client );
}

void trap_EA_MoveUp( int client ) {
	gameImports->EA_MoveUp( client );
}

void trap_EA_MoveDown( int client ) {
	gameImports->EA_MoveDown( client );
}

void trap_EA_MoveForward( int client ) {
	gameImports->EA_MoveForward( client );
}

void trap_EA_MoveBack( int client ) {
	gameImports->EA_MoveBack( client );
}

void trap_EA_MoveLeft( int client ) {
	gameImports->EA_MoveLeft( client );
}

void trap_EA_MoveRight( int client ) {
	gameImports->EA_MoveRight( client );
}

void trap_EA_Move( int client, vec3_t dir, float speed ) {
	gameImports->EA_Move( client, dir, speed );
}

void trap_EA_View( int client, vec3_t viewangles ) {
	gameImports->EA_View( client, viewangles );
}

void trap_EA_EndRegular( int client, float thinktime ) {
	gameImports->EA_EndRegular( client, thinktime );
}

void trap_EA_GetInput( int client, float thinktime, struct bot_input_s *input ) {
	gameImports->EA_GetInput( client, thinktime, input );
}

void trap_EA_ResetInput( int client, struct bot_input_s *init ) {
	gameImports->EA_ResetInput( client, init );
}

int trap_BotLoadCharacter( char *charfile, int skill ) {
	return gameImports->BotLoadCharacter( charfile, skill );
}

void trap_BotFreeCharacter( int character ) {
	gameImports->BotFreeCharacter( character );
}

float trap_Characteristic_Float( int character, int index ) {
	return gameImports->Characteristic_Float( character, index );
}

float trap_Characteristic_BFloat( int character, int index, float min, float max ) {
	return gameImports->Characteristic_BFloat( character, index, min, max );
}

int trap_Characteristic_Integer( int character, int index ) {
	return gameImports->Characteristic_Integer( character, index );
}

int trap_Characteristic_BInteger( int character, int index, int min, int max ) {
	return gameImports->Characteristic_BInteger( character, index, min, max );
}

void trap_Characteristic_String( int character, int index, char *buf, int size ) {
	gameImports->Characteristic_String( character, index, buf, size );
}

int trap_BotAllocChatState( void ) {
	return gameImports->BotAllocChatState(  );
}

void trap_BotFreeChatState( int handle ) {
	gameImports->BotFreeChatState( handle );
}

void trap_BotQueueConsoleMessage( int chatstate, int type, char *message ) {
	gameImports->BotQueueConsoleMessage( chatstate, type, message );
}

void trap_BotRemoveConsoleMessage( int chatstate, int handle ) {
	gameImports->BotRemoveConsoleMessage( chatstate, handle );
}

int trap_BotNextConsoleMessage( int chatstate, struct bot_consolemessage_s *cm ) {
	return gameImports->BotNextConsoleMessage( chatstate, cm );
}

int trap_BotNumConsoleMessages( int chatstate ) {
	return gameImports->BotNumConsoleMessages( chatstate );
}

void trap_BotInitialChat( int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 ) {
	gameImports->BotInitialChat( chatstate, type, mcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
}

int trap_BotNumInitialChats( int chatstate, char *type ) {
	return gameImports->BotNumInitialChats( chatstate, type );
}

int trap_BotReplyChat( int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 ) {
	return gameImports->BotReplyChat( chatstate, message, mcontext, vcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
}

int trap_BotChatLength( int chatstate ) {
	return gameImports->BotChatLength( chatstate );
}

void trap_BotEnterChat( int chatstate, int client, int sendto ) {
	gameImports->BotEnterChat( chatstate, client, sendto );
}

void trap_BotGetChatMessage( int chatstate, char *buf, int size ) {
	gameImports->BotGetChatMessage( chatstate, buf, size );
}

int trap_StringContains( char *str1, char *str2, int casesensitive ) {
	return gameImports->StringContains( str1, str2, casesensitive );
}

int trap_BotFindMatch( char *str, struct bot_match_s *match, unsigned long int context ) {
	return gameImports->BotFindMatch( str, match, context );
}

void trap_BotMatchVariable( struct bot_match_s *match, int variable, char *buf, int size ) {
	gameImports->BotMatchVariable( match, variable, buf, size );
}

void trap_UnifyWhiteSpaces( char *string ) {
	gameImports->UnifyWhiteSpaces( string );
}

void trap_BotReplaceSynonyms( char *string, unsigned long int context ) {
	gameImports->BotReplaceSynonyms( string, context );
}

int trap_BotLoadChatFile( int chatstate, char *chatfile, char *chatname ) {
	return gameImports->BotLoadChatFile( chatstate, chatfile, chatname );
}

void trap_BotSetChatGender( int chatstate, int gender ) {
	gameImports->BotSetChatGender( chatstate, gender );
}

void trap_BotSetChatName( int chatstate, char *name ) {
	gameImports->BotSetChatName( chatstate, name );
}

void trap_BotResetGoalState( int goalstate ) {
	gameImports->BotResetGoalState( goalstate );
}

void trap_BotResetAvoidGoals( int goalstate ) {
	gameImports->BotResetAvoidGoals( goalstate );
}

void trap_BotRemoveFromAvoidGoals( int goalstate, int number ) {
	gameImports->BotRemoveFromAvoidGoals( goalstate, number );
}

void trap_BotPushGoal( int goalstate, struct bot_goal_s *goal ) {
	gameImports->BotPushGoal( goalstate, goal );
}

void trap_BotPopGoal( int goalstate ) {
	gameImports->BotPopGoal( goalstate );
}

void trap_BotEmptyGoalStack( int goalstate ) {
	gameImports->BotEmptyGoalStack( goalstate );
}

void trap_BotDumpAvoidGoals( int goalstate ) {
	gameImports->BotDumpAvoidGoals( goalstate );
}

void trap_BotDumpGoalStack( int goalstate ) {
	gameImports->BotDumpGoalStack( goalstate );
}

void trap_BotGoalName( int number, char *name, int size ) {
	gameImports->BotGoalName( number, name, size );
}

int trap_BotGetTopGoal( int goalstate, struct bot_goal_s *goal ) {
	return gameImports->BotGetTopGoal( goalstate, goal );
}

int trap_BotGetSecondGoal( int goalstate, struct bot_goal_s *goal ) {
	return gameImports->BotGetSecondGoal( goalstate, goal );
}

int trap_BotChooseLTGItem( int goalstate, vec3_t origin, int *inventory, int travelflags ) {
	return gameImports->BotChooseLTGItem( goalstate, origin, inventory, travelflags );
}

int trap_BotChooseNBGItem( int goalstate, vec3_t origin, int *inventory, int travelflags, struct bot_goal_s *ltg, float maxtime ) {
	return gameImports->BotChooseNBGItem( goalstate, origin, inventory, travelflags, ltg, maxtime );
}

int trap_BotTouchingGoal( vec3_t origin, struct bot_goal_s *goal ) {
	return gameImports->BotTouchingGoal( origin, goal );
}

int trap_BotItemGoalInVisButNotVisible( int viewer, vec3_t eye, vec3_t viewangles, struct bot_goal_s *goal ) {
	return gameImports->BotItemGoalInVisButNotVisible( viewer, eye, viewangles, goal );
}

int trap_BotGetLevelItemGoal( int index, char *classname, struct bot_goal_s *goal ) {
	return gameImports->BotGetLevelItemGoal( index, classname, goal );
}

int trap_BotGetNextCampSpotGoal( int num, struct bot_goal_s *goal ) {
	return gameImports->BotGetNextCampSpotGoal( num, goal );
}

int trap_BotGetMapLocationGoal( char *name, struct bot_goal_s *goal ) {
	return gameImports->BotGetMapLocationGoal( name, goal );
}

float trap_BotAvoidGoalTime( int goalstate, int number ) {
	return gameImports->BotAvoidGoalTime( goalstate, number );
}

void trap_BotInitLevelItems( void ) {
	gameImports->BotInitLevelItems(  );
}

void trap_BotUpdateEntityItems( void ) {
	gameImports->BotUpdateEntityItems(  );
}

int trap_BotLoadItemWeights( int goalstate, char *filename ) {
	return gameImports->BotLoadItemWeights( goalstate, filename );
}

void trap_BotFreeItemWeights( int goalstate ) {
	gameImports->BotFreeItemWeights( goalstate );
}

void trap_BotInterbreedGoalFuzzyLogic( int parent1, int parent2, int child ) {
	gameImports->BotInterbreedGoalFuzzyLogic( parent1, parent2, child );
}

void trap_BotSaveGoalFuzzyLogic( int goalstate, char *filename ) {
	gameImports->BotSaveGoalFuzzyLogic( goalstate, filename );
}

void trap_BotMutateGoalFuzzyLogic( int goalstate, float range ) {
	gameImports->BotMutateGoalFuzzyLogic( goalstate, range );
}

int trap_BotAllocGoalState( int state ) {
	return gameImports->BotAllocGoalState( state );
}

void trap_BotFreeGoalState( int handle ) {
	gameImports->BotFreeGoalState( handle );
}

void trap_BotResetMoveState( int movestate ) {
	gameImports->BotResetMoveState( movestate );
}

void trap_BotMoveToGoal( struct bot_moveresult_s *result, int movestate, struct bot_goal_s *goal, int travelflags ) {
	gameImports->BotMoveToGoal( result, movestate, goal, travelflags );
}

int trap_BotMoveInDirection( int movestate, vec3_t dir, float speed, int type ) {
	return gameImports->BotMoveInDirection( movestate, dir, speed, type );
}

void trap_BotResetAvoidReach( int movestate ) {
	gameImports->BotResetAvoidReach( movestate );
}

void trap_BotResetLastAvoidReach( int movestate ) {
	gameImports->BotResetLastAvoidReach( movestate );
}

int trap_BotReachabilityArea( vec3_t origin, int testground ) {
	return gameImports->BotReachabilityArea( origin, testground );
}

int trap_BotMovementViewTarget( int movestate, struct bot_goal_s *goal, int travelflags, float lookahead, vec3_t target ) {
	return gameImports->BotMovementViewTarget( movestate, goal, travelflags, lookahead, target );
}

int trap_BotPredictVisiblePosition( vec3_t origin, int areanum, struct bot_goal_s *goal, int travelflags, vec3_t target ) {
	return gameImports->BotPredictVisiblePosition( origin, areanum, goal, travelflags, target );
}

int trap_BotAllocMoveState( void ) {
	return gameImports->BotAllocMoveState(  );
}

void trap_BotFreeMoveState( int handle ) {
	gameImports->BotFreeMoveState( handle );
}

void trap_BotInitMoveState( int handle, struct bot_initmove_s *initmove ) {
	gameImports->BotInitMoveState( handle, initmove );
}

// Ridah
void trap_BotInitAvoidReach( int handle ) {
	gameImports->BotInitAvoidReach( handle );
}
// Done.

int trap_BotChooseBestFightWeapon( int weaponstate, int *inventory ) {
	return gameImports->BotChooseBestFightWeapon( weaponstate, inventory );
}

void trap_BotGetWeaponInfo( int weaponstate, int weapon, struct weaponinfo_s *weaponinfo ) {
	gameImports->BotGetWeaponInfo( weaponstate, weapon, weaponinfo );
}

int trap_BotLoadWeaponWeights( int weaponstate, char *filename ) {
	return gameImports->BotLoadWeaponWeights( weaponstate, filename );
}

int trap_BotAllocWeaponState( void ) {
	return gameImports->BotAllocWeaponState(  );
}

void trap_BotFreeWeaponState( int weaponstate ) {
	gameImports->BotFreeWeaponState( weaponstate );
}

void trap_BotResetWeaponState( int weaponstate ) {
	gameImports->BotResetWeaponState( weaponstate );
}

int trap_GeneticParentsAndChildSelection( int numranks, float *ranks, int *parent1, int *parent2, int *child ) {
	return gameImports->GeneticParentsAndChildSelection( numranks, ranks, parent1, parent2, child );
}
