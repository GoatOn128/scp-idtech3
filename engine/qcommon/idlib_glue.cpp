/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (“RTCW SP Source Code”).  

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

// Binds the engine's console and error functions into the idlib interface
// classes declared in idlib/idlib_adapters.h.

#include "q_shared.h"
#include "qcommon.h"
#include "idlib/idlib_adapters.h"

// declared in idlib/sys/sys_assert.h
bool AssertFailed( const char *file, int line, const char *expression );

// idlib statics, unbound until Idlib_Init runs
idCommon * idLib::common = NULL;
idCVarSystem * idLib::cvarSystem = NULL;
idFileSystem * idLib::fileSystem = NULL;
int idLib::frameNumber = 0;

#define IDLIB_PRINT_MSG 4096

class idCommonEngine : public idCommon {
public:
	virtual void Printf( const char *fmt, ... );
	virtual void Warning( const char *fmt, ... );
	virtual void Error( const char *fmt, ... );
	virtual void FatalError( const char *fmt, ... );
};

static idCommonEngine idlibCommonEngine;

static void Idlib_VPrintf( const char *prefix, const char *fmt, va_list args ) {
	char buffer[IDLIB_PRINT_MSG];
	vsnprintf( buffer, sizeof( buffer ), fmt, args );
	Com_Printf( "%s%s", prefix, buffer );
}

void idCommonEngine::Printf( const char *fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	Idlib_VPrintf( "", fmt, args );
	va_end( args );
}

void idCommonEngine::Warning( const char *fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	Idlib_VPrintf( S_COLOR_YELLOW "WARNING: ", fmt, args );
	va_end( args );
}

void idCommonEngine::Error( const char *fmt, ... ) {
	char buffer[IDLIB_PRINT_MSG];
	va_list args;
	va_start( args, fmt );
	vsnprintf( buffer, sizeof( buffer ), fmt, args );
	va_end( args );
	Com_Error( ERR_DROP, "%s", buffer );
}

void idCommonEngine::FatalError( const char *fmt, ... ) {
	char buffer[IDLIB_PRINT_MSG];
	va_list args;
	va_start( args, fmt );
	vsnprintf( buffer, sizeof( buffer ), fmt, args );
	va_end( args );
	Com_Error( ERR_FATAL, "%s", buffer );
}

void idLib::Printf( const char *fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	Idlib_VPrintf( "", fmt, args );
	va_end( args );
}

void idLib::Warning( const char *fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	Idlib_VPrintf( S_COLOR_YELLOW "WARNING: ", fmt, args );
	va_end( args );
}

void idLib::Error( const char *fmt, ... ) {
	char buffer[IDLIB_PRINT_MSG];
	va_list args;
	va_start( args, fmt );
	vsnprintf( buffer, sizeof( buffer ), fmt, args );
	va_end( args );
	Com_Error( ERR_DROP, "%s", buffer );
}

void idLib::FatalError( const char *fmt, ... ) {
	char buffer[IDLIB_PRINT_MSG];
	va_list args;
	va_start( args, fmt );
	vsnprintf( buffer, sizeof( buffer ), fmt, args );
	va_end( args );
	Com_Error( ERR_FATAL, "%s", buffer );
}

/*
================
Idlib_Init

Binds the engine console into the idlib interfaces so idlib code can
print through the engine console.
================
*/
void Idlib_Init( void ) {
	idLib::common = &idlibCommonEngine;
}

/*
================
AssertFailed

Prints a failed idlib assertion through the engine console and returns
false so execution falls through to the platform assert.
================
*/
bool AssertFailed( const char *file, int line, const char *expression ) {
	Com_Printf( S_COLOR_RED "Assertion failed: %s (%s:%d)\n", expression, file, line );
	return false;
}
