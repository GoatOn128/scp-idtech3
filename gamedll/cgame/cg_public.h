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



#define CMD_BACKUP          64
#define CMD_MASK            ( CMD_BACKUP - 1 )
// allow a lot of command backups for very fast systems
// multiple commands may be combined into a single packet, so this
// needs to be larger than PACKET_BACKUP


#define MAX_ENTITIES_IN_SNAPSHOT    256

// snapshots are a view of the server at a given time

// Snapshots are generated at regular time intervals by the server,
// but they may not be sent if a client's rate level is exceeded, or
// they may be dropped by the network.
typedef struct {
	int snapFlags;                      // SNAPFLAG_RATE_DELAYED, etc
	int ping;

	int serverTime;                 // server time the message is valid for (in msec)

	byte areamask[MAX_MAP_AREA_BYTES];                  // portalarea visibility bits

	playerState_t ps;                       // complete information about the current player at this time

	int numEntities;                        // all of the entities that need to be presented
	entityState_t entities[MAX_ENTITIES_IN_SNAPSHOT];   // at the time of this snapshot

	int numServerCommands;                  // text based server commands to execute when this
	int serverCommandSequence;              // snapshot becomes current
} snapshot_t;

enum {
	CGAME_EVENT_NONE,
	CGAME_EVENT_TEAMMENU,
	CGAME_EVENT_SCOREBOARD,
	CGAME_EVENT_EDITHUD
};


/*
==================================================================

functions imported from the main executable

==================================================================
*/

#define CGAME_IMPORT_API_VERSION    3

typedef struct {
	void ( QDECL *Com_Printf )( const char *fmt );
	void ( QDECL *Com_Error )( const char *fmt );
	int ( QDECL *Sys_Milliseconds )( void );
	void ( QDECL *Cvar_Register )( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
	void ( QDECL *Cvar_Update )( vmCvar_t *vmCvar );
	void ( QDECL *Cvar_Set )( const char *var_name, const char *value );
	void ( QDECL *Cvar_VariableStringBuffer )( const char *var_name, char *buffer, int bufsize );
	int ( QDECL *Cmd_Argc )( void );
	void ( QDECL *Cmd_ArgvBuffer )( int n, char *buffer, int bufferLength );
	void ( QDECL *Cmd_ArgsBuffer )( char *buffer, int bufferLength );
	int ( QDECL *FS_FOpenFileByMode )( const char *qpath, fileHandle_t *f, fsMode_t mode );
	void ( QDECL *FS_Read )( void *buffer, int len, fileHandle_t f );
	void ( QDECL *FS_Write )( const void *buffer, int len, fileHandle_t f );
	void ( QDECL *FS_FCloseFile )( fileHandle_t f );
	void ( QDECL *Cbuf_AddText )( const char *text );
	void ( QDECL *CL_AddCgameCommand )( const char *cmdName );
	void ( QDECL *CL_AddReliableCommand )( const char *s );
	void ( QDECL *SCR_UpdateScreen )( void );
	void ( QDECL *CL_CM_LoadMap )( const char *mapname );
	int ( QDECL *CM_NumInlineModels )( void );
	clipHandle_t ( QDECL *CM_InlineModel )( int index );
	clipHandle_t ( QDECL *CM_TempBoxModel )( const vec3_t mins, const vec3_t maxs );
	clipHandle_t ( QDECL *CM_TempCapsuleModel )( const vec3_t mins, const vec3_t maxs );
	int ( QDECL *CM_PointContents )( const vec3_t p, clipHandle_t model );
	int ( QDECL *CM_TransformedPointContents )( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
	void ( QDECL *CM_BoxTrace )( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask );
	void ( QDECL *CM_TransformedBoxTrace )( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles );
	void ( QDECL *CM_CapsuleTrace )( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask );
	void ( QDECL *CM_TransformedCapsuleTrace )( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles );
	int ( QDECL *RE_MarkFragments )( int numPoints, const vec3_t *points, const vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer );
	void ( QDECL *S_StartSound )( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );
	void ( QDECL *S_StartSoundEx )( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx, int flags );
	void ( QDECL *S_StartLocalSound )( sfxHandle_t sfx, int channelNum );
	void ( QDECL *S_ClearLoopingSounds )( qboolean killall );
	void ( QDECL *S_AddLoopingSound )( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int volume );
	void ( QDECL *S_AddRangedLoopingSound )( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int range );
	void ( QDECL *S_AddRealLoopingSound )( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
	void ( QDECL *S_StopLoopingSound )( int entityNum );
	void ( QDECL *S_StopEntStreamingSound )( int entityNum );
	void ( QDECL *S_UpdateEntityPosition )( int entityNum, const vec3_t origin );
	int ( QDECL *S_GetVoiceAmplitude )( int entityNum );
	void ( QDECL *S_Respatialize )( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
	sfxHandle_t ( QDECL *S_RegisterSound )( const char *sample );
	void ( QDECL *S_StartBackgroundTrack )( const char *intro, const char *loop, int fadeupTime );
	void ( QDECL *S_FadeStreamingSound )( float targetvol, int time, int num );
	void ( QDECL *S_FadeAllSounds )( float targetvol, int time );
	void ( QDECL *S_StartStreamingSound )( const char *intro, const char *loop, int entnum, int channel, int attenuation );
	void ( QDECL *RE_LoadWorld )( const char *mapname );
	qhandle_t ( QDECL *RE_RegisterModel )( const char *name );
	qboolean ( QDECL *RE_GetSkinModel )( qhandle_t skinid, const char *type, char *name );
	qhandle_t ( QDECL *RE_GetShaderFromModel )( qhandle_t modelid, int surfnum, int withlightmap );
	qhandle_t ( QDECL *RE_RegisterSkin )( const char *name );
	qhandle_t ( QDECL *RE_RegisterShader )( const char *name );
	qhandle_t ( QDECL *RE_RegisterShaderNoMip )( const char *name );
	void ( QDECL *RE_RegisterFont )( const char *fontName, int pointSize, fontInfo_t *font );
	void ( QDECL *RE_ClearScene )( void );
	void ( QDECL *RE_AddRefEntityToScene )( const refEntity_t *re );
	void ( QDECL *RE_AddPolyToScene )( qhandle_t hShader, int numVerts, const polyVert_t *verts );
	void ( QDECL *RE_AddPolysToScene )( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys );
	void ( QDECL *RE_ZombieFXAddNewHit )( int entityNum, const vec3_t hitPos, const vec3_t hitDir );
	void ( QDECL *RE_AddLightToScene )( const vec3_t org, float intensity, float r, float g, float b, int overdraw );
	void ( QDECL *RE_AddCoronaToScene )( const vec3_t org, float r, float g, float b, float scale, int id, int flags );
	void ( QDECL *RE_SetFog )( int fogvar, int var1, int var2, float r, float g, float b, float density );
	void ( QDECL *RE_RenderScene )( const refdef_t *fd );
	void ( QDECL *RE_SetColor )( const float *rgba );
	void ( QDECL *RE_DrawStretchPic )( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
	void ( QDECL *RE_DrawStretchPicGradient )( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader, const float *gradientColor, int gradientType );
	void ( QDECL *RE_ModelBounds )( clipHandle_t model, vec3_t mins, vec3_t maxs );
	int ( QDECL *RE_LerpTag )( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex );
	void ( QDECL *RE_RemapShader )( const char *oldShader, const char *newShader, const char *timeOffset );
	void ( QDECL *CL_GetGlconfig )( glconfig_t *glconfig );
	void ( QDECL *CL_GetGameState )( gameState_t *gamestate );
	void ( QDECL *CL_GetCurrentSnapshotNumber )( int *snapshotNumber, int *serverTime );
	qboolean ( QDECL *CL_GetSnapshot )( int snapshotNumber, snapshot_t *snapshot );
	qboolean ( QDECL *CL_GetServerCommand )( int serverCommandNumber );
	int ( QDECL *CL_GetCurrentCmdNumber )( void );
	qboolean ( QDECL *CL_GetUserCmd )( int cmdNumber, usercmd_t *ucmd );
	void ( QDECL *CL_SetUserCmdValue )( int stateValue, int holdableValue, float sensitivityScale, int cld );
	int ( QDECL *Hunk_MemoryRemaining )( void );
	qboolean ( QDECL *loadCamera )( int camNum, const char *name );
	void ( QDECL *startCamera )( int camNum, int time );
	void ( QDECL *CL_StopCamera )( int camNum );
	qboolean ( QDECL *getCameraInfo )( int camNum, int time, vec3_t *origin, vec3_t *angles, float *fov );
	qboolean ( QDECL *Key_IsDown )( int keynum );
	int ( QDECL *Key_GetCatcher )( void );
	void ( QDECL *Key_SetCatcher )( int catcher );
	int ( QDECL *Key_GetKey )( const char *binding );
	int ( QDECL *PC_AddGlobalDefine )( char *define );
	int ( QDECL *PC_LoadSourceHandle )( const char *filename );
	int ( QDECL *PC_FreeSourceHandle )( int handle );
	int ( QDECL *PC_ReadTokenHandle )( int handle, pc_token_t *pc_token );
	int ( QDECL *PC_SourceFileAndLine )( int handle, char *filename, int *line );
	void ( QDECL *S_StopBackgroundTrack )( void );
	int ( QDECL *Com_RealTime )( qtime_t *qtime );
	void ( QDECL *Sys_SnapVector )( float *v );
	void ( QDECL *SV_SendMoveSpeedsToGame )( int entnum, char *movespeeds );
	int ( QDECL *CIN_PlayCinematic )( const char *arg0, int xpos, int ypos, int width, int height, int bits );
	e_status ( QDECL *CIN_StopCinematic )( int handle );
	e_status ( QDECL *CIN_RunCinematic )( int handle );
	void ( QDECL *CIN_DrawCinematic )( int handle );
	void ( QDECL *CIN_SetExtents )( int handle, int x, int y, int w, int h );
	qboolean ( QDECL *RE_GetEntityToken )( char *buffer, int bufferSize );
	void ( QDECL *Com_PrintfInt )( char *string, int value );
	void ( QDECL *Com_PrintfFloat )( char *string, float value );
	void ( QDECL *CL_InGamePopup )( const char *arg0 );
	void ( QDECL *CL_AddToLimboChat )( const char *arg0 );
	void ( QDECL *CL_ClosePopup )( const char *arg0 );
	qboolean ( QDECL *SV_GetModelInfo )( int clientNum, char *modelName, void **modelInfo );
} cgameImport_t;


/*
==================================================================

functions exported to the main executable

==================================================================
*/

typedef struct {
	int apiVersion;
	void ( *CG_INIT )( int serverMessageNum, int serverCommandSequence );
	void ( *CG_SHUTDOWN )( void );
	qboolean ( *CG_CONSOLE_COMMAND )( void );
	void ( *CG_DRAW_ACTIVE_FRAME )( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );
	int ( *CG_CROSSHAIR_PLAYER )( void );
	int ( *CG_LAST_ATTACKER )( void );
	void ( *CG_KEY_EVENT )( int key, qboolean down );
	void ( *CG_MOUSE_EVENT )( int dx, int dy );
	void ( *CG_EVENT_HANDLING )( int type );
	qboolean ( *CG_GET_TAG )( int clientNum, char *tagname, orientation_t *orientation );
} cgameExport_t;

typedef const cgameExport_t *( QDECL *GetCGameAPI_t )( const cgameImport_t *imports );

//----------------------------------------------
