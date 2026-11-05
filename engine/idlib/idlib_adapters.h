/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 BFG Edition GPL Source Code (“Doom 3 BFG Edition Source Code”).  

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __IDLIB_ADAPTERS_H__
#define __IDLIB_ADAPTERS_H__

/*
===============================================================================

	Interface classes and declarations the idlib code touches. In Doom 3 these
	live in the framework; here they are trimmed to the surface the built idlib
	sources actually use. The engine binds them in engine/qcommon/idlib_glue.cpp.

===============================================================================
*/

class idCmdArgs;

#ifndef ID_TIME_T
#define ID_TIME_T long long
#endif

class idFile {
public:
	virtual					~idFile() {}
	virtual const char *	GetName() const = 0;
	virtual const char *	GetFullPath() const = 0;
	virtual int				Read( void *buffer, int len ) = 0;
	virtual int				Length() const = 0;
	virtual ID_TIME_T		Timestamp() const = 0;
};

class idFileSystem {
public:
	virtual					~idFileSystem() {}
	virtual idFile *		OpenFileRead( const char *relativePath ) = 0;
	virtual idFile *		OpenExplicitFileRead( const char *relativePath ) = 0;
	virtual void			CloseFile( idFile *file ) = 0;
};

class idCommon {
public:
	virtual					~idCommon() {}
	virtual void			Printf( const char *fmt, ... ) = 0;
	virtual void			Warning( const char *fmt, ... ) = 0;
	virtual void			Error( const char *fmt, ... ) = 0;
	virtual void			FatalError( const char *fmt, ... ) = 0;
};

class idCVarSystem {
public:
	virtual					~idCVarSystem() {}
	virtual const char *	GetCVarString( const char *name ) = 0;
};

class idLib {
public:
	static class idCommon *	common;
	static class idCVarSystem * cvarSystem;
	static class idFileSystem * fileSystem;
	static int				frameNumber;

	static void				Printf( const char *fmt, ... );
	static void				Warning( const char *fmt, ... );
	static void				Error( const char *fmt, ... );
	static void				FatalError( const char *fmt, ... );
	static bool				IsMainThread() { return true; }
};

// the BFG framework turns these into registered console commands; without it
// they compile down to plain unreferenced functions
#define CONSOLE_COMMAND( name, description, id ) void name( const idCmdArgs &args )

// localization id; the engine does not use the BFG localization system
class idStrId {
public:
							idStrId( const char * = NULL ) { }
	const char *		GetLocalizedString() const { return ""; }
};

#endif /* !__IDLIB_ADAPTERS_H__ */
