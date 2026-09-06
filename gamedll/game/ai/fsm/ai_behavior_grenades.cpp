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
AIFunc_AvoidDanger()
============
*/
char *AIFunc_AvoidDanger( cast_state_t *cs ) {
	bot_moveresult_t *moveresult;
	ai_client_state_t *bs;
	vec3_t destorg, vec;
	float dist;
	int enemies[MAX_CLIENTS], numEnemies, i;
	qboolean shouldAttack;
	gentity_t *ent;
	trace_t tr;
	vec3_t end;
	gentity_t *danger;

	// we need to move towards it
	bs = cs->clientState;
	ent = g_entities + cs->entityNum;
	//
	// TODO: if we are on fire, play the correct torso animation
	if ( ent->s.onFireEnd > level.time ) {
		// set the animation, and a short timer, but long enough to last until the next frame
		//if (g_cheats.integer) G_Printf( "TODO: torso onfire animation\n" );
	}
	//
	// is the danger gone?
	if ( cs->dangerEntityValidTime < level.time ) {
		return AIFunc_DefaultStart( cs );
	}
	//
	// special case: if it's a grenade, and it's going to land near us with some time left before it
	// explodes, try and kick it back
	//
	danger = &g_entities[cs->dangerEntity];
	if ( ent->s.onFireEnd < level.time ) {
		if ( ( danger->s.weapon == WP_GRENADE_LAUNCHER || danger->s.weapon == WP_GRENADE_PINEAPPLE ) &&
			 ( danger->nextthink - level.time > 1500 ) && ( level.lastGrenadeKick < level.time - 3000 ) &&
			 ( cs->aiFlags & AIFL_CATCH_GRENADE ) && !( danger->flags & FL_AI_GRENADE_KICK ) ) {
			// if it was thrown by a friend of ours, leave it alone
			if ( !AICast_SameTeam( cs, danger->r.ownerNum ) ) {
				if ( G_PredictMissile( danger, danger->nextthink - level.time, cs->combat.takeCoverPos, qfalse ) ) {
					// make sure it's a valid position, and drop it down to the ground
					cs->combat.takeCoverPos[2] += -ent->r.mins[2] + 12;
					VectorCopy( cs->combat.takeCoverPos, end );
					end[2] -= 90;
					trap_Trace( &tr, cs->combat.takeCoverPos, ent->r.mins, ent->r.maxs, end, cs->entityNum,
								MASK_SOLID );
					VectorCopy( tr.endpos, cs->combat.takeCoverPos );
					if ( !tr.startsolid && ( tr.fraction < 1.0 ) &&
						 VectorDistance( cs->clientState->origin, cs->combat.takeCoverPos ) <
							 cs->attributes[RUNNING_SPEED] * 0.0004 * ( danger->nextthink - level.time - 2000 ) ) {

						// check for a clear path to the grenade
						trap_Trace( &tr, cs->clientState->origin, ent->r.mins, ent->r.maxs, cs->combat.takeCoverPos,
									cs->entityNum, MASK_SOLID );

						if ( VectorDistance( tr.endpos, cs->combat.takeCoverPos ) < 8 ) {
							danger->flags |= FL_AI_GRENADE_KICK;
							ent->flags |= FL_AI_GRENADE_KICK;
							level.lastGrenadeKick = level.time;
							return AIFunc_GrenadeKickStart(
								cs ); // we should decide our course of action within this start function (dive or return grenade)
						}
					}
				}
			}
			// if it's really close to us, and we're heading for it, may as well pick it up
			if ( VectorLength( danger->s.pos.trDelta ) < 10 &&
				 VectorDistance( danger->r.currentOrigin, cs->clientState->origin ) < 128 &&
				 ( level.lastGrenadeKick < level.time - 3000 ) && ( cs->aiFlags & AIFL_CATCH_GRENADE ) ) {
				vec3_t vec;
				VectorSubtract( danger->r.currentOrigin, cs->clientState->origin, vec );
				if ( DotProduct( vec, cs->clientState->velocity ) > 0 ) {
					danger->flags |= FL_AI_GRENADE_KICK;
					ent->flags |= FL_AI_GRENADE_KICK;
					level.lastGrenadeKick = level.time;
					return AIFunc_GrenadeKickStart(
						cs ); // we should decide our course of action within this start function (dive or return grenade)
				}
			}
		}
	}
	//
	if ( g_entities[cs->dangerEntity].inuse ) {
		// is our current destination still safe?
		if ( Distance( cs->dangerEntityPos, cs->combat.takeCoverPos ) < cs->dangerDist &&
			 AICast_VisibleFromPos( cs->dangerEntityPos, cs->dangerEntity, cs->combat.takeCoverPos, cs->entityNum,
									qfalse ) ) {
			//G_Printf("current coverPos is dangerous, looking for a better place..\n" );
			if ( !AICast_GetTakeCoverPos( cs, cs->dangerEntity, cs->dangerEntityPos, cs->combat.takeCoverPos ) ) {
				// just run away from it ???
			}
		}
	} else {
		// the entity isn't here anymore, so stop hiding
		cs->dangerEntityValidTime = -1;
		return AIFunc_DefaultStart( cs );
	}
	//
	VectorCopy( cs->combat.takeCoverPos, destorg );
	VectorSubtract( destorg, cs->clientState->origin, vec );
	vec[2] *= 0.2;
	dist = VectorLength( vec );
	//
	shouldAttack = qfalse;
	if ( ent->s.onFireEnd < level.time ) {
		// look for things we should attack
		numEnemies = AICast_ScanForEnemies( cs, enemies );
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
		}
	}
	//
	// if we are now safe from the danger, stop running away
	if ( cs->dangerEntity >= MAX_CLIENTS &&
		 Distance( cs->dangerEntityPos, cs->clientState->origin ) > cs->dangerDist * 1.5 ) {
		// don't move, wait for danger to pass
	} else
		// are we close enough to the goal?
		if ( dist > 8 ) {
			moveresult = AICast_MoveToPos( cs, destorg, -1 );
			if ( moveresult ) {
				//if the movement failed
				if ( moveresult->failure || moveresult->blocked ) {
					//reset the avoid reach, otherwise bot is stuck in current area
					trap_BotResetAvoidReach( bs->ms );
					if ( g_entities[cs->dangerEntity].inuse ) {
						// find a better spot?
						AICast_GetTakeCoverPos( cs, cs->dangerEntity, cs->dangerEntityPos, cs->combat.takeCoverPos );
					} else {
						VectorCopy( cs->clientState->origin, cs->combat.takeCoverPos );
					}
				}
			}
			if ( ent->s.onFireEnd < level.time ) {
				// slow down real close to the goal, so we don't go passed it
				cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, 0 );
			}
			//
			// pretend we can still see them while we run to our hide pos, this way they are less likely
			// to forget about their enemy once they get there
			if ( ent->s.onFireEnd < level.time && cs->enemyNum >= 0 &&
				 cs->perception.vislist[cs->enemyNum].real_visible_timestamp &&
				 ( cs->perception.vislist[cs->enemyNum].real_visible_timestamp > level.time - 10000 ) ) {
				AICast_UpdateVisibility( &g_entities[cs->entityNum], &g_entities[cs->enemyNum], qfalse,
										 cs->perception.vislist[cs->enemyNum].real_visible_timestamp ==
											 cs->perception.vislist[cs->enemyNum].lastcheck_timestamp );
			}

		} else {
			// set our origin as the hidepos, that way if we are still in danger, we should find a better spot
			VectorCopy( cs->clientState->origin, cs->combat.takeCoverPos );
			// check for a movement we should be making
			if ( cs->navigation.obstructingTime > level.time ) {
				AICast_MoveToPos( cs, cs->navigation.obstructingPos, -1 );
			}

			// if we are on fire, never stop
			if ( ent->s.onFireEnd > level.time ) {
				VectorCopy( cs->clientState->origin, cs->dangerEntityPos );
				cs->dangerEntityValidTime = level.time + 10000;
			}
		}
	//
	// if we should be attacking something on our way
	if ( shouldAttack ) {
		//attack the enemy if possible
		AICast_ProcessAttack( cs );
	} else { //if (dist < 48) {
		// if we've recently been in a fight, look towards the enemy
		if ( cs->lastEnemy >= 0 ) {
			// if we only just recently saw them, face that direction
			if ( cs->perception.vislist[cs->lastEnemy].visible_timestamp > ( level.time - 20000 ) ) {
				vec3_t dir;
				//
				VectorSubtract( cs->perception.vislist[cs->lastEnemy].visible_pos, cs->clientState->origin, dir );
				VectorNormalize( dir );
				vectoangles( dir, cs->ideal_viewangles );
			}
		}
	}

	// reload?
	AICast_IdleReload( cs );

	return NULL;
}

/*
============
AIFunc_AvoidDangerStart()
============
*/
char *AIFunc_AvoidDangerStart( cast_state_t *cs ) {
	//
	//if (!AICast_CanMoveWhileFiringWeapon( cs->weaponNum )) {
	// always run to the cover point
	cs->attackcrouch_time = 0;
	//}
	// make sure we move if we are allowed (scripting will overwrite this if necessary)
	cs->script.castScriptStatus.scriptNoMoveTime = 0;
	// resume following once danger has gone
	cs->script.castScriptStatus.scriptGotoId = -1;
	//
	AI_SetLegacyState( cs, AIFunc_AvoidDanger );
	return "AIFunc_AvoidDanger";
}

/*
============
AIFunc_GrenadeFlush()
============
*/
char *AIFunc_GrenadeFlush( cast_state_t *cs ) {
	int enemies[MAX_CLIENTS];
	int numEnemies;
	bot_moveresult_t *moveresult;
	vec3_t dir;
	gentity_t *followent, *grenade, *ent;
	ai_client_state_t *bs;
	vec3_t destorg, endPos;
	qboolean moved = qfalse;
	int hitclient;
	//qboolean attacked = qfalse; // TTimo: unused
	float dist, oldyaw;
	int grenadeType;
	int *ammo;

	bs = cs->clientState;
	ent = &g_entities[cs->entityNum];
	//
	// if we are throwing the grenade, keep holding down fire

	ammo = cs->clientState->cur_ps.ammo;
	if ( AICast_GotEnoughAmmoForWeapon( cs, WP_GRENADE_LAUNCHER ) ) {
		grenadeType = WP_GRENADE_LAUNCHER;
	} else if ( AICast_GotEnoughAmmoForWeapon( cs, WP_GRENADE_PINEAPPLE ) ) {
		grenadeType = WP_GRENADE_PINEAPPLE;
	} else { // not enough ammo, abort
		return AIFunc_DefaultStart( cs );
	}

	// (SA) probably read the fweapon from t
	if ( cs->grenade.grenadeFlushFiring ) {
		// are we still moving?
		if ( VectorLength( cs->clientState->cur_ps.velocity ) ) {
			// keep waiting
			// pause for a bit, so the grenade comes out correctly
			cs->lockViewAnglesTime = level.time + 1200;
			if ( cs->script.castScriptStatus.scriptNoMoveTime < level.time + 1200 ) {
				cs->script.castScriptStatus.scriptNoMoveTime = level.time + 1200;
			}
			return NULL;
		}
		if ( cs->combat.weaponFireTimes[cs->grenade.grenadeFlushFiring] < cs->thinkFuncChangeTime ) {
			// have we switched weapons?
			if ( cs->weaponNum != cs->grenade.grenadeFlushFiring ) {
				// damn
				hitclient = -1;
			} else {
				// keep checking it's ok
				CalcMuzzlePoints( ent, grenadeType );
				// fire a dummy grenade
				grenade = weapon_grenadelauncher_fire( ent, WP_GRENADE_LAUNCHER );
				// check to see what will happen
				hitclient = AICast_SafeMissileFire( grenade, grenade->nextthink - level.time, cs->enemyNum, destorg,
													cs->entityNum, endPos );
				// kill the grenade
				G_FreeEntity( grenade );
				if ( hitclient != 1 ) { // it wont hit them, abort
					hitclient = -1; // a miss is as bad as a friendly kill
				}
			}
			if ( hitclient == -1 ) { // doh
				//G_Printf("aborted grenade\n");
				cs->script.castScriptStatus.scriptNoMoveTime = 0;
				cs->lockViewAnglesTime = 0;
				AICast_ChooseWeapon( cs, qfalse );
				return AIFunc_DefaultStart( cs );
			}
			if ( !cs->clientState->cur_ps.grenadeTimeLeft ) {
				// hold fire button down
				trap_EA_Attack( bs->client );
				cs->bFlags |= AI_FRAME_ATTACKED;
			}
			cs->lockViewAnglesTime = level.time + 500;
			return NULL;
		}
		// the grenade/pineapple has been released!
		cs->lockViewAnglesTime = -1;
		cs->grenade.startGrenadeFlushTime = level.time + 2000 + rand() % 2000; // dont throw one again for a bit
		return AIFunc_DefaultStart( cs );
	}
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
	if ( cs->weaponNum && ( cs->script.castScriptStatus.scriptFlags & SFL_NOCHANGEWEAPON ) ) {
		return AIFunc_IdleStart( cs );
	}
	//
	if ( cs->enemyNum < 0 ) {
		return AIFunc_IdleStart( cs );
	}
	//
	// if we have started a script, abort the grenade flush
	if ( cs->script.castScriptStatus.castScriptEventIndex >= 0 ) {
		return AIFunc_IdleStart( cs );
	}
	// trying for too long?
	if ( cs->grenade.startGrenadeFlushTime < level.time - 4000 ) {
		cs->grenade.startGrenadeFlushTime = level.time;
		return AIFunc_IdleStart( cs );
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
	// if we can see them, go back to an attack state after some time
	if ( AICast_CheckAttack( cs, cs->enemyNum, qfalse ) &&
		 cs->navigation.obstructingTime <
			 level.time ) { // give us some time to throw the grenade, otherwise go back to attack state
		//if ((cs->grenade.grenadeFlushEndTime > 0 && cs->grenade.grenadeFlushEndTime < level.time)) {
		//G_Printf("aborting, enemy is attackable\n");
		return AIFunc_BattleStart( cs );
		//} else if (cs->grenade.grenadeFlushEndTime < 0) {
		//	cs->grenade.grenadeFlushEndTime = level.time + 1500;
		//}
		//attack the enemy if possible
		//AICast_ProcessAttack( cs );
		//attacked = qtrue;
	} else {
		// not visible, go to their previously visible position
		if ( !cs->perception.vislist[cs->enemyNum].visible_timestamp ||
			 Distance( bs->origin, cs->perception.vislist[cs->enemyNum].real_visible_pos ) < 16 ) {
			// we're done attacking, go back to default state, which in turn will recall previous state
			//
			// face the direction they currently are from this position (bit of a hack, but it looks best)
			VectorSubtract( g_entities[cs->enemyNum].client->ps.origin,
							cs->perception.vislist[cs->enemyNum].visible_pos, dir );
			vectoangles( dir, cs->ideal_viewangles );
			//
			//G_Printf("aborting, reached visible pos\n");
			return AIFunc_DefaultStart( cs );
		}
	}

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
	} else if ( !( cs->bFlags & AI_FRAME_ATTACKED ) && numEnemies > 0 ) {
		int i;
		for ( i = 0; i < numEnemies; i++ ) {
			if ( enemies[i] != cs->enemyNum && AICast_CheckAttack( cs, enemies[i], qfalse ) ) {
				cs->enemyNum = enemies[i];
				//G_Printf("aborting, other enemy\n");
				return AIFunc_BattleStart( cs );
			}
		}
	}

	if ( followent->client ) { // go to the last visible position
		VectorCopy( cs->perception.vislist[cs->enemyNum].visible_pos, destorg );
	} else // assume we know where other entities are
	{
		VectorCopy( followent->s.pos.trBase, destorg );
	}
	//
	dist = VectorDistance( destorg, cs->clientState->origin );
	//
	if ( cs->perception.vislist[cs->enemyNum].lastcheck_timestamp >
			 cs->perception.vislist[cs->enemyNum].real_visible_timestamp ||
		 dist > 128 ) {
		//
		// go to them
		//
		if ( followent->client && followent->health <= 0 ) {
			cs->enemyNum = -1;
			//G_Printf("aborting, enemy dead\n");
			return AIFunc_DefaultStart( cs );
		}
		//
		// ...........................................................
		// Do the movement..
		//
		// move straight to them if we can
		if ( ( cs->leaderNum < 0 ) && ( cs->clientState->cur_ps.groundEntityNum != ENTITYNUM_NONE ||
										g_entities[cs->entityNum].waterlevel > 1 ) ) {
			aicast_predictmove_t move;
			vec3_t dir;
			bot_input_t bi;
			usercmd_t ucmd;
			trace_t tr;

			// trace will eliminate most unsuccessful paths
			trap_Trace( &tr, cs->clientState->origin, g_entities[cs->entityNum].r.mins,
						g_entities[cs->entityNum].r.maxs, followent->r.currentOrigin, cs->entityNum,
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
					vectoangles( dir, cs->ideal_viewangles );
					cs->ideal_viewangles[2] *= 0.5;
					moved = qtrue;
				} else { // clear movement
					//trap_EA_Move(cs->entityNum, dir, 0);
				}
			}
		}
		// just go to them
		if ( !moved ) {
			moveresult = AICast_MoveToPos( cs, destorg, cs->enemyNum );
			if ( moveresult && moveresult->failure ) { // no path, so go back to idle behaviour
				cs->enemyNum = -1;
				//G_Printf("aborting, movement failure\n");
				return AIFunc_DefaultStart( cs );
			} else {
				moved = qtrue;
			}
		}
	}
	//
	// ...........................................................
	// if we throw a grenade from here, will it get their last visible position?
	//
	CalcMuzzlePoints( ent, grenadeType );
	// fire a dummy grenade
	grenade = weapon_grenadelauncher_fire( ent, WP_GRENADE_LAUNCHER );
	// check to see what will happen
	hitclient = AICast_SafeMissileFire( grenade, grenade->nextthink - level.time, cs->enemyNum, destorg, cs->entityNum,
										endPos );
	// kill the grenade
	G_FreeEntity( grenade );
	//if (!attacked)
	//	cs->weaponNum = grenadeType;	// select grenade launcher
	// set our angles for the next frame
	oldyaw = cs->ideal_viewangles[YAW];
	AICast_AimAtEnemy( cs );
	// if we can't see them, keep facing our movement dir, but use the pitch information
	if ( !AICast_EntityVisible( cs, cs->enemyNum, qtrue ) ) {
		cs->ideal_viewangles[YAW] = oldyaw;
	}

	if ( hitclient == 1 ) {
		// it will hit their last visible position
		// give us some time to aim and adjust
		if ( cs->thinkFuncChangeTime < level.time - 200 ) {
			cs->bFlags |= AI_FRAME_ATTACKED;
			cs->weaponNum = grenadeType; // select grenade launcher
			cs->grenade.grenadeFlushFiring = cs->weaponNum;
			// pause for a bit, so the grenade comes out correctly
			cs->lockViewAnglesTime = level.time + 1200;
			if ( cs->script.castScriptStatus.scriptNoMoveTime < level.time + 1200 ) {
				cs->script.castScriptStatus.scriptNoMoveTime = level.time + 1200;
			}
			return NULL;
		}
	} else if ( hitclient == -1 ) {
		// hit a friendly
		cs->grenade.startGrenadeFlushTime = level.time + 3000; // don't try again for a while
		//G_Printf("aborting, too dangerous\n");
		return AIFunc_DefaultStart( cs );
	} else if ( hitclient == -2 ) {
		// went too far, so angle down a bit
		cs->ideal_viewangles[PITCH] += 15 * random();
	} else {
		if ( cs->thinkFuncChangeTime < level.time - 200 ) {
			// if it went reasonably close to them, but safe from us, then fire away
			if ( Distance( endPos, cs->clientState->origin ) >
				 100 + Distance( endPos, g_entities[cs->enemyNum].r.currentOrigin ) ) {
				trap_EA_Attack( bs->client );
				cs->bFlags |= AI_FRAME_ATTACKED;
				cs->weaponNum = grenadeType; // select grenade launcher
				cs->grenade.grenadeFlushFiring = cs->weaponNum;
				// pause for a bit, so the grenade comes out correctly
				cs->lockViewAnglesTime = level.time + 1200;
				if ( cs->script.castScriptStatus.scriptNoMoveTime < level.time + 1200 ) {
					cs->script.castScriptStatus.scriptNoMoveTime = level.time + 1200;
				}
				return NULL;
			}
		}
		cs->ideal_viewangles[PITCH] += -10 * random();
	}
	//
	return NULL;
}

/*
============
AIFunc_GrenadeFlushStart()
============
*/
char *AIFunc_GrenadeFlushStart( cast_state_t *cs ) {
	ai_encounter.lastGrenadeFlushTime = level.time; // + rand()%2000;
	cs->grenade.startGrenadeFlushTime = level.time;
	cs->grenade.grenadeFlushEndTime = -1;
	cs->lockViewAnglesTime = 0;
	cs->navigation.combatGoalTime = 0;
	cs->grenade.grenadeFlushFiring = qfalse;
	// don't wait too long before taking cover, if we just aborted one
	if ( cs->combat.takeCoverTime > level.time + 1000 ) {
		cs->combat.takeCoverTime = level.time + 500 + rand() % 500;
	}
	//
	AI_SetLegacyState( cs, AIFunc_GrenadeFlush );
	return "AIFunc_GrenadeFlush";
}

/*
============
AIFunc_GrenadeKick()
============
*/
char *AIFunc_GrenadeKick( cast_state_t *cs ) {
	bot_moveresult_t *moveresult;
	ai_client_state_t *bs;
	vec3_t destorg, vec;
	float dist, speed;
	int enemies[MAX_CLIENTS], numEnemies, i;
	qboolean shouldAttack;
	int *ammo;
	gentity_t *danger;
	gentity_t *ent;
	vec3_t end;
	trace_t tr;
	vec3_t dir;
	int weapon;

	// !!! NOTE: the only way control should pass out of here, is by calling AIFunc_DefaultStart()

	ent = &g_entities[cs->entityNum];
	danger = &g_entities[cs->dangerEntity];

	weapon = cs->grenade.grenadeKickWeapon;

	// just to be sure, give us the grenade launcher
	//ent->client->ps.weapons |= (1 << weapon);

	//
	// NOTE: ignore all danger, since we are trying to solve the situation anyway
	//
	// we need to move towards it
	bs = cs->clientState;
	//
	// are we throwing it back?
	if ( cs->grenade.grenadeFlushFiring ) {
		// wait until the animation is done
		if ( !ent->client->ps.legsTimer ) {
			return AIFunc_DefaultStart( cs );
		}
		// wait till its finished
		return NULL;
		/*
		cs->weaponNum = weapon;	// select grenade launcher
		//
		if (cs->combat.weaponFireTimes[weapon] < cs->thinkFuncChangeTime) {
			if (!cs->clientState->cur_ps.grenadeTimeLeft) {
				// hold fire button down
				AICast_AimAtEnemy( cs );
				trap_EA_Attack(bs->client);
				cs->bFlags |= AI_FRAME_ATTACKED;
			}
			//
			return NULL;
		}
		// the grenade has been released!
		//
		// modify the explode time
		g_entities[ent->grenadeFired].nextthink = ent->grenadeExplodeTime;
		if (g_entities[ent->grenadeFired].nextthink < level.time + 200) {	// cut them some slack
			g_entities[ent->grenadeFired].nextthink = level.time + 200 + rand()%500;
		}
		// make sure no-one tries to throw this back again (hot potatoe syndrome)
		g_entities[ent->grenadeFired].flags |= FL_AI_GRENADE_KICK;
		//
		cs->grenade.grenadeFlushFiring = qfalse;
		cs->lockViewAnglesTime = -1;
		cs->grenade.startGrenadeFlushTime = level.time + 2000 + rand()%2000;	// dont throw one again for a bit
		level.lastGrenadeKick = level.time;
		return AIFunc_DefaultStart( cs );
		*/
	}
	//
	/*
	// have we caught the grenade?
	if (!(ent->flags & FL_AI_GRENADE_KICK)) {
		// select grenades
		cs->weaponNum = weapon;	// select grenade launcher
		AICast_AimAtEnemy( cs );
		// hold fire
		trap_EA_Attack(bs->client);
		cs->bFlags |= AI_FRAME_ATTACKED;
		cs->grenade.grenadeFlushFiring = qtrue;
		//
		return NULL;
	}
*/
	//
	// is it about to explode in our face?
	if ( level.time >
		 danger->nextthink - (int)( 2.0 * VectorDistance( cs->clientState->origin, danger->r.currentOrigin ) ) ) {
		// abort!!
		if ( !AICast_GetTakeCoverPos( cs, cs->dangerEntity, cs->dangerEntityPos, cs->combat.takeCoverPos ) ) {
			// shit??
		}
		// go to a position that cannot be seen from the dangerPos
		cs->combat.takeCoverTime = danger->nextthink + 1000;
		cs->attackcrouch_time = 0;
		level.lastGrenadeKick = level.time;
		return AIFunc_AvoidDangerStart( cs );
	}
	//
	/*
	// are we close enough to start crouching?
	if (danger->s.pos.trDelta[2] < 40 && VectorDistance( danger->r.currentOrigin, cs->clientState->origin ) < 48 && (danger->r.currentOrigin[2] < cs->clientState->origin[2]) &&
		VectorLength(danger->s.pos.trDelta) < 40) {
		// crouch to pick it up
		cs->attackcrouch_time = level.time + 300;
	}
	*/
	cs->attackcrouch_time = 0; // animation is played from standing start
	//
	// are we close enough to pick it up?
	if ( /*cs->grenade.grenadeGrabFlag <= 0 || */
		 ( danger->s.pos.trDelta[2] < 20 && VectorDistance( danger->r.currentOrigin, cs->clientState->origin ) < 48 &&
		   ( danger->r.currentOrigin[2] < cs->clientState->origin[2] ) &&
		   VectorLength( danger->s.pos.trDelta ) < 50 ) ) {
		//
		// we have a choice here, either pick up and return, or just kick it
		//		if ((cs->grenade.grenadeGrabFlag == -1) || (cs->grenade.grenadeGrabFlag == qtrue && level.time > danger->nextthink - 2000)) {	// kick

		// play the kick anim
		if ( cs->grenade.grenadeGrabFlag == qtrue ) {
			AICast_AimAtEnemy( cs );
			// play the kick anim
			BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_KICKGRENADE, qfalse, qtrue );
			cs->grenade.grenadeGrabFlag = -1;
			// stop the grenade from moving away
			danger->s.pos.trDelta[0] = 0;
			danger->s.pos.trDelta[1] = 0;
			if ( danger->s.pos.trDelta[2] > 0 ) {
				danger->s.pos.trDelta[2] = 0;
			}
		} else if ( ent->client->ps.legsTimer < 800 ) {
			// send the grenade on its way
			cs->grenade.grenadeFlushFiring = qtrue;
			AngleVectors( cs->viewangles, dir, NULL, NULL );
			dir[2] = 0.4;
			VectorNormalize( dir );
			speed = 400;
			if ( cs->enemyNum >= 0 ) {
				speed = 1.5 * VectorDistance( danger->r.currentOrigin, g_entities[cs->enemyNum].r.currentOrigin );
				if ( speed > 650 ) {
					speed = 650;
				}
			}
			VectorScale( dir, speed, danger->s.pos.trDelta );
			danger->r.ownerNum = ent->s.number; // we are now the owner, let it pass through us
			danger->s.pos.trTime = level.time - 50; // move a bit on the very first frame
			VectorCopy( danger->r.currentOrigin, danger->s.pos.trBase );
			danger->s.pos.trType = TR_GRAVITY;
			SnapVector( danger->s.pos.trDelta ); // save net bandwidth
		}
		/*
		} else { // throw

			if (cs->grenade.grenadeGrabFlag == qtrue) {
				AICast_AimAtEnemy( cs );
				// play the pickup anim
				BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_PICKUPGRENADE, qfalse, qtrue );
				cs->grenade.grenadeGrabFlag = qfalse;
				// stop the grenade from moving away
				danger->s.pos.trDelta[0] = 0;
				danger->s.pos.trDelta[1] = 0;
				if (danger->s.pos.trDelta[2] > 0) {
					danger->s.pos.trDelta[2] = 0;
				}
			} else if (ent->client->ps.legsTimer < 400) {
				// send the grenade on its way
				cs->grenade.grenadeFlushFiring = qtrue;
				AngleVectors( cs->viewangles, dir, NULL, NULL );
				dir[2] = 0.4;
				VectorNormalize( dir );
				speed = 500;
				if (cs->enemyNum >= 0) {
					speed = 2*VectorDistance(danger->r.currentOrigin, g_entities[cs->enemyNum].r.currentOrigin);
					if (speed > 650)
						speed = 650;
				}
				VectorScale( dir, speed, danger->s.pos.trDelta );
				trap_LinkEntity( danger );
			} else if (ent->client->ps.legsTimer < 800) {
				// stop showing the grenade
				trap_UnlinkEntity( danger );
			}
		}
*/
		//
		return NULL;
	}
	//
	cs->grenade.grenadeGrabFlag = qtrue; // we must play the anim before we can grab it
	//
	// is the danger gone?
	if ( level.time > cs->dangerEntityValidTime || !danger->inuse ) {
		return AIFunc_DefaultStart( cs );
	}
	//
	// update the predicted position of the grenade
	if ( G_PredictMissile( danger, danger->nextthink - level.time, cs->combat.takeCoverPos, qfalse ) ) {
		// make sure it's a valid position, and drop it down to the ground
		cs->combat.takeCoverPos[2] += -ent->r.mins[2] + 8;
		VectorCopy( cs->combat.takeCoverPos, end );
		end[2] -= 80;
		trap_Trace( &tr, cs->combat.takeCoverPos, ent->r.mins, ent->r.maxs, end, cs->entityNum, MASK_SOLID );
		if ( tr.startsolid ) { // not a valid position, abort
			level.lastGrenadeKick = level.time;
			return AIFunc_DefaultStart( cs );
		}
		VectorCopy( tr.endpos, cs->combat.takeCoverPos );
	} else { // prediction failed, so use current position
		VectorCopy( danger->r.currentOrigin, cs->combat.takeCoverPos );
		cs->combat.takeCoverPos[2] += 16; // lift it off the floor
	}

	VectorCopy( cs->combat.takeCoverPos, destorg );
	VectorSubtract( destorg, cs->clientState->origin, vec );
	//vec[2] *= 0.2;
	dist = VectorLength( vec );
	//
	// look for things we should attack
	// if we are out of ammo, we shouldn't bother trying to attack
	ammo = cs->clientState->cur_ps.ammo;
	shouldAttack = qfalse;
	numEnemies = 0;
	if ( AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
		numEnemies = AICast_ScanForEnemies( cs, enemies );
		if ( numEnemies == -1 ) { // query mode
			return NULL;
		}
		if ( numEnemies == -2 ) { // inspection may be required
			char *retval;
			// TTimo: gcc: suggest () around assignment used as truth value
			if ( ( retval = AIFunc_InspectFriendlyStart( cs, enemies[0] ) ) ) {
				return retval;
			}
		}
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
		}
	}
	//
	// are we close enough to the goal?
	if ( dist > 12 ) { // not close enough
		//
		moveresult = AICast_MoveToPos( cs, destorg, -1 );
		if ( moveresult ) {
			//if the movement failed
			if ( moveresult->failure ) {
				//reset the avoid reach, otherwise bot is stuck in current area
				trap_BotResetAvoidReach( bs->ms );
				// couldn't get there, so stop trying to get there
				level.lastGrenadeKick = level.time;
				return AIFunc_DefaultStart( cs );
			}
			//
			if ( moveresult->blocked ) { // abort if we get blocked at any point
				level.lastGrenadeKick = level.time;
				return AIFunc_DefaultStart( cs );
			}
		}
		// we should slow down on approaching it
		cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, 0 );
	}
	/*	//
	// if we are close, put our weapon away and get ready to catch it
	if (VectorDistance( danger->r.currentOrigin, cs->clientState->origin ) < 128) {
		// put weapon away, select grenades
		// FIXME: does this fail if we don't have any grenades?
		cs->weaponNum = weapon;	// select grenade launcher
		shouldAttack = qfalse;	// don't attack until we've caught it
	}
*/
	// if we should be attacking something on our way
	if ( shouldAttack ) {
		vec3_t vec, dir;

		//attack the enemy if possible
		AICast_ProcessAttack( cs );
		//
		// if they are close, and we're heading for them, we should abort this manouver
		VectorSubtract( g_entities[cs->enemyNum].client->ps.origin, bs->origin, vec );
		if ( VectorNormalize( vec ) < 64 ) {
			if ( VectorNormalize2( bs->velocity, dir ) > 20 ) { // we are moving
				if ( DotProduct( dir, vec ) > 0 ) {
					// abort
					level.lastGrenadeKick = level.time;
					return AIFunc_DefaultStart( cs );
				}
			}
		}
	} else {
		// face the direction that the grenade is coming
		VectorSubtract( danger->r.currentOrigin, cs->clientState->origin, dir );
		dir[2] = 0;
		VectorNormalize( dir );
		vectoangles( dir, cs->ideal_viewangles );
	}

	return NULL;
}

/*
=============
AIFunc_GrenadeKickStart()
=============
*/
char *AIFunc_GrenadeKickStart( cast_state_t *cs ) {
	gentity_t *danger;
	gentity_t *ent;
	//gentity_t *trav;
	//int numFriends, i;

	//G_Printf( "Excuse me, you dropped something\n" );

	ent = &g_entities[cs->entityNum];
	danger = &g_entities[cs->dangerEntity];
	// should we dive onto the grenade?
	/*
	if (danger->s.pos.trDelta[2] < 30) {
		// count the number of friends near us
		numFriends = 0;
		for (i=0, trav=g_entities; i<aicast_maxclients; i++, trav++) {
			if (!trav->inuse)
				continue;
			if (trav->aiInactive)
				continue;
			if (trav->health <= 0)
				continue;
			if (!AICast_SameTeam( cs, i ))
				continue;
			if (VectorDistance( cs->combat.takeCoverPos, trav->r.currentOrigin ) > 200)
				continue;
			numFriends++;
		}
		// if there are enough friends around, and we have a clear path to the position, sacrifice ourself!
		if (numFriends > 2) {
			trace_t tr;
			trap_Trace( &tr, cs->clientState->origin, ent->r.mins, ent->r.maxs, cs->combat.takeCoverPos, cs->entityNum, MASK_SOLID );
			if (tr.fraction == 1.0 && !tr.startsolid) {
				return AIFunc_GrenadeDiveStart( cs );
			}
		}
	}
	*/
	//
	// we have decided to kick or throw the grenade away
	cs->grenade.grenadeKickWeapon = danger->s.weapon;
	cs->grenade.grenadeFlushFiring = qfalse;
	AI_SetLegacyState( cs, AIFunc_GrenadeKick );
	return "AIFunc_GrenadeKick";
}
