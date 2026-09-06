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

#include "ai_local.h"

/*
=================
AICast_StateChange

  returns qfalse if scripting has denied the action
=================
*/
qboolean AICast_StateChange( cast_state_t *cs, aistateEnum_t newaistate ) {
	gentity_t *ent;
	int result, scriptIndex;
	aistateEnum_t oldstate;

	ent = &g_entities[cs->entityNum];

	oldstate = cs->aiState;

	// RF, if the state is the same, ignore
	if ( oldstate == newaistate ) {
		return qtrue;
	}

	scriptIndex = cs->script.scriptCallIndex;

	// check scripting to see if this event should be ignored (no anim or handling)
	cs->aiFlags &= ~AIFL_DENYACTION;
	AICast_ScriptEvent( cs, "statechange",
						va( "%s %s", animStateStr[oldstate].string, animStateStr[newaistate].string ) );

	if ( cs->aiFlags & AIFL_DENYACTION ) {
		return qfalse;
	}

	if ( !( cs->aiFlags & AIFL_DENYACTION ) ) {
		// if no script was found, try enemysight
		if ( newaistate == AISTATE_COMBAT && AI_HasLivingTarget( cs ) && cs->script.scriptCallIndex == scriptIndex &&
			 !( cs->perception.vislist[cs->enemyNum].flags &
				AIVIS_SIGHT_SCRIPT_CALLED ) ) { // no script was found, so default back to enemysight
			AICast_ScriptEvent( cs, "enemysight", g_entities[cs->enemyNum].aiName );
			if ( !( cs->aiFlags & AIFL_DENYACTION ) ) {
				G_AddEvent( ent, EV_GENERAL_SOUND,
							G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[SIGHTSOUNDSCRIPT] ) );
			}

			if ( cs->aiFlags & AIFL_DENYACTION ) {
				// don't run any dynamic handling or default anims
				return qfalse;
			}
			cs->perception.vislist[cs->enemyNum].flags |= AIVIS_SIGHT_SCRIPT_CALLED;
		}

		cs->aiState = newaistate;
		// if moving from query mode, kill the anim and pausetime
		if ( oldstate == AISTATE_QUERY ) {
			// stop playing the animation
			ent->client->ps.torsoTimer = 0;
			ent->client->ps.legsTimer = 0;
			cs->pauseTime = 0;
		}

		// if moving to combat mode, default back to normal movetype (fast)
		if ( newaistate == AISTATE_COMBAT ) {
			cs->movestate = MS_DEFAULT;
			cs->movestateType = MSTYPE_NONE;
		}

		// look for an animation
		result = BG_AnimScriptStateChange( &ent->client->ps, newaistate, oldstate );

		if ( result > 0 ) {
			// pause while the animation plays
			cs->pauseTime = level.time + result;
		}
	}

	// set query mode fields
	if ( newaistate == AISTATE_QUERY ) {
		cs->perception.queryStartTime = level.time;
		if ( cs->perception.queryCountValidTime < level.time ) {
			cs->perception.queryCount = 0;
		} else {
			cs->perception.queryCount++;
		}
		cs->perception.queryCountValidTime = level.time + 60000; // one minute
		switch ( cs->perception.queryCount ) {
		case 0:
			cs->perception.queryAlertSightTime = level.time + 1000;
			break;
		case 1:
			cs->perception.queryAlertSightTime = level.time + 500;
			break;
		default:
			cs->perception.queryAlertSightTime = -1; // IMMEDIATE COMBAT MODE
			break;
		}
	}

	return qtrue;
}
