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

#include "../precompiled.h"

#include "../../engine/qcommon/q_shared.h"
#include "aasfile.h"
#include "../game/botlib.h"
#include "be_aas.h"
#include "be_nav_interface.h"

#include "be_interface.h"
#include "../../engine/qcommon/wolf_nav.h"

#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../../idlib/Dict.h"
#include "../../idlib/containers/HashTable.h"
#include "../../idlib/containers/List.h"

namespace {
struct NavWorld {
	wolf::nav::FileHeader header{};
	dtNavMesh *mesh = nullptr;
	dtNavMeshQuery *query = nullptr;
	dtQueryFilter filter;
	idList<dtPolyRef> areaRefs;
	idHashTableT<dtPolyRef, int> refAreas;
	idList<idVec3> centers;
	idList<aas_reachability_t> reaches;
	idList<idList<int>> areaReaches;
	bool loaded = false;

	NavWorld() {
		areaRefs.Append( 0 );
		centers.Append( idVec3() );
		reaches.Append( aas_reachability_t{} );
		areaReaches.Append( idList<int>() );
	}
};

NavWorld worlds[MAX_AAS_WORLDS];
int currentWorld = 0;
float navTime = 0;
bool navSetup = false;
idList<aas_entityinfo_t> entities;
idList<idDict> bspEntities;

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

void PolyCenter( NavWorld &world, dtPolyRef ref, idVec3 &center ) {
	const dtMeshTile *tile = nullptr;
	const dtPoly *poly = nullptr;
	if ( !world.mesh || dtStatusFailed( world.mesh->getTileAndPolyByRef( ref, &tile, &poly ) ) ) {
		center.Zero();
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
	wolf::nav::NavToQuake( nav, center.ToFloatPtr() );
}

int AreaForRef( const NavWorld &world, dtPolyRef ref ) {
	const int *area = nullptr;
	return world.refAreas.Get( ref, &area ) ? *area : 0;
}

dtPolyRef RefForArea( const NavWorld &world, int area ) {
	return area > 0 && area < world.areaRefs.Num() ? world.areaRefs[area] : 0;
}

unsigned char PolyArea( const NavWorld &world, int area ) {
	const dtPolyRef ref = RefForArea( world, area );
	const dtMeshTile *tile = nullptr;
	const dtPoly *poly = nullptr;
	if ( !ref || !world.mesh || dtStatusFailed( world.mesh->getTileAndPolyByRef( ref, &tile, &poly ) ) )
		return 0;
	return poly->getArea();
}

int NearestArea( NavWorld &world, const idVec3 &point, idVec3 *nearestEngine = nullptr ) {
	if ( !world.loaded || !world.query )
		return 0;
	float nav[3], nearest[3];
	wolf::nav::QuakeToNav( point.ToFloatPtr(), nav );
	const float extents[3]{ Max( 64.0f, world.header.agentRadius * 2 ),
							Max( 128.0f, world.header.agentHeight * 2 ),
							Max( 64.0f, world.header.agentRadius * 2 ) };
	dtPolyRef ref = 0;
	if ( dtStatusFailed( world.query->findNearestPoly( nav, extents, &world.filter, &ref, nearest ) ) || !ref )
		return 0;
	if ( nearestEngine )
		wolf::nav::NavToQuake( nearest, nearestEngine->ToFloatPtr() );
	return AreaForRef( world, ref );
}

float NavDistance( const idVec3 &a, const idVec3 &b ) {
	return ( a - b ).Length();
}

int FindPath( NavWorld &world, int startArea, const idVec3 &start, int goalArea, idList<dtPolyRef> &path ) {
	const dtPolyRef startRef = RefForArea( world, startArea ), goalRef = RefForArea( world, goalArea );
	if ( !startRef || !goalRef )
		return 0;
	float navStart[3], navGoal[3];
	wolf::nav::QuakeToNav( start.ToFloatPtr(), navStart );
	wolf::nav::QuakeToNav( world.centers[goalArea].ToFloatPtr(), navGoal );
	path.Resize( 4096 );
	int count = 0;
	if ( dtStatusFailed( world.query->findPath( startRef, goalRef, navStart, navGoal, &world.filter, path.Ptr(),
												&count, path.Num() ) ) )
		return 0;
	path.Resize( count );
	return count && path[count - 1] == goalRef ? count : 0;
}

int ReachBetween( const NavWorld &world, int fromArea, int toArea ) {
	if ( fromArea <= 0 || fromArea >= world.areaReaches.Num() )
		return 0;
	const idList<int> &areaReaches = world.areaReaches[fromArea];
	for ( int i = 0; i < areaReaches.Num(); ++i ) {
		const int reach = areaReaches[i];
		if ( world.reaches[reach].areanum == toArea )
			return reach;
	}
	return 0;
}

void BuildAreaIndex( NavWorld &world ) {
	world.areaRefs.Clear();
	world.areaRefs.Append( 0 );
	world.refAreas.Clear();
	world.centers.Clear();
	world.centers.Append( idVec3() );
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
			const int area = world.areaRefs.Num();
			world.areaRefs.Append( ref );
			world.refAreas.Set( ref, area );
			idVec3 center;
			PolyCenter( world, ref, center );
			world.centers.Append( center );
		}
	}
	world.areaReaches.Clear();
	world.areaReaches.SetNum( world.areaRefs.Num() );
	world.reaches.Clear();
	world.reaches.Append( aas_reachability_t{} );
	for ( int area = 1; area < world.areaRefs.Num(); ++area ) {
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
			idVec3 portal;
			wolf::nav::NavToQuake( navPortal, portal.ToFloatPtr() );
			float dx = world.centers[targetArea].x - world.centers[area].x;
			float dy = world.centers[targetArea].y - world.centers[area].y;
			const float length = idMath::Sqrt( dx * dx + dy * dy );
			if ( length > 0.001f ) {
				dx /= length;
				dy /= length;
			} else {
				idVec3 engineA, engineB;
				wolf::nav::NavToQuake( navA, engineA.ToFloatPtr() );
				wolf::nav::NavToQuake( navB, engineB.ToFloatPtr() );
				dx = -( engineB.y - engineA.y );
				dy = engineB.x - engineA.x;
				const float perpendicularLength = idMath::Sqrt( dx * dx + dy * dy );
				if ( perpendicularLength > 0.001f ) {
					dx /= perpendicularLength;
					dy /= perpendicularLength;
				}
			}
			constexpr float portalInset = 4.0f;
			reach.start.Set( portal.x - dx * portalInset, portal.y - dy * portalInset, portal.z + 1.0f );
			reach.end.Set( portal.x + dx * portalInset, portal.y + dy * portalInset, portal.z + 1.0f );
			reach.traveltime = AAS_AreaTravelTime( area, reach.start, reach.end );
			world.reaches.Append( reach );
			world.areaReaches[area].Append( world.reaches.Num() - 1 );
		}
	}
}

bool LoadWorldFile( NavWorld &world, const char *path ) {
	fileHandle_t file = 0;
	const int length = botimport.FS_FOpenFile( path, &file, FS_READ );
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

idStr BaseMapName( const char *mapname ) {
	idStr name = mapname ? mapname : "";
	const int slash = Max( name.Last( '/' ), name.Last( '\\' ) );
	if ( slash >= 0 )
		name = name.Mid( slash + 1, name.Length() - slash - 1 );
	const int dot = name.Last( '.' );
	if ( dot >= 0 )
		name.CapLength( dot );
	return name;
}

void ParseBspEntities() {
	bspEntities.Clear();
	const char *data = botimport.BSPEntityData ? botimport.BSPEntityData() : nullptr;
	if ( !data )
		return;
	const char *p = data;
	auto token = [&]() -> idStr {
		while ( *p && static_cast<unsigned char>( *p ) <= ' ' )
			++p;
		if ( *p == '{' || *p == '}' )
			return idStr( *p++ );
		if ( *p == '"' ) {
			++p;
			const char *start = p;
			while ( *p && *p != '"' )
				++p;
			idStr result( start, 0, static_cast<int>( p - start ) );
			if ( *p )
				++p;
			return result;
		}
		const char *start = p;
		while ( *p && static_cast<unsigned char>( *p ) > ' ' && *p != '{' && *p != '}' )
			++p;
		return idStr( start, 0, static_cast<int>( p - start ) );
	};
	for ( ;; ) {
		idStr open = token();
		if ( open.IsEmpty() )
			break;
		if ( open != "{" )
			continue;
		idDict entity;
		for ( ;; ) {
			idStr key = token();
			if ( key.IsEmpty() || key == "}" )
				break;
			idStr value = token();
			entity.Set( key.c_str(), value.c_str() );
		}
		bspEntities.Append( entity );
	}
}
} // namespace

int AAS_Setup() {
	navSetup = true;
	entities.SetNum( Max( 2048, botlibglobals.maxentities ) );
	for ( int i = 0; i < entities.Num(); ++i )
		entities[i] = aas_entityinfo_t{};
	return BLERR_NOERROR;
}
void AAS_Shutdown() {
	for ( auto &world : worlds )
		ClearWorld( world );
	entities.Clear();
	bspEntities.Clear();
	navSetup = false;
}
int AAS_LoadMap( const char *mapname ) {
	for ( auto &world : worlds )
		ClearWorld( world );
	const idStr base = BaseMapName( mapname );
	int loaded = 0;
	for ( int i = 0; i < MAX_AAS_WORLDS; ++i )
		loaded += LoadWorldFile( worlds[i], idStr( "maps/" ) + base + "_b" + idStr( i ) + ".nav" ) ? 1 : 0;
	ParseBspEntities();
	currentWorld = 0;
	if ( !loaded ) {
		botimport.Print( PRT_ERROR, "No navmesh found for maps/%s_b[01].nav\n", base.c_str() );
		return BLERR_NOAASFILE;
	}
	return BLERR_NOERROR;
}
int AAS_StartFrame( float time ) {
	navTime = time;
	for ( int i = 0; i < entities.Num(); ++i )
		entities[i].valid = qfalse;
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
	idStr::snPrintf( model, sizeof( model ), "*%d", index );
	return model;
}
int AAS_IndexFromModel( char *modelname ) {
	return modelname && modelname[0] == '*' ? atoi( modelname + 1 ) : 0;
}

int AAS_UpdateEntity( int entnum, bot_entitystate_t *state ) {
	if ( entnum < 0 || entnum >= entities.Num() || !state )
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
	info.lastvisorigin = info.origin;
	info.origin = state->origin;
	info.angles = state->angles;
	info.old_origin = state->old_origin;
	info.mins = state->mins;
	info.maxs = state->maxs;
	return BLERR_NOERROR;
}
void AAS_EntityInfo( int entnum, aas_entityinfo_t *info ) {
	if ( !info )
		return;
	if ( entnum >= 0 && entnum < entities.Num() )
		*info = entities[entnum];
	else
		memset( info, 0, sizeof( *info ) );
}
int AAS_NextEntity( int entnum ) {
	for ( int i = entnum + 1; i < entities.Num(); ++i )
		if ( entities[i].valid )
			return i;
	return 0;
}
int AAS_EntityType( int entnum ) {
	return entnum >= 0 && entnum < entities.Num() ? entities[entnum].type : 0;
}
int AAS_EntityModelindex( int entnum ) {
	return entnum >= 0 && entnum < entities.Num() ? entities[entnum].modelindex : 0;
}
int AAS_EntityModelNum( int entnum ) {
	return AAS_EntityModelindex( entnum );
}
int AAS_OriginOfEntityWithModelNum( int modelnum, idVec3 &origin ) {
	for ( int i = 0; i < entities.Num(); ++i ) {
		const aas_entityinfo_t &entity = entities[i];
		if ( entity.valid && entity.modelindex == modelnum ) {
			origin = entity.origin;
			return qtrue;
		}
	}
	return qfalse;
}
int AAS_IsEntityInArea( int ignore1, int ignore2, int area ) {
	for ( int i = 0; i < entities.Num(); ++i ) {
		const aas_entityinfo_t &entity = entities[i];
		if ( entity.valid && entity.number != ignore1 && entity.number != ignore2 &&
			 AAS_PointAreaNum( entity.origin ) == area )
			return qtrue;
	}
	return qfalse;
}

bsp_trace_t AAS_Trace( const idVec3 &start, const idVec3 *mins, const idVec3 *maxs, const idVec3 &end, int passent, int contentmask ) {
	bsp_trace_t trace{};
	botimport.Trace( &trace, start, mins, maxs, end, passent, contentmask );
	return trace;
}
int AAS_PointContents( const idVec3 &point ) {
	return botimport.PointContents( point );
}
qboolean AAS_inPVS( const idVec3 &p1, const idVec3 &p2 ) {
	return botimport.inPVS( p1, p2 );
}
qboolean AAS_AreasConnected( int area1, int area2 ) {
	NavWorld &world = World();
	if ( !RefForArea( world, area1 ) || !RefForArea( world, area2 ) )
		return qfalse;
	idList<dtPolyRef> path;
	return FindPath( world, area1, world.centers[area1], area2, path ) ? qtrue : qfalse;
}
void AAS_BSPModelMinsMaxsOrigin( int modelnum, const idVec3 &angles, idVec3 &mins, idVec3 &maxs, idVec3 *origin ) {
	botimport.BSPModelMinsMaxsOrigin( modelnum, angles, mins, maxs, origin );
}
int AAS_NextBSPEntity( int ent ) {
	const int next = ent + 1;
	return next > 0 && next <= bspEntities.Num() ? next : 0;
}
int AAS_ValueForBSPEpairKey( int ent, char *key, char *value, int size ) {
	if ( value && size )
		value[0] = 0;
	if ( ent <= 0 || ent > bspEntities.Num() || !key || !value || size <= 0 )
		return qfalse;
	const idKeyValue *entry = bspEntities[ent - 1].FindKey( key );
	if ( !entry )
		return qfalse;
	strncpy( value, entry->GetValue().c_str(), size - 1 );
	value[size - 1] = 0;
	return qtrue;
}
int AAS_VectorForBSPEpairKey( int ent, char *key, idVec3 &v ) {
	char value[MAX_EPAIRKEY]{};
	if ( !AAS_ValueForBSPEpairKey( ent, key, value, sizeof( value ) ) )
		return qfalse;
	return sscanf( value, "%f %f %f", &v[0], &v[1], &v[2] ) == 3;
}
int AAS_FloatForBSPEpairKey( int ent, char *key, float *value ) {
	char text[MAX_EPAIRKEY]{};
	if ( !AAS_ValueForBSPEpairKey( ent, key, text, sizeof( text ) ) )
		return qfalse;
	*value = strtof( text, nullptr );
	return qtrue;
}
int AAS_IntForBSPEpairKey( int ent, char *key, int *value ) {
	char text[MAX_EPAIRKEY]{};
	if ( !AAS_ValueForBSPEpairKey( ent, key, text, sizeof( text ) ) )
		return qfalse;
	*value = atoi( text );
	return qtrue;
}

void AAS_PresenceTypeBoundingBox( int presencetype, idVec3 &mins, idVec3 &maxs ) {
	const auto &h = World().header;
	const float radius = h.agentRadius, height = presencetype == PRESENCE_CROUCH ? h.crouchHeight : h.agentHeight;
	mins.Set( -radius, -radius, -24 );
	maxs.Set( radius, radius, height - 24 );
}
int AAS_PointAreaNum( const idVec3 &point ) {
	return NearestArea( World(), point );
}
int AAS_TraceAreas( const idVec3 &start, const idVec3 &end, int *areas, idVec3 *points, int maxareas ) {
	if ( maxareas <= 0 )
		return 0;
	int count = 0, previous = 0;
	for ( int sample = 0; sample <= 64 && count < maxareas; ++sample ) {
		const float t = sample / 64.0f;
		idVec3 p;
		for ( int i = 0; i < 3; ++i )
			p[i] = start[i] + ( end[i] - start[i] ) * t;
		const int area = AAS_PointAreaNum( p );
		if ( area && area != previous ) {
			areas[count] = area;
			if ( points )
				points[count] = p;
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
int AAS_PointPresenceType( const idVec3 &point ) {
	return AAS_AreaPresenceType( AAS_PointAreaNum( point ) );
}
int AAS_AreaReachability( int areanum ) {
	const dtPolyRef ref = RefForArea( World(), areanum );
	unsigned short flags = 0;
	return ref && dtStatusSucceed( World().mesh->getPolyFlags( ref, &flags ) ) && !( flags & wolf::nav::PolyDisabled );
}
int AAS_BestReachableArea( const idVec3 &origin, const idVec3 &, const idVec3 &, idVec3 &goalorigin ) {
	const int area = NearestArea( World(), origin, &goalorigin );
	if ( !area )
		goalorigin = origin;
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
	if ( area <= 0 || area >= World().areaReaches.Num() )
		return 0;
	const idList<int> &list = World().areaReaches[area];
	if ( list.Num() == 0 )
		return 0;
	if ( !reach )
		return list[0];
	const int index = list.FindIndex( reach );
	return index >= 0 && index + 1 < list.Num() ? list[index + 1] : 0;
}
void AAS_ReachabilityFromNum( int num, aas_reachability_t *reach ) {
	if ( !reach )
		return;
	if ( num > 0 && num < World().reaches.Num() )
		*reach = World().reaches[num];
	else
		memset( reach, 0, sizeof( *reach ) );
}
unsigned short AAS_AreaTravelTime( int, const idVec3 &start, const idVec3 &end ) {
	const float speed = Max( 1.0f, World().header.maxWalkVelocity );
	return static_cast<unsigned short>(
		Min( 65535.0f, Max( 1.0f, NavDistance( start, end ) * 100.0f / speed ) ) );
}
int AAS_AreaRouteToGoalArea( int area, const idVec3 &origin, int goal, int, int *travelTime, int *reachNum ) {
	if ( travelTime )
		*travelTime = 0;
	if ( reachNum )
		*reachNum = 0;
	idList<dtPolyRef> path;
	if ( !FindPath( World(), area, origin, goal, path ) )
		return qfalse;
	float total = NavDistance( origin, World().centers[area] );
	for ( int i = 1; i < path.Num(); ++i )
		total += NavDistance( World().centers[AreaForRef( World(), path[i - 1] )],
							  World().centers[AreaForRef( World(), path[i] )] );
	if ( travelTime )
		*travelTime = static_cast<int>( total * 100 / Max( 1.0f, World().header.maxWalkVelocity ) );
	if ( reachNum && path.Num() > 1 )
		*reachNum = ReachBetween( World(), area, AreaForRef( World(), path[1] ) );
	return qtrue;
}
int AAS_AreaTravelTimeToGoalArea( int area, const idVec3 &origin, int goal, int flags ) {
	int time = 0, reach = 0;
	return AAS_AreaRouteToGoalArea( area, origin, goal, flags, &time, &reach ) ? Max( 1, time ) : 0;
}
int AAS_AreaTravelTimeToGoalAreaCheckLoop( int area, const idVec3 &origin, int goal, int flags, int loop ) {
	return area == loop ? 0 : AAS_AreaTravelTimeToGoalArea( area, origin, goal, flags );
}
int AAS_RandomGoalArea( int area, int, int *goalArea, idVec3 &goalOrigin ) {
	if ( World().areaRefs.Num() <= 1 )
		return qfalse;
	int chosen = 1 + rand() % ( World().areaRefs.Num() - 1 );
	if ( chosen == area && World().areaRefs.Num() > 2 )
		chosen = chosen % ( World().areaRefs.Num() - 1 ) + 1;
	*goalArea = chosen;
	goalOrigin = World().centers[chosen];
	return qtrue;
}

int AAS_OnGround( const idVec3 &origin, int presencetype, int passent ) {
	idVec3 mins, maxs, end;
	AAS_PresenceTypeBoundingBox( presencetype, mins, maxs );
	end = origin;
	end[2] -= 3;
	return AAS_Trace( origin, &mins, &maxs, end, passent, CONTENTS_SOLID | CONTENTS_PLAYERCLIP ).fraction < 1.0f;
}
int AAS_Swimming( const idVec3 &origin ) {
	return ( AAS_PointContents( origin ) & CONTENTS_WATER ) != 0;
}
void AAS_JumpReachRunStart( aas_reachability_t *reach, idVec3 &runstart ) {
	if ( reach )
		runstart = reach->start;
	else
		runstart.Zero();
}
int AAS_AgainstLadder( const idVec3 &, int ) {
	return qfalse;
}
int AAS_HorizontalVelocityForJump( float zvel, const idVec3 &start, const idVec3 &end, float *velocity ) {
	const float gravity = Max( 1.0f, World().header.gravity );
	const float dz = end[2] - start[2];
	const float disc = zvel * zvel - 2 * gravity * dz;
	if ( disc < 0 )
		return qfalse;
	const float time = ( zvel + idMath::Sqrt( disc ) ) / gravity;
	if ( time <= 0 )
		return qfalse;
	*velocity =
		idMath::Sqrt( ( end[0] - start[0] ) * ( end[0] - start[0] ) + ( end[1] - start[1] ) * ( end[1] - start[1] ) ) /
		time;
	return qtrue;
}
int AAS_DropToFloor( idVec3 &origin, const idVec3 &, const idVec3 & ) {
	idVec3 nearest;
	if ( !NearestArea( World(), origin, &nearest ) )
		return qfalse;
	origin = nearest;
	return qtrue;
}
aas_trace_t AAS_TraceClientBBox( const idVec3 &start, const idVec3 &end, int presence, int passent ) {
	idVec3 mins, maxs;
	AAS_PresenceTypeBoundingBox( presence, mins, maxs );
	bsp_trace_t bsp = AAS_Trace( start, &mins, &maxs, end, passent, CONTENTS_SOLID | CONTENTS_PLAYERCLIP );
	aas_trace_t result{};
	result.startsolid = bsp.startsolid;
	result.fraction = bsp.fraction;
	result.endpos = bsp.endpos;
	result.ent = bsp.ent;
	result.lastarea = AAS_PointAreaNum( bsp.endpos );
	result.area = result.lastarea;
	result.planenum = bsp.sidenum;
	return result;
}
int AAS_PredictClientMovement( aas_clientmove_t *move, int entnum, const idVec3 &origin, int presence, int onground,
							   const idVec3 &velocity, const idVec3 &cmdmove, int cmdframes, int maxframes, float dt, int stopEvent,
							   int stopArea, int ) {
	if ( !move )
		return qfalse;
	memset( move, 0, sizeof( *move ) );
	idVec3 pos, vel;
	pos = origin;
	vel = velocity;
	for ( int frame = 0; frame < maxframes; ++frame ) {
		if ( frame < cmdframes ) {
			vel[0] += cmdmove[0] * dt;
			vel[1] += cmdmove[1] * dt;
			vel[2] += cmdmove[2] * dt;
		}
		if ( !onground )
			vel[2] -= World().header.gravity * dt;
		idVec3 end{ pos[0] + vel[0] * dt, pos[1] + vel[1] * dt, pos[2] + vel[2] * dt };
		move->trace = AAS_TraceClientBBox( pos, end, presence, entnum );
		pos = move->trace.endpos;
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
	move->endpos = pos;
	move->velocity = vel;
	move->presencetype = presence;
	move->endcontents = static_cast<float>( AAS_PointContents( pos ) );
	return qtrue;
}

void AAS_SetAASBlockingEntity( const idVec3 &mins, const idVec3 &maxs, qboolean blocking ) {
	NavWorld &world = World();
	if ( !world.loaded )
		return;
	for ( int area = 1; area < world.centers.Num(); ++area ) {
		const idVec3 &point = world.centers[area];
		if ( point.x >= mins.x && point.x <= maxs.x && point.y >= mins.y && point.y <= maxs.y &&
			 point.z >= mins.z && point.z <= maxs.z )
			world.mesh->setPolyFlags( world.areaRefs[area], blocking ? wolf::nav::PolyDisabled : wolf::nav::PolyWalk );
	}
}
void AAS_RT_ShowRoute( const idVec3 &, int, int ) {
}
qboolean AAS_RT_GetHidePos( const idVec3 &, int, int, const idVec3 &, int, int, idVec3 & ) {
	return qfalse;
}
int AAS_FindAttackSpotWithinRange( int, int, int, float, int, float * ) {
	return qfalse;
}
qboolean AAS_GetRouteFirstVisPos( const idVec3 &src, const idVec3 &dest, int, idVec3 &result ) {
	NavWorld &world = World();
	const int startArea = AAS_PointAreaNum( src ), goalArea = AAS_PointAreaNum( dest );
	idList<dtPolyRef> path;
	if ( !FindPath( world, startArea, src, goalArea, path ) )
		return qfalse;
	result = path.Num() > 1 ? world.centers[AreaForRef( world, path[1] )] : dest;
	return qtrue;
}

void AAS_ClearShownDebugLines() {
}
void AAS_ClearShownPolygons() {
}
void AAS_DebugLine( const idVec3 &start, const idVec3 &end, int color ) {
	const int line = botimport.DebugLineCreate();
	botimport.DebugLineShow( line, start, end, color );
}
void AAS_DrawCross( const idVec3 &origin, float size, int color ) {
	for ( int axis = 0; axis < 3; ++axis ) {
		idVec3 a, b;
		a = origin;
		b = origin;
		a[axis] -= size;
		b[axis] += size;
		AAS_DebugLine( a, b, color );
	}
}
void AAS_DrawPermanentCross( const idVec3 &origin, float size, int color ) {
	AAS_DrawCross( origin, size, color );
}
void AAS_DrawPlaneCross( const idVec3 &point, const idVec3 &, float, int, int color ) {
	AAS_DrawCross( point, 8, color );
}
void AAS_ShowBoundingBox( const idVec3 &, const idVec3 &, const idVec3 & ) {
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
