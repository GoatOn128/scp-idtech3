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

#pragma hdrstop

#include "precompiled.h"

#include <DetourNavMeshBuilder.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <Recast.h>

#include <float.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>

#include "navmesh_builder.h"

#include "../../engine/qcommon/wolf_nav.h"
#include "../../engine/qcommon/qfiles.h"

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
		fprintf( category == RC_LOG_ERROR ? stderr : stdout, "recast: %s\n", message );
	}
};

bool ReadFile( const char *path, idList<byte> &bytes ) {
	FILE *file = fopen( path, "rb" );
	if ( !file )
		return false;
	if ( fseek( file, 0, SEEK_END ) != 0 ) {
		fclose( file );
		return false;
	}
	const long length = ftell( file );
	if ( length < 0 || length > INT_MAX || fseek( file, 0, SEEK_SET ) != 0 ) {
		fclose( file );
		return false;
	}
	bytes.Resize( static_cast<int>( length ) );
	const bool read = fread( bytes.Ptr(), 1, static_cast<size_t>( length ), file ) == static_cast<size_t>( length );
	fclose( file );
	return read;
}

bool ReadVector( idLexer &parser, float out[3] ) {
	idToken token;
	if ( !parser.ExpectTokenString( "{" ) )
		return false;
	for ( int i = 0; i < 3; ) {
		if ( !parser.ReadToken( &token ) )
			return false;
		if ( token == "," )
			continue;
		out[i++] = static_cast<float>( atof( token.c_str() ) );
	}
	return parser.ExpectTokenString( "}" );
}

float ParseNumber( idLexer &parser, float fallback ) {
	idToken token;
	return parser.ReadToken( &token ) ? static_cast<float>( atof( token.c_str() ) ) : fallback;
}

bool ParseConfig( const char *path, AgentConfig &cfg, idStr &error ) {
	idList<byte> text;
	if ( !ReadFile( path, text ) || text.Num() == 0 ) {
		error = idStr( "could not read config " ) + path;
		return false;
	}
	idLexer parser;
	parser.SetFlags( LEXFL_NOERRORS | LEXFL_NOWARNINGS | LEXFL_NOSTRINGESCAPECHARS );
	if ( !parser.LoadMemory( reinterpret_cast<const char *>( text.Ptr() ), text.Num(), path ) ) {
		error = idStr( "could not parse config " ) + path;
		return false;
	}
	float boxes[2][6]{};
	int boxCount = 0;
	idToken token;
	while ( parser.ReadToken( &token ) ) {
		if ( token == "bbox" ) {
			if ( !parser.ExpectTokenString( "{" ) )
				return false;
			float box[6]{};
			while ( parser.ReadToken( &token ) && token != "}" ) {
				if ( token == "mins" ) {
					if ( !ReadVector( parser, box ) )
						return false;
				} else if ( token == "maxs" ) {
					if ( !ReadVector( parser, box + 3 ) )
						return false;
				} else {
					idToken value;
					if ( parser.ReadToken( &value ) && value == "{" )
						parser.SkipBracedSection( false );
				}
			}
			if ( boxCount < 2 )
				memcpy( boxes[boxCount++], box, sizeof( box ) );
		} else if ( token == "phys_gravity" ) {
			cfg.gravity = ParseNumber( parser, cfg.gravity );
		} else if ( token == "phys_maxwalkvelocity" ) {
			cfg.maxWalkVelocity = ParseNumber( parser, cfg.maxWalkVelocity );
		} else if ( token == "phys_maxcrouchvelocity" ) {
			cfg.maxCrouchVelocity = ParseNumber( parser, cfg.maxCrouchVelocity );
		} else if ( token == "phys_maxstep" ) {
			cfg.maxStep = ParseNumber( parser, cfg.maxStep );
		} else if ( token == "phys_maxsteepness" ) {
			cfg.maxSteepness = ParseNumber( parser, cfg.maxSteepness );
		} else if ( token == "phys_maxbarrier" ) {
			cfg.maxBarrier = ParseNumber( parser, cfg.maxBarrier );
		} else if ( token == "phys_jumpvel" ) {
			cfg.jumpVelocity = ParseNumber( parser, cfg.jumpVelocity );
		} else if ( token == "rs_allowladders" ) {
			cfg.allowLadders = ParseNumber( parser, 1 ) != 0;
		}
	}
	if ( boxCount < 2 ) {
		error = "config must contain normal and crouch bbox blocks";
		return false;
	}
	for ( int i = 0; i < 3; ++i ) {
		cfg.mins[i] = boxes[0][i]; cfg.maxs[i] = boxes[0][i + 3];
		cfg.crouchMins[i] = boxes[1][i]; cfg.crouchMaxs[i] = boxes[1][i + 3];
	}
	return true;
}

template<typename T>
bool ReadLump( const idList<byte> &file, const lump_t &lump, idList<T> &out ) {
	if ( lump.fileofs < 0 || lump.filelen < 0 || lump.filelen % sizeof( T ) != 0 ) return false;
	const size_t offset = static_cast<size_t>( lump.fileofs );
	const size_t length = static_cast<size_t>( lump.filelen );
	if ( offset > static_cast<size_t>( file.Num() ) || length > static_cast<size_t>( file.Num() ) - offset ) return false;
	out.Resize( static_cast<int>( length / sizeof( T ) ) );
	memcpy( out.Ptr(), file.Ptr() + offset, length );
	return true;
}

void AddVertex( idList<float> &verts, const idVec3 &quake ) {
	float nav[3];
	wolf::nav::QuakeToNav( quake.ToFloatPtr(), nav );
	for ( int i = 0; i < 3; ++i )
		verts.Append( nav[i] );
}

idVec3 Bezier( const idVec3 &a, const idVec3 &b, const idVec3 &c, float t ) {
	const float s = 1.0f - t;
	return idVec3( s * s * a[0] + 2 * s * t * b[0] + t * t * c[0],
				   s * s * a[1] + 2 * s * t * b[1] + t * t * c[1],
				   s * s * a[2] + 2 * s * t * b[2] + t * t * c[2] );
}

idVec3 PatchPoint( const idVec3 control[3][3], float u, float v ) {
	idVec3 rows[3];
	for ( int y = 0; y < 3; ++y ) {
		rows[y] = Bezier( control[y][0], control[y][1], control[y][2], u );
	}
	return Bezier( rows[0], rows[1], rows[2], v );
}

void AddTriangle( idList<int> &tris, int a, int b, int c ) {
	tris.Append( a );
	tris.Append( b );
	tris.Append( c );
}

bool ExtractGeometry( const char *path, idList<float> &verts, idList<int> &tris, uint32 &checksum, idStr &error ) {
	idList<byte> file;
	if ( !ReadFile( path, file ) ) { error = idStr( "could not read BSP " ) + path; return false; }
	if ( file.Num() < static_cast<int>( sizeof( dheader_t ) ) ) { error = "truncated BSP"; return false; }
	dheader_t header{};
	memcpy( &header, file.Ptr(), sizeof( header ) );
	if ( header.ident != BSP_IDENT || header.version != BSP_VERSION ) { error = "not an RTCW IBSP v47 file"; return false; }
	idList<dshader_t> shaders;
	idList<dplane_t> planes;
	idList<dbrush_t> brushes;
	idList<dbrushside_t> brushSides;
	idList<drawVert_t> drawVerts;
	idList<int> indexes;
	idList<dsurface_t> surfaces;
	if ( !ReadLump( file, header.lumps[LUMP_SHADERS], shaders ) || !ReadLump( file, header.lumps[LUMP_PLANES], planes ) ||
		 !ReadLump( file, header.lumps[LUMP_BRUSHES], brushes ) || !ReadLump( file, header.lumps[LUMP_BRUSHSIDES], brushSides ) ||
		 !ReadLump( file, header.lumps[LUMP_DRAWVERTS], drawVerts ) ||
		 !ReadLump( file, header.lumps[LUMP_DRAWINDEXES], indexes ) || !ReadLump( file, header.lumps[LUMP_SURFACES], surfaces ) ) {
		error = "invalid BSP geometry lumps"; return false;
	}
	checksum = 2166136261u;
	for ( int i = 0; i < file.Num(); ++i ) checksum = ( checksum ^ file[i] ) * 16777619u;

	for ( int surfaceIndex = 0; surfaceIndex < surfaces.Num(); ++surfaceIndex ) {
		const dsurface_t &surface = surfaces[surfaceIndex];
		if ( surface.shaderNum < 0 || surface.shaderNum >= shaders.Num() ) continue;
		const auto &shader = shaders[surface.shaderNum];
		if ( shader.surfaceFlags & ( kSurfaceSky | kSurfaceNonSolid ) ) continue;
		if ( shader.contentFlags & ( kContentsLava | kContentsSlime | kContentsWater | kContentsFog ) ) continue;
		if ( surface.firstVert < 0 || surface.numVerts < 0 || surface.firstVert + surface.numVerts > drawVerts.Num() ) continue;

		if ( surface.surfaceType == MST_PLANAR || surface.surfaceType == MST_TRIANGLE_SOUP ) {
			const int base = verts.Num() / 3;
			for ( int i = 0; i < surface.numVerts; ++i ) AddVertex( verts, drawVerts[surface.firstVert + i].xyz );
			if ( surface.firstIndex < 0 || surface.numIndexes < 0 || surface.firstIndex + surface.numIndexes > indexes.Num() ) continue;
			for ( int i = 0; i + 2 < surface.numIndexes; i += 3 ) {
				const int a = indexes[surface.firstIndex + i], b = indexes[surface.firstIndex + i + 1], c = indexes[surface.firstIndex + i + 2];
				if ( a >= 0 && b >= 0 && c >= 0 && a < surface.numVerts && b < surface.numVerts && c < surface.numVerts )
					AddTriangle( tris, base + a, base + b, base + c );
			}
		} else if ( surface.surfaceType == MST_PATCH && surface.patchWidth >= 3 && surface.patchHeight >= 3 ) {
			constexpr int subdivisions = 4;
			for ( int py = 0; py + 2 < surface.patchHeight; py += 2 ) for ( int px = 0; px + 2 < surface.patchWidth; px += 2 ) {
				idVec3 control[3][3];
				for ( int y = 0; y < 3; ++y ) for ( int x = 0; x < 3; ++x )
					control[y][x] = drawVerts[surface.firstVert + ( py + y ) * surface.patchWidth + px + x].xyz;
				const int base = verts.Num() / 3;
				for ( int y = 0; y <= subdivisions; ++y ) for ( int x = 0; x <= subdivisions; ++x ) {
					idVec3 point = PatchPoint( control, x / float( subdivisions ), y / float( subdivisions ) );
					AddVertex( verts, point );
				}
				for ( int y = 0; y < subdivisions; ++y ) for ( int x = 0; x < subdivisions; ++x ) {
					const int a = base + y * ( subdivisions + 1 ) + x;
					const int b = a + 1, c = a + subdivisions + 1, d = c + 1;
					AddTriangle( tris, a, c, b );
					AddTriangle( tris, b, c, d );
				}
			}
		}
	}

	// Add collision-only clip brushes. They do not necessarily have render
	// surfaces, but they must constrain the navmesh exactly like game movement.
	for ( int brushIndex = 0; brushIndex < brushes.Num(); ++brushIndex ) {
		const dbrush_t &brush = brushes[brushIndex];
		if ( brush.shaderNum < 0 || brush.shaderNum >= shaders.Num() || brush.firstSide < 0 ||
			brush.numSides < 4 || brush.firstSide + brush.numSides > brushSides.Num() ) continue;
		const int contents = shaders[brush.shaderNum].contentFlags;
		if ( !( contents & ( CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP ) ) ||
			( contents & ( kContentsLava | kContentsSlime | kContentsWater | kContentsFog ) ) ) continue;
		for ( int sideIndex = 0; sideIndex < brush.numSides; ++sideIndex ) {
			const dbrushside_t &side = brushSides[brush.firstSide + sideIndex];
			if ( side.planeNum < 0 || side.planeNum >= planes.Num() ) continue;
			const dplane_t &plane = planes[side.planeNum];
			const idVec3 normal( plane.normal[0], plane.normal[1], plane.normal[2] );
			idVec3 up( idMath::Fabs( normal[0] ) < 0.6f ? 1.0f : 0.0f,
				idMath::Fabs( normal[0] ) < 0.6f ? 0.0f : 1.0f, 0.0f );
			up -= normal * ( up * normal );
			if ( up.Normalize() < 0.001f ) continue;
			const idVec3 right = normal.Cross( up );
			constexpr float extent = 131072.0f;
			const idVec3 origin = normal * plane.dist;
			idWinding winding( 4 );
			winding.AddPoint( origin - right * extent + up * extent );
			winding.AddPoint( origin + right * extent + up * extent );
			winding.AddPoint( origin + right * extent - up * extent );
			winding.AddPoint( origin - right * extent - up * extent );
			for ( int clipIndex = 0; clipIndex < brush.numSides && winding.GetNumPoints() >= 3; ++clipIndex ) {
				if ( clipIndex == sideIndex ) continue;
				const dbrushside_t &clipSide = brushSides[brush.firstSide + clipIndex];
				if ( clipSide.planeNum < 0 || clipSide.planeNum >= planes.Num() ) { winding.Clear(); break; }
				const dplane_t &clip = planes[clipSide.planeNum];
				const idPlane clipPlane( idVec3( clip.normal[0], clip.normal[1], clip.normal[2] ), clip.dist );
				winding.ClipInPlace( -clipPlane, 0.1f );
			}
			if ( winding.GetNumPoints() < 3 ) continue;
			const int base = verts.Num() / 3;
			for ( int i = 0; i < winding.GetNumPoints(); ++i ) AddVertex( verts, winding[i].ToVec3() );
			for ( int i = 1; i + 1 < winding.GetNumPoints(); ++i ) AddTriangle( tris, base, base + i, base + i + 1 );
		}
	}
	if ( verts.Num() == 0 || tris.Num() == 0 ) { error = "BSP contained no usable collision surfaces"; return false; }
	return true;
}

template<typename T, void ( *FreeFn )( T * )>
struct RcOwner {
	T *value = nullptr;
	~RcOwner() { if ( value ) FreeFn( value ); }
	T &operator*() const { return *value; }
	T *operator->() const { return value; }
};

void CreateOutputDirectories( const char *path ) {
	const idStr output = path;
	const int lastSlash = Max( output.Last( '/' ), output.Last( '\\' ) );
	if ( lastSlash < 0 )
		return;
	const idStr directory = output.Left( lastSlash );
	for ( int i = 0; i <= directory.Length(); ++i ) {
		if ( i != directory.Length() && directory[i] != '/' && directory[i] != '\\' )
			continue;
		const idStr part = directory.Left( i );
		if ( part.IsEmpty() || part == "." || ( part.Length() == 2 && part[1] == ':' ) )
			continue;
		std::error_code error;
		std::filesystem::create_directory( part.c_str(), error );
	}
}
}

NavMeshBuilder::NavMeshBuilder( const NavBuildOptions &buildOptions ) : options( buildOptions ) {
}

bool NavMeshBuilder::Build( idStr &error ) const {
	AgentConfig agent;
	if ( !ParseConfig( options.configPath.c_str(), agent, error ) ) return false;
	idList<float> verts;
	idList<int> tris;
	uint32 checksum = 0;
	if ( !ExtractGeometry( options.bspPath.c_str(), verts, tris, checksum, error ) ) return false;

	BuildContext context;
	rcConfig cfg{};
	cfg.cs = idMath::ClampFloat( 4.0f, 8.0f, Max( idMath::Fabs( agent.mins[0] ), idMath::Fabs( agent.mins[1] ) ) / 2.0f );
	cfg.ch = 2.0f;
	cfg.walkableSlopeAngle = idMath::ACos( idMath::ClampFloat( 0.0f, 1.0f, agent.maxSteepness ) ) * 180.0f / idMath::PI;
	const int normalHeight = static_cast<int>( idMath::Ceil( ( agent.maxs[2] - agent.mins[2] ) / cfg.ch ) );
	const int crouchHeight = static_cast<int>( idMath::Ceil( ( agent.crouchMaxs[2] - agent.crouchMins[2] ) / cfg.ch ) );
	cfg.walkableHeight = crouchHeight;
	cfg.walkableClimb = static_cast<int>( idMath::Floor( agent.maxStep / cfg.ch ) );
	cfg.walkableRadius = static_cast<int>( idMath::Ceil( Max( idMath::Fabs( agent.mins[0] ), idMath::Fabs( agent.mins[1] ) ) / cfg.cs ) );
	cfg.maxEdgeLen = static_cast<int>( 12.0f * cfg.walkableRadius );
	cfg.maxSimplificationError = 1.3f;
	cfg.minRegionArea = rcSqr( 8 );
	cfg.mergeRegionArea = rcSqr( 20 );
	cfg.maxVertsPerPoly = DT_VERTS_PER_POLYGON;
	cfg.detailSampleDist = cfg.cs * 6.0f;
	cfg.detailSampleMaxError = cfg.ch;
	rcCalcBounds( verts.Ptr(), verts.Num() / 3, cfg.bmin, cfg.bmax );
	rcCalcGridSize( cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height );
	printf( "input: %d vertices, %d triangles\n", verts.Num() / 3, tris.Num() / 3 );
	printf( "agent: radius=%g height=%g crouch=%g climb=%g slope=%g ladders=%s\n",
			cfg.walkableRadius * cfg.cs, cfg.walkableHeight * cfg.ch,
			agent.crouchMaxs[2] - agent.crouchMins[2], cfg.walkableClimb * cfg.ch,
			cfg.walkableSlopeAngle, agent.allowLadders ? "yes" : "no" );

	RcOwner<rcHeightfield, rcFreeHeightField> solid{ rcAllocHeightfield() };
	if ( !solid.value || !rcCreateHeightfield( &context, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch ) ) { error = "could not create heightfield"; return false; }
	idList<unsigned char> areas;
	areas.Resize( tris.Num() / 3 );
	rcMarkWalkableTriangles( &context, cfg.walkableSlopeAngle, verts.Ptr(), verts.Num() / 3, tris.Ptr(), tris.Num() / 3, areas.Ptr() );
	if ( !rcRasterizeTriangles( &context, verts.Ptr(), verts.Num() / 3, tris.Ptr(), areas.Ptr(), tris.Num() / 3, *solid, cfg.walkableClimb ) ) { error = "could not rasterize BSP"; return false; }
	rcFilterLowHangingWalkableObstacles( &context, cfg.walkableClimb, *solid );
	rcFilterLedgeSpans( &context, cfg.walkableHeight, cfg.walkableClimb, *solid );
	rcFilterWalkableLowHeightSpans( &context, cfg.walkableHeight, *solid );
	RcOwner<rcCompactHeightfield, rcFreeCompactHeightfield> compact{ rcAllocCompactHeightfield() };
	if ( !compact.value || !rcBuildCompactHeightfield( &context, cfg.walkableHeight, cfg.walkableClimb, *solid, *compact ) ) { error = "could not build compact heightfield"; return false; }
	for ( int i = 0; i < compact->spanCount; ++i ) if ( compact->areas[i] != RC_NULL_AREA && static_cast<int>( compact->spans[i].h ) < normalHeight ) compact->areas[i] = 1;
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
	params.walkableRadius = Max( idMath::Fabs( agent.mins[0] ), idMath::Fabs( agent.mins[1] ) );
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
	header.navDataSize = static_cast<uint32>( navDataSize ); header.sourceChecksum = checksum; header.agentIndex = options.agentIndex;
	header.agentRadius = params.walkableRadius; header.agentHeight = params.walkableHeight;
	header.crouchHeight = agent.crouchMaxs[2] - agent.crouchMins[2]; header.maxClimb = agent.maxStep;
	header.maxSlope = cfg.walkableSlopeAngle; header.gravity = agent.gravity; header.jumpVelocity = agent.jumpVelocity;
	header.maxWalkVelocity = agent.maxWalkVelocity; header.maxCrouchVelocity = agent.maxCrouchVelocity;
	header.maxBarrier = agent.maxBarrier; header.allowLadders = agent.allowLadders ? 1u : 0u;
	CreateOutputDirectories( options.outputPath.c_str() );
	FILE *output = fopen( options.outputPath.c_str(), "wb" );
	if ( !output ) { dtFree( navData ); error = idStr( "could not create " ) + options.outputPath; return false; }
	const bool wroteHeader = fwrite( &header, 1, sizeof( header ), output ) == sizeof( header );
	const bool wroteData = fwrite( navData, 1, static_cast<size_t>( navDataSize ), output ) == static_cast<size_t>( navDataSize );
	fclose( output );
	dtFree( navData );
	if ( !wroteHeader || !wroteData ) { error = idStr( "failed writing " ) + options.outputPath; return false; }
	printf( "navmesh: %d vertices, %d polygons, %d bytes\n", mesh->nverts, mesh->npolys, navDataSize );
	return true;
}
