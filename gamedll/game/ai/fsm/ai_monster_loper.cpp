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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with IceWolf. If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#include "../ai_local.h"

//=================================================================================
//
// LOPER MELEE ATTACK
//
//=================================================================================

#define NUM_LOPERMELEE_ANIMS 2
int loperHitTimes[NUM_LOPERMELEE_ANIMS] = { ( ANIMLENGTH( 3, 15 ) ), ( ANIMLENGTH( 6, 15 ) ) };

#define LOPER_MELEE_DAMAGE 20
#define LOPER_MELEE_RANGE 48

/*
===============
AIFunc_LoperMelee()

  Loper's close range melee attack
===============
*/
char *AIFunc_LoperMelee( cast_state_t *cs ) {
	trace_t *tr;
	gentity_t *ent;
	int anim;
	//
	ent = &g_entities[cs->entityNum];
	//
	// draw the client-side lightning effect
	//ent->client->ps.eFlags |= EF_MONSTER_EFFECT;
	//
	// have we inflicted the damage?
	if ( cs->combat.weaponFireTimes[WP_MONSTER_ATTACK1] > cs->thinkFuncChangeTime ) {
		// has the animation finished?
		if ( !ent->client->ps.legsTimer ) {
			return AIFunc_DefaultStart( cs );
		}
		return NULL; // just wait for anim to finish
	}
	// ready to inflict damage?
	anim = ( ent->client->ps.legsAnim & ~ANIM_TOGGLEBIT ) - BG_AnimationIndexForString( "legs_extra", cs->entityNum );
	if ( anim < 0 || anim >= NUM_LOPERMELEE_ANIMS ) {
		return AIFunc_DefaultStart( cs );
	}
	if ( cs->thinkFuncChangeTime < level.time - loperHitTimes[anim] ) {
		// check for damage
		// TTimo: gcc: suggests () around assignment used as truth value
		if ( ( tr = CheckMeleeAttack( &g_entities[cs->entityNum], LOPER_MELEE_RANGE, qfalse ) ) ) {
			G_Damage( &g_entities[tr->entityNum], ent, ent, vec3_origin, tr->endpos, LOPER_MELEE_DAMAGE, 0,
					  MOD_LOPER_HIT );
			// sound
			if ( anim == 0 ) {
				G_AddEvent( ent, EV_GENERAL_SOUND,
							G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[ORDERSDENYSOUNDSCRIPT] ) );
			} else {
				G_AddEvent( ent, EV_GENERAL_SOUND,
							G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[MISC1SOUNDSCRIPT] ) );
			}
		}
		cs->combat.weaponFireTimes[WP_MONSTER_ATTACK1] = level.time;
	}

	return NULL;
}

char *AIFunc_LoperMeleeStart( cast_state_t *cs ) {
	gentity_t *ent;
	//
	ent = &g_entities[cs->entityNum];
	// face them
	AICast_AimAtEnemy( cs );
	// start the animation
	if ( rand() % 2 ) {
		G_AddEvent( ent, EV_GENERAL_SOUND,
					G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[FOLLOWSOUNDSCRIPT] ) );
		BG_PlayAnimName( &ent->client->ps, "legs_extra", ANIM_BP_LEGS, qtrue, qfalse, qtrue );
	} else {
		G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[STAYSOUNDSCRIPT] ) );
		BG_PlayAnimName( &ent->client->ps, "legs_extra2", ANIM_BP_LEGS, qtrue, qfalse, qtrue );
	}
	//
	AI_SetLegacyState( cs, AIFunc_LoperMelee );
	return "AIFunc_LoperMelee";
}

//=================================================================================
//
// LOPER LEAP ATTACK
//
//=================================================================================

static const int LOPER_LEAP_DAMAGE = 10;
static const int LOPER_LEAP_DELAY = 100;
static const float LOPER_LEAP_RANGE = 200.0f;
static const float LOPER_LEAP_VELOCITY_START = 750.0f;
static const float LOPER_LEAP_VELOCITY_END = 1050.0f;
static const float LOPER_LEAP_VELOCITY_Z = 300.0f;
static const float LOPER_LEAP_LAND_MOMENTUM = 250.0f;
static const int LOPER_LAND_DURATION = 21 * ( 1000 / 15 );
static const int LOPER_FACE_TIMEOUT = 2000;
static const int LOPER_AIRBORNE_TIMEOUT = 8000;

static void AI_LoperSetPhase( cast_state_t *cs, AILoperLeapPhase phase ) {
	cs->ability.loperLeapPhase = phase;
	cs->ability.loperPhaseStartedAt = level.time;
}

bool AI_CanStartLoperLeap( const cast_state_t *cs ) {
	return cs->aiCharacter == AICHAR_LOPER && cs->script.castScriptStatus.scriptNoMoveTime < level.time &&
		   cs->script.castScriptStatus.scriptNoAttackTime < level.time;
}

char *AI_EnterLoperLeap( cast_state_t *cs ) {
	VectorClear( cs->ability.loperLeapVel );
	cs->ability.loperLegAnimation = -1;
	cs->aiFlags &= ~AIFL_LAND_ANIM_PLAYED;
	AI_LoperSetPhase( cs, AILoperLeapPhase::FaceTarget );
	return "LoperLeap";
}

void AI_ExitLoperLeap( cast_state_t *cs ) {
	// Release only the animation owned by this ability. Scripts may have replaced it.
	gentity_t *ent = &g_entities[cs->entityNum];
	const int animation = ent->client->ps.legsAnim;
	if ( cs->ability.loperLeapPhase != AILoperLeapPhase::None &&
		 cs->ability.loperLeapPhase != AILoperLeapPhase::FaceTarget && animation == cs->ability.loperLegAnimation ) {
		ent->client->ps.legsTimer = 0;
	}
	ent->client->ps.eFlags &= ~EF_MONSTER_EFFECT;
	cs->aiFlags &= ~AIFL_LAND_ANIM_PLAYED;
	VectorClear( cs->ability.loperLeapVel );
	cs->ability.loperLegAnimation = -1;
	AI_LoperSetPhase( cs, AILoperLeapPhase::None );
}

static char *AI_LoperFinishLeap( cast_state_t *cs, AITransitionReason reason ) {
	return AI_RequestState( cs, AI_HasLivingTarget( cs ) ? AIStateId::Combat : AIStateId::Idle, reason );
}

static void AI_LoperLaunch( cast_state_t *cs, const vec3_t direction ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	if ( BG_PlayAnimName( &ent->client->ps, "legs_extra3", ANIM_BP_LEGS, qtrue, qfalse, qtrue ) < 0 ) {
		return; // A higher priority animation still owns the legs.
	}
	cs->ability.loperLegAnimation = ent->client->ps.legsAnim;
	ent->client->ps.legsTimer = 500;
	VectorScale( direction, LOPER_LEAP_VELOCITY_START, ent->client->ps.velocity );
	ent->client->ps.velocity[2] = LOPER_LEAP_VELOCITY_Z;
	VectorCopy( ent->client->ps.velocity, cs->ability.loperLeapVel );
	cs->combat.weaponFireTimes[WP_MONSTER_ATTACK2] = level.time;
	AI_LoperSetPhase( cs, AILoperLeapPhase::Airborne );
}

static void AI_LoperLeapDamage( cast_state_t *cs ) {
	if ( !AI_HasLivingTarget( cs ) || cs->clientState->cur_ps.groundEntityNum != ENTITYNUM_NONE ||
		 level.time - cs->ability.loperPhaseStartedAt <= LOPER_LEAP_DELAY ||
		 level.time - cs->combat.weaponFireTimes[WP_MONSTER_ATTACK2] <= 50 ) {
		return;
	}
	gentity_t *ent = &g_entities[cs->entityNum];
	gentity_t *target = &g_entities[cs->enemyNum];
	if ( VectorDistance( cs->clientState->origin, target->client->ps.origin ) >= LOPER_LEAP_RANGE ) {
		return;
	}
	ent->client->ps.eFlags |= EF_MONSTER_EFFECT;
	G_Damage( target, ent, ent, vec3_origin, cs->clientState->origin, LOPER_LEAP_DAMAGE, 0, MOD_LOPER_LEAP );
	G_Sound( ent, level.loperZapSound );
	cs->combat.weaponFireTimes[WP_MONSTER_ATTACK2] = level.time;
}

static bool AI_LoperPredictLanding( cast_state_t *cs ) {
	if ( cs->clientState->cur_ps.groundEntityNum != ENTITYNUM_NONE ) {
		return true;
	}
	aicast_predictmove_t move = {};
	AICast_PredictMovement( cs, 1, 0.2f, &move, &cs->lastucmd, cs->enemyNum );
	return move.groundEntityNum != ENTITYNUM_NONE;
}

static void AI_LoperAccelerate( cast_state_t *cs ) {
	vec3_t direction;
	VectorCopy( cs->ability.loperLeapVel, direction );
	direction[2] = 0;
	VectorNormalize( direction );
	const float fraction = Com_Clamp( 0.0f, 1.0f, 0.002f * ( level.time - cs->ability.loperPhaseStartedAt ) );
	VectorScale( direction,
				 LOPER_LEAP_VELOCITY_START + fraction * ( LOPER_LEAP_VELOCITY_END - LOPER_LEAP_VELOCITY_START ),
				 direction );
	g_entities[cs->entityNum].s.pos.trDelta[0] = direction[0];
	g_entities[cs->entityNum].s.pos.trDelta[1] = direction[1];
}

char *AIFunc_LoperLeap( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	const int elapsed = level.time - cs->ability.loperPhaseStartedAt;
	switch ( cs->ability.loperLeapPhase ) {
	case AILoperLeapPhase::FaceTarget: {
		if ( !AI_HasLivingTarget( cs ) || elapsed >= LOPER_FACE_TIMEOUT ) {
			return AI_LoperFinishLeap( cs, AITransitionReason::TargetLost );
		}
		AICast_AimAtEnemy( cs );
		vec3_t direction, facing;
		VectorSubtract( g_entities[cs->enemyNum].client->ps.origin, cs->clientState->origin, direction );
		VectorNormalize( direction );
		AngleVectors( cs->viewangles, facing, NULL, NULL );
		if ( DotProduct( direction, facing ) >= 0.9f ) {
			AI_LoperLaunch( cs, direction );
		}
		return NULL;
	}
	case AILoperLeapPhase::Airborne:
		// Target loss stops damage, but landing must finish even if the enemy dies.
		AI_LoperLeapDamage( cs );
		if ( elapsed >= LOPER_AIRBORNE_TIMEOUT ) {
			return AI_LoperFinishLeap( cs, AITransitionReason::Completed );
		}
		if ( elapsed > LOPER_LEAP_DELAY && AI_LoperPredictLanding( cs ) ) {
			if ( BG_PlayAnimName( &ent->client->ps, "legs_extra4", ANIM_BP_LEGS, qtrue, qfalse, qtrue ) < 0 ) {
				return AI_LoperFinishLeap( cs, AITransitionReason::Script );
			}
			cs->ability.loperLegAnimation = ent->client->ps.legsAnim;
			AI_LoperSetPhase( cs, AILoperLeapPhase::Landing );
		} else {
			AI_LoperAccelerate( cs );
			ent->client->ps.legsTimer = 500;
		}
		return NULL;
	case AILoperLeapPhase::Landing: {
		if ( ent->client->ps.legsTimer < 800 || elapsed >= LOPER_LAND_DURATION ) {
			ent->client->ps.legsTimer = 0;
			AI_LoperSetPhase( cs, AILoperLeapPhase::Recovery );
			return NULL;
		}
		vec3_t direction;
		AngleVectors( cs->viewangles, direction, NULL, NULL );
		const float fraction = Com_Clamp( 0.0f, 1.0f, (float)ent->client->ps.legsTimer / LOPER_LAND_DURATION );
		trap_EA_Move( cs->entityNum, direction, fraction * LOPER_LEAP_LAND_MOMENTUM );
		return NULL;
	}
	case AILoperLeapPhase::Recovery:
		return AI_LoperFinishLeap( cs, AITransitionReason::Completed );
	default:
		return AI_LoperFinishLeap( cs, AITransitionReason::TargetLost );
	}
}

char *AIFunc_LoperLeapStart( cast_state_t *cs ) {
	return AI_RequestState( cs, AIStateId::LoperLeap, AITransitionReason::Decision );
}

//=================================================================================
//
// LOPER GROUND ATTACK
//
//=================================================================================

#define LOPER_GROUND_ANIM LEGS_EXTRA5
#define LOPER_GROUND_FPS 10

#define LOPER_GROUND_FRAME_COUNT 8
#define LOPER_GROUND_DURATION ( LOPER_GROUND_FRAME_COUNT * ( 1000 / LOPER_GROUND_FPS ) )

#define LOPER_GROUND_DAMAGE 20

/*
===============
AIFunc_LoperGroundZap()

  Loper's ground electrical attack
===============
*/
char *AIFunc_LoperGroundZap( cast_state_t *cs ) {
	gentity_t *ent;
	qboolean hitClient = qfalse;
	//
	ent = &g_entities[cs->entityNum];
	//
	// done with this attack?
	if ( !ent->client->ps.legsTimer ) {
		cs->pauseTime = level.time + 600; // don't move until effect is done
		ent->client->ps.legsTimer = 600; // stay down until effect is done
		return AIFunc_DefaultStart( cs );
	}
	// ready to inflict damage?
	if ( cs->thinkFuncChangeTime < level.time - 900 ) {
		//
		// draw the client-side lightning effect
		ent->client->ps.eFlags |= EF_MONSTER_EFFECT3;
		//ent->s.effect3Time = level.time + 500;//cs->thinkFuncChangeTime + LOPER_GROUND_DELAY - 200;
		//
		// are we waiting to inflict damage?
		if ( cs->combat.weaponFireTimes[WP_MONSTER_ATTACK3] < level.time - 100 ) {
			// check for damage
			hitClient = G_RadiusDamage( cs->clientState->origin, ent, LOPER_GROUND_DAMAGE, LOPER_GROUND_RANGE, ent,
										MOD_LOPER_GROUND );
			//
			cs->combat.weaponFireTimes[WP_MONSTER_ATTACK3] = level.time;
			// TODO: client-side visual effect
			// TODO: throw them backwards (away from us)
		} else {
			hitClient = qtrue; // so we don't abort
		}
		//
		if ( !hitClient && ( ent->client->ps.legsTimer > 400 ) &&
			 cs->thinkFuncChangeTime < ( level.time - 1000 ) ) { // we're done with this attack
			cs->pauseTime = level.time + 400; // don't move until effect is done
			ent->client->ps.legsTimer = 400; // stay down until effect is done
			return AIFunc_DefaultStart( cs );
		}
	}
	//
	return NULL;
}

char *AIFunc_LoperGroundZapStart( cast_state_t *cs ) {
	gentity_t *ent;
	//
	ent = &g_entities[cs->entityNum];
	//
	// face them
	AICast_AimAtEnemy( cs );
	// play the animation
	BG_PlayAnimName( &ent->client->ps, "legs_extra5", ANIM_BP_LEGS, qtrue, qfalse, qtrue );
	//
	// play the buildup sound
	// TODO
	//
	AI_SetLegacyState( cs, AIFunc_LoperGroundZap );
	return "AIFunc_LoperGroundZap";
}

// Ability selection and tuning belong to this creature.
static float AI_LoperAbilityRange( cast_state_t *cs, int weaponnum ) {
	switch ( weaponnum ) {
	case WP_MONSTER_ATTACK2:
		// loper leap attack
		return 8000; // use it to gain on them also
	case WP_MONSTER_ATTACK3:
		// loper ground attack
		return LOPER_GROUND_RANGE;
	default:
		return 3000;
	}
}

static qboolean AI_LoperAbilityAvailable( cast_state_t *cs, int weaponNum ) {
	int delay = -1;
	float dist = -1;
	if ( cs->enemyNum >= 0 ) {
		dist = Distance( cs->clientState->origin, g_entities[cs->enemyNum].s.pos.trBase );
	}
	switch ( weaponNum ) {
	case WP_MONSTER_ATTACK2: {
		// loper leap attack
		if ( cs->clientState->areanum && VectorLength( cs->clientState->velocity ) >
											 1 ) { // if we are in a valid area, and are persuing, then leave a delay
			// if there isn't a direct trace to our enemy, then fail
			if ( cs->enemyNum >= 0 ) {
				trace_t trace;
				vec3_t mins;
				VectorCopy( cs->clientState->cur_ps.mins, mins );
				mins[0] = 0;
				trap_Trace( &trace, g_entities[cs->entityNum].client->ps.origin, mins, cs->clientState->cur_ps.maxs,
							g_entities[cs->enemyNum].client->ps.origin, cs->entityNum,
							g_entities[cs->entityNum].clipmask );
				if ( trace.entityNum != cs->enemyNum && trace.fraction < 1.0 ) {
					return qfalse;
				}
			}
			delay = 4500;
			if ( dist < 200 ) {
				return qfalse;
			}
		} else {
			delay = 0; // jump to get out of trouble
		}
		break;
	}
	case WP_MONSTER_ATTACK3: {
		// loper ground zap
		delay = 3500;
		if ( dist < 0 || dist > LOPER_GROUND_RANGE ) {
			return qfalse;
		}
		break;
	}
	default:
		return qfalse;
	}
	return !cs->combat.weaponFireTimes[weaponNum] || cs->combat.weaponFireTimes[weaponNum] < level.time - delay;
}

static const ai_ability_definition_t loperDefinitions[] = {
	{ "LoperLeap", WP_MONSTER_ATTACK2, AIFunc_LoperLeapStart, AI_LoperAbilityRange, AI_LoperAbilityAvailable },
	{ "LoperGroundZap", WP_MONSTER_ATTACK3, AIFunc_LoperGroundZapStart, AI_LoperAbilityRange,
	  AI_LoperAbilityAvailable },
};

const ai_ability_set_t ai_LoperAbilities = { loperDefinitions,
											 sizeof( loperDefinitions ) / sizeof( loperDefinitions[0] ) };
