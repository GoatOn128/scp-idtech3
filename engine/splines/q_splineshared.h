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

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing to id Software LLC, at the address below.

If you have questions concerning this license or the applicable terms, please contact in writing to id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, MD 20850 USA.

===========================================================================
*/

#ifndef __Q_SHARED_H
#define __Q_SHARED_H

// q_splineshared.h -- included first by ALL program modules.
// these are the definitions that have no dependance on
// central system services, and can be used by any part
// of the program without any state issues.

// the math, string and container types come from the vendored idlib
#include "idlib/precompiled.h"

#if defined( _MSC_VER )
#pragma warning(disable : 4018) // signed/unsigned mismatch
#pragma warning(disable : 4244) // conversion double to float, the old camera code relies on it
#pragma warning(disable : 4305) // truncation from double to float
#endif

typedef enum {qfalse, qtrue}	qboolean;

typedef unsigned char byte;

typedef int fileHandle_t;

#define nanmask ( 255 << 23 )

#define IS_NAN( x ) ( ( ( *(int *)&x ) & nanmask ) == nanmask )

// TTimo
// handy stuff when tracking isnan problems
#ifndef NDEBUG
#define CHECK_NAN( x ) assert( !IS_NAN( x ) )
#define CHECK_NAN_VEC( v ) assert( !IS_NAN( v[0] ) && !IS_NAN( v[1] ) && !IS_NAN( v[2] ) )
#else
#define CHECK_NAN
#define CHECK_NAN_VEC
#endif

// provided by the engine filesystem and command buffer, bound at link time
void QDECL Com_Printf( const char *msg, ... );

#endif  // __Q_SHARED_H
