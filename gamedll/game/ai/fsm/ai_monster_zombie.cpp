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
// ZOMBIE SPECIAL ATTACKS
//
//=================================================================================

/*
============
AIFunc_ZombieFlameAttack()

  Zombie "Flaming Bats" attack.

  NOTE: this actually uses the EFFECT3 slot for client-side effects (others are taken)
============
*/

#define ZOMBIE_FLAME_DURATION 4000

char *AIFunc_ZombieFlameAttack( cast_state_t *cs ) {
	ai_client_state_t *bs;
	gentity_t *ent;
	//
	ent = &g_entities[cs->entityNum];
	bs = cs->clientState;
	//
	ent->s.onFireEnd = level.time + 2000;
	//
	if ( ent->health < 0 ) {
		ent->s.onFireEnd = 0;
		return AIFunc_DefaultStart( cs );
	}
	//
	if ( cs->enemyNum < 0 ) {
		ent->s.onFireEnd = level.time + 1500;
		ent->client->ps.torsoTimer = 0;
		ent->client->ps.legsTimer = 0;
		return AIFunc_DefaultStart( cs );
	}
	/*	disabled, keep going so they cant come back for the easy kill
	//
	// if we can't see them anymore, abort immediately
	if (cs->perception.vislist[cs->enemyNum].real_visible_timestamp != cs->perception.vislist[cs->enemyNum].real_update_timestamp) {
		ent->s.onFireEnd = level.time + 1500;
		ent->client->ps.torsoTimer = 0;
		ent->client->ps.legsTimer = 0;
		return AIFunc_DefaultStart( cs );
	}
*/
	// if outside range, move closer
	if ( VectorDistance( cs->clientState->origin, cs->perception.vislist[cs->enemyNum].visible_pos ) >
		 ZOMBIE_FLAME_RADIUS ) {
		ent->s.onFireEnd = level.time + 1500;
		ent->client->ps.torsoTimer = 0;
		ent->client->ps.legsTimer = 0;
		return AIFunc_DefaultStart( cs );
	}
	// we are firing this weapon, so record it
	cs->combat.weaponFireTimes[WP_MONSTER_ATTACK1] = level.time;
	// once an attack has started, only abort once the player leaves our view, or time runs out
	if ( cs->thinkFuncChangeTime < level.time - ZOMBIE_FLAME_DURATION ) {

		// finish this attack
		ent->client->ps.torsoTimer = 0;
		ent->client->ps.legsTimer = 0;
		return AIFunc_DefaultStart( cs );

	} else {

		//ent->client->ps.torsoTimer = 400;
		//ent->client->ps.legsTimer = 400;

		// draw the client-side effect
		ent->client->ps.eFlags |= EF_MONSTER_EFFECT3;

		// inform the client of our enemies position
		//VectorCopy( g_entities[cs->enemyNum].client->ps.origin, ent->s.origin2 );
		//ent->s.origin2[2] += g_entities[cs->enemyNum].client->ps.viewheight;

		// keep facing them
		AICast_AimAtEnemy( cs );

		// look slightly downwards since animation is facing upwards slightly
		cs->ideal_viewangles[PITCH] += 20;
	}
	//
	//
	return NULL;
}

char *AIFunc_ZombieFlameAttackStart( cast_state_t *cs ) {
	gentity_t *ent;
	//
	ent = &g_entities[cs->entityNum];
	ent->s.otherEntityNum2 = cs->enemyNum;
	ent->s.effect3Time = level.time;
	//
	// dont turn
	cs->ideal_viewangles[YAW] = cs->viewangles[YAW];
	//cs->ideal_viewangles[PITCH] = -45;	// look upwards
	// start the flame
	ent->s.onFireStart = level.time;
	ent->s.onFireEnd = level.time + ZOMBIE_FLAME_DURATION;
	//
	// set the correct animation
	BG_PlayAnimName( &ent->client->ps, "both_attack1", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
	ent->client->ps.legsTimer = ZOMBIE_FLAME_DURATION;
	ent->client->ps.torsoTimer = ZOMBIE_FLAME_DURATION;
	//
	AI_SetLegacyState( cs, AIFunc_ZombieFlameAttack );
	return "AIFunc_ZombieFlameAttack";
}

/*
============
AIFunc_ZombieSpirit()

  Zombie "Evil Spirit" attack.

  Character draws the light from surrounding walls (expanding negative light) and builds
  up to the release of a flying translucent skull with trail effect (and beady eyes).

  Spirit should track it's enemy slightly, inflicting lots of damage, removing sprint bar,
  and effecting sight temporarily.

  Speed of spirit is effected by skill level, higher skill = faster speed

  Spirits inflicting AI soldiers should kill instantly, removing all flesh from the
  soldier's face (draw skull under head model, then fade head model away over a short period).
============
*/
extern void weapon_zombiespirit( gentity_t *ent, gentity_t *missile );

#define ZOMBIE_SPIRIT_BUILDUP_TIME 6000 // last for this long
#define ZOMBIE_SPIRIT_FADEOUT_TIME 1000
#define ZOMBIE_SPIRIT_DLIGHT_RADIUS_MAX 256
#define ZOMBIE_SPIRIT_FIRE_INTERVAL 1000

char *AIFunc_ZombieSpirit( cast_state_t *cs ) {
	ai_client_state_t *bs;
	gentity_t *ent;
	//
	ent = &g_entities[cs->entityNum];
	bs = cs->clientState;
	//
	if ( cs->enemyNum < 0 ) {
		return AIFunc_DefaultStart( cs );
	}
	//
	// if we can't see them anymore, abort immediately
	if ( cs->perception.vislist[cs->enemyNum].real_visible_timestamp !=
		 cs->perception.vislist[cs->enemyNum].real_update_timestamp ) {
		return AIFunc_DefaultStart( cs );
	}
	//
	ai_encounter.lastSpiritAttackTime = level.time;
	// we are firing this weapon, so record it
	cs->combat.weaponFireTimes[WP_MONSTER_ATTACK2] = level.time;
	// once an attack has started, only abort once the player leaves our view, or time runs out
	if ( cs->thinkFuncChangeTime < level.time - ZOMBIE_SPIRIT_BUILDUP_TIME ) {
		// if enough time has elapsed, finish this attack
		if ( level.time > cs->thinkFuncChangeTime + ZOMBIE_SPIRIT_BUILDUP_TIME + ZOMBIE_SPIRIT_FADEOUT_TIME ) {
			return AIFunc_DefaultStart( cs );
		}
	} else {

		// set torso to the correct animation
		// TODO
		//ent->client->ps.torsoTimer = 300;	// leave enough time to cancel if we stop coming in here, but stay in the anim if we come back next thing

		// draw the client-side effect
		ent->client->ps.eFlags |= EF_MONSTER_EFFECT;

		// inform the client of our enemies position
		VectorCopy( g_entities[cs->enemyNum].client->ps.origin, ent->s.origin2 );
		ent->s.origin2[2] += g_entities[cs->enemyNum].client->ps.viewheight;
	}
	//
	//
	return NULL;
}

char *AIFunc_ZombieSpiritStart( cast_state_t *cs ) {
	gentity_t *ent;
	//
	// don't allow 2 consecutive spirit attacks at once
	if ( ai_encounter.lastSpiritAttackTime <= level.time && ai_encounter.lastSpiritAttackTime > level.time - 1000 ) {
		return NULL;
	}
	ai_encounter.lastSpiritAttackTime = level.time;
	//
	ent = &g_entities[cs->entityNum];
	ent->s.otherEntityNum2 = cs->enemyNum;
	ent->s.effect1Time = level.time;
	//
	// dont turn
	cs->ideal_viewangles[YAW] = cs->viewangles[YAW];
	// set torso to the correct animation
	// TODO
	//ent->client->ps.torsoAnim =
	//	( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | BOTH_SALUTE;	// FIXME: need a specific anim for this
	//
	AI_SetLegacyState( cs, AIFunc_ZombieSpirit );
	return "AIFunc_ZombieSpirit";
}

/*
================
AIFunc_ZombieMelee
================
*/

int zombieHitDamage[5] = { 16, 16, 16, 12, 20 };

#define NUM_ZOMBIE_ANIMS 5
int zombieHitTimes[NUM_ZOMBIE_ANIMS][3] = {
	// up to three hits per attack
	{ ANIMLENGTH( 11, 20 ), -1 },
	{ ANIMLENGTH( 9, 20 ), -1 },
	{ ANIMLENGTH( 9, 20 ), -1 },
	{ ANIMLENGTH( 8, 20 ), ANIMLENGTH( 16, 20 ), -1 },
	{ ANIMLENGTH( 8, 20 ), ANIMLENGTH( 15, 20 ), ANIMLENGTH( 24, 20 ) },
};

char *AIFunc_ZombieMelee( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	int hitDelay = -1, anim;
	trace_t *tr;
	cast_state_t *ecs = AICast_GetCastState( cs->enemyNum );
	aicast_predictmove_t move;
	float enemyDist;

	if ( !ent->client->ps.torsoTimer ) {
		return AIFunc_DefaultStart( cs );
	}

	if ( ecs ) {

		anim = ( ent->client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) - BG_AnimationIndexForString( "attack1", cs->entityNum );
		if ( anim < 0 || anim >= NUM_ZOMBIE_ANIMS ) {
			// animation interupted
			return AIFunc_DefaultStart( cs );
		}
		if ( cs->ability.animHitCount < 3 && zombieHitTimes[anim][cs->ability.animHitCount] >= 0 ) {

			if ( !cs->ability.animHitCount ) {
				hitDelay = zombieHitTimes[anim][cs->ability.animHitCount];
			} else {
				hitDelay =
					zombieHitTimes[anim][cs->ability.animHitCount] - zombieHitTimes[anim][cs->ability.animHitCount - 1];
			}

			// check for inflicting damage
			if ( level.time - cs->combat.weaponFireTimes[cs->weaponNum] > hitDelay ) {
				// do melee damage
				if ( ( tr = CheckMeleeAttack( ent, AICast_WeaponRange( cs, cs->weaponNum ) + 4.0, qfalse ) ) &&
					 ( tr->entityNum == cs->enemyNum ) ) {
					G_Damage( &g_entities[tr->entityNum], ent, ent, vec3_origin, tr->endpos, zombieHitDamage[anim], 0,
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
		if ( !ent->client->ps.legsTimer || cs->ability.animHitCount >= 3 ||
			 zombieHitTimes[anim][cs->ability.animHitCount] < 0 ) {
			// if they are outside range, move forward
			AICast_PredictMovement( ecs, 2, 0.5, &move, &g_entities[cs->enemyNum].client->pers.cmd, -1 );
			enemyDist = Distance( move.endpos, cs->clientState->origin );
			enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
			enemyDist -= ent->r.maxs[0];
			if ( /*anim != 4 &&*/ ( enemyDist > 16 ) ) { // we can get closer
				if ( ent->client->ps.legsTimer ) {
					ent->client->ps.legsTimer = 0; // allow legs us to move
					if ( cs->script.castScriptStatus.scriptNoMoveTime <
						 level.time + 200 ) { // dont move until the legs are done lerping out of attack anim
						cs->script.castScriptStatus.scriptNoMoveTime = level.time + 200;
					}
				}
				if ( !ent->client->ps.legsTimer && cs->script.castScriptStatus.scriptNoMoveTime < level.time ) {
					trap_EA_MoveForward( cs->entityNum );
				}
			}
		}
	}

	return NULL;
}

/*
================
AIFunc_ZombieMeleeStart
================
*/
char *AIFunc_ZombieMeleeStart( cast_state_t *cs ) {
	gentity_t *ent;

	ent = &g_entities[cs->entityNum];
	cs->combat.weaponFireTimes[cs->weaponNum] = level.time;
	cs->ability.animHitCount = 0;

	// face them
	AICast_AimAtEnemy( cs );

	// audible sound
	AIChar_AttackSound( cs );

	// play an anim
	BG_UpdateConditionValue( cs->entityNum, ANIM_COND_WEAPON, cs->weaponNum, qtrue );
	BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );

	AI_SetLegacyState( cs, AIFunc_ZombieMelee );
	return "AIFunc_ZombieMelee";
}

//===================================================================

/*
==============
AIFunc_FlameZombie_Portal
==============
*/
char *AIFunc_FlameZombie_Portal( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	//
	if ( cs->thinkFuncChangeTime < level.time - PORTAL_ZOMBIE_SPAWNTIME ) {
		// HACK, make them aware of the player
		AICast_UpdateVisibility( &g_entities[cs->entityNum], AICast_FindEntityForName( "player" ), qfalse, qtrue );
		ent->s.time2 = 0; // turn spawning effect off
		return AIFunc_DefaultStart( cs );
	}
	//
	return NULL;
}

/*
==============
AIFunc_FlameZombie_PortalStart
==============
*/
char *AIFunc_FlameZombie_PortalStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	//
	ent->s.time2 = level.time + 200; // hijacking this for portal spawning effect
	//
	// play a special animation
	ent->client->ps.torsoAnim = ( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | BOTH_EXTRA1;
	ent->client->ps.legsAnim = ( ( ent->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | BOTH_EXTRA1;
	ent->client->ps.torsoTimer = PORTAL_ZOMBIE_SPAWNTIME - 200;
	ent->client->ps.legsTimer = PORTAL_ZOMBIE_SPAWNTIME - 200;
	//
	cs->thinkFuncChangeTime = level.time;
	//
	AI_SetLegacyState( cs, AIFunc_FlameZombie_Portal );
	return "AIFunc_FlameZombie_Portal";
}

// Ability selection and tuning belong to this creature.
static float AI_ZombieAbilityRange( cast_state_t *cs, int weaponnum ) {
	switch ( weaponnum ) {
	case WP_MONSTER_ATTACK1:
		// zombie flaming attack
		return ZOMBIE_FLAME_RADIUS - 50; // get well within range before starting
	case WP_MONSTER_ATTACK2:
		// zombie spirit attack
		return 1000;
	case WP_MONSTER_ATTACK3:
		return 44;
	default:
		return 3000;
	}
}

static qboolean AI_ZombieAbilityAvailable( cast_state_t *cs, int weaponNum ) {
	int delay = -1;
	float dist = -1;
	if ( cs->enemyNum >= 0 ) {
		dist = Distance( cs->clientState->origin, g_entities[cs->enemyNum].s.pos.trBase );
	}
	switch ( weaponNum ) {
	case WP_MONSTER_ATTACK1: {
		// zombie flaming attack
		delay = 4000;
		if ( dist < 0 ) { // || dist < 128) {
			return qfalse;
		}
		if ( dist > 1200 ) {
			return qfalse;
		}
		if ( cs->enemyNum < 0 ) {
			return qfalse;
		}
		//if (cs->perception.vislist[cs->enemyNum].notvisible_timestamp > level.time - 500) {
		//	return qfalse;
		//}
		break;
	}
	case WP_MONSTER_ATTACK2: {
		delay = 6000;
		// zombie "flying spirit" attack
		if ( dist < 64 ) {
			return qfalse;
		}
		if ( dist > 1200 ) {
			return qfalse;
		}
		if ( cs->enemyNum < 0 ) {
			return qfalse;
		}
		if ( cs->perception.vislist[cs->enemyNum].notvisible_timestamp > level.time - 1500 ) {
			return qfalse;
		}
		break;
	}
	case WP_MONSTER_ATTACK3: {
		return qtrue; // always usable
	}
	default:
		return qfalse;
	}
	return !cs->combat.weaponFireTimes[weaponNum] || cs->combat.weaponFireTimes[weaponNum] < level.time - delay;
}

static const ai_ability_definition_t zombieDefinitions[] = {
	{ "ZombieFlame", WP_MONSTER_ATTACK1, AIFunc_ZombieFlameAttackStart, AI_ZombieAbilityRange,
	  AI_ZombieAbilityAvailable },
	{ "ZombieSpirit", WP_MONSTER_ATTACK2, AIFunc_ZombieSpiritStart, AI_ZombieAbilityRange, AI_ZombieAbilityAvailable },
	{ "ZombieMelee", WP_MONSTER_ATTACK3, AIFunc_ZombieMeleeStart, AI_ZombieAbilityRange, AI_ZombieAbilityAvailable },
};

const ai_ability_set_t ai_ZombieAbilities = { zombieDefinitions,
											  sizeof( zombieDefinitions ) / sizeof( zombieDefinitions[0] ) };
