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

// g_public.h -- game module information visible to server

#define GAME_API_VERSION    8

typedef struct gentity_s gentity_t;
struct bot_entitystate_s;
struct aas_entityinfo_s;
struct aas_clientmove_s;
struct bot_input_s;
struct bot_consolemessage_s;
struct bot_match_s;
struct bot_goal_s;
struct bot_moveresult_s;
struct bot_initmove_s;
struct weaponinfo_s;

// entity->svFlags
// the server does not know how to interpret most of the values
// in entityStates (level eType), so the game must explicitly flag
// special server behaviors
#define SVF_NOCLIENT            0x00000001  // don't send entity to clients, even if it has effects
#define SVF_VISDUMMY            0x00000004  // this ent is a "visibility dummy" and needs it's master to be sent to clients that can see it even if they can't see the master ent
#define SVF_BOT                 0x00000008
// Wolfenstein
#define SVF_CASTAI              0x00000010
// done.
#define SVF_BROADCAST           0x00000020  // send to all connected clients
#define SVF_PORTAL              0x00000040  // merge a second pvs at origin2 into snapshots
#define SVF_USE_CURRENT_ORIGIN  0x00000080  // entity->r.currentOrigin instead of entity->s.origin
											// for link position (missiles and movers)
// Ridah
#define SVF_NOFOOTSTEPS         0x00000100
// done.
// MrE:
#define SVF_CAPSULE             0x00000200  // use capsule for collision detection

#define SVF_VISDUMMY_MULTIPLE   0x00000400  // so that one vis dummy can add to snapshot multiple speakers

// recent id changes
#define SVF_SINGLECLIENT        0x00000800  // only send to a single client (entityShared_t->singleClient)
#define SVF_NOSERVERINFO        0x00001000  // don't send CS_SERVERINFO updates to this client
											// so that it can be updated for ping tools without
											// lagging clients
#define SVF_NOTSINGLECLIENT     0x00002000  // send entity to everyone but one client
											// (entityShared_t->singleClient)

//===============================================================


typedef struct {
	entityState_t s;                // communicated by server to clients

	qboolean linked;                // qfalse if not in any good cluster
	int linkcount;

	int svFlags;                    // SVF_NOCLIENT, SVF_BROADCAST, etc
	int singleClient;               // only send to this client when SVF_SINGLECLIENT is set

	qboolean bmodel;                // if false, assume an explicit mins / maxs bounding box
									// only set by trap_SetBrushModel
	vec3_t mins, maxs;
	int contents;                   // CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
									// a non-solid entity should set to 0

	vec3_t absmin, absmax;          // derived from mins/maxs and origin + rotation

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t currentOrigin;
	vec3_t currentAngles;

	// when a trace call is made and passEntityNum != ENTITYNUM_NONE,
	// an ent will be excluded from testing if:
	// ent->s.number == passEntityNum	(don't interact with self)
	// ent->s.ownerNum = passEntityNum	(don't interact with your own missiles)
	// entity[ent->s.ownerNum].ownerNum = passEntityNum	(don't interact with other missiles from owner)
	int ownerNum;
	int eventTime;
} entityShared_t;



// the server looks at a sharedEntity, which is the start of the game's gentity_t structure
typedef struct {
	entityState_t s;                // communicated by server to clients
	entityShared_t r;               // shared by both the server system and game
} sharedEntity_t;



//===============================================================

//
// system traps provided by the main engine
//
typedef struct {
	void ( QDECL *Com_Printf )( const char *fmt );
	void ( QDECL *Com_Error )( const char *fmt );
	void ( QDECL *Com_ErrorEndGame )( void );
	int ( QDECL *Sys_Milliseconds )( void );
	int ( QDECL *Cmd_Argc )( void );
	void ( QDECL *Cmd_ArgvBuffer )( int n, char *buffer, int bufferLength );
	int ( QDECL *FS_FOpenFileByMode )( const char *qpath, fileHandle_t *f, fsMode_t mode );
	void ( QDECL *FS_Read )( void *buffer, int len, fileHandle_t f );
	int ( QDECL *FS_Write )( const void *buffer, int len, fileHandle_t f );
	int ( QDECL *FS_Rename )( const char *from, const char *to );
	void ( QDECL *FS_FCloseFile )( fileHandle_t f );
	void ( QDECL *FS_CopyFileOS )( char *from, char *to );
	int ( QDECL *FS_GetFileList )( const char *path, const char *extension, char *listbuf, int bufsize );
	void ( QDECL *Cbuf_ExecuteText )( int exec_when, const char *text );
	void ( QDECL *SV_LocateGameData )( sharedEntity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *clients, int sizeofGClient );
	void ( QDECL *Cvar_Register )( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
	void ( QDECL *Cvar_Update )( vmCvar_t *cvar );
	void ( QDECL *Cvar_Set )( const char *var_name, const char *value );
	int ( QDECL *Cvar_VariableIntegerValue )( const char *var_name );
	void ( QDECL *Cvar_VariableStringBuffer )( const char *var_name, char *buffer, int bufsize );
	void ( QDECL *SV_GameDropClient )( int clientNum, const char *reason );
	void ( QDECL *SV_GameSendServerCommand )( int clientNum, const char *text );
	void ( QDECL *SV_SetConfigstring )( int num, const char *string );
	void ( QDECL *SV_GetConfigstring )( int num, char *buffer, int bufferSize );
	void ( QDECL *SV_GetUserinfo )( int num, char *buffer, int bufferSize );
	void ( QDECL *SV_SetUserinfo )( int num, const char *buffer );
	void ( QDECL *SV_GetServerinfo )( char *buffer, int bufferSize );
	void ( QDECL *SV_SetBrushModel )( gentity_t *ent, const char *name );
	void ( QDECL *SV_Trace )( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
	void ( QDECL *SV_TraceCapsule )( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
	int ( QDECL *SV_PointContents )( const vec3_t point, int passEntityNum );
	qboolean ( QDECL *SV_inPVS )( const vec3_t p1, const vec3_t p2 );
	qboolean ( QDECL *SV_inPVSIgnorePortals )( const vec3_t p1, const vec3_t p2 );
	void ( QDECL *SV_AdjustAreaPortalState )( gentity_t *ent, qboolean open );
	qboolean ( QDECL *CM_AreasConnected )( int area1, int area2 );
	void ( QDECL *SV_LinkEntity )( gentity_t *ent );
	void ( QDECL *SV_UnlinkEntity )( gentity_t *ent );
	int ( QDECL *SV_AreaEntities )( const vec3_t mins, const vec3_t maxs, int *list, int maxcount );
	qboolean ( QDECL *SV_EntityContact )( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
	qboolean ( QDECL *SV_EntityContactCapsule )( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
	int ( QDECL *SV_BotAllocateClient )( void );
	void ( QDECL *SV_BotFreeClient )( int clientNum );
	void ( QDECL *SV_GetUsercmd )( int clientNum, usercmd_t *cmd );
	qboolean ( QDECL *SV_GetEntityToken )( char *buffer, int bufferSize );
	int ( QDECL *BotImport_DebugPolygonCreate )( int color, int numPoints, vec3_t *points );
	void ( QDECL *BotImport_DebugPolygonDelete )( int id );
	int ( QDECL *Com_RealTime )( qtime_t *qtime );
	void ( QDECL *Sys_SnapVector )( float *v );
	qboolean ( QDECL *SV_GetTag )( int clientNum, char *tagName, orientation_t *or );
	int ( QDECL *SV_BotLibSetup )( void );
	int ( QDECL *SV_BotLibShutdown )( void );
	int ( QDECL *BotLibVarSet )( char *var_name, char *value );
	int ( QDECL *BotLibVarGet )( char *var_name, char *value, int size );
	int ( QDECL *PC_AddGlobalDefine )( char *string );
	int ( QDECL *BotLibStartFrame )( float time );
	int ( QDECL *BotLibLoadMap )( const char *mapname );
	int ( QDECL *BotLibUpdateEntity )( int ent, struct bot_entitystate_s *bue );
	int ( QDECL *Test )( int parm0, char *parm1, vec3_t parm2, vec3_t parm3 );
	int ( QDECL *SV_BotGetSnapshotEntity )( int clientNum, int sequence );
	int ( QDECL *SV_BotGetConsoleMessage )( int clientNum, char *message, int size );
	void ( QDECL *SV_ClientThink )( int clientNum, usercmd_t *ucmd );
	void ( QDECL *AAS_EntityInfo )( int entnum, struct aas_entityinfo_s *info );
	int ( QDECL *AAS_Initialized )( void );
	void ( QDECL *AAS_PresenceTypeBoundingBox )( int presencetype, vec3_t mins, vec3_t maxs );
	float ( QDECL *AAS_Time )( void );
	void ( QDECL *AAS_SetCurrentWorld )( int index );
	int ( QDECL *AAS_PointAreaNum )( vec3_t point );
	int ( QDECL *AAS_TraceAreas )( vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas );
	int ( QDECL *AAS_PointContents )( vec3_t point );
	int ( QDECL *AAS_NextBSPEntity )( int ent );
	int ( QDECL *AAS_ValueForBSPEpairKey )( int ent, char *key, char *value, int size );
	int ( QDECL *AAS_VectorForBSPEpairKey )( int ent, char *key, vec3_t v );
	int ( QDECL *AAS_FloatForBSPEpairKey )( int ent, char *key, float *value );
	int ( QDECL *AAS_IntForBSPEpairKey )( int ent, char *key, int *value );
	int ( QDECL *AAS_AreaReachability )( int areanum );
	int ( QDECL *AAS_AreaTravelTimeToGoalArea )( int areanum, vec3_t origin, int goalareanum, int travelflags );
	int ( QDECL *AAS_Swimming )( vec3_t origin );
	int ( QDECL *AAS_PredictClientMovement )( struct aas_clientmove_s *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize );
	void ( QDECL *AAS_RT_ShowRoute )( vec3_t srcpos, int srcnum, int destnum );
	qboolean ( QDECL *AAS_RT_GetHidePos )( vec3_t srcpos, int srcnum, int srcarea, vec3_t destpos, int destnum, int destarea, vec3_t returnPos );
	int ( QDECL *AAS_FindAttackSpotWithinRange )( int srcnum, int rangenum, int enemynum, float rangedist, int travelflags, float *outpos );
	qboolean ( QDECL *AAS_GetRouteFirstVisPos )( vec3_t srcpos, vec3_t destpos, int travelflags, vec3_t retpos );
	void ( QDECL *AAS_SetAASBlockingEntity )( vec3_t absmin, vec3_t absmax, qboolean blocking );
	void ( QDECL *EA_Say )( int client, char *str );
	void ( QDECL *EA_SayTeam )( int client, char *str );
	void ( QDECL *EA_UseItem )( int client, char *it );
	void ( QDECL *EA_DropItem )( int client, char *it );
	void ( QDECL *EA_UseInv )( int client, char *inv );
	void ( QDECL *EA_DropInv )( int client, char *inv );
	void ( QDECL *EA_Gesture )( int client );
	void ( QDECL *EA_Command )( int client, char *command );
	void ( QDECL *EA_SelectWeapon )( int client, int weapon );
	void ( QDECL *EA_Talk )( int client );
	void ( QDECL *EA_Attack )( int client );
	void ( QDECL *EA_Reload )( int client );
	void ( QDECL *EA_Use )( int client );
	void ( QDECL *EA_Respawn )( int client );
	void ( QDECL *EA_Jump )( int client );
	void ( QDECL *EA_DelayedJump )( int client );
	void ( QDECL *EA_Crouch )( int client );
	void ( QDECL *EA_MoveUp )( int client );
	void ( QDECL *EA_MoveDown )( int client );
	void ( QDECL *EA_MoveForward )( int client );
	void ( QDECL *EA_MoveBack )( int client );
	void ( QDECL *EA_MoveLeft )( int client );
	void ( QDECL *EA_MoveRight )( int client );
	void ( QDECL *EA_Move )( int client, vec3_t dir, float speed );
	void ( QDECL *EA_View )( int client, vec3_t viewangles );
	void ( QDECL *EA_EndRegular )( int client, float thinktime );
	void ( QDECL *EA_GetInput )( int client, float thinktime, struct bot_input_s *input );
	void ( QDECL *EA_ResetInput )( int client, struct bot_input_s *init );
	int ( QDECL *BotLoadCharacter )( char *charfile, int skill );
	void ( QDECL *BotFreeCharacter )( int character );
	float ( QDECL *Characteristic_Float )( int character, int index );
	float ( QDECL *Characteristic_BFloat )( int character, int index, float min, float max );
	int ( QDECL *Characteristic_Integer )( int character, int index );
	int ( QDECL *Characteristic_BInteger )( int character, int index, int min, int max );
	void ( QDECL *Characteristic_String )( int character, int index, char *buf, int size );
	int ( QDECL *BotAllocChatState )( void );
	void ( QDECL *BotFreeChatState )( int handle );
	void ( QDECL *BotQueueConsoleMessage )( int chatstate, int type, char *message );
	void ( QDECL *BotRemoveConsoleMessage )( int chatstate, int handle );
	int ( QDECL *BotNextConsoleMessage )( int chatstate, struct bot_consolemessage_s *cm );
	int ( QDECL *BotNumConsoleMessages )( int chatstate );
	void ( QDECL *BotInitialChat )( int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
	int ( QDECL *BotNumInitialChats )( int chatstate, char *type );
	int ( QDECL *BotReplyChat )( int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
	int ( QDECL *BotChatLength )( int chatstate );
	void ( QDECL *BotEnterChat )( int chatstate, int client, int sendto );
	void ( QDECL *BotGetChatMessage )( int chatstate, char *buf, int size );
	int ( QDECL *StringContains )( char *str1, char *str2, int casesensitive );
	int ( QDECL *BotFindMatch )( char *str, struct bot_match_s *match, unsigned long int context );
	void ( QDECL *BotMatchVariable )( struct bot_match_s *match, int variable, char *buf, int size );
	void ( QDECL *UnifyWhiteSpaces )( char *string );
	void ( QDECL *BotReplaceSynonyms )( char *string, unsigned long int context );
	int ( QDECL *BotLoadChatFile )( int chatstate, char *chatfile, char *chatname );
	void ( QDECL *BotSetChatGender )( int chatstate, int gender );
	void ( QDECL *BotSetChatName )( int chatstate, char *name );
	void ( QDECL *BotResetGoalState )( int goalstate );
	void ( QDECL *BotResetAvoidGoals )( int goalstate );
	void ( QDECL *BotRemoveFromAvoidGoals )( int goalstate, int number );
	void ( QDECL *BotPushGoal )( int goalstate, struct bot_goal_s *goal );
	void ( QDECL *BotPopGoal )( int goalstate );
	void ( QDECL *BotEmptyGoalStack )( int goalstate );
	void ( QDECL *BotDumpAvoidGoals )( int goalstate );
	void ( QDECL *BotDumpGoalStack )( int goalstate );
	void ( QDECL *BotGoalName )( int number, char *name, int size );
	int ( QDECL *BotGetTopGoal )( int goalstate, struct bot_goal_s *goal );
	int ( QDECL *BotGetSecondGoal )( int goalstate, struct bot_goal_s *goal );
	int ( QDECL *BotChooseLTGItem )( int goalstate, vec3_t origin, int *inventory, int travelflags );
	int ( QDECL *BotChooseNBGItem )( int goalstate, vec3_t origin, int *inventory, int travelflags, struct bot_goal_s *ltg, float maxtime );
	int ( QDECL *BotTouchingGoal )( vec3_t origin, struct bot_goal_s *goal );
	int ( QDECL *BotItemGoalInVisButNotVisible )( int viewer, vec3_t eye, vec3_t viewangles, struct bot_goal_s *goal );
	int ( QDECL *BotGetLevelItemGoal )( int index, char *classname, struct bot_goal_s *goal );
	int ( QDECL *BotGetNextCampSpotGoal )( int num, struct bot_goal_s *goal );
	int ( QDECL *BotGetMapLocationGoal )( char *name, struct bot_goal_s *goal );
	float ( QDECL *BotAvoidGoalTime )( int goalstate, int number );
	void ( QDECL *BotInitLevelItems )( void );
	void ( QDECL *BotUpdateEntityItems )( void );
	int ( QDECL *BotLoadItemWeights )( int goalstate, char *filename );
	void ( QDECL *BotFreeItemWeights )( int goalstate );
	void ( QDECL *BotInterbreedGoalFuzzyLogic )( int parent1, int parent2, int child );
	void ( QDECL *BotSaveGoalFuzzyLogic )( int goalstate, char *filename );
	void ( QDECL *BotMutateGoalFuzzyLogic )( int goalstate, float range );
	int ( QDECL *BotAllocGoalState )( int state );
	void ( QDECL *BotFreeGoalState )( int handle );
	void ( QDECL *BotResetMoveState )( int movestate );
	void ( QDECL *BotMoveToGoal )( struct bot_moveresult_s *result, int movestate, struct bot_goal_s *goal, int travelflags );
	int ( QDECL *BotMoveInDirection )( int movestate, vec3_t dir, float speed, int type );
	void ( QDECL *BotResetAvoidReach )( int movestate );
	void ( QDECL *BotResetLastAvoidReach )( int movestate );
	int ( QDECL *BotReachabilityArea )( vec3_t origin, int testground );
	int ( QDECL *BotMovementViewTarget )( int movestate, struct bot_goal_s *goal, int travelflags, float lookahead, vec3_t target );
	int ( QDECL *BotPredictVisiblePosition )( vec3_t origin, int areanum, struct bot_goal_s *goal, int travelflags, vec3_t target );
	int ( QDECL *BotAllocMoveState )( void );
	void ( QDECL *BotFreeMoveState )( int handle );
	void ( QDECL *BotInitMoveState )( int handle, struct bot_initmove_s *initmove );
	void ( QDECL *BotInitAvoidReach )( int handle );
	int ( QDECL *BotChooseBestFightWeapon )( int weaponstate, int *inventory );
	void ( QDECL *BotGetWeaponInfo )( int weaponstate, int weapon, struct weaponinfo_s *weaponinfo );
	int ( QDECL *BotLoadWeaponWeights )( int weaponstate, char *filename );
	int ( QDECL *BotAllocWeaponState )( void );
	void ( QDECL *BotFreeWeaponState )( int weaponstate );
	void ( QDECL *BotResetWeaponState )( int weaponstate );
	int ( QDECL *GeneticParentsAndChildSelection )( int numranks, float *ranks, int *parent1, int *parent2, int *child );
} gameImport_t;


//
// functions exported by the game subsystem
//
typedef struct {
	int apiVersion;
	void ( *GAME_INIT )( int levelTime, int randomSeed, int restart );
	void ( *GAME_SHUTDOWN )( int restart );
	char *( *GAME_CLIENT_CONNECT )( int clientNum, qboolean firstTime, qboolean isBot );
	void ( *GAME_CLIENT_BEGIN )( int clientNum );
	void ( *GAME_CLIENT_USERINFO_CHANGED )( int clientNum );
	void ( *GAME_CLIENT_DISCONNECT )( int clientNum );
	void ( *GAME_CLIENT_COMMAND )( int clientNum );
	void ( *GAME_CLIENT_THINK )( int clientNum );
	void ( *GAME_RUN_FRAME )( int levelTime );
	qboolean ( *GAME_CONSOLE_COMMAND )( void );
	int ( *BOTAI_START_FRAME )( int time );
	qboolean ( *AICAST_VISIBLEFROMPOS )( vec3_t srcpos, int srcnum, vec3_t destpos, int destnum, qboolean updateVisPos );
	qboolean ( *AICAST_CHECKATTACKATPOS )( int entnum, int enemy, vec3_t pos, qboolean ducking, qboolean allowHitWorld );
	void ( *GAME_RETRIEVE_MOVESPEEDS_FROM_CLIENT )( int entnum, char *text );
	qboolean ( *GAME_GETMODELINFO )( int clientNum, char *modelName, void **modelInfo );
} gameExport_t;

typedef const gameExport_t *( QDECL *GetGameAPI_t )( const gameImport_t *imports );
