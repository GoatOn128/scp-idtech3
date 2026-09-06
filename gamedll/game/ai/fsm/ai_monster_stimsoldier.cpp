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
// STIM SOLDIER FLYING ATTACK
//
//=================================================================================

#define STIMSOLDIER_FLYJUMP_ANIM LEGS_EXTRA1
#define STIMSOLDIER_FLYJUMP_FPS 15
#define STIMSOLDIER_FLYJUMP_FRAME_COUNT 28
#define STIMSOLDIER_FLYJUMP_DURATION ( STIMSOLDIER_FLYJUMP_FRAME_COUNT * ( 1000 / STIMSOLDIER_FLYJUMP_FPS ) )
#define STIMSOLDIER_FLYJUMP_DELAY ( STIMSOLDIER_FLYJUMP_DURATION + 3000 )

// hover plays continuously
#define STIMSOLDIER_FLYHOVER_ANIM LEGS_EXTRA2
#define STIMSOLDIER_FLYHOVER_FPS 5

#define STIMSOLDIER_FLYLAND_ANIM LEGS_LAND
#define STIMSOLDIER_FLYLAND_FPS 15
#define STIMSOLDIER_FLYLAND_FRAME_COUNT 14
#define STIMSOLDIER_FLYLAND_DURATION ( STIMSOLDIER_FLYLAND_FRAME_COUNT * ( 1000 / STIMSOLDIER_FLYLAND_FPS ) )

#define STIMSOLDIER_STARTJUMP_DELAY ( STIMSOLDIER_FLYJUMP_DURATION * 0.5 )

char *AIFunc_StimSoldierFlight( cast_state_t *cs ) {
	gentity_t *ent;
	vec3_t vec;
	static vec3_t up = { 0, 0, 1 };
	//
	ent = &g_entities[cs->entityNum];
	cs->combat.weaponFireTimes[WP_MONSTER_ATTACK1] = level.time;
	// face them
	AICast_AimAtEnemy( cs );
	//
	// are we done with this attack?
	if ( cs->thinkFuncChangeTime < level.time - STIMSOLDIER_FLYJUMP_DELAY ) {
		// have we hit the ground yet?
		if ( ent->s.groundEntityNum != ENTITYNUM_NONE ) {
			// we are on something, have we started the landing animation?
			if ( !( cs->aiFlags & AIFL_LAND_ANIM_PLAYED ) ) {
				ent->client->ps.legsAnim =
					( ( ent->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | STIMSOLDIER_FLYLAND_ANIM;
				ent->client->ps.legsTimer = STIMSOLDIER_FLYLAND_DURATION; // stay down until attack is finished
				//
				cs->combat.noAttackTime = level.time + STIMSOLDIER_FLYLAND_DURATION;
				cs->aiFlags |= AIFL_LAND_ANIM_PLAYED;
			} else {
				if ( !ent->client->ps.legsTimer ) { // animation has finished, resume AI
					return AIFunc_DefaultStart( cs );
				}
			}
		} else {
			// still flying
		}
		return NULL;
	}
	//
	// are we ready to start flying?
	if ( cs->thinkFuncChangeTime < ( level.time - STIMSOLDIER_STARTJUMP_DELAY ) ) {
		if ( !ent->client->ps.powerups[PW_FLIGHT] ) {
			// play a special ignition sound?
		}
		ent->client->ps.powerups[PW_FLIGHT] = 1; // let them fly
		ent->s.loopSound = level.stimSoldierFlySound;
		ent->client->ps.eFlags |= EF_MONSTER_EFFECT; // client-side stim engine effect
		if ( ent->s.effect1Time != ( cs->thinkFuncChangeTime + STIMSOLDIER_STARTJUMP_DELAY ) ) {
			ent->s.effect1Time = ( cs->thinkFuncChangeTime + STIMSOLDIER_STARTJUMP_DELAY );
			// start the hovering animation
			ent->client->ps.legsAnim =
				( ( ent->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | STIMSOLDIER_FLYHOVER_ANIM;
		}
		// give us some upwards velocity?
		if ( cs->thinkFuncChangeTime > level.time - STIMSOLDIER_FLYJUMP_DURATION * 0.9 ) {
			trap_EA_Move( cs->entityNum, up, 300 );
			//trap_EA_Jump(cs->entityNum);
			VectorCopy( cs->clientState->origin, cs->ability.stimFlyAttackPos );
		} else {
			// attack them
			//
			// if we can't attack, abort
			if ( AICast_CheckAttack( cs, cs->enemyNum, qfalse ) ) {
				// apply weapons..
				trap_EA_Attack( cs->entityNum );
			}
			// we're done here
			cs->thinkFuncChangeTime = -9999;
		}
	} else {
		// still on ground, so move forward to account for stepping animation
		AngleVectors( cs->viewangles, vec, NULL, NULL );
		trap_EA_Move( cs->entityNum, vec, 300 );
	}
	//
	if ( ent->client->ps.legsTimer < 1000 ) {
		ent->client->ps.legsTimer = 1000; // stay down until effect is done
	}
	//
	return NULL;
}

char *AIFunc_StimSoldierFlightStart( cast_state_t *cs ) {
	gentity_t *ent;
	//static vec3_t mins={-96,-96,0}, maxs={96,96,72};
	vec3_t pos, dir;
	trace_t tr;
	//
	cs->combat.weaponFireTimes[cs->weaponNum] = level.time;
	ent = &g_entities[cs->entityNum];
	//
	// face them
	AICast_AimAtEnemy( cs );
	// first, check if this is a good place to start the flying attack
	AngleVectors( cs->ideal_viewangles, dir, NULL, NULL );
	VectorMA( cs->clientState->origin, 300, dir, pos );
	pos[2] += 128;
	trap_Trace( &tr, cs->clientState->origin, cs->clientState->cur_ps.mins, cs->clientState->cur_ps.maxs, pos,
				cs->entityNum, ent->clipmask );
	if ( tr.startsolid || tr.allsolid ) {
		return NULL; // not a good place
	}
	// check we can attack them from there
	// select our special weapon (rocket launcher or tesla)
	if ( COM_BitCheck( cs->clientState->cur_ps.weapons, WP_PANZERFAUST ) ) {
		cs->weaponNum = WP_PANZERFAUST;
	} else if ( COM_BitCheck( cs->clientState->cur_ps.weapons, WP_TESLA ) ) {
		cs->weaponNum = WP_TESLA;
	} else { // no weapon?
		G_Error( "stim soldier tried special jump attack without a tesla or rocket launcher\n" );
	}
	if ( !AICast_CheckAttackAtPos( cs->entityNum, cs->enemyNum, pos, qfalse, qfalse ) ) {
		AICast_ChooseWeapon( cs, qfalse );
		return NULL;
	}
	// play the animation
	ent->client->ps.legsAnim =
		( ( ent->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | STIMSOLDIER_FLYJUMP_ANIM;
	ent->client->ps.legsTimer = STIMSOLDIER_FLYJUMP_DELAY; // stay down until attack is finished
	//
	cs->aiFlags &= ~AIFL_LAND_ANIM_PLAYED;
	// play the buildup sound
	// TODO
	//
	AI_SetLegacyState( cs, AIFunc_StimSoldierFlight );
	return "AIFunc_StimSoldierFlight";
}

//=================================================================================
//
// STIM SOLDIER DUAL MACHINEGUN ATTACK
//
//=================================================================================

char *AIFunc_StimSoldierDualMachinegun( cast_state_t *cs ) {
	return AIFunc_DefaultStart( cs );
}

char *AIFunc_StimSoldierDualMachinegunStart( cast_state_t *cs ) {
	// Reserved until the dual-machinegun behavior is implemented.
	return NULL;
}

// Ability selection and tuning belong to this creature.
static float AI_StimRocketAbilityRange( cast_state_t *cs, int weaponnum ) {
	switch ( weaponnum ) {
	case WP_MONSTER_ATTACK1:
		return 3000;
	default:
		return 3000;
	}
}

static qboolean AI_StimRocketAbilityAvailable( cast_state_t *cs, int weaponNum ) {
	int delay = -1;
	float dist = -1;
	if ( cs->enemyNum >= 0 ) {
		dist = Distance( cs->clientState->origin, g_entities[cs->enemyNum].s.pos.trBase );
	}
	switch ( weaponNum ) {
	case WP_MONSTER_ATTACK1: {
		delay = 7000;
		if ( dist < 0 || dist < 300 ) {
			return qfalse;
		}
		break;
	}
	default:
		return qfalse;
	}
	return !cs->combat.weaponFireTimes[weaponNum] || cs->combat.weaponFireTimes[weaponNum] < level.time - delay;
}

static const ai_ability_definition_t stimrocketDefinitions[] = {
	{ "StimRocketFlight", WP_MONSTER_ATTACK1, AIFunc_StimSoldierFlightStart, AI_StimRocketAbilityRange,
	  AI_StimRocketAbilityAvailable },
};

const ai_ability_set_t ai_StimRocketAbilities = { stimrocketDefinitions, sizeof( stimrocketDefinitions ) /
																			 sizeof( stimrocketDefinitions[0] ) };

// Ability selection and tuning belong to this creature.
static float AI_StimTeslaAbilityRange( cast_state_t *cs, int weaponnum ) {
	switch ( weaponnum ) {
	case WP_MONSTER_ATTACK1:
		return TESLA_RANGE;
	default:
		return 3000;
	}
}

static qboolean AI_StimTeslaAbilityAvailable( cast_state_t *cs, int weaponNum ) {
	int delay = -1;
	float dist = -1;
	if ( cs->enemyNum >= 0 ) {
		dist = Distance( cs->clientState->origin, g_entities[cs->enemyNum].s.pos.trBase );
	}
	switch ( weaponNum ) {
	case WP_MONSTER_ATTACK1: {
		// stim flying tesla attack
		delay = 7000;
		if ( dist < 0 || dist < 300 ) {
			return qfalse;
		}
		break;
	}
	default:
		return qfalse;
	}
	return !cs->combat.weaponFireTimes[weaponNum] || cs->combat.weaponFireTimes[weaponNum] < level.time - delay;
}

static const ai_ability_definition_t stimteslaDefinitions[] = {
	{ "StimTeslaFlight", WP_MONSTER_ATTACK1, AIFunc_StimSoldierFlightStart, AI_StimTeslaAbilityRange,
	  AI_StimTeslaAbilityAvailable },
};

const ai_ability_set_t ai_StimTeslaAbilities = { stimteslaDefinitions,
												 sizeof( stimteslaDefinitions ) / sizeof( stimteslaDefinitions[0] ) };
