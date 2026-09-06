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
AICast_ScanForEnemies

  returns the number of enemies visible, filling the "enemies" list before exiting

  if we only found queryEnemies (possibly hostile, but not sure) while relaxed,
  then a negative count is returned
=================
*/
int AICast_ScanForEnemies( cast_state_t *cs, int *enemies ) {
	int i, j, enemyCount, queryCount, friendlyAlertCount;
	float distances[MAX_CLIENTS];
	int oldEnemy, oldPauseTime;
	cast_state_t *ocs;

	if ( cs->script.castScriptStatus.scriptAttackEnt >= 0 ) {
		if ( g_entities[cs->script.castScriptStatus.scriptAttackEnt].health <= 0 ) {
			cs->script.castScriptStatus.scriptAttackEnt = -1;
		} else {
			// if we are not in combat mode, then an enemy should trigger a state change straight to combat mode
			if ( cs->aiState < AISTATE_COMBAT ) {
				AICast_StateChange( cs, AISTATE_COMBAT ); // just go straight to combat mode
			}
			enemies[0] = cs->script.castScriptStatus.scriptAttackEnt;
			return 1;
		}
	}

	if ( cs->script.castScriptStatus.scriptNoAttackTime >= level.time ) {
		return qfalse;
	}

	if ( cs->combat.noAttackTime >= level.time ) {
		return qfalse;
	}

	if ( cs->script.castScriptStatus.scriptNoSightTime >= level.time ) {
		return qfalse;
	}

	if ( cs->pauseTime > level.time ) {
		return qfalse;
	}

	enemyCount = 0;
	queryCount = 0;
	friendlyAlertCount = 0;

	// while we're here, may as well check for some baddies
	for ( i = 0; i < g_maxclients.integer; i++ ) {
		if ( g_entities[i].inuse ) {
			// try not to commit suicide
			if ( i != cs->entityNum ) {
				if ( AICast_EntityVisible( cs, i, qfalse ) ) {
					// how should we deal with them?
					if ( ( g_entities[i].health > 0 ) && AICast_HostileEnemy( cs, i ) ) { // visible and a baddy!
						enemies[enemyCount] = i;
						enemyCount++;
						queryCount = 0;
						friendlyAlertCount = 0;
					} else if ( !enemyCount && ( g_entities[i].health > 0 ) && AICast_QueryEnemy( cs, i ) &&
								( cs->perception.vislist[i].flags & AIVIS_PROCESS_SIGHTING ) ) {
						enemies[queryCount] = i;
						queryCount++;
						friendlyAlertCount = 0;
					} else if ( !queryCount && !enemyCount && ( cs->perception.vislist[i].flags & AIVIS_INSPECT ) ) {
						enemies[friendlyAlertCount] = i;
						friendlyAlertCount++;
					}
					// the sighting has been processed
					cs->perception.vislist[i].flags &= ~AIVIS_PROCESS_SIGHTING;
				}
			}
		}
	}

	if ( !enemyCount ) {
		if ( queryCount ) {
			enemyCount = queryCount;
		} else if ( friendlyAlertCount ) {
			enemyCount = friendlyAlertCount;
		}
	}

	if ( !enemyCount ) { // nothing worth doing anything about
		// look for audible events that we should investigate
		if ( cs->perception.audibleEventTime && cs->perception.audibleEventTime < level.time &&
			 cs->perception.audibleEventTime > level.time - 2000 ) {
			return -4;
		}
		// look for bullet impacts that have occured recently
		if ( cs->perception.bulletImpactTime && cs->perception.bulletImpactTime < level.time &&
			 cs->perception.bulletImpactTime > level.time - 1000 ) {
			return -3;
		}
		return 0;
	}

	// sort the enemies by distance
	for ( i = 0; i < enemyCount; i++ ) {
		distances[i] = Distance( cs->clientState->origin, g_entities[enemies[i]].client->ps.origin );
		if ( !distances[i] ) {
			G_Printf( "WARNING: zero distance between enemies:\n%s at %s, %s at %s\n", g_entities[cs->entityNum].aiName,
					  vtos( cs->clientState->origin ), g_entities[enemies[i]].aiName,
					  vtos( g_entities[enemies[i]].client->ps.origin ) );
			distances[i] = 999998; // try to ignore them (HACK)
		}
	}
	for ( i = 1; i < enemyCount; i++ ) {
		const int candidate = enemies[i];
		const float distance = distances[i];
		for ( j = i; j > 0 && distances[j - 1] > distance; j-- ) {
			enemies[j] = enemies[j - 1];
			distances[j] = distances[j - 1];
		}
		enemies[j] = candidate;
		distances[j] = distance;
	}

	// if we are not in combat mode, then an enemy should trigger a state change straight to combat mode
	if ( !queryCount && !friendlyAlertCount && enemyCount && cs->aiState < AISTATE_COMBAT ) {
		// face them while making the transition
		oldEnemy = cs->enemyNum;
		// set it temporarily
		cs->enemyNum = enemies[0];
		//
		AICast_AimAtEnemy( cs );
		AICast_StateChange( cs, AISTATE_COMBAT ); // just go straight to combat mode
		// set it back
		cs->enemyNum = oldEnemy;
	}

	// if we are in relaxed state, and we see a query enemy, then go into query mode
	if ( queryCount ) {
		if ( cs->aiState == AISTATE_RELAXED ) {
			// go into query mode
			if ( AICast_StateChange( cs, AISTATE_QUERY ) ) {
				cs->enemyNum = enemies[0]; // lock onto the closest potential enemy
				return -1;
			}
			return 0; // scripting obviously doesn't want us to progress from relaxed just yet
		}
		// else ignore the query mode, since we are already above relaxed mode
		return 0;
	}
	if ( friendlyAlertCount ) {
		// call a script event
		oldPauseTime = cs->script.scriptPauseTime;
		if ( g_entities[enemies[0]].health <= 0 ) {
			AICast_ForceScriptEvent( cs, "inspectbodystart", g_entities[enemies[0]].aiName );
			if ( cs->aiFlags & AIFL_DENYACTION ) {
				// ignore this friendly
				cs->perception.vislist[enemies[0]].flags |= AIVIS_INSPECTED; // they have been notified
				cs->perception.vislist[enemies[0]].flags &= ~AIVIS_INSPECT; // they have been notified
				return 0;
			}
		}
		//
		if ( cs->aiState < AISTATE_COMBAT ) {
			// go into alert mode, and return this entity so we can inspect it or something,
			// but let dynamic AI sort out what it wants to do
			if ( cs->aiState == AISTATE_ALERT || AICast_StateChange( cs, AISTATE_ALERT ) ) {
				// only return the entity if they are in combat mode or dead
				ocs = AICast_GetCastState( enemies[0] );
				if ( ( g_entities[enemies[0]].health <= 0 ) || ( ocs->aiState >= AISTATE_COMBAT ) ) {
					return -2;
				}
			}
			return 0; // scripting failed or they're not worth physically inspecting
		}
		// ignore the friendly, we have our hands full
		return 0;
	}

	// must be hostile enemy(s) found, so return them
	return enemyCount;
}

/*
==================
AICast_HostileEnemy

  returns qtrue if the entity is hostile
==================
*/
qboolean AICast_HostileEnemy( cast_state_t *cs, int enemynum ) {
	// if we hate them, they are an enemy
	if ( cs->perception.vislist[enemynum].flags & AIVIS_ENEMY ) {
		return qtrue;
	} else {
		return qfalse;
	}
}

/*
==================
AICast_QueryEnemy

  returns qtrue if the entity can become hostile (they hurt us or we recognize them)
==================
*/
qboolean AICast_QueryEnemy( cast_state_t *cs, int enemynum ) {

	if ( g_entities[cs->entityNum].aiTeam != g_entities[enemynum].aiTeam ) {
		if ( g_entities[cs->entityNum].aiTeam == AITEAM_MONSTER || g_entities[enemynum].aiTeam == AITEAM_MONSTER ) {
			// monsters hate all non-monsters and vice-versa
			return qtrue;
		} else {
			// neutral's can only possibly become hostile if they hurt us, otherwise they are assumed to be harmless
			if ( g_entities[cs->entityNum].aiTeam == AITEAM_NEUTRAL || g_entities[enemynum].aiTeam == AITEAM_NEUTRAL ) {
				return qfalse; // one of us is neutral, assume harmless
			}
			return qtrue;
		}
	} else { // same team
		return qfalse; // same team
	}
}

/*
==================
AICast_SameTeam

  the player is always team 1, AI's default to team 0

  MONSTER's hate everyone else except other MONSTER's

  NEUTRAL's are cool with everyone that hasn't hurt them
==================
*/
qboolean AICast_SameTeam( cast_state_t *cs, int enemynum ) {

	if ( g_entities[cs->entityNum].aiTeam != g_entities[enemynum].aiTeam ) {
		if ( g_entities[cs->entityNum].aiTeam == AITEAM_NEUTRAL || g_entities[enemynum].aiTeam == AITEAM_NEUTRAL ) {
			// if we hate them, they are an enemy
			if ( cs->perception.vislist[enemynum].flags & AIVIS_ENEMY ) {
				return qfalse;
			} else {
				return qtrue;
			}
		} else {
			return qfalse; // they are an enemy
		}
	} else {
		return qtrue; // same team
	}
}
