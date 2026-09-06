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
// BLACK GUARD MELEE KICK ATTACK
//
//=================================================================================

#define BLACKGUARD_KICK_DELAY 300
#define BLACKGUARD_KICK_RANGE BLACKGUARD_MELEE_RANGE + 16
#define BLACKGUARD_KICK_DAMAGE 25

char *AIFunc_BlackGuardKick( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	trace_t *tr;
	vec3_t fwd;

	if ( !ent->client->ps.legsTimer ) {
		return AIFunc_DefaultStart( cs );
	}
	//
	if ( cs->enemyNum < 0 ) {
		return NULL;
	}
	// time for the melee?
	if ( !cs->ability.meleeHitApplied ) {
		// face them
		AICast_AimAtEnemy( cs );
		// ready for damage?
		if ( cs->thinkFuncChangeTime < level.time - BLACKGUARD_KICK_DELAY ) {
			cs->ability.meleeHitApplied = qtrue;
			// keep checking for impact status
			tr = CheckMeleeAttack( ent, BLACKGUARD_KICK_RANGE, qfalse );
			// do melee damage?
			if ( tr && ( tr->entityNum == cs->enemyNum ) ) {
				AngleVectors( cs->viewangles, fwd, NULL, NULL );
				G_Damage( &g_entities[tr->entityNum], ent, ent, fwd, tr->endpos, BLACKGUARD_KICK_DAMAGE, 0,
						  MOD_GAUNTLET );
				// throw them in direction of impact
				fwd[2] = 0.5;
				VectorMA( g_entities[cs->enemyNum].client->ps.velocity, 300, fwd,
						  g_entities[cs->enemyNum].client->ps.velocity );
			}
		}
	}

	return NULL;
}

char *AIFunc_BlackGuardKickStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	//
	cs->combat.weaponFireTimes[cs->weaponNum] = level.time;
	// face them
	AICast_AimAtEnemy( cs );
	// audible sound
	AIChar_AttackSound( cs );
	// start the animation
	BG_PlayAnimName( &ent->client->ps, "kick", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
	// clear flags
	cs->ability.meleeHitApplied = qfalse;
	//
	AI_SetLegacyState( cs, AIFunc_BlackGuardKick );
	return "AIFunc_BlackGuardKick";
}

// Ability selection and tuning belong to this creature.
static float AI_BlackGuardAbilityRange( cast_state_t *cs, int weaponnum ) {
	switch ( weaponnum ) {
	case WP_MONSTER_ATTACK1:
		return BLACKGUARD_MELEE_RANGE;
	default:
		return 3000;
	}
}

static qboolean AI_BlackGuardAbilityAvailable( cast_state_t *cs, int weaponNum ) {
	int delay = -1;
	float dist = -1;
	if ( cs->enemyNum >= 0 ) {
		dist = Distance( cs->clientState->origin, g_entities[cs->enemyNum].s.pos.trBase );
	}
	switch ( weaponNum ) {
	case WP_MONSTER_ATTACK1: {
		delay = 5000;
		if ( dist < 0 || dist > BLACKGUARD_MELEE_RANGE ) {
			return qfalse;
		}
		break;
	}
	default:
		return qfalse;
	}
	return !cs->combat.weaponFireTimes[weaponNum] || cs->combat.weaponFireTimes[weaponNum] < level.time - delay;
}

static const ai_ability_definition_t blackguardDefinitions[] = {
	{ "BlackGuardKick", WP_MONSTER_ATTACK1, AIFunc_BlackGuardKickStart, AI_BlackGuardAbilityRange,
	  AI_BlackGuardAbilityAvailable },
};

const ai_ability_set_t ai_BlackGuardAbilities = { blackguardDefinitions, sizeof( blackguardDefinitions ) /
																			 sizeof( blackguardDefinitions[0] ) };
