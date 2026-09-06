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
// WARRIOR ZOMBIE
//
// Standing melee attacks
//
//=================================================================================

int warriorHitDamage[5] = { 16, 16, 16, 12, 20 };

#define NUM_WARRIOR_ANIMS 5
int warriorHitTimes[NUM_WARRIOR_ANIMS][3] = {
	// up to three hits per attack
	{ ANIMLENGTH( 10, 20 ), -1 }, { ANIMLENGTH( 15, 20 ), -1 }, { ANIMLENGTH( 18, 20 ), -1 },
	{ ANIMLENGTH( 15, 20 ), -1 }, { ANIMLENGTH( 14, 20 ), -1 },
};

/*
================
AIFunc_WarriorZombieMelee
================
*/
char *AIFunc_WarriorZombieMelee( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	int hitDelay = -1, anim;
	trace_t *tr;
	cast_state_t *ecs = AICast_GetCastState( cs->enemyNum );
	aicast_predictmove_t move;
	float enemyDist;

	if ( !ent->client->ps.torsoTimer ) {
		return AIFunc_DefaultStart( cs );
	}
	//
	if ( cs->enemyNum < 0 ) {
		return NULL;
	}
	if ( ecs ) {

		anim = ( ent->client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) - BG_AnimationIndexForString( "attack1", cs->entityNum );
		if ( anim < 0 || anim >= NUM_WARRIOR_ANIMS ) {
			// animation interupted
			return AIFunc_DefaultStart( cs );
		}
		if ( cs->ability.animHitCount < 3 && warriorHitTimes[anim][cs->ability.animHitCount] >= 0 ) {

			if ( !cs->ability.animHitCount ) {
				hitDelay = warriorHitTimes[anim][cs->ability.animHitCount];
			} else {
				hitDelay = warriorHitTimes[anim][cs->ability.animHitCount] -
						   warriorHitTimes[anim][cs->ability.animHitCount - 1];
			}

			// check for inflicting damage
			if ( level.time - cs->combat.weaponFireTimes[cs->weaponNum] > hitDelay ) {
				// do melee damage
				if ( ( tr = CheckMeleeAttack( ent, 44, qfalse ) ) && ( tr->entityNum == cs->enemyNum ) ) {
					G_Damage( &g_entities[tr->entityNum], ent, ent, vec3_origin, tr->endpos, warriorHitDamage[anim], 0,
							  MOD_GAUNTLET );
					G_AddEvent( ent, EV_GENERAL_SOUND,
								G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[STAYSOUNDSCRIPT] ) );
				} else {
					G_AddEvent( ent, EV_GENERAL_SOUND,
								G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[FOLLOWSOUNDSCRIPT] ) );
				}
				cs->combat.weaponFireTimes[cs->weaponNum] = level.time;
				cs->ability.animHitCount++;
			}
		}
		// face them
		AICast_AimAtEnemy( cs );
		if ( anim < 3 ) { // back handed-swinging, dont allow legs to move
			// if they are outside range, move forward
			AICast_PredictMovement( ecs, 2, 0.5, &move, &g_entities[cs->enemyNum].client->pers.cmd, -1 );
			enemyDist = Distance( move.endpos, cs->clientState->origin );
			enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
			enemyDist -= ent->r.maxs[0];
			if ( enemyDist > 16 ) { // we can get closer
				if ( ent->client->ps.legsTimer ) {
					ent->client->ps.legsTimer = 0; // allow legs us to move
					if ( cs->script.castScriptStatus.scriptNoMoveTime <
						 level.time + 200 ) { // dont move until the legs are done lerping out of attack anim
						cs->script.castScriptStatus.scriptNoMoveTime = level.time + 200;
					}
				}
				if ( cs->script.castScriptStatus.scriptNoMoveTime < level.time ) {
					trap_EA_MoveForward( cs->entityNum );
				}
			}
		}
	}

	return NULL;
}

/*
================
AIFunc_WarriorZombieMeleeStart
================
*/
char *AIFunc_WarriorZombieMeleeStart( cast_state_t *cs ) {
	gentity_t *ent;

	ent = &g_entities[cs->entityNum];
	ent->s.effect1Time = level.time;
	cs->ideal_viewangles[YAW] = cs->viewangles[YAW];
	cs->combat.weaponFireTimes[cs->weaponNum] = level.time;
	cs->ability.animHitCount = 0;

	// face them
	AICast_AimAtEnemy( cs );

	// audible sound
	AIChar_AttackSound( cs );

	// play an anim
	BG_UpdateConditionValue( cs->entityNum, ANIM_COND_WEAPON, cs->weaponNum, qtrue );
	BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );

	// stop charging
	BG_UpdateConditionValue( cs->entityNum, ANIM_COND_CHARGING, 0, qfalse );
	ent->flags &= ~FL_WARZOMBIECHARGE;

	AI_SetLegacyState( cs, AIFunc_WarriorZombieMelee );
	return "AIFunc_WarriorZombieMelee";
}

// Warrior "sight" animation
/*
================
AIFunc_WarriorZombieSight
================
*/
char *AIFunc_WarriorZombieSight( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];

	if ( !ent->client->ps.torsoTimer ) {
		return AIFunc_DefaultStart( cs );
	}
	return NULL;
}

/*
================
AIFunc_WarriorZombieSightStart
================
*/
char *AIFunc_WarriorZombieSightStart( cast_state_t *cs ) {
	gentity_t *ent;

	// RF, disabled
	return NULL;

	ent = &g_entities[cs->entityNum];
	cs->ideal_viewangles[YAW] = cs->viewangles[YAW];
	cs->combat.weaponFireTimes[cs->weaponNum] = level.time;

	// face them
	AICast_AimAtEnemy( cs );

	// anim
	BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_FIRSTSIGHT, qfalse, qtrue );
	//BG_PlayAnimName( &ent->client->ps, "first_sight", ANIM_BP_BOTH, qtrue, qfalse, qtrue );

	AI_SetLegacyState( cs, AIFunc_WarriorZombieSight );
	return "AIFunc_WarriorZombieSight";
}

/*
================
AIFunc_WarriorZombieDefense
================
*/
char *AIFunc_WarriorZombieDefense( cast_state_t *cs ) {
	gentity_t *ent, *enemy;
	vec3_t enemyDir, vec;
	float dist;

	ent = &g_entities[cs->entityNum];

	if ( !( ent->flags & FL_DEFENSE_GUARD ) ) {
		if ( cs->combat.weaponFireTimes[cs->weaponNum] < level.time - 100 ) {
			return AIFunc_DefaultStart( cs );
		}
		return NULL;
	}

	if ( ( cs->enemyNum < 0 ) || ( cs->dangerEntityValidTime >= level.time ) ) {
		ent->flags &= ~FL_DEFENSE_GUARD;
		ent->client->ps.torsoTimer = 0;
		ent->client->ps.legsTimer = 0;
		return NULL;
	}

	enemy = &g_entities[cs->enemyNum];

	if ( cs->thinkFuncChangeTime < level.time - 1500 ) {
		// if we cant see them
		if ( !AICast_EntityVisible( cs, cs->enemyNum, qtrue ) ) {
			ent->flags &= ~FL_DEFENSE_GUARD;
			ent->client->ps.torsoTimer = 0;
			ent->client->ps.legsTimer = 0;
			return NULL;
		}

		// if our enemy isn't using a dangerous weapon
		if ( enemy->client->ps.weapon < WP_LUGER || enemy->client->ps.weapon > WP_CLASS_SPECIAL ) {
			ent->flags &= ~FL_DEFENSE_GUARD;
			ent->client->ps.torsoTimer = 0;
			ent->client->ps.legsTimer = 0;
			return NULL;
		}

		// if our enemy isn't looking right at us, abort
		VectorSubtract( ent->client->ps.origin, enemy->client->ps.origin, vec );
		dist = VectorNormalize( vec );
		if ( dist > 512 ) {
			dist = 512;
		}
		AngleVectors( enemy->client->ps.viewangles, enemyDir, NULL, NULL );
		if ( DotProduct( vec, enemyDir ) < ( 0.98 - 0.2 * ( dist / 512 ) ) ) {
			ent->flags &= ~FL_DEFENSE_GUARD;
			ent->client->ps.torsoTimer = 0;
			ent->client->ps.legsTimer = 0;
			return NULL;
		}
	}

	cs->combat.weaponFireTimes[cs->weaponNum] = level.time;

	if ( !ent->client->ps.torsoTimer ) {
		ent->flags &= ~FL_DEFENSE_GUARD;
		ent->client->ps.torsoTimer = 0;
		ent->client->ps.legsTimer = 0;
		return NULL;
	}

	// face them
	AICast_AimAtEnemy( cs );
	// crouching position, use smaller bounding box
	trap_EA_Crouch( cs->clientState->client );

	return NULL;
}

/*
================
AIFunc_WarriorZombieDefenseStart
================
*/
char *AIFunc_WarriorZombieDefenseStart( cast_state_t *cs ) {
	gentity_t *ent, *enemy;
	vec3_t enemyDir, vec;
	float dist;
	static int lastWarriorDefense;

	if ( lastWarriorDefense <= level.time && lastWarriorDefense > level.time - 3000 ) {
		return NULL; // dont all go into defense at once
	}
	lastWarriorDefense = level.time;

	ent = &g_entities[cs->entityNum];
	enemy = &g_entities[cs->enemyNum];

	// if our enemy isn't using a dangerous weapon
	if ( enemy->client->ps.weapon < WP_LUGER || enemy->client->ps.weapon > WP_CLASS_SPECIAL ) {
		return NULL;
	}

	// if we are doing a goto
	if ( cs->navigation.followEntity >= 0 ) {
		return NULL;
	}

	// if our enemy isn't looking right at us, abort
	VectorSubtract( ent->client->ps.origin, enemy->client->ps.origin, vec );
	dist = VectorNormalize( vec );
	if ( dist > 512 ) {
		dist = 512;
	}
	if ( dist < 128 ) {
		return NULL;
	}
	AngleVectors( enemy->client->ps.viewangles, enemyDir, NULL, NULL );
	if ( DotProduct( vec, enemyDir ) < ( 0.98 - 0.2 * ( dist / 512 ) ) ) {
		return NULL;
	}

	cs->combat.weaponFireTimes[cs->weaponNum] = level.time;

	// face them
	AICast_AimAtEnemy( cs );

	// anim
	BG_UpdateConditionValue( cs->entityNum, ANIM_COND_WEAPON, cs->weaponNum, qtrue );
	BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );
	ent->client->ps.torsoTimer = 3000;
	ent->client->ps.legsTimer = 3000;

	ent->flags |= FL_DEFENSE_GUARD;

	// when they come out of defense mode, go into charge mode
	BG_UpdateConditionValue( cs->entityNum, ANIM_COND_CHARGING, 1, qfalse );
	ent->flags |= FL_WARZOMBIECHARGE;

	AI_SetLegacyState( cs, AIFunc_WarriorZombieDefense );
	return "AIFunc_WarriorZombieDefense";
}

// Ability selection and tuning belong to this creature.
static float AI_WarZombieAbilityRange( cast_state_t *cs, int weaponnum ) {
	switch ( weaponnum ) {
	case WP_MONSTER_ATTACK1:
		return 80; // make it larger so we can start swinging early, and move in while swinging
	case WP_MONSTER_ATTACK3:
		// warzombie defense
		return 2000;
	default:
		return 3000;
	}
}

static qboolean AI_WarZombieAbilityAvailable( cast_state_t *cs, int weaponNum ) {
	int delay = -1;
	float dist = -1;
	if ( cs->enemyNum >= 0 ) {
		dist = Distance( cs->clientState->origin, g_entities[cs->enemyNum].s.pos.trBase );
	}
	switch ( weaponNum ) {
	case WP_MONSTER_ATTACK1: {
		return qtrue; // always usable
	}
	case WP_MONSTER_ATTACK3: {
		// warzombie defense
		delay = 7000;
		if ( dist < 120 || dist > 2000 ) {
			return qfalse;
		}
		break;
	}
	default:
		return qfalse;
	}
	return !cs->combat.weaponFireTimes[weaponNum] || cs->combat.weaponFireTimes[weaponNum] < level.time - delay;
}

static const ai_ability_definition_t warzombieDefinitions[] = {
	{ "WarZombieMelee", WP_MONSTER_ATTACK1, AIFunc_WarriorZombieMeleeStart, AI_WarZombieAbilityRange,
	  AI_WarZombieAbilityAvailable },
	{ "WarZombieDefense", WP_MONSTER_ATTACK3, AIFunc_WarriorZombieDefenseStart, AI_WarZombieAbilityRange,
	  AI_WarZombieAbilityAvailable },
};

const ai_ability_set_t ai_WarZombieAbilities = { warzombieDefinitions,
												 sizeof( warzombieDefinitions ) / sizeof( warzombieDefinitions[0] ) };
