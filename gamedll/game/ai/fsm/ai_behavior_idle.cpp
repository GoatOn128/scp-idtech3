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
============
AIFunc_Idle()

  The cast AI is standing around, contemplating the meaning of life
============
*/
char *AIFunc_Idle( cast_state_t *cs ) {
	int enemies[MAX_CLIENTS];
	int numEnemies;
	gentity_t *ent = &g_entities[cs->entityNum];

	// we are in an idle state, looking for something to do

	//
	// do we need to avoid a danger?
	if ( cs->dangerEntityValidTime >= level.time ) {
		if ( !AICast_GetTakeCoverPos( cs, cs->dangerEntity, cs->dangerEntityPos, cs->combat.takeCoverPos ) ) {
			// shit??
		}
		// go to a position that cannot be seen from the dangerPos
		cs->combat.takeCoverTime = cs->dangerEntityValidTime + 1000;
		cs->attackcrouch_time = 0;
		return AIFunc_AvoidDangerStart( cs );
	}
	//
	// are we waiting for a door?
	if ( cs->navigation.doorMarkerTime > level.time - 100 ) {
		return AIFunc_DoorMarkerStart( cs, cs->navigation.doorMarkerDoor, cs->navigation.doorMarkerNum );
	}
	//
	// do we need to go to our leader?
	if ( cs->leaderNum >= 0 &&
		 Distance( cs->clientState->origin, g_entities[cs->leaderNum].r.currentOrigin ) > MAX_LEADER_DIST ) {
		return AIFunc_ChaseGoalStart( cs, cs->leaderNum, AICAST_LEADERDIST_MAX, qtrue );
	}
	//
	// look for things we should attack
	numEnemies = AICast_ScanForEnemies( cs, enemies );
	if ( numEnemies == -1 ) { // query mode
		return NULL;
	} else if ( numEnemies == -2 ) { // inspection may be required
		char *retval;
		// TTimo: gcc: suggest () around assignment used as truth value
		if ( ( retval = AIFunc_InspectFriendlyStart( cs, enemies[0] ) ) ) {
			return retval;
		}
	} else if ( numEnemies == -3 ) { // bullet impact
		if ( cs->aiState < AISTATE_COMBAT ) {
			return AIFunc_InspectBulletImpactStart( cs );
		}
	} else if ( numEnemies == -4 ) { // audible event
		if ( cs->aiState < AISTATE_COMBAT ) {
			return AIFunc_InspectAudibleEventStart( cs, cs->perception.audibleEventEnt );
		}
	} else if ( numEnemies > 0 ) {
		int i;

		cs->enemyNum = -1;
		// choose an enemy
		for ( i = 0; i < numEnemies; i++ ) {
			if (
				Distance( cs->clientState->origin, cs->perception.vislist[enemies[i]].visible_pos ) >
				16 ) { // if we are really close to the last place we saw them, no point trying to attack, since we'll just end up back here
				if ( cs->enemyNum < 0 ) {
					cs->enemyNum = enemies[i];
				} else if ( AICast_CheckAttack( cs, enemies[i], qfalse ) ) {
					cs->enemyNum = enemies[i];
					return AIFunc_BattleStart( cs );
				}
			}
		}
		if ( cs->enemyNum >= 0 ) {
			if ( ( ( cs->leaderNum < 0 ) || ( cs->thinkFuncChangeTime < level.time - 3000 ) ) &&
				 AICast_WantsToChase( cs ) ) { // don't leave our leader as soon as we get to them
				return AIFunc_BattleStart( cs );
			} else if ( AICast_EntityVisible( AICast_GetCastState( cs->enemyNum ), cs->entityNum, qtrue ) ||
						AICast_CheckAttack( AICast_GetCastState( cs->enemyNum ), cs->entityNum, qfalse ) ) {
				// if we are tactical enough, look for a hiding spot
				if ( !( cs->leaderNum >= 0 ) && cs->attributes[TACTICAL] > 0.4 && cs->attributes[AGGRESSION] < 1.0 ) {
					// they can see us, and we want to hide from them
					if ( AICast_GetTakeCoverPos( cs, cs->enemyNum, cs->perception.vislist[cs->enemyNum].visible_pos,
												 cs->combat.takeCoverPos ) ) {
						// go to a position that cannot be seen from the last place we saw the enemy, and wait there for some time
						cs->combat.takeCoverTime = level.time + 2000 + rand() % 3000;
						return AIFunc_BattleTakeCoverStart( cs );
					}
				}
				// attack them if nothing else to do, since they can attack us here
				return AIFunc_BattleStart( cs );
			} else if ( cs->leaderNum < 0 ) { // we should pursue if no leader, and not wanting to hide
				return AIFunc_BattleStart( cs );
			} else {
				// they can't see us anyway, so ignore them
				cs->lastEnemy = cs->enemyNum; // at least face them if they come to get us
				cs->enemyNum = -1;
				// crouching makes us look like we are hiding, which is what we are doing
				if ( cs->attributes[ATTACK_CROUCH] > 0.5 ) {
					cs->attackcrouch_time = level.time + 1000;
				}
			}
		}
	}
	//
	// if we are in combat mode, then we should relax, since we dont have an enemy
	if ( cs->aiState >= AISTATE_COMBAT ) {
		AICast_StateChange( cs, AISTATE_ALERT );
	}
	//
	// if we couldn't find anything, see if our previous enemy is still around, if so, go find them
	// this is an attempt to prevent guys from running away to hide from something, never to
	// be seen again. They shouldn't really "forget" that they are indeed soldiers.
	if ( !( cs->leaderNum >= 0 ) && cs->lastEnemy >= 0 && g_entities[cs->lastEnemy].health > 0 &&
		 cs->perception.vislist[cs->lastEnemy].real_visible_timestamp < level.time - 5000 &&
		 cs->combat.takeCoverTime < level.time - 5000 ) {
		cs->enemyNum = cs->lastEnemy; // just go to the place we last saw them
		return AIFunc_BattleStart( cs );
	}
	//
	// if we've recently been in a fight, keep looking around, so we don't look stupid
	if ( cs->lastEnemy >= 0 ) {
		// we have been in a battle, so face our enemy in anticipation of their return
		if ( ent->aiTeam != AITEAM_ALLIES ) {
			vec3_t dir;
			//
			//VectorSubtract( cs->perception.vislist[cs->lastEnemy].visible_pos, cs->clientState->origin, dir );
			// hack, use their real angles, prevent them from looking dumb when the player returns
			VectorSubtract( g_entities[cs->lastEnemy].s.pos.trBase, cs->clientState->origin, dir );
			if ( VectorLength( dir ) < 1 ) {
				cs->ideal_viewangles[PITCH] = 0;
			} else {
				VectorNormalize( dir );
				vectoangles( dir, cs->ideal_viewangles );
				cs->ideal_viewangles[PITCH] = AngleNormalize180( cs->ideal_viewangles[PITCH] ) * 0.5;
			}
		} else if ( cs->attributes[TACTICAL] && cs->nextIdleAngleChange < level.time ) {
			// wait a second before changing again
			if ( ( cs->nextIdleAngleChange + 3000 ) < level.time ) {

				// FIXME: This could be changed to use some AAS sampling, which would:
				//
				//	Given a src area, pick a random dest area which is visible from that area
				//	and return it's position, which we'd then use to set the next view angles
				//
				//	This would result in more efficient, more realistic behaviour, since they'd
				//	also use PITCH angles to look at areas above/below them

				cs->idleYaw = AICast_GetRandomViewAngle( cs, 512 );

				if ( fabs( AngleDelta( cs->idleYaw, cs->ideal_viewangles[YAW] ) ) < 45 ) {
					cs->nextIdleAngleChange = level.time + 1000 + rand() % 2500;
				} else { // do really fast
					cs->nextIdleAngleChange = level.time + 500;
				}

				// adjust with time
				cs->idleYawChange = AngleDelta( cs->idleYaw, cs->ideal_viewangles[YAW] );
				/// ((float)(cs->nextIdleAngleChange - level.time) / 1000.0);

				cs->ideal_viewangles[PITCH] = 0;
			}
		} else if ( cs->idleYawChange ) {
			cs->idleYawChange = AngleDelta( cs->idleYaw, cs->ideal_viewangles[YAW] );
			cs->ideal_viewangles[YAW] =
				AngleMod( cs->ideal_viewangles[YAW] + ( cs->idleYawChange * cs->clientState->thinktime ) );
		}
	}

	// check for a movement we should be making
	if ( cs->navigation.obstructingTime > level.time ) {
		AICast_MoveToPos( cs, cs->navigation.obstructingPos, -1 );
		if ( cs->movestate != MS_CROUCH ) {
			cs->movestate = MS_WALK;
		}
		cs->movestateType = MSTYPE_TEMPORARY;
	}

	// set head look flag if no enemy
	if ( cs->enemyNum < 0 && cs->attributes[TACTICAL] >= 0.5 && !( cs->aiFlags & AIFL_NO_HEADLOOK ) ) {
		g_entities[cs->entityNum].client->ps.eFlags |= EF_HEADLOOK;
	}

	// reload?
	AICast_IdleReload( cs );

	return NULL;
}

/*
============
AIFunc_IdleStart()
============
*/
char *AI_EnterIdle( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	ent->flags &= ~FL_AI_GRENADE_KICK;
	cs->navigation.followEntity = -1;

	const bool hasEnemy = cs->enemyNum >= 0 && cs->enemyNum < MAX_CLIENTS && g_entities[cs->enemyNum].inuse;
	if ( hasEnemy && ent->aiTeam == AITEAM_NAZI && g_entities[cs->enemyNum].health <= 0 ) {
		return AIFunc_InspectBodyStart( cs );
	}
	cs->enemyNum = -1;
	trap_BotInitAvoidReach( cs->clientState->ms );
	if ( cs->aiFlags & AIFL_STAND_IDLE2 ) {
		ent->client->ps.eFlags |= EF_STAND_IDLE2;
	}
	return "Idle";
}

char *AIFunc_IdleStart( cast_state_t *cs ) {
	return AI_RequestState( cs, AIStateId::Idle, AITransitionReason::Decision );
}
