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

// sv_game.c -- interface to the game dll

#include "server.h"

#include "../game/botlib.h"

botlib_export_t *botlib_export;

void SV_GameError( const char *string ) {
	Com_Error( ERR_DROP, "%s", string );
}

void SV_GamePrint( const char *string ) {
	Com_Printf( "%s", string );
}

// these functions must be used instead of pointer arithmetic, because
// the game allocates gentities with private information after the server shared part
int SV_NumForGentity( sharedEntity_t *ent ) {
	int num;

	num = ( (byte *)ent - (byte *)sv.gentities ) / sv.gentitySize;

	return num;
}

sharedEntity_t *SV_GentityNum( int num ) {
	sharedEntity_t *ent;

	ent = ( sharedEntity_t * )( (byte *)sv.gentities + sv.gentitySize * ( num ) );

	return ent;
}

playerState_t *SV_GameClientNum( int num ) {
	playerState_t   *ps;

	ps = ( playerState_t * )( (byte *)sv.gameClients + sv.gameClientSize * ( num ) );

	return ps;
}

svEntity_t  *SV_SvEntityForGentity( sharedEntity_t *gEnt ) {
	if ( !gEnt || gEnt->s.number < 0 || gEnt->s.number >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}
	return &sv.svEntities[ gEnt->s.number ];
}

sharedEntity_t *SV_GEntityForSvEntity( svEntity_t *svEnt ) {
	int num;

	num = svEnt - sv.svEntities;
	return SV_GentityNum( num );
}

/*
===============
SV_GameSendServerCommand

Sends a command string to a client
===============
*/
void SV_GameSendServerCommand( int clientNum, const char *text ) {
	if ( clientNum == -1 ) {
		SV_SendServerCommand( NULL, "%s", text );
	} else {
		if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
			return;
		}
		SV_SendServerCommand( svs.clients + clientNum, "%s", text );
	}
}


/*
===============
SV_GameDropClient

Disconnects the client with a message
===============
*/
void SV_GameDropClient( int clientNum, const char *reason ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		return;
	}
	SV_DropClient( svs.clients + clientNum, reason );
}


/*
=================
SV_SetBrushModel

sets mins and maxs for inline bmodels
=================
*/
void SV_SetBrushModel( sharedEntity_t *ent, const char *name ) {
	clipHandle_t h;
	vec3_t mins, maxs;

	if ( !name ) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: NULL" );
	}

	if ( name[0] != '*' ) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );
	}


	ent->s.modelindex = atoi( name + 1 );

	h = CM_InlineModel( ent->s.modelindex );
	CM_ModelBounds( h, mins, maxs );
	VectorCopy( mins, ent->r.mins );
	VectorCopy( maxs, ent->r.maxs );
	ent->r.bmodel = qtrue;

	ent->r.contents = -1;       // we don't know exactly what is in the brushes

	SV_LinkEntity( ent );       // FIXME: remove
}



/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
qboolean SV_inPVS( const vec3_t p1, const vec3_t p2 ) {
	int leafnum;
	int cluster;
	int area1, area2;
	byte    *mask;

	leafnum = CM_PointLeafnum( p1 );
	cluster = CM_LeafCluster( leafnum );
	area1 = CM_LeafArea( leafnum );
	mask = CM_ClusterPVS( cluster );

	leafnum = CM_PointLeafnum( p2 );
	cluster = CM_LeafCluster( leafnum );
	area2 = CM_LeafArea( leafnum );
	if ( mask && ( !( mask[cluster >> 3] & ( 1 << ( cluster & 7 ) ) ) ) ) {
		return qfalse;
	}
	if ( !CM_AreasConnected( area1, area2 ) ) {
		return qfalse;      // a door blocks sight
	}
	return qtrue;
}


/*
=================
SV_inPVSIgnorePortals

Does NOT check portalareas
=================
*/
qboolean SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {
	int leafnum;
	int cluster;
	int area1, area2;
	byte    *mask;

	leafnum = CM_PointLeafnum( p1 );
	cluster = CM_LeafCluster( leafnum );
	area1 = CM_LeafArea( leafnum );
	mask = CM_ClusterPVS( cluster );

	leafnum = CM_PointLeafnum( p2 );
	cluster = CM_LeafCluster( leafnum );
	area2 = CM_LeafArea( leafnum );

	if ( mask && ( !( mask[cluster >> 3] & ( 1 << ( cluster & 7 ) ) ) ) ) {
		return qfalse;
	}

	return qtrue;
}


/*
========================
SV_AdjustAreaPortalState
========================
*/
void SV_AdjustAreaPortalState( sharedEntity_t *ent, qboolean open ) {
	svEntity_t  *svEnt;

	svEnt = SV_SvEntityForGentity( ent );
	if ( svEnt->areanum2 == -1 ) {
		return;
	}
	CM_AdjustAreaPortalState( svEnt->areanum, svEnt->areanum2, open );
}


/*
==================
SV_GameAreaEntities
==================
*/
qboolean    SV_EntityContact( const vec3_t mins, const vec3_t maxs, const sharedEntity_t *gEnt, const int capsule ) {
	const float *origin, *angles;
	clipHandle_t ch;
	trace_t trace;

	// check for exact collision
	origin = gEnt->r.currentOrigin;
	angles = gEnt->r.currentAngles;

	ch = SV_ClipHandleForEntity( gEnt );
	CM_TransformedBoxTrace( &trace, vec3_origin, vec3_origin, mins, maxs,
							ch, -1, origin, angles, capsule );

	return trace.startsolid;
}


/*
===============
SV_GetServerinfo

===============
*/
void SV_GetServerinfo( char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize );
	}
	Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO ), bufferSize );
}

/*
===============
SV_LocateGameData

===============
*/
void SV_LocateGameData( sharedEntity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						playerState_t *clients, int sizeofGameClient ) {
	sv.gentities = gEnts;
	sv.gentitySize = sizeofGEntity_t;
	sv.num_entities = numGEntities;

	sv.gameClients = clients;
	sv.gameClientSize = sizeofGameClient;
}


/*
===============
SV_GetUsercmd

===============
*/
void SV_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum );
	}
	*cmd = svs.clients[clientNum].lastUsercmd;
}

//==============================================

/*
====================
SV_CreateGameImports

Build the native game function-pointer import table.
====================
*/
static gameImport_t SV_CreateGameImports() {
	gameImport_t imports = {};
	imports.Com_Printf = []( const char *fmt ) -> void {
		Com_Printf( "%s", fmt );
		return;
	};
	imports.Com_Error = []( const char *fmt ) -> void {
		Com_Error( ERR_DROP, "%s", fmt );
		return;
	};
	imports.Com_ErrorEndGame = []( void ) -> void {
		Com_Error( ERR_ENDGAME, "endgame" );  // no message, no error print
		return;
	};
	imports.Sys_Milliseconds = []( void ) -> int {
		return Sys_Milliseconds();
	};
	imports.Cmd_Argc = []( void ) -> int {
		return Cmd_Argc();
	};
	imports.Cmd_ArgvBuffer = []( int n, char *buffer, int bufferLength ) -> void {
		Cmd_ArgvBuffer( n, buffer, bufferLength );
		return;
	};
	imports.FS_FOpenFileByMode = []( const char *qpath, fileHandle_t *f, fsMode_t mode ) -> int {
		return FS_FOpenFileByMode( qpath, f, (fsMode_t)mode );
	};
	imports.FS_Read = []( void *buffer, int len, fileHandle_t f ) -> void {
		FS_Read( buffer, len, f );
		return;
	};
	imports.FS_Write = []( const void *buffer, int len, fileHandle_t f ) -> int {
		return FS_Write( buffer, len, f );
	};
	imports.FS_Rename = []( const char *from, const char *to ) -> int {
		FS_Rename( from, to );
		return 0;
	};
	imports.FS_FCloseFile = []( fileHandle_t f ) -> void {
		FS_FCloseFile( f );
		return;
	};
	imports.FS_CopyFileOS = []( char *from, char *to ) -> void {
		FS_CopyFileOS( from, to );    //DAJ
		return;
	};
	imports.FS_GetFileList = []( const char *path, const char *extension, char *listbuf, int bufsize ) -> int {
		return FS_GetFileList( path, extension, listbuf, bufsize );
	};
	imports.Cbuf_ExecuteText = []( int exec_when, const char *text ) -> void {
		Cbuf_ExecuteText( exec_when, text );
		return;
	};
	imports.SV_LocateGameData = []( sharedEntity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *clients, int sizeofGClient ) -> void {
		SV_LocateGameData( gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient );
		return;
	};
	imports.Cvar_Register = []( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) -> void {
		Cvar_Register( cvar, var_name, value, flags );
		return;
	};
	imports.Cvar_Update = []( vmCvar_t *cvar ) -> void {
		Cvar_Update( cvar );
		return;
	};
	imports.Cvar_Set = []( const char *var_name, const char *value ) -> void {
		Cvar_Set( (const char *)var_name, (const char *)value );
		return;
	};
	imports.Cvar_VariableIntegerValue = []( const char *var_name ) -> int {
		return Cvar_VariableIntegerValue( (const char *)var_name );
	};
	imports.Cvar_VariableStringBuffer = []( const char *var_name, char *buffer, int bufsize ) -> void {
		Cvar_VariableStringBuffer( var_name, buffer, bufsize );
		return;
	};
	imports.SV_GameDropClient = []( int clientNum, const char *reason ) -> void {
		SV_GameDropClient( clientNum, reason );
		return;
	};
	imports.SV_GameSendServerCommand = []( int clientNum, const char *text ) -> void {
		SV_GameSendServerCommand( clientNum, text );
		return;
	};
	imports.SV_SetConfigstring = []( int num, const char *string ) -> void {
		SV_SetConfigstring( num, string );
		return;
	};
	imports.SV_GetConfigstring = []( int num, char *buffer, int bufferSize ) -> void {
		SV_GetConfigstring( num, buffer, bufferSize );
		return;
	};
	imports.SV_GetUserinfo = []( int num, char *buffer, int bufferSize ) -> void {
		SV_GetUserinfo( num, buffer, bufferSize );
		return;
	};
	imports.SV_SetUserinfo = []( int num, const char *buffer ) -> void {
		SV_SetUserinfo( num, buffer );
		return;
	};
	imports.SV_GetServerinfo = []( char *buffer, int bufferSize ) -> void {
		SV_GetServerinfo( buffer, bufferSize );
		return;
	};
	imports.SV_SetBrushModel = []( gentity_t *ent, const char *name ) -> void {
		SV_SetBrushModel( (sharedEntity_t *)ent, name );
		return;
	};
	imports.SV_Trace = []( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) -> void {
		SV_Trace( results, start, mins, maxs, end, passEntityNum, contentmask, /* int capsule */ qfalse );
		return;
	};
	imports.SV_TraceCapsule = []( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) -> void {
		SV_Trace( results, start, mins, maxs, end, passEntityNum, contentmask, /* int capsule */ qtrue );
		return;
	};
	imports.SV_PointContents = []( const vec3_t point, int passEntityNum ) -> int {
		return SV_PointContents( point, passEntityNum );
	};
	imports.SV_inPVS = []( const vec3_t p1, const vec3_t p2 ) -> qboolean {
		return SV_inPVS( p1, p2 );
	};
	imports.SV_inPVSIgnorePortals = []( const vec3_t p1, const vec3_t p2 ) -> qboolean {
		return SV_inPVSIgnorePortals( p1, p2 );
	};
	imports.SV_AdjustAreaPortalState = []( gentity_t *ent, qboolean open ) -> void {
		SV_AdjustAreaPortalState( (sharedEntity_t *)ent, open );
		return;
	};
	imports.CM_AreasConnected = []( int area1, int area2 ) -> qboolean {
		return CM_AreasConnected( area1, area2 );
	};
	imports.SV_LinkEntity = []( gentity_t *ent ) -> void {
		SV_LinkEntity( (sharedEntity_t *)ent );
		return;
	};
	imports.SV_UnlinkEntity = []( gentity_t *ent ) -> void {
		SV_UnlinkEntity( (sharedEntity_t *)ent );
		return;
	};
	imports.SV_AreaEntities = []( const vec3_t mins, const vec3_t maxs, int *list, int maxcount ) -> int {
		return SV_AreaEntities( mins, maxs, list, maxcount );
	};
	imports.SV_EntityContact = []( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) -> qboolean {
		return SV_EntityContact( mins, maxs, (const sharedEntity_t *)ent, /* int capsule */ qfalse );
	};
	imports.SV_EntityContactCapsule = []( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) -> qboolean {
		return SV_EntityContact( mins, maxs, (const sharedEntity_t *)ent, /* int capsule */ qtrue );
	};
	imports.SV_BotAllocateClient = []( void ) -> int {
		return SV_BotAllocateClient();
	};
	imports.SV_BotFreeClient = []( int clientNum ) -> void {
		SV_BotFreeClient( clientNum );
		return;
	};
	imports.SV_GetUsercmd = []( int clientNum, usercmd_t *cmd ) -> void {
		SV_GetUsercmd( clientNum, cmd );
		return;
	};
	imports.SV_GetEntityToken = []( char *buffer, int bufferSize ) -> qboolean {
		{
		const char  *s;

		s = COM_Parse( &sv.entityParsePoint );
		Q_strncpyz( buffer, s, bufferSize );
		if ( !sv.entityParsePoint && !s[0] ) {
		return qfalse;
		} else {
		return qtrue;
		}
		}
	};
	imports.BotImport_DebugPolygonCreate = []( int color, int numPoints, vec3_t *points ) -> int {
		return BotImport_DebugPolygonCreate( color, numPoints, points );
	};
	imports.BotImport_DebugPolygonDelete = []( int id ) -> void {
		BotImport_DebugPolygonDelete( id );
		return;
	};
	imports.Com_RealTime = []( qtime_t *qtime ) -> int {
		return Com_RealTime( qtime );
	};
	imports.Sys_SnapVector = []( float *v ) -> void {
		Sys_SnapVector( v );
		return;
	};
	imports.SV_GetTag = []( int clientNum, char *tagName, orientation_t *or ) -> qboolean {
		return SV_GetTag( clientNum, tagName, or );

		//====================================
	};
	imports.SV_BotLibSetup = []( void ) -> int {
		return SV_BotLibSetup();
	};
	imports.SV_BotLibShutdown = []( void ) -> int {
		return SV_BotLibShutdown();
	};
	imports.BotLibVarSet = []( char *var_name, char *value ) -> int {
		return botlib_export->BotLibVarSet( var_name, value );
	};
	imports.BotLibVarGet = []( char *var_name, char *value, int size ) -> int {
		return botlib_export->BotLibVarGet( var_name, value, size );
	};
	imports.PC_AddGlobalDefine = []( char *string ) -> int {
		return botlib_export->PC_AddGlobalDefine( string );
	};
	imports.BotLibStartFrame = []( float time ) -> int {
		return botlib_export->BotLibStartFrame( time );
	};
	imports.BotLibLoadMap = []( const char *mapname ) -> int {
		return botlib_export->BotLibLoadMap( mapname );
	};
	imports.BotLibUpdateEntity = []( int ent, struct bot_entitystate_s *bue ) -> int {
		return botlib_export->BotLibUpdateEntity( ent, (bot_entitystate_t *)bue );
	};
	imports.Test = []( int parm0, char *parm1, vec3_t parm2, vec3_t parm3 ) -> int {
		return botlib_export->Test( parm0, parm1, parm2, parm3 );
	};
	imports.SV_BotGetSnapshotEntity = []( int clientNum, int sequence ) -> int {
		return SV_BotGetSnapshotEntity( clientNum, sequence );
	};
	imports.SV_BotGetConsoleMessage = []( int clientNum, char *message, int size ) -> int {
		return SV_BotGetConsoleMessage( clientNum, message, size );
	};
	imports.SV_ClientThink = []( int clientNum, usercmd_t *ucmd ) -> void {
		SV_ClientThink( &svs.clients[clientNum], ucmd );
		return;
	};
	imports.AAS_EntityInfo = []( int entnum, struct aas_entityinfo_s *info ) -> void {
		botlib_export->aas.AAS_EntityInfo( entnum, (struct aas_entityinfo_s *)info );
		return;
	};
	imports.AAS_Initialized = []( void ) -> int {
		return botlib_export->aas.AAS_Initialized();
	};
	imports.AAS_PresenceTypeBoundingBox = []( int presencetype, vec3_t mins, vec3_t maxs ) -> void {
		botlib_export->aas.AAS_PresenceTypeBoundingBox( presencetype, mins, maxs );
		return;
	};
	imports.AAS_Time = []( void ) -> float {
		return  botlib_export->aas.AAS_Time() ;

		// Ridah
	};
	imports.AAS_SetCurrentWorld = []( int index ) -> void {
		botlib_export->aas.AAS_SetCurrentWorld( index );
		return;
		// done.
	};
	imports.AAS_PointAreaNum = []( vec3_t point ) -> int {
		return botlib_export->aas.AAS_PointAreaNum( point );
	};
	imports.AAS_TraceAreas = []( vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas ) -> int {
		return botlib_export->aas.AAS_TraceAreas( start, end, areas, points, maxareas );
	};
	imports.AAS_PointContents = []( vec3_t point ) -> int {
		return botlib_export->aas.AAS_PointContents( point );
	};
	imports.AAS_NextBSPEntity = []( int ent ) -> int {
		return botlib_export->aas.AAS_NextBSPEntity( ent );
	};
	imports.AAS_ValueForBSPEpairKey = []( int ent, char *key, char *value, int size ) -> int {
		return botlib_export->aas.AAS_ValueForBSPEpairKey( ent, key, value, size );
	};
	imports.AAS_VectorForBSPEpairKey = []( int ent, char *key, vec3_t v ) -> int {
		return botlib_export->aas.AAS_VectorForBSPEpairKey( ent, key, v );
	};
	imports.AAS_FloatForBSPEpairKey = []( int ent, char *key, float *value ) -> int {
		return botlib_export->aas.AAS_FloatForBSPEpairKey( ent, key, value );
	};
	imports.AAS_IntForBSPEpairKey = []( int ent, char *key, int *value ) -> int {
		return botlib_export->aas.AAS_IntForBSPEpairKey( ent, key, value );
	};
	imports.AAS_AreaReachability = []( int areanum ) -> int {
		return botlib_export->aas.AAS_AreaReachability( areanum );
	};
	imports.AAS_AreaTravelTimeToGoalArea = []( int areanum, vec3_t origin, int goalareanum, int travelflags ) -> int {
		return botlib_export->aas.AAS_AreaTravelTimeToGoalArea( areanum, origin, goalareanum, travelflags );
	};
	imports.AAS_Swimming = []( vec3_t origin ) -> int {
		return botlib_export->aas.AAS_Swimming( origin );
	};
	imports.AAS_PredictClientMovement = []( struct aas_clientmove_s *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize ) -> int {
		return botlib_export->aas.AAS_PredictClientMovement( (struct aas_clientmove_s *)move, entnum, origin, presencetype, onground,
		 velocity, cmdmove, cmdframes, maxframes, frametime, stopevent, stopareanum, visualize );

		// Ridah, route-tables
	};
	imports.AAS_RT_ShowRoute = []( vec3_t srcpos, int srcnum, int destnum ) -> void {
		botlib_export->aas.AAS_RT_ShowRoute( srcpos, srcnum, destnum );
		return;
	};
	imports.AAS_RT_GetHidePos = []( vec3_t srcpos, int srcnum, int srcarea, vec3_t destpos, int destnum, int destarea, vec3_t returnPos ) -> qboolean {
		return botlib_export->aas.AAS_RT_GetHidePos( srcpos, srcnum, srcarea, destpos, destnum, destarea, returnPos );
	};
	imports.AAS_FindAttackSpotWithinRange = []( int srcnum, int rangenum, int enemynum, float rangedist, int travelflags, float *outpos ) -> int {
		return botlib_export->aas.AAS_FindAttackSpotWithinRange( srcnum, rangenum, enemynum, rangedist, travelflags, outpos );
	};
	imports.AAS_GetRouteFirstVisPos = []( vec3_t srcpos, vec3_t destpos, int travelflags, vec3_t retpos ) -> qboolean {
		return botlib_export->aas.AAS_GetRouteFirstVisPos( srcpos, destpos, travelflags, retpos );
	};
	imports.AAS_SetAASBlockingEntity = []( vec3_t absmin, vec3_t absmax, qboolean blocking ) -> void {
		botlib_export->aas.AAS_SetAASBlockingEntity( absmin, absmax, blocking );
		return;
		// done.
	};
	imports.EA_Say = []( int client, char *str ) -> void {
		botlib_export->ea.EA_Say( client, str );
		return;
	};
	imports.EA_SayTeam = []( int client, char *str ) -> void {
		botlib_export->ea.EA_SayTeam( client, str );
		return;
	};
	imports.EA_UseItem = []( int client, char *it ) -> void {
		botlib_export->ea.EA_UseItem( client, it );
		return;
	};
	imports.EA_DropItem = []( int client, char *it ) -> void {
		botlib_export->ea.EA_DropItem( client, it );
		return;
	};
	imports.EA_UseInv = []( int client, char *inv ) -> void {
		botlib_export->ea.EA_UseInv( client, inv );
		return;
	};
	imports.EA_DropInv = []( int client, char *inv ) -> void {
		botlib_export->ea.EA_DropInv( client, inv );
		return;
	};
	imports.EA_Gesture = []( int client ) -> void {
		botlib_export->ea.EA_Gesture( client );
		return;
	};
	imports.EA_Command = []( int client, char *command ) -> void {
		botlib_export->ea.EA_Command( client, command );
		return;
	};
	imports.EA_SelectWeapon = []( int client, int weapon ) -> void {
		botlib_export->ea.EA_SelectWeapon( client, weapon );
		return;
	};
	imports.EA_Talk = []( int client ) -> void {
		botlib_export->ea.EA_Talk( client );
		return;
	};
	imports.EA_Attack = []( int client ) -> void {
		botlib_export->ea.EA_Attack( client );
		return;
	};
	imports.EA_Reload = []( int client ) -> void {
		botlib_export->ea.EA_Reload( client );
		return;
	};
	imports.EA_Use = []( int client ) -> void {
		botlib_export->ea.EA_Use( client );
		return;
	};
	imports.EA_Respawn = []( int client ) -> void {
		botlib_export->ea.EA_Respawn( client );
		return;
	};
	imports.EA_Jump = []( int client ) -> void {
		botlib_export->ea.EA_Jump( client );
		return;
	};
	imports.EA_DelayedJump = []( int client ) -> void {
		botlib_export->ea.EA_DelayedJump( client );
		return;
	};
	imports.EA_Crouch = []( int client ) -> void {
		botlib_export->ea.EA_Crouch( client );
		return;
	};
	imports.EA_MoveUp = []( int client ) -> void {
		botlib_export->ea.EA_MoveUp( client );
		return;
	};
	imports.EA_MoveDown = []( int client ) -> void {
		botlib_export->ea.EA_MoveDown( client );
		return;
	};
	imports.EA_MoveForward = []( int client ) -> void {
		botlib_export->ea.EA_MoveForward( client );
		return;
	};
	imports.EA_MoveBack = []( int client ) -> void {
		botlib_export->ea.EA_MoveBack( client );
		return;
	};
	imports.EA_MoveLeft = []( int client ) -> void {
		botlib_export->ea.EA_MoveLeft( client );
		return;
	};
	imports.EA_MoveRight = []( int client ) -> void {
		botlib_export->ea.EA_MoveRight( client );
		return;
	};
	imports.EA_Move = []( int client, vec3_t dir, float speed ) -> void {
		botlib_export->ea.EA_Move( client, dir, speed );
		return;
	};
	imports.EA_View = []( int client, vec3_t viewangles ) -> void {
		botlib_export->ea.EA_View( client, viewangles );
		return;
	};
	imports.EA_EndRegular = []( int client, float thinktime ) -> void {
		botlib_export->ea.EA_EndRegular( client, thinktime );
		return;
	};
	imports.EA_GetInput = []( int client, float thinktime, struct bot_input_s *input ) -> void {
		botlib_export->ea.EA_GetInput( client, thinktime, (bot_input_t *)input );
		return;
	};
	imports.EA_ResetInput = []( int client, struct bot_input_s *init ) -> void {
		botlib_export->ea.EA_ResetInput( client, (bot_input_t *)init );
		return;
	};
	imports.BotLoadCharacter = []( char *charfile, int skill ) -> int {
		return botlib_export->ai.BotLoadCharacter( charfile, skill );
	};
	imports.BotFreeCharacter = []( int character ) -> void {
		botlib_export->ai.BotFreeCharacter( character );
		return;
	};
	imports.Characteristic_Float = []( int character, int index ) -> float {
		return  botlib_export->ai.Characteristic_Float( character, index ) ;
	};
	imports.Characteristic_BFloat = []( int character, int index, float min, float max ) -> float {
		return  botlib_export->ai.Characteristic_BFloat( character, index, min, max ) ;
	};
	imports.Characteristic_Integer = []( int character, int index ) -> int {
		return botlib_export->ai.Characteristic_Integer( character, index );
	};
	imports.Characteristic_BInteger = []( int character, int index, int min, int max ) -> int {
		return botlib_export->ai.Characteristic_BInteger( character, index, min, max );
	};
	imports.Characteristic_String = []( int character, int index, char *buf, int size ) -> void {
		botlib_export->ai.Characteristic_String( character, index, buf, size );
		return;
	};
	imports.BotAllocChatState = []( void ) -> int {
		return botlib_export->ai.BotAllocChatState();
	};
	imports.BotFreeChatState = []( int handle ) -> void {
		botlib_export->ai.BotFreeChatState( handle );
		return;
	};
	imports.BotQueueConsoleMessage = []( int chatstate, int type, char *message ) -> void {
		botlib_export->ai.BotQueueConsoleMessage( chatstate, type, message );
		return;
	};
	imports.BotRemoveConsoleMessage = []( int chatstate, int handle ) -> void {
		botlib_export->ai.BotRemoveConsoleMessage( chatstate, handle );
		return;
	};
	imports.BotNextConsoleMessage = []( int chatstate, struct bot_consolemessage_s *cm ) -> int {
		return botlib_export->ai.BotNextConsoleMessage( chatstate, (struct bot_consolemessage_s *)cm );
	};
	imports.BotNumConsoleMessages = []( int chatstate ) -> int {
		return botlib_export->ai.BotNumConsoleMessages( chatstate );
	};
	imports.BotInitialChat = []( int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 ) -> void {
		botlib_export->ai.BotInitialChat( chatstate, type, mcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
		return;
	};
	imports.BotNumInitialChats = []( int chatstate, char *type ) -> int {
		return botlib_export->ai.BotNumInitialChats( chatstate, type );
	};
	imports.BotReplyChat = []( int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 ) -> int {
		return botlib_export->ai.BotReplyChat( chatstate, message, mcontext, vcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
	};
	imports.BotChatLength = []( int chatstate ) -> int {
		return botlib_export->ai.BotChatLength( chatstate );
	};
	imports.BotEnterChat = []( int chatstate, int client, int sendto ) -> void {
		botlib_export->ai.BotEnterChat( chatstate, client, sendto );
		return;
	};
	imports.BotGetChatMessage = []( int chatstate, char *buf, int size ) -> void {
		botlib_export->ai.BotGetChatMessage( chatstate, buf, size );
		return;
	};
	imports.StringContains = []( char *str1, char *str2, int casesensitive ) -> int {
		return botlib_export->ai.StringContains( str1, str2, casesensitive );
	};
	imports.BotFindMatch = []( char *str, struct bot_match_s *match, unsigned long int context ) -> int {
		return botlib_export->ai.BotFindMatch( str, (struct bot_match_s *)match, context );
	};
	imports.BotMatchVariable = []( struct bot_match_s *match, int variable, char *buf, int size ) -> void {
		botlib_export->ai.BotMatchVariable( (struct bot_match_s *)match, variable, buf, size );
		return;
	};
	imports.UnifyWhiteSpaces = []( char *string ) -> void {
		botlib_export->ai.UnifyWhiteSpaces( string );
		return;
	};
	imports.BotReplaceSynonyms = []( char *string, unsigned long int context ) -> void {
		botlib_export->ai.BotReplaceSynonyms( string, context );
		return;
	};
	imports.BotLoadChatFile = []( int chatstate, char *chatfile, char *chatname ) -> int {
		return botlib_export->ai.BotLoadChatFile( chatstate, chatfile, chatname );
	};
	imports.BotSetChatGender = []( int chatstate, int gender ) -> void {
		botlib_export->ai.BotSetChatGender( chatstate, gender );
		return;
	};
	imports.BotSetChatName = []( int chatstate, char *name ) -> void {
		botlib_export->ai.BotSetChatName( chatstate, name );
		return;
	};
	imports.BotResetGoalState = []( int goalstate ) -> void {
		botlib_export->ai.BotResetGoalState( goalstate );
		return;
	};
	imports.BotResetAvoidGoals = []( int goalstate ) -> void {
		botlib_export->ai.BotResetAvoidGoals( goalstate );
		return;
	};
	imports.BotRemoveFromAvoidGoals = []( int goalstate, int number ) -> void {
		botlib_export->ai.BotRemoveFromAvoidGoals( goalstate, number );
		return;
	};
	imports.BotPushGoal = []( int goalstate, struct bot_goal_s *goal ) -> void {
		botlib_export->ai.BotPushGoal( goalstate, (struct bot_goal_s *)goal );
		return;
	};
	imports.BotPopGoal = []( int goalstate ) -> void {
		botlib_export->ai.BotPopGoal( goalstate );
		return;
	};
	imports.BotEmptyGoalStack = []( int goalstate ) -> void {
		botlib_export->ai.BotEmptyGoalStack( goalstate );
		return;
	};
	imports.BotDumpAvoidGoals = []( int goalstate ) -> void {
		botlib_export->ai.BotDumpAvoidGoals( goalstate );
		return;
	};
	imports.BotDumpGoalStack = []( int goalstate ) -> void {
		botlib_export->ai.BotDumpGoalStack( goalstate );
		return;
	};
	imports.BotGoalName = []( int number, char *name, int size ) -> void {
		botlib_export->ai.BotGoalName( number, name, size );
		return;
	};
	imports.BotGetTopGoal = []( int goalstate, struct bot_goal_s *goal ) -> int {
		return botlib_export->ai.BotGetTopGoal( goalstate, (struct bot_goal_s *)goal );
	};
	imports.BotGetSecondGoal = []( int goalstate, struct bot_goal_s *goal ) -> int {
		return botlib_export->ai.BotGetSecondGoal( goalstate, (struct bot_goal_s *)goal );
	};
	imports.BotChooseLTGItem = []( int goalstate, vec3_t origin, int *inventory, int travelflags ) -> int {
		return botlib_export->ai.BotChooseLTGItem( goalstate, origin, inventory, travelflags );
	};
	imports.BotChooseNBGItem = []( int goalstate, vec3_t origin, int *inventory, int travelflags, struct bot_goal_s *ltg, float maxtime ) -> int {
		return botlib_export->ai.BotChooseNBGItem( goalstate, origin, inventory, travelflags, (struct bot_goal_s *)ltg, maxtime );
	};
	imports.BotTouchingGoal = []( vec3_t origin, struct bot_goal_s *goal ) -> int {
		return botlib_export->ai.BotTouchingGoal( origin, (struct bot_goal_s *)goal );
	};
	imports.BotItemGoalInVisButNotVisible = []( int viewer, vec3_t eye, vec3_t viewangles, struct bot_goal_s *goal ) -> int {
		return botlib_export->ai.BotItemGoalInVisButNotVisible( viewer, eye, viewangles, (struct bot_goal_s *)goal );
	};
	imports.BotGetLevelItemGoal = []( int index, char *classname, struct bot_goal_s *goal ) -> int {
		return botlib_export->ai.BotGetLevelItemGoal( index, classname, (struct bot_goal_s *)goal );
	};
	imports.BotGetNextCampSpotGoal = []( int num, struct bot_goal_s *goal ) -> int {
		return botlib_export->ai.BotGetNextCampSpotGoal( num, (struct bot_goal_s *)goal );
	};
	imports.BotGetMapLocationGoal = []( char *name, struct bot_goal_s *goal ) -> int {
		return botlib_export->ai.BotGetMapLocationGoal( name, (struct bot_goal_s *)goal );
	};
	imports.BotAvoidGoalTime = []( int goalstate, int number ) -> float {
		return  botlib_export->ai.BotAvoidGoalTime( goalstate, number ) ;
	};
	imports.BotInitLevelItems = []( void ) -> void {
		botlib_export->ai.BotInitLevelItems();
		return;
	};
	imports.BotUpdateEntityItems = []( void ) -> void {
		botlib_export->ai.BotUpdateEntityItems();
		return;
	};
	imports.BotLoadItemWeights = []( int goalstate, char *filename ) -> int {
		return botlib_export->ai.BotLoadItemWeights( goalstate, filename );
	};
	imports.BotFreeItemWeights = []( int goalstate ) -> void {
		botlib_export->ai.BotFreeItemWeights( goalstate );
		return;
	};
	imports.BotInterbreedGoalFuzzyLogic = []( int parent1, int parent2, int child ) -> void {
		botlib_export->ai.BotInterbreedGoalFuzzyLogic( parent1, parent2, child );
		return;
	};
	imports.BotSaveGoalFuzzyLogic = []( int goalstate, char *filename ) -> void {
		botlib_export->ai.BotSaveGoalFuzzyLogic( goalstate, filename );
		return;
	};
	imports.BotMutateGoalFuzzyLogic = []( int goalstate, float range ) -> void {
		botlib_export->ai.BotMutateGoalFuzzyLogic( goalstate, range );
		return;
	};
	imports.BotAllocGoalState = []( int state ) -> int {
		return botlib_export->ai.BotAllocGoalState( state );
	};
	imports.BotFreeGoalState = []( int handle ) -> void {
		botlib_export->ai.BotFreeGoalState( handle );
		return;
	};
	imports.BotResetMoveState = []( int movestate ) -> void {
		botlib_export->ai.BotResetMoveState( movestate );
		return;
	};
	imports.BotMoveToGoal = []( struct bot_moveresult_s *result, int movestate, struct bot_goal_s *goal, int travelflags ) -> void {
		botlib_export->ai.BotMoveToGoal( (struct bot_moveresult_s *)result, movestate, (struct bot_goal_s *)goal, travelflags );
		return;
	};
	imports.BotMoveInDirection = []( int movestate, vec3_t dir, float speed, int type ) -> int {
		return botlib_export->ai.BotMoveInDirection( movestate, dir, speed, type );
	};
	imports.BotResetAvoidReach = []( int movestate ) -> void {
		botlib_export->ai.BotResetAvoidReach( movestate );
		return;
	};
	imports.BotResetLastAvoidReach = []( int movestate ) -> void {
		botlib_export->ai.BotResetLastAvoidReach( movestate );
		return;
	};
	imports.BotReachabilityArea = []( vec3_t origin, int testground ) -> int {
		return botlib_export->ai.BotReachabilityArea( origin, testground );
	};
	imports.BotMovementViewTarget = []( int movestate, struct bot_goal_s *goal, int travelflags, float lookahead, vec3_t target ) -> int {
		return botlib_export->ai.BotMovementViewTarget( movestate, (struct bot_goal_s *)goal, travelflags, lookahead, target );
	};
	imports.BotPredictVisiblePosition = []( vec3_t origin, int areanum, struct bot_goal_s *goal, int travelflags, vec3_t target ) -> int {
		return botlib_export->ai.BotPredictVisiblePosition( origin, areanum, (struct bot_goal_s *)goal, travelflags, target );
	};
	imports.BotAllocMoveState = []( void ) -> int {
		return botlib_export->ai.BotAllocMoveState();
	};
	imports.BotFreeMoveState = []( int handle ) -> void {
		botlib_export->ai.BotFreeMoveState( handle );
		return;
	};
	imports.BotInitMoveState = []( int handle, struct bot_initmove_s *initmove ) -> void {
		botlib_export->ai.BotInitMoveState( handle, (struct bot_initmove_s *)initmove );
		return;
		// Ridah
	};
	imports.BotInitAvoidReach = []( int handle ) -> void {
		botlib_export->ai.BotInitAvoidReach( handle );
		return;
		// done.
	};
	imports.BotChooseBestFightWeapon = []( int weaponstate, int *inventory ) -> int {
		return botlib_export->ai.BotChooseBestFightWeapon( weaponstate, inventory );
	};
	imports.BotGetWeaponInfo = []( int weaponstate, int weapon, struct weaponinfo_s *weaponinfo ) -> void {
		botlib_export->ai.BotGetWeaponInfo( weaponstate, weapon, (struct weaponinfo_s *)weaponinfo );
		return;
	};
	imports.BotLoadWeaponWeights = []( int weaponstate, char *filename ) -> int {
		return botlib_export->ai.BotLoadWeaponWeights( weaponstate, filename );
	};
	imports.BotAllocWeaponState = []( void ) -> int {
		return botlib_export->ai.BotAllocWeaponState();
	};
	imports.BotFreeWeaponState = []( int weaponstate ) -> void {
		botlib_export->ai.BotFreeWeaponState( weaponstate );
		return;
	};
	imports.BotResetWeaponState = []( int weaponstate ) -> void {
		botlib_export->ai.BotResetWeaponState( weaponstate );
		return;
	};
	imports.GeneticParentsAndChildSelection = []( int numranks, float *ranks, int *parent1, int *parent2, int *child ) -> int {
		return botlib_export->ai.GeneticParentsAndChildSelection( numranks, ranks, parent1, parent2, child );
	};
	return imports;
}



/*
===============
SV_ShutdownGameProgs

Called every time a map changes
===============
*/
void SV_ShutdownGameProgs( void ) {
	if ( !gvm ) {
		return;
	}
	gvm->GAME_SHUTDOWN(qfalse );
	gvm = NULL;
	Sys_UnloadDll( gvmHandle );
	gvmHandle = NULL;
}

/*
==================
SV_InitGameVM

Called for both a full init and a restart
==================
*/
static void SV_InitGameVM( qboolean restart ) {
	int i;

	// start the entity parsing at the beginning
	sv.entityParsePoint = CM_EntityString();

	// use the current msec count for a random seed
	// init for this gamestate
	gvm->GAME_INIT(svs.time, Com_Milliseconds(), restart );

	// clear all gentity pointers that might still be set from
	// a previous level
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		svs.clients[i].gentity = NULL;
	}
}



/*
===================
SV_RestartGameProgs

Called on a map_restart, but not on a normal map change
===================
*/
void SV_RestartGameProgs( void ) {
	if ( !gvm ) {
		return;
	}
	gvm->GAME_SHUTDOWN(qtrue );

	Sys_UnloadDll( gvmHandle );
	gvm = NULL;
	gvmHandle = NULL;
	void *entryPoint = NULL;
	GetGameAPI_t getAPI;
	static const gameImport_t imports = SV_CreateGameImports();
	gvmHandle = Sys_LoadDll( "qagame", "GetGameAPI", &entryPoint );
	getAPI = (GetGameAPI_t)entryPoint;
	gvm = getAPI ? getAPI( &imports ) : NULL;
	if ( !gvm ) {
		Com_Error( ERR_FATAL, "Could not reload the game function API" );
	}
	if ( gvm->apiVersion != GAME_API_VERSION ) {
		Com_Error( ERR_FATAL, "Game is version %d, expected %d", gvm->apiVersion, GAME_API_VERSION );
	}
	SV_InitGameVM( qtrue );
}


/*
===============
SV_InitGameProgs

Called on a normal map change, not on a map_restart
===============
*/
void SV_InitGameProgs( void ) {
	cvar_t  *var;
	//FIXME these are temp while I make bots run in vm
	extern int bot_enable;

	var = Cvar_Get( "bot_enable", "1", CVAR_LATCH );
	if ( var ) {
		bot_enable = var->integer;
	} else {
		bot_enable = 0;
	}

	void *entryPoint = NULL;
	GetGameAPI_t getAPI;
	static const gameImport_t imports = SV_CreateGameImports();
	gvmHandle = Sys_LoadDll( "qagame", "GetGameAPI", &entryPoint );
	getAPI = (GetGameAPI_t)entryPoint;
	gvm = getAPI ? getAPI( &imports ) : NULL;
	if ( !gvm ) {
		Com_Error( ERR_FATAL, "Could not load the game function API" );
	}
	if ( gvm->apiVersion != GAME_API_VERSION ) {
		Com_Error( ERR_FATAL, "Game is version %d, expected %d", gvm->apiVersion, GAME_API_VERSION );
	}

	SV_InitGameVM( qfalse );
}


/*
====================
SV_GameCommand

See if the current console command is claimed by the game
====================
*/
qboolean SV_GameCommand( void ) {
	if ( sv.state != SS_GAME ) {
		return qfalse;
	}

	return gvm->GAME_CONSOLE_COMMAND();
}


/*
====================
SV_SendMoveSpeedsToGame
====================
*/
void SV_SendMoveSpeedsToGame( int entnum, char *text ) {
	if ( !gvm ) {
		return;
	}
	gvm->GAME_RETRIEVE_MOVESPEEDS_FROM_CLIENT(entnum, text );
}

/*
====================
SV_GetTag

  return qfalse if unable to retrieve tag information for this client
====================
*/
extern qboolean CL_GetTag( int clientNum, char *tagname, orientation_t *or );

qboolean SV_GetTag( int clientNum, char *tagname, orientation_t *or ) {
	if ( com_dedicated->integer ) {
		return qfalse;
	}

	return CL_GetTag( clientNum, tagname, or );
}

/*
===================
SV_GetModelInfo

  request this modelinfo from the game
===================
*/
qboolean SV_GetModelInfo( int clientNum, char *modelName, animModelInfo_t **modelInfo ) {
	return gvm->GAME_GETMODELINFO( clientNum, modelName, (void **)modelInfo );
}
