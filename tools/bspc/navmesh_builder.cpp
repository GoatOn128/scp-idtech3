/*
===========================================================================

IceWolf GPL Source Code
Copyright (C) 2026 Justin Marshall (justinmarshall20@gmail.com and git.retrodamage.com)

This file is part of the IceWolf.

IceWolf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

IceWolf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with IceWolf.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#include "navmesh_builder.h"

#include "../nav/wolf_nav.h"
#include "../game/q_shared.h"
#include "q3files.h"

#include <DetourNavMeshBuilder.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <Recast.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace {
constexpr int kSurfaceSky = 0x4;
constexpr int kSurfaceNonSolid = 0x4000;
constexpr int kContentsLava = 8;
constexpr int kContentsSlime = 16;
constexpr int kContentsWater = 32;
constexpr int kContentsFog = 64;

struct AgentConfig {
	float mins[3]{ -18, -18, -24 };
	float maxs[3]{ 18, 18, 48 };
	float crouchMins[3]{ -18, -18, -24 };
	float crouchMaxs[3]{ 18, 18, 24 };
	float gravity = 800;
	float maxWalkVelocity = 220;
	float maxCrouchVelocity = 100;
	float maxStep = 18;
	float maxSteepness = 0.7f;
	float maxBarrier = 33;
	float jumpVelocity = 270;
	bool allowLadders = true;
};

class BuildContext final : public rcContext {
protected:
	void doLog( const rcLogCategory category, const char *message, int ) override {
		std::ostream &stream = category == RC_LOG_ERROR ? std::cerr : std::cout;
		stream << "recast: " << message << '\n';
	}
};

std::string ReadText( const std::filesystem::path &path ) {
	std::ifstream stream( path );
	if ( !stream ) return {};
	return { std::istreambuf_iterator<char>( stream ), std::istreambuf_iterator<char>() };
}

bool ParseVec( const std::string &block, const char *name, float out[3] ) {
	const std::regex pattern( std::string( "\\b" ) + name + R"(\s*\{\s*([-+0-9.eE]+)\s*,?\s*([-+0-9.eE]+)\s*,?\s*([-+0-9.eE]+)\s*\})" );
	std::smatch match;
	if ( !std::regex_search( block, match, pattern ) ) return false;
	for ( int i = 0; i < 3; ++i ) out[i] = std::stof( match[i + 1].str() );
	return true;
}

float ParseNumber( const std::string &text, const char *name, float fallback ) {
	const std::regex pattern( std::string( "\\b" ) + name + R"(\s+([-+0-9.eE]+))" );
	std::smatch match;
	return std::regex_search( text, match, pattern ) ? std::stof( match[1].str() ) : fallback;
}

bool ParseConfig( const std::filesystem::path &path, AgentConfig &cfg, std::string &error ) {
	std::string text = ReadText( path );
	if ( text.empty() ) {
		error = "could not read config " + path.string();
		return false;
	}
	text = std::regex_replace( text, std::regex( R"(//[^\r\n]*)" ), "" );
	// The legacy file has normal then crouch presence boxes. Parse them explicitly
	// so both clearances survive in the WNAV metadata.
	const std::regex bboxPattern( R"(bbox\s*\{[\s\S]*?\bmins\s*\{\s*([-+0-9.eE]+)\s*,?\s*([-+0-9.eE]+)\s*,?\s*([-+0-9.eE]+)\s*\}[\s\S]*?\bmaxs\s*\{\s*([-+0-9.eE]+)\s*,?\s*([-+0-9.eE]+)\s*,?\s*([-+0-9.eE]+)\s*\}[\s\S]*?\})" );
	std::vector<std::array<float, 6>> boxes;
	for ( std::sregex_iterator it( text.begin(), text.end(), bboxPattern ), end; it != end; ++it ) {
		std::array<float, 6> box{};
		for ( int i = 0; i < 6; ++i ) box[i] = std::stof( ( *it )[i + 1].str() );
		boxes.push_back( box );
	}
	if ( boxes.size() < 2 ) {
		error = "config must contain normal and crouch bbox blocks";
		return false;
	}
	for ( int i = 0; i < 3; ++i ) {
		cfg.mins[i] = boxes[0][i]; cfg.maxs[i] = boxes[0][i + 3];
		cfg.crouchMins[i] = boxes[1][i]; cfg.crouchMaxs[i] = boxes[1][i + 3];
	}
	cfg.gravity = ParseNumber( text, "phys_gravity", cfg.gravity );
	cfg.maxWalkVelocity = ParseNumber( text, "phys_maxwalkvelocity", cfg.maxWalkVelocity );
	cfg.maxCrouchVelocity = ParseNumber( text, "phys_maxcrouchvelocity", cfg.maxCrouchVelocity );
	cfg.maxStep = ParseNumber( text, "phys_maxstep", cfg.maxStep );
	cfg.maxSteepness = ParseNumber( text, "phys_maxsteepness", cfg.maxSteepness );
	cfg.maxBarrier = ParseNumber( text, "phys_maxbarrier", cfg.maxBarrier );
	cfg.jumpVelocity = ParseNumber( text, "phys_jumpvel", cfg.jumpVelocity );
	cfg.allowLadders = ParseNumber( text, "rs_allowladders", 1 ) != 0;
	return true;
}

template<typename T>
bool ReadLump( const std::vector<std::uint8_t> &file, const q3_lump_t &lump, std::vector<T> &out ) {
	if ( lump.fileofs < 0 || lump.filelen < 0 || lump.filelen % sizeof( T ) != 0 ) return false;
	const std::size_t offset = static_cast<std::size_t>( lump.fileofs );
	const std::size_t length = static_cast<std::size_t>( lump.filelen );
	if ( offset > file.size() || length > file.size() - offset ) return false;
	out.resize( length / sizeof( T ) );
	std::memcpy( out.data(), file.data() + offset, length );
	return true;
}

void AddVertex( std::vector<float> &verts, const float quake[3] ) {
	float nav[3];
	wolf::nav::QuakeToNav( quake, nav );
	verts.insert( verts.end(), nav, nav + 3 );
}

std::array<float, 3> Bezier( const q3_drawVert_t &a, const q3_drawVert_t &b, const q3_drawVert_t &c, float t ) {
	const float s = 1.0f - t;
	return { s * s * a.xyz[0] + 2 * s * t * b.xyz[0] + t * t * c.xyz[0],
		s * s * a.xyz[1] + 2 * s * t * b.xyz[1] + t * t * c.xyz[1],
		s * s * a.xyz[2] + 2 * s * t * b.xyz[2] + t * t * c.xyz[2] };
}

std::array<float, 3> PatchPoint( const q3_drawVert_t control[3][3], float u, float v ) {
	q3_drawVert_t rows[3]{};
	for ( int y = 0; y < 3; ++y ) {
		auto p = Bezier( control[y][0], control[y][1], control[y][2], u );
		std::copy( p.begin(), p.end(), rows[y].xyz );
	}
	return Bezier( rows[0], rows[1], rows[2], v );
}

bool ExtractGeometry( const std::filesystem::path &path, std::vector<float> &verts, std::vector<int> &tris, std::uint32_t &checksum, std::string &error ) {
	std::ifstream stream( path, std::ios::binary );
	if ( !stream ) { error = "could not read BSP " + path.string(); return false; }
	std::vector<std::uint8_t> file{ std::istreambuf_iterator<char>( stream ), std::istreambuf_iterator<char>() };
	if ( file.size() < sizeof( q3_dheader_t ) ) { error = "truncated BSP"; return false; }
	q3_dheader_t header{};
	std::memcpy( &header, file.data(), sizeof( header ) );
	if ( header.ident != Q3_BSP_IDENT || header.version != Q3_BSP_VERSION ) { error = "not an RTCW IBSP v47 file"; return false; }
	std::vector<q3_dshader_t> shaders;
	std::vector<q3_dplane_t> planes;
	std::vector<q3_dbrush_t> brushes;
	std::vector<q3_dbrushside_t> brushSides;
	std::vector<q3_drawVert_t> drawVerts;
	std::vector<int> indexes;
	std::vector<q3_dsurface_t> surfaces;
	if ( !ReadLump( file, header.lumps[Q3_LUMP_SHADERS], shaders ) || !ReadLump( file, header.lumps[Q3_LUMP_PLANES], planes ) ||
		 !ReadLump( file, header.lumps[Q3_LUMP_BRUSHES], brushes ) || !ReadLump( file, header.lumps[Q3_LUMP_BRUSHSIDES], brushSides ) ||
		 !ReadLump( file, header.lumps[Q3_LUMP_DRAWVERTS], drawVerts ) ||
		 !ReadLump( file, header.lumps[Q3_LUMP_DRAWINDEXES], indexes ) || !ReadLump( file, header.lumps[Q3_LUMP_SURFACES], surfaces ) ) {
		error = "invalid BSP geometry lumps"; return false;
	}
	checksum = 2166136261u;
	for ( std::uint8_t byte : file ) checksum = ( checksum ^ byte ) * 16777619u;

	for ( const auto &surface : surfaces ) {
		if ( surface.shaderNum < 0 || surface.shaderNum >= static_cast<int>( shaders.size() ) ) continue;
		const auto &shader = shaders[surface.shaderNum];
		if ( shader.surfaceFlags & ( kSurfaceSky | kSurfaceNonSolid ) ) continue;
		if ( shader.contentFlags & ( kContentsLava | kContentsSlime | kContentsWater | kContentsFog ) ) continue;
		if ( surface.firstVert < 0 || surface.numVerts < 0 || surface.firstVert + surface.numVerts > static_cast<int>( drawVerts.size() ) ) continue;

		if ( surface.surfaceType == MST_PLANAR || surface.surfaceType == MST_TRIANGLE_SOUP ) {
			const int base = static_cast<int>( verts.size() / 3 );
			for ( int i = 0; i < surface.numVerts; ++i ) AddVertex( verts, drawVerts[surface.firstVert + i].xyz );
			if ( surface.firstIndex < 0 || surface.numIndexes < 0 || surface.firstIndex + surface.numIndexes > static_cast<int>( indexes.size() ) ) continue;
			for ( int i = 0; i + 2 < surface.numIndexes; i += 3 ) {
				const int a = indexes[surface.firstIndex + i], b = indexes[surface.firstIndex + i + 1], c = indexes[surface.firstIndex + i + 2];
				if ( a >= 0 && b >= 0 && c >= 0 && a < surface.numVerts && b < surface.numVerts && c < surface.numVerts )
					tris.insert( tris.end(), { base + a, base + b, base + c } );
			}
		} else if ( surface.surfaceType == MST_PATCH && surface.patchWidth >= 3 && surface.patchHeight >= 3 ) {
			constexpr int subdivisions = 4;
			for ( int py = 0; py + 2 < surface.patchHeight; py += 2 ) for ( int px = 0; px + 2 < surface.patchWidth; px += 2 ) {
				q3_drawVert_t control[3][3]{};
				for ( int y = 0; y < 3; ++y ) for ( int x = 0; x < 3; ++x )
					control[y][x] = drawVerts[surface.firstVert + ( py + y ) * surface.patchWidth + px + x];
				const int base = static_cast<int>( verts.size() / 3 );
				for ( int y = 0; y <= subdivisions; ++y ) for ( int x = 0; x <= subdivisions; ++x ) {
					auto p = PatchPoint( control, x / float( subdivisions ), y / float( subdivisions ) );
					AddVertex( verts, p.data() );
				}
				for ( int y = 0; y < subdivisions; ++y ) for ( int x = 0; x < subdivisions; ++x ) {
					const int a = base + y * ( subdivisions + 1 ) + x;
					const int b = a + 1, c = a + subdivisions + 1, d = c + 1;
					tris.insert( tris.end(), { a, c, b, b, c, d } );
				}
			}
		}
	}

	// Add collision-only clip brushes. They do not necessarily have render
	// surfaces, but they must constrain the navmesh exactly like game movement.
	for ( const auto &brush : brushes ) {
		if ( brush.shaderNum < 0 || brush.shaderNum >= static_cast<int>( shaders.size() ) || brush.firstSide < 0 ||
			brush.numSides < 4 || brush.firstSide + brush.numSides > static_cast<int>( brushSides.size() ) ) continue;
		const int contents = shaders[brush.shaderNum].contentFlags;
		if ( !( contents & ( CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP ) ) ||
			( contents & ( kContentsLava | kContentsSlime | kContentsWater | kContentsFog ) ) ) continue;
		for ( int sideIndex = 0; sideIndex < brush.numSides; ++sideIndex ) {
			const q3_dbrushside_t &side = brushSides[brush.firstSide + sideIndex];
			if ( side.planeNum < 0 || side.planeNum >= static_cast<int>( planes.size() ) ) continue;
			const q3_dplane_t &plane = planes[side.planeNum];
			float up[3]{ std::abs( plane.normal[0] ) < 0.6f ? 1.0f : 0.0f, std::abs( plane.normal[0] ) < 0.6f ? 0.0f : 1.0f, 0.0f };
			const float projection = up[0] * plane.normal[0] + up[1] * plane.normal[1] + up[2] * plane.normal[2];
			for ( int j = 0; j < 3; ++j ) up[j] -= projection * plane.normal[j];
			const float upLength = std::sqrt( up[0] * up[0] + up[1] * up[1] + up[2] * up[2] );
			if ( upLength < 0.001f ) continue;
			for ( float &component : up ) component /= upLength;
			float right[3]{ plane.normal[1] * up[2] - plane.normal[2] * up[1],
				plane.normal[2] * up[0] - plane.normal[0] * up[2], plane.normal[0] * up[1] - plane.normal[1] * up[0] };
			constexpr float extent = 131072.0f;
			std::vector<std::array<float, 3>> winding( 4 );
			for ( int j = 0; j < 3; ++j ) {
				const float origin = plane.normal[j] * plane.dist;
				winding[0][j] = origin - right[j] * extent + up[j] * extent;
				winding[1][j] = origin + right[j] * extent + up[j] * extent;
				winding[2][j] = origin + right[j] * extent - up[j] * extent;
				winding[3][j] = origin - right[j] * extent - up[j] * extent;
			}
			for ( int clipIndex = 0; clipIndex < brush.numSides && winding.size() >= 3; ++clipIndex ) {
				if ( clipIndex == sideIndex ) continue;
				const q3_dbrushside_t &clipSide = brushSides[brush.firstSide + clipIndex];
				if ( clipSide.planeNum < 0 || clipSide.planeNum >= static_cast<int>( planes.size() ) ) { winding.clear(); break; }
				const q3_dplane_t &clip = planes[clipSide.planeNum];
				std::vector<std::array<float, 3>> clipped;
				for ( std::size_t j = 0; j < winding.size(); ++j ) {
					const auto &a = winding[j], &b = winding[( j + 1 ) % winding.size()];
					const float da = a[0] * clip.normal[0] + a[1] * clip.normal[1] + a[2] * clip.normal[2] - clip.dist;
					const float db = b[0] * clip.normal[0] + b[1] * clip.normal[1] + b[2] * clip.normal[2] - clip.dist;
					if ( da <= 0.1f ) clipped.push_back( a );
					if ( ( da < -0.1f && db > 0.1f ) || ( da > 0.1f && db < -0.1f ) ) {
						const float fraction = da / ( da - db );
						clipped.push_back( { a[0] + ( b[0] - a[0] ) * fraction, a[1] + ( b[1] - a[1] ) * fraction, a[2] + ( b[2] - a[2] ) * fraction } );
					}
				}
				winding.swap( clipped );
			}
			if ( winding.size() < 3 ) continue;
			const int base = static_cast<int>( verts.size() / 3 );
			for ( const auto &point : winding ) AddVertex( verts, point.data() );
			for ( int i = 1; i + 1 < static_cast<int>( winding.size() ); ++i ) tris.insert( tris.end(), { base, base + i, base + i + 1 } );
		}
	}
	if ( verts.empty() || tris.empty() ) { error = "BSP contained no usable collision surfaces"; return false; }
	return true;
}

template<typename T, void ( *FreeFn )( T * )>
struct RcOwner {
	T *value = nullptr;
	~RcOwner() { if ( value ) FreeFn( value ); }
	T &operator*() const { return *value; }
	T *operator->() const { return value; }
};
}

bool BuildNavMesh( const NavBuildOptions &options, std::string &error ) {
	AgentConfig agent;
	if ( !ParseConfig( options.configPath, agent, error ) ) return false;
	std::vector<float> verts;
	std::vector<int> tris;
	std::uint32_t checksum = 0;
	if ( !ExtractGeometry( options.bspPath, verts, tris, checksum, error ) ) return false;

	BuildContext context;
	rcConfig cfg{};
	cfg.cs = ( std::clamp )( ( std::max )( std::abs( agent.mins[0] ), std::abs( agent.mins[1] ) ) / 2.0f, 4.0f, 8.0f );
	cfg.ch = 2.0f;
	cfg.walkableSlopeAngle = std::acos( ( std::clamp )( agent.maxSteepness, 0.0f, 1.0f ) ) * 180.0f / 3.14159265358979323846f;
	const int normalHeight = static_cast<int>( std::ceil( ( agent.maxs[2] - agent.mins[2] ) / cfg.ch ) );
	const int crouchHeight = static_cast<int>( std::ceil( ( agent.crouchMaxs[2] - agent.crouchMins[2] ) / cfg.ch ) );
	cfg.walkableHeight = crouchHeight;
	cfg.walkableClimb = static_cast<int>( std::floor( agent.maxStep / cfg.ch ) );
	cfg.walkableRadius = static_cast<int>( std::ceil( ( std::max )( std::abs( agent.mins[0] ), std::abs( agent.mins[1] ) ) / cfg.cs ) );
	cfg.maxEdgeLen = static_cast<int>( 12.0f * cfg.walkableRadius );
	cfg.maxSimplificationError = 1.3f;
	cfg.minRegionArea = rcSqr( 8 );
	cfg.mergeRegionArea = rcSqr( 20 );
	cfg.maxVertsPerPoly = DT_VERTS_PER_POLYGON;
	cfg.detailSampleDist = cfg.cs * 6.0f;
	cfg.detailSampleMaxError = cfg.ch;
	rcCalcBounds( verts.data(), static_cast<int>( verts.size() / 3 ), cfg.bmin, cfg.bmax );
	rcCalcGridSize( cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height );
	std::cout << "input: " << verts.size() / 3 << " vertices, " << tris.size() / 3 << " triangles\n";
	std::cout << "agent: radius=" << cfg.walkableRadius * cfg.cs << " height=" << cfg.walkableHeight * cfg.ch
		<< " crouch=" << agent.crouchMaxs[2] - agent.crouchMins[2] << " climb=" << cfg.walkableClimb * cfg.ch
		<< " slope=" << cfg.walkableSlopeAngle << " ladders=" << ( agent.allowLadders ? "yes" : "no" ) << '\n';

	RcOwner<rcHeightfield, rcFreeHeightField> solid{ rcAllocHeightfield() };
	if ( !solid.value || !rcCreateHeightfield( &context, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch ) ) { error = "could not create heightfield"; return false; }
	std::vector<unsigned char> areas( tris.size() / 3 );
	rcMarkWalkableTriangles( &context, cfg.walkableSlopeAngle, verts.data(), static_cast<int>( verts.size() / 3 ), tris.data(), static_cast<int>( tris.size() / 3 ), areas.data() );
	if ( !rcRasterizeTriangles( &context, verts.data(), static_cast<int>( verts.size() / 3 ), tris.data(), areas.data(), static_cast<int>( tris.size() / 3 ), *solid, cfg.walkableClimb ) ) { error = "could not rasterize BSP"; return false; }
	rcFilterLowHangingWalkableObstacles( &context, cfg.walkableClimb, *solid );
	rcFilterLedgeSpans( &context, cfg.walkableHeight, cfg.walkableClimb, *solid );
	rcFilterWalkableLowHeightSpans( &context, cfg.walkableHeight, *solid );
	RcOwner<rcCompactHeightfield, rcFreeCompactHeightfield> compact{ rcAllocCompactHeightfield() };
	if ( !compact.value || !rcBuildCompactHeightfield( &context, cfg.walkableHeight, cfg.walkableClimb, *solid, *compact ) ) { error = "could not build compact heightfield"; return false; }
	for ( int i = 0; i < compact->spanCount; ++i ) if ( compact->areas[i] != RC_NULL_AREA && compact->spans[i].h < normalHeight ) compact->areas[i] = 1;
	if ( !rcErodeWalkableArea( &context, cfg.walkableRadius, *compact ) || !rcBuildDistanceField( &context, *compact ) ||
		 !rcBuildRegions( &context, *compact, 0, cfg.minRegionArea, cfg.mergeRegionArea ) ) { error = "could not build walkable regions"; return false; }
	RcOwner<rcContourSet, rcFreeContourSet> contours{ rcAllocContourSet() };
	if ( !contours.value || !rcBuildContours( &context, *compact, cfg.maxSimplificationError, cfg.maxEdgeLen, *contours ) ) { error = "could not build contours"; return false; }
	RcOwner<rcPolyMesh, rcFreePolyMesh> mesh{ rcAllocPolyMesh() };
	if ( !mesh.value || !rcBuildPolyMesh( &context, *contours, cfg.maxVertsPerPoly, *mesh ) ) { error = "could not build polygon mesh"; return false; }
	RcOwner<rcPolyMeshDetail, rcFreePolyMeshDetail> detail{ rcAllocPolyMeshDetail() };
	if ( !detail.value || !rcBuildPolyMeshDetail( &context, *mesh, *compact, cfg.detailSampleDist, cfg.detailSampleMaxError, *detail ) ) { error = "could not build detail mesh"; return false; }
	for ( int i = 0; i < mesh->npolys; ++i ) {
		if ( mesh->areas[i] == RC_WALKABLE_AREA ) mesh->areas[i] = 0;
		mesh->flags[i] = wolf::nav::PolyWalk;
	}
	dtNavMeshCreateParams params{};
	params.verts = mesh->verts; params.vertCount = mesh->nverts; params.polys = mesh->polys;
	params.polyAreas = mesh->areas; params.polyFlags = mesh->flags; params.polyCount = mesh->npolys; params.nvp = mesh->nvp;
	params.detailMeshes = detail->meshes; params.detailVerts = detail->verts; params.detailVertsCount = detail->nverts;
	params.detailTris = detail->tris; params.detailTriCount = detail->ntris;
	params.walkableHeight = agent.maxs[2] - agent.mins[2];
	params.walkableRadius = ( std::max )( std::abs( agent.mins[0] ), std::abs( agent.mins[1] ) );
	params.walkableClimb = agent.maxStep;
	rcVcopy( params.bmin, mesh->bmin ); rcVcopy( params.bmax, mesh->bmax );
	params.cs = cfg.cs; params.ch = cfg.ch; params.buildBvTree = true;
	unsigned char *navData = nullptr;
	int navDataSize = 0;
	if ( !dtCreateNavMeshData( &params, &navData, &navDataSize ) ) { error = "Detour rejected the polygon mesh"; return false; }
	dtNavMesh *validationMesh = dtAllocNavMesh();
	dtNavMeshQuery *validationQuery = dtAllocNavMeshQuery();
	const bool validMesh = validationMesh && dtStatusSucceed( validationMesh->init( navData, navDataSize, 0 ) );
	const bool validQuery = validMesh && validationQuery && dtStatusSucceed( validationQuery->init( validationMesh, 4096 ) );
	if ( validationQuery ) dtFreeNavMeshQuery( validationQuery );
	if ( validationMesh ) dtFreeNavMesh( validationMesh );
	if ( !validQuery ) { dtFree( navData ); error = "generated Detour navmesh failed validation"; return false; }

	wolf::nav::FileHeader header{};
	header.magic = wolf::nav::kMagic; header.version = wolf::nav::kVersion; header.headerSize = sizeof( header );
	header.navDataSize = static_cast<std::uint32_t>( navDataSize ); header.sourceChecksum = checksum; header.agentIndex = options.agentIndex;
	header.agentRadius = params.walkableRadius; header.agentHeight = params.walkableHeight;
	header.crouchHeight = agent.crouchMaxs[2] - agent.crouchMins[2]; header.maxClimb = agent.maxStep;
	header.maxSlope = cfg.walkableSlopeAngle; header.gravity = agent.gravity; header.jumpVelocity = agent.jumpVelocity;
	header.maxWalkVelocity = agent.maxWalkVelocity; header.maxCrouchVelocity = agent.maxCrouchVelocity;
	header.maxBarrier = agent.maxBarrier; header.allowLadders = agent.allowLadders ? 1u : 0u;
	std::filesystem::create_directories( options.outputPath.parent_path() );
	std::ofstream output( options.outputPath, std::ios::binary );
	if ( !output ) { dtFree( navData ); error = "could not create " + options.outputPath.string(); return false; }
	output.write( reinterpret_cast<const char *>( &header ), sizeof( header ) );
	output.write( reinterpret_cast<const char *>( navData ), navDataSize );
	dtFree( navData );
	if ( !output ) { error = "failed writing " + options.outputPath.string(); return false; }
	std::cout << "navmesh: " << mesh->nverts << " vertices, " << mesh->npolys << " polygons, " << navDataSize << " bytes\n";
	return true;
}
