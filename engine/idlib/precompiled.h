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

#ifndef __PRECOMPILED_H__
#define __PRECOMPILED_H__

// Trimmed standalone precompiled header for the idlib subset this engine
// builds (math, containers, strings, lexer, parser). The original BFG
// version pulled in the whole Doom 3 framework, which does not exist here.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

// MSVC pulls in its deprecated CodeAnalysis annotations header along the
// standard library chain idlib uses; silence the deprecation warning
#if defined( _MSC_VER )
#pragma warning( disable : 4467 )
#endif

#include "sys/sys_assert.h"
#include "sys/sys_defines.h"
#include "sys/sys_types.h"

#define ID_TIME_T int64

#include "idlib_adapters.h"

#include "Heap.h"
#include "containers/Sort.h"
#include "containers/List.h"
#include "Str.h"
#include "CmdArgs.h"
#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "math/Math.h"
#include "math/Random.h"
#include "math/Vector.h"
#include "math/Matrix.h"
#include "math/Angles.h"
#include "math/Quat.h"
#include "math/Rotation.h"
#include "math/Plane.h"
#include "containers/HashIndex.h"
#include "bv/Sphere.h"
#include "bv/Bounds.h"

#endif /* !__PRECOMPILED_H__ */
