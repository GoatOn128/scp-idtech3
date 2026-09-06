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

// Native module bindings supplied through the typed cgame import table.
#include "cg_local.h"

static const cgameImport_t *cgImports;

void CG_SetImports( const cgameImport_t *imports ) {
	cgImports = imports;
}

void trap_Print( const char *fmt ) {
	cgImports->Com_Printf( fmt );
}

void trap_Error( const char *fmt ) {
	cgImports->Com_Error( fmt );
}

int trap_Milliseconds( void ) {
	return cgImports->Sys_Milliseconds(  );
}

void trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags ) {
	cgImports->Cvar_Register( vmCvar, varName, defaultValue, flags );
}

void trap_Cvar_Update( vmCvar_t *vmCvar ) {
	cgImports->Cvar_Update( vmCvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	cgImports->Cvar_Set( var_name, value );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	cgImports->Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}

int trap_Argc( void ) {
	return cgImports->Cmd_Argc(  );
}

void trap_Argv( int n, char *buffer, int bufferLength ) {
	cgImports->Cmd_ArgvBuffer( n, buffer, bufferLength );
}

void trap_Args( char *buffer, int bufferLength ) {
	cgImports->Cmd_ArgsBuffer( buffer, bufferLength );
}

int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return cgImports->FS_FOpenFileByMode( qpath, f, mode );
}

void trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	cgImports->FS_Read( buffer, len, f );
}

void trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	cgImports->FS_Write( buffer, len, f );
}

void trap_FS_FCloseFile( fileHandle_t f ) {
	cgImports->FS_FCloseFile( f );
}

void trap_SendConsoleCommand( const char *text ) {
	cgImports->Cbuf_AddText( text );
}

void trap_AddCommand( const char *cmdName ) {
	cgImports->CL_AddCgameCommand( cmdName );
}

void trap_SendClientCommand( const char *s ) {
	cgImports->CL_AddReliableCommand( s );
}

void trap_UpdateScreen( void ) {
	cgImports->SCR_UpdateScreen(  );
}

void trap_CM_LoadMap( const char *mapname ) {
	cgImports->CL_CM_LoadMap( mapname );
}

int trap_CM_NumInlineModels( void ) {
	return cgImports->CM_NumInlineModels(  );
}

clipHandle_t trap_CM_InlineModel( int index ) {
	return cgImports->CM_InlineModel( index );
}

clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs ) {
	return cgImports->CM_TempBoxModel( mins, maxs );
}

clipHandle_t trap_CM_TempCapsuleModel( const vec3_t mins, const vec3_t maxs ) {
	return cgImports->CM_TempCapsuleModel( mins, maxs );
}

int trap_CM_PointContents( const vec3_t p, clipHandle_t model ) {
	return cgImports->CM_PointContents( p, model );
}

int trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles ) {
	return cgImports->CM_TransformedPointContents( p, model, origin, angles );
}

void    trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask ) {
	cgImports->CM_BoxTrace( results, start, end, mins, maxs, model, brushmask );
}

void    trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
									 const vec3_t mins, const vec3_t maxs,
									 clipHandle_t model, int brushmask,
									 const vec3_t origin, const vec3_t angles ) {
	cgImports->CM_TransformedBoxTrace( results, start, end, mins, maxs, model, brushmask, origin, angles );
}

void    trap_CM_CapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
							  const vec3_t mins, const vec3_t maxs,
							  clipHandle_t model, int brushmask ) {
	cgImports->CM_CapsuleTrace( results, start, end, mins, maxs, model, brushmask );
}

void    trap_CM_TransformedCapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end,
										 const vec3_t mins, const vec3_t maxs,
										 clipHandle_t model, int brushmask,
										 const vec3_t origin, const vec3_t angles ) {
	cgImports->CM_TransformedCapsuleTrace( results, start, end, mins, maxs, model, brushmask, origin, angles );
}

int     trap_CM_MarkFragments( int numPoints, const vec3_t *points,
							   const vec3_t projection,
							   int maxPoints, vec3_t pointBuffer,
							   int maxFragments, markFragment_t *fragmentBuffer ) {
	return cgImports->RE_MarkFragments( numPoints, points, projection, maxPoints, pointBuffer, maxFragments, fragmentBuffer );
}

void trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx ) {
	cgImports->S_StartSound( origin, entityNum, entchannel, sfx );
}

//----(SA)	added
void trap_S_StartSoundEx( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx, int flags ) {
	cgImports->S_StartSoundEx( origin, entityNum, entchannel, sfx, flags );
}
//----(SA)	end

void trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) {
	cgImports->S_StartLocalSound( sfx, channelNum );
}

void trap_S_ClearLoopingSounds( qboolean killall ) {
	cgImports->S_ClearLoopingSounds( killall );
}

void trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int volume ) {
	cgImports->S_AddLoopingSound( entityNum, origin, velocity, sfx, volume );
}

//----(SA)	added
void trap_S_AddRangedLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int range ) {
	cgImports->S_AddRangedLoopingSound( entityNum, origin, velocity, sfx, range );
}
//----(SA)	end

void trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx ) {
	cgImports->S_AddRealLoopingSound( entityNum, origin, velocity, sfx );
}

void trap_S_StopLoopingSound( int entityNum ) {
	cgImports->S_StopLoopingSound( entityNum );
}

//----(SA)	added
void trap_S_StopStreamingSound( int entityNum ) {
	cgImports->S_StopEntStreamingSound( entityNum );
}
//----(SA)	end

void trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin ) {
	cgImports->S_UpdateEntityPosition( entityNum, origin );
}

// Ridah, talking animations
int trap_S_GetVoiceAmplitude( int entityNum ) {
	return cgImports->S_GetVoiceAmplitude( entityNum );
}
// done.

void trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater ) {
	cgImports->S_Respatialize( entityNum, origin, axis, inwater );
}

sfxHandle_t trap_S_RegisterSound( const char *sample ) {
	return cgImports->S_RegisterSound( sample );
}

void trap_S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime ) {
	cgImports->S_StartBackgroundTrack( intro, loop, fadeupTime );
}

void trap_S_FadeBackgroundTrack( float targetvol, int time, int num ) {
	cgImports->S_FadeStreamingSound( targetvol, time, num );
}

void trap_S_FadeAllSound( float targetvol, int time ) {
	cgImports->S_FadeAllSounds( targetvol, time );
}

//----(SA)	end

void trap_S_StartStreamingSound( const char *intro, const char *loop, int entnum, int channel, int attenuation ) {
	cgImports->S_StartStreamingSound( intro, loop, entnum, channel, attenuation );
}

void trap_R_LoadWorldMap( const char *mapname ) {
	cgImports->RE_LoadWorld( mapname );
}

qhandle_t trap_R_RegisterModel( const char *name ) {
	return cgImports->RE_RegisterModel( name );
}

//----(SA)	added
qboolean trap_R_GetSkinModel( qhandle_t skinid, const char *type, char *name ) {
	return cgImports->RE_GetSkinModel( skinid, type, name );
}

qhandle_t trap_R_GetShaderFromModel( qhandle_t modelid, int surfnum, int withlightmap ) {
	return cgImports->RE_GetShaderFromModel( modelid, surfnum, withlightmap );
}
//----(SA)	end

qhandle_t trap_R_RegisterSkin( const char *name ) {
	return cgImports->RE_RegisterSkin( name );
}

qhandle_t trap_R_RegisterShader( const char *name ) {
	return cgImports->RE_RegisterShader( name );
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) {
	return cgImports->RE_RegisterShaderNoMip( name );
}

void trap_R_RegisterFont( const char *fontName, int pointSize, fontInfo_t *font ) {
	cgImports->RE_RegisterFont( fontName, pointSize, font );
}

void trap_R_ClearScene( void ) {
	cgImports->RE_ClearScene(  );
}

void trap_R_AddRefEntityToScene( const refEntity_t *re ) {
	cgImports->RE_AddRefEntityToScene( re );
}

void trap_R_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts ) {
	cgImports->RE_AddPolyToScene( hShader, numVerts, verts );
}

// Ridah
void trap_R_AddPolysToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys ) {
	cgImports->RE_AddPolysToScene( hShader, numVerts, verts, numPolys );
}

void trap_RB_ZombieFXAddNewHit( int entityNum, const vec3_t hitPos, const vec3_t hitDir ) {
	cgImports->RE_ZombieFXAddNewHit( entityNum, hitPos, hitDir );
}
// done.

void trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b, int overdraw ) {
	cgImports->RE_AddLightToScene( org, intensity, r, g, b, overdraw );
}

//----(SA)
void trap_R_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, int flags ) {
	cgImports->RE_AddCoronaToScene( org, r, g, b, scale, id, flags );
}
//----(SA)

//----(SA)
void trap_R_SetFog( int fogvar, int var1, int var2, float r, float g, float b, float density ) {
	cgImports->RE_SetFog( fogvar, var1, var2, r, g, b, density );
}
//----(SA)
void trap_R_RenderScene( const refdef_t *fd ) {
	cgImports->RE_RenderScene( fd );
}

void trap_R_SetColor( const float *rgba ) {
	cgImports->RE_SetColor( rgba );
}

void    trap_R_DrawStretchPic( float x, float y, float w, float h,
							   float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	cgImports->RE_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, hShader );
}

void    trap_R_DrawStretchPicGradient(  float x, float y, float w, float h,
										float s1, float t1, float s2, float t2, qhandle_t hShader,
										const float *gradientColor, int gradientType ) {
	cgImports->RE_DrawStretchPicGradient( x, y, w, h, s1, t1, s2, t2, hShader, gradientColor, gradientType );
}

void trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	cgImports->RE_ModelBounds( model, mins, maxs );
}

int trap_R_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex ) {
	return cgImports->RE_LerpTag( tag, refent, tagName, startIndex );
}

void trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) {
	cgImports->RE_RemapShader( oldShader, newShader, timeOffset );
}

void trap_GetGlconfig( glconfig_t *glconfig ) {
	cgImports->CL_GetGlconfig( glconfig );
}

void trap_GetGameState( gameState_t *gamestate ) {
	cgImports->CL_GetGameState( gamestate );
}

void trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) {
	cgImports->CL_GetCurrentSnapshotNumber( snapshotNumber, serverTime );
}

qboolean trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot ) {
	return cgImports->CL_GetSnapshot( snapshotNumber, snapshot );
}

qboolean trap_GetServerCommand( int serverCommandNumber ) {
	return cgImports->CL_GetServerCommand( serverCommandNumber );
}

int trap_GetCurrentCmdNumber( void ) {
	return cgImports->CL_GetCurrentCmdNumber(  );
}

qboolean trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd ) {
	return cgImports->CL_GetUserCmd( cmdNumber, ucmd );
}

void trap_SetUserCmdValue( int stateValue, int holdableValue, float sensitivityScale, int cld ) {
	cgImports->CL_SetUserCmdValue( stateValue, holdableValue, sensitivityScale, cld );
}

void        testPrintInt( char *string, int i ) {
	cgImports->Com_PrintfInt( string, i );
}

void        testPrintFloat( char *string, float f ) {
	cgImports->Com_PrintfFloat( string, f );
}

int trap_MemoryRemaining( void ) {
	return cgImports->Hunk_MemoryRemaining(  );
}

qboolean trap_loadCamera( int camNum, const char *name ) {
	return cgImports->loadCamera( camNum, name );
}

void trap_startCamera( int camNum, int time ) {
	cgImports->startCamera( camNum, time );
}

//----(SA)	added
void trap_stopCamera( int camNum ) {
	cgImports->CL_StopCamera( camNum );
}
//----(SA)	end

qboolean trap_getCameraInfo( int camNum, int time, vec3_t *origin, vec3_t *angles, float *fov ) {
	return cgImports->getCameraInfo( camNum, time, origin, angles, fov );
}


qboolean trap_Key_IsDown( int keynum ) {
	return cgImports->Key_IsDown( keynum );
}

int trap_Key_GetCatcher( void ) {
	return cgImports->Key_GetCatcher(  );
}

void trap_Key_SetCatcher( int catcher ) {
	cgImports->Key_SetCatcher( catcher );
}

int trap_Key_GetKey( const char *binding ) {
	return cgImports->Key_GetKey( binding );
}


int trap_PC_AddGlobalDefine( char *define ) {
	return cgImports->PC_AddGlobalDefine( define );
}

int trap_PC_LoadSource( const char *filename ) {
	return cgImports->PC_LoadSourceHandle( filename );
}

int trap_PC_FreeSource( int handle ) {
	return cgImports->PC_FreeSourceHandle( handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return cgImports->PC_ReadTokenHandle( handle, pc_token );
}


int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return cgImports->PC_SourceFileAndLine( handle, filename, line );
}

void trap_S_StopBackgroundTrack( void ) {
	cgImports->S_StopBackgroundTrack(  );
}

int trap_RealTime( qtime_t *qtime ) {
	return cgImports->Com_RealTime( qtime );
}

void trap_SnapVector( float *v ) {
	cgImports->Sys_SnapVector( v );
}

void trap_SendMoveSpeedsToGame( int entnum, char *movespeeds ) {
	cgImports->SV_SendMoveSpeedsToGame( entnum, movespeeds );
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits ) {
	return cgImports->CIN_PlayCinematic( arg0, xpos, ypos, width, height, bits );
}

// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status trap_CIN_StopCinematic( int handle ) {
	return cgImports->CIN_StopCinematic( handle );
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status trap_CIN_RunCinematic( int handle ) {
	return cgImports->CIN_RunCinematic( handle );
}


// draws the current frame
void trap_CIN_DrawCinematic( int handle ) {
	cgImports->CIN_DrawCinematic( handle );
}


// allows you to resize the animation dynamically
void trap_CIN_SetExtents( int handle, int x, int y, int w, int h ) {
	cgImports->CIN_SetExtents( handle, x, y, w, h );
}

qboolean trap_GetEntityToken( char *buffer, int bufferSize ) {
	return cgImports->RE_GetEntityToken( buffer, bufferSize );
}

//----(SA)	added
// bring up a popup menu
extern void Menus_OpenByName( const char *p );

void trap_UI_Popup( const char *arg0 ) {
	cgImports->CL_InGamePopup( arg0 );
}

// NERVE - SMF
void trap_UI_LimboChat( const char *arg0 ) {
	cgImports->CL_AddToLimboChat( arg0 );
}

void trap_UI_ClosePopup( const char *arg0 ) {
	cgImports->CL_ClosePopup( arg0 );
}
// -NERVE - SMF

qboolean trap_GetModelInfo( int clientNum, char *modelName, animModelInfo_t **modelInfo ) {
	return cgImports->SV_GetModelInfo( clientNum, modelName, (void **)modelInfo );
}
