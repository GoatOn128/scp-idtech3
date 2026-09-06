/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

// Event/map adapters only. Appearance and animation live in effects/wolf/*.efx.
#include "FxScheduler.h"
#include "cg_fx.h"

qboolean CG_ParticleLODCheck( void );

static void CG_PlayParticle( const char *name, const vec3_t origin, const FxParameters &p ) {
    if ( !CG_ParticleLODCheck() ) { return; }
    CG_FxPlayParameters( name, origin, vec3_origin, p );
}

qboolean CG_ParticleLODCheck( void ) {
    return cg_particleLOD.integer <= 1 || rand() % cg_particleLOD.integer == 0 ? qtrue : qfalse;
}

void CG_ParticleSnowFlurry( qhandle_t shader, centity_t *cent ) {
    FxParameters p;
    p.shader = shader; p.life = cent->currentState.time; p.fadeTime = cent->currentState.time2;
    VectorCopy( cent->currentState.angles, p.velocity );
    CG_PlayParticle( "wolf/snow_flurry", cent->currentState.origin, p );
}

static void CG_FxWeather( const char *name, qhandle_t shader, vec3_t origin, vec3_t end, float spread, int group ) {
    FxParameters p;
    p.shader = shader; p.group = group; p.spread = spread;
    p.heightMin = origin[2] < end[2] ? origin[2] : end[2];
    p.heightMax = origin[2] > end[2] ? origin[2] : end[2];
    CG_PlayParticle( name, origin, p );
}

void CG_ParticleSnow( qhandle_t shader, vec3_t origin, vec3_t end, int turbulent, float spread, int group ) {
    CG_FxWeather( turbulent ? "wolf/snow_turbulent" : "wolf/snow", shader, origin, end, spread, group );
}
void CG_ParticleBubble( qhandle_t shader, vec3_t origin, vec3_t end, int turbulent, float spread, int group ) {
    CG_FxWeather( turbulent ? "wolf/bubbles_turbulent" : "wolf/bubbles", shader, origin, end, spread, group );
}
void CG_SnowLink( centity_t *cent, qboolean visible ) {
    vec3_t value = { static_cast<float>(visible), 0, 0 };
    FX_GroupMessage( cent->currentState.frame, 0, value );
}

void CG_ParticleSmoke( qhandle_t shader, centity_t *cent ) {
    FxParameters p;
    p.shader = shader; p.life = cent->currentState.time; p.fadeTime = cent->currentState.time2;
    p.size = cent->currentState.angles2[0]; p.sizeEnd = cent->currentState.angles2[1];
    p.speed = cent->currentState.angles2[2];
    VectorCopy( cent->currentState.origin2, p.velocity );
    const char *effect = "wolf/smoke_black";
    switch ( cent->currentState.density ) {
        case 1: effect = "wolf/smoke_small"; break;
        case 2: effect = "wolf/smoke_rotating"; break;
        case 3: effect = "wolf/smoke_cannon"; break;
        case 4: effect = "wolf/smoke_white"; break;
        case 5: effect = "wolf/smoke_gas"; break;
    }
    if ( cent->currentState.frame == 1 ) { p.verticalSign = -1; }
    CG_PlayParticle( effect, cent->currentState.origin, p );
}

void CG_ParticleBulletDebris( vec3_t origin, vec3_t velocity, int duration ) {
    FxParameters p; p.life = duration; VectorCopy( velocity, p.velocity );
    CG_PlayParticle( "wolf/bullet_debris", origin, p );
}
void CG_ParticleDirtBulletDebris( vec3_t origin, vec3_t velocity, int duration ) {
    FxParameters p; p.life = duration; VectorCopy( velocity, p.velocity );
    CG_PlayParticle( "wolf/dirt_debris", origin, p );
}
void CG_ParticleDirtBulletDebris_Core( vec3_t origin, vec3_t velocity, int duration,
        float width, float height, float alpha, char *shader ) {
    FxParameters p; p.life = duration; p.size = width; p.aspect = width > 0 ? height / width : 1; p.alpha = alpha;
    p.shader = trap_R_RegisterShader( shader ); VectorCopy( velocity, p.velocity );
    CG_PlayParticle( "wolf/dirt_core", origin, p );
}
void CG_ParticleExplosion( char *animation, vec3_t origin, vec3_t velocity, int duration, int size, int sizeEnd ) {
    if ( !animation || strstr(animation, "..") || strchr(animation, '/') || strchr(animation, '\\') ) { return; }
    char effect[MAX_QPATH]; Com_sprintf( effect, sizeof(effect), "wolf/%s", animation );
    FxParameters p; p.life = abs(duration); p.size = size; p.sizeEnd = sizeEnd;
    VectorCopy( velocity, p.velocity ); CG_PlayParticle( effect, origin, p );
}
void CG_AddParticleShrapnel( fxEntity_t *entity ) {}

void CG_ParticleBat( centity_t *cent ) {
    FxParameters p; p.group = cent->currentState.frame;
    CG_PlayParticle( "wolf/bat_single", cent->lerpOrigin, p );
}
void CG_ParticleBats( qhandle_t shader, centity_t *cent ) {
    FxParameters p; p.shader = shader; p.group = cent->currentState.frame;
    VectorScale( cent->currentState.angles, cent->currentState.time, p.velocity );
    CG_PlayParticle( "wolf/bats", cent->currentState.origin, p );
}
void CG_BatsUpdatePosition( centity_t *cent ) {
    vec3_t velocity; VectorScale( cent->currentState.angles, cent->currentState.time, velocity );
    FX_GroupMessage( cent->currentState.frame, 1, velocity );
}

void CG_ParticleImpactSmokePuffExtended( qhandle_t shader, vec3_t origin, vec3_t direction, int size,
        int life, int velocity, int acceleration, int maxroll, float alpha ) {
    FxParameters p; p.shader = shader; p.life = life; p.size = size; p.alpha = alpha;
    p.speed = velocity; p.acceleration = acceleration;
    CG_FxPlayParameters( "wolf/impact_smoke", origin, direction, p );
}
void CG_ParticleImpactSmokePuff( qhandle_t shader, vec3_t origin ) {
    FxParameters p; p.shader = shader;
    CG_PlayParticle( "wolf/impact_smoke_default", origin, p );
}
void CG_Particle_Bleed( qhandle_t shader, vec3_t origin, vec3_t direction, int flesh, int duration ) {
	if ( !cg_blood.integer ) { return; }
    FxParameters p; p.shader = shader; p.life = duration;
    CG_PlayParticle( flesh ? "wolf/bleed_undead" : "wolf/bleed", origin, p );
}
void CG_Particle_OilParticle( qhandle_t shader, vec3_t origin, vec3_t direction, int time, int group ) {
    FxParameters p; p.shader = shader; p.group = group;
    float ratio = cg.time + time > 0 ? static_cast<float>(time) / (cg.time + time) : 0;
    VectorScale( direction, ratio, p.velocity ); CG_PlayParticle( "wolf/oil_drop", origin, p );
}
void CG_Particle_OilSlick( qhandle_t shader, centity_t *cent ) {
    FxParameters p; p.shader = shader; p.group = cent->currentState.density;
    if ( cent->currentState.angles2[2] > 0 ) { p.life = cent->currentState.angles2[2]; }
    if ( cent->currentState.angles2[0] > 0 ) { p.size = cent->currentState.angles2[0]; }
    if ( cent->currentState.angles2[1] > 0 ) { p.sizeEnd = cent->currentState.angles2[1]; }
    CG_PlayParticle( "wolf/oil_slick", cent->currentState.origin, p );
}
void CG_OilSlickRemove( centity_t *cent ) {
    FX_GroupMessage( cent->currentState.density, 2, vec3_origin );
}
void CG_BloodPool( fxEntity_t *entity, qhandle_t shader, trace_t *trace ) {
    if ( !cg_blood.integer || trace->entityNum != ENTITYNUM_WORLD || trace->plane.normal[2] < 0.9f ) { return; }
    FxParameters p; p.shader = shader;
    CG_FxPlayParameters( "wolf/blood_pool", trace->endpos, trace->plane.normal, p );
}
void CG_ParticleBloodCloud( centity_t *cent, vec3_t origin, vec3_t direction ) {
    if ( !cg_blood.integer ) { return; }
    FxParameters p;
    const char *name = cent->currentState.aiChar == AICHAR_ZOMBIE ? "wolf/blood_cloud_undead" : "wolf/blood_cloud";
    if ( cent->currentState.aiChar == AICHAR_HELGA || cent->currentState.aiChar == AICHAR_HEINRICH ) { name = "wolf/blood_cloud_boss"; }
    p.pathLength = VectorLength( direction );
    char effect[MAX_QPATH];
    Com_sprintf( effect, sizeof(effect), "%s%s", name, cent->currentState.density ? "_large" : "" );
    CG_PlayParticle( effect, origin, p );
}
void CG_ParticleBloodCloudZombie( centity_t *cent, vec3_t origin, vec3_t direction ) {
    if ( !cg_blood.integer ) { return; }
    FxParameters p;
    p.pathLength = VectorLength( direction );
    const char *name = p.pathLength > 0 ? "wolf/blood_zombie" : "wolf/blood_zombie_small";
    char effect[MAX_QPATH];
    Com_sprintf( effect, sizeof(effect), "%s%s", name, cent->currentState.density ? "_large" : "" );
    CG_PlayParticle( effect, origin, p );
}
void CG_ParticleSparks( vec3_t origin, vec3_t velocity, int duration, float x, float y, float speed ) {
    FxParameters p; p.life = duration; p.speed = speed; p.spreadAxes[0] = x; p.spreadAxes[1] = y; VectorCopy( velocity, p.velocity );
    CG_PlayParticle( "wolf/sparks", origin, p );
}
void CG_ParticleDust( centity_t *cent, vec3_t origin, vec3_t direction ) {
    FxParameters p;
    CG_FxPlayParameters( cent->currentState.density ? "wolf/dust_large" : "wolf/dust", origin, direction, p );
}
void CG_ParticleMisc( qhandle_t shader, vec3_t origin, int size, int duration, float alpha ) {
    FxParameters p; p.shader = shader; p.size = size; p.alpha = alpha; p.life = duration > 0 ? duration : 1;
    CG_PlayParticle( "wolf/misc", origin, p );
}

int CG_NewParticleArea( int num ) {
	// const char *str;
	char *str;
	char *token;
	int type;
	vec3_t origin, origin2;
	int i;
	float range = 0;
	int turb;
	int numparticles;
	int snum;

	str = (char *) CG_ConfigString( num );
	if ( !str[0] ) {
		return ( 0 );
	}

	// returns type 128 64 or 32
	token = COM_Parse( &str );
	type = atoi( token );

	if ( type == 1 ) {
		range = 128;
	} else if ( type == 2 ) {
		range = 64;
	} else if ( type == 3 ) {
		range = 32;
	} else if ( type == 0 ) {
		range = 256;
	} else if ( type == 4 ) {
		range = 8;
	} else if ( type == 5 ) {
		range = 16;
	} else if ( type == 6 ) {
		range = 32;
	} else if ( type == 7 ) {
		range = 64;
	}


	for ( i = 0; i < 3; i++ )
	{
		token = COM_Parse( &str );
		origin[i] = atof( token );
	}

	for ( i = 0; i < 3; i++ )
	{
		token = COM_Parse( &str );
		origin2[i] = atof( token );
	}

	token = COM_Parse( &str );
	numparticles = atoi( token );

	token = COM_Parse( &str );
	turb = atoi( token );

	token = COM_Parse( &str );
	snum = atoi( token );

	for ( i = 0; i < numparticles; i++ )
	{
		if ( type >= 4 ) {
			CG_ParticleBubble( cgs.media.waterBubbleShader, origin, origin2, turb, range, snum );
		} else {
			CG_ParticleSnow( cgs.media.snowShader, origin, origin2, turb, range, snum );
		}
	}

	return ( 1 );
}


