#pragma once

#include "../../idlib/sys/sys_types.h"

namespace wolf::nav {

constexpr uint32 kMagic = 0x56414E57u; // "WNAV"
constexpr uint32 kVersion = 1;

enum PolyFlags : uint16 {
	PolyWalk = 1u << 0,
	PolySwim = 1u << 1,
	PolyDoor = 1u << 2,
	PolyJump = 1u << 3,
	PolyDisabled = 1u << 15,
};

#pragma pack(push, 1)
struct FileHeader {
	uint32 magic;
	uint32 version;
	uint32 headerSize;
	uint32 navDataSize;
	uint32 sourceChecksum;
	uint32 agentIndex;
	float agentRadius;
	float agentHeight;
	float crouchHeight;
	float maxClimb;
	float maxSlope;
	float gravity;
	float jumpVelocity;
	float maxWalkVelocity;
	float maxCrouchVelocity;
	float maxBarrier;
	uint32 allowLadders;
	uint32 reserved[7];
};
#pragma pack(pop)

inline void QuakeToNav( const float in[3], float out[3] ) {
	out[0] = in[0];
	out[1] = in[2];
	out[2] = -in[1];
}

inline void NavToQuake( const float in[3], float out[3] ) {
	out[0] = in[0];
	out[1] = -in[2];
	out[2] = in[1];
}

} // namespace wolf::nav
