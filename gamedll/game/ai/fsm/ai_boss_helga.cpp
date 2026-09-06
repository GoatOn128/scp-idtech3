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

// TTimo: unused
//static vec3_t forward, right, up;

//=================================================================================
//
// Helga, the first boss
//
//=================================================================================

#define HELGA_SPIRIT_BUILDUP_TIME 8000 // last for this long
#define HELGA_SPIRIT_FADEOUT_TIME 1000
#define HELGA_SPIRIT_DLIGHT_RADIUS_MAX 384
#define HELGA_SPIRIT_FIRE_INTERVAL 1000

char *AIFunc_Helga_SpiritAttack( cast_state_t *cs ) {
	ai_client_state_t *bs;
	gentity_t *ent;
	//
	cs->aiFlags |= AIFL_SPECIAL_FUNC;
	ent = &g_entities[cs->entityNum];
	bs = cs->clientState;
	// make sure we're still playing the right anim
	if ( ( ent->client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) - BG_AnimationIndexForString( "attack1", cs->entityNum ) ) {
		return AIFunc_DefaultStart( cs );
	}
	//
	if ( cs->enemyNum < 0 ) {
		ent->client->ps.torsoTimer = 0;
		ent->client->ps.legsTimer = 0;
		return AIFunc_DefaultStart( cs );
	}
	//
	// if we can't see them anymore, abort immediately
	if ( cs->perception.vislist[cs->enemyNum].real_visible_timestamp !=
		 cs->perception.vislist[cs->enemyNum].real_update_timestamp ) {
		ent->client->ps.torsoTimer = 0;
		ent->client->ps.legsTimer = 0;
		return AIFunc_DefaultStart( cs );
	}
	// we are firing this weapon, so record it
	cs->combat.weaponFireTimes[WP_MONSTER_ATTACK2] = level.time;
	//
	// once an attack has started, only abort once the player leaves our view, or time runs out
	if ( cs->thinkFuncChangeTime < level.time - HELGA_SPIRIT_BUILDUP_TIME ) {
		// if enough time has elapsed, finish this attack
		if ( level.time > cs->thinkFuncChangeTime + HELGA_SPIRIT_BUILDUP_TIME + HELGA_SPIRIT_FADEOUT_TIME ) {
			ent->client->ps.torsoTimer = 0;
			ent->client->ps.legsTimer = 0;
			return AIFunc_DefaultStart( cs );
		}
	} else {

		// set timers
		ent->client->ps.torsoTimer = 1000;
		ent->client->ps.legsTimer = 1000;

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

char *AIFunc_Helga_SpiritAttack_Start( cast_state_t *cs ) {
	gentity_t *ent;
	//
	ent = &g_entities[cs->entityNum];
	ent->s.otherEntityNum2 = cs->enemyNum;
	ent->s.effect1Time = level.time;
	cs->aiFlags |= AIFL_SPECIAL_FUNC;
	//
	// dont turn
	cs->ideal_viewangles[YAW] = cs->viewangles[YAW];
	// play an anim
	BG_UpdateConditionValue( cs->entityNum, ANIM_COND_WEAPON, WP_MONSTER_ATTACK2, qtrue );
	BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );
	//
	AI_SetLegacyState( cs, AIFunc_Helga_SpiritAttack );
	return "AIFunc_Helga_SpiritAttack";
}

//=================================================================================
//
// Standing melee attacks
//
//=================================================================================

#define NUM_HELGA_ANIMS 3
#define MAX_HELGA_IMPACTS 3
int helgaHitTimes[NUM_HELGA_ANIMS][MAX_HELGA_IMPACTS] = {
	// up to three hits per attack
	{ ANIMLENGTH( 16, 20 ), -1 },
	{ ANIMLENGTH( 11, 20 ), ANIMLENGTH( 19, 20 ), -1 },
	{ ANIMLENGTH( 10, 20 ), ANIMLENGTH( 17, 20 ), ANIMLENGTH( 26, 20 ) },
};
int helgaHitDamage[NUM_HELGA_ANIMS] = { 20, 14, 12 };

/*
================
AIFunc_Helga_Melee
================
*/
char *AIFunc_Helga_Melee( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	gentity_t *enemy;
	cast_state_t *ecs;
	int hitDelay = -1, anim;
	trace_t tr;
	float enemyDist;
	aicast_predictmove_t move;
	vec3_t vec;

	cs->aiFlags |= AIFL_SPECIAL_FUNC;

	if ( !ent->client->ps.torsoTimer || !ent->client->ps.legsTimer ) {
		cs->aiFlags &= ~AIFL_SPECIAL_FUNC;
		return AIFunc_DefaultStart( cs );
	}

	if ( cs->enemyNum < 0 ) {
		ent->client->ps.legsTimer = 0; // allow legs us to move
		ent->client->ps.torsoTimer = 0; // allow legs us to move
		cs->aiFlags &= ~AIFL_SPECIAL_FUNC;
		return AIFunc_DefaultStart( cs );
	}

	ecs = AICast_GetCastState( cs->enemyNum );
	enemy = &g_entities[cs->enemyNum];

	anim = ( ent->client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) - BG_AnimationIndexForString( "attack3", cs->entityNum );
	if ( anim < 0 || anim >= NUM_HELGA_ANIMS ) {
		// animation interupted
		cs->aiFlags &= ~AIFL_SPECIAL_FUNC;
		return AIFunc_DefaultStart( cs );
		//G_Error( "AIFunc_HelgaZombieMelee: helgaBoss using invalid or unknown attack anim" );
	}
	if ( cs->ability.animHitCount < MAX_HELGA_IMPACTS && helgaHitTimes[anim][cs->ability.animHitCount] >= 0 ) {

		// face them
		VectorCopy( cs->clientState->origin, vec );
		vec[2] += ent->client->ps.viewheight;
		VectorSubtract( enemy->client->ps.origin, vec, vec );
		VectorNormalize( vec );
		vectoangles( vec, cs->ideal_viewangles );
		cs->ideal_viewangles[PITCH] = AngleNormalize180( cs->ideal_viewangles[PITCH] );

		// get hitDelay
		if ( !cs->ability.animHitCount ) {
			hitDelay = helgaHitTimes[anim][cs->ability.animHitCount];
		} else {
			hitDelay =
				helgaHitTimes[anim][cs->ability.animHitCount] - helgaHitTimes[anim][cs->ability.animHitCount - 1];
		}

		// check for inflicting damage
		if ( level.time - cs->combat.weaponFireTimes[cs->weaponNum] > hitDelay ) {
			// do melee damage
			enemyDist = VectorDistance( enemy->r.currentOrigin, ent->r.currentOrigin );
			enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
			enemyDist -= ent->r.maxs[0];
			if ( enemyDist < 10 + AICast_WeaponRange( cs, cs->weaponNum ) ) {
				trap_Trace( &tr, ent->r.currentOrigin, NULL, NULL, enemy->r.currentOrigin, ent->s.number, MASK_SHOT );
				if ( tr.entityNum == cs->enemyNum ) {
					G_Damage( &g_entities[tr.entityNum], ent, ent, vec3_origin, tr.endpos, helgaHitDamage[anim], 0,
							  MOD_GAUNTLET );
					G_AddEvent( enemy, EV_GENERAL_SOUND,
								G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[STAYSOUNDSCRIPT] ) );
				}
			}
			cs->combat.weaponFireTimes[cs->weaponNum] = level.time;
			cs->ability.animHitCount++;
		}
	}

	// if they are outside range, move forward
	AICast_PredictMovement( ecs, 2, 0.3, &move, &g_entities[cs->enemyNum].client->pers.cmd, -1 );
	VectorSubtract( move.endpos, cs->clientState->origin, vec );
	vec[2] = 0;
	enemyDist = VectorLength( vec );
	enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
	enemyDist -= ent->r.maxs[0];
	if ( enemyDist > 8 ) { // we can get closer
		//if (!ent->client->ps.legsTimer) {
		//	cs->script.castScriptStatus.scriptNoMoveTime = 0;
		trap_EA_MoveForward( cs->entityNum );
		//}
		//ent->client->ps.legsTimer = 0;		// allow legs us to move
	}

	return NULL;
}

/*
================
AIFunc_Helga_MeleeStart
================
*/
char *AIFunc_Helga_MeleeStart( cast_state_t *cs ) {
	gentity_t *ent;

	ent = &g_entities[cs->entityNum];
	ent->s.effect1Time = level.time;
	cs->ideal_viewangles[YAW] = cs->viewangles[YAW];
	cs->combat.weaponFireTimes[cs->weaponNum] = level.time;
	cs->ability.animHitCount = 0;
	cs->aiFlags |= AIFL_SPECIAL_FUNC;

	// face them
	AICast_AimAtEnemy( cs );

	// play an anim
	BG_UpdateConditionValue( cs->entityNum, ANIM_COND_WEAPON, cs->weaponNum, qtrue );
	BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_FIREWEAPON, qfalse, qtrue );

	// play a sound
	G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[ATTACKSOUNDSCRIPT] ) );

	AI_SetLegacyState( cs, AIFunc_Helga_Melee );
	cs->currentState( cs ); // think once now, to prevent a delay
	return "AIFunc_Helga_Melee";
}

// Ability selection and tuning belong to this creature.
static float AI_HelgaAbilityRange( cast_state_t *cs, int weaponnum ) {
	switch ( weaponnum ) {
	case WP_MONSTER_ATTACK1:
		// helga BOSS1 melee
		return 80;
	case WP_MONSTER_ATTACK2:
		// zombie spirit attack
		return 1900;
	default:
		return 3000;
	}
}

static qboolean AI_HelgaAbilityAvailable( cast_state_t *cs, int weaponNum ) {
	int delay = -1;
	float dist = -1;
	if ( cs->enemyNum >= 0 ) {
		dist = Distance( cs->clientState->origin, g_entities[cs->enemyNum].s.pos.trBase );
	}
	switch ( weaponNum ) {
	case WP_MONSTER_ATTACK1: {
		delay = -1;
		break;
	}
	case WP_MONSTER_ATTACK2: {
		delay = 8000;
		// zombie "flying spirit" attack
		if ( dist < 0 || dist < 80 ) {
			return qfalse;
		}
		if ( dist > 2000 ) {
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
	default:
		return qfalse;
	}
	return !cs->combat.weaponFireTimes[weaponNum] || cs->combat.weaponFireTimes[weaponNum] < level.time - delay;
}

static const ai_ability_definition_t helgaDefinitions[] = {
	{ "HelgaMelee", WP_MONSTER_ATTACK1, AIFunc_Helga_MeleeStart, AI_HelgaAbilityRange, AI_HelgaAbilityAvailable },
	{ "HelgaSpirit", WP_MONSTER_ATTACK2, AIFunc_Helga_SpiritAttack_Start, AI_HelgaAbilityRange,
	  AI_HelgaAbilityAvailable },
};

const ai_ability_set_t ai_HelgaAbilities = { helgaDefinitions,
											 sizeof( helgaDefinitions ) / sizeof( helgaDefinitions[0] ) };
