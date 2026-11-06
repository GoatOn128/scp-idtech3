#pragma once
#include <cmath>
#include <cassert>
#include <cstdarg>
#include "cg_local.h"

// Private FX renderer payload. Never passed across the renderer syscall ABI.
static const refEntityType_t RT_LINE = static_cast<refEntityType_t>(100);
static const refEntityType_t RT_ELECTRICITY = static_cast<refEntityType_t>(101);
static const refEntityType_t RT_ORIENTED_QUAD = static_cast<refEntityType_t>(102);
static const refEntityType_t RT_CYLINDER = static_cast<refEntityType_t>(103);
static const int RF_FORKED = 1 << 24;
static const int RF_TAPERED = 1 << 25;
static const int RF_GROW = 1 << 26;
struct FxRefEntity : refEntity_t {
    idVec3 angles;
    int endTime;
    float aspectRatio;
};
#define WORLD_SIZE 65536
#define CHAN_LESS_ATTEN CHAN_AUTO
