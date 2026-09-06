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

/*
=============
AIFunc_BattleRoll()
=============
*/
char *AIFunc_BattleRoll( cast_state_t *cs ) {
	gclient_t *client = &level.clients[cs->entityNum];
	vec3_t dir;
	//
	// record the time
	cs->lastRollMove = level.time;
	client->ps.eFlags |= EF_NOSWINGANGLES;
	//
	if ( !client->ps.torsoTimer ) {
		if ( cs->battleRollTime < level.time ) {
			return AIFunc_Restore( cs );
		} else {
			// attack?
			if ( cs->enemyNum >= 0 ) {
				AICast_ProcessAttack( cs );
			}
		}
	}
	if ( g_entities[cs->entityNum].health <= 0 ) {
		return AIFunc_DefaultStart( cs );
	}
	//
	trap_EA_Crouch( cs->entityNum );
	cs->attackcrouch_time = level.time + 500;
	// all characters so far only move during the first second of animation
	if ( cs->thinkFuncChangeTime > level.time - 1000 ) {
		// just move in the direction of our ideal_viewangles
		AngleVectors( cs->ideal_viewangles, dir, NULL, NULL );
		trap_EA_Move( cs->entityNum, dir, 300 );
		// we are crouching, move a little faster than normal
		cs->speedScale = 1.5;
	} else if ( cs->combat.takeCoverTime > level.time ) {
		//
		// if we are taking Cover, use this position, if it's bad, we'll just look for a better spot once we're done here
		VectorCopy( cs->clientState->origin, cs->combat.takeCoverPos );
	} else if ( cs->enemyNum >= 0 ) {
		//
		// start turning towards our enemy
		AICast_ProcessAttack( cs );
	}
	//
	return NULL;
}

/*
=============
AIFunc_BattleRollStart()
=============
*/
char *AIFunc_BattleRollStart( cast_state_t *cs, vec3_t vec ) {
	int duration;
	//	gclient_t *client = &level.clients[cs->entityNum];
	//
	// backup the current thinkfunc, so we can return to it when done
	cs->previousState = cs->currentState;
	//
	// face the direction of movement
	vectoangles( vec, cs->ideal_viewangles );
	// do the roll
	duration = BG_AnimScriptEvent( &g_entities[cs->entityNum].client->ps, ANIM_ET_ROLL, qfalse, qtrue );
	//
	if ( duration < 0 ) { // it failed
		return NULL;
	}
	// add some duration to make sure it fully plays out
	duration += 100;
	g_entities[cs->entityNum].client->ps.legsTimer = duration;
	g_entities[cs->entityNum].client->ps.torsoTimer = duration;
	//
	cs->combat.noAttackTime = level.time + duration - 200;
	// set the duration
	cs->battleRollTime = level.time + duration;
	// move into crouch position
	//cs->attackcrouch_time = level.time + (duration) + 1000;
	// record the time
	cs->lastRollMove = level.time;
	//
	// make sure we move this frame
	AIFunc_BattleRoll( cs );
	//
	AI_SetLegacyState( cs, AIFunc_BattleRoll );
	return "AIFunc_BattleRoll";
}

/*
=============
AIFunc_BattleDiveStart()
=============
*/
char *AIFunc_BattleDiveStart( cast_state_t *cs, vec3_t vec ) {
	int duration;
	//	gclient_t *client = &level.clients[cs->entityNum];
	//
	// backup the current thinkfunc, so we can return to it when done
	cs->previousState = cs->currentState;
	//
	// face the direction of movement
	vectoangles( vec, cs->ideal_viewangles );
	// force crouching anim
	BG_UpdateConditionValue( cs->entityNum, ANIM_COND_CROUCHING, qtrue, qfalse );
	// do the roll
	duration = BG_AnimScriptEvent( &g_entities[cs->entityNum].client->ps, ANIM_ET_DIVE, qfalse, qtrue );
	//
	if ( duration < 0 ) { // it failed
		return NULL;
	}
	//
	cs->combat.noAttackTime = level.time + duration - 200;
	// set the duration
	cs->battleRollTime = level.time + duration;
	// move into crouch position
	//cs->attackcrouch_time = level.time + (duration) + 1000;
	// record the time
	cs->lastRollMove = level.time;
	//
	// make sure we move this frame
	AIFunc_BattleRoll( cs );
	//
	AI_SetLegacyState( cs, AIFunc_BattleRoll );
	return "AIFunc_BattleRoll";
}

/*
=============
AIFunc_FlipMove()
=============
*/
char *AIFunc_FlipMove( cast_state_t *cs ) {
	gclient_t *client = &level.clients[cs->entityNum];
	vec3_t dir;
	//
	if ( !client->ps.torsoTimer ) {
		cs->attackcrouch_time = 0;
		return AIFunc_Restore( cs );
	}
	if ( g_entities[cs->entityNum].health <= 0 ) {
		return AIFunc_DefaultStart( cs );
	}
	//
	// just move in the direction of our ideal_viewangles
	AngleVectors( cs->ideal_viewangles, dir, NULL, NULL );
	trap_EA_Move( cs->entityNum, dir, 400 );
	// if we are crouching, move a little faster than normal
	if ( cs->attackcrouch_time > level.time ) {
		cs->speedScale = 1.5;
	}
	//
	return NULL;
}

/*
=============
AIFunc_FlipMoveStart()
=============
*/
char *AIFunc_FlipMoveStart( cast_state_t *cs, vec3_t vec ) {
	int duration;
	//	gclient_t *client = &level.clients[cs->entityNum];
	//
	// backup the current thinkfunc, so we can return to it when done
	cs->previousState = cs->currentState;
	//
	// record the time
	cs->lastRollMove = level.time;
	// face the direction of movement
	vectoangles( vec, cs->ideal_viewangles );
	cs->combat.noAttackTime = level.time + 1200;
	// do the roll
	duration = BG_AnimScriptEvent( &g_entities[cs->entityNum].client->ps, ANIM_ET_ROLL, qfalse, qfalse );
	//
	if ( duration < 0 ) { // it failed
		return NULL;
	}
	// move into crouch position
	cs->attackcrouch_time = level.time + 800;
	//
	// make sure we move this frame
	AIFunc_FlipMove( cs );
	//
	AI_SetLegacyState( cs, AIFunc_FlipMove );
	return "AIFunc_FlipMove";
}
