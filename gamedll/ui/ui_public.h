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

#ifndef __UI_PUBLIC_H__
#define __UI_PUBLIC_H__

#define UI_API_VERSION  4

typedef struct {
	connstate_t connState;
	int connectPacketCount;
	int clientNum;
	char servername[MAX_STRING_CHARS];
	char updateInfoString[MAX_STRING_CHARS];
	char messageString[MAX_STRING_CHARS];
} uiClientState_t;

typedef struct {
	void ( QDECL *Com_Printf )( const char *string );
	void ( QDECL *Com_Error )( const char *string );
	int ( QDECL *Sys_Milliseconds )( void );
	void ( QDECL *Cvar_Register )( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
	void ( QDECL *Cvar_Update )( vmCvar_t *cvar );
	void ( QDECL *Cvar_Set )( const char *var_name, const char *value );
	float ( QDECL *Cvar_VariableValue )( const char *var_name );
	void ( QDECL *Cvar_VariableStringBuffer )( const char *var_name, char *buffer, int bufsize );
	void ( QDECL *Cvar_SetValue )( const char *var_name, float value );
	void ( QDECL *Cvar_Reset )( const char *name );
	void ( QDECL *Cvar_Get )( const char *var_name, const char *var_value, int flags );
	void ( QDECL *Cvar_InfoStringBuffer )( int bit, char *buffer, int bufsize );
	int ( QDECL *Cmd_Argc )( void );
	void ( QDECL *Cmd_ArgvBuffer )( int n, char *buffer, int bufferLength );
	void ( QDECL *Cbuf_ExecuteText )( int exec_when, const char *text );
	int ( QDECL *FS_FOpenFileByMode )( const char *qpath, fileHandle_t *f, fsMode_t mode );
	void ( QDECL *FS_Read )( void *buffer, int len, fileHandle_t f );
	void ( QDECL *FS_Seek )( fileHandle_t f, long offset, int origin );
	void ( QDECL *FS_Write )( const void *buffer, int len, fileHandle_t f );
	void ( QDECL *FS_FCloseFile )( fileHandle_t f );
	int ( QDECL *FS_GetFileList )( const char *path, const char *extension, char *listbuf, int bufsize );
	int ( QDECL *FS_Delete )( const char *filename );
	qhandle_t ( QDECL *RE_RegisterModel )( const char *name );
	qhandle_t ( QDECL *RE_RegisterSkin )( const char *name );
	void ( QDECL *RE_RegisterFont )( const char *fontName, int pointSize, fontInfo_t *font );
	qhandle_t ( QDECL *RE_RegisterShaderNoMip )( const char *name );
	void ( QDECL *RE_ClearScene )( void );
	void ( QDECL *RE_AddRefEntityToScene )( const refEntity_t *re );
	void ( QDECL *RE_AddPolyToScene )( qhandle_t hShader, int numVerts, const polyVert_t *verts );
	void ( QDECL *RE_AddLightToScene )( const vec3_t org, float intensity, float r, float g, float b, int overdraw );
	void ( QDECL *RE_AddCoronaToScene )( const vec3_t org, float r, float g, float b, float scale, int id, int flags );
	void ( QDECL *RE_RenderScene )( const refdef_t *fd );
	void ( QDECL *RE_SetColor )( const float *rgba );
	void ( QDECL *RE_DrawStretchPic )( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
	void ( QDECL *RE_ModelBounds )( clipHandle_t model, vec3_t mins, vec3_t maxs );
	void ( QDECL *SCR_UpdateScreen )( void );
	int ( QDECL *RE_LerpTag )( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex );
	void ( QDECL *S_StartLocalSound )( sfxHandle_t sfx, int channelNum );
	sfxHandle_t ( QDECL *S_RegisterSound )( const char *sample );
	void ( QDECL *S_FadeStreamingSound )( float targetvol, int time, int num );
	void ( QDECL *S_FadeAllSounds )( float targetvol, int time );
	void ( QDECL *Key_KeynumToStringBuf )( int keynum, char *buf, int buflen );
	void ( QDECL *Key_GetBindingBuf )( int keynum, char *buf, int buflen );
	void ( QDECL *Key_SetBinding )( int keynum, const char *binding );
	qboolean ( QDECL *Key_IsDown )( int keynum );
	qboolean ( QDECL *Key_GetOverstrikeMode )( void );
	void ( QDECL *Key_SetOverstrikeMode )( qboolean state );
	void ( QDECL *Key_ClearStates )( void );
	int ( QDECL *Key_GetCatcher )( void );
	void ( QDECL *Key_SetCatcher )( int catcher );
	void ( QDECL *GetClipboardData )( char *buf, int bufsize );
	void ( QDECL *GetClientState )( uiClientState_t *state );
	void ( QDECL *CL_GetGlconfig )( glconfig_t *glconfig );
	int ( QDECL *GetConfigString )( int index, char* buff, int buffsize );
	int ( QDECL *LAN_GetLocalServerCount )( void );
	void ( QDECL *LAN_GetLocalServerAddressString )( int n, char *buf, int buflen );
	int ( QDECL *LAN_GetGlobalServerCount )( void );
	void ( QDECL *LAN_GetGlobalServerAddressString )( int n, char *buf, int buflen );
	int ( QDECL *LAN_GetPingQueueCount )( void );
	void ( QDECL *LAN_ClearPing )( int n );
	void ( QDECL *LAN_GetPing )( int n, char *buf, int buflen, int *pingtime );
	void ( QDECL *LAN_GetPingInfo )( int n, char *buf, int buflen );
	qboolean ( QDECL *LAN_UpdateVisiblePings )( int source );
	int ( QDECL *LAN_GetServerCount )( int source );
	int ( QDECL *LAN_CompareServers )( int source, int sortKey, int sortDir, int s1, int s2 );
	void ( QDECL *LAN_GetServerAddressString )( int source, int n, char *buf, int buflen );
	void ( QDECL *LAN_GetServerInfo )( int source, int n, char *buf, int buflen );
	int ( QDECL *LAN_AddServer )( int source, const char *name, const char *addr );
	void ( QDECL *LAN_RemoveServer )( int source, const char *addr );
	int ( QDECL *LAN_GetServerPing )( int source, int n );
	int ( QDECL *LAN_ServerIsVisible )( int source, int n );
	int ( QDECL *LAN_GetServerStatus )( const char *serverAddress, char *serverStatus, int maxLen );
	void ( QDECL *LAN_SaveServersToCache )( void );
	void ( QDECL *LAN_LoadCachedServers )( void );
	void ( QDECL *LAN_MarkServerVisible )( int source, int n, qboolean visible );
	void ( QDECL *LAN_ResetPings )( int n );
	int ( QDECL *Hunk_MemoryRemaining )( void );
	void ( QDECL *CLUI_GetCDKey )( char *buf, int buflen );
	void ( QDECL *CLUI_SetCDKey )( char *buf );
	int ( QDECL *PC_AddGlobalDefine )( char *define );
	int ( QDECL *PC_LoadSourceHandle )( const char *filename );
	int ( QDECL *PC_FreeSourceHandle )( int handle );
	int ( QDECL *PC_ReadTokenHandle )( int handle, pc_token_t *pc_token );
	int ( QDECL *PC_SourceFileAndLine )( int handle, char *filename, int *line );
	void ( QDECL *S_StopBackgroundTrack )( void );
	void ( QDECL *S_StartBackgroundTrack )( const char *intro, const char *loop, int fadeupTime );
	int ( QDECL *Com_RealTime )( qtime_t *qtime );
	int ( QDECL *CIN_PlayCinematic )( const char *arg0, int xpos, int ypos, int width, int height, int bits );
	e_status ( QDECL *CIN_StopCinematic )( int handle );
	e_status ( QDECL *CIN_RunCinematic )( int handle );
	void ( QDECL *CIN_DrawCinematic )( int handle );
	void ( QDECL *CIN_SetExtents )( int handle, int x, int y, int w, int h );
	void ( QDECL *RE_RemapShader )( const char *oldShader, const char *newShader, const char *timeOffset );
	qboolean ( QDECL *CL_CDKeyValidate )( const char *key, const char *chksum );
	qboolean ( QDECL *CL_GetLimboString )( int index, char *buf );
} uiImport_t;

typedef enum {
	UIMENU_NONE,
	UIMENU_MAIN,
	UIMENU_INGAME,
	UIMENU_NEED_CD,
	UIMENU_ENDGAME,
	UIMENU_BAD_CD_KEY,
	UIMENU_TEAM,
	UIMENU_PREGAME,
	UIMENU_POSTGAME,
	UIMENU_NOTEBOOK,
	UIMENU_CLIPBOARD,
	UIMENU_HELP,
	UIMENU_BOOK1,
	UIMENU_BOOK2,
	UIMENU_BOOK3,
	UIMENU_WM_PICKTEAM,
	UIMENU_WM_PICKPLAYER,
	UIMENU_WM_QUICKMESSAGE,
	UIMENU_WM_LIMBO,
	UIMENU_BRIEFING
} uiMenuCommand_t;

#define SORT_HOST           0
#define SORT_MAP            1
#define SORT_CLIENTS        2
#define SORT_GAME           3
#define SORT_PING           4

#define SORT_SAVENAME       0
#define SORT_SAVETIME       1

typedef struct {
	int apiVersion;
	void ( *UI_INIT )( qboolean inGameLoad );
	void ( *UI_SHUTDOWN )( void );
	void ( *UI_KEY_EVENT )( int key, qboolean down );
	void ( *UI_MOUSE_EVENT )( int dx, int dy );
	void ( *UI_REFRESH )( int realtime );
	qboolean ( *UI_IS_FULLSCREEN )( void );
	void ( *UI_SET_ACTIVE_MENU )( uiMenuCommand_t menu );
	uiMenuCommand_t ( *UI_GET_ACTIVE_MENU )( void );
	qboolean ( *UI_CONSOLE_COMMAND )( int realtime );
	void ( *UI_DRAW_CONNECT_SCREEN )( qboolean overlay );
	qboolean ( *UI_HASUNIQUECDKEY )( void );
} uiExport_t;

typedef const uiExport_t *( QDECL *GetUIAPI_t )( const uiImport_t *imports );

#endif
