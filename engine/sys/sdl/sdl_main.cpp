// sdl_main.cpp -- SDL3 entry point and core system services,
// replaces win_main.cpp / win_shared.cpp / win_wndproc.cpp

#include "sdl_local.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// win32 console window helpers, implemented in win_syscon.cpp
#ifdef _WIN32
void Conbuf_AppendText( const char *msg );
void Sys_CreateConsole( void );
void Sys_DestroyConsole( void );
void Sys_ShowConsole( int level, qboolean quitOnClose );
void Sys_SetErrorText( const char *text );
#endif

extern char *FS_BuildOSPath( const char *base, const char *game, const char *qpath );

static char sys_cmdline[MAX_STRING_CHARS];
static qboolean s_minimized = qfalse;

int totalMsec = 0;
int countMsec = 0;

/*
================
Sys_Milliseconds
================
*/
int Sys_Milliseconds( void ) {
	static qboolean initialized = qfalse;
	static Uint64 base;
	Uint64 now = SDL_GetTicks();

	if ( !initialized ) {
		base = now;
		initialized = qtrue;
	}

	return (int)( now - base );
}

/*
================
Sys_LowPhysicalMemory
================
*/
qboolean Sys_LowPhysicalMemory() {
	return SDL_GetSystemRAM() <= 96;
}

unsigned int Sys_ProcessorCount() {
	return (unsigned int)SDL_GetNumLogicalCPUCores();
}

void Sys_BeginProfiling( void ) {
}

void Sys_EndProfiling( void ) {
}

/*
================
Sys_SnapVector
================
*/
void Sys_SnapVector( float *v ) {
	int i;

	for ( i = 0; i < 3; i++ ) {
		v[i] = (float)(int)( v[i] + ( v[i] > 0.0f ? 0.5f : -0.5f ) );
	}
}

/*
================
Sys_InitStreamThread

Stream file access used to run through a win32 helper thread, the
stream functions are plain stubs now
================
*/
void Sys_InitStreamThread( void ) {
}

void Sys_ShutdownStreamThread( void ) {
}

void Sys_BeginStreamedFile( fileHandle_t f, int readahead ) {
	(void)f;
	(void)readahead;
}

void Sys_EndStreamedFile( fileHandle_t f ) {
	(void)f;
}

int Sys_StreamedRead( void *buffer, int size, int count, fileHandle_t f ) {
	// f is an fsh index, not a FILE*, so the read must go through the filesystem
	return FS_Read( buffer, size * count, f );
}

void Sys_StreamSeek( fileHandle_t f, int offset, int origin ) {
	FS_Seek( f, offset, origin );
}

void *Sys_InitializeCriticalSection() {
	return (void *)-1;
}

void Sys_EnterCriticalSection( void *ptr ) {
	(void)ptr;
}

void Sys_LeaveCriticalSection( void *ptr ) {
	(void)ptr;
}

/*
================
Sys_GetCurrentUser
================
*/
char *Sys_GetCurrentUser( void ) {
	static char s_userName[1024];
	const char *userName;

	userName = getenv( "USERNAME" );
	if ( userName == NULL || userName[0] == '\0' ) {
		userName = getenv( "USER" );
	}
	if ( userName == NULL || userName[0] == '\0' ) {
		userName = "player";
	}

	Q_strncpyz( s_userName, userName, sizeof( s_userName ) );
	return s_userName;
}

/*
================
Sys_GetClipboardData
================
*/
char *Sys_GetClipboardData( void ) {
	char *data = NULL;
	char *cliptext = SDL_GetClipboardText();

	if ( cliptext != NULL && cliptext[0] != '\0' ) {
		int len = (int)strlen( cliptext );
		data = (char *)Z_Malloc( len + 1 );
		Q_strncpyz( data, cliptext, len + 1 );
	}

	if ( cliptext != NULL ) {
		SDL_free( cliptext );
	}

	if ( data != NULL ) {
		// cut off any trailing newlines or other control characters
		strtok( data, "\n\r\b" );
	}

	return data;
}

/*
================
Sys_StartProcess
================
*/
void Sys_StartProcess( char *exeName, qboolean doexit ) {
#ifdef _WIN32
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;

	memset( &startupInfo, 0, sizeof( startupInfo ) );
	startupInfo.cb = sizeof( startupInfo );
	memset( &processInfo, 0, sizeof( processInfo ) );

	if ( !CreateProcess( exeName, NULL, NULL, NULL, FALSE, 0, NULL, Sys_Cwd(),
						  &startupInfo, &processInfo ) ) {
		return;
	}
#else
	pid_t pid = fork();
	if ( pid == 0 ) {
		chdir( Sys_Cwd() );
		execl( exeName, exeName, (char *)NULL );
		_exit( 1 );
	}
#endif
	if ( doexit ) {
		Cbuf_ExecuteText( EXEC_APPEND, "quit" );
	}
}

/*
================
Sys_OpenURL
================
*/
void Sys_OpenURL( char *url, qboolean doexit ) {
	if ( !SDL_OpenURL( url ) ) {
		Com_Error( ERR_DROP, "Could not open url %s\n", url );
		return;
	}

	if ( doexit ) {
		Cbuf_ExecuteText( EXEC_APPEND, "quit" );
	}
}

/*
================
Sys_LoadDll

Used to load a development dll instead of a virtual machine
================
*/
void *QDECL Sys_LoadDll( const char *name, const char *entryPointName, void **entryPoint ) {
	SDL_SharedObject *libHandle;
	const char *basepath;
	const char *cdpath;
	const char *gamedir;
	char *fn;
	char filename[MAX_QPATH];

#ifdef _WIN32
	Com_sprintf( filename, sizeof( filename ), "%sx86.dll", name );
#else
	Com_sprintf( filename, sizeof( filename ), "%s.so", name );
#endif

	// check the current working directory first, like the old code
	libHandle = SDL_LoadObject( filename );

	if ( libHandle == NULL ) {
		basepath = Cvar_VariableString( "fs_basepath" );
		cdpath = Cvar_VariableString( "fs_cdpath" );
		gamedir = Cvar_VariableString( "fs_game" );

		fn = FS_BuildOSPath( basepath, gamedir, filename );
		libHandle = SDL_LoadObject( fn );
		if ( libHandle == NULL && cdpath[0] != '\0' ) {
			fn = FS_BuildOSPath( cdpath, gamedir, filename );
			libHandle = SDL_LoadObject( fn );
		}
		if ( libHandle == NULL ) {
			return NULL;
		}
	}

	*entryPoint = (void *)SDL_LoadFunction( libHandle, entryPointName );
	if ( *entryPoint == NULL ) {
		SDL_UnloadObject( libHandle );
		return NULL;
	}

	return libHandle;
}

/*
================
Sys_UnloadDll
================
*/
void Sys_UnloadDll( void *dllHandle ) {
	if ( dllHandle == NULL ) {
		return;
	}
	SDL_UnloadObject( (SDL_SharedObject *)dllHandle );
}

/*
================
Sys_GetProcessorId
================
*/
int Sys_GetProcessorId( void ) {
	// the old per-cpu detection is gone, everything modern is generic
	return CPUID_GENERIC;
}

int Sys_GetHighQualityCPU() {
	return 1;
}

/*
================
Sys_Error
================
*/
void QDECL Sys_Error( const char *error, ... ) {
	va_list argptr;
	char text[4096];

	va_start( argptr, error );
	vsnprintf( text, sizeof( text ), error, argptr );
	va_end( argptr );

	Sys_Print( text );
	Sys_Print( "\n" );

#ifdef _WIN32
	Sys_SetErrorText( text );
	Sys_ShowConsole( 1, qtrue );
#endif

	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "WolfSP", text, NULL );

	IN_Shutdown();

	exit( 1 );
}

/*
================
Sys_Quit
================
*/
void Sys_Quit( void ) {
	IN_Shutdown();
#ifdef _WIN32
	Sys_DestroyConsole();
#endif
	exit( 0 );
}

/*
================
Sys_Print
================
*/
void Sys_Print( const char *msg ) {
#ifdef _WIN32
	Conbuf_AppendText( msg );
#else
	fputs( msg, stdout );
#endif
}

/*
================
Sys_Mkdir
================
*/
void Sys_Mkdir( const char *path ) {
#ifdef _WIN32
	_mkdir( path );
#else
	mkdir( path, 0777 );
#endif
}

char *Sys_Cwd( void ) {
	static char cwd[MAX_OSPATH];

#ifdef _WIN32
	_getcwd( cwd, sizeof( cwd ) - 1 );
#else
	getcwd( cwd, sizeof( cwd ) - 1 );
#endif
	cwd[MAX_OSPATH - 1] = '\0';

	return cwd;
}

char *Sys_DefaultCDPath( void ) {
	return "";
}

char *Sys_DefaultBasePath( void ) {
	return Sys_Cwd();
}

char *Sys_DefaultInstallPath( void ) {
	return Sys_Cwd();
}

char *Sys_DefaultHomePath( void ) {
	return NULL;
}

qboolean Sys_CheckCD( void ) {
	return qtrue;
}

#ifndef _WIN32
// the win32 console window does not exist on other platforms
void Sys_DisplaySystemConsole( qboolean show ) {
	(void)show;
}

void Sys_ShowConsole( int level, qboolean quitOnClose ) {
	(void)level;
	(void)quitOnClose;
}

void Sys_SetErrorText( const char *text ) {
	(void)text;
}

/*
================
Sys_ConsoleInput

Read a line from the terminal if one is ready, without blocking
================
*/
char *Sys_ConsoleInput( void ) {
	static char text[256];
	int len = 0;
	int ready;
	char ch;
	fd_set fds;
	struct timeval tv;

	FD_ZERO( &fds );
	FD_SET( fileno( stdin ), &fds );
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	ready = select( fileno( stdin ) + 1, &fds, NULL, NULL, &tv );
	if ( ready <= 0 ) {
		return NULL;
	}

	while ( read( fileno( stdin ), &ch, 1 ) == 1 ) {
		if ( ch == '\n' ) {
			text[len] = '\0';
			return text;
		}
		if ( len < (int)sizeof( text ) - 1 ) {
			text[len] = ch;
			len++;
		}
	}

	if ( len > 0 ) {
		text[len] = '\0';
		return text;
	}

	return NULL;
}
#endif

/*
================
Sys_ListFiles

Returns a list of files in a directory, optionally filtered and
optionally restricted to subdirectories
================
*/
#define MAX_FOUND_FILES  0x1000

// returns qtrue if s0 is lexically greater than s1
static qboolean strgtr( const char *s0, const char *s1 ) {
	int l0 = (int)strlen( s0 );
	int l1 = (int)strlen( s1 );
	int i;

	if ( l0 < l1 ) {
		return qfalse;
	}
	if ( l0 > l1 ) {
		return qtrue;
	}

	for ( i = 0; i < l1; i++ ) {
		if ( s0[i] > s1[i] ) {
			return qtrue;
		}
	}
	return qfalse;
}

#ifdef _WIN32

/*
================
Sys_ListFilteredFiles
================
*/
static void Sys_ListFilteredFiles( const char *basedir, const char *subdirs, char *filter, char **list, int *numfiles ) {
	char search[MAX_OSPATH];
	char newsubdirs[MAX_OSPATH];
	char filename[MAX_OSPATH];
	WIN32_FIND_DATAA findinfo;
	HANDLE findhandle;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if ( strlen( subdirs ) > 0 ) {
		Com_sprintf( search, sizeof( search ), "%s/%s", basedir, subdirs );
	} else {
		Com_sprintf( search, sizeof( search ), "%s", basedir );
	}

	Com_sprintf( filename, sizeof( filename ), "%s/*", search );
	findhandle = FindFirstFileA( filename, &findinfo );
	if ( findhandle == INVALID_HANDLE_VALUE ) {
		return;
	}

	do {
		if ( Q_stricmp( findinfo.cFileName, "." ) == 0 || Q_stricmp( findinfo.cFileName, ".." ) == 0 ) {
			continue;
		}

		if ( ( findinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) {
			// recurse into the subdirectory
			if ( strlen( subdirs ) > 0 ) {
				Com_sprintf( newsubdirs, sizeof( newsubdirs ), "%s/%s", subdirs, findinfo.cFileName );
			} else {
				Com_sprintf( newsubdirs, sizeof( newsubdirs ), "%s", findinfo.cFileName );
			}
			Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
		} else {
			// build the relative filename and test it against the filter
			if ( strlen( subdirs ) > 0 ) {
				Com_sprintf( filename, sizeof( filename ), "%s/%s", subdirs, findinfo.cFileName );
			} else {
				Com_sprintf( filename, sizeof( filename ), "%s", findinfo.cFileName );
			}
			if ( Com_FilterPath( filter, filename, qfalse ) ) {
				list[*numfiles] = CopyString( filename );
				(*numfiles)++;
			}
		}

		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
	} while ( FindNextFileA( findhandle, &findinfo ) != 0 );

	FindClose( findhandle );
}

/*
================
Sys_ListFiles
================
*/
char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, qboolean wantsubs ) {
	char *list[MAX_FOUND_FILES];
	char **listCopy;
	WIN32_FIND_DATAA findinfo;
	HANDLE findhandle;
	char pattern[MAX_OSPATH];
	int nfiles = 0;
	int i, j;
	int dlen;
	int elen;

	if ( filter != NULL ) {
		memset( list, 0, sizeof( list ) );
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );

		*numfiles = nfiles;
		if ( nfiles == 0 ) {
			return NULL;
		}

		list[nfiles] = NULL;
		listCopy = (char **)Z_Malloc( ( nfiles + 1 ) * sizeof( char * ) );
		memcpy( listCopy, list, ( nfiles + 1 ) * sizeof( char * ) );
		return listCopy;
	}

	if ( extension == NULL ) {
		// pass a slash extension to list directories only
		extension = "";
	}

	Com_sprintf( pattern, sizeof( pattern ), "%s/*", directory );
	findhandle = FindFirstFileA( pattern, &findinfo );
	if ( findhandle == INVALID_HANDLE_VALUE ) {
		*numfiles = 0;
		return NULL;
	}

	do {
		if ( Q_stricmp( findinfo.cFileName, "." ) == 0 || Q_stricmp( findinfo.cFileName, ".." ) == 0 ) {
			continue;
		}

		// keep only what the caller asked for: dirs or plain files
		if ( ( ( findinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) != wantsubs ) {
			continue;
		}

		// match the extension on plain files
		if ( !wantsubs ) {
			dlen = (int)strlen( findinfo.cFileName );
			elen = (int)strlen( extension );
			if ( elen > 0 && ( dlen < elen || Q_stricmp( findinfo.cFileName + dlen - elen, extension ) != 0 ) ) {
				continue;
			}
		}

		if ( nfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		list[nfiles] = CopyString( findinfo.cFileName );
		nfiles++;
	} while ( FindNextFileA( findhandle, &findinfo ) != 0 );

	FindClose( findhandle );

	// sort the list (bubble sort, same as the old code)
	for ( i = 0; i < nfiles - 1; i++ ) {
		for ( j = 0; j < nfiles - 1 - i; j++ ) {
			if ( strgtr( list[j], list[j + 1] ) ) {
				char *temp = list[j];
				list[j] = list[j + 1];
				list[j + 1] = temp;
			}
		}
	}

	*numfiles = nfiles;
	if ( nfiles == 0 ) {
		return NULL;
	}

	list[nfiles] = NULL;
	listCopy = (char **)Z_Malloc( ( nfiles + 1 ) * sizeof( char * ) );
	memcpy( listCopy, list, ( nfiles + 1 ) * sizeof( char * ) );
	return listCopy;
}

#else

static void Sys_ListFilteredFiles( const char *basedir, const char *subdirs, char *filter, char **list, int *numfiles ) {
	char search[MAX_OSPATH];
	char newsubdirs[MAX_OSPATH];
	char filename[MAX_OSPATH];
	char fullpath[MAX_OSPATH];
	DIR *dir;
	struct dirent *d;
	struct stat st;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if ( strlen( subdirs ) > 0 ) {
		Com_sprintf( search, sizeof( search ), "%s/%s", basedir, subdirs );
	} else {
		Com_sprintf( search, sizeof( search ), "%s", basedir );
	}

	dir = opendir( search );
	if ( dir == NULL ) {
		return;
	}

	while ( ( d = readdir( dir ) ) != NULL ) {
		if ( Q_stricmp( d->d_name, "." ) == 0 || Q_stricmp( d->d_name, ".." ) == 0 ) {
			continue;
		}

		Com_sprintf( fullpath, sizeof( fullpath ), "%s/%s", search, d->d_name );
		if ( stat( fullpath, &st ) == 0 && S_ISDIR( st.st_mode ) ) {
			// recurse into the subdirectory
			if ( strlen( subdirs ) > 0 ) {
				Com_sprintf( newsubdirs, sizeof( newsubdirs ), "%s/%s", subdirs, d->d_name );
			} else {
				Com_sprintf( newsubdirs, sizeof( newsubdirs ), "%s", d->d_name );
			}
			Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
		} else {
			// build the relative filename and test it against the filter
			if ( strlen( subdirs ) > 0 ) {
				Com_sprintf( filename, sizeof( filename ), "%s/%s", subdirs, d->d_name );
			} else {
				Com_sprintf( filename, sizeof( filename ), "%s", d->d_name );
			}
			if ( Com_FilterPath( filter, filename, qfalse ) ) {
				list[*numfiles] = CopyString( filename );
				(*numfiles)++;
			}
		}

		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
	}
	closedir( dir );
}

char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, qboolean wantsubs ) {
	char *list[MAX_FOUND_FILES];
	char **listCopy;
	int nfiles = 0;
	int i, j;
	int dlen;
	int elen;
	DIR *dir;
	struct dirent *d;
	struct stat st;
	char fullpath[MAX_OSPATH];

	if ( filter != NULL ) {
		memset( list, 0, sizeof( list ) );
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );

		*numfiles = nfiles;
		if ( nfiles == 0 ) {
			return NULL;
		}

		list[nfiles] = NULL;
		listCopy = (char **)Z_Malloc( ( nfiles + 1 ) * sizeof( char * ) );
		memcpy( listCopy, list, ( nfiles + 1 ) * sizeof( char * ) );
		return listCopy;
	}

	if ( extension == NULL ) {
		// pass a slash extension to list directories only
		extension = "";
	}

	dir = opendir( directory );
	if ( dir == NULL ) {
		*numfiles = 0;
		return NULL;
	}

	while ( ( d = readdir( dir ) ) != NULL && nfiles < MAX_FOUND_FILES - 1 ) {
		if ( Q_stricmp( d->d_name, "." ) == 0 || Q_stricmp( d->d_name, ".." ) == 0 ) {
			continue;
		}

		Com_sprintf( fullpath, sizeof( fullpath ), "%s/%s", directory, d->d_name );
		if ( stat( fullpath, &st ) != 0 ) {
			continue;
		}

		// keep only what the caller asked for: dirs or plain files
		if ( S_ISDIR( st.st_mode ) != wantsubs ) {
			continue;
		}

		// match the extension on plain files
		if ( !wantsubs ) {
			dlen = (int)strlen( d->d_name );
			elen = (int)strlen( extension );
			if ( elen > 0 && ( dlen < elen || Q_stricmp( d->d_name + dlen - elen, extension ) != 0 ) ) {
				continue;
			}
		}

		list[nfiles] = CopyString( d->d_name );
		nfiles++;
	}
	closedir( dir );

	// sort the list (bubble sort, same as the old code)
	for ( i = 0; i < nfiles - 1; i++ ) {
		for ( j = 0; j < nfiles - 1 - i; j++ ) {
			if ( strgtr( list[j], list[j + 1] ) ) {
				char *temp = list[j];
				list[j] = list[j + 1];
				list[j + 1] = temp;
			}
		}
	}

	*numfiles = nfiles;
	if ( nfiles == 0 ) {
		return NULL;
	}

	list[nfiles] = NULL;
	listCopy = (char **)Z_Malloc( ( nfiles + 1 ) * sizeof( char * ) );
	memcpy( listCopy, list, ( nfiles + 1 ) * sizeof( char * ) );
	return listCopy;
}

#endif

void Sys_FreeFileList( char **list ) {
	int i;

	if ( list == NULL ) {
		return;
	}

	for ( i = 0; list[i] != NULL; i++ ) {
		Z_Free( list[i] );
	}
	Z_Free( list );
}

/*
================
Sys_QueEvent

The system queues events that the engine polls through Sys_GetEvent
================
*/
#define MAX_QUED_EVENTS    256
#define MASK_QUED_EVENTS   ( MAX_QUED_EVENTS - 1 )

static sysEvent_t eventQue[MAX_QUED_EVENTS];
static int eventHead = 0;
static int eventTail = 0;

void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr ) {
	sysEvent_t *ev = &eventQue[eventHead & MASK_QUED_EVENTS];

	// if the queue overflowed, we have to drop the least recent event
	if ( eventHead - eventTail >= MAX_QUED_EVENTS ) {
		Com_Printf( "Sys_QueEvent: overflow\n" );
		if ( ev->evPtr ) {
			Z_Free( ev->evPtr );
		}
		eventTail++;
	} else {
		eventHead++;
	}

	if ( time == 0 ) {
		time = Sys_Milliseconds();
	}

	ev->evTime = time;
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
}

/*
================
Sys_KeyFromScancode

Translate an SDL scancode into an engine key code
================
*/
static int Sys_KeyFromScancode( SDL_Scancode sc ) {
	switch ( sc ) {
	case SDL_SCANCODE_A: return 'a';
	case SDL_SCANCODE_B: return 'b';
	case SDL_SCANCODE_C: return 'c';
	case SDL_SCANCODE_D: return 'd';
	case SDL_SCANCODE_E: return 'e';
	case SDL_SCANCODE_F: return 'f';
	case SDL_SCANCODE_G: return 'g';
	case SDL_SCANCODE_H: return 'h';
	case SDL_SCANCODE_I: return 'i';
	case SDL_SCANCODE_J: return 'j';
	case SDL_SCANCODE_K: return 'k';
	case SDL_SCANCODE_L: return 'l';
	case SDL_SCANCODE_M: return 'm';
	case SDL_SCANCODE_N: return 'n';
	case SDL_SCANCODE_O: return 'o';
	case SDL_SCANCODE_P: return 'p';
	case SDL_SCANCODE_Q: return 'q';
	case SDL_SCANCODE_R: return 'r';
	case SDL_SCANCODE_S: return 's';
	case SDL_SCANCODE_T: return 't';
	case SDL_SCANCODE_U: return 'u';
	case SDL_SCANCODE_V: return 'v';
	case SDL_SCANCODE_W: return 'w';
	case SDL_SCANCODE_X: return 'x';
	case SDL_SCANCODE_Y: return 'y';
	case SDL_SCANCODE_Z: return 'z';
	case SDL_SCANCODE_1: return '1';
	case SDL_SCANCODE_2: return '2';
	case SDL_SCANCODE_3: return '3';
	case SDL_SCANCODE_4: return '4';
	case SDL_SCANCODE_5: return '5';
	case SDL_SCANCODE_6: return '6';
	case SDL_SCANCODE_7: return '7';
	case SDL_SCANCODE_8: return '8';
	case SDL_SCANCODE_9: return '9';
	case SDL_SCANCODE_0: return '0';
	case SDL_SCANCODE_RETURN: return K_ENTER;
	case SDL_SCANCODE_ESCAPE: return K_ESCAPE;
	case SDL_SCANCODE_BACKSPACE: return K_BACKSPACE;
	case SDL_SCANCODE_TAB: return K_TAB;
	case SDL_SCANCODE_SPACE: return K_SPACE;
	case SDL_SCANCODE_APOSTROPHE: return '\'';
	case SDL_SCANCODE_COMMA: return ',';
	case SDL_SCANCODE_MINUS: return '-';
	case SDL_SCANCODE_PERIOD: return '.';
	case SDL_SCANCODE_SLASH: return '/';
	case SDL_SCANCODE_SEMICOLON: return ';';
	case SDL_SCANCODE_EQUALS: return '=';
	case SDL_SCANCODE_LEFTBRACKET: return '[';
	case SDL_SCANCODE_BACKSLASH: return '\\';
	case SDL_SCANCODE_RIGHTBRACKET: return ']';
	case SDL_SCANCODE_GRAVE: return '`';
	case SDL_SCANCODE_CAPSLOCK: return K_CAPSLOCK;
	case SDL_SCANCODE_PAUSE: return K_PAUSE;
	case SDL_SCANCODE_INSERT: return K_INS;
	case SDL_SCANCODE_HOME: return K_HOME;
	case SDL_SCANCODE_PAGEUP: return K_PGUP;
	case SDL_SCANCODE_DELETE: return K_DEL;
	case SDL_SCANCODE_END: return K_END;
	case SDL_SCANCODE_PAGEDOWN: return K_PGDN;
	case SDL_SCANCODE_RIGHT: return K_RIGHTARROW;
	case SDL_SCANCODE_LEFT: return K_LEFTARROW;
	case SDL_SCANCODE_DOWN: return K_DOWNARROW;
	case SDL_SCANCODE_UP: return K_UPARROW;
	case SDL_SCANCODE_NUMLOCKCLEAR: return K_KP_NUMLOCK;
	case SDL_SCANCODE_KP_DIVIDE: return K_KP_SLASH;
	case SDL_SCANCODE_KP_MULTIPLY: return K_KP_STAR;
	case SDL_SCANCODE_KP_MINUS: return K_KP_MINUS;
	case SDL_SCANCODE_KP_PLUS: return K_KP_PLUS;
	case SDL_SCANCODE_KP_ENTER: return K_KP_ENTER;
	case SDL_SCANCODE_KP_1: return K_KP_END;
	case SDL_SCANCODE_KP_2: return K_KP_DOWNARROW;
	case SDL_SCANCODE_KP_3: return K_KP_PGDN;
	case SDL_SCANCODE_KP_4: return K_KP_LEFTARROW;
	case SDL_SCANCODE_KP_5: return K_KP_5;
	case SDL_SCANCODE_KP_6: return K_KP_RIGHTARROW;
	case SDL_SCANCODE_KP_7: return K_KP_HOME;
	case SDL_SCANCODE_KP_8: return K_KP_UPARROW;
	case SDL_SCANCODE_KP_9: return K_KP_PGUP;
	case SDL_SCANCODE_KP_0: return K_KP_INS;
	case SDL_SCANCODE_KP_PERIOD: return K_KP_DEL;
	case SDL_SCANCODE_F1: return K_F1;
	case SDL_SCANCODE_F2: return K_F2;
	case SDL_SCANCODE_F3: return K_F3;
	case SDL_SCANCODE_F4: return K_F4;
	case SDL_SCANCODE_F5: return K_F5;
	case SDL_SCANCODE_F6: return K_F6;
	case SDL_SCANCODE_F7: return K_F7;
	case SDL_SCANCODE_F8: return K_F8;
	case SDL_SCANCODE_F9: return K_F9;
	case SDL_SCANCODE_F10: return K_F10;
	case SDL_SCANCODE_F11: return K_F11;
	case SDL_SCANCODE_F12: return K_F12;
	case SDL_SCANCODE_LSHIFT:
	case SDL_SCANCODE_RSHIFT: return K_SHIFT;
	case SDL_SCANCODE_LCTRL:
	case SDL_SCANCODE_RCTRL: return K_CTRL;
	case SDL_SCANCODE_LALT:
	case SDL_SCANCODE_RALT: return K_ALT;
	default: return 0;
	}
}

/*
================
Sys_GetEvent

Pump the SDL event loop and return one engine event
================
*/
sysEvent_t Sys_GetEvent( void ) {
	sysEvent_t ev;
	char *s;
	int key;
	int i;
	SDL_Event e;

	// pump the SDL event loop
	while ( SDL_PollEvent( &e ) ) {
		switch ( e.type ) {
		case SDL_EVENT_QUIT:
			Cbuf_ExecuteText( EXEC_APPEND, "quit" );
			break;

		case SDL_EVENT_KEY_DOWN:
			// alt+enter toggles fullscreen, like the old win32 code
			if ( e.key.key == SDLK_RETURN && ( e.key.mod & SDL_KMOD_ALT ) && !e.key.repeat ) {
				Cvar_SetValue( "r_fullscreen", !Cvar_VariableIntegerValue( "r_fullscreen" ) );
				Cbuf_ExecuteText( EXEC_APPEND, "vid_restart" );
				break;
			}
			key = Sys_KeyFromScancode( e.key.scancode );
			if ( key ) {
				Sys_QueEvent( 0, SE_KEY, key, qtrue, 0, NULL );

				// the console and chat fields edit their text through char
				// events and the old win32 code delivered backspace as
				// char 8, so queue one as well
				if ( key == K_BACKSPACE ) {
					Sys_QueEvent( 0, SE_CHAR, 8, 0, 0, NULL );
				}
			}
			break;

		case SDL_EVENT_KEY_UP:
			key = Sys_KeyFromScancode( e.key.scancode );
			if ( key ) {
				Sys_QueEvent( 0, SE_KEY, key, qfalse, 0, NULL );
			}
			break;

		case SDL_EVENT_TEXT_INPUT:
			for ( i = 0; e.text.text[i] != '\0'; i++ ) {
				if ( (unsigned char)e.text.text[i] < 128 ) {
					Sys_QueEvent( 0, SE_CHAR, e.text.text[i], 0, 0, NULL );
				}
			}
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP: {
			qboolean down = ( e.type == SDL_EVENT_MOUSE_BUTTON_DOWN ) ? qtrue : qfalse;

			switch ( e.button.button ) {
			case SDL_BUTTON_LEFT:   key = K_MOUSE1; break;
			case SDL_BUTTON_RIGHT:  key = K_MOUSE2; break;
			case SDL_BUTTON_MIDDLE: key = K_MOUSE3; break;
			case SDL_BUTTON_X1:     key = K_MOUSE4; break;
			case SDL_BUTTON_X2:     key = K_MOUSE5; break;
			default:                key = 0; break;
			}
			if ( key ) {
				Sys_QueEvent( 0, SE_KEY, key, down, 0, NULL );
			}
			break;
		}

		case SDL_EVENT_MOUSE_WHEEL:
			// the old win32 code queued wheel down and up pairs
			if ( e.wheel.y > 0.0f ) {
				Sys_QueEvent( 0, SE_KEY, K_MWHEELUP, qtrue, 0, NULL );
				Sys_QueEvent( 0, SE_KEY, K_MWHEELUP, qfalse, 0, NULL );
			} else if ( e.wheel.y < 0.0f ) {
				Sys_QueEvent( 0, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL );
				Sys_QueEvent( 0, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL );
			}
			break;

		case SDL_EVENT_WINDOW_FOCUS_GAINED:
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			Key_ClearStates();
			IN_Activate( e.type == SDL_EVENT_WINDOW_FOCUS_GAINED ? qtrue : qfalse );
			SNDDMA_Activate();
			break;

		case SDL_EVENT_WINDOW_MINIMIZED:
			s_minimized = qtrue;
			break;

		case SDL_EVENT_WINDOW_RESTORED:
			s_minimized = qfalse;
			break;

		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			// keep the client's copy of the video size in sync with the
			// live window size, then let the renderer pick it up as well
			if ( e.window.data1 > 0 && e.window.data2 > 0 ) {
				cls.glconfig.vidWidth = e.window.data1;
				cls.glconfig.vidHeight = e.window.data2;
				cls.glconfig.windowAspect = (float)e.window.data1 / (float)e.window.data2;
				GLimp_UpdateWindowSize();
			}
			break;

		default:
			break;
		}
	}

	// check for console commands
	s = Sys_ConsoleInput();
	if ( s ) {
		char *b;
		int len = (int)strlen( s ) + 1;

		b = (char *)Z_Malloc( len );
		memcpy( b, s, len );
		Sys_QueEvent( 0, SE_CONSOLE, 0, 0, len, b );
	}

	// check for network packets
	{
		netadr_t from;
		msg_t netmsg;
		static byte netmsgBuffer[MAX_MSGLEN];

		MSG_Init( &netmsg, netmsgBuffer, sizeof( netmsgBuffer ) );
		if ( Sys_GetPacket( &from, &netmsg ) ) {
			netadr_t *buf;
			int len = sizeof( netadr_t ) + netmsg.cursize;

			buf = (netadr_t *)Z_Malloc( len );
			*buf = from;
			memcpy( buf + 1, netmsg.data, netmsg.cursize );
			Sys_QueEvent( 0, SE_PACKET, 0, 0, len, buf );
		}
	}

	// return if we have data
	if ( eventHead > eventTail ) {
		eventTail++;
		return eventQue[( eventTail - 1 ) & MASK_QUED_EVENTS];
	}

	// create an empty event to return
	memset( &ev, 0, sizeof( ev ) );
	ev.evTime = Sys_Milliseconds();

	return ev;
}

/*
================
Sys_Init

Called after the memory systems are up and running
================
*/
static void Sys_In_Restart_f( void ) {
	IN_Shutdown();
	IN_Init();
}

static void Sys_Net_Restart_f( void ) {
	NET_Restart();
}

void Sys_Init( void ) {
	cvar_t *cv;

	Cmd_AddCommand( "in_restart", Sys_In_Restart_f );
	Cmd_AddCommand( "net_restart", Sys_Net_Restart_f );

#if defined( _WIN32 )
	Cvar_Set( "arch", "windows" );
#elif defined( __APPLE__ )
	Cvar_Set( "arch", "macos" );
#else
	Cvar_Set( "arch", "linux" );
#endif

	// figure out our cpu
	cv = Cvar_Get( "sys_cpustring", "detect", 0 );
	if ( Q_stricmp( cv->string, "detect" ) == 0 ) {
		// the old per-cpu detection is gone, everything modern is generic
		Cvar_Set( "sys_cpustring", "generic" );
	}
	Com_Printf( "CPU: %s\n", cv->string );

	Cvar_Get( "sys_cpuid", "none", 0 );
	Cvar_SetValue( "sys_cpuid", Sys_GetProcessorId() );

	Cvar_Set( "username", Sys_GetCurrentUser() );

	IN_Init();
}

/*
================
Sys_MainBody

The actual game loop, shared between the platform entry points
================
*/
static int Sys_MainBody( void ) {
	int startTime;
	int endTime;

	// set the initial time base
	Sys_Milliseconds();

#ifdef _WIN32
	Sys_CreateConsole();
#endif

	// initialize the stream thread stubs
	Sys_InitStreamThread();

	Com_Init( sys_cmdline );

	NET_Init();

	Com_Printf( "Working directory: %s\n", Sys_Cwd() );

#ifdef _WIN32
	// hide the console if we're not a dedicated server and not debugging
	if ( !com_dedicated->integer && !com_viewlog->integer ) {
		Sys_ShowConsole( 0, qfalse );
	}
#endif

	// main game loop
	while ( 1 ) {
		// allow the engine to idle when minimized or running dedicated
		if ( s_minimized || com_dedicated->integer ) {
			SDL_Delay( 5 );
		}

		startTime = Sys_Milliseconds();

		IN_Frame();

		// run the game
		Com_Frame();

		endTime = Sys_Milliseconds();
		totalMsec += endTime - startTime;
		countMsec++;
	}

	// never reached
}

#ifdef _WIN32

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
	// shut up compiler warnings about unused parameters
	(void) hInstance;
	(void) hPrevInstance;
	(void) nCmdShow;

	Q_strncpyz( sys_cmdline, lpCmdLine, sizeof( sys_cmdline ) );

	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_GAMEPAD ) ) {
		return Sys_MainBody();
	}

	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "WolfSP", SDL_GetError(), NULL );
	return 1;
}

#else

int main( int argc, char **argv ) {
	int i;

	sys_cmdline[0] = '\0';
	for ( i = 1; i < argc; i++ ) {
		Q_strcat( sys_cmdline, sizeof( sys_cmdline ), argv[i] );
		Q_strcat( sys_cmdline, sizeof( sys_cmdline ), " " );
	}

	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_GAMEPAD ) ) {
		return Sys_MainBody();
	}

	fprintf( stderr, "SDL_Init failed: %s\n", SDL_GetError() );
	return 1;
}

#endif
