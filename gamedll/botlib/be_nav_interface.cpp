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

#include "../game/q_shared.h"
#include "aasfile.h"
#include "../game/botlib.h"
#include "be_aas.h"
#include "be_nav_interface.h"

#include "be_interface.h"
#include "../nav/wolf_nav.h"

#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
struct NavPoint {
	float xyz[3]{};
	operator float *() {
		return xyz;
	}
	operator const float *() const {
		return xyz;
	}
};

struct NavWorld {
	wolf::nav::FileHeader header{};
	dtNavMesh *mesh = nullptr;
	dtNavMeshQuery *query = nullptr;
	dtQueryFilter filter;
	std::vector<dtPolyRef> areaRefs{ 0 };
	std::unordered_map<dtPolyRef, int> refAreas;
	std::vector<NavPoint> centers = std::vector<NavPoint>( 1 );
	std::vector<aas_reachability_t> reaches{ 1 };
	std::vector<std::vector<int>> areaReaches{ 1 };
	bool loaded = false;
};

NavWorld worlds[MAX_AAS_WORLDS];
int currentWorld = 0;
float navTime = 0;
bool navSetup = false;
std::vector<aas_entityinfo_t> entities;
std::vector<std::unordered_map<std::string, std::string>> bspEntities;

NavWorld &World() {
	return worlds[currentWorld];
}

void ClearWorld( NavWorld &world ) {
	if ( world.query )
		dtFreeNavMeshQuery( world.query );
	if ( world.mesh )
		dtFreeNavMesh( world.mesh );
	world = NavWorld{};
}

void CopyVec( const float *from, float *to ) {
	to[0] = from[0];
	to[1] = from[1];
	to[2] = from[2];
}

void PolyCenter( NavWorld &world, dtPolyRef ref, vec3_t center ) {
	const dtMeshTile *tile = nullptr;
	const dtPoly *poly = nullptr;
	if ( !world.mesh || dtStatusFailed( world.mesh->getTileAndPolyByRef( ref, &tile, &poly ) ) ) {
		VectorClear( center );
		return;
	}
	float nav[3]{ 0, 0, 0 };
	for ( unsigned int i = 0; i < poly->vertCount; ++i ) {
		const float *v = &tile->verts[poly->verts[i] * 3];
		nav[0] += v[0];
		nav[1] += v[1];
		nav[2] += v[2];
	}
	const float scale = poly->vertCount ? 1.0f / poly->vertCount : 0.0f;
	nav[0] *= scale;
	nav[1] *= scale;
	nav[2] *= scale;
	wolf::nav::NavToQuake( nav, center );
}

int AreaForRef( const NavWorld &world, dtPolyRef ref ) {
	auto it = world.refAreas.find( ref );
	return it == world.refAreas.end() ? 0 : it->second;
}

dtPolyRef RefForArea( const NavWorld &world, int area ) {
	return area > 0 && area < static_cast<int>( world.areaRefs.size() ) ? world.areaRefs[area] : 0;
}

unsigned char PolyArea( const NavWorld &world, int area ) {
	const dtPolyRef ref = RefForArea( world, area );
	const dtMeshTile *tile = nullptr;
	const dtPoly *poly = nullptr;
	if ( !ref || !world.mesh || dtStatusFailed( world.mesh->getTileAndPolyByRef( ref, &tile, &poly ) ) )
		return 0;
	return poly->getArea();
}

int NearestArea( NavWorld &world, const vec3_t point, float *nearestQuake = nullptr ) {
	if ( !world.loaded || !world.query )
		return 0;
	float nav[3], nearest[3];
	wolf::nav::QuakeToNav( point, nav );
	const float extents[3]{ ( std::max )( 64.0f, world.header.agentRadius * 2 ),
							( std::max )( 128.0f, world.header.agentHeight * 2 ),
							( std::max )( 64.0f, world.header.agentRadius * 2 ) };
	dtPolyRef ref = 0;
	if ( dtStatusFailed( world.query->findNearestPoly( nav, extents, &world.filter, &ref, nearest ) ) || !ref )
		return 0;
	if ( nearestQuake )
		wolf::nav::NavToQuake( nearest, nearestQuake );
	return AreaForRef( world, ref );
}

float NavDistance( const float *a, const float *b ) {
	const float x = a[0] - b[0], y = a[1] - b[1], z = a[2] - b[2];
	return std::sqrt( x * x + y * y + z * z );
}

int FindPath( NavWorld &world, int startArea, const vec3_t start, int goalArea, std::vector<dtPolyRef> &path ) {
	const dtPolyRef startRef = RefForArea( world, startArea ), goalRef = RefForArea( world, goalArea );
	if ( !startRef || !goalRef )
		return 0;
	float navStart[3], navGoal[3];
	wolf::nav::QuakeToNav( start, navStart );
	wolf::nav::QuakeToNav( world.centers[goalArea], navGoal );
	path.resize( 4096 );
	int count = 0;
	if ( dtStatusFailed( world.query->findPath( startRef, goalRef, navStart, navGoal, &world.filter, path.data(),
												&count, static_cast<int>( path.size() ) ) ) )
		return 0;
	path.resize( count );
	return count && path.back() == goalRef ? count : 0;
}

int ReachBetween( const NavWorld &world, int fromArea, int toArea ) {
	if ( fromArea <= 0 || fromArea >= static_cast<int>( world.areaReaches.size() ) )
		return 0;
	for ( int reach : world.areaReaches[fromArea] )
		if ( world.reaches[reach].areanum == toArea )
			return reach;
	return 0;
}

void BuildAreaIndex( NavWorld &world ) {
	world.areaRefs.assign( 1, 0 );
	world.refAreas.clear();
	world.centers.resize( 1 );
	for ( int tileIndex = 0; tileIndex < world.mesh->getMaxTiles(); ++tileIndex ) {
		const dtMeshTile *tile = static_cast<const dtNavMesh *>( world.mesh )->getTile( tileIndex );
		if ( !tile || !tile->header )
			continue;
		const dtPolyRef base = world.mesh->getPolyRefBase( tile );
		for ( int polyIndex = 0; polyIndex < tile->header->polyCount; ++polyIndex ) {
			const dtPoly &poly = tile->polys[polyIndex];
			if ( poly.getType() != DT_POLYTYPE_GROUND )
				continue;
			const dtPolyRef ref = base | static_cast<dtPolyRef>( polyIndex );
			const int area = static_cast<int>( world.areaRefs.size() );
			world.areaRefs.push_back( ref );
			world.refAreas[ref] = area;
			vec3_t center;
			PolyCenter( world, ref, center );
			world.centers.push_back( {} );
			CopyVec( center, world.centers.back() );
		}
	}
	world.areaReaches.assign( world.areaRefs.size(), {} );
	world.reaches.assign( 1, {} );
	for ( int area = 1; area < static_cast<int>( world.areaRefs.size() ); ++area ) {
		const dtMeshTile *tile = nullptr;
		const dtPoly *poly = nullptr;
		if ( dtStatusFailed( world.mesh->getTileAndPolyByRef( world.areaRefs[area], &tile, &poly ) ) )
			continue;
		for ( unsigned int edge = 0; edge < poly->vertCount; ++edge ) {
			const unsigned short nei = poly->neis[edge];
			if ( !nei || ( nei & DT_EXT_LINK ) )
				continue;
			const dtPolyRef targetRef = world.mesh->getPolyRefBase( tile ) | static_cast<dtPolyRef>( nei - 1 );
			const int targetArea = AreaForRef( world, targetRef );
			if ( !targetArea )
				continue;
			aas_reachability_t reach{};
			reach.areanum = targetArea;
			reach.traveltype =
				PolyArea( world, area ) == 1 || PolyArea( world, targetArea ) == 1 ? TRAVEL_CROUCH : TRAVEL_WALK;
			// AAS movement steers to reach.start before crossing to reach.end.  Polygon
			// centers are unsuitable here: on stairs they can send the bot back up the
			// current tread.  Put both points on opposite sides of the shared portal.
			const float *navA = &tile->verts[poly->verts[edge] * 3];
			const float *navB = &tile->verts[poly->verts[( edge + 1 ) % poly->vertCount] * 3];
			float navPortal[3]{ ( navA[0] + navB[0] ) * 0.5f, ( navA[1] + navB[1] ) * 0.5f,
								( navA[2] + navB[2] ) * 0.5f };
			vec3_t portal;
			wolf::nav::NavToQuake( navPortal, portal );
			float dx = world.centers[targetArea].xyz[0] - world.centers[area].xyz[0];
			float dy = world.centers[targetArea].xyz[1] - world.centers[area].xyz[1];
			const float length = std::sqrt( dx * dx + dy * dy );
			if ( length > 0.001f ) {
				dx /= length;
				dy /= length;
			} else {
				float quakeA[3], quakeB[3];
				wolf::nav::NavToQuake( navA, quakeA );
				wolf::nav::NavToQuake( navB, quakeB );
				dx = -( quakeB[1] - quakeA[1] );
				dy = quakeB[0] - quakeA[0];
				const float perpendicularLength = std::sqrt( dx * dx + dy * dy );
				if ( perpendicularLength > 0.001f ) {
					dx /= perpendicularLength;
					dy /= perpendicularLength;
				}
			}
			constexpr float portalInset = 4.0f;
			VectorSet( reach.start, portal[0] - dx * portalInset, portal[1] - dy * portalInset, portal[2] + 1.0f );
			VectorSet( reach.end, portal[0] + dx * portalInset, portal[1] + dy * portalInset, portal[2] + 1.0f );
			reach.traveltime = AAS_AreaTravelTime( area, reach.start, reach.end );
			world.reaches.push_back( reach );
			world.areaReaches[area].push_back( static_cast<int>( world.reaches.size() - 1 ) );
		}
	}
}

bool LoadWorldFile( NavWorld &world, const std::string &path ) {
	fileHandle_t file = 0;
	const int length = botimport.FS_FOpenFile( path.c_str(), &file, FS_READ );
	if ( length < static_cast<int>( sizeof( wolf::nav::FileHeader ) ) || !file )
		return false;
	wolf::nav::FileHeader header{};
	botimport.FS_Read( &header, sizeof( header ), file );
	if ( header.magic != wolf::nav::kMagic || header.version != wolf::nav::kVersion ||
		 header.headerSize != sizeof( header ) ||
		 header.navDataSize > static_cast<unsigned int>( length - sizeof( header ) ) ) {
		botimport.FS_FCloseFile( file );
		return false;
	}
	unsigned char *data = static_cast<unsigned char *>( dtAlloc( header.navDataSize, DT_ALLOC_PERM ) );
	if ( !data ) {
		botimport.FS_FCloseFile( file );
		return false;
	}
	botimport.FS_Read( data, header.navDataSize, file );
	botimport.FS_FCloseFile( file );
	world.mesh = dtAllocNavMesh();
	world.query = dtAllocNavMeshQuery();
	if ( !world.mesh || !world.query ) {
		dtFree( data );
		ClearWorld( world );
		return false;
	}
	if ( dtStatusFailed( world.mesh->init( data, header.navDataSize, DT_TILE_FREE_DATA ) ) ) {
		dtFree( data );
		ClearWorld( world );
		return false;
	}
	// A successful navmesh init transfers ownership of data to the mesh.
	if ( dtStatusFailed( world.query->init( world.mesh, 4096 ) ) ) {
		ClearWorld( world );
		return false;
	}
	world.header = header;
	world.filter.setIncludeFlags( wolf::nav::PolyWalk | wolf::nav::PolySwim | wolf::nav::PolyDoor |
								  wolf::nav::PolyJump );
	world.filter.setExcludeFlags( wolf::nav::PolyDisabled );
	world.loaded = true;
	BuildAreaIndex( world );
	return true;
}

std::string BaseMapName( const char *mapname ) {
	std::string name = mapname ? mapname : "";
	const std::size_t slash = name.find_last_of( "/\\" );
	if ( slash != std::string::npos )
		name.erase( 0, slash + 1 );
	const std::size_t dot = name.find_last_of( '.' );
	if ( dot != std::string::npos )
		name.erase( dot );
	return name;
}

void ParseBspEntities() {
	bspEntities.clear();
	const char *data = botimport.BSPEntityData ? botimport.BSPEntityData() : nullptr;
	if ( !data )
		return;
	const char *p = data;
	auto token = [&]() -> std::string {
		while ( *p && static_cast<unsigned char>( *p ) <= ' ' )
			++p;
		if ( *p == '{' || *p == '}' )
			return std::string( 1, *p++ );
		if ( *p == '"' ) {
			++p;
			const char *start = p;
			while ( *p && *p != '"' )
				++p;
			std::string result( start, p );
			if ( *p )
				++p;
			return result;
		}
		const char *start = p;
		while ( *p && static_cast<unsigned char>( *p ) > ' ' && *p != '{' && *p != '}' )
			++p;
		return std::string( start, p );
	};
	for ( ;; ) {
		std::string open = token();
		if ( open.empty() )
			break;
		if ( open != "{" )
			continue;
		std::unordered_map<std::string, std::string> entity;
		for ( ;; ) {
			std::string key = token();
			if ( key.empty() || key == "}" )
				break;
			entity[key] = token();
		}
		bspEntities.push_back( std::move( entity ) );
	}
}
} // namespace

int AAS_Setup() {
	navSetup = true;
	entities.assign( ( std::max )( 2048, botlibglobals.maxentities ), {} );
	return BLERR_NOERROR;
}
void AAS_Shutdown() {
	for ( auto &world : worlds )
		ClearWorld( world );
	entities.clear();
	bspEntities.clear();
	navSetup = false;
}
int AAS_LoadMap( const char *mapname ) {
	for ( auto &world : worlds )
		ClearWorld( world );
	const std::string base = BaseMapName( mapname );
	int loaded = 0;
	for ( int i = 0; i < MAX_AAS_WORLDS; ++i )
		loaded += LoadWorldFile( worlds[i], "maps/" + base + "_b" + std::to_string( i ) + ".nav" ) ? 1 : 0;
	ParseBspEntities();
	currentWorld = 0;
	if ( !loaded ) {
		botimport.Print( PRT_ERROR, "No navmesh found for maps/%s_b[01].nav\n", const_cast<char *>( base.c_str() ) );
		return BLERR_NOAASFILE;
	}
	return BLERR_NOERROR;
}
int AAS_StartFrame( float time ) {
	navTime = time;
	for ( auto &entity : entities )
		entity.valid = qfalse;
	return BLERR_NOERROR;
}
int AAS_Initialized() {
	return navSetup && World().loaded;
}
int AAS_Loaded() {
	return World().loaded;
}
float AAS_Time() {
	return navTime;
}
void AAS_SetCurrentWorld( int index ) {
	if ( index >= 0 && index < MAX_AAS_WORLDS && worlds[index].loaded )
		currentWorld = index;
}
char *AAS_ModelFromIndex( int index ) {
	static char model[32];
	std::snprintf( model, sizeof( model ), "*%d", index );
	return model;
}
int AAS_IndexFromModel( char *modelname ) {
	return modelname && modelname[0] == '*' ? std::atoi( modelname + 1 ) : 0;
}

int AAS_UpdateEntity( int entnum, bot_entitystate_t *state ) {
	if ( entnum < 0 || entnum >= static_cast<int>( entities.size() ) || !state )
		return BLERR_INVALIDENTITYNUMBER;
	aas_entityinfo_t &info = entities[entnum];
	info.update_time = navTime - info.ltime;
	info.ltime = navTime;
	info.valid = qtrue;
	info.number = entnum;
	info.type = state->type;
	info.flags = state->flags;
	info.solid = state->solid;
	info.groundent = state->groundent;
	info.modelindex = state->modelindex;
	info.modelindex2 = state->modelindex2;
	info.frame = state->frame;
	info.eventParm = state->eventParm;
	info.powerups = state->powerups;
	info.weapon = state->weapon;
	info.legsAnim = state->legsAnim;
	info.torsoAnim = state->torsoAnim;
	CopyVec( info.origin, info.lastvisorigin );
	CopyVec( state->origin, info.origin );
	CopyVec( state->angles, info.angles );
	CopyVec( state->old_origin, info.old_origin );
	CopyVec( state->mins, info.mins );
	CopyVec( state->maxs, info.maxs );
	return BLERR_NOERROR;
}
void AAS_EntityInfo( int entnum, aas_entityinfo_t *info ) {
	if ( !info )
		return;
	if ( entnum >= 0 && entnum < static_cast<int>( entities.size() ) )
		*info = entities[entnum];
	else
		std::memset( info, 0, sizeof( *info ) );
}
int AAS_NextEntity( int entnum ) {
	for ( int i = entnum + 1; i < static_cast<int>( entities.size() ); ++i )
		if ( entities[i].valid )
			return i;
	return 0;
}
int AAS_EntityType( int entnum ) {
	return entnum >= 0 && entnum < static_cast<int>( entities.size() ) ? entities[entnum].type : 0;
}
int AAS_EntityModelindex( int entnum ) {
	return entnum >= 0 && entnum < static_cast<int>( entities.size() ) ? entities[entnum].modelindex : 0;
}
int AAS_EntityModelNum( int entnum ) {
	return AAS_EntityModelindex( entnum );
}
int AAS_OriginOfEntityWithModelNum( int modelnum, vec3_t origin ) {
	for ( const auto &entity : entities )
		if ( entity.valid && entity.modelindex == modelnum ) {
			CopyVec( entity.origin, origin );
			return qtrue;
		}
	return qfalse;
}
int AAS_IsEntityInArea( int ignore1, int ignore2, int area ) {
	for ( const auto &entity : entities )
		if ( entity.valid && entity.number != ignore1 && entity.number != ignore2 &&
			 AAS_PointAreaNum( const_cast<float *>( entity.origin ) ) == area )
			return qtrue;
	return qfalse;
}

bsp_trace_t AAS_Trace( vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int passent, int contentmask ) {
	bsp_trace_t trace{};
	botimport.Trace( &trace, start, mins, maxs, end, passent, contentmask );
	return trace;
}
int AAS_PointContents( vec3_t point ) {
	return botimport.PointContents( point );
}
qboolean AAS_inPVS( vec3_t p1, vec3_t p2 ) {
	return botimport.inPVS( p1, p2 );
}
qboolean AAS_AreasConnected( int area1, int area2 ) {
	NavWorld &world = World();
	if ( !RefForArea( world, area1 ) || !RefForArea( world, area2 ) )
		return qfalse;
	std::vector<dtPolyRef> path;
	return FindPath( world, area1, world.centers[area1], area2, path ) ? qtrue : qfalse;
}
void AAS_BSPModelMinsMaxsOrigin( int modelnum, vec3_t angles, vec3_t mins, vec3_t maxs, vec3_t origin ) {
	botimport.BSPModelMinsMaxsOrigin( modelnum, angles, mins, maxs, origin );
}
int AAS_NextBSPEntity( int ent ) {
	const int next = ent + 1;
	return next > 0 && next <= static_cast<int>( bspEntities.size() ) ? next : 0;
}
int AAS_ValueForBSPEpairKey( int ent, char *key, char *value, int size ) {
	if ( value && size )
		value[0] = 0;
	if ( ent <= 0 || ent > static_cast<int>( bspEntities.size() ) || !key || !value || size <= 0 )
		return qfalse;
	auto it = bspEntities[ent - 1].find( key );
	if ( it == bspEntities[ent - 1].end() )
		return qfalse;
	std::strncpy( value, it->second.c_str(), size - 1 );
	value[size - 1] = 0;
	return qtrue;
}
int AAS_VectorForBSPEpairKey( int ent, char *key, vec3_t v ) {
	char value[MAX_EPAIRKEY]{};
	if ( !AAS_ValueForBSPEpairKey( ent, key, value, sizeof( value ) ) )
		return qfalse;
	return std::sscanf( value, "%f %f %f", &v[0], &v[1], &v[2] ) == 3;
}
int AAS_FloatForBSPEpairKey( int ent, char *key, float *value ) {
	char text[MAX_EPAIRKEY]{};
	if ( !AAS_ValueForBSPEpairKey( ent, key, text, sizeof( text ) ) )
		return qfalse;
	*value = std::strtof( text, nullptr );
	return qtrue;
}
int AAS_IntForBSPEpairKey( int ent, char *key, int *value ) {
	char text[MAX_EPAIRKEY]{};
	if ( !AAS_ValueForBSPEpairKey( ent, key, text, sizeof( text ) ) )
		return qfalse;
	*value = std::atoi( text );
	return qtrue;
}

void AAS_PresenceTypeBoundingBox( int presencetype, vec3_t mins, vec3_t maxs ) {
	const auto &h = World().header;
	const float radius = h.agentRadius, height = presencetype == PRESENCE_CROUCH ? h.crouchHeight : h.agentHeight;
	VectorSet( mins, -radius, -radius, -24 );
	VectorSet( maxs, radius, radius, height - 24 );
}
int AAS_PointAreaNum( vec3_t point ) {
	return NearestArea( World(), point );
}
int AAS_TraceAreas( vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas ) {
	if ( maxareas <= 0 )
		return 0;
	int count = 0, previous = 0;
	for ( int sample = 0; sample <= 64 && count < maxareas; ++sample ) {
		const float t = sample / 64.0f;
		vec3_t p;
		for ( int i = 0; i < 3; ++i )
			p[i] = start[i] + ( end[i] - start[i] ) * t;
		const int area = AAS_PointAreaNum( p );
		if ( area && area != previous ) {
			areas[count] = area;
			if ( points )
				CopyVec( p, points[count] );
			previous = area;
			++count;
		}
	}
	return count;
}
int AAS_AreaCluster( int areanum ) {
	return RefForArea( World(), areanum ) ? 1 : 0;
}
int AAS_AreaPresenceType( int areanum ) {
	if ( !RefForArea( World(), areanum ) )
		return PRESENCE_NONE;
	return PolyArea( World(), areanum ) == 1 ? PRESENCE_CROUCH : PRESENCE_NORMAL | PRESENCE_CROUCH;
}
int AAS_PointPresenceType( vec3_t point ) {
	return AAS_AreaPresenceType( AAS_PointAreaNum( point ) );
}
int AAS_AreaReachability( int areanum ) {
	const dtPolyRef ref = RefForArea( World(), areanum );
	unsigned short flags = 0;
	return ref && dtStatusSucceed( World().mesh->getPolyFlags( ref, &flags ) ) && !( flags & wolf::nav::PolyDisabled );
}
int AAS_BestReachableArea( vec3_t origin, vec3_t, vec3_t, vec3_t goalorigin ) {
	const int area = NearestArea( World(), origin, goalorigin );
	if ( !area )
		CopyVec( origin, goalorigin );
	return area;
}
int AAS_NextModelReachability( int, int ) {
	return 0;
}
int AAS_AreaLava( int ) {
	return qfalse;
}
int AAS_AreaSlime( int ) {
	return qfalse;
}
int AAS_AreaJumpPad( int ) {
	return qfalse;
}
int AAS_AreaDoNotEnter( int ) {
	return qfalse;
}
int AAS_AreaDoNotEnterLarge( int ) {
	return qfalse;
}

int AAS_TravelFlagForType( int type ) {
	switch ( type & TRAVELTYPE_MASK ) {
	case TRAVEL_CROUCH:
		return TFL_CROUCH;
	case TRAVEL_JUMP:
		return TFL_JUMP;
	case TRAVEL_LADDER:
		return TFL_LADDER;
	case TRAVEL_SWIM:
		return TFL_SWIM;
	default:
		return TFL_WALK;
	}
}
int AAS_AreaContentsTravelFlag( int ) {
	return TFL_WALK;
}
int AAS_NextAreaReachability( int area, int reach ) {
	const auto &list = area > 0 && area < static_cast<int>( World().areaReaches.size() ) ? World().areaReaches[area]
																						 : std::vector<int>{};
	if ( list.empty() )
		return 0;
	if ( !reach )
		return list.front();
	auto it = std::find( list.begin(), list.end(), reach );
	return it != list.end() && ++it != list.end() ? *it : 0;
}
void AAS_ReachabilityFromNum( int num, aas_reachability_t *reach ) {
	if ( !reach )
		return;
	if ( num > 0 && num < static_cast<int>( World().reaches.size() ) )
		*reach = World().reaches[num];
	else
		std::memset( reach, 0, sizeof( *reach ) );
}
unsigned short AAS_AreaTravelTime( int, vec3_t start, vec3_t end ) {
	const float speed = ( std::max )( 1.0f, World().header.maxWalkVelocity );
	return static_cast<unsigned short>(
		( std::min )( 65535.0f, ( std::max )( 1.0f, NavDistance( start, end ) * 100.0f / speed ) ) );
}
int AAS_AreaRouteToGoalArea( int area, vec3_t origin, int goal, int, int *travelTime, int *reachNum ) {
	if ( travelTime )
		*travelTime = 0;
	if ( reachNum )
		*reachNum = 0;
	std::vector<dtPolyRef> path;
	if ( !FindPath( World(), area, origin, goal, path ) )
		return qfalse;
	float total = NavDistance( origin, World().centers[area] );
	for ( std::size_t i = 1; i < path.size(); ++i )
		total += NavDistance( World().centers[AreaForRef( World(), path[i - 1] )],
							  World().centers[AreaForRef( World(), path[i] )] );
	if ( travelTime )
		*travelTime = static_cast<int>( total * 100 / ( std::max )( 1.0f, World().header.maxWalkVelocity ) );
	if ( reachNum && path.size() > 1 )
		*reachNum = ReachBetween( World(), area, AreaForRef( World(), path[1] ) );
	return qtrue;
}
int AAS_AreaTravelTimeToGoalArea( int area, vec3_t origin, int goal, int flags ) {
	int time = 0, reach = 0;
	return AAS_AreaRouteToGoalArea( area, origin, goal, flags, &time, &reach ) ? ( std::max )( 1, time ) : 0;
}
int AAS_AreaTravelTimeToGoalAreaCheckLoop( int area, vec3_t origin, int goal, int flags, int loop ) {
	return area == loop ? 0 : AAS_AreaTravelTimeToGoalArea( area, origin, goal, flags );
}
int AAS_RandomGoalArea( int area, int, int *goalArea, vec3_t goalOrigin ) {
	if ( World().areaRefs.size() <= 1 )
		return qfalse;
	int chosen = 1 + std::rand() % ( static_cast<int>( World().areaRefs.size() ) - 1 );
	if ( chosen == area && World().areaRefs.size() > 2 )
		chosen = chosen % ( static_cast<int>( World().areaRefs.size() ) - 1 ) + 1;
	*goalArea = chosen;
	CopyVec( World().centers[chosen], goalOrigin );
	return qtrue;
}

int AAS_OnGround( vec3_t origin, int presencetype, int passent ) {
	vec3_t mins, maxs, end;
	AAS_PresenceTypeBoundingBox( presencetype, mins, maxs );
	CopyVec( origin, end );
	end[2] -= 3;
	return AAS_Trace( origin, mins, maxs, end, passent, CONTENTS_SOLID | CONTENTS_PLAYERCLIP ).fraction < 1.0f;
}
int AAS_Swimming( vec3_t origin ) {
	return ( AAS_PointContents( origin ) & CONTENTS_WATER ) != 0;
}
void AAS_JumpReachRunStart( aas_reachability_t *reach, vec3_t runstart ) {
	if ( reach )
		CopyVec( reach->start, runstart );
	else
		VectorClear( runstart );
}
int AAS_AgainstLadder( vec3_t, int ) {
	return qfalse;
}
int AAS_HorizontalVelocityForJump( float zvel, vec3_t start, vec3_t end, float *velocity ) {
	const float gravity = ( std::max )( 1.0f, World().header.gravity );
	const float dz = end[2] - start[2];
	const float disc = zvel * zvel - 2 * gravity * dz;
	if ( disc < 0 )
		return qfalse;
	const float time = ( zvel + std::sqrt( disc ) ) / gravity;
	if ( time <= 0 )
		return qfalse;
	*velocity =
		std::sqrt( ( end[0] - start[0] ) * ( end[0] - start[0] ) + ( end[1] - start[1] ) * ( end[1] - start[1] ) ) /
		time;
	return qtrue;
}
int AAS_DropToFloor( vec3_t origin, vec3_t, vec3_t ) {
	vec3_t nearest;
	if ( !NearestArea( World(), origin, nearest ) )
		return qfalse;
	CopyVec( nearest, origin );
	return qtrue;
}
aas_trace_t AAS_TraceClientBBox( vec3_t start, vec3_t end, int presence, int passent ) {
	vec3_t mins, maxs;
	AAS_PresenceTypeBoundingBox( presence, mins, maxs );
	bsp_trace_t bsp = AAS_Trace( start, mins, maxs, end, passent, CONTENTS_SOLID | CONTENTS_PLAYERCLIP );
	aas_trace_t result{};
	result.startsolid = bsp.startsolid;
	result.fraction = bsp.fraction;
	CopyVec( bsp.endpos, result.endpos );
	result.ent = bsp.ent;
	result.lastarea = AAS_PointAreaNum( bsp.endpos );
	result.area = result.lastarea;
	result.planenum = bsp.sidenum;
	return result;
}
int AAS_PredictClientMovement( aas_clientmove_t *move, int entnum, vec3_t origin, int presence, int onground,
							   vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float dt, int stopEvent,
							   int stopArea, int ) {
	if ( !move )
		return qfalse;
	std::memset( move, 0, sizeof( *move ) );
	vec3_t pos, vel;
	CopyVec( origin, pos );
	CopyVec( velocity, vel );
	for ( int frame = 0; frame < maxframes; ++frame ) {
		if ( frame < cmdframes ) {
			vel[0] += cmdmove[0] * dt;
			vel[1] += cmdmove[1] * dt;
			vel[2] += cmdmove[2] * dt;
		}
		if ( !onground )
			vel[2] -= World().header.gravity * dt;
		vec3_t end{ pos[0] + vel[0] * dt, pos[1] + vel[1] * dt, pos[2] + vel[2] * dt };
		move->trace = AAS_TraceClientBBox( pos, end, presence, entnum );
		CopyVec( move->trace.endpos, pos );
		move->frames = frame + 1;
		move->time += dt;
		const int area = AAS_PointAreaNum( pos );
		if ( move->trace.fraction < 1 ) {
			move->stopevent = SE_HITGROUND;
			break;
		}
		if ( ( stopEvent & SE_ENTERAREA ) && area == stopArea ) {
			move->stopevent = SE_ENTERAREA;
			break;
		}
	}
	CopyVec( pos, move->endpos );
	CopyVec( vel, move->velocity );
	move->presencetype = presence;
	move->endcontents = static_cast<float>( AAS_PointContents( pos ) );
	return qtrue;
}

void AAS_SetAASBlockingEntity( vec3_t mins, vec3_t maxs, qboolean blocking ) {
	NavWorld &world = World();
	if ( !world.loaded )
		return;
	for ( int area = 1; area < static_cast<int>( world.centers.size() ); ++area ) {
		const float *p = world.centers[area];
		if ( p[0] >= mins[0] && p[0] <= maxs[0] && p[1] >= mins[1] && p[1] <= maxs[1] && p[2] >= mins[2] &&
			 p[2] <= maxs[2] )
			world.mesh->setPolyFlags( world.areaRefs[area], blocking ? wolf::nav::PolyDisabled : wolf::nav::PolyWalk );
	}
}
void AAS_RT_ShowRoute( vec3_t, int, int ) {
}
qboolean AAS_RT_GetHidePos( vec3_t, int, int, vec3_t, int, int, vec3_t ) {
	return qfalse;
}
int AAS_FindAttackSpotWithinRange( int, int, int, float, int, float * ) {
	return qfalse;
}
qboolean AAS_GetRouteFirstVisPos( vec3_t src, vec3_t dest, int, vec3_t result ) {
	NavWorld &world = World();
	const int startArea = AAS_PointAreaNum( src ), goalArea = AAS_PointAreaNum( dest );
	std::vector<dtPolyRef> path;
	if ( !FindPath( world, startArea, src, goalArea, path ) )
		return qfalse;
	CopyVec( path.size() > 1 ? world.centers[AreaForRef( world, path[1] )] : dest, result );
	return qtrue;
}

void AAS_ClearShownDebugLines() {
}
void AAS_ClearShownPolygons() {
}
void AAS_DebugLine( vec3_t start, vec3_t end, int color ) {
	const int line = botimport.DebugLineCreate();
	botimport.DebugLineShow( line, start, end, color );
}
void AAS_DrawCross( vec3_t origin, float size, int color ) {
	for ( int axis = 0; axis < 3; ++axis ) {
		vec3_t a, b;
		CopyVec( origin, a );
		CopyVec( origin, b );
		a[axis] -= size;
		b[axis] += size;
		AAS_DebugLine( a, b, color );
	}
}
void AAS_DrawPermanentCross( vec3_t origin, float size, int color ) {
	AAS_DrawCross( origin, size, color );
}
void AAS_DrawPlaneCross( vec3_t point, vec3_t, float, int, int color ) {
	AAS_DrawCross( point, 8, color );
}
void AAS_ShowBoundingBox( vec3_t, vec3_t, vec3_t ) {
}
void AAS_ShowArea( int, int ) {
}
void AAS_ShowAreaPolygons( int, int, int ) {
}
void AAS_ShowReachability( aas_reachability_t *reach ) {
	if ( reach )
		AAS_DebugLine( reach->start, reach->end, 4 );
}
void AAS_ShowReachableAreas( int area ) {
	int reach = 0;
	aas_reachability_t value{};
	while ( ( reach = AAS_NextAreaReachability( area, reach ) ) != 0 ) {
		AAS_ReachabilityFromNum( reach, &value );
		AAS_ShowReachability( &value );
	}
}
void AAS_PrintTravelType( int traveltype ) {
	botimport.Print( PRT_MESSAGE, "nav travel type %d\n", traveltype );
}
