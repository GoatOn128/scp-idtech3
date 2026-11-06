/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

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

If you have questions concerning the license or the applicable additional terms, you should contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __SYS_PUBLIC_H__
#define __SYS_PUBLIC_H__

typedef enum {
	ID_CPUID_NONE						= 0x00000,
	ID_CPUID_UNSUPPORTED				= 0x00001,
	ID_CPUID_GENERIC					= 0x00002,
	ID_CPUID_INTEL						= 0x00004,
	ID_CPUID_AMD						= 0x00008,
	ID_CPUID_MMX						= 0x00010,
	ID_CPUID_3DNOW						= 0x00020,
	ID_CPUID_SSE						= 0x00040,
	ID_CPUID_SSE2						= 0x00080,
	ID_CPUID_SSE3						= 0x00100,
	ID_CPUID_ALTIVEC					= 0x00200,
	ID_CPUID_HTT						= 0x01000,
	ID_CPUID_CMOV						= 0x02000,
	ID_CPUID_FTZ						= 0x04000,
	ID_CPUID_DAZ						= 0x08000
} cpuid_t;

// minimal system service interfaces that idlib calls into. The engine
// provides the implementations and hooks them up through idLib::Init()

class idFile {
public:
	virtual					~idFile() {}
	virtual const char *	GetName() const = 0;
	virtual const char *	GetFullPath() const = 0;
	virtual int				Read( void *buffer, int len ) = 0;
	virtual int				Write( const void *buffer, int len ) = 0;
	virtual int				WriteFloatString( const char *fmt, ... ) = 0;
	virtual int				Length() const = 0;
	virtual int				Tell() const = 0;
	virtual int				Seek( long offset, int origin ) = 0;
	virtual ID_TIME_T		Timestamp() const = 0;
};

typedef idFile * idFilePtr;

#define FILE_NOT_FOUND_TIMESTAMP	ID_TIME_T( -1 )

class idFileSystem {
public:
	virtual					~idFileSystem() {}
	virtual idFilePtr		OpenFileRead( const char *relativePath ) = 0;
	virtual idFilePtr		OpenExplicitFileRead( const char *pathname ) = 0;
	virtual idFilePtr		OpenFileWrite( const char *relativePath, const char *basePath = NULL ) = 0;
	virtual idFilePtr		OpenExplicitFileWrite( const char *pathname ) = 0;
	virtual void			CloseFile( idFilePtr file ) = 0;
	virtual int				ReadFile( const char *relativePath, void **buffer, ID_TIME_T *timestamp = NULL ) = 0;
	virtual void			FreeFile( void *buffer ) = 0;
};

class idCVarSystem {
public:
	virtual					~idCVarSystem() {}
	virtual const char *	GetCVarString( const char *name ) const = 0;
};

class idCommon {
public:
	virtual					~idCommon() {}
	virtual void			Printf( const char *fmt, ... ) = 0;
	virtual void			VPrintf( const char *fmt, va_list args ) = 0;
	virtual void			DWarning( const char *fmt, ... ) = 0;
	virtual void			Warning( const char *fmt, ... ) = 0;
	virtual void			Error( const char *fmt, ... ) = 0;
	virtual void			FatalError( const char *fmt, ... ) = 0;
	virtual void			SetRefreshOnPrint( bool enable ) = 0;
};

class idSys {
public:
	virtual					~idSys() {}
	virtual int				Milliseconds() = 0;
	virtual double			GetClockTicks() const = 0;
	virtual double			ClockTicksPerSecond() const = 0;
	virtual cpuid_t			GetProcessorId() = 0;
	virtual void			FPU_SetFTZ( bool enable ) = 0;
	virtual void			FPU_SetDAZ( bool enable ) = 0;
	virtual void			DebugPrintf( const char *fmt, ... ) = 0;
};

#endif /* !__SYS_PUBLIC_H__ */
