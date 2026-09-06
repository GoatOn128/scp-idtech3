#pragma once

#include <cstdint>

namespace wolf::nav {

constexpr std::uint32_t kMagic = 0x56414E57u; // "WNAV"
constexpr std::uint32_t kVersion = 1;

enum PolyFlags : std::uint16_t {
	PolyWalk = 1u << 0,
	PolySwim = 1u << 1,
	PolyDoor = 1u << 2,
	PolyJump = 1u << 3,
	PolyDisabled = 1u << 15,
};

#pragma pack(push, 1)
struct FileHeader {
	std::uint32_t magic;
	std::uint32_t version;
	std::uint32_t headerSize;
	std::uint32_t navDataSize;
	std::uint32_t sourceChecksum;
	std::uint32_t agentIndex;
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
	std::uint32_t allowLadders;
	std::uint32_t reserved[7];
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
