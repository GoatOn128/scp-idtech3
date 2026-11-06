#ifndef __TOOLS_PRECOMPILED_H__
#define __TOOLS_PRECOMPILED_H__

#include "../idlib/sys/sys_defines.h"
#include "../idlib/sys/sys_builddefines.h"
#include "../idlib/sys/sys_includes.h"
#include "../idlib/sys/sys_assert.h"
#include "../idlib/sys/sys_types.h"
#include "../idlib/sys/sys_intrinsics.h"
#include "../idlib/sys/sys_threading.h"
#define ID_TIME_T int64
#include "../idlib/sys/sys_public.h"
#include "../idlib/Lib.h"

// Keep the tool PCH independent from engine and gamedll implementation
// headers. These facilities are shared by the C++ tool front ends.
#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <vector>

#endif
