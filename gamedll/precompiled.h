#ifndef __GAMEDLL_PRECOMPILED_H__
#define __GAMEDLL_PRECOMPILED_H__

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
#include "../engine/qcommon/q_shared.h"
#include "game/bg_public.h"

// Shared module-side facilities. Do not pull engine-only qcommon or
// renderer headers into the game DLL PCH.
#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <vector>

#endif
