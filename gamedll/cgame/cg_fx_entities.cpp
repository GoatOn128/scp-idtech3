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


// Wolf-specific debris, shell, spirit and emitter behavior hosted by the FX runtime.
#include "FxScheduler.h"
#include "cg_fx.h"

static bool CG_FxUpdateEntity( fxEntity_t *entity );

class CWolfEntityEffect : public CEffect, public fxEntity_t {
public:
    CWolfEntityEffect() { memset( static_cast<fxEntity_t *>( this ), 0, sizeof( fxEntity_t ) ); }
    bool Update() override {
        return CG_FxUpdateEntity( this );
    }
};

fxEntity_t *CG_FxSpawnModel( const char *name ) {
	const CPrimitiveTemplate *definition = theFxScheduler.Definition( name );
	if ( !definition || definition->mType != Controller ) { CG_Error( "FX: missing model controller %s", name ); }
    CWolfEntityEffect *effect = new CWolfEntityEffect;
    CEffect *primitive = effect;
    FX_AddPrimitive( &primitive, 24 * 60 * 60 * 1000 );
    return effect;
}

void CG_FxFreeEntity( fxEntity_t *entity ) {
    // Destruction is deferred until Update returns, so impact callbacks remain valid.
    entity->endTime = 0;
}

void CG_BloodTrail( fxEntity_t *le ) {
	int t;
	int t2;
	int step;
	vec3_t newOrigin;

#ifndef BLOOD_PARTICLE_TRAIL
	static vec3_t col = {1,1,1};
#endif

	centity_t   *cent;
	cent = &cg_entities[le->ownerNum];

	if ( !cg_blood.integer ) {
		return;
	}

	// step = 150;
#ifdef BLOOD_PARTICLE_TRAIL
	step = CG_FxInt( "wolf/controller_bloodtrail", "step" );
#else
	// time it takes to move 3 units
	step = ( 1000 * 3 ) / VectorLength( le->pos.trDelta );
#endif

	if ( cent && cent->currentState.aiChar == AICHAR_ZOMBIE ) {
		step = CG_FxInt( "wolf/controller_bloodtrail", "step_2" );
	}

	t = step * ( ( cg.time - cg.frametime + step ) / step );
	t2 = step * ( cg.time / step );

	for ( ; t <= t2; t += step ) {
		BG_EvaluateTrajectory( &le->pos, t, newOrigin );

#ifdef BLOOD_PARTICLE_TRAIL
		CG_Particle_Bleed( CG_FxMaterial( "wolf/controller_bloodtrail", "smokePuffShader" ), newOrigin, vec3_origin, CG_FxInt( "wolf/controller_bloodtrail", "Particle_Bleed_value" ), CG_FxInt( "wolf/controller_bloodtrail", "tuning" ) + rand() % CG_FxInt( "wolf/controller_bloodtrail", "tuning_2" ) );
#else


		if ( cent && cent->currentState.aiChar == AICHAR_ZOMBIE ) {
			CG_Particle_Bleed( CG_FxMaterial( "wolf/controller_bloodtrail", "smokePuffShader" ), newOrigin, vec3_origin, CG_FxInt( "wolf/controller_bloodtrail", "Particle_Bleed_value_2" ), CG_FxInt( "wolf/controller_bloodtrail", "tuning_3" ) + rand() % CG_FxInt( "wolf/controller_bloodtrail", "tuning_4" ) );
		} else {
			// Ridah, blood trail using trail code (should be faster since we don't have to spawn as many)
			le->headJuncIndex = CG_FxTrail( "wolf/trail_gib_blood", le->headJuncIndex, newOrigin,
			FxTrailInputs().Time( t ) );
		}
#endif

	}
}


/*
================
CG_FragmentBounceMark
================
*/
void CG_FragmentBounceMark( fxEntity_t *le, trace_t *trace ) {
	int radius;

	if ( le->leMarkType == LEMT_BLOOD ) {
		static int lastBloodMark;

		// don't drop too many blood marks
		if ( !( lastBloodMark > cg.time || lastBloodMark > cg.time - CG_FxInt( "wolf/controller_fragmentbouncemark", "tuning" ) ) ) {
			radius = CG_FxInt( "wolf/controller_fragmentbouncemark", "radius" ) + ( rand() & CG_FxInt( "wolf/controller_fragmentbouncemark", "radius_2" ) );
			CG_ImpactMark( cgs.media.bloodDotShaders[rand() % 5], trace->endpos, trace->plane.normal, random() * CG_FxInt( "wolf/controller_fragmentbouncemark", "tuning_2" ),
						   CG_FxInt( "wolf/controller_fragmentbouncemark", "ImpactMark_value" ),CG_FxInt( "wolf/controller_fragmentbouncemark", "ImpactMark_value_2" ),CG_FxInt( "wolf/controller_fragmentbouncemark", "ImpactMark_value_3" ),CG_FxInt( "wolf/controller_fragmentbouncemark", "ImpactMark_value_4" ), qtrue, radius, qfalse, cg_bloodTime.integer * CG_FxInt( "wolf/controller_fragmentbouncemark", "ImpactMark_value_5" ) );

			lastBloodMark = cg.time;
		}
	}

	// don't allow a fragment to make multiple marks, or they
	// pile up while settling
	le->leMarkType = LEMT_NONE;
}

/*
================
CG_FragmentBounceSound
================
*/
void CG_FragmentBounceSound( fxEntity_t *le, trace_t *trace ) {
	if ( le->leBounceSoundType == LEBS_BLOOD ) {
		// half the gibs will make splat sounds
		if ( rand() & 1 ) {
			int r = rand() & 3;
			sfxHandle_t s;

			if ( r < 2 ) {
				s = cgs.media.gibBounce1Sound;
			} else if ( r == 2 ) {
				s = cgs.media.gibBounce2Sound;
			} else {
				s = cgs.media.gibBounce3Sound;
			}
			trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO, s );
		}
	} else if ( le->leBounceSoundType == LEBS_BRASS ) {

//----(SA) added
	} else if ( le->leBounceSoundType == LEBS_ROCK ) {
		// half the hits will make thunk sounds	(this is just to start since we don't even have the sound yet... (SA))
		if ( rand() & 1 ) {
			int r = rand() & 3;
			sfxHandle_t s;

			if ( r < 2 ) {
				s = cgs.media.debBounce1Sound;
			} else if ( r == 2 ) {
				s = cgs.media.debBounce2Sound;
			} else {
				s = cgs.media.debBounce3Sound;
			}
			trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO, s );
		}
//----(SA) end

	} else if ( le->leBounceSoundType == LEBS_BONE ) {

		trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.boneBounceSound );

	}

	// don't allow a fragment to make multiple bounce sounds,
	// or it gets too noisy as they settle
	le->leBounceSoundType = LEBS_NONE;
}


/*
================
CG_ReflectVelocity
================
*/
void CG_ReflectVelocity( fxEntity_t *le, trace_t *trace ) {
	vec3_t velocity;
	float dot;
	int hitTime;

	// reflect the velocity on the trace plane
	hitTime = cg.time - cg.frametime + cg.frametime * trace->fraction;
	BG_EvaluateTrajectoryDelta( &le->pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2 * dot, trace->plane.normal, le->pos.trDelta );

	VectorScale( le->pos.trDelta, le->bounceFactor, le->pos.trDelta );

	VectorCopy( trace->endpos, le->pos.trBase );
	le->pos.trTime = cg.time;


	// check for stop, making sure that even on low FPS systems it doesn't bobble

	if ( le->leMarkType == LEMT_BLOOD && trace->startsolid ) {
		//centity_t *cent;
		//cent = &cg_entities[trace->entityNum];
		//if (cent && cent->currentState.apos.trType != TR_STATIONARY)
		//	le->pos.trType = TR_STATIONARY;
	} else if ( trace->allsolid ||
				( trace->plane.normal[2] > 0 &&
				  ( le->pos.trDelta[2] < 40 || le->pos.trDelta[2] < -cg.frametime * le->pos.trDelta[2] ) ) ) {

//----(SA)	if it's a fragment and it's not resting on the world...
//			if(le->leType == LE_DEBRIS && trace->entityNum < (MAX_ENTITIES - 1))
		if ( le->leType == LE_FRAGMENT && trace->entityNum < ( MAX_ENTITIES - 1 ) ) {
			le->pos.trType = TR_GRAVITY_PAUSED;
		} else
		{
			le->pos.trType = TR_STATIONARY;
		}
	} else {

	}
}


//----(SA)	added

/*
==============
CG_AddEmitter
==============
*/
void CG_AddEmitter( fxEntity_t *le ) {
	vec3_t dir;
	int nextTime = CG_FxInt( "wolf/controller_addemitter", "nextTime" );

	if ( le->breakCount > cg.time ) {  // using 'breakCount' for 'wait'
		return;
	}

	if ( cg_paused.integer ) { // don't add while paused
		return;
	}

	// TODO: look up particle script and use proper effect rather than this check
	//if(water){}
	//else if(oil) {}
	//else if(steam) {}
	//else if(wine) {}

	switch ( le->headJuncIndex ) {
//		CG_ParticleImpactSmokePuff (cgs.media.oilParticle, le->pos.trBase);
//		CG_Particle_Bleed(cgs.media.smokePuffShader, le->pos.trBase, dir, 0, 200);

	case 1:     // oil
		VectorScale( le->angles.trBase, le->radius, dir );
		CG_Particle_OilParticle( CG_FxMaterial( "wolf/controller_addemitter", "oilParticle" ), le->pos.trBase, dir,  CG_FxInt( "wolf/controller_addemitter", "tuning" ), le->ownerNum );
		break;
	case 2:     // water
		VectorScale( le->angles.trBase, le->radius, dir );
		CG_Particle_OilParticle( CG_FxMaterial( "wolf/controller_addemitter", "oilParticle" ), le->pos.trBase, dir,  CG_FxInt( "wolf/controller_addemitter", "tuning_2" ), le->ownerNum );
		break;
	case 3:     // steam
		nextTime = CG_FxInt( "wolf/controller_addemitter", "nextTime_2" );
//			CG_ParticleImpactSmokePuffExtended(cgs.media.smokeParticleShader, le->pos.trBase, 8, 1000, 8, 20, 20, 0.25f);
		CG_ParticleImpactSmokePuffExtended( CG_FxMaterial( "wolf/controller_addemitter", "smokeParticleShader" ), le->pos.trBase, le->angles.trBase, CG_FxInt( "wolf/controller_addemitter", "tuning_3" ), CG_FxInt( "wolf/controller_addemitter", "ParticleImpactSmokePuffExtended_value" ), CG_FxInt( "wolf/controller_addemitter", "tuning_4" ), le->radius, CG_FxInt( "wolf/controller_addemitter", "tuning_5" ), CG_FxValue( "wolf/controller_addemitter", "ParticleImpactSmokePuffExtended_value_2" ) );
		break;
	case 4:     // wine
		VectorScale( le->angles.trBase, le->radius, dir );
		CG_Particle_OilParticle( CG_FxMaterial( "wolf/controller_addemitter", "oilParticle" ), le->pos.trBase, dir,  CG_FxInt( "wolf/controller_addemitter", "tuning_6" ), le->ownerNum );
		break;
	case 5:     // smoke
		nextTime = CG_FxInt( "wolf/controller_addemitter", "nextTime_3" );
		CG_ParticleImpactSmokePuffExtended( CG_FxMaterial( "wolf/controller_addemitter", "smokeParticleShader" ), le->pos.trBase, dir, CG_FxInt( "wolf/controller_addemitter", "tuning_7" ), CG_FxInt( "wolf/controller_addemitter", "ParticleImpactSmokePuffExtended_value_3" ), CG_FxInt( "wolf/controller_addemitter", "tuning_8" ), CG_FxInt( "wolf/controller_addemitter", "tuning_9" ), CG_FxInt( "wolf/controller_addemitter", "tuning_10" ), CG_FxValue( "wolf/controller_addemitter", "ParticleImpactSmokePuffExtended_value_4" ) );
		break;
	case 6:     // electrical
		nextTime = CG_FxInt( "wolf/controller_addemitter", "nextTime_4" );
		CG_AddBulletParticles( le->pos.trBase, dir, CG_FxInt( "wolf/controller_addemitter", "AddBulletParticles_value" ), CG_FxInt( "wolf/controller_addemitter", "tuning_11" ), CG_FxInt( "wolf/controller_addemitter", "tuning_12" ), CG_FxValue( "wolf/controller_addemitter", "tuning_13" ) );
//			CG_AddBulletParticles( le->pos.trBase, dir, 1, 800, 4, 0 );
		break;

	case 0:
	default:
		nextTime = CG_FxInt( "wolf/controller_addemitter", "nextTime_5" );
		CG_ParticleImpactSmokePuffExtended( CG_FxMaterial( "wolf/controller_addemitter", "smokeParticleShader" ), le->pos.trBase, dir, CG_FxInt( "wolf/controller_addemitter", "tuning_14" ), CG_FxInt( "wolf/controller_addemitter", "ParticleImpactSmokePuffExtended_value_5" ), CG_FxInt( "wolf/controller_addemitter", "tuning_15" ), CG_FxInt( "wolf/controller_addemitter", "tuning_16" ), CG_FxInt( "wolf/controller_addemitter", "tuning_17" ), CG_FxValue( "wolf/controller_addemitter", "ParticleImpactSmokePuffExtended_value_6" ) );
		break;
	}

	le->breakCount = cg.time + nextTime;
}

//----(SA)	end


/*
================
CG_AddFragment
================
*/
void CG_AddFragment( fxEntity_t *le ) {
	vec3_t newOrigin;
	trace_t trace;
	refEntity_t     *re;
	float flameAlpha = CG_FxValue( "wolf/controller_addfragment", "flameAlpha" );   // TTimo: init
	vec3_t flameDir;
	qboolean hasFlame = qfalse;
	int i;
	int contents;

	// Ridah
	re = &le->refEntity;
	if ( !re->fadeStartTime || re->fadeEndTime < le->endTime ) {
		if ( le->endTime - cg.time > CG_FxInt( "wolf/controller_addfragment", "tuning" ) ) {
			re->fadeStartTime = le->endTime - CG_FxInt( "wolf/controller_addfragment", "re_fadeStartTime" );
		} else {
			re->fadeStartTime = le->endTime - CG_FxInt( "wolf/controller_addfragment", "re__fadeStartTime_value" );
		}
		re->fadeEndTime = le->endTime;
	}

	// Ridah, flaming gibs
	if ( le->onFireStart && ( le->onFireStart < cg.time && le->onFireEnd > cg.time ) ) {
		hasFlame = qtrue;
		// calc the alpha
		flameAlpha = CG_FxValue( "wolf/controller_addfragment", "flameAlpha_2" ) - ( (float)( cg.time - le->onFireStart ) / (float)( le->onFireEnd - le->onFireStart ) );
		if ( flameAlpha < 0.0 ) {
			flameAlpha = CG_FxValue( "wolf/controller_addfragment", "flameAlpha_3" );
		}
		if ( flameAlpha > 1.0 ) {
			flameAlpha = CG_FxValue( "wolf/controller_addfragment", "flameAlpha_4" );
		}
		trap_S_AddLoopingSound( -1, le->refEntity.origin, vec3_origin, cgs.media.flameCrackSound, (int)( CG_FxValue( "wolf/controller_addfragment", "tuning_2" ) * flameAlpha ) );
	}

//----(SA)	added
	if ( le->leFlags & LEF_SMOKING ) {
		float alpha;
		refEntity_t flash;

		// create a little less smoke

		//	TODO: FIXME: this is not quite right, because it'll become fps dependant - in a bad way.
		//		the slower the fps, the /more/ smoke there'll be, probably driving the fps lower.
		if ( !cg_paused.integer ) {    // don't add while paused
			if ( !( rand() % CG_FxInt( "wolf/controller_addfragment", "tuning_3" ) ) ) {
				alpha = CG_FxValue( "wolf/controller_addfragment", "alpha" ) - ( (float)( cg.time - le->startTime ) / (float)( le->endTime - le->startTime ) );
				alpha *= CG_FxValue( "wolf/controller_addfragment", "alpha_2" );
				memset( &flash, 0, sizeof( flash ) );
				CG_PositionEntityOnTag( &flash, &le->refEntity, "tag_flash", 0, NULL );
				CG_ParticleImpactSmokePuffExtended( CG_FxMaterial( "wolf/controller_addfragment", "smokeParticleShader" ), flash.origin, tv( 0,0,1 ), CG_FxInt( "wolf/controller_addfragment", "tuning_4" ), CG_FxInt( "wolf/controller_addfragment", "ParticleImpactSmokePuffExtended_value" ), CG_FxInt( "wolf/controller_addfragment", "tuning_5" ), CG_FxInt( "wolf/controller_addfragment", "tuning_6" ), CG_FxInt( "wolf/controller_addfragment", "tuning_7" ), alpha );
			}
		}
	}
//----(SA)	end

	if ( le->pos.trType == TR_STATIONARY ) {
		int t;

		// Ridah, add the flame
		if ( hasFlame ) {
			refEntity_t backupEnt;

			backupEnt = le->refEntity;

			VectorClear( flameDir );
			flameDir[2] = 1;

			le->refEntity.shaderRGBA[3] = ( unsigned char )( 255.0 * flameAlpha );
			VectorCopy( flameDir, le->refEntity.fireRiseDir );
			le->refEntity.customShader = CG_FxMaterial( "wolf/controller_addfragment", "onFireShader" );
			trap_R_AddRefEntityToScene( &le->refEntity );
			le->refEntity.customShader = CG_FxMaterial( "wolf/controller_addfragment", "onFireShader2" );
			trap_R_AddRefEntityToScene( &le->refEntity );

			le->refEntity = backupEnt;
		}

		t = le->endTime - cg.time;
		trap_R_AddRefEntityToScene( &le->refEntity );

		return;

	} else if ( le->pos.trType == TR_GRAVITY_PAUSED ) {
		int t;

		// Ridah, add the flame
		if ( hasFlame ) {
			refEntity_t backupEnt;

			backupEnt = le->refEntity;

			VectorClear( flameDir );
			flameDir[2] = 1;

			le->refEntity.shaderRGBA[3] = ( unsigned char )( 255.0 * flameAlpha );
			VectorCopy( flameDir, le->refEntity.fireRiseDir );
			le->refEntity.customShader = CG_FxMaterial( "wolf/controller_addfragment", "onFireShader" );
			trap_R_AddRefEntityToScene( &le->refEntity );
			le->refEntity.customShader = CG_FxMaterial( "wolf/controller_addfragment", "onFireShader2" );
			trap_R_AddRefEntityToScene( &le->refEntity );

			le->refEntity = backupEnt;
		}

		t = le->endTime - cg.time;
		trap_R_AddRefEntityToScene( &le->refEntity );


		// trace a line from previous position down, to see if I should start falling again

		VectorCopy( le->refEntity.origin, newOrigin );
		newOrigin [2] -= CG_FxInt( "wolf/controller_addfragment", "_" );
		CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, -1, CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_MISSILECLIP );

		if ( trace.fraction == 1.0 ) { // it's clear, start moving again
			VectorClear( le->pos.trDelta );
			VectorClear( le->angles.trDelta );
			le->pos.trType = TR_GRAVITY;    // nothing below me, start falling again
		} else {
			return;
		}
	}

	// calculate new position
	BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );

	if ( hasFlame ) {
		// calc the flame dir
		VectorSubtract( le->refEntity.origin, newOrigin, flameDir );
		if ( VectorLength( flameDir ) == 0 ) {
			flameDir[2] = 1;
			// play a burning sound when not moving
			trap_S_AddLoopingSound( 0, newOrigin, vec3_origin, cgs.media.flameSound, (int)( 0.3 * 255.0 * flameAlpha ) );
		} else {
			VectorNormalize( flameDir );
			// play a flame blow sound when moving
			trap_S_AddLoopingSound( 0, newOrigin, vec3_origin, cgs.media.flameBlowSound, (int)( 0.3 * 255.0 * flameAlpha ) );
		}
	}

	contents = CONTENTS_SOLID;

	// trace a line from previous position to new position
	if ( ( le->leFlags & LEF_NOTOUCHPARENT ) && le->ownerNum ) {
		CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, le->ownerNum, contents );
	} else {
		CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, -1, contents );
	}

	// did we hit someone?
	if ( le->leFlags & LEF_PLAYER_DAMAGE ) {
		// only do damage if travelling at a fast enough velocity
		if ( newOrigin[2] < le->refEntity.origin[2] ) {
			vec3_t pmin, pmax, dmin = {-CG_FxInt( "wolf/controller_addfragment", "dmin" ),-CG_FxInt( "wolf/controller_addfragment", "dmin_2" ),-CG_FxInt( "wolf/controller_addfragment", "dmin_3" )}, dmax = {CG_FxInt( "wolf/controller_addfragment", "dmin_4" ),CG_FxInt( "wolf/controller_addfragment", "dmin_5" ),CG_FxInt( "wolf/controller_addfragment", "dmin_6" )};
			// debris bounds
			VectorAdd( dmin, newOrigin, dmin );
			VectorAdd( dmax, newOrigin, dmax );
			dmax[2] += le->refEntity.origin[2] - newOrigin[2];  // add falling distance to box
			// are we touching the player?
			VectorAdd( cg.snap->ps.mins, cg.snap->ps.origin, pmin );
			VectorAdd( cg.snap->ps.maxs, cg.snap->ps.origin, pmax );
			pmin[2] = pmax[2] - CG_FxInt( "wolf/controller_addfragment", "pmin_z" ); // only hit on the head
			for ( i = 0; i < 3; i++ ) {
				if ( ( dmax[i] < pmin[i] ) || ( dmin[i] > pmax[i] ) ) {
					break;
				}
			}
			if ( i == 3 ) {
				trap_S_StartSound( cg.snap->ps.origin, cg.snap->ps.clientNum, CHAN_VOICE, cgs.media.debrisHitSound );
				CG_ClientDamage( cg.snap->ps.clientNum, ENTITYNUM_WORLD, CLDMG_DEBRIS );
				// disable damage now for this debris
				le->leFlags &= ~LEF_PLAYER_DAMAGE;
			}
		}
	}

	if ( trace.fraction == 1.0 ) {

		// still in free fall
		VectorCopy( newOrigin, le->refEntity.origin );

		if ( le->leFlags & LEF_TUMBLE || le->angles.trType == TR_LINEAR ) {
			vec3_t angles;

			BG_EvaluateTrajectory( &le->angles, cg.time, angles );
			AnglesToAxis( angles, le->refEntity.axis );
			if ( le->sizeScale && le->sizeScale != 1.0 ) {
				for ( i = 0; i < 3; i++ ) VectorScale( le->refEntity.axis[i], le->sizeScale, le->refEntity.axis[i] );
			}
		}

		// Ridah, add the flame
		if ( hasFlame ) {
			refEntity_t backupEnt;

			backupEnt = le->refEntity;

			le->refEntity.shaderRGBA[3] = ( unsigned char )( 255.0 * flameAlpha );
			VectorCopy( flameDir, le->refEntity.fireRiseDir );
			le->refEntity.customShader = CG_FxMaterial( "wolf/controller_addfragment", "onFireShader" );
			trap_R_AddRefEntityToScene( &le->refEntity );
			le->refEntity.customShader = CG_FxMaterial( "wolf/controller_addfragment", "onFireShader2" );
			trap_R_AddRefEntityToScene( &le->refEntity );

			le->refEntity = backupEnt;
		}

		trap_R_AddRefEntityToScene( &le->refEntity );

		// add a blood trail
		if ( le->leBounceSoundType == LEBS_BLOOD ) {
			CG_BloodTrail( le );
		}

		return;
	}

	// if it is in a nodrop zone, remove it
	// this keeps gibs from waiting at the bottom of pits of death
	// and floating levels
	if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
		CG_FxFreeEntity( le );
		return;
	}

	// do a bouncy sound
	CG_FragmentBounceSound( le, &trace );

	// reflect the velocity on the trace plane
	CG_ReflectVelocity( le, &trace );


	// (SA) disabled since it caused debris to get stuck in walls (flags mostly)
	// RF, if it has come to a halt, pause velocity
//	if (VectorLength( le->pos.trDelta ) < 1) {
//		le->pos.trType = TR_STATIONARY;
//		VectorCopy( trace.endpos, le->pos.trBase );
//		VectorClear( le->pos.trDelta );
//	}

	// break on contact?
	if ( le->breakCount ) {
		clientInfo_t    *ci;
		int clientNum;
		fxEntity_t   *nle;
		vec3_t dir;

		clientNum = le->ownerNum;
		if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
			CG_Error( "Bad clientNum on player entity" );
		}
		ci = &cgs.clientinfo[ clientNum ];

		// spawn some new fragments
		for ( i = 0; i <= le->breakCount; i++ ) {
			nle = CG_FxSpawnModel( "wolf/controller_addfragment" );
			*nle = *le;
			if ( nle->breakCount-- < 2 ) {
				nle->refEntity.hModel = ci->gibModels[rand() % 2];
			} else {
				nle->refEntity.hModel = ci->gibModels[rand() % 4];
			}
			// make it smaller
			nle->endTime = le->endTime + ( cg.time - le->startTime );
			nle->sizeScale *= CG_FxValue( "wolf/controller_addfragment", "nle_sizeScale" );
			if ( nle->sizeScale < 0.7 ) {
				nle->sizeScale = CG_FxValue( "wolf/controller_addfragment", "nle_sizeScale_2" );
				nle->leBounceSoundType = (leBounceSoundType_t)CG_FxInt( "wolf/controller_addfragment", "nle_leBounceSoundType" );
			}
			// move us a bit
			VectorNormalize2( nle->pos.trDelta, dir );
			VectorMA( trace.endpos, CG_FxValue( "wolf/controller_addfragment", "tuning_8" ) * le->sizeScale * i, dir, nle->pos.trBase );
			// randomize vel a bit
			VectorMA( nle->pos.trDelta, VectorLength( nle->pos.trDelta ) * 0.3, bytedirs[rand() % NUMVERTEXNORMALS], nle->pos.trDelta );
		}
		// we're done
		CG_FxFreeEntity( le );
		return;
	}

	if ( le->pos.trType == TR_STATIONARY && le->leMarkType == LEMT_BLOOD ) {
		// RF, disabled for performance reasons in boss1
		//if (le->leBounceSoundType)
		//	CG_BloodPool (le, cgs.media.bloodPool, &trace);

		// leave a mark
		if ( le->leMarkType ) {
			CG_FragmentBounceMark( le, &trace );
		}
	}

	// Ridah, add the flame
	if ( hasFlame ) {
		refEntity_t backupEnt;

		backupEnt = le->refEntity;

		le->refEntity.shaderRGBA[3] = ( unsigned char )( 255.0 * flameAlpha );
		VectorCopy( flameDir, le->refEntity.fireRiseDir );
		le->refEntity.customShader = CG_FxMaterial( "wolf/controller_addfragment", "onFireShader" );
		trap_R_AddRefEntityToScene( &le->refEntity );
		le->refEntity.customShader = CG_FxMaterial( "wolf/controller_addfragment", "onFireShader2" );
		trap_R_AddRefEntityToScene( &le->refEntity );

		le->refEntity = backupEnt;
	}

	trap_R_AddRefEntityToScene( &le->refEntity );
}

// Ridah
/*
================
CG_AddMovingTracer
================
*/
void CG_AddMovingTracer( fxEntity_t *le ) {
	vec3_t start, end, dir;

	BG_EvaluateTrajectory( &le->pos, cg.time, start );
	VectorNormalize2( le->pos.trDelta, dir );
	VectorMA( start, cg_tracerLength.value, dir, end );

	CG_DrawTracer( start, end );
}

/*
================
CG_AddSparkElements
================
*/
void CG_AddSparkElements( fxEntity_t *le ) {
	vec3_t newOrigin;
	trace_t trace;
	float time;
	float lifeFrac;

	time = (float)( cg.time - cg.frametime );

	while ( 1 ) {
		// calculate new position
		BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );

//		if ((le->endTime - le->startTime) > 500) {

		// trace a line from previous position to new position
		CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, -1, MASK_SHOT );

		// if stuck, kill it
		if ( trace.startsolid ) {
			// HACK, some walls screw up, so just pass through if starting in a solid
			VectorCopy( newOrigin, trace.endpos );
			trace.fraction = 1.0;
		}

		// moved some distance
		VectorCopy( trace.endpos, le->refEntity.origin );
/*
		} else
		{	// just move it there

			VectorCopy( newOrigin, le->refEntity.origin );
			trace.fraction = 1.0;

		}
*/
		time += cg.frametime * trace.fraction;

		lifeFrac = (float)( cg.time - le->startTime ) / (float)( le->endTime - le->startTime );

		// add a trail
		le->headJuncIndex = CG_FxTrail( "wolf/trail_impact_spark", le->headJuncIndex, le->refEntity.origin,
			FxTrailInputs().Alpha( 1.0 - lifeFrac ).Width( lifeFrac * 2.0 * ( ( ( le->endTime - le->startTime ) > CG_FxInt( "wolf/controller_addsparkelements", "tuning" ) ) + 1 ) * 1.5 ).WidthEnd( lifeFrac * 2.0 * ( ( ( le->endTime - le->startTime ) > CG_FxInt( "wolf/controller_addsparkelements", "tuning_2" ) ) + 1 ) * 1.5 ).Shader( le->refEntity.customShader ) );

		// if it is in a nodrop zone, remove it
		// this keeps gibs from waiting at the bottom of pits of death
		// and floating levels
// for some reason SFM1.BSP is one big NODROP zone
//		if ( trap_CM_PointContents( le->refEntity.origin, 0 ) & CONTENTS_NODROP ) {
//			CG_FxFreeEntity( le );
//			return;
//		}

		if ( trace.fraction < 1.0 ) {
			// just kill it
			CG_FxFreeEntity( le );
			return;
/*
			// reflect the velocity on the trace plane
			CG_ReflectVelocity( le, &trace );
			// the intersection is a fraction of the frametime
			le->pos.trTime = (int)time;
*/
		}

		if ( trace.fraction == 1.0 || time >= (float)cg.time ) {
			return;
		}
	}

}



/*
================
CG_AddBloodElements
================
*/
/*
===================
CG_AddSpiritViewflash
===================
*/
void CG_AddSpiritViewflash( fxEntity_t *le ) {
	float alpha;
#define SPIRIT_FLASH_FADEIN     50
#define SPIRIT_FLASH_DURATION   400
#define SPIRIT_FLASH_FADEOUT    2000

	if ( cg.viewFade > 1.0 ) {
		return;
	}

	if ( cg.time < le->startTime + SPIRIT_FLASH_FADEIN ) {
		alpha = (float)( cg.time - le->startTime ) / (float)SPIRIT_FLASH_FADEIN;
	} else if ( cg.time < le->startTime + SPIRIT_FLASH_FADEIN + SPIRIT_FLASH_DURATION )     {
		alpha = CG_FxValue( "wolf/controller_addspiritviewflash", "alpha" );
	} else if ( cg.time < le->startTime + SPIRIT_FLASH_FADEIN + SPIRIT_FLASH_DURATION + SPIRIT_FLASH_FADEOUT )     {
		alpha = CG_FxValue( "wolf/controller_addspiritviewflash", "alpha_2" ) - (float)( cg.time - ( le->startTime + SPIRIT_FLASH_FADEIN + SPIRIT_FLASH_DURATION ) ) / (float)SPIRIT_FLASH_FADEOUT;
	} else {
		return;
	}

	if ( alpha < 0 ) {
		return;
	}

	// only ever use the highest fade
	if ( cg.viewFade < alpha ) {
		cg.viewFade = alpha;
	}
}

/*
================
CG_AddClientCritter
================
*/
void CG_AddClientCritter( fxEntity_t *le ) {
	vec3_t newOrigin;
	trace_t trace;
	int time, step = CG_FxInt( "wolf/controller_addclientcritter", "step" ), i;
	vec3_t v, ang, v2, oDelta;
	fxEntity_t backup;
	float oldSpeed, enemyDist, of;
	vec3_t enemyPos;
	float alpha = CG_FxValue( "wolf/controller_addclientcritter", "alpha" ), fadeRatio; // TTimo: init

	if ( cg_entities[le->ownerNum].currentState.otherEntityNum2 == cg.snap->ps.clientNum ) {
		VectorCopy( cg.snap->ps.origin, enemyPos );
		enemyPos[2] += cg.snap->ps.viewheight;
	} else {
		VectorCopy( cg_entities[le->ownerNum].currentState.origin2, enemyPos );
	}

	VectorCopy( le->pos.trDelta, oDelta );

	// vary the enemyPos to create a psuedo-randomness
	of = (float)cg.time + le->startTime;
	enemyPos[0] += CG_FxInt( "wolf/controller_addclientcritter", "enemyPos_x" ) * ( sin( of / CG_FxInt( "wolf/controller_addclientcritter", "enemyPos_x_2" ) ) * cos( of / CG_FxInt( "wolf/controller_addclientcritter", "enemyPos_x_3" ) ) );
	enemyPos[1] += CG_FxInt( "wolf/controller_addclientcritter", "enemyPos_y" ) * ( sin( of / CG_FxInt( "wolf/controller_addclientcritter", "enemyPos_y_2" ) ) * cos( of / CG_FxInt( "wolf/controller_addclientcritter", "enemyPos_y_3" ) ) );
	enemyPos[2] += CG_FxInt( "wolf/controller_addclientcritter", "enemyPos_z" ) * ( sin( of / CG_FxInt( "wolf/controller_addclientcritter", "enemyPos_z_2" ) ) * cos( of / CG_FxInt( "wolf/controller_addclientcritter", "enemyPos_z_3" ) ) );

	time = le->lastTrailTime + step;

	fadeRatio = (float)( cg.time - le->startTime ) / CG_FxValue( "wolf/controller_addclientcritter", "fadeRatio" );
	if ( fadeRatio < 0.0 ) {
		fadeRatio = 0.0;
	}
	if ( fadeRatio > 1.0 ) {
		fadeRatio = 1.0;
	}

	while ( time <= cg.time ) {
		if ( time > le->refEntity.fadeStartTime ) {
			alpha = (float)( time - le->refEntity.fadeStartTime ) / (float)( le->refEntity.fadeEndTime - le->refEntity.fadeStartTime );
			if ( alpha < 0 ) {
				alpha = CG_FxInt( "wolf/controller_addclientcritter", "alpha_2" );
			} else if ( alpha > 1 ) {
				alpha = CG_FxInt( "wolf/controller_addclientcritter", "alpha_3" );
			}
		} else {
			alpha = fadeRatio;
		}

		// calculate new position
		BG_EvaluateTrajectory( &le->pos, time, newOrigin );

		VectorSubtract( enemyPos, le->refEntity.origin, v );
		enemyDist = VectorNormalize( v );

		// trace a line from previous position to new position
		CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, le->ownerNum, MASK_SHOT );

		// if stuck, kill it
		if ( trace.startsolid || ( trace.fraction < 1.0 ) ) {
			// let heinrich spirits pass through geometry
			if ( !( le->leType == LE_HELGA_SPIRIT && ( le->refEntity.hModel == CG_FxModel( "wolf/controller_addclientcritter", "ssSpiritSkullModel" ) ) ) ) {
				// kill it
				CG_FxFreeEntity( le );
				return;
			} else {
				VectorCopy( newOrigin, trace.endpos );
			}
		}

		// moved some distance
		VectorCopy( trace.endpos, le->refEntity.origin );

		// record this pos
		le->validOldPos[le->oldPosHead] = 1;
		VectorCopy( le->refEntity.origin, le->oldPos[le->oldPosHead] );
		if ( ++le->oldPosHead >= MAX_OLD_POS ) {
			le->oldPosHead = 0;
		}

		if ( le->leType == LE_ZOMBIE_SPIRIT ) {
			le->headJuncIndex = CG_FxTrail( "wolf/trail_zombie_spirit", le->headJuncIndex, le->refEntity.origin,
			FxTrailInputs().Time( time ).Life( (int)le->effectWidth ).Alpha( 0.3 * alpha ).Width( le->radius ) );
		}

		if ( le->leType == LE_HELGA_SPIRIT && le->refEntity.hModel != CG_FxModel( "wolf/controller_addclientcritter", "ssSpiritSkullModel" ) ) {
			le->headJuncIndex = CG_FxTrail( "wolf/trail_helga_spirit", le->headJuncIndex, le->refEntity.origin,
			FxTrailInputs().Time( time ).Life( (int)le->effectWidth ).Alpha( 0.3 * alpha ).Width( le->radius ) );
		}

		// tracking factor
		if ( le->leType == LE_ZOMBIE_BAT ) {
			le->bounceFactor = CG_FxValue( "wolf/controller_addclientcritter", "le_bounceFactor" ) * (float)step / 1000.0;
		} else {
			le->bounceFactor = CG_FxValue( "wolf/controller_addclientcritter", "le_bounceFactor_2" ) * (float)step / 1000.0;
		}
		oldSpeed = VectorLength( le->pos.trDelta );

		// track the enemy
		backup = *le;
		VectorSubtract( enemyPos, le->refEntity.origin, v );
		enemyDist = VectorNormalize( v );

		if ( alpha > 0.5 && ( le->lastSpiritDmgTime < time - CG_FxInt( "wolf/controller_addclientcritter", "tuning" ) ) && enemyDist < CG_FxInt( "wolf/controller_addclientcritter", "tuning_2" ) ) {
			fxEntity_t *fb;

			// if dead, ignore
			if ( !( le->ownerNum != cg.snap->ps.clientNum ? cg_entities[le->ownerNum].currentState.eFlags & EF_DEAD : cg.snap->ps.pm_type == PM_DEAD ) ) {
				// inflict the damage!
				CG_ClientDamage( cg_entities[le->ownerNum].currentState.otherEntityNum2, le->ownerNum, CLDMG_SPIRIT );
				le->lastSpiritDmgTime = time;

				if ( le->leType == LE_HELGA_SPIRIT ) {
					// spawn a "flashbang" thinker
					fb = CG_FxSpawnModel( "wolf/controller_addclientcritter" );
					fb->leType = LE_SPIRIT_VIEWFLASH;
					fb->startTime = cg.time + CG_FxInt( "wolf/controller_addclientcritter", "fb_startTime" );
					fb->endTime = fb->startTime + SPIRIT_FLASH_FADEIN + SPIRIT_FLASH_DURATION + SPIRIT_FLASH_FADEOUT;
					// gasp!
					CG_SoundPlayIndexedScript( cgs.media.helgaGaspSound, NULL, cg_entities[le->ownerNum].currentState.otherEntityNum2 );
				}
			}
		}

		VectorMA( le->pos.trDelta, le->bounceFactor * oldSpeed, v, le->pos.trDelta );
		//VectorCopy( v, le->pos.trDelta );
		if ( VectorLength( le->pos.trDelta ) < 1 ) {
			CG_FxFreeEntity( le );
			return;
		}

		le->bounceFactor = CG_FxValue( "wolf/controller_addclientcritter", "le_bounceFactor_3" ) * (float)step / 1000.0;  // avoidance factor

		// the intersection is a fraction of the frametime
		le->pos.trTime = time;
		VectorCopy( le->refEntity.origin, le->pos.trBase );
		VectorNormalize( le->pos.trDelta );
		VectorScale( le->pos.trDelta, oldSpeed, le->pos.trDelta );

		// now trace ahead of time, if we're going to hit something, then avoid it
		// only avoid dangers if we don't have direct sight to the enemy
		trap_CM_BoxTrace( &trace, le->refEntity.origin, enemyPos, NULL, NULL, 0, MASK_SOLID );
		if ( trace.fraction < 1.0 ) {
			BG_EvaluateTrajectory( &le->pos, time + 1000, newOrigin );

			// if we would go passed the enemy, don't bother
			if ( VectorDistance( le->refEntity.origin, enemyPos ) > VectorDistance( le->refEntity.origin, newOrigin ) ) {

				trap_CM_BoxTrace( &trace, le->refEntity.origin, newOrigin, NULL, NULL, 0, MASK_SOLID );

				if ( trace.fraction < 1.0 ) {
					// make sure we are not heading away from the enemy too much
					VectorNormalize2( le->pos.trDelta, v2 );
					if ( DotProduct( v, v2 ) > 0.7 ) {
						// avoid world geometry
						backup = *le;
						le->bounceFactor = ( CG_FxValue( "wolf/controller_addclientcritter", "le_bounceFactor_4" ) - trace.fraction ) * CG_FxValue( "wolf/controller_addclientcritter", "le_bounceFactor_5" ) * (float)step / 1000.0;  // tracking and avoidance factor
						// reflect the velocity on the trace plane
						VectorMA( le->pos.trDelta, le->bounceFactor * oldSpeed, trace.plane.normal, le->pos.trDelta );
						if ( VectorLength( le->pos.trDelta ) < 1 ) {
							CG_FxFreeEntity( le );
							return;
						}
						// the intersection is a fraction of the frametime
						le->pos.trTime = time;
						VectorCopy( le->refEntity.origin, le->pos.trBase );
						VectorNormalize( le->pos.trDelta );
						VectorScale( le->pos.trDelta, oldSpeed, le->pos.trDelta );
						//
						// double check end velocity
						VectorNormalize2( le->pos.trDelta, v2 );
						if ( DotProduct( v, v2 ) <= 0.2 ) {
							// restore
							*le = backup;
						}
					}
				}
			}
		}

		// set the angles
		VectorNormalize2( oDelta, v );
		// HACK!!! skull model is back-to-front, need to fix
		if ( le->leType == LE_ZOMBIE_SPIRIT /*|| le->leType == LE_HELGA_SPIRIT*/ ) {
			VectorInverse( v );
		}
		vectoangles( v, ang );
		AnglesToAxis( ang, le->refEntity.axis );
		// lean when turning
		if ( le->leType == LE_ZOMBIE_BAT || le->leType == LE_HELGA_SPIRIT ) {
			VectorSubtract( le->pos.trDelta, oDelta, v2 );
			ang[ROLL] = -CG_FxValue( "wolf/controller_addclientcritter", "ang_ROLL_" ) * DotProduct( le->refEntity.axis[1], v2 );
			if ( fabs( ang[ROLL] ) < CG_FxInt( "wolf/controller_addclientcritter", "tuning_3" ) ) {
				ang[ROLL] = CG_FxInt( "wolf/controller_addclientcritter", "ang_ROLL__2" );
			} else {
				if ( ang[ROLL] < 0 ) {
					ang[ROLL] += CG_FxInt( "wolf/controller_addclientcritter", "ang_ROLL__3" );
				} else {
					ang[ROLL] -= CG_FxInt( "wolf/controller_addclientcritter", "ang_ROLL__4" );
				}
			}
			if ( fabs( ang[ROLL] ) > CG_FxInt( "wolf/controller_addclientcritter", "tuning_4" ) ) {
				if ( ang[ROLL] > CG_FxInt( "wolf/controller_addclientcritter", "tuning_5" ) ) {
					ang[ROLL] = CG_FxInt( "wolf/controller_addclientcritter", "ang_ROLL__5" );
				} else { ang[ROLL] = -CG_FxInt( "wolf/controller_addclientcritter", "ang_ROLL__6" );}
			}
			AnglesToAxis( ang, le->refEntity.axis );
		}

		// HACK: the skull is slightly higher than the origin
		if ( le->leType == LE_ZOMBIE_SPIRIT ) {
			// set the size scale
			for ( i = 0; i < 3; i++ )
				VectorScale( le->refEntity.axis[i], 0.35, le->refEntity.axis[i] );
			VectorMA( le->refEntity.origin, -CG_FxInt( "wolf/controller_addclientcritter", "tuning_6" ), le->refEntity.axis[2], le->refEntity.origin );
		}

		le->lastTrailTime = time;
		time += step;
	}

	// Bats, set the frame
	if ( le->leType == LE_ZOMBIE_BAT ) {
		#define BAT_ANIM_FRAMETIME  30
		le->refEntity.frame = ( cg.time / BAT_ANIM_FRAMETIME + 1 ) % CG_FxInt( "wolf/controller_addclientcritter", "le_refEntity_frame" );
		le->refEntity.oldframe = ( cg.time / BAT_ANIM_FRAMETIME ) % CG_FxInt( "wolf/controller_addclientcritter", "le_refEntity_oldframe" );
		le->refEntity.backlerp = 1.0 - ( (float)( cg.time % BAT_ANIM_FRAMETIME ) / (float)BAT_ANIM_FRAMETIME );
	}

	// add the sound
	if ( le->loopingSound ) {
		if ( cg.time > le->refEntity.fadeStartTime ) {
			trap_S_AddLoopingSound( 0, le->refEntity.origin, vec3_origin, le->loopingSound, 255 - (int)( 255.0 * (float)( cg.time - le->refEntity.fadeStartTime ) / (float)( le->refEntity.fadeEndTime - le->refEntity.fadeStartTime ) ) );
		} else {
			trap_S_AddLoopingSound( 0, le->refEntity.origin, vec3_origin, le->loopingSound, 255 - (int)( 255.0 * ( 1.0 - alpha ) ) );
		}
	}
/*
	if (le->leType == LE_HELGA_SPIRIT) {
		int cnt=1;
		float alpha;
		refEntity_t re;

		// add the "motion blur" ghosts
		re = le->refEntity;
		i = le->oldPosHead - 1;
		if (i < 0) i = MAX_OLD_POS-1;
		while (i != le->oldPosHead && le->validOldPos[i]) {
			alpha = 1.0 - ((float)cnt / (float)MAX_OLD_POS);
			if (alpha > 1.0) alpha = 1.0;
			if (alpha < 0.0) alpha = 0.0;

			re.shaderTime = le->refEntity.shaderTime - cnt*100;
			VectorCopy( le->oldPos[i], re.origin );
			re.shaderRGBA[3] = (unsigned char)(255.0 * alpha);
			trap_R_AddRefEntityToScene( &re );

			if (--i<0) i=MAX_OLD_POS-1;
			cnt++;
		}
	} else {
*/  trap_R_AddRefEntityToScene( &le->refEntity );
//	}

	// Bats, add the flame
	if ( le->leType == LE_ZOMBIE_BAT ) {
		//
		le->refEntity.shaderRGBA[3] = 255;
		VectorNormalize2( le->pos.trDelta, v );
		VectorInverse( v );
		v[2] += 1;
		VectorNormalize2( v, le->refEntity.fireRiseDir );

		le->refEntity.customShader = CG_FxMaterial( "wolf/controller_addclientcritter", "onFireShader2" );
		trap_R_AddRefEntityToScene( &le->refEntity );
		le->refEntity.shaderTime = CG_FxInt( "wolf/controller_addclientcritter", "le_refEntity_shaderTime" );
		trap_R_AddRefEntityToScene( &le->refEntity );

		le->refEntity.customShader = 0;
		le->refEntity.shaderTime = 0;
	}
}

/*
================
CG_AddDebrisElements
================
*/
void CG_AddDebrisElements( fxEntity_t *le ) {
	vec3_t newOrigin;
	trace_t trace;
	float lifeFrac;
	int t, step = CG_FxInt( "wolf/controller_adddebriselements", "step" );

	for ( t = le->lastTrailTime + step; t < cg.time; t += step ) {
		// calculate new position
		BG_EvaluateTrajectory( &le->pos, t, newOrigin );

		// trace a line from previous position to new position
		CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, -1, MASK_SHOT );

		// if stuck, kill it
		if ( trace.startsolid ) {
			// HACK, some walls screw up, so just pass through if starting in a solid
			VectorCopy( newOrigin, trace.endpos );
			trace.fraction = 1.0;
		}

		// moved some distance
		VectorCopy( trace.endpos, le->refEntity.origin );

		// add a trail
		lifeFrac = (float)( t - le->startTime ) / (float)( le->endTime - le->startTime );


		// smoke
		if ( le->effectFlags & 1 ) {
			le->headJuncIndex2 = CG_FxTrail( "wolf/trail_debris_smoke", le->headJuncIndex2, le->refEntity.origin,
			FxTrailInputs().Life( (int)( CG_FxValue( "wolf/controller_adddebriselements", "tuning" ) * ( 0.5 + 0.5 * ( 1.0 - lifeFrac ) ) ) ).Alpha( 1.0 * ( trace.fraction == 1.0 ) * ( 0.5 + 0.5 * ( 1.0 - lifeFrac ) ) ).WidthEnd( (int)( CG_FxValue( "wolf/controller_adddebriselements", "tuning_2" ) * ( 0.5 + 0.5 * ( 1.0 - lifeFrac ) ) ) ) ); // end width
		}

		// if it is in a nodrop zone, remove it
		// this keeps gibs from waiting at the bottom of pits of death
		// and floating levels
//		if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
//			CG_FxFreeEntity( le );
//			return;
//		}

		if ( trace.fraction < 1.0 ) {
			// reflect the velocity on the trace plane
			CG_ReflectVelocity( le, &trace );
			if ( VectorLength( le->pos.trDelta ) < 1 ) {
				CG_FxFreeEntity( le );
				return;
			}
			// the intersection is a fraction of the frametime
			le->pos.trTime = t;
		}

		le->lastTrailTime = t;
	}

}

// Rafael Shrapnel
/*
===============
CG_AddShrapnel
===============
*/
void CG_AddShrapnel( fxEntity_t *le ) {
	vec3_t newOrigin;
	trace_t trace;

	if ( le->pos.trType == TR_STATIONARY ) {
		// sink into the ground if near the removal time
		int t;
		float oldZ;

		t = le->endTime - cg.time;
		if ( t < SINK_TIME ) {
			// we must use an explicit lighting origin, otherwise the
			// lighting would be lost as soon as the origin went
			// into the ground
			VectorCopy( le->refEntity.origin, le->refEntity.lightingOrigin );
			le->refEntity.renderfx |= RF_LIGHTING_ORIGIN;
			oldZ = le->refEntity.origin[2];
			le->refEntity.origin[2] -= CG_FxInt( "wolf/controller_addshrapnel", "le_refEntity_origin_z" ) * ( 1.0 - (float)t / SINK_TIME );
			trap_R_AddRefEntityToScene( &le->refEntity );
			le->refEntity.origin[2] = oldZ;
		} else {
			trap_R_AddRefEntityToScene( &le->refEntity );
			CG_AddParticleShrapnel( le );
		}

		return;
	}

	// calculate new position
	BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );

	// trace a line from previous position to new position
	CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, -1, CONTENTS_SOLID );
	if ( trace.fraction == 1.0 ) {
		// still in free fall
		VectorCopy( newOrigin, le->refEntity.origin );

		if ( le->leFlags & LEF_TUMBLE ) {
			vec3_t angles;

			BG_EvaluateTrajectory( &le->angles, cg.time, angles );
			AnglesToAxis( angles, le->refEntity.axis );
		}

		trap_R_AddRefEntityToScene( &le->refEntity );
		CG_AddParticleShrapnel( le );
		return;
	}

	// if it is in a nodrop zone, remove it
	// this keeps gibs from waiting at the bottom of pits of death
	// and floating levels
	if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
		CG_FxFreeEntity( le );
		return;
	}

	// leave a mark
	CG_FragmentBounceMark( le, &trace );

	// do a bouncy sound
	CG_FragmentBounceSound( le, &trace );

	// reflect the velocity on the trace plane
	CG_ReflectVelocity( le, &trace );

	trap_R_AddRefEntityToScene( &le->refEntity );
	CG_AddParticleShrapnel( le );
}
// done.

/*
=====================================================================

TRIVIAL LOCAL ENTITIES

These only do simple scaling or modulation before passing to the renderer
=====================================================================
*/

/*
====================
CG_AddFadeRGB
====================
*/
void CG_AddFadeRGB( fxEntity_t *le ) {
	refEntity_t *re;
	float c;

	re = &le->refEntity;

	c = ( le->endTime - cg.time ) * le->lifeRate;
	c *= 0xff;

	re->shaderRGBA[0] = le->color[0] * c;
	re->shaderRGBA[1] = le->color[1] * c;
	re->shaderRGBA[2] = le->color[2] * c;
	re->shaderRGBA[3] = le->color[3] * c;

	trap_R_AddRefEntityToScene( re );
}











/*
================
CG_AddExplosion
================
*/
static void CG_AddExplosion( fxEntity_t *ex ) {
	refEntity_t *ent;

	ent = &ex->refEntity;

	// add the entity
	// RF, don't add if shader is invalid
	if ( ent->customShader >= 0 ) {
		trap_R_AddRefEntityToScene( ent );
	}

	// add the dlight
	if ( ex->light ) {
		float light;

		light = (float)( cg.time - ex->startTime ) / ( ex->endTime - ex->startTime );
		if ( light < 0.5 ) {
			light = CG_FxValue( "wolf/controller_addexplosion", "light" );
		} else {
			light = CG_FxValue( "wolf/controller_addexplosion", "light_2" ) - ( light - CG_FxValue( "wolf/controller_addexplosion", "light_3" ) ) * CG_FxInt( "wolf/controller_addexplosion", "light_4" );
		}
		light = ex->light * light;
		trap_R_AddLightToScene( ent->origin, light, ex->lightColor[0], ex->lightColor[1], ex->lightColor[2], 0 );
	}
}

/*
================
CG_AddSpriteExplosion
================
*/
static void CG_AddSpriteExplosion( fxEntity_t *le ) {
	refEntity_t re;
	float c;

	re = le->refEntity;

	c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
	if ( c > 1 ) {
		c = 1.0;    // can happen during connection problems
	}

	re.shaderRGBA[0] = 0xff;
	re.shaderRGBA[1] = 0xff;
	re.shaderRGBA[2] = 0xff;
	re.shaderRGBA[3] = 0xff * c * CG_FxValue( "wolf/controller_addspriteexplosion", "re_shaderRGBA_a" );

	re.reType = RT_SPRITE;
	re.radius = CG_FxInt( "wolf/controller_addspriteexplosion", "re_radius" ) * ( CG_FxValue( "wolf/controller_addspriteexplosion", "re_radius_2" ) - c ) + CG_FxInt( "wolf/controller_addspriteexplosion", "re_radius_3" );

	// Ridah, move away from surface
	VectorMA( le->pos.trBase, ( 1.0 - c ), le->pos.trDelta, re.origin );
	// done.

	// RF, don't add if shader is invalid
	if ( re.customShader >= 0 ) {
		trap_R_AddRefEntityToScene( &re );
	}

	// add the dlight
	if ( le->light ) {
		float light;

		// Ridah, modified this so the light fades out rather than shrinking
		/*
		light = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
		if ( light < 0.5 ) {
			light = 1.0;
		} else {
			light = 1.0 - ( light - 0.5 ) * 2;
		}
		light = le->light * light;
		trap_R_AddLightToScene(re.origin, light, le->lightColor[0], le->lightColor[1], le->lightColor[2], 0 );
		*/
		light = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
		if ( light < 0.5 ) {
			light = CG_FxValue( "wolf/controller_addspriteexplosion", "light" );
		} else {
			light = CG_FxValue( "wolf/controller_addspriteexplosion", "light_2" ) - ( light - CG_FxValue( "wolf/controller_addspriteexplosion", "light_3" ) ) * CG_FxInt( "wolf/controller_addspriteexplosion", "light_4" );
		}
		trap_R_AddLightToScene( re.origin, le->light, light * le->lightColor[0], light * le->lightColor[1], light * le->lightColor[2], 0 );
		// done.
	}
}

//==============================================================================

static bool CG_FxUpdateEntity( fxEntity_t *le ) {
    if ( cg.time >= le->endTime ) { return false; }
		switch ( le->leType ) {
		default:
			CG_Error( "Bad leType: %i", le->leType );
			break;

			// Ridah
		case LE_MOVING_TRACER:
			CG_AddMovingTracer( le );
			break;
		case LE_SPARK:
			CG_AddSparkElements( le );
			break;
		case LE_DEBRIS:
			CG_AddDebrisElements( le );
			break;
		case LE_HELGA_SPIRIT:
		case LE_ZOMBIE_SPIRIT:
		case LE_ZOMBIE_BAT:
			CG_AddClientCritter( le );
			break;
		case LE_SPIRIT_VIEWFLASH:
			CG_AddSpiritViewflash( le );
			// done.

		case LE_MARK:
			break;

		case LE_SPRITE_EXPLOSION:
			CG_AddSpriteExplosion( le );
			break;

		case LE_EXPLOSION:
			CG_AddExplosion( le );
			break;

		case LE_FRAGMENT:           // gibs and brass
			CG_AddFragment( le );
			break;


		case LE_FADE_RGB:               // teleporters, railtrails
			CG_AddFadeRGB( le );
			break;



		case LE_EMITTER:
			CG_AddEmitter( le );
			break;

		}
    return le->endTime > cg.time;
}
