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

#include "ui_local.h"

// Native module bindings supplied through the typed UI import table.

static const uiImport_t *uiImports;

void UI_SetImports( const uiImport_t *imports ) {
	uiImports = imports;
}


void trap_Print( const char *string ) {
	uiImports->Com_Printf( string );
}

void trap_Error( const char *string ) {
	uiImports->Com_Error( string );
}

int trap_Milliseconds( void ) {
	return uiImports->Sys_Milliseconds(  );
}

void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	uiImports->Cvar_Register( cvar, var_name, value, flags );
}

void trap_Cvar_Update( vmCvar_t *cvar ) {
	uiImports->Cvar_Update( cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	uiImports->Cvar_Set( var_name, value );
}

float trap_Cvar_VariableValue( const char *var_name ) {
	return uiImports->Cvar_VariableValue( var_name );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	uiImports->Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}

void trap_Cvar_SetValue( const char *var_name, float value ) {
	uiImports->Cvar_SetValue( var_name, value );
}

void trap_Cvar_Reset( const char *name ) {
	uiImports->Cvar_Reset( name );
}

void trap_Cvar_Create( const char *var_name, const char *var_value, int flags ) {
	uiImports->Cvar_Get( var_name, var_value, flags );
}

void trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize ) {
	uiImports->Cvar_InfoStringBuffer( bit, buffer, bufsize );
}

int trap_Argc( void ) {
	return uiImports->Cmd_Argc(  );
}

void trap_Argv( int n, char *buffer, int bufferLength ) {
	uiImports->Cmd_ArgvBuffer( n, buffer, bufferLength );
}

void trap_Cmd_ExecuteText( int exec_when, const char *text ) {
	uiImports->Cbuf_ExecuteText( exec_when, text );
}

int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return uiImports->FS_FOpenFileByMode( qpath, f, mode );
}

void trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	uiImports->FS_Read( buffer, len, f );
}

//----(SA)	added
void trap_FS_Seek( fileHandle_t f, long offset, int origin ) {
	uiImports->FS_Seek( f, offset, origin );
}
//----(SA)	end

void trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	uiImports->FS_Write( buffer, len, f );
}

void trap_FS_FCloseFile( fileHandle_t f ) {
	uiImports->FS_FCloseFile( f );
}

int trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize ) {
	return uiImports->FS_GetFileList( path, extension, listbuf, bufsize );
}

int trap_FS_Delete( const char *filename ) {
	return uiImports->FS_Delete( filename );
}

qhandle_t trap_R_RegisterModel( const char *name ) {
	return uiImports->RE_RegisterModel( name );
}

qhandle_t trap_R_RegisterSkin( const char *name ) {
	return uiImports->RE_RegisterSkin( name );
}

void trap_R_RegisterFont( const char *fontName, int pointSize, fontInfo_t *font ) {
	uiImports->RE_RegisterFont( fontName, pointSize, font );
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) {
	return uiImports->RE_RegisterShaderNoMip( name );
}

void trap_R_ClearScene( void ) {
	uiImports->RE_ClearScene(  );
}

void trap_R_AddRefEntityToScene( const refEntity_t *re ) {
	uiImports->RE_AddRefEntityToScene( re );
}

void trap_R_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts ) {
	uiImports->RE_AddPolyToScene( hShader, numVerts, verts );
}

void trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b, int overdraw ) {
	uiImports->RE_AddLightToScene( org, intensity, r, g, b, overdraw );
}

void trap_R_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, int flags ) {
	uiImports->RE_AddCoronaToScene( org, r, g, b, scale, id, flags );
}

void trap_R_RenderScene( const refdef_t *fd ) {
	uiImports->RE_RenderScene( fd );
}

void trap_R_SetColor( const float *rgba ) {
	uiImports->RE_SetColor( rgba );
}

void trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	uiImports->RE_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, hShader );
}

void trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	uiImports->RE_ModelBounds( model, mins, maxs );
}

void trap_UpdateScreen( void ) {
	uiImports->SCR_UpdateScreen(  );
}

int trap_CM_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex ) {
	return uiImports->RE_LerpTag( tag, refent, tagName, startIndex );
}

void trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	uiImports->S_StartLocalSound( sfx, channelNum );
}

sfxHandle_t trap_S_RegisterSound( const char *sample ) {
	return uiImports->S_RegisterSound( sample );
}

//----(SA)	added (already in cg)
void trap_S_FadeBackgroundTrack( float targetvol, int time, int num ) {
	uiImports->S_FadeStreamingSound( targetvol, time, num );
}

void trap_S_FadeAllSound( float targetvol, int time ) {
	uiImports->S_FadeAllSounds( targetvol, time );
}
//----(SA)	end


void trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	uiImports->Key_KeynumToStringBuf( keynum, buf, buflen );
}

void trap_Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	uiImports->Key_GetBindingBuf( keynum, buf, buflen );
}

void trap_Key_SetBinding( int keynum, const char *binding ) {
	uiImports->Key_SetBinding( keynum, binding );
}

qboolean trap_Key_IsDown( int keynum ) {
	return uiImports->Key_IsDown( keynum );
}

qboolean trap_Key_GetOverstrikeMode( void ) {
	return uiImports->Key_GetOverstrikeMode(  );
}

void trap_Key_SetOverstrikeMode( qboolean state ) {
	uiImports->Key_SetOverstrikeMode( state );
}

void trap_Key_ClearStates( void ) {
	uiImports->Key_ClearStates(  );
}

int trap_Key_GetCatcher( void ) {
	return uiImports->Key_GetCatcher(  );
}

void trap_Key_SetCatcher( int catcher ) {
	uiImports->Key_SetCatcher( catcher );
}

void trap_GetClipboardData( char *buf, int bufsize ) {
	uiImports->GetClipboardData( buf, bufsize );
}

void trap_GetClientState( uiClientState_t *state ) {
	uiImports->GetClientState( state );
}

void trap_GetGlconfig( glconfig_t *glconfig ) {
	uiImports->CL_GetGlconfig( glconfig );
}

int trap_GetConfigString( int index, char* buff, int buffsize ) {
	return uiImports->GetConfigString( index, buff, buffsize );
}

int trap_LAN_GetLocalServerCount( void ) {
	return uiImports->LAN_GetLocalServerCount(  );
}

void trap_LAN_GetLocalServerAddressString( int n, char *buf, int buflen ) {
	uiImports->LAN_GetLocalServerAddressString( n, buf, buflen );
}

int trap_LAN_GetGlobalServerCount( void ) {
	return uiImports->LAN_GetGlobalServerCount(  );
}

void trap_LAN_GetGlobalServerAddressString( int n, char *buf, int buflen ) {
	uiImports->LAN_GetGlobalServerAddressString( n, buf, buflen );
}

int trap_LAN_GetPingQueueCount( void ) {
	return uiImports->LAN_GetPingQueueCount(  );
}

void trap_LAN_ClearPing( int n ) {
	uiImports->LAN_ClearPing( n );
}

void trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	uiImports->LAN_GetPing( n, buf, buflen, pingtime );
}

void trap_LAN_GetPingInfo( int n, char *buf, int buflen ) {
	uiImports->LAN_GetPingInfo( n, buf, buflen );
}

// NERVE - SMF
qboolean trap_LAN_UpdateVisiblePings( int source ) {
	return uiImports->LAN_UpdateVisiblePings( source );
}

int trap_LAN_GetServerCount( int source ) {
	return uiImports->LAN_GetServerCount( source );
}

int trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	return uiImports->LAN_CompareServers( source, sortKey, sortDir, s1, s2 );
}

void trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	uiImports->LAN_GetServerAddressString( source, n, buf, buflen );
}

void trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	uiImports->LAN_GetServerInfo( source, n, buf, buflen );
}

int trap_LAN_AddServer( int source, const char *name, const char *addr ) {
	return uiImports->LAN_AddServer( source, name, addr );
}

void trap_LAN_RemoveServer( int source, const char *addr ) {
	uiImports->LAN_RemoveServer( source, addr );
}

int trap_LAN_GetServerPing( int source, int n ) {
	return uiImports->LAN_GetServerPing( source, n );
}

int trap_LAN_ServerIsVisible( int source, int n ) {
	return uiImports->LAN_ServerIsVisible( source, n );
}

int trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen ) {
	return uiImports->LAN_GetServerStatus( serverAddress, serverStatus, maxLen );
}

void trap_LAN_SaveCachedServers( void ) {
	uiImports->LAN_SaveServersToCache();
}

void trap_LAN_LoadCachedServers( void ) {
	uiImports->LAN_LoadCachedServers();
}

void trap_LAN_MarkServerVisible( int source, int n, qboolean visible ) {
	uiImports->LAN_MarkServerVisible( source, n, visible );
}

void trap_LAN_ResetPings( int n ) {
	uiImports->LAN_ResetPings( n );
}
// -NERVE - SMF

int trap_MemoryRemaining( void ) {
	return uiImports->Hunk_MemoryRemaining(  );
}


void trap_GetCDKey( char *buf, int buflen ) {
	uiImports->CLUI_GetCDKey( buf, buflen );
}

void trap_SetCDKey( char *buf ) {
	uiImports->CLUI_SetCDKey( buf );
}

int trap_PC_AddGlobalDefine( char *define ) {
	return uiImports->PC_AddGlobalDefine( define );
}

int trap_PC_LoadSource( const char *filename ) {
	return uiImports->PC_LoadSourceHandle( filename );
}

int trap_PC_FreeSource( int handle ) {
	return uiImports->PC_FreeSourceHandle( handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return uiImports->PC_ReadTokenHandle( handle, pc_token );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return uiImports->PC_SourceFileAndLine( handle, filename, line );
}

void trap_S_StopBackgroundTrack( void ) {
	uiImports->S_StopBackgroundTrack(  );
}

void trap_S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime ) {
	uiImports->S_StartBackgroundTrack( intro, loop, fadeupTime );
}

int trap_RealTime( qtime_t *qtime ) {
	return uiImports->Com_RealTime( qtime );
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits ) {
	return uiImports->CIN_PlayCinematic( arg0, xpos, ypos, width, height, bits );
}

// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status trap_CIN_StopCinematic( int handle ) {
	return uiImports->CIN_StopCinematic( handle );
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status trap_CIN_RunCinematic( int handle ) {
	return uiImports->CIN_RunCinematic( handle );
}


// draws the current frame
void trap_CIN_DrawCinematic( int handle ) {
	uiImports->CIN_DrawCinematic( handle );
}


// allows you to resize the animation dynamically
void trap_CIN_SetExtents( int handle, int x, int y, int w, int h ) {
	uiImports->CIN_SetExtents( handle, x, y, w, h );
}


void trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) {
	uiImports->RE_RemapShader( oldShader, newShader, timeOffset );
}

qboolean trap_VerifyCDKey( const char *key, const char *chksum ) {
	return uiImports->CL_CDKeyValidate( key, chksum );
}

// NERVE - SMF
qboolean trap_GetLimboString( int index, char *buf ) {
	return uiImports->CL_GetLimboString( index, buf );
}
// -NERVE - SMF
