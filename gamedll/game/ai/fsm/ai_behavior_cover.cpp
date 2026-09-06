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
AIFunc_BattleAmbush()
=============
*/
char *AIFunc_BattleAmbush( cast_state_t *cs ) {
	bot_moveresult_t *moveresult;
	ai_client_state_t *bs;
	vec3_t destorg, vec;
	float dist, moveDist;
	int enemies[MAX_CLIENTS], numEnemies, i;
	qboolean shouldAttack, idleYaw;
	aicast_predictmove_t move;
	int *ammo;
	vec3_t dir;
	//	gclient_t	*client = &level.clients[cs->entityNum];
	//
	// do we need to avoid a danger?
	if ( cs->dangerEntityValidTime >= level.time ) {
		if ( AICast_GetTakeCoverPos( cs, cs->dangerEntity, cs->dangerEntityPos, cs->combat.takeCoverPos ) ) {
			// go to a position that cannot be seen from the dangerPos
			cs->combat.takeCoverTime = cs->dangerEntityValidTime + 1000;
			cs->attackcrouch_time = 0;
			return AIFunc_AvoidDangerStart( cs );
		}
		// if not found, then keep trying, hopefully a spot will free up soon so we can run the hidepos function
	}
	//
	// are we waiting for a door?
	if ( cs->navigation.doorMarkerTime > level.time - 100 ) {
		return AIFunc_DoorMarkerStart( cs, cs->navigation.doorMarkerDoor, cs->navigation.doorMarkerNum );
	}

	// we need to move towards it
	bs = cs->clientState;
	//
	// note: removing this will cause problems down below!
	if ( cs->enemyNum < 0 ) {
		return AIFunc_IdleStart( cs );
	}
	//
	// have we spent enough time in combat mode?
	if ( cs->aiState == AISTATE_COMBAT ) {
		if ( cs->perception.vislist[cs->enemyNum].visible_timestamp < level.time - COMBAT_TIMEOUT ) {
			AICast_StateChange( cs, AISTATE_ALERT );
		}
	}
	// while hunting, use crouch mode if possible
	if ( cs->attributes[ATTACK_CROUCH] >= 0.1 ) {
		cs->attackcrouch_time = level.time + 2000;
	}
	//
	VectorCopy( cs->combat.takeCoverPos, destorg );
	VectorSubtract( destorg, cs->clientState->origin, vec );
	vec[2] *= 0.2;
	dist = VectorLength( vec );
	//
	// update the chase marker
	if ( cs->perception.vislist[cs->enemyNum].chase_marker_count > 0 ) {
		VectorCopy( cs->perception.vislist[cs->enemyNum]
						.chase_marker[cs->perception.vislist[cs->enemyNum].chase_marker_count - 1],
					cs->navigation.combatGoalOrigin );
	}
	//
	// look for things we should attack
	// if we are out of ammo, we shouldn't bother trying to attack (and we should keep hiding)
	ammo = cs->clientState->cur_ps.ammo;
	shouldAttack = qfalse;
	numEnemies = AICast_ScanForEnemies( cs, enemies );

	// we shouldnt be interrupted from BattleAmbush mode, so try to handle these without interference
	if ( numEnemies == -1 ) { // query mode
		// ...
	} else if ( numEnemies == -2 ) { // inspection may be required
		cs->perception.vislist[enemies[0]].flags |= AIVIS_INSPECTED; // they have been notified
		cs->perception.vislist[enemies[0]].flags &= ~AIVIS_INSPECT; // they have been notified
	} else if ( numEnemies == -3 ) { // bullet impact
		// ...
	} else if ( numEnemies == -4 ) { // audible event
		// ...
	} else if ( numEnemies > 0 ) {

		if ( AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
			// default to the first known enemy, overwrite if we find a clearer shot
			cs->enemyNum = enemies[0];
			//
			for ( i = 0; i < numEnemies; i++ ) {
				// if (we can get them from here) or (they can get us, AND we have stopped going to our ambush spot)
				if ( ( AICast_EntityVisible( cs, enemies[i], qfalse ) &&
					   AICast_CheckAttack( cs, enemies[i], qfalse ) ) ||
					 ( ( VectorLength( cs->combat.takeCoverPos ) < 1 || dist <= 8 ) &&
					   ( AICast_EntityVisible( AICast_GetCastState( enemies[i] ), cs->entityNum, qfalse ) ||
						 AICast_CheckAttack( AICast_GetCastState( enemies[i] ), cs->entityNum, qfalse ) ||
						 AICast_EntityVisible( AICast_GetCastState( enemies[i] ), cs->entityNum, qtrue ) ) ) ) {
					cs->enemyNum = enemies[i];
					return AIFunc_BattleStart( cs );
				} else if ( cs->enemyNum < 0 ) {
					cs->lastEnemy = enemies[i];
				} else if ( AICast_EntityVisible( cs, enemies[i], qfalse ) ) {
					bot_input_t bi_back;
					// try and move to them, if successful, then start chasing
					trap_EA_GetInput( cs->entityNum, (float)level.time / 1000, &bi_back );
					moveresult = AICast_MoveToPos( cs, g_entities[enemies[i]].client->ps.origin, enemies[i] );
					if ( moveresult ) {
						if ( !moveresult->failure ) {
							cs->enemyNum = enemies[i];
							return AIFunc_BattleChaseStart( cs );
						}
					} else {
						trap_EA_ResetInput( cs->entityNum, &bi_back );
					}
				}
			}
		} else {
			AICast_ChooseWeapon( cs, qfalse );
			//
			if ( !AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
				// NO AMMO LEFT!!
				// hide?
				if ( AICast_GetTakeCoverPos( cs, cs->enemyNum, cs->perception.vislist[cs->enemyNum].visible_pos,
											 cs->combat.takeCoverPos ) ) {
					// go to a position that cannot be seen from the last place we saw the enemy, and wait there for some time
					cs->combat.takeCoverTime = level.time + 2000 + rand() % 3000;
					return AIFunc_BattleTakeCoverStart( cs );
				}
			}
		}
	}
	//
	// keep hiding forever
	cs->combat.takeCoverTime = level.time + 1000;
	//
	memset( &move, 0, sizeof( move ) );
	//
	// are we close enough to the goal?
	if ( VectorLength( cs->combat.takeCoverPos ) > 1 && dist > 8 &&
		 ( cs->navigation.obstructingTime < level.time ) /*&& !shouldAttack*/ ) {
		const float simTime = 0.8;
		float enemyDist;
		//
		// we haven't reached it yet, make sure we at least wait there for a few seconds after arriving
		cs->combat.takeCoverTime = level.time + 2000 + rand() % 2000;
		//
		moveresult = AICast_MoveToPos( cs, destorg, -1 );
		if ( moveresult ) {
			//if the movement failed
			if ( moveresult->failure ) {
				//reset the avoid reach, otherwise bot is stuck in current area
				trap_BotResetAvoidReach( bs->ms );
				// couldn't get there, so stop trying to get there
				VectorClear( cs->combat.takeCoverPos );
				dist = 0;
			}
			//
			if ( moveresult->blocked ) {
				// abort the TakeCover
				VectorClear( cs->combat.takeCoverPos );
				dist = 0;
			}
		}
		//
		// NOTE: this is also used by hidepos prediction (below)
		// if we are going to bump into something soon, abort it
		AICast_PredictMovement( cs, 1, simTime, &move, &cs->lastucmd, -1 );
		enemyDist = Distance( cs->clientState->origin, g_entities[cs->enemyNum].s.pos.trBase );
		VectorSubtract( move.endpos, cs->clientState->origin, vec );
		moveDist = VectorNormalize( vec );
		//
		if ( ( move.numtouch && move.touchents[0] < aicast_maxclients ) // hit something
			 // or moved closer to the enemy
			 || ( ( enemyDist < 128 ) &&
				  ( ( enemyDist - 1 ) > ( Distance( move.endpos, g_entities[cs->enemyNum].s.pos.trBase ) ) ) ) ) {
			// abort the manouver
			VectorClear( cs->combat.takeCoverPos );
			dist = 0;
		}
		//
		// we should slow down on approaching the destination point
		else if ( dist < 64 ) {
			cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, 0 );
		}

		// don't actually hide, check if we are about to, so we can hide right here
		if ( !( cs->aiFlags & AIFL_MISCFLAG1 ) ) {
			if ( move.numtouch || !AICast_VisibleFromPos( move.endpos, cs->entityNum, cs->navigation.combatGoalOrigin,
														  cs->enemyNum, qfalse ) ) {
				// abort the manouver, reached a good spot
				cs->aiFlags |= AIFL_MISCFLAG1;
				VectorCopy( cs->clientState->origin, cs->combat.takeCoverPos );
			}
		}

	} else {
		//
		// check for a movement we should be making
		if ( cs->navigation.obstructingTime > level.time ) {
			AICast_MoveToPos( cs, cs->navigation.obstructingPos, -1 );
		}
		// if we have some enemies that we can attack immediately (without going anywhere to chase them)
		if ( shouldAttack ) {
			return AIFunc_BattleStart( cs );
		}
		// do we need to go to our leader?
		else if ( cs->leaderNum >= 0 &&
				  Distance( cs->clientState->origin, g_entities[cs->leaderNum].r.currentOrigin ) > MAX_LEADER_DIST ) {
			// wait until we've been hiding for long enough
			if ( level.time > cs->combat.takeCoverTime ) {
				return AIFunc_ChaseGoalStart( cs, cs->leaderNum, AICAST_LEADERDIST_MAX, qtrue );
			}
		}
		// else, crouch while we hide
		if ( cs->attributes[ATTACK_CROUCH] > 0.1 || cs->crouchHideFlag ) {
			cs->attackcrouch_time = level.time + 2000;
		}
	}
	//
	if ( !( cs->aiFlags & AIFL_WALKFORWARD ) || !VectorLength( cs->clientState->cur_ps.velocity ) ) {
		idleYaw = qtrue;
		// if we know who we are hiding from, face them (hack, so they dont face stupid directions)
		if ( cs->enemyNum >= 0 ) {
			VectorSubtract( g_entities[cs->enemyNum].s.pos.trBase, cs->clientState->origin, dir );
			vectoangles( dir, cs->ideal_viewangles );
			idleYaw = qfalse;
		} else if ( cs->lastEnemy >= 0 ) {
			VectorSubtract( g_entities[cs->lastEnemy].s.pos.trBase, cs->clientState->origin, dir );
			vectoangles( dir, cs->ideal_viewangles );
			idleYaw = qfalse;
		}
		// if we can see the place we are hiding from, look at it
		if ( idleYaw && AICast_VisibleFromPos( cs->clientState->origin, cs->entityNum, cs->navigation.combatGoalOrigin,
											   cs->lastEnemy, qfalse ) ) {
			// face the position we are retreating from
			VectorSubtract( cs->navigation.combatGoalOrigin, cs->clientState->origin, dir );
			dir[2] = 0;
			if ( VectorNormalize( dir ) > 4 ) {
				idleYaw = qfalse;
				vectoangles( dir, cs->ideal_viewangles );
			}
		}
		//
		if ( idleYaw ) { // look around randomly (but not straight into walls)

			if ( cs->nextIdleAngleChange < level.time ) {
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
	}
	//
	if ( !cs->crouchHideFlag && cs->enemyNum < 0 ) { // no enemy, and no need to crouch, so stop crouching
		if ( cs->attackcrouch_time > level.time + 1000 ) {
			cs->attackcrouch_time = level.time + 1000;
		}
	}

	// reload?
	AICast_IdleReload( cs );

	return NULL;
}

/*
=============
AIFunc_BattleAmbushStart()
=============
*/
char *AIFunc_BattleAmbushStart( cast_state_t *cs ) {
	if ( !AICast_CanMoveWhileFiringWeapon( cs->weaponNum ) ) {
		// always run to the cover point
		cs->attackcrouch_time = 0;
	} else if ( cs->attackcrouch_time > level.time + 1000 ) {
		cs->attackcrouch_time = level.time + 1000;
	}

	//
	// start a crouch attack?
	if ( cs->attributes[ATTACK_CROUCH] > 0.1 && cs->attackcrouch_time >= level.time ) {
		// continue
		cs->attackcrouch_time = level.time + 1000;
	}
	// if we arent crouching, start crouching soon after we start retreating
	if ( cs->attributes[ATTACK_CROUCH] > 0.1 ) {
		cs->aiFlags |= AIFL_ATTACK_CROUCH;
	} else {
		cs->aiFlags &= ~AIFL_ATTACK_CROUCH;
	}

	// miscflag1 used to set predicted point as our goal, so we dont keep setting this over and over
	cs->aiFlags &= ~AIFL_MISCFLAG1;

	AI_SetLegacyState( cs, AIFunc_BattleAmbush );
	return "AIFunc_BattleAmbush";
}

/*
============
AIFunc_BattleTakeCover()
============
*/
char *AIFunc_BattleTakeCover( cast_state_t *cs ) {
	bot_moveresult_t *moveresult;
	ai_client_state_t *bs;
	vec3_t destorg, vec;
	float dist, moveDist;
	int enemies[MAX_CLIENTS], numEnemies, i;
	qboolean shouldAttack;
	aicast_predictmove_t move;
	int *ammo;
	gclient_t *client = &level.clients[cs->entityNum];
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

	// we need to move towards it
	bs = cs->clientState;
	//
	// note: removing this will cause problems down below!
	if ( cs->enemyNum < 0 ) {
		return AIFunc_IdleStart( cs );
	}
	//
	if ( VectorLength( cs->combat.takeCoverPos ) < 1 ) {
		dist = 0;
	} else {
		VectorCopy( cs->combat.takeCoverPos, destorg );
		VectorSubtract( destorg, cs->clientState->origin, vec );
		vec[2] *= 0.2;
		dist = VectorLength( vec );
	}
	//
	// look for things we should attack
	// if we are out of ammo, we shouldn't bother trying to attack (and we should keep hiding)
	ammo = cs->clientState->cur_ps.ammo;
	shouldAttack = qfalse;
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

		if ( AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
			// default to the first known enemy, overwrite if we find a clearer shot
			cs->enemyNum = enemies[0];
			//
			for ( i = 0; i < numEnemies; i++ ) {
				if ( AICast_CheckAttack( cs, enemies[i], qfalse ) ||
					 AICast_CheckAttack( AICast_GetCastState( enemies[i] ), cs->entityNum, qfalse ) ||
					 AICast_EntityVisible( AICast_GetCastState( enemies[i] ), cs->entityNum, qtrue ) ) {
					if ( ( cs->aiFlags & AIFL_WALKFORWARD ) || ( dist <= 12 ) ) {
						// we are at our hidepos, abort!
						cs->enemyNum = enemies[i];
						return AIFunc_BattleStart( cs );
					} else {
						shouldAttack = qtrue; // fire at them as we go
					}
				} else if ( cs->enemyNum < 0 ) {
					cs->lastEnemy = enemies[i];
				}
			}

		} else {
			AICast_ChooseWeapon( cs, qfalse );
			//
			if ( dist <= 12 ) {
				if ( !AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
					// NO AMMO LEFT!!
					// hide?
					if ( AICast_GetTakeCoverPos( cs, cs->enemyNum, cs->perception.vislist[cs->enemyNum].visible_pos,
												 cs->combat.takeCoverPos ) ) {
						// go to a position that cannot be seen from the last place we saw the enemy, and wait there for some time
						cs->combat.takeCoverTime = level.time + 2000 + rand() % 3000;
						//return AIFunc_BattleTakeCoverStart( cs );
					}
				}
			}
		}
	}
	//
	//if (!shouldAttack)
	// always do this check, if our destination sucks, abort it
	{
		// if the enemy can see our hide position, find a better spot
		if ( AICast_VisibleFromPos( cs->perception.vislist[cs->enemyNum].visible_pos, cs->enemyNum,
									cs->combat.takeCoverPos, bs->entitynum, qfalse ) ) {
			if ( !AICast_GetTakeCoverPos( cs, cs->enemyNum, cs->perception.vislist[cs->enemyNum].visible_pos,
										  cs->combat.takeCoverPos ) ) {
				// shit!! umm.. try and fire?
				return AIFunc_BattleStart( cs );
			} else { // recalc distance
				VectorCopy( cs->combat.takeCoverPos, destorg );
				VectorSubtract( destorg, cs->clientState->origin, vec );
				vec[2] *= 0.2;
				dist = VectorLength( vec );
			}
		} else if ( dist < 8 ) {
			// if they can see us, find a better spot
			if ( AICast_EntityVisible( AICast_GetCastState( cs->enemyNum ), cs->entityNum, qtrue ) ||
				 AICast_CheckAttack( AICast_GetCastState( cs->enemyNum ), cs->entityNum, qfalse ) ) {
				if ( !AICast_GetTakeCoverPos( cs, cs->enemyNum, cs->perception.vislist[cs->enemyNum].visible_pos,
											  cs->combat.takeCoverPos ) ) {
					// shit!! umm.. try and fire?
					return AIFunc_BattleStart( cs );
				} else { // recalc distance
					VectorCopy( cs->combat.takeCoverPos, destorg );
					VectorSubtract( destorg, cs->clientState->origin, vec );
					vec[2] *= 0.2;
					dist = VectorLength( vec );
				}
			}
		}
		//cs->combat.takeCoverTime = level.time + 1000;
	}
	//
	// pretend we can still see them while we run to our hide pos, this way they are less likely
	// to forget about their enemy once they get there
	// DISABLED: doesn't work well with new AI system
	//if (cs->enemyNum >= 0 && cs->perception.vislist[cs->enemyNum].real_visible_timestamp && (cs->perception.vislist[cs->enemyNum].real_visible_timestamp > level.time - 2000)) {
	//	AICast_UpdateVisibility( &g_entities[cs->entityNum], &g_entities[cs->enemyNum], qfalse, cs->perception.vislist[cs->enemyNum].real_visible_timestamp == cs->perception.vislist[cs->enemyNum].lastcheck_timestamp );
	//}
	//
	memset( &move, 0, sizeof( move ) );
	//
	// are we close enough to the goal?
	if ( VectorLength( cs->combat.takeCoverPos ) > 1 && dist > 8 ) {
		const float simTime = 1.5;
		float enemyDist;
		//
		// we haven't reached it yet, make sure we at least wait there for a few seconds after arriving
		cs->combat.takeCoverTime = level.time + 2000 + rand() % 2000;
		//
		moveresult = AICast_MoveToPos( cs, destorg, -1 );
		if ( moveresult ) {
			//if the movement failed
			if ( moveresult->failure ) {
				//reset the avoid reach, otherwise bot is stuck in current area
				trap_BotResetAvoidReach( bs->ms );
				// couldn't get there, so stop trying to get there
				VectorClear( cs->combat.takeCoverPos );
				dist = 0;
			}
			//
			if ( moveresult->blocked ) {
				// abort the TakeCover
				VectorClear( cs->combat.takeCoverPos );
				dist = 0;
			}
		}
		//
		// if we are going to bump into something soon, abort it
		AICast_PredictMovement( cs, 1, simTime, &move, &cs->lastucmd, -1 );
		enemyDist = Distance( cs->clientState->origin, g_entities[cs->enemyNum].s.pos.trBase );
		VectorSubtract( move.endpos, cs->clientState->origin, vec );
		moveDist = VectorNormalize( vec );
		//
		if ( ( move.numtouch && move.touchents[0] < aicast_maxclients ) // hit something
			 // or moved closer to the enemy
			 || ( ( enemyDist < 128 ) &&
				  ( ( enemyDist - 1 ) > ( Distance( move.endpos, g_entities[cs->enemyNum].s.pos.trBase ) ) ) ) ) {
			// abort the manouver
			VectorClear( cs->combat.takeCoverPos );
			dist = 0;
		}
		//
		// do we want to play a rolling animation into a cover position?
		else if ( ( cs->aiFlags & AIFL_DIVE_ANIM && !client->ps.torsoTimer &&
					cs->script.castScriptStatus.castScriptEventIndex < 0 && cs->lastRollMove < level.time - 800 &&
					move.numtouch == 0 && ( moveDist > simTime * cs->attributes[RUNNING_SPEED] * 0.98 ) &&
					move.groundEntityNum == ENTITYNUM_WORLD ) &&
				  ( shouldAttack && !AICast_VisibleFromPos( g_entities[cs->enemyNum].s.pos.trBase, cs->enemyNum,
															move.endpos, cs->entityNum, qfalse ) ) ) {
			VectorClear( cs->combat.takeCoverPos ); // stay there when done rolling
			return AIFunc_BattleDiveStart( cs, vec );
		}
		//
		// we should slow down on approaching the destination point
		else if ( dist < 64 ) {
			cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, 0 );
		}
		//
		// if they cant see us, then stay here
		if ( !( cs->aiFlags & AIFL_MISCFLAG1 ) &&
			 !AICast_VisibleFromPos( cs->perception.vislist[cs->enemyNum].real_visible_pos, cs->enemyNum, move.endpos,
									 cs->entityNum, qfalse ) &&
			 !AICast_VisibleFromPos( cs->perception.vislist[cs->enemyNum].real_visible_pos, cs->enemyNum,
									 cs->clientState->origin, cs->entityNum, qfalse ) &&
			 trap_AAS_PointAreaNum( move.endpos ) ) { // make sure the endpos is in a valid area
			VectorCopy( move.endpos, cs->combat.takeCoverPos );
			dist = 0;
			cs->aiFlags |= AIFL_MISCFLAG1; // dont do this again
		}
		//
		if ( cs->aiFlags & AIFL_FLIP_ANIM && cs->lastRollMove < level.time - 800 && !client->ps.torsoTimer &&
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
		// set crouching status
		//if (dist && (cs->thinkFuncChangeTime < level.time - 2000) && (cs->crouchHideFlag || cs->aiFlags & AIFL_ATTACK_CROUCH)) {
		if ( cs->crouchHideFlag ||
			 ( ( cs->thinkFuncChangeTime < level.time - 2000 ) && ( cs->aiFlags & AIFL_ATTACK_CROUCH ) ) ) {
			cs->attackcrouch_time = level.time + 1000;
		}

	} else {
		//
		// have we been Taking Cover for enough time?
		if ( level.time > cs->combat.takeCoverTime ) {
			return AIFunc_DefaultStart( cs );
		}
		//
		// check for a movement we should be making
		if ( cs->navigation.obstructingTime > level.time ) {
			VectorClear( cs->combat.takeCoverPos );
			AICast_MoveToPos( cs, cs->navigation.obstructingPos, -1 );
		}
		// if we have some enemies that we can attack immediately (without going anywhere to chase them)
		if ( shouldAttack ) {
			return AIFunc_BattleStart( cs );
		}
		// if we have some enemies in sight, but they can't attack us, flee if possible, otherwise if we are not afraid, go attack them
		else if ( numEnemies ) {

			// are they reloading? if so we should attack!
			if ( g_entities[cs->entityNum].client->ps.weaponDelay < 100 &&
				 g_entities[cs->enemyNum].client->ps.weaponDelay > 1100 ) {
				if ( AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) && AICast_WeaponUsable( cs, cs->weaponNum ) ) {
					return AIFunc_BattleStart( cs );
				}
			}

			// we can't hit them and they cant hit us, so dont bother doing anything

			//if (!AICast_GetTakeCoverPos( cs, cs->enemyNum, cs->perception.vislist[cs->enemyNum].visible_pos, cs->combat.takeCoverPos )) {
			//if (!AICast_WantsToTakeCover(cs, qfalse))
			//return AIFunc_BattleStart( cs );
			//}
		}
		// do we need to go to our leader?
		else if ( cs->leaderNum >= 0 &&
				  Distance( cs->clientState->origin, g_entities[cs->leaderNum].r.currentOrigin ) > MAX_LEADER_DIST ) {
			// wait until we've been hiding for long enough
			if ( level.time > cs->combat.takeCoverTime ) {
				return AIFunc_ChaseGoalStart( cs, cs->leaderNum, AICAST_LEADERDIST_MAX, qtrue );
			}
		}

		// else, crouch while we hide
		if ( cs->attributes[ATTACK_CROUCH] > 0.1 || cs->crouchHideFlag ) {
			cs->attackcrouch_time = level.time + 1000;
		}
	}
	//
	// if we should be attacking something on our way
	if ( shouldAttack ) {
		vec3_t vec, dir;
		float dist;
		//
		// if they are close, and we're heading for them, we should abort this manouver
		VectorSubtract( g_entities[cs->enemyNum].client->ps.origin, bs->origin, vec );
		if ( ( dist = VectorNormalize( vec ) ) < 256 ) {
			VectorCopy( bs->velocity, vec );
			vec[2] = 0;
			if ( VectorNormalize2( vec, dir ) > 20 ) { // we are moving
				if ( DotProduct( dir, vec ) > 0.4 ) {
					// abort
					return AIFunc_BattleStart( cs );
				}
			}
		}
		//
		// if the enemy can see our hide position, abort the manouver
		if ( ( cs->thinkFuncChangeTime < level.time - 1000 ) &&
			 ( AICast_VisibleFromPos( g_entities[cs->enemyNum].client->ps.origin, cs->enemyNum, cs->combat.takeCoverPos,
									  bs->entitynum, qfalse ) ) ) {
			// abort
			return AIFunc_BattleStart( cs );
		}
		//
		// if we are tactical and can crouch, do so
		if ( !move.numtouch && ( cs->thinkFuncChangeTime < level.time - 2000 ) && ( dist > 128 ) &&
			 cs->attributes[TACTICAL] > 0.4 && cs->attributes[ATTACK_CROUCH] > 0.1 &&
			 ( cs->attackcrouch_time >= level.time ) ) {
			cs->attackcrouch_time = level.time + 1000;
		}
		//
		//attack the enemy if possible
		AICast_ProcessAttack( cs );
		//
	} else /*if (dist < 48)*/ {
		// if we've recently been in a fight, look towards the enemy
		if ( cs->enemyNum >= 0 ) {
			AICast_AimAtEnemy( cs );
		} else if ( cs->lastEnemy >= 0 ) {
			// if we are not moving, face them
			if ( VectorLength( cs->clientState->cur_ps.velocity ) < 50 ) {
				vec3_t dir;
				//
				VectorSubtract( cs->perception.vislist[cs->lastEnemy].visible_pos, cs->clientState->origin, dir );
				VectorNormalize( dir );
				vectoangles( dir, cs->ideal_viewangles );
			}
		} else if ( !cs->crouchHideFlag ) { // no enemy, and no need to crouch, so stop crouching
			//if (cs->attackcrouch_time > level.time + 1000) {
			//	cs->attackcrouch_time = level.time + 1000;
			//}
		}
		// reload?
		AICast_IdleReload( cs );
	}

	return NULL;
}

/*
============
AIFunc_BattleTakeCoverStart()
============
*/
char *AIFunc_BattleTakeCoverStart( cast_state_t *cs ) {
// debugging
#ifdef DEBUG
//	if (cs->attributes[AGGRESSION] >= 1.0)
//		AICast_Printf( 0, "AI taking cover with full aggression!\n" );
#endif

	if ( !AICast_CanMoveWhileFiringWeapon( cs->weaponNum ) ) {
		// always run to the cover point
		cs->attackcrouch_time = 0;
		cs->aiFlags &= ~AIFL_ATTACK_CROUCH;
	} else {
		// if we arent crouching, start crouching soon after we start retreating
		if ( cs->attributes[ATTACK_CROUCH] > 0.1 ) {
			cs->aiFlags |= AIFL_ATTACK_CROUCH;
		} else {
			cs->aiFlags &= ~AIFL_ATTACK_CROUCH;
		}
		cs->attackcrouch_time = 0;
	}

	// miscflag1 used to set predicted point as our goal, so we dont keep setting this over and over
	cs->aiFlags &= ~AIFL_MISCFLAG1;

	AI_SetLegacyState( cs, AIFunc_BattleTakeCover );
	return "AIFunc_BattleTakeCover";
}
