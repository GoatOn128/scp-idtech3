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


#include "client.h"

#include "../game/botlib.h"

extern botlib_export_t *botlib_export;

const uiExport_t *uivm;
void             *uivmHandle;

extern char cl_cdkey[34];


/*
====================
GetClientState
====================
*/
static void GetClientState( uiClientState_t *state ) {
	state->connectPacketCount = clc.connectPacketCount;
	state->connState = cls.state;
	Q_strncpyz( state->servername, cls.servername, sizeof( state->servername ) );
	Q_strncpyz( state->updateInfoString, cls.updateInfoString, sizeof( state->updateInfoString ) );
	Q_strncpyz( state->messageString, clc.serverMessage, sizeof( state->messageString ) );
	state->clientNum = cl.snap.ps.clientNum;
}

/*
====================
LAN_LoadCachedServers
====================
*/
void LAN_LoadCachedServers() {
	// TTimo: stub, this is only relevant to MP, SP kills the servercache.dat (and favorites)
	// show_bug.cgi?id=445
	/*
	  int size;
	  fileHandle_t fileIn;
	  cls.numglobalservers = cls.nummplayerservers = cls.numfavoriteservers = 0;
	  cls.numGlobalServerAddresses = 0;
	  if (FS_SV_FOpenFileRead("servercache.dat", &fileIn)) {
		  FS_Read(&cls.numglobalservers, sizeof(int), fileIn);
		  FS_Read(&cls.nummplayerservers, sizeof(int), fileIn);
		  FS_Read(&cls.numfavoriteservers, sizeof(int), fileIn);
		  FS_Read(&size, sizeof(int), fileIn);
		  if (size == sizeof(cls.globalServers) + sizeof(cls.favoriteServers) + sizeof(cls.mplayerServers)) {
			  FS_Read(&cls.globalServers, sizeof(cls.globalServers), fileIn);
			  FS_Read(&cls.mplayerServers, sizeof(cls.mplayerServers), fileIn);
			  FS_Read(&cls.favoriteServers, sizeof(cls.favoriteServers), fileIn);
		  } else {
			  cls.numglobalservers = cls.nummplayerservers = cls.numfavoriteservers = 0;
			  cls.numGlobalServerAddresses = 0;
		  }
		  FS_FCloseFile(fileIn);
	  }
	*/
}

/*
====================
LAN_SaveServersToCache
====================
*/
void LAN_SaveServersToCache() {
	// TTimo: stub, this is only relevant to MP, SP kills the servercache.dat (and favorites)
	// show_bug.cgi?id=445
	/*
	  int size;
	  fileHandle_t fileOut;
  #ifdef __MACOS__	//DAJ MacOS file typing
	  {
		  extern _MSL_IMP_EXP_C long _fcreator, _ftype;
		  _ftype = 'WlfB';
		  _fcreator = 'WlfS';
	  }
  #endif
	  fileOut = FS_SV_FOpenFileWrite("servercache.dat");
	  FS_Write(&cls.numglobalservers, sizeof(int), fileOut);
	  FS_Write(&cls.nummplayerservers, sizeof(int), fileOut);
	  FS_Write(&cls.numfavoriteservers, sizeof(int), fileOut);
	  size = sizeof(cls.globalServers) + sizeof(cls.favoriteServers) + sizeof(cls.mplayerServers);
	  FS_Write(&size, sizeof(int), fileOut);
	  FS_Write(&cls.globalServers, sizeof(cls.globalServers), fileOut);
	  FS_Write(&cls.mplayerServers, sizeof(cls.mplayerServers), fileOut);
	  FS_Write(&cls.favoriteServers, sizeof(cls.favoriteServers), fileOut);
	  FS_FCloseFile(fileOut);
	*/
}


/*
====================
LAN_ResetPings
====================
*/
static void LAN_ResetPings( int source ) {
	int count,i;
	serverInfo_t *servers = NULL;
	count = 0;

	switch ( source ) {
	case AS_LOCAL:
		servers = &cls.localServers[0];
		count = MAX_OTHER_SERVERS;
		break;
	case AS_MPLAYER:
		servers = &cls.mplayerServers[0];
		count = MAX_OTHER_SERVERS;
		break;
	case AS_GLOBAL:
		servers = &cls.globalServers[0];
		count = MAX_GLOBAL_SERVERS;
		break;
	case AS_FAVORITES:
		servers = &cls.favoriteServers[0];
		count = MAX_OTHER_SERVERS;
		break;
	}
	if ( servers ) {
		for ( i = 0; i < count; i++ ) {
			servers[i].ping = -1;
		}
	}
}

/*
====================
LAN_AddServer
====================
*/
static int LAN_AddServer( int source, const char *name, const char *address ) {
	int max, *count, i;
	netadr_t adr;
	serverInfo_t *servers = NULL;
	max = MAX_OTHER_SERVERS;
	count = 0;

	switch ( source ) {
	case AS_LOCAL:
		count = &cls.numlocalservers;
		servers = &cls.localServers[0];
		break;
	case AS_MPLAYER:
		count = &cls.nummplayerservers;
		servers = &cls.mplayerServers[0];
		break;
	case AS_GLOBAL:
		max = MAX_GLOBAL_SERVERS;
		count = &cls.numglobalservers;
		servers = &cls.globalServers[0];
		break;
	case AS_FAVORITES:
		count = &cls.numfavoriteservers;
		servers = &cls.favoriteServers[0];
		break;
	}
	if ( servers && *count < max ) {
		NET_StringToAdr( address, &adr );
		for ( i = 0; i < *count; i++ ) {
			if ( NET_CompareAdr( servers[i].adr, adr ) ) {
				break;
			}
		}
		if ( i >= *count ) {
			servers[*count].adr = adr;
			Q_strncpyz( servers[*count].hostName, name, sizeof( servers[*count].hostName ) );
			servers[*count].visible = qtrue;
			( *count )++;
			return 1;
		}
		return 0;
	}
	return -1;
}

/*
====================
LAN_RemoveServer
====================
*/
static void LAN_RemoveServer( int source, const char *addr ) {
	int *count, i;
	serverInfo_t *servers = NULL;
	count = 0;
	switch ( source ) {
	case AS_LOCAL:
		count = &cls.numlocalservers;
		servers = &cls.localServers[0];
		break;
	case AS_MPLAYER:
		count = &cls.nummplayerservers;
		servers = &cls.mplayerServers[0];
		break;
	case AS_GLOBAL:
		count = &cls.numglobalservers;
		servers = &cls.globalServers[0];
		break;
	case AS_FAVORITES:
		count = &cls.numfavoriteservers;
		servers = &cls.favoriteServers[0];
		break;
	}
	if ( servers ) {
		netadr_t comp;
		NET_StringToAdr( addr, &comp );
		for ( i = 0; i < *count; i++ ) {
			if ( NET_CompareAdr( comp, servers[i].adr ) ) {
				int j = i;
				while ( j < *count - 1 ) {
					Com_Memcpy( &servers[j], &servers[j + 1], sizeof( servers[j] ) );
					j++;
				}
				( *count )--;
				break;
			}
		}
	}
}


/*
====================
LAN_GetServerCount
====================
*/
static int LAN_GetServerCount( int source ) {
	switch ( source ) {
	case AS_LOCAL:
		return cls.numlocalservers;
		break;
	case AS_MPLAYER:
		return cls.nummplayerservers;
		break;
	case AS_GLOBAL:
		return cls.numglobalservers;
		break;
	case AS_FAVORITES:
		return cls.numfavoriteservers;
		break;
	}
	return 0;
}

/*
====================
LAN_GetLocalServerAddressString
====================
*/
static void LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	switch ( source ) {
	case AS_LOCAL:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			Q_strncpyz( buf, NET_AdrToString( cls.localServers[n].adr ), buflen );
			return;
		}
		break;
	case AS_MPLAYER:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			Q_strncpyz( buf, NET_AdrToString( cls.mplayerServers[n].adr ), buflen );
			return;
		}
		break;
	case AS_GLOBAL:
		if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
			Q_strncpyz( buf, NET_AdrToString( cls.globalServers[n].adr ), buflen );
			return;
		}
		break;
	case AS_FAVORITES:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			Q_strncpyz( buf, NET_AdrToString( cls.favoriteServers[n].adr ), buflen );
			return;
		}
		break;
	}
	buf[0] = '\0';
}

/*
====================
LAN_GetServerInfo
====================
*/
static void LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	char info[MAX_STRING_CHARS];
	serverInfo_t *server = NULL;
	info[0] = '\0';
	switch ( source ) {
	case AS_LOCAL:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.localServers[n];
		}
		break;
	case AS_MPLAYER:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.mplayerServers[n];
		}
		break;
	case AS_GLOBAL:
		if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
			server = &cls.globalServers[n];
		}
		break;
	case AS_FAVORITES:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.favoriteServers[n];
		}
		break;
	}
	if ( server && buf ) {
		buf[0] = '\0';
		Info_SetValueForKey( info, "hostname", server->hostName );
		Info_SetValueForKey( info, "mapname", server->mapName );
		Info_SetValueForKey( info, "clients", va( "%i",server->clients ) );
		Info_SetValueForKey( info, "sv_maxclients", va( "%i",server->maxClients ) );
		Info_SetValueForKey( info, "ping", va( "%i",server->ping ) );
		Info_SetValueForKey( info, "minping", va( "%i",server->minPing ) );
		Info_SetValueForKey( info, "maxping", va( "%i",server->maxPing ) );
		Info_SetValueForKey( info, "game", server->game );
		Info_SetValueForKey( info, "gametype", va( "%i",server->gameType ) );
		Info_SetValueForKey( info, "nettype", va( "%i",server->netType ) );
		Info_SetValueForKey( info, "addr", NET_AdrToString( server->adr ) );
		Info_SetValueForKey( info, "sv_allowAnonymous", va( "%i", server->allowAnonymous ) );
		Q_strncpyz( buf, info, buflen );
	} else {
		if ( buf ) {
			buf[0] = '\0';
		}
	}
}

/*
====================
LAN_GetServerPing
====================
*/
static int LAN_GetServerPing( int source, int n ) {
	serverInfo_t *server = NULL;
	switch ( source ) {
	case AS_LOCAL:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.localServers[n];
		}
		break;
	case AS_MPLAYER:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.mplayerServers[n];
		}
		break;
	case AS_GLOBAL:
		if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
			server = &cls.globalServers[n];
		}
		break;
	case AS_FAVORITES:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			server = &cls.favoriteServers[n];
		}
		break;
	}
	if ( server ) {
		return server->ping;
	}
	return -1;
}

/*
====================
LAN_GetServerPtr
====================
*/
static serverInfo_t *LAN_GetServerPtr( int source, int n ) {
	switch ( source ) {
	case AS_LOCAL:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return &cls.localServers[n];
		}
		break;
	case AS_MPLAYER:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return &cls.mplayerServers[n];
		}
		break;
	case AS_GLOBAL:
		if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
			return &cls.globalServers[n];
		}
		break;
	case AS_FAVORITES:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return &cls.favoriteServers[n];
		}
		break;
	}
	return NULL;
}

/*
====================
LAN_CompareServers
====================
*/
static int LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	int res;
	serverInfo_t *server1, *server2;

	server1 = LAN_GetServerPtr( source, s1 );
	server2 = LAN_GetServerPtr( source, s2 );
	if ( !server1 || !server2 ) {
		return 0;
	}

	res = 0;
	switch ( sortKey ) {
	case SORT_HOST:
		res = Q_stricmp( server1->hostName, server2->hostName );
		break;

	case SORT_MAP:
		res = Q_stricmp( server1->mapName, server2->mapName );
		break;
	case SORT_CLIENTS:
		if ( server1->clients < server2->clients ) {
			res = -1;
		} else if ( server1->clients > server2->clients )     {
			res = 1;
		} else {
			res = 0;
		}
		break;
	case SORT_GAME:
		if ( server1->gameType < server2->gameType ) {
			res = -1;
		} else if ( server1->gameType > server2->gameType )     {
			res = 1;
		} else {
			res = 0;
		}
		break;
	case SORT_PING:
		if ( server1->ping < server2->ping ) {
			res = -1;
		} else if ( server1->ping > server2->ping )     {
			res = 1;
		} else {
			res = 0;
		}
		break;
	}

	if ( sortDir ) {
		if ( res < 0 ) {
			return 1;
		}
		if ( res > 0 ) {
			return -1;
		}
		return 0;
	}
	return res;
}

/*
====================
LAN_GetPingQueueCount
====================
*/
static int LAN_GetPingQueueCount( void ) {
	return ( CL_GetPingQueueCount() );
}

/*
====================
LAN_ClearPing
====================
*/
static void LAN_ClearPing( int n ) {
	CL_ClearPing( n );
}

/*
====================
LAN_GetPing
====================
*/
static void LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	CL_GetPing( n, buf, buflen, pingtime );
}

/*
====================
LAN_GetPingInfo
====================
*/
static void LAN_GetPingInfo( int n, char *buf, int buflen ) {
	CL_GetPingInfo( n, buf, buflen );
}

/*
====================
LAN_MarkServerVisible
====================
*/
static void LAN_MarkServerVisible( int source, int n, qboolean visible ) {
	if ( n == -1 ) {
		int count = MAX_OTHER_SERVERS;
		serverInfo_t *server = NULL;
		switch ( source ) {
		case AS_LOCAL:
			server = &cls.localServers[0];
			break;
		case AS_MPLAYER:
			server = &cls.mplayerServers[0];
			break;
		case AS_GLOBAL:
			server = &cls.globalServers[0];
			count = MAX_GLOBAL_SERVERS;
			break;
		case AS_FAVORITES:
			server = &cls.favoriteServers[0];
			break;
		}
		if ( server ) {
			for ( n = 0; n < count; n++ ) {
				server[n].visible = visible;
			}
		}

	} else {
		switch ( source ) {
		case AS_LOCAL:
			if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
				cls.localServers[n].visible = visible;
			}
			break;
		case AS_MPLAYER:
			if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
				cls.mplayerServers[n].visible = visible;
			}
			break;
		case AS_GLOBAL:
			if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
				cls.globalServers[n].visible = visible;
			}
			break;
		case AS_FAVORITES:
			if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
				cls.favoriteServers[n].visible = visible;
			}
			break;
		}
	}
}


/*
=======================
LAN_ServerIsVisible
=======================
*/
static int LAN_ServerIsVisible( int source, int n ) {
	switch ( source ) {
	case AS_LOCAL:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return cls.localServers[n].visible;
		}
		break;
	case AS_MPLAYER:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return cls.mplayerServers[n].visible;
		}
		break;
	case AS_GLOBAL:
		if ( n >= 0 && n < MAX_GLOBAL_SERVERS ) {
			return cls.globalServers[n].visible;
		}
		break;
	case AS_FAVORITES:
		if ( n >= 0 && n < MAX_OTHER_SERVERS ) {
			return cls.favoriteServers[n].visible;
		}
		break;
	}
	return qfalse;
}

/*
=======================
LAN_UpdateVisiblePings
=======================
*/
qboolean LAN_UpdateVisiblePings( int source ) {
	return CL_UpdateVisiblePings_f( source );
}

/*
====================
LAN_GetServerStatus
====================
*/
int LAN_GetServerStatus( char *serverAddress, char *serverStatus, int maxLen ) {
	return CL_ServerStatus( serverAddress, serverStatus, maxLen );
}

/*
====================
CL_GetGlConfig
====================
*/
static void CL_GetGlconfig( glconfig_t *config ) {
	*config = cls.glconfig;
}

/*
====================
GetClipboardData
====================
*/
static void GetClipboardData( char *buf, int buflen ) {
	char    *cbd;

	cbd = Sys_GetClipboardData();

	if ( !cbd ) {
		*buf = 0;
		return;
	}

	Q_strncpyz( buf, cbd, buflen );

	Z_Free( cbd );
}

/*
====================
Key_KeynumToStringBuf
====================
*/
static void Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	Q_strncpyz( buf, Key_KeynumToString( keynum, qtrue ), buflen );
}

/*
====================
Key_GetBindingBuf
====================
*/
static void Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	char    *value;

	value = Key_GetBinding( keynum );
	if ( value ) {
		Q_strncpyz( buf, value, buflen );
	} else {
		*buf = 0;
	}
}

/*
====================
Key_GetCatcher
====================
*/
int Key_GetCatcher( void ) {
	return cls.keyCatchers;
}

/*
====================
Ket_SetCatcher
====================
*/
void Key_SetCatcher( int catcher ) {
	cls.keyCatchers = catcher;
}


/*
====================
CLUI_GetCDKey
====================
*/
static void CLUI_GetCDKey( char *buf, int buflen ) {
	cvar_t  *fs;
	fs = Cvar_Get( "fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO );
	if ( UI_usesUniqueCDKey() && fs && fs->string[0] != 0 ) {
		memcpy( buf, &cl_cdkey[16], 16 );
		buf[16] = 0;
	} else {
		memcpy( buf, cl_cdkey, 16 );
		buf[16] = 0;
	}
}


/*
====================
CLUI_SetCDKey
====================
*/
static void CLUI_SetCDKey( char *buf ) {
	cvar_t  *fs;
	fs = Cvar_Get( "fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO );
	if ( UI_usesUniqueCDKey() && fs && fs->string[0] != 0 ) {
		memcpy( &cl_cdkey[16], buf, 16 );
		cl_cdkey[32] = 0;
		// set the flag so the fle will be written at the next opportunity
		cvar_modifiedFlags |= CVAR_ARCHIVE;
	} else {
		memcpy( cl_cdkey, buf, 16 );
		// set the flag so the fle will be written at the next opportunity
		cvar_modifiedFlags |= CVAR_ARCHIVE;
	}
}


/*
====================
GetConfigString
====================
*/
static int GetConfigString( int index, char *buf, int size ) {
	int offset;

	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		return qfalse;
	}

	offset = cl.gameState.stringOffsets[index];
	if ( !offset ) {
		if ( size ) {
			buf[0] = 0;
		}
		return qfalse;
	}

	Q_strncpyz( buf, cl.gameState.stringData + offset, size );

	return qtrue;
}

static uiImport_t CL_CreateUIImports() {
	uiImport_t imports = {};
	imports.Com_Printf = []( const char *string ) -> void {
		Com_Printf( "%s", string );
		return;
	};
	imports.Com_Error = []( const char *string ) -> void {
		Com_Error( ERR_DROP, "%s", string );
		return;
	};
	imports.Sys_Milliseconds = []( void ) -> int {
		return Sys_Milliseconds();
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
		Cvar_Set( var_name, value );
		return;
	};
	imports.Cvar_VariableValue = []( const char *var_name ) -> float {
		return  Cvar_VariableValue( var_name ) ;
	};
	imports.Cvar_VariableStringBuffer = []( const char *var_name, char *buffer, int bufsize ) -> void {
		Cvar_VariableStringBuffer( var_name, buffer, bufsize );
		return;
	};
	imports.Cvar_SetValue = []( const char *var_name, float value ) -> void {
		Cvar_SetValue( var_name, value );
		return;
	};
	imports.Cvar_Reset = []( const char *name ) -> void {
		Cvar_Reset( name );
		return;
	};
	imports.Cvar_Get = []( const char *var_name, const char *var_value, int flags ) -> void {
		Cvar_Get( var_name, var_value, flags );
		return;
	};
	imports.Cvar_InfoStringBuffer = []( int bit, char *buffer, int bufsize ) -> void {
		Cvar_InfoStringBuffer( bit, buffer, bufsize );
		return;
	};
	imports.Cmd_Argc = []( void ) -> int {
		return Cmd_Argc();
	};
	imports.Cmd_ArgvBuffer = []( int n, char *buffer, int bufferLength ) -> void {
		Cmd_ArgvBuffer( n, buffer, bufferLength );
		return;
	};
	imports.Cbuf_ExecuteText = []( int exec_when, const char *text ) -> void {
		Cbuf_ExecuteText( exec_when, text );
		return;
	};
	imports.FS_FOpenFileByMode = []( const char *qpath, fileHandle_t *f, fsMode_t mode ) -> int {
		return FS_FOpenFileByMode( qpath, f, (fsMode_t)mode );
	};
	imports.FS_Read = []( void *buffer, int len, fileHandle_t f ) -> void {
		FS_Read( buffer, len, f );
		return;
		//----(SA)	added
	};
	imports.FS_Seek = []( fileHandle_t f, long offset, int origin ) -> void {
		FS_Seek( f, offset, origin );
		return;
		//----(SA)	end
	};
	imports.FS_Write = []( const void *buffer, int len, fileHandle_t f ) -> void {
		FS_Write( buffer, len, f );
		return;
	};
	imports.FS_FCloseFile = []( fileHandle_t f ) -> void {
		FS_FCloseFile( f );
		return;
	};
	imports.FS_GetFileList = []( const char *path, const char *extension, char *listbuf, int bufsize ) -> int {
		return FS_GetFileList( path, extension, listbuf, bufsize );
	};
	imports.FS_Delete = []( const char *filename ) -> int {
		return FS_Delete( (char *)filename );
	};
	imports.RE_RegisterModel = []( const char *name ) -> qhandle_t {
		return re.RegisterModel( name );
	};
	imports.RE_RegisterSkin = []( const char *name ) -> qhandle_t {
		return re.RegisterSkin( name );
	};
	imports.RE_RegisterFont = []( const char *fontName, int pointSize, fontInfo_t *font ) -> void {
		re.RegisterFont( fontName, pointSize, font );
		return;
	};
	imports.RE_RegisterShaderNoMip = []( const char *name ) -> qhandle_t {
		return re.RegisterShaderNoMip( name );
	};
	imports.RE_ClearScene = []( void ) -> void {
		re.ClearScene();
		return;
	};
	imports.RE_AddRefEntityToScene = []( const refEntity_t *entity ) -> void {
		re.AddRefEntityToScene( entity );
		return;
	};
	imports.RE_AddPolyToScene = []( qhandle_t hShader, int numVerts, const polyVert_t *verts ) -> void {
		re.AddPolyToScene( hShader, numVerts, verts );
		return;
		// Ridah
	};
	imports.RE_AddLightToScene = []( const vec3_t org, float intensity, float r, float g, float b, int overdraw ) -> void {
		re.AddLightToScene( org, intensity, r, g, b, overdraw );
		return;
	};
	imports.RE_AddCoronaToScene = []( const vec3_t org, float r, float g, float b, float scale, int id, int flags ) -> void {
		re.AddCoronaToScene( org, r, g, b, scale, id, flags );
		return;
	};
	imports.RE_RenderScene = []( const refdef_t *fd ) -> void {
		re.RenderScene( fd );
		return;
	};
	imports.RE_SetColor = []( const float *rgba ) -> void {
		re.SetColor( rgba );
		return;
	};
	imports.RE_DrawStretchPic = []( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader ) -> void {
		re.DrawStretchPic( x, y, w, h, s1, t1, s2, t2, hShader );
		return;
	};
	imports.RE_ModelBounds = []( clipHandle_t model, vec3_t mins, vec3_t maxs ) -> void {
		re.ModelBounds( model, mins, maxs );
		return;
	};
	imports.SCR_UpdateScreen = []( void ) -> void {
		SCR_UpdateScreen();
		return;
	};
	imports.RE_LerpTag = []( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex ) -> int {
		return re.LerpTag( tag, refent, tagName, startIndex );
	};
	imports.S_StartLocalSound = []( sfxHandle_t sfx, int channelNum ) -> void {
		S_StartLocalSound( sfx, channelNum );
		return;
		//----(SA)	added
	};
	imports.S_RegisterSound = []( const char *sample ) -> sfxHandle_t {
		#ifdef DOOMSOUND    ///// (SA) DOOMSOUND
		return S_RegisterSound( sample );
		#else
		return S_RegisterSound( sample, qfalse );
		#endif  ///// (SA) DOOMSOUND
	};
	imports.S_FadeStreamingSound = []( float targetvol, int time, int num ) -> void {
		S_FadeStreamingSound( targetvol, time, num );
		return;
	};
	imports.S_FadeAllSounds = []( float targetvol, int time ) -> void {
		S_FadeAllSounds( targetvol, time );
		return;
		//----(SA)	end
	};
	imports.Key_KeynumToStringBuf = []( int keynum, char *buf, int buflen ) -> void {
		Key_KeynumToStringBuf( keynum, buf, buflen );
		return;
	};
	imports.Key_GetBindingBuf = []( int keynum, char *buf, int buflen ) -> void {
		Key_GetBindingBuf( keynum, buf, buflen );
		return;
	};
	imports.Key_SetBinding = []( int keynum, const char *binding ) -> void {
		Key_SetBinding( keynum, binding );
		return;
	};
	imports.Key_IsDown = []( int keynum ) -> qboolean {
		return Key_IsDown( keynum );
	};
	imports.Key_GetOverstrikeMode = []( void ) -> qboolean {
		return Key_GetOverstrikeMode();
	};
	imports.Key_SetOverstrikeMode = []( qboolean state ) -> void {
		Key_SetOverstrikeMode( state );
		return;
	};
	imports.Key_ClearStates = []( void ) -> void {
		Key_ClearStates();
		return;
	};
	imports.Key_GetCatcher = []( void ) -> int {
		return Key_GetCatcher();
	};
	imports.Key_SetCatcher = []( int catcher ) -> void {
		Key_SetCatcher( catcher );
		return;
	};
	imports.GetClipboardData = []( char *buf, int bufsize ) -> void {
		GetClipboardData( buf, bufsize );
		return;
	};
	imports.GetClientState = []( uiClientState_t *state ) -> void {
		GetClientState( state );
		return;
	};
	imports.CL_GetGlconfig = []( glconfig_t *glconfig ) -> void {
		CL_GetGlconfig( glconfig );
		return;
	};
	imports.GetConfigString = []( int index, char* buff, int buffsize ) -> int {
		return GetConfigString( index, buff, buffsize );
	};
	imports.LAN_GetLocalServerCount = []( void ) -> int { return cls.numlocalservers; };
	imports.LAN_GetLocalServerAddressString = []( int n, char *buf, int buflen ) -> void { LAN_GetServerAddressString( AS_LOCAL, n, buf, buflen ); };
	imports.LAN_GetGlobalServerCount = []( void ) -> int { return cls.numglobalservers; };
	imports.LAN_GetGlobalServerAddressString = []( int n, char *buf, int buflen ) -> void { LAN_GetServerAddressString( AS_GLOBAL, n, buf, buflen ); };
	imports.LAN_GetPingQueueCount = []( void ) -> int {
		return LAN_GetPingQueueCount();
	};
	imports.LAN_ClearPing = []( int n ) -> void {
		LAN_ClearPing( n );
		return;
	};
	imports.LAN_GetPing = []( int n, char *buf, int buflen, int *pingtime ) -> void {
		LAN_GetPing( n, buf, buflen, pingtime );
		return;
	};
	imports.LAN_GetPingInfo = []( int n, char *buf, int buflen ) -> void {
		LAN_GetPingInfo( n, buf, buflen );
		return;
	};
	imports.LAN_UpdateVisiblePings = []( int source ) -> qboolean {
		return LAN_UpdateVisiblePings( source );
	};
	imports.LAN_GetServerCount = []( int source ) -> int {
		return LAN_GetServerCount( source );
	};
	imports.LAN_CompareServers = []( int source, int sortKey, int sortDir, int s1, int s2 ) -> int {
		return LAN_CompareServers( source, sortKey, sortDir, s1, s2 );
	};
	imports.LAN_GetServerAddressString = []( int source, int n, char *buf, int buflen ) -> void {
		LAN_GetServerAddressString( source, n, buf, buflen );
		return;
	};
	imports.LAN_GetServerInfo = []( int source, int n, char *buf, int buflen ) -> void {
		LAN_GetServerInfo( source, n, buf, buflen );
		return;
	};
	imports.LAN_AddServer = []( int source, const char *name, const char *addr ) -> int {
		return LAN_AddServer( source, name, addr );
	};
	imports.LAN_RemoveServer = []( int source, const char *addr ) -> void {
		LAN_RemoveServer( source, addr );
		return;
	};
	imports.LAN_GetServerPing = []( int source, int n ) -> int {
		return LAN_GetServerPing( source, n );
	};
	imports.LAN_ServerIsVisible = []( int source, int n ) -> int {
		return LAN_ServerIsVisible( source, n );
	};
	imports.LAN_GetServerStatus = []( const char *serverAddress, char *serverStatus, int maxLen ) -> int {
		return LAN_GetServerStatus( (char *)serverAddress, serverStatus, maxLen );
	};
	imports.LAN_SaveServersToCache = []( void ) -> void {
		LAN_SaveServersToCache();
		return;
	};
	imports.LAN_LoadCachedServers = []( void ) -> void {
		LAN_LoadCachedServers();
		return;
	};
	imports.LAN_MarkServerVisible = []( int source, int n, qboolean visible ) -> void {
		LAN_MarkServerVisible( source, n, visible );
		return;
	};
	imports.LAN_ResetPings = []( int n ) -> void {
		LAN_ResetPings( n );
		return;
	};
	imports.Hunk_MemoryRemaining = []( void ) -> int {
		return Hunk_MemoryRemaining();
	};
	imports.CLUI_GetCDKey = []( char *buf, int buflen ) -> void {
		CLUI_GetCDKey( buf, buflen );
		return;
	};
	imports.CLUI_SetCDKey = []( char *buf ) -> void {
		CLUI_SetCDKey( buf );
		return;
	};
	imports.PC_AddGlobalDefine = []( char *define ) -> int {
		return botlib_export->PC_AddGlobalDefine( define );
	};
	imports.PC_LoadSourceHandle = []( const char *filename ) -> int {
		return botlib_export->PC_LoadSourceHandle( filename );
	};
	imports.PC_FreeSourceHandle = []( int handle ) -> int {
		return botlib_export->PC_FreeSourceHandle( handle );
	};
	imports.PC_ReadTokenHandle = []( int handle, pc_token_t *pc_token ) -> int {
		return botlib_export->PC_ReadTokenHandle( handle, pc_token );
	};
	imports.PC_SourceFileAndLine = []( int handle, char *filename, int *line ) -> int {
		return botlib_export->PC_SourceFileAndLine( handle, filename, line );
	};
	imports.S_StopBackgroundTrack = []( void ) -> void {
		S_StopBackgroundTrack();
		return;
	};
	imports.S_StartBackgroundTrack = []( const char *intro, const char *loop, int fadeupTime ) -> void {
		S_StartBackgroundTrack( intro, loop, fadeupTime );   //----(SA)	added fadeup time
		return;
	};
	imports.Com_RealTime = []( qtime_t *qtime ) -> int {
		return Com_RealTime( qtime );
	};
	imports.CIN_PlayCinematic = []( const char *arg0, int xpos, int ypos, int width, int height, int bits ) -> int {
		Com_DPrintf( "UI_CIN_PlayCinematic\n" );
		return CIN_PlayCinematic( arg0, xpos, ypos, width, height, bits );
	};
	imports.CIN_StopCinematic = []( int handle ) -> e_status {
		return CIN_StopCinematic( handle );
	};
	imports.CIN_RunCinematic = []( int handle ) -> e_status {
		return CIN_RunCinematic( handle );
	};
	imports.CIN_DrawCinematic = []( int handle ) -> void {
		CIN_DrawCinematic( handle );
		return;
	};
	imports.CIN_SetExtents = []( int handle, int x, int y, int w, int h ) -> void {
		CIN_SetExtents( handle, x, y, w, h );
		return;
	};
	imports.RE_RemapShader = []( const char *oldShader, const char *newShader, const char *timeOffset ) -> void {
		re.RemapShader( oldShader, newShader, timeOffset );
		return;
	};
	imports.CL_CDKeyValidate = []( const char *key, const char *chksum ) -> qboolean {
		return CL_CDKeyValidate( key, chksum );

		// NERVE - SMF
	};
	imports.CL_GetLimboString = []( int index, char *buf ) -> qboolean {
		return CL_GetLimboString( index, buf );
		// -NERVE - SMF
	};
	return imports;
}



/*
====================
CL_ShutdownUI
====================
*/
void CL_ShutdownUI( void ) {
	cls.keyCatchers &= ~KEYCATCH_UI;
	cls.uiStarted = qfalse;
	if ( !uivm ) {
		return;
	}
	uivm->UI_SHUTDOWN();
	uivm = NULL;
	Sys_UnloadDll( uivmHandle );
	uivmHandle = NULL;
}

/*
====================
CL_InitUI
====================
*/

void CL_InitUI( void ) {
	void *entryPoint = NULL;
	GetUIAPI_t getAPI;
	static const uiImport_t imports = CL_CreateUIImports();

	uivmHandle = Sys_LoadDll( "ui", "GetUIAPI", &entryPoint );
	getAPI = (GetUIAPI_t)entryPoint;
	uivm = getAPI ? getAPI( &imports ) : NULL;

	if ( !uivm ) {
		Com_Error( ERR_FATAL, "Could not load the UI function API" );
	}

	if ( uivm->apiVersion != UI_API_VERSION ) {
		Com_Error( ERR_FATAL, "User Interface is version %d, expected %d", uivm->apiVersion, UI_API_VERSION );
		cls.uiStarted = qfalse;
	}

	// init for this gamestate
//	uivm->UI_INIT();
	uivm->UI_INIT(( cls.state >= CA_AUTHORIZING && cls.state < CA_ACTIVE ) );
}


qboolean UI_usesUniqueCDKey() {
	if ( uivm ) {
		return ( uivm->UI_HASUNIQUECDKEY() == qtrue );
	} else {
		return qfalse;
	}
}

/*
====================
UI_GameCommand

See if the current console command is claimed by the ui
====================
*/
qboolean UI_GameCommand( void ) {
	if ( !uivm ) {
		return qfalse;
	}

	return uivm->UI_CONSOLE_COMMAND(cls.realtime );
}
