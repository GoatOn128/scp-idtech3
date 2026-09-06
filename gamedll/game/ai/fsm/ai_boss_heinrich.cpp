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
// Heinrich, the LAST boss
//
//=================================================================================

//
// Special Sound Precache
//

typedef enum {
	HEINRICH_SWORDIMPACT,
	HEINRICH_SWORDLUNGE_START,
	HEINRICH_SWORDKNOCKBACK_START,
	HEINRICH_SWORDKNOCKBACK_WEAPON,
	HEINRICH_SWORDSIDESLASH_START,
	HEINRICH_SWORDSIDESLASH_WEAPON,
	HEINRICH_EARTHQUAKE_START,
	HEINRICH_RAISEDEAD_START,
	HEINRICH_TAUNT_GOODHEALTH,
	HEINRICH_TAUNT_LOWHEALTH,

	MAX_HEINRICH_SOUNDS
} heinrichSounds_t;

char *heinrichSounds[MAX_HEINRICH_SOUNDS] = {
	"heinrichSwordImpact",			"heinrichSwordLungeStart",	   "heinrichSwordKnockbackStart",
	"heinrichSwordKnockbackWeapon", "heinrichSwordSideSlashStart", "heinrichSwordSideSlashWeapon",
	"heinrichSwordEarthquakeStart", "heinrichRaiseWarriorStart",   "heinrichTauntGoodHealth",
	"heinrichTauntLowHealth",
};

int heinrichSoundIndex[MAX_HEINRICH_SOUNDS];

void AICast_Heinrich_SoundPrecache( void ) {
	int i;
	for ( i = 0; i < MAX_HEINRICH_SOUNDS; i++ ) {
		heinrichSoundIndex[i] = G_SoundIndex( heinrichSounds[i] );
	}
}

void AICast_Heinrich_Taunt( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	static int lastTaunt;
	// sound
	if ( ent->health > cs->attributes[STARTING_HEALTH] * 0.25 ) {
		if ( lastTaunt > level.time || lastTaunt < level.time - 20000 ) {
			G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_TAUNT_GOODHEALTH] );
			lastTaunt = level.time;
		}
	} else {
		if ( lastTaunt > level.time || lastTaunt < level.time - 40000 ) {
			G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_TAUNT_LOWHEALTH] );
			lastTaunt = level.time;
		}
	}
}

#define HEINRICH_LUNGE_DELAY ANIMLENGTH( 15, 20 )
#define HEINRICH_LUNGE_RANGE 170
#define HEINRICH_LUNGE_DAMAGE ( 50 + rand() % 20 )

char *AIFunc_Heinrich_SwordLunge( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	trace_t *tr;
	vec3_t left;
	float enemyDist;
	aicast_predictmove_t move;
	vec3_t vec;
	cast_state_t *ecs;

	cs->aiFlags |= AIFL_SPECIAL_FUNC;

	if ( cs->enemyNum < 0 ) {
		if ( ent->client->ps.torsoTimer ) {
			return NULL;
		}
		return AIFunc_DefaultStart( cs );
	}

	ecs = AICast_GetCastState( cs->enemyNum );

	if ( ent->client->ps.torsoTimer < 500 ) {
		if ( !ent->client->ps.legsTimer ) {
			trap_EA_MoveForward( cs->entityNum );
		}
		ent->client->ps.legsTimer = 0;
		ent->client->ps.torsoTimer = 0;
		cs->script.castScriptStatus.scriptNoMoveTime = 0;
		AICast_Heinrich_Taunt( cs );
		return AIFunc_BattleChaseStart( cs );
	}

	// time for the melee?
	if ( cs->enemyNum >= 0 && !( cs->aiFlags & AIFL_MISCFLAG1 ) ) {
		// face them
		AICast_AimAtEnemy( cs );
		// keep checking for impact status
		tr = CheckMeleeAttack( ent, HEINRICH_LUNGE_RANGE, qfalse );
		/*		// do we need to move?
		if (!(tr && (tr->entityNum == cs->enemyNum))) {
			ent->client->ps.legsTimer = 0;
			cs->script.castScriptStatus.scriptNoMoveTime = 0;
			trap_EA_MoveForward( cs->entityNum );
		}
*/                                                                                                                                                                                                           // ready for damage?
		if ( cs->thinkFuncChangeTime < level.time - HEINRICH_LUNGE_DELAY ) {
			cs->aiFlags |= AIFL_MISCFLAG1;
			// do melee damage
			if ( tr && ( tr->entityNum == cs->enemyNum ) ) {
				G_Damage( &g_entities[tr->entityNum], ent, ent, left, tr->endpos, HEINRICH_LUNGE_DAMAGE, 0,
						  MOD_GAUNTLET );
				// sound
				G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDIMPACT] );
			}
		}
	}

	// if they are outside range, move forward
	AICast_PredictMovement( ecs, 2, 0.3, &move, &g_entities[cs->enemyNum].client->pers.cmd, -1 );
	VectorSubtract( move.endpos, cs->clientState->origin, vec );
	vec[2] = 0;
	enemyDist = VectorLength( vec );
	enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
	enemyDist -= ent->r.maxs[0];
	if ( enemyDist > 30 ) { // we can get closer
		if ( ent->client->ps.legsTimer ) {
			cs->script.castScriptStatus.scriptNoMoveTime = level.time + 100;
			ent->client->ps.legsTimer = 0; // allow legs to move us
		}
		if ( cs->script.castScriptStatus.scriptNoMoveTime < level.time ) {
			trap_EA_MoveForward( cs->entityNum );
		}
	}

	return NULL;
}

char *AIFunc_Heinrich_SwordLungeStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	//	gentity_t	*enemy = &g_entities[cs->enemyNum];

	cs->aiFlags |= AIFL_SPECIAL_FUNC;
	// sound
	G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDLUNGE_START] );
	// face them
	AICast_AimAtEnemy( cs );
	// clear flags
	cs->aiFlags &= ~( AIFL_MISCFLAG1 | AIFL_MISCFLAG2 );
	// play the anim
	BG_PlayAnimName( &ent->client->ps, "attack9", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
	// start the func
	AI_SetLegacyState( cs, AIFunc_Heinrich_SwordLunge );
	return "AIFunc_Heinrich_SwordLunge";
}

#define HEINRICH_KNOCKBACK_DELAY ANIMLENGTH( 26, 20 )
#define HEINRICH_KNOCKBACK_RANGE 150
#define HEINRICH_KNOCKBACK_DAMAGE ( 60 + rand() % 20 )

char *AIFunc_Heinrich_SwordKnockback( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	trace_t *tr;
	vec3_t right, left;
	//	float	enemyDist;
	//	aicast_predictmove_t move;
	//	vec3_t	vec;
	cast_state_t *ecs;

	cs->aiFlags |= AIFL_SPECIAL_FUNC;

	if ( cs->enemyNum < 0 ) {
		if ( ent->client->ps.torsoTimer ) {
			return NULL;
		}
		return AIFunc_DefaultStart( cs );
	}

	ecs = AICast_GetCastState( cs->enemyNum );

	if ( ent->client->ps.torsoTimer < 500 ) {
		if ( !ent->client->ps.legsTimer ) {
			trap_EA_MoveForward( cs->entityNum );
		}
		ent->client->ps.legsTimer = 0;
		ent->client->ps.torsoTimer = 0;
		cs->script.castScriptStatus.scriptNoMoveTime = 0;
		AICast_Heinrich_Taunt( cs );
		return AIFunc_BattleChaseStart( cs );
	}

	// time for the melee?
	if ( cs->enemyNum >= 0 && !( cs->aiFlags & AIFL_MISCFLAG1 ) ) {
		// face them
		AICast_AimAtEnemy( cs );
		// keep checking for impact status
		tr = CheckMeleeAttack( ent, HEINRICH_KNOCKBACK_RANGE, qfalse );
		/*		// do we need to move?
		if (!(tr && (tr->entityNum == cs->enemyNum))) {
			ent->client->ps.legsTimer = 0;
			cs->script.castScriptStatus.scriptNoMoveTime = 0;
			trap_EA_MoveForward( cs->entityNum );
		}
*/                                                                                                                                                                                                           // ready for damage?
		if ( cs->thinkFuncChangeTime < level.time - HEINRICH_KNOCKBACK_DELAY ) {
			cs->aiFlags |= AIFL_MISCFLAG1;
			// do melee damage
			if ( tr && ( tr->entityNum == cs->enemyNum ) ) {
				AngleVectors( cs->viewangles, NULL, right, NULL );
				VectorNegate( right, left );
				G_Damage( &g_entities[tr->entityNum], ent, ent, left, tr->endpos, HEINRICH_KNOCKBACK_DAMAGE, 0,
						  MOD_GAUNTLET );
				// sound
				G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDIMPACT] );
				// throw them in direction of impact
				if ( ( ent->client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) ==
					 BG_AnimationIndexForString( "attack2", cs->entityNum ) ) {
					// right
					right[2] = 0.5;
					VectorMA( g_entities[cs->enemyNum].client->ps.velocity, 400, right,
							  g_entities[cs->enemyNum].client->ps.velocity );
				} else {
					// left
					left[2] = 0.5;
					VectorMA( g_entities[cs->enemyNum].client->ps.velocity, 400, left,
							  g_entities[cs->enemyNum].client->ps.velocity );
				}
			}
		}
	}
	/*	DISABLED FOR SWORDKNOCKBACK..looks bad
	// if they are outside range, move forward
	AICast_PredictMovement( ecs, 2, 0.3, &move, &g_entities[cs->enemyNum].client->pers.cmd, -1 );
	VectorSubtract( move.endpos, cs->clientState->origin, vec );
	vec[2] = 0;
	enemyDist = VectorLength( vec );
	enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
	enemyDist -= ent->r.maxs[0];
	if (enemyDist > 30) {	// we can get closer
		if (ent->client->ps.legsTimer) {
			cs->script.castScriptStatus.scriptNoMoveTime = level.time + 100;
			ent->client->ps.legsTimer = 0;		// allow legs to move us
		}
		if (cs->script.castScriptStatus.scriptNoMoveTime < level.time) {
			trap_EA_MoveForward(cs->entityNum);
		}
	}
*/
	return NULL;
}

char *AIFunc_Heinrich_SwordKnockbackStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	//	gentity_t	*enemy = &g_entities[cs->enemyNum];

	cs->aiFlags |= AIFL_SPECIAL_FUNC;
	// sound
	G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDKNOCKBACK_START] );
	// weapon sound
	G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDKNOCKBACK_WEAPON] );
	// face them
	AICast_AimAtEnemy( cs );
	// clear flags
	cs->aiFlags &= ~( AIFL_MISCFLAG1 | AIFL_MISCFLAG2 );
	// play the anim
	if ( rand() % 2 ) {
		BG_PlayAnimName( &ent->client->ps, "attack2", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
	} else {
		BG_PlayAnimName( &ent->client->ps, "attack3", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
	}
	// start the func
	AI_SetLegacyState( cs, AIFunc_Heinrich_SwordKnockback );
	return "AIFunc_Heinrich_SwordKnockback";
}

#define HEINRICH_SLASH_DELAY ANIMLENGTH( 17, 25 )
#define HEINRICH_SLASH_RANGE 140
#define HEINRICH_SLASH_DAMAGE ( 30 + rand() % 15 )

char *AIFunc_Heinrich_SwordSideSlash( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	trace_t *tr;
	vec3_t right, left;
	float enemyDist;
	aicast_predictmove_t move;
	vec3_t vec;
	cast_state_t *ecs;

	cs->aiFlags |= AIFL_SPECIAL_FUNC;

	if ( cs->enemyNum < 0 ) {
		if ( ent->client->ps.torsoTimer ) {
			return NULL;
		}
		return AIFunc_DefaultStart( cs );
	}

	ecs = AICast_GetCastState( cs->enemyNum );

	if ( ent->client->ps.torsoTimer < 500 ) {
		if ( !ent->client->ps.legsTimer ) {
			trap_EA_MoveForward( cs->entityNum );
		}
		ent->client->ps.legsTimer = 0;
		ent->client->ps.torsoTimer = 0;
		cs->script.castScriptStatus.scriptNoMoveTime = 0;
		AICast_Heinrich_Taunt( cs );
		return AIFunc_BattleChaseStart( cs );
	}

	// time for the melee?
	if ( cs->enemyNum >= 0 && !( cs->aiFlags & AIFL_MISCFLAG1 ) ) {
		// face them
		AICast_AimAtEnemy( cs );
		// keep checking for impact status
		tr = CheckMeleeAttack( ent, HEINRICH_SLASH_RANGE, qfalse );
		// ready for damage?
		if ( cs->thinkFuncChangeTime < level.time - HEINRICH_SLASH_DELAY ) {
			cs->aiFlags |= AIFL_MISCFLAG1;
			// do melee damage
			if ( tr && ( tr->entityNum == cs->enemyNum ) ) {
				AngleVectors( cs->viewangles, NULL, right, NULL );
				VectorNegate( right, left );
				G_Damage( &g_entities[tr->entityNum], ent, ent, left, tr->endpos, HEINRICH_SLASH_DAMAGE, 0,
						  MOD_GAUNTLET );
				// sound
				G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDIMPACT] );
				// throw them in direction of impact
				left[2] = 0.5;
				VectorMA( g_entities[cs->enemyNum].client->ps.velocity, 400, left,
						  g_entities[cs->enemyNum].client->ps.velocity );
			}
		}
	}

	// if they are outside range, move forward
	AICast_PredictMovement( ecs, 2, 0.3, &move, &g_entities[cs->enemyNum].client->pers.cmd, -1 );
	VectorSubtract( move.endpos, cs->clientState->origin, vec );
	vec[2] = 0;
	enemyDist = VectorLength( vec );
	enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
	enemyDist -= ent->r.maxs[0];
	if ( enemyDist > 30 ) { // we can get closer
		if ( ent->client->ps.legsTimer ) {
			cs->script.castScriptStatus.scriptNoMoveTime = level.time + 100;
			ent->client->ps.legsTimer = 0; // allow legs to move us
		}
		if ( cs->script.castScriptStatus.scriptNoMoveTime < level.time ) {
			trap_EA_MoveForward( cs->entityNum );
		}
	}

	return NULL;
}

char *AIFunc_Heinrich_SwordSideSlashStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];

	cs->aiFlags |= AIFL_SPECIAL_FUNC;
	// sound
	G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDSIDESLASH_START] );
	// weapon sound
	G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_SWORDSIDESLASH_WEAPON] );
	// face them
	AICast_AimAtEnemy( cs );
	// clear flags
	cs->aiFlags &= ~( AIFL_MISCFLAG1 | AIFL_MISCFLAG2 );
	// play the anim
	BG_PlayAnimName( &ent->client->ps, "attack8", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
	// start the func
	AI_SetLegacyState( cs, AIFunc_Heinrich_SwordSideSlash );
	return "AIFunc_Heinrich_SwordSideSlash";
}

#define HEINRICH_STOMP_DELAY 900
#define HEINRICH_STOMP_RANGE 1024.0
#define HEINRICH_STOMP_VELOCITY_Z 420
#define HEINRICH_STOMP_DAMAGE 35

char *AIFunc_Heinrich_Earthquake( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	gentity_t *enemy;
	cast_state_t *ecs;
	vec3_t enemyVec;
	float enemyDist, scale;
	trace_t *tr;

	cs->aiFlags |= AIFL_SPECIAL_FUNC;

	if ( cs->enemyNum < 0 ) {
		if ( !ent->client->ps.torsoTimer ) {
			return AIFunc_DefaultStart( cs );
		}
		return NULL;
	}

	enemy = &g_entities[cs->enemyNum];
	ecs = AICast_GetCastState( cs->enemyNum );

	VectorMA( enemy->r.currentOrigin, HEINRICH_STOMP_DELAY, enemy->client->ps.velocity, enemyVec );
	enemyDist = VectorDistance( ent->r.currentOrigin, enemyVec );

	if ( ent->client->ps.torsoTimer < 500 ) {
		int rnd;
		aicast_predictmove_t move;
		vec3_t vec;

		AICast_PredictMovement( ecs, 2, 0.5, &move, &g_entities[cs->enemyNum].client->pers.cmd, -1 );
		VectorSubtract( move.endpos, cs->clientState->origin, vec );
		vec[2] = 0;
		enemyDist = VectorLength( vec );
		enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
		enemyDist -= ent->r.maxs[0];
		//
		if ( enemyDist < 140 ) {
			// combo attack
			rnd = rand() % 3;
			switch ( rnd ) {
			case 0:
				return AIFunc_Heinrich_SwordSideSlashStart( cs );
			case 1:
				return AIFunc_Heinrich_SwordKnockbackStart( cs );
			case 2:
				return AIFunc_Heinrich_SwordLungeStart( cs );
			}
		} else { // back to roaming
			ent->client->ps.legsTimer = 0;
			ent->client->ps.torsoTimer = 0;
			cs->script.castScriptStatus.scriptNoMoveTime = 0;
			AICast_Heinrich_Taunt( cs );
			return AIFunc_DefaultStart( cs );
		}
	}

	// time for the thump?
	if ( !( cs->aiFlags & AIFL_MISCFLAG1 ) ) {
		// face them
		AICast_AimAtEnemy( cs );
		// ready for damage?
		if ( cs->thinkFuncChangeTime < level.time - HEINRICH_STOMP_DELAY ) {
			cs->aiFlags |= AIFL_MISCFLAG1;
			// play the stomp sound
			G_AddEvent( ent, EV_GENERAL_SOUND,
						G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[ORDERSDENYSOUNDSCRIPT] ) );
			// check for striking the player
			tr = CheckMeleeAttack( ent, 70, qfalse );
			// do melee damage
			if ( tr && ( tr->entityNum == cs->enemyNum ) ) {
				G_Damage( &g_entities[tr->entityNum], ent, ent, vec3_origin, tr->endpos, HEINRICH_STOMP_DAMAGE, 0,
						  MOD_GAUNTLET );
			}
			// call the debris trigger
			AICast_ScriptEvent( cs, "trigger", "quake" );
		}
	}

	enemyDist = Distance( enemy->s.pos.trBase, ent->s.pos.trBase );

	// do the earthquake effects
	if ( cs->thinkFuncChangeTime < level.time - HEINRICH_STOMP_DELAY ) {
		// throw the player into the air, if they are on the ground
		if ( ( enemy->s.groundEntityNum != ENTITYNUM_NONE ) && enemyDist < HEINRICH_STOMP_RANGE ) {
			scale = 0.5 + 0.5 * ( (float)ent->client->ps.torsoTimer / 1000.0 );
			if ( scale > 1.0 ) {
				scale = 1.0;
			}
			VectorSubtract( ent->s.pos.trBase, enemy->s.pos.trBase, enemyVec );
			VectorScale( enemyVec,
						 2.0 * ( 0.6 + 0.5 * random() ) * scale *
							 ( 0.6 + 0.6 * ( 1.0 - ( enemyDist / HEINRICH_STOMP_RANGE ) ) ),
						 enemyVec );
			enemyVec[2] = scale * HEINRICH_STOMP_VELOCITY_Z * ( 1.0 - 0.5 * ( enemyDist / HEINRICH_STOMP_RANGE ) );
			// bounce the player using this velocity
			VectorAdd( enemy->client->ps.velocity, enemyVec, enemy->client->ps.velocity );
		}
	}

	return NULL;
}

char *AIFunc_Heinrich_MeleeStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	gentity_t *enemy = &g_entities[cs->enemyNum];
	int rnd;
	static int lastStomp;

	if ( cs->enemyNum < 0 ) {
		return NULL;
	}

	// record weapon fire
	cs->combat.weaponFireTimes[cs->weaponNum] = level.time;
	// face them
	AICast_AimAtEnemy( cs );
	// clear flags
	cs->aiFlags &= ~( AIFL_MISCFLAG1 | AIFL_MISCFLAG2 );
	// decide which attack to use
	if ( VectorDistance( ent->r.currentOrigin, enemy->r.currentOrigin ) < 60 ) {
		rnd = 0; // sword slash up close
	} else if ( VectorDistance( ent->r.currentOrigin, enemy->r.currentOrigin ) >= HEINRICH_SLASH_RANGE ) {
		rnd = 1; // too far away, stomp
	} else {
		// pick at random
		rnd = rand() % 2;
	}
	//
	switch ( rnd ) {
	case 0: {
		int rnd = rand() % 3;
		switch ( rnd ) {
		case 0:
			return AIFunc_Heinrich_SwordSideSlashStart( cs );
		case 1:
			return AIFunc_Heinrich_SwordKnockbackStart( cs );
		case 2:
			return AIFunc_Heinrich_SwordLungeStart( cs );
		}
	}
	case 1:
		// dont do stomp too often
		if ( lastStomp > level.time - 12000 ) { // plenty of time to let debris disappear
			return NULL;
		}
		lastStomp = level.time;
		cs->aiFlags |= AIFL_SPECIAL_FUNC;
		// sound
		G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_EARTHQUAKE_START] );
		// play the anim
		BG_PlayAnimName( &ent->client->ps, "attack7", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
		// start the func
		AI_SetLegacyState( cs, AIFunc_Heinrich_Earthquake );
		return "AIFunc_Heinrich_Earthquake";
	}
	// shutup compiler
	return NULL;
}

#define HEINRICH_RAISEDEAD_DELAY 1200
#define HEINRICH_RAISEDEAD_COUNT 3
int lastRaise;

char *AIFunc_Heinrich_RaiseDead( cast_state_t *cs ) {
	int i;
	gentity_t *ent = &g_entities[cs->entityNum];
	gentity_t *enemy = &g_entities[cs->enemyNum];
	gentity_t *trav, *closest;
	float closestDist, dist;
	//
	cs->aiFlags |= AIFL_SPECIAL_FUNC;
	if ( cs->enemyNum < 0 ) {
		if ( !ent->client->ps.torsoTimer ) {
			return AIFunc_DefaultStart( cs );
		}
		return NULL;
	}
	//
	// record weapon fire
	cs->combat.weaponFireTimes[cs->weaponNum] = level.time;
	//
	if ( !ent->client->ps.torsoTimer ) {
		return AIFunc_DefaultStart( cs );
	}
	if ( ent->count2 && lastRaise < level.time - HEINRICH_RAISEDEAD_DELAY ) {
		lastRaise = level.time;
		// summons the closest warrior
		closest = NULL;
		closestDist = 0; // shutup the compiler
		for ( i = 0, trav = g_entities; i < level.maxclients; i++, trav++ ) {
			if ( !trav->inuse ) {
				continue;
			}
			if ( !trav->aiInactive ) {
				continue;
			}
			if ( trav->aiCharacter != AICHAR_WARZOMBIE ) {
				continue;
			}
			dist = VectorDistance( trav->s.pos.trBase, enemy->r.currentOrigin );
			if ( !closest || dist < closestDist ) {
				closest = trav;
				closestDist = dist;
			}
		}
		//
		if ( closest ) {
			closest->AIScript_AlertEntity( closest );
			// make them aware of the player
			AICast_UpdateVisibility( closest, enemy, qtrue, qtrue );
			// reduce the count
			ent->count2--;
		}
	}
	//
	return NULL;
}

char *AIFunc_Heinrich_RaiseDeadStart( cast_state_t *cs ) {
	int i, cnt, free;
	gentity_t *ent = &g_entities[cs->entityNum];
	//	gentity_t	*enemy = &g_entities[cs->enemyNum];
	gentity_t *trav, *spirits;
	float circleDist;
	//
	// count the number of active warriors
	cnt = 0;
	free = 0;
	for ( i = 0, trav = g_entities; i < level.maxclients; i++, trav++ ) {
		if ( !trav->inuse ) {
			continue;
		}
		if ( trav->aiCharacter != AICHAR_WARZOMBIE ) {
			continue;
		}
		if ( trav->aiInactive ) {
			free++;
			continue;
		}
		if ( trav->health <= 0 ) {
			continue;
		}
		cnt++;
	}
	//
	if ( cnt < HEINRICH_RAISEDEAD_COUNT && free ) { // need a new one
		cs->aiFlags &= ~AIFL_MISCFLAG1;
		ent->count2 = HEINRICH_RAISEDEAD_COUNT - cnt;
		lastRaise = level.time;
		cs->aiFlags |= AIFL_SPECIAL_FUNC;
		// start the animation
		BG_PlayAnimName( &ent->client->ps, "attack4", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
		// play the sound
		G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_RAISEDEAD_START] );
		// start the func
		AI_SetLegacyState( cs, AIFunc_Heinrich_RaiseDead );
		return "AIFunc_Heinrich_RaiseDead";
	}
	// enable all the spirit spawners
	trav = NULL;
	// TTimo: gcc: suggest () around assignment used as truth value
	while ( ( trav = G_Find( trav, FOFS( classname ), "func_bats" ) ) ) {
		if ( !trav->active && trav->spawnflags & 4 ) {
			trav->active = 1; // let them release spirits now
		}
	}
	// is the player outside the circle?
	trav = NULL;
	// TTimo: gcc: suggest () around assignment used as truth value
	while ( ( trav = G_Find( trav, FOFS( classname ), "func_bats" ) ) ) {
		if ( trav->spawnflags & 4 ) {
			spirits = trav;
			circleDist = trav->radius;
			trav = G_Find( NULL, FOFS( targetname ), trav->target );
			if ( trav ) {
				if ( VectorDistance( g_entities[0].s.pos.trBase, trav->s.origin ) > circleDist ) {
					cs->aiFlags &= ~AIFL_MISCFLAG1;
					ent->count2 = 0;
					cs->aiFlags |= AIFL_SPECIAL_FUNC;
					// start the animation
					BG_PlayAnimName( &ent->client->ps, "attack4", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
					// play the sound
					G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_RAISEDEAD_START] );
					// start the func
					AI_SetLegacyState( cs, AIFunc_Heinrich_RaiseDead );
					return "AIFunc_Heinrich_RaiseDead";
				}
			}
			break;
		}
	}
	//
	return NULL;
}

char *AIFunc_Heinrich_SpawnSpiritsStart( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	gentity_t *trav, *spirits;
	float circleDist;
	//
	// enable all the spirit spawners
	trav = NULL;
	// TTimo: gcc: suggest () around assignment used as truth value
	while ( ( trav = G_Find( trav, FOFS( classname ), "func_bats" ) ) ) {
		if ( !trav->active && trav->spawnflags & 4 ) {
			trav->active = 1; // let them release spirits now
		}
	}
	// is the player outside the circle?
	trav = NULL;
	// TTimo: gcc: suggest () around assignment used as truth value
	while ( ( trav = G_Find( trav, FOFS( classname ), "func_bats" ) ) ) {
		if ( trav->spawnflags & 4 ) {
			spirits = trav;
			circleDist = trav->radius;
			trav = G_Find( NULL, FOFS( targetname ), trav->target );
			if ( trav ) {
				if ( VectorDistance( g_entities[0].s.pos.trBase, trav->s.origin ) > circleDist ) {
					cs->aiFlags &= ~AIFL_MISCFLAG1;
					ent->count2 = 0;
					cs->aiFlags |= AIFL_SPECIAL_FUNC;
					// start the animation
					BG_PlayAnimName( &ent->client->ps, "attack4", ANIM_BP_BOTH, qtrue, qfalse, qtrue );
					// play the sound
					G_AddEvent( ent, EV_GENERAL_SOUND, heinrichSoundIndex[HEINRICH_RAISEDEAD_START] );
					// start the func
					AI_SetLegacyState( cs,
									   AIFunc_Heinrich_RaiseDead ); // just do raise dead, without raising any warriors
					return "AIFunc_Heinrich_RaiseDead";
				}
			}
			break;
		}
	}
	//
	return NULL;
}

// Ability selection and tuning belong to this creature.
static float AI_HeinrichAbilityRange( cast_state_t *cs, int weaponnum ) {
	switch ( weaponnum ) {
	case WP_MONSTER_ATTACK1:
		if ( cs->combat.weaponFireTimes[weaponnum] < level.time - 8000 ) {
			return 500; // lots of room for stomping
		} else {
			return 120; // come in real close
		}
	case WP_MONSTER_ATTACK2:
		return 8000;
	case WP_MONSTER_ATTACK3:
		// spirits
		return 50000;
	default:
		return 3000;
	}
}

static qboolean AI_HeinrichAbilityAvailable( cast_state_t *cs, int weaponNum ) {
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
		delay = 6000;
		break;
	}
	case WP_MONSTER_ATTACK3: {
		// spirits
		delay = 7000;
		break;
	}
	default:
		return qfalse;
	}
	return !cs->combat.weaponFireTimes[weaponNum] || cs->combat.weaponFireTimes[weaponNum] < level.time - delay;
}

static const ai_ability_definition_t heinrichDefinitions[] = {
	{ "HeinrichMelee", WP_MONSTER_ATTACK1, AIFunc_Heinrich_MeleeStart, AI_HeinrichAbilityRange,
	  AI_HeinrichAbilityAvailable },
	{ "HeinrichRaiseDead", WP_MONSTER_ATTACK2, AIFunc_Heinrich_RaiseDeadStart, AI_HeinrichAbilityRange,
	  AI_HeinrichAbilityAvailable },
	{ "HeinrichSpirits", WP_MONSTER_ATTACK3, AIFunc_Heinrich_SpawnSpiritsStart, AI_HeinrichAbilityRange,
	  AI_HeinrichAbilityAvailable },
};

const ai_ability_set_t ai_HeinrichAbilities = { heinrichDefinitions,
												sizeof( heinrichDefinitions ) / sizeof( heinrichDefinitions[0] ) };
