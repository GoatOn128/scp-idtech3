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
AIFunc_ChaseGoalIdle()
============
*/
char *AIFunc_ChaseGoalIdle( cast_state_t *cs ) {
	int enemies[MAX_CLIENTS];
	int numEnemies;
	gentity_t *followent;
	vec3_t dir;

	if ( cs->navigation.followEntity < 0 ) {
		AICast_EndChase( cs );
		return AIFunc_IdleStart( cs );
	}

	followent = &g_entities[cs->navigation.followEntity];

	// CHECK: will this interfere with scripting?
	//
	// do we need to avoid a danger?
	if ( cs->dangerEntityValidTime >= level.time ) {
		if ( AICast_GetTakeCoverPos( cs, cs->dangerEntity, cs->dangerEntityPos, cs->combat.takeCoverPos ) ) {
			// go to a position that cannot be seen from the dangerPos
			cs->combat.takeCoverTime = cs->dangerEntityValidTime + 1000;
			cs->attackcrouch_time = 0;
			return AIFunc_AvoidDangerStart( cs );
		}
	}
	//
	// are we waiting for a door?
	if ( cs->navigation.doorMarkerTime > level.time - 100 ) {
		return AIFunc_DoorMarkerStart( cs, cs->navigation.doorMarkerDoor, cs->navigation.doorMarkerNum );
	}
	//
	// if the player is not ready yet, wait
	if ( !followent->inuse ) {
		return NULL;
	}

	// has the scripting stopped asking us to pursue this goal?
	if ( cs->navigation.followIsGoto && ( cs->navigation.followTime < level.time ) ) {
		return AIFunc_Idle( cs );
	}

	// they are ready, are they outside range?
	if ( Distance( followent->r.currentOrigin, cs->clientState->origin ) > cs->navigation.followDist ) {
		return AIFunc_ChaseGoalStart( cs, cs->navigation.followEntity, cs->navigation.followDist, qtrue );
	}

	// check for a movement we should be making
	if ( cs->navigation.obstructingTime > level.time ) {
		AICast_MoveToPos( cs, cs->navigation.obstructingPos, -1 );
		cs->speedScale = cs->attributes[WALKING_SPEED] / cs->attributes[RUNNING_SPEED];
	}
	// if we have an enemy, fire if they're visible
	else if ( cs->enemyNum >= 0 ) {
		//attack the enemy if possible
		AICast_ProcessAttack( cs );
	}
	// if we had an enemy recently, face them
	else if ( cs->lastEnemy >= 0 ) {
		vec3_t dir;
		//
		VectorSubtract( cs->perception.vislist[cs->lastEnemy].visible_pos, cs->clientState->origin, dir );
		if ( VectorLength( dir ) < 1 ) {
			cs->ideal_viewangles[PITCH] = 0;
		} else {
			VectorNormalize( dir );
			vectoangles( dir, cs->ideal_viewangles );
		}
		// reload?
		AICast_IdleReload( cs );
	} else if ( followent->client ) {
		// face them
		VectorSubtract( followent->r.currentOrigin, cs->clientState->origin, dir );
		dir[2] += followent->client->ps.viewheight - g_entities[cs->clientState->entitynum].client->ps.viewheight;
		VectorNormalize( dir );
		vectoangles( dir, cs->ideal_viewangles );
	}

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
		cs->enemyNum = enemies[0]; // just attack the first one
	}

	// set head look flag if no enemy
	if ( cs->enemyNum < 0 && cs->attributes[TACTICAL] >= 0.5 && !( cs->aiFlags & AIFL_NO_HEADLOOK ) ) {
		g_entities[cs->entityNum].client->ps.eFlags |= EF_HEADLOOK;
	}

	return NULL;
}

/*
============
AIFunc_ChaseGoalIdleStart()
============
*/
char *AIFunc_ChaseGoalIdleStart( cast_state_t *cs, int entitynum, float reachdist ) {
	// make sure we don't avoid any areas when we start again
	trap_BotInitAvoidReach( cs->clientState->ms );

	// if we are following someone, always use the default (ready for action) anim
	if ( entitynum < MAX_CLIENTS ) {
		g_entities[cs->entityNum].client->ps.eFlags &= ~EF_STAND_IDLE2;
	} else {
		// randomly choose idle animation
		//----(SA)	try always using the 'casual' stand on spawn and change to crouching one when 'alerted'
		if ( cs->aiFlags & AIFL_STAND_IDLE2 ) {
			//			if (cs->lastEnemy < 0)
			g_entities[cs->entityNum].client->ps.eFlags |= EF_STAND_IDLE2;
			//			else
			//				g_entities[cs->entityNum].client->ps.eFlags &= ~EF_STAND_IDLE2;
		}
	}

	cs->navigation.followEntity = entitynum;
	cs->navigation.followDist = reachdist;
	AI_SetLegacyState( cs, AIFunc_ChaseGoalIdle );
	return "AIFunc_ChaseGoalIdle";
}

/*
============
AIFunc_ChaseGoal()
============
*/
char *AIFunc_ChaseGoal( cast_state_t *cs ) {
	int enemies[MAX_CLIENTS];
	bot_moveresult_t *moveresult;
	gentity_t *followent, *ent;
	ai_client_state_t *bs;
	vec3_t destorg;
	float dist;
	qboolean moved = qfalse;

	ent = &g_entities[cs->entityNum];

	if ( cs->navigation.followEntity < 0 ) {
		AICast_EndChase( cs );
		return AIFunc_IdleStart( cs );
	}

	// CHECK: will this mess with scripting?
	//
	// do we need to avoid a danger?
	if ( cs->dangerEntityValidTime >= level.time ) {
		if ( AICast_GetTakeCoverPos( cs, cs->dangerEntity, cs->dangerEntityPos, cs->combat.takeCoverPos ) ) {
			// go to a position that cannot be seen from the dangerPos
			cs->combat.takeCoverTime = cs->dangerEntityValidTime + 1000;
			cs->attackcrouch_time = 0;
			cs->script.castScriptStatus.scriptGotoId = -1;
			cs->movestate = MS_DEFAULT;
			cs->movestateType = MSTYPE_NONE;
			return AIFunc_AvoidDangerStart( cs );
		}
	}
	//
	// are we waiting for a door?
	if ( cs->navigation.doorMarkerTime > level.time - 100 ) {
		return AIFunc_DoorMarkerStart( cs, cs->navigation.doorMarkerDoor, cs->navigation.doorMarkerNum );
	}

	followent = &g_entities[cs->navigation.followEntity];

	// if the entity is not ready yet
	if ( !followent->inuse ) {
		// if it's a connecting client, wait
		if ( cs->navigation.followEntity < MAX_CLIENTS &&
			 ( ( followent->client && followent->client->pers.connected == CON_CONNECTING ) ||
			   ( level.time < 3000 ) ) ) {
			return AIFunc_ChaseGoalIdleStart( cs, cs->navigation.followEntity, cs->navigation.followDist );
		} else // stop following it
		{
			AICast_EndChase( cs );
			return AIFunc_IdleStart( cs );
		}
	}

	// has the scripting stopped asking us to pursue this goal?
	if ( cs->navigation.followIsGoto && ( cs->navigation.followTime < level.time ) ) {
		return AIFunc_IdleStart( cs );
	}

	if ( followent->client ) {
		VectorCopy( followent->client->ps.origin, destorg );
	} else {
		VectorCopy( followent->r.currentOrigin, destorg );
	}

	// they are ready, are they inside range? FIXME: make configurable
	dist = Distance( destorg, cs->clientState->origin );
	if ( cs->navigation.followSlowApproach && dist < cs->navigation.followDist &&
		 ( ent->waterlevel || ( cs->clientState->cur_ps.groundEntityNum != ENTITYNUM_NONE ) ) ) {
		// if this is a scripted GOTO, stop following now
		if ( cs->navigation.followEntity == cs->script.castScriptStatus.scriptGotoEnt ) {
			AICast_EndChase( cs );
			return AIFunc_IdleStart( cs );
		}
		// if we have reached our leader
		else {
			if ( cs->navigation.followEntity == cs->leaderNum ) {
				if ( dist < AICAST_LEADERDIST_MIN ) {
					AICast_EndChase( cs );
					return AIFunc_IdleStart( cs );
				} else {
					trace_t tr;
					// if we have a clear line to our leader, move closer, since there may be others following also
					trap_Trace( &tr, cs->clientState->origin, cs->clientState->cur_ps.mins,
								cs->clientState->cur_ps.maxs, g_entities[cs->navigation.followEntity].r.currentOrigin,
								cs->entityNum, g_entities[cs->entityNum].clipmask );
					if ( tr.entityNum != cs->navigation.followEntity ) {
						AICast_EndChase( cs );
						return AIFunc_IdleStart( cs );
					}
					// if we have crouching ability, then use it while we are just moving closer
					if ( cs->attributes[ATTACK_CROUCH] > 0.1 ) {
						cs->attackcrouch_time = level.time + 1000;
					}
				}
			} else {
				return AIFunc_ChaseGoalIdleStart( cs, cs->navigation.followEntity, cs->navigation.followDist );
			}
		}
	}
	//
	// go to them
	//
	bs = cs->clientState;
	//
	// RF, disabled this, MIKE sees dead people
	//if (followent->client && followent->health <= 0) {
	//	AICast_EndChase( cs );
	//	return AIFunc_IdleStart(cs);
	//}

	// move straight to them if we can
	if ( !moved &&
		 ( cs->clientState->cur_ps.groundEntityNum != ENTITYNUM_NONE || g_entities[cs->entityNum].waterlevel > 1 ) ) {
		aicast_predictmove_t move;
		vec3_t dir;
		bot_input_t bi;
		usercmd_t ucmd;
		trace_t tr;
		qboolean simTest = qfalse;
		float frameTime = 0.8, goaldist;

		if ( cs->attributes[RUNNING_SPEED] < 120 ) {
			simTest = qtrue;
		}

		if ( !simTest ) {
			// trace will eliminate most unsuccessful paths
			trap_Trace( &tr, cs->clientState->origin, NULL /*g_entities[cs->entityNum].r.mins*/,
						NULL /*g_entities[cs->entityNum].r.maxs*/, followent->r.currentOrigin, cs->entityNum,
						g_entities[cs->entityNum].clipmask );
			if ( tr.entityNum == cs->navigation.followEntity || tr.fraction == 1 ) {
				simTest = qtrue;
			}
		}

		if ( simTest ) {
			// try walking straight to them
			VectorSubtract( followent->r.currentOrigin, cs->clientState->origin, dir );
			if ( !ent->waterlevel ) {
				dir[2] = 0;
			}
			goaldist = VectorNormalize( dir );
			//trap_EA_Move(cs->entityNum, dir, 400);
			trap_EA_GetInput( cs->entityNum, (float)level.time / 1000, &bi );
			VectorCopy( dir, bi.dir );
			bi.speed = 400;
			bi.actionflags = 0;
			AICast_InputToUserCommand( cs, &bi, &ucmd, bs->cur_ps.delta_angles );
			AICast_PredictMovement( cs, 10, frameTime, &move, &ucmd, cs->navigation.followEntity );

			if ( move.stopevent == PREDICTSTOP_HITENT ) { // success!
				// make sure we didnt spend a lot of time sliding along an obstacle
				if ( ( move.frames * frameTime ) <
					 ( 1.0 + ( goaldist / ( bs->cur_ps.speed * bs->cur_ps.runSpeedScale ) ) ) ) {
					trap_EA_Move( cs->entityNum, dir, 400 );
					vectoangles( dir, cs->ideal_viewangles );
					cs->ideal_viewangles[2] *= 0.5;
					moved = qtrue;
				}
			}
			if ( !moved ) {
				//trap_EA_Move(cs->entityNum, dir, 0);
			}
		}
	}
	//
	if ( !moved ) {
		// use AAS routing
		moveresult = AICast_MoveToPos( cs, followent->r.currentOrigin, cs->navigation.followEntity );
		if ( moveresult && moveresult->failure ) {
			// shit?
		}
	}

	// should we slow down?
	if ( cs->navigation.followDist && cs->navigation.followSlowApproach && cs->navigation.followDist < 48 ) {
		cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, cs->navigation.followDist );
	}

	// check for a movement we should be making
	if ( cs->navigation.obstructingTime > level.time ) {
		AICast_MoveToPos( cs, cs->navigation.obstructingPos, -1 );
		if ( cs->movestate != MS_CROUCH ) {
			cs->movestate = MS_WALK;
		}
		cs->movestateType = MSTYPE_TEMPORARY;
	}

	// if we have an enemy, fire if they're visible
	if ( cs->enemyNum >= 0 ) { //attack the enemy if possible
		AICast_ProcessAttack( cs );
	} else {
		int numEnemies;
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

			cs->enemyNum = enemies[0]; // just attack the first one
			// override with a visible enemy
			for ( i = 1; i < numEnemies; i++ ) {
				if ( AICast_CheckAttack( cs, enemies[i], qfalse ) ) {
					cs->enemyNum = enemies[i];
					break;
				} else if ( cs->enemyNum < 0 ) {
					cs->lastEnemy = enemies[i];
				}
			}
		}
		// reload?
		AICast_IdleReload( cs );
	}

	// set head look flag if no enemy
	if ( cs->enemyNum < 0 && cs->attributes[TACTICAL] >= 0.5 && !( cs->aiFlags & AIFL_NO_HEADLOOK ) ) {
		g_entities[cs->entityNum].client->ps.eFlags |= EF_HEADLOOK;
	}

	return NULL;
}

/*
============
AIFunc_ChaseGoalStart()
============
*/
char *AIFunc_ChaseGoalStart( cast_state_t *cs, int entitynum, float reachdist, qboolean slowApproach ) {
	cs->navigation.followEntity = entitynum;
	cs->navigation.followDist = reachdist;
	cs->navigation.followIsGoto = qfalse;
	cs->navigation.followSlowApproach = slowApproach;
	AI_SetLegacyState( cs, AIFunc_ChaseGoal );
	return "AIFunc_ChaseGoal";
}

/*
=============
AIFunc_BattleHunt()
=============
*/
char *AIFunc_BattleHunt( cast_state_t *cs ) {
	int enemies[MAX_CLIENTS];
	bot_moveresult_t *moveresult;
	const float chaseDist = 32;
	gentity_t *followent, *ent;
	ai_client_state_t *bs;
	vec3_t destorg;
	qboolean moved = qfalse;
	//	gclient_t *client = &level.clients[cs->entityNum];
	char *rval;
	float dist;
	cast_state_t *ocs;
	int *ammo, i;

	ent = &g_entities[cs->entityNum];

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
	bs = cs->clientState;
	//
	if ( cs->enemyNum < 0 ) {
		return AIFunc_IdleStart( cs );
	}
	//
	ocs = AICast_GetCastState( cs->enemyNum );
	//
	if ( cs->aiFlags & AIFL_ATTACK_CROUCH ) {
		cs->attackcrouch_time = level.time + 1000;
	}
	//
	followent = &g_entities[cs->enemyNum];
	//
	// if the entity is not ready yet
	if ( !followent->inuse ) {
		// if it's a connecting client, wait
		if ( !( ( cs->enemyNum < MAX_CLIENTS ) &&
				( ( followent->client && followent->client->pers.connected == CON_CONNECTING ) ||
				  ( level.time < 3000 ) ) ) ) { // they don't exist anymore, stop attacking
			cs->enemyNum = -1;
		}

		return AIFunc_IdleStart( cs );
	}
	//
	// if we can see them, go back to an attack state
	AICast_ChooseWeapon( cs, qtrue ); // enable special weapons, if we cant get them, change back
	if ( AICast_EntityVisible( cs, cs->enemyNum, qtrue ) // take into account reaction time
		 && AICast_CheckAttack( cs, cs->enemyNum, qfalse ) && cs->navigation.obstructingTime < level.time ) {
		if ( AICast_StopAndAttack( cs ) ) {
			// TTimo: gcc: suggest () around assignment used as truth value
			if ( ( rval = AIFunc_BattleStart( cs ) ) ) {
				return rval;
			}
		} else { // just attack them now and keep chasing
			AICast_ProcessAttack( cs );
		}
		AICast_ChooseWeapon( cs, qfalse );
	} else {
		int numEnemies, shouldAttack;

		AICast_ChooseWeapon( cs, qfalse );

		ammo = cs->clientState->cur_ps.ammo;
		shouldAttack = qfalse;
		numEnemies = AICast_ScanForEnemies( cs, enemies );
		if ( numEnemies == -1 ) { // query mode
			return NULL;
		} else if ( numEnemies == -2 ) { // inspection may be required
			char *retval;
			if ( cs->aiState < AISTATE_COMBAT ) {
				// TTimo: gcc: suggest () around assignment used as truth value
				if ( ( retval = AIFunc_InspectFriendlyStart( cs, enemies[0] ) ) ) {
					return retval;
				}
			}
		} else if ( numEnemies == -3 ) { // bullet impact
			if ( cs->aiState < AISTATE_COMBAT ) {
				return AIFunc_InspectBulletImpactStart( cs );
			}
		} else if ( numEnemies == -4 ) { // audible event
			if ( cs->aiState < AISTATE_COMBAT ) {
				return AIFunc_InspectAudibleEventStart( cs, cs->perception.audibleEventEnt );
			}
		} else if ( AICast_GotEnoughAmmoForWeapon( cs, cs->clientState->cur_ps.weapon ) ) {
			if ( numEnemies > 0 ) {
				// default to the first known enemy, overwrite if we find a clearer shot
				cs->enemyNum = enemies[0];
				//
				for ( i = 0; i < numEnemies; i++ ) {
					if ( AICast_CheckAttack( cs, enemies[i], qfalse ) ||
						 AICast_CheckAttack( AICast_GetCastState( enemies[i] ), cs->entityNum, qfalse ) ) {
						cs->enemyNum = enemies[i];
						shouldAttack = qtrue;
						break;
					} else if ( cs->enemyNum < 0 ) {
						cs->lastEnemy = enemies[i];
					}
				}
				// note: next frame we'll process this new enemy an begin an attack if necessary
			}
		}
		AICast_ChooseWeapon( cs, qfalse );
	}

	// have we spent enough time in combat mode?
	if ( cs->aiState == AISTATE_COMBAT ) {
		if ( cs->perception.vislist[cs->enemyNum].visible_timestamp < level.time - COMBAT_TIMEOUT ) {
			AICast_StateChange( cs, AISTATE_ALERT );
		}
	}

	// while hunting, use crouch mode if possible
	if ( cs->attributes[ATTACK_CROUCH] >= 0.1 ) {
		cs->attackcrouch_time = level.time + 1000;
	}

	if ( cs->combat.battleHuntPauseTime ) {
		if ( cs->combat.battleHuntPauseTime < level.time ) {
			// pausetime has expired, so go into ambush mode
			if ( AICast_GetTakeCoverPos(
					 cs, cs->enemyNum, cs->perception.vislist[cs->enemyNum].chase_marker[cs->combat.battleChaseMarker],
					 cs->combat.takeCoverPos ) ) {
				// wait in ambush, for them to return
				VectorCopy( cs->perception.vislist[cs->enemyNum].chase_marker[cs->combat.battleChaseMarker],
							cs->navigation.combatGoalOrigin );
				return AIFunc_BattleAmbushStart( cs );
			}
			// couldn't find a spot, so just stay here?
			VectorCopy( cs->clientState->origin, cs->navigation.combatGoalOrigin );
			VectorCopy( cs->clientState->origin, cs->combat.takeCoverPos );
			return AIFunc_BattleAmbushStart( cs );
		} else {
			// stay here, looking around
			if ( cs->combat.battleHuntViewTime < level.time ) {
				cs->combat.battleHuntViewTime = level.time + 700 + rand() % 1000;
				// set a random viewangle
				cs->ideal_viewangles[YAW] =
					AngleMod( cs->ideal_viewangles[YAW] + ( 45.0 + random() * 45.0 ) * ( 2 * ( rand() % 2 ) - 1 ) );
				cs->ideal_viewangles[PITCH] = 0;
			}
		}
	} else {
		// cycle through markers
		VectorCopy( cs->perception.vislist[cs->enemyNum].chase_marker[cs->combat.battleChaseMarker], destorg );
		if ( ( dist = Distance( destorg, cs->clientState->origin ) ) < chaseDist ) {
			if ( cs->combat.battleChaseMarker == ( cs->perception.vislist[cs->enemyNum].chase_marker_count - 1 ) ) {
				cs->combat.battleHuntPauseTime = level.time + 4000;
				cs->combat.battleHuntViewTime = level.time + 1000;
			} else {
				cs->combat.battleChaseMarker += cs->combat.battleChaseMarkerDir;
				if ( cs->combat.battleChaseMarker > cs->perception.vislist[cs->enemyNum].chase_marker_count ) {
					cs->combat.battleChaseMarkerDir *= -1;
					cs->combat.battleChaseMarker = cs->perception.vislist[cs->enemyNum].chase_marker_count - 1;
				}
				if ( cs->combat.battleChaseMarker < 0 ) {
					cs->combat.battleChaseMarkerDir = 1;
					cs->combat.battleChaseMarker = 0;
				}
			}
		}
		//
		if ( cs->combat.battleHuntPauseTime < level.time ) {
			// just go to them
			if ( !moved && cs->leaderNum < 0 ) {
				moveresult = AICast_MoveToPos( cs, destorg, cs->enemyNum );
				if ( moveresult && moveresult->failure ) { // no path, so go back to idle behaviour
					// try to go to ambush mode
					cs->enemyNum = -1;
					return AIFunc_DefaultStart( cs );
				} else {
					moved = qtrue;
				}
				// slow down real close to the goal, so we don't go passed it
				cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, chaseDist );
			}
		}
	}

	// reload?
	AICast_IdleReload( cs );

	return NULL;
}

/*
=============
AIFunc_BattleHuntStart()
=============
*/
char *AIFunc_BattleHuntStart( cast_state_t *cs ) {
	cs->navigation.combatGoalTime = 0;
	cs->combat.battleChaseMarker = 0;
	cs->combat.battleChaseMarkerDir = 1;
	cs->combat.battleHuntPauseTime = 0;
	//
	AI_SetLegacyState( cs, AIFunc_BattleHunt );
	return "AIFunc_BattleHunt";
}

/*
============
AIFunc_BattleChase()
============
*/
char *AIFunc_BattleChase( cast_state_t *cs ) {
	int enemies[MAX_CLIENTS];
	int numEnemies;
	bot_moveresult_t *moveresult;
	const float chaseDist = 32;
	gentity_t *followent, *ent;
	ai_client_state_t *bs;
	vec3_t destorg;
	qboolean moved = qfalse;
	gclient_t *client = &level.clients[cs->entityNum];
	char *rval;
	float dist;
	cast_state_t *ocs;

	ent = &g_entities[cs->entityNum];

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

	bs = cs->clientState;
	//
	if ( cs->enemyNum < 0 ) {
		return AIFunc_IdleStart( cs );
	}
	//
	// Retreat?
	if ( AICast_WantToRetreat( cs ) ) {
		if ( AICast_GetTakeCoverPos( cs, cs->enemyNum, cs->perception.vislist[cs->enemyNum].visible_pos,
									 cs->combat.takeCoverPos ) ) {
			// go to a position that cannot be seen from the last place we saw the enemy, and wait there for some time
			cs->combat.takeCoverTime = level.time + 2000 + rand() % 3000;
			return AIFunc_BattleTakeCoverStart( cs );
		}
	}
	//
	ocs = AICast_GetCastState( cs->enemyNum );
	//
	if ( cs->aiFlags & AIFL_ATTACK_CROUCH ) {
		if ( cs->attackcrouch_time > level.time || ( cs->thinkFuncChangeTime < level.time - 1000 ) ) {
			cs->attackcrouch_time = level.time + 1000;
		}
	}
	//
	followent = &g_entities[cs->enemyNum];
	//
	// if the entity is not ready yet
	if ( !followent->inuse ) {
		// if it's a connecting client, wait
		if ( !( ( cs->enemyNum < MAX_CLIENTS ) &&
				( ( followent->client && followent->client->pers.connected == CON_CONNECTING ) ||
				  ( level.time < 3000 ) ) ) ) { // they don't exist anymore, stop attacking
			cs->enemyNum = -1;
		}

		return AIFunc_IdleStart( cs );
	}
	//
	// if we can see them, go back to an attack state
	AICast_ChooseWeapon( cs, qtrue ); // enable special weapons, if we cant get them, change back
	if ( AICast_EntityVisible( cs, cs->enemyNum, qtrue ) // take into account reaction time
		 && AICast_CheckAttack( cs, cs->enemyNum, qfalse ) && cs->navigation.obstructingTime < level.time ) {
		if ( AICast_StopAndAttack( cs ) ) {
			// TTimo: gcc: suggest () around assignment used as truth value
			if ( ( rval = AIFunc_BattleStart( cs ) ) ) {
				return rval;
			}
		} else { // just attack them now and keep chasing
			AICast_ProcessAttack( cs );
		}
		AICast_ChooseWeapon( cs, qfalse );
	} else {
		AICast_ChooseWeapon( cs, qfalse );
		// not visible, go to their previously visible position
		/*
		if (!cs->perception.vislist[cs->enemyNum].visible_timestamp || Distance( bs->origin, cs->perception.vislist[cs->enemyNum].visible_pos ) < 16)
		{
			// we're done attacking, go back to default state, which in turn will recall previous state
			//
			return AIFunc_DefaultStart( cs );
		}
		*/
	}
	//
	// find the chase position
	if ( followent->client ) {
		// go to the last visible position
		VectorCopy( cs->perception.vislist[cs->enemyNum].visible_pos, destorg );
		// if we have reached it, go into hunt mode
		if ( ( dist = Distance( destorg, cs->clientState->origin ) ) < chaseDist ) {
			// if we haven't been hunted for a while, do so
			if ( ocs->combat.lastBattleHunted < level.time - 5000 ) {
				ocs->combat.lastBattleHunted = level.time;
				return AIFunc_BattleHuntStart( cs );
			}
			// otherwise, go into ambush mode
			if ( AICast_GetTakeCoverPos( cs, cs->enemyNum, cs->perception.vislist[cs->enemyNum].real_visible_pos,
										 cs->combat.takeCoverPos ) ) {
				VectorCopy( cs->perception.vislist[cs->enemyNum].real_visible_pos, cs->navigation.combatGoalOrigin );
				return AIFunc_BattleAmbushStart( cs );
			}
			// couldn't find a spot, so just stay here?
			VectorCopy( cs->clientState->origin, cs->navigation.combatGoalOrigin );
			VectorCopy( cs->clientState->origin, cs->combat.takeCoverPos );
			return AIFunc_BattleAmbushStart( cs );
		}
	} else // assume we know where other entities are
	{
		VectorCopy( followent->s.pos.trBase, destorg );
		dist = Distance( cs->clientState->origin, destorg );
	}
	//
	// if the enemy is inside a CONTENTS_DONOTENTER brush, and we are close enough, stop chasing them
	if ( AICast_EntityVisible( cs, cs->enemyNum, qtrue ) && VectorDistance( cs->clientState->origin, destorg ) < 384 ) {
		if ( trap_PointContents( destorg, cs->enemyNum ) & ( CONTENTS_DONOTENTER | CONTENTS_DONOTENTER_LARGE ) ) {
			// just stay here, and hope they move out of the brush without finding a spot where they can hit us but we can't hit them
			return NULL;
		}
	}
	//
	// is there someone else we can go for instead?
	numEnemies = AICast_ScanForEnemies( cs, enemies );
	if ( numEnemies == -1 ) { // query mode
		return NULL;
	} else if ( numEnemies == -2 ) { // inspection may be required
		char *retval;
		// TTimo: gcc: suggest () around assignment used as truth value
		if ( ( retval = AIFunc_InspectFriendlyStart( cs, enemies[0] ) ) ) {
			return retval;
		}
	}
	AICast_ChooseWeapon( cs, qtrue ); // enable special weapons, if we cant get them, change back
	if ( numEnemies > 0 ) {
		int i;
		for ( i = 0; i < numEnemies; i++ ) {
			if ( enemies[i] != cs->enemyNum && AICast_CheckAttack( cs, enemies[i], qfalse ) ) {
				cs->enemyNum = enemies[i];
				return AIFunc_BattleStart( cs );
			}
		}
	}
	AICast_ChooseWeapon( cs, qfalse );

	//
	// if we only recently saw them, face them
	//
	/* RF: disabled 9/19/01, characters like boss2 supersoldier are forced to walk backwards and look wierd
	if (cs->perception.vislist[cs->enemyNum].visible_timestamp > level.time - 3000) {
		AICast_AimAtEnemy( cs );	// be ready for an attack if they become visible again
		//if (cs->attributes[ATTACK_CROUCH] > 0.1) {	// crouching for combat
		//	cs->attackcrouch_time = level.time + 1000;
		//}
	}
	*/

	//
	// Lob a Grenade?
	// if we haven't thrown a grenade in a bit, go into "grenade flush mode"
	if ( ( ai_encounter.lastGrenadeFlushTime > level.time || ai_encounter.lastGrenadeFlushTime < level.time - 5000 ) &&
		 ( cs->aiState >= AISTATE_COMBAT ) && ( cs->script.castScriptStatus.castScriptEventIndex < 0 ) &&
		 ( cs->grenade.startGrenadeFlushTime < level.time - 3000 ) &&
		 ( COM_BitCheck( cs->clientState->cur_ps.weapons, WP_GRENADE_LAUNCHER ) ) &&
		 ( AICast_GotEnoughAmmoForWeapon( cs, WP_GRENADE_LAUNCHER ) ) &&
		 ( cs->combat.weaponFireTimes[WP_GRENADE_LAUNCHER] < level.time - (int)( 1000 + aicast_skillscale * 1000 ) ) &&
		 ( ( cs->weaponNum == WP_GRENADE_LAUNCHER ) ||
		   !( cs->script.castScriptStatus.scriptFlags & SFL_NOCHANGEWEAPON ) ) &&
		 ( Distance( cs->clientState->origin, cs->perception.vislist[cs->enemyNum].visible_pos ) > 100 ) &&
		 ( Distance( cs->clientState->origin, cs->perception.vislist[cs->enemyNum].visible_pos ) < 1400 ) ) {
		// try and flush them out with a grenade
		//G_Printf("pineapple?\n");
		return AIFunc_GrenadeFlushStart( cs );
	} else if ( ( ai_encounter.lastGrenadeFlushTime > level.time ||
				  ai_encounter.lastGrenadeFlushTime < level.time - 5000 ) &&
				( cs->aiState >= AISTATE_COMBAT ) && ( cs->script.castScriptStatus.castScriptEventIndex < 0 ) &&
				( cs->grenade.startGrenadeFlushTime < level.time - 3000 ) &&
				( COM_BitCheck( cs->clientState->cur_ps.weapons, WP_GRENADE_PINEAPPLE ) ) &&
				( AICast_GotEnoughAmmoForWeapon( cs, WP_GRENADE_PINEAPPLE ) ) &&
				( cs->combat.weaponFireTimes[WP_GRENADE_PINEAPPLE] <
				  level.time - (int)( 1000 + aicast_skillscale * 1000 ) ) &&
				( ( cs->weaponNum == WP_GRENADE_PINEAPPLE ) ||
				  !( cs->script.castScriptStatus.scriptFlags & SFL_NOCHANGEWEAPON ) ) &&
				( Distance( cs->clientState->origin, cs->perception.vislist[cs->enemyNum].visible_pos ) > 100 ) &&
				( Distance( cs->clientState->origin, cs->perception.vislist[cs->enemyNum].visible_pos ) < 1400 ) ) {
		// try and flush them out with a grenade
		//G_Printf("pineapple?\n");
		return AIFunc_GrenadeFlushStart( cs );
	}
	//
	// Flaming Zombie? Shoot flames while running
	if ( ( cs->aiCharacter == AICHAR_ZOMBIE ) && ( IS_FLAMING_ZOMBIE( ent->s ) ) &&
		 ( fabs( cs->ideal_viewangles[YAW] - cs->viewangles[YAW] ) < 5 ) ) {
		if ( fabs( sin( ( level.time + cs->entityNum * 314 ) / 1000 ) *
				   cos( ( level.time + cs->entityNum * 267 ) / 979 ) ) < 0.5 ) {
			ent->s.time = level.time + 800;
		}
	}
	// reload?
	AICast_IdleReload( cs );

	if ( dist < chaseDist ) {
		return NULL;
	}

	//
	// go to them
	//
	// ...........................................................
	// Do the movement..
	//
	// move straight to them if we can
	if ( !moved && cs->leaderNum < 0 &&
		 ( cs->clientState->cur_ps.groundEntityNum != ENTITYNUM_NONE || g_entities[cs->entityNum].waterlevel > 1 ) &&
		 AICast_EntityVisible( cs, cs->enemyNum, qtrue ) ) {
		aicast_predictmove_t move;
		vec3_t dir;
		bot_input_t bi;
		usercmd_t ucmd;
		trace_t tr;

		// trace will eliminate most unsuccessful paths
		trap_Trace( &tr, cs->clientState->origin, NULL, NULL, followent->r.currentOrigin, cs->entityNum,
					g_entities[cs->entityNum].clipmask );
		if ( tr.entityNum == followent->s.number ) {
			// try walking straight to them
			VectorSubtract( followent->r.currentOrigin, cs->clientState->origin, dir );
			VectorNormalize( dir );
			if ( !ent->waterlevel ) {
				dir[2] = 0;
			}
			//trap_EA_Move(cs->entityNum, dir, 400);
			trap_EA_GetInput( cs->entityNum, (float)level.time / 1000, &bi );
			VectorCopy( dir, bi.dir );
			bi.speed = 400;
			bi.actionflags = 0;
			AICast_InputToUserCommand( cs, &bi, &ucmd, bs->cur_ps.delta_angles );
			AICast_PredictMovement( cs, 5, 2.0, &move, &ucmd, cs->enemyNum );

			if ( move.stopevent == PREDICTSTOP_HITENT ) { // success!
				trap_EA_Move( cs->entityNum, dir, 400 );
				// RF, if we are really close, we might be stuck on a corner, so randomly move sideways
				if ( ( VectorLength( followent->client->ps.velocity ) < 50 ) &&
					 ( dist <
					   10 + ( sqrt( cs->clientState->cur_ps.maxs[0] * cs->clientState->cur_ps.maxs[0] * 8.0 ) / 2.0 +
							  sqrt( followent->client->ps.maxs[0] * followent->client->ps.maxs[0] * 8.0 ) / 2.0 ) ) ) {
					// if the box trace is unsuccessful
					trap_Trace( &tr, cs->clientState->origin, cs->clientState->cur_ps.mins,
								cs->clientState->cur_ps.maxs, followent->r.currentOrigin, cs->entityNum,
								g_entities[cs->entityNum].clipmask );
					if ( tr.entityNum != followent->s.number ) {
						if ( level.time % 6000 < 2000 ) {
							trap_EA_MoveRight( cs->entityNum );
						} else {
							trap_EA_MoveLeft( cs->entityNum );
						}
					}
				}
				vectoangles( dir, cs->ideal_viewangles );
				cs->ideal_viewangles[2] *= 0.5;
				moved = qtrue;
			} else { // clear movement
				//trap_EA_Move(cs->entityNum, dir, 0);
			}
		}
	}
	//
	// if they are visible, but not attackable, look for a spot where we can attack them, and head
	// for there. This should prevent AI's getting stuck in a bunch.
	if ( !moved && cs->weaponNum >= WP_LUGER && cs->weaponNum <= WP_AKIMBO && cs->attributes[TACTICAL] >= 0.1 ) {
		//
		// check for another movement we should be making
		if ( cs->navigation.obstructingTime > level.time ) {
			AICast_MoveToPos( cs, cs->navigation.obstructingPos, -1 );
			moved = qtrue;
		}
		//
		if ( cs->leaderNum >= 0 ) {
			if ( cs->navigation.combatGoalTime < level.time ) {
				if ( cs->combat.attackSpotTime < level.time ) {
					cs->combat.attackSpotTime = level.time + 500 + rand() % 500;
					if ( trap_AAS_FindAttackSpotWithinRange( cs->entityNum, cs->leaderNum, cs->enemyNum,
															 MAX_LEADER_DIST, AICAST_TFL_DEFAULT,
															 cs->navigation.combatGoalOrigin ) ) {
						cs->navigation.combatGoalTime = level.time + 2000;
					}
				}
			}
			if ( cs->navigation.combatGoalTime > level.time ) {
				if ( Distance( cs->navigation.combatGoalOrigin, g_entities[cs->leaderNum].r.currentOrigin ) >
					 MAX_LEADER_DIST ) {
					// go find a new combatSpot
					cs->navigation.combatGoalTime = 0;
				} else {
					// go to the combat spot
					moveresult = AICast_MoveToPos( cs, cs->navigation.combatGoalOrigin, -1 );
					if ( moveresult && moveresult->failure ) { // no path, so go back to idle behaviour
						cs->navigation.combatGoalTime = 0;
					} else {
						moved = qtrue;
						if ( Distance( cs->clientState->origin, cs->navigation.combatGoalOrigin ) < 32 ) {
							cs->navigation.combatGoalTime = 0;
						}
					}
				}
			} else {
				// we can't find a way to get to our enemy, so go back to our leader if outside range
				// do we need to go to our leader?
				if ( Distance( cs->clientState->origin, g_entities[cs->leaderNum].r.currentOrigin ) >
					 MAX_LEADER_DIST ) {
					return AIFunc_ChaseGoalStart( cs, cs->leaderNum, AICAST_LEADERDIST_MAX, qtrue );
				}
			}
		} else {
			if ( cs->navigation.combatGoalTime < level.time ) {
				if ( cs->combat.attackSpotTime < level.time ) {
					cs->combat.attackSpotTime = level.time + 500 + rand() % 500;
					if ( trap_AAS_FindAttackSpotWithinRange( cs->entityNum, cs->entityNum, cs->enemyNum, 512,
															 AICAST_TFL_DEFAULT, cs->navigation.combatGoalOrigin ) ) {
						cs->navigation.combatGoalTime = level.time + 2000;
					}
				}
			}
			if ( cs->navigation.combatGoalTime > level.time ) {
				// go to the combat spot
				moveresult = AICast_MoveToPos( cs, cs->navigation.combatGoalOrigin, -1 );
				if ( moveresult && moveresult->failure ) { // no path, so go back to idle behaviour
					cs->navigation.combatGoalTime = 0;
				} else {
					moved = qtrue;
					if ( Distance( cs->clientState->origin, cs->navigation.combatGoalOrigin ) < 32 ) {
						cs->navigation.combatGoalTime = 0;
						cs->combat.attackSpotTime =
							level.time +
							12000; // dont go to another combatspot for some time, prevent repetitive behaviour
					}
				}
			}
		}
	}
	// just go to them
	if ( !moved && cs->leaderNum < 0 ) {
		moveresult = AICast_MoveToPos( cs, destorg, cs->enemyNum );
		if ( moveresult && moveresult->failure ) { // no path, so try and hude from them
			// pausetime has expired, so go into ambush mode
			if ( AICast_GetTakeCoverPos( cs, cs->enemyNum, cs->perception.vislist[cs->enemyNum].real_visible_pos,
										 cs->combat.takeCoverPos ) ) {
				// wait in ambush, for them to return
				VectorCopy( cs->clientState->origin, cs->navigation.combatGoalOrigin );
				return AIFunc_BattleAmbushStart( cs );
			}
			// HACK, help lopers get out of bad spots
			if ( cs->aiCharacter == AICHAR_LOPER ) {
				cs->combat.weaponFireTimes[WP_MONSTER_ATTACK2] = 0;
			}
			// couldn't find a spot, so just stay here?
			if ( cs->clientState->areanum ) { // if our area is valid
				VectorCopy( cs->clientState->origin, cs->navigation.combatGoalOrigin );
				VectorCopy( cs->clientState->origin, cs->combat.takeCoverPos );
				return AIFunc_BattleAmbushStart( cs );
			}
		} else {
			moved = qtrue;
		}
	}
	//
	// slow down real close to the goal, so we don't go passed it
	cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, chaseDist );
//
// ...........................................................
// speed up over some time
#define BATTLE_CHASE_ACCEL_TIME 300
	if ( ( cs->attributes[RUNNING_SPEED] > 170 ) && ( cs->weaponNum != WP_GAUNTLET ) &&
		 ( level.time < ( cs->combat.startBattleChaseTime + BATTLE_CHASE_ACCEL_TIME ) ) ) {
		float ideal;

		ideal = 0.5 +
				0.5 * ( 1.0 - ( (float)( ( cs->combat.startBattleChaseTime + BATTLE_CHASE_ACCEL_TIME ) - level.time ) /
								BATTLE_CHASE_ACCEL_TIME ) );
		if ( ideal < cs->speedScale ) {
			cs->speedScale = ideal;
		}
	}
	//
	// if we are going to reach them soon, predict the attack
	{
		float simTime = 1.5;
		aicast_predictmove_t move;
		float moveDist;
		vec3_t vec;
		//
		if ( cs->weaponNum == WP_GAUNTLET ) {
			simTime = 0.5;
		}
		//
		AICast_PredictMovement( cs, 1, simTime, &move, &cs->lastucmd, cs->enemyNum );
		VectorSubtract( move.endpos, cs->clientState->origin, vec );
		moveDist = VectorNormalize( vec );
		//
		if ( cs->weaponNum == WP_GAUNTLET ) {
			if ( move.stopevent == PREDICTSTOP_HITENT ) {
				AICast_AimAtEnemy( cs );
				trap_EA_Attack( bs->client );
				cs->bFlags |= AI_FRAME_ATTACKED;
			}
		}
		//
		// do we went to play a diving animation into a cover position?
		else if ( ( ( cs->attributes[TACTICAL] > 0.85 ) && ( cs->aiFlags & AIFL_ROLL_ANIM ) && !client->ps.torsoTimer &&
					!client->ps.legsTimer && ( cs->lastRollMove < level.time - 800 ) && ( move.numtouch == 0 ) &&
					( moveDist > simTime * cs->attributes[RUNNING_SPEED] * 0.98 ) &&
					move.groundEntityNum == ENTITYNUM_WORLD ) &&
				  ( AICast_CheckAttackAtPos( cs->entityNum, cs->enemyNum, move.endpos,
											 cs->attackcrouch_time > level.time, qfalse ) ) ) {
			cs->combat.takeCoverTime = 0;
			return AIFunc_BattleRollStart( cs, vec );
		}
		//
		else if ( cs->aiFlags & AIFL_FLIP_ANIM && cs->lastRollMove < level.time - 800 && !client->ps.torsoTimer &&
				  cs->script.castScriptStatus.castScriptEventIndex < 0 && move.numtouch == 0 &&
				  moveDist > simTime * cs->attributes[RUNNING_SPEED] * 0.9 && move.groundEntityNum == ENTITYNUM_WORLD &&
				  cs->attackcrouch_time < level.time ) {
			int destarea, simarea, starttravel, simtravel;
			// if we'll be closer after the move, proceed
			destarea = AI_PointAreaNum( destorg );
			simarea = AI_PointAreaNum( move.endpos );
			starttravel = trap_AAS_AreaTravelTimeToGoalArea( cs->clientState->areanum, cs->clientState->origin,
															 destarea, cs->travelflags );
			simtravel = trap_AAS_AreaTravelTimeToGoalArea( simarea, move.endpos, destarea, cs->travelflags );
			if ( simtravel < starttravel ) {
				return AIFunc_FlipMoveStart( cs, vec );
			}
		}
		// slow down? so we don't go too far from behind the obstruction which is protecting us
		else if ( !( cs->aiFlags & AIFL_WALKFORWARD ) &&
				  ( VectorDistance( cs->clientState->origin, g_entities[cs->enemyNum].s.pos.trBase ) <
					AICast_WeaponRange( cs, cs->weaponNum ) ) &&
				  ( cs->navigation.obstructingTime < level.time ) && ( cs->attributes[TACTICAL] > 0.1 ) &&
				  ( AICast_VisibleFromPos( cs->perception.vislist[cs->enemyNum].visible_pos, cs->enemyNum, move.endpos,
										   cs->entityNum, qfalse ) ) ) {
			// start a crouch attack?
			//if (cs->attributes[ATTACK_CROUCH] > 0.1) {
			//	cs->attackcrouch_time = level.time + 3000;
			//else
			cs->attackcrouch_time = 0;
			if ( cs->clientState->cur_ps.viewheight > cs->clientState->cur_ps.crouchViewHeight &&
				 cs->attributes[RUNNING_SPEED] * cs->speedScale > 120 ) {
				cs->speedScale = 120.0 * cs->attributes[RUNNING_SPEED];
			}
			// also face them, ready for the attack
			if ( cs->attributes[RUNNING_SPEED] > 140 ) {
				AICast_AimAtEnemy( cs );
			}
			/* disabled, causes them to use up ammo in the clip before they get visible
			if ((cs->script.castScriptStatus.scriptNoAttackTime < level.time) && (cs->combat.noAttackTime < level.time)) {
				// if we are using a bullet weapon, start firing now
				switch (cs->weaponNum) {
				case WP_MP40:
				case WP_VENOM:
				case WP_THOMPSON:
				case WP_STEN:	//----(SA)	added
					trap_EA_Attack(cs->entityNum);
				}
			}
			*/
		}
	}

	// reload?
	AICast_IdleReload( cs );

	return NULL;
}

/*
============
AIFunc_BattleChaseStart()
============
*/
char *AI_EnterChase( cast_state_t *cs ) {
	cs->combat.startBattleChaseTime = level.time;
	cs->navigation.combatGoalTime = 0;
	cs->combat.battleChaseMarker = -99;
	cs->combat.battleChaseMarkerDir = 1;
	// don't wait too long before taking cover, if we just aborted one
	if ( cs->combat.takeCoverTime > level.time ) {
		cs->combat.takeCoverTime = level.time + 1500 + rand() % 500;
	}
	//
	// start a crouch attack?
	if ( cs->attributes[ATTACK_CROUCH] > 0.1 ) {
		cs->aiFlags |= AIFL_ATTACK_CROUCH;
	} else {
		cs->aiFlags &= ~AIFL_ATTACK_CROUCH;
	}
	//
	return "Chase";
}

char *AIFunc_BattleChaseStart( cast_state_t *cs ) {
	return AI_RequestState( cs, AIStateId::Chase, AITransitionReason::Decision );
}
