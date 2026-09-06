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
============
AICast_SpecialFunc
============
*/
void AICast_SpecialFunc( cast_state_t *cs ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	gentity_t *enemy = NULL;

	if ( cs->enemyNum >= 0 ) {
		enemy = &g_entities[cs->enemyNum];
	}

	switch ( cs->aiCharacter ) {
	case AICHAR_WARZOMBIE:
		// disable defense unless we want it
		ent->flags &= ~FL_DEFENSE_CROUCH;
		// if we are pursuing the player from a distance, use our "crouch moving defense"
		if ( ( enemy ) && ( cs->perception.vislist[cs->enemyNum].real_visible_timestamp > level.time - 5000 ) &&
			 ( Distance( cs->clientState->origin, enemy->s.pos.trBase ) > 200 ) &&
			 ( Distance( cs->clientState->origin, enemy->s.pos.trBase ) < 600 ) &&
			 ( cs->clientState->cur_ps.groundEntityNum != ENTITYNUM_NONE )
			 //&&	(infront( ent, enemy ))
			 && ( infront( enemy, ent ) ) ) {
			// crouch
			trap_EA_Crouch( cs->entityNum );
			// enable defense pose
			ent->flags |= FL_DEFENSE_CROUCH;
		}
		break;
	case AICHAR_HELGA:
		// if she has recently finished a spirit attack, go into charge mode
		if ( ( cs->combat.weaponFireTimes[WP_MONSTER_ATTACK2] &&
			   ( cs->combat.weaponFireTimes[WP_MONSTER_ATTACK2] > level.time - 12000 ) ) ||
			 ( cs->combat.weaponFireTimes[WP_MONSTER_ATTACK1] &&
			   ( cs->combat.weaponFireTimes[WP_MONSTER_ATTACK1] > level.time - 6000 ) ) ) {
			BG_UpdateConditionValue( cs->entityNum, ANIM_COND_CHARGING, 1, qfalse );
			cs->actionFlags &= ~CASTACTION_WALK;
		} else { // not charging
			BG_UpdateConditionValue( cs->entityNum, ANIM_COND_CHARGING, 0, qfalse );
		}
		//
		if ( ent->health <= 0 && ent->takedamage ) {
			if ( ent->client->ps.torsoTimer < 500 ) {
				// blow up
				GibEntity( ent, 0 );
				ent->takedamage = qfalse;
				ent->r.contents = 0;
				ent->health = GIB_HEALTH - 1;
			}
		}
		break;
	case AICHAR_HEINRICH:
		if ( ( ent->health <= 0.25 * cs->attributes[STARTING_HEALTH] ) ||
			 ( cs->combat.weaponFireTimes[WP_MONSTER_ATTACK1] > level.time - 6000 ) // walk for period after attack
			 || ( cs->combat.weaponFireTimes[WP_MONSTER_ATTACK1] % 8000 < 3000 ) ) { // dont run constantly
			cs->actionFlags |= CASTACTION_WALK;
		} else { // charging
			cs->actionFlags &= ~CASTACTION_WALK;
		}
		// allow running while attacking
		if ( ent->client->ps.torsoTimer && !ent->client->ps.legsTimer ) {
			cs->actionFlags &= ~CASTACTION_WALK;
		}
		//
		if ( ent->health <= 0 && ent->takedamage ) {
			if ( ent->client->ps.torsoTimer < 500 ) {
				// blow up
				GibEntity( ent, 0 );
				ent->takedamage = qfalse;
				ent->r.contents = 0;
				ent->health = GIB_HEALTH - 1;
			}
			// blow up other warriors left around
			if ( !ent->takedamage || ( ent->count2 < level.time && ent->client->ps.torsoTimer < 4000 ) ) {
				int i;
				gentity_t *trav;
				for ( i = 0, trav = g_entities; i < level.maxclients; i++, trav++ ) {
					if ( !trav->inuse ) {
						continue;
					}
					if ( trav->aiCharacter != AICHAR_WARZOMBIE ) {
						continue;
					}
					if ( trav->aiInactive ) {
						continue;
					}
					if ( trav->health <= 0 ) {
						continue;
					}
					// blow it up, set some delay
					G_Damage( trav, ent, ent, NULL, NULL, 99999, 0, MOD_CRUSH );
					if ( ent->takedamage ) {
						ent->count2 = level.time + 200 + rand() % 1500;
					}
				}
			}
		}
		break;
	case AICHAR_ZOMBIE:
		if ( COM_BitCheck( cs->clientState->cur_ps.weapons, WP_MONSTER_ATTACK1 ) ) { // flaming zombie, run
			BG_UpdateConditionValue( cs->entityNum, ANIM_COND_CHARGING, 1, qfalse );
		}
		break;
	}
}

/*
==============
AICast_PredictMovement

  Simulates movement over a number of frames, returning the end position
==============
*/
void AICast_PredictMovement( cast_state_t *cs, int numframes, float frametime, aicast_predictmove_t *move,
							 usercmd_t *ucmd, int checkHitEnt ) {
	int frame, i;
	playerState_t ps;
	pmove_t pm = {};
	trace_t tr;
	vec3_t end, startHitVec, thisHitVec, lastOrg, projPoint;
	qboolean checkReachMarker;
	gentity_t *ent = &g_entities[cs->entityNum];
	bot_input_t bi;

	//int pretime = Sys_MilliSeconds();
	//G_Printf("PredictMovement: %f duration, %i frames\n", frametime, numframes );

	if ( cs->clientState ) {
		ps = cs->clientState->cur_ps;
		trap_EA_GetInput( cs->entityNum, (float)level.time / 1000, &bi );
	} else {
		ps = g_entities[cs->entityNum].client->ps;
	}

	ps.eFlags |= EF_DUMMY_PMOVE;

	memset( move, 0, sizeof( *move ) );
	move->stopevent = PREDICTSTOP_NONE;
	if ( numframes <= 0 || frametime <= 0 ) {
		VectorCopy( ps.origin, move->endpos );
		VectorCopy( ps.velocity, move->velocity );
		move->groundEntityNum = ps.groundEntityNum;
		return;
	}

	if ( checkHitEnt >= 0 && !Q_stricmp( g_entities[checkHitEnt].classname, "ai_marker" ) ) {
		checkReachMarker = qtrue;
		VectorSubtract( g_entities[checkHitEnt].r.currentOrigin, ps.origin, startHitVec );
		VectorCopy( ps.origin, lastOrg );
	} else {
		checkReachMarker = qfalse;
	}

	// don't let the frametime be too high
	//	while (frametime > 0.2) {
	//		numframes *= 2;
	//		frametime /= 2;
	//	}

	for ( frame = 0; frame < numframes; frame++ ) {
		memset( &pm, 0, sizeof( pm ) );
		pm.ps = &ps;
		pm.cmd = *ucmd;
		pm.oldcmd = *ucmd;
		pm.ps->commandTime = 0;
		pm.cmd.serverTime = (int)( 1000.0 * frametime );
		pm.tracemask = g_entities[cs->entityNum].clipmask; //MASK_PLAYERSOLID;

		pm.trace = trap_TraceCapsule; //trap_Trace;
		pm.pointcontents = trap_PointContents;
		pm.debugLevel = qfalse;
		pm.noFootsteps = qtrue;
		// RF, not needed for prediction
		//pm.noWeapClips = qtrue;	// (SA) AI's ignore weapon clips

		// perform a pmove
		Pmove( &pm );

		if ( checkHitEnt >= 0 ) {
			// if we've hit the checkent, abort
			if ( checkReachMarker ) {
				VectorSubtract( g_entities[checkHitEnt].r.currentOrigin, pm.ps->origin, thisHitVec );
				if ( DotProduct( startHitVec, thisHitVec ) < 0 ) {
					// project the marker onto the movement vec, and check distance
					ProjectPointOntoVector( g_entities[checkHitEnt].r.currentOrigin, lastOrg, pm.ps->origin,
											projPoint );
					if ( VectorDistance( g_entities[checkHitEnt].r.currentOrigin, projPoint ) < 8 ) {
						move->stopevent = PREDICTSTOP_HITENT;
						goto done;
					}
				}
				// use this position as the base for the next test
				//VectorCopy( thisHitVec, startHitVec );
				VectorCopy( pm.ps->origin, lastOrg );
			}
			// if we didnt reach the marker, then check for something that blocked us
			for ( i = 0; i < pm.numtouch; i++ ) {
				if ( pm.touchents[i] == pm.ps->groundEntityNum ) {
					continue;
				}
				if ( pm.touchents[i] == checkHitEnt ) {
					move->stopevent = PREDICTSTOP_HITENT;
					goto done;
				} else if ( pm.touchents[i] < MAX_CLIENTS ||
							( pm.touchents[i] != ENTITYNUM_WORLD &&
							  ( g_entities[pm.touchents[i]].s.eType != ET_MOVER ||
								g_entities[pm.touchents[i]].moverState != MOVER_POS1 ) ) ) {
					// we have hit another entity, so abort
					move->stopevent = PREDICTSTOP_HITCLIENT;
					goto done;
				} else if ( !Q_stricmp( g_entities[pm.touchents[i]].classname, "script_mover" ) ) {
					// avoid script_mover's
					move->stopevent = PREDICTSTOP_HITCLIENT;
					goto done;
				}
			}
			// if we are trying to get to something, we should keep adjusting our input to head towards them
			if ( cs->clientState && checkHitEnt >= 0 ) {
				// keep walking straight to them
				VectorSubtract( g_entities[checkHitEnt].r.currentOrigin, pm.ps->origin, bi.dir );
				if ( !ent->waterlevel ) {
					bi.dir[2] = 0;
				}
				VectorNormalize( bi.dir );
				bi.actionflags = 0;
				bi.speed = 400;
				AICast_InputToUserCommand( cs, &bi, ucmd, ps.delta_angles );
			}
		}
	}

done:

	// hack, if we are above ground, chances are it's because we only did one frame, and gravity isn't applied until
	// after the frame, so try and drop us down some
	if ( pm.ps->groundEntityNum == ENTITYNUM_NONE ) {
		VectorCopy( pm.ps->origin, end );
		end[2] -= 32;
		trap_Trace( &tr, pm.ps->origin, pm.mins, pm.maxs, end, pm.ps->clientNum, pm.tracemask );
		if ( !tr.startsolid && !tr.allsolid && tr.fraction < 1 ) {
			VectorCopy( tr.endpos, pm.ps->origin );
			pm.ps->groundEntityNum = tr.entityNum;
		}
	}

	// copy off the results
	VectorCopy( pm.ps->origin, move->endpos );
	move->frames = frame;
	VectorCopy( pm.ps->velocity, move->velocity );
	move->numtouch = pm.numtouch;
	memcpy( move->touchents, pm.touchents, sizeof( pm.touchents ) );
	move->groundEntityNum = pm.ps->groundEntityNum;

	//G_Printf("PredictMovement: %i ms\n", -pretime + Sys_MilliSeconds() );
}

/*
============
AICast_GetAvoid
============
*/
qboolean AICast_GetAvoid( cast_state_t *cs, bot_goal_t *goal, vec3_t outpos, qboolean reverse, int blockEnt ) {
	float yaw, oldyaw, distmoved, bestmoved, bestyaw;
	vec3_t bestpos;
	aicast_predictmove_t castmove;
	usercmd_t ucmd;
	qboolean enemyVisible;
	float angleDiff;
	int starttraveltime = 0, besttraveltime, traveltime = 0; // TTimo: init
	int invert;
	float inc;
	qboolean averting = qfalse;
	float maxYaw, simTime;
	static int lastTime;
	//
	// if we are in the air, no chance of avoiding
	if ( cs->clientState->cur_ps.groundEntityNum == ENTITYNUM_NONE && g_entities[cs->entityNum].waterlevel <= 1 ) {
		return qfalse;
	}
	//
	if ( cs->navigation.lastAvoid > level.time - rand() % 500 ) {
		return qfalse;
	}
	cs->navigation.lastAvoid = level.time + 50 + rand() % 500;
	//
	if ( lastTime == level.time ) {
		return qfalse;
	}
	lastTime = level.time;

	// if they have an enemy, and can currently see them, don't move out of their view
	enemyVisible = ( cs->enemyNum >= 0 ) && ( AICast_CheckAttack( cs, cs->enemyNum, qfalse ) );
	//
	// look for a good direction to move out of the way
	bestmoved = 0;
	bestyaw = 360;
	besttraveltime = 9999999;
	if ( goal ) {
		starttraveltime = trap_AAS_AreaTravelTimeToGoalArea( cs->clientState->areanum, cs->clientState->origin,
															 goal->areanum, cs->travelflags );
	}
	memcpy( &ucmd, &cs->lastucmd, sizeof( usercmd_t ) );
	ucmd.forwardmove = 127;
	ucmd.rightmove = 0;
	ucmd.upmove = 0;
	if ( cs->dangerEntity >= 0 && cs->dangerEntityValidTime >= level.time ) {
		averting = qtrue;
	} else if ( !goal ) {
		averting = qtrue; // not heading for a goal, so we must be getting out of someone's way
	}
	//
	maxYaw = 0;
	simTime = 1.2;
	//
	if ( averting ) {
		// avoiding danger, go anywhere!
		angleDiff = 300;
		inc = 60;
		invert = 1;
	} else {
		if ( level.time % 1000 < 500 ) {
			invert = 1;
		} else {
			invert = -1;
		}
		angleDiff = 140;
		inc = 35;
	}
	if ( blockEnt > aicast_maxclients ) {
		maxYaw = angleDiff;
		simTime = 0.5;
	}
	//
	for ( yaw = -angleDiff * invert; yaw * invert <= maxYaw; yaw += inc * invert ) {
		if ( !averting && !yaw ) {
			continue;
		}
		oldyaw = cs->clientState->cur_ps.viewangles[YAW];
		cs->clientState->cur_ps.viewangles[YAW] += yaw + reverse * 180;
		//
		ucmd.angles[YAW] = ANGLE2SHORT( AngleMod( cs->clientState->cur_ps.viewangles[YAW] ) );
		//
		AICast_PredictMovement( cs, 5, 0.4, &castmove, &ucmd, -1 );
		// if we have a danger entity, try and get away from it at all costs
		if ( cs->dangerEntity >= 0 && cs->dangerEntityValidTime >= level.time ) {
			distmoved = Distance( castmove.endpos, cs->dangerEntityPos );
		} else if ( goal ) {
			//distmoved = 99999 - trap_AAS_AreaTravelTimeToGoalArea( AI_PointAreaNum(castmove.endpos), castmove.endpos, goal->areanum, cs->travelflags );
			distmoved = 99999 - Distance( castmove.endpos, goal->origin );
		} else {
			distmoved = Distance( castmove.endpos, cs->clientState->cur_ps.origin );
		}
		if ( ( distmoved > bestmoved )
			 //&&	((cs->clientState->origin[2] - castmove.endpos[2]) < 64)	// allow up, but not down (falling)
			 && ( castmove.groundEntityNum != ENTITYNUM_NONE ) ) {
			// they all passed, check any other stuff
			if ( !enemyVisible ||
				 AICast_CheckAttackAtPos( cs->entityNum, cs->enemyNum, castmove.endpos, qfalse, qfalse ) ) {
				if ( !goal || ( traveltime = trap_AAS_AreaTravelTimeToGoalArea(
									AI_PointAreaNum( castmove.endpos ), castmove.endpos, goal->areanum,
									cs->travelflags ) ) < ( starttraveltime + 200 ) ) {
					bestyaw = yaw;
					bestmoved = distmoved;
					besttraveltime = traveltime;
					VectorCopy( castmove.endpos, bestpos );
				}
			}
		}
		//
		cs->clientState->cur_ps.viewangles[YAW] = oldyaw;
	}
	//
	if ( bestmoved > 0 ) {
		VectorCopy( bestpos, outpos );
		return qtrue;
	} else {
		return qfalse;
	}

	//G_Printf("GetAvoid: %i ms\n", -pretime + Sys_MilliSeconds() );
}

/*
============
AICast_Blocked
============
*/
void AICast_Blocked( cast_state_t *cs, bot_moveresult_t *moveresult, int activate, bot_goal_t *goal ) {
	vec3_t pos, dir;
	aicast_predictmove_t move;
	usercmd_t ucmd;
	bot_input_t bi;
	cast_state_t *ocs;
	int i, blockEnt = -1;
	bot_goal_t ogoal;

	if ( cs->navigation.blockedAvoidTime < level.time ) {
		if ( cs->navigation.blockedAvoidTime < level.time - 300 ) {
			if ( VectorCompare( cs->clientState->cur_ps.velocity, vec3_origin ) && !cs->lastucmd.forwardmove &&
				 !cs->lastucmd.rightmove ) {
				// not moving, don't bother checking
				cs->navigation.blockedAvoidTime = level.time - 1;
				return;
			}
			// are we going to hit someone soon?
			trap_EA_GetInput( cs->entityNum, (float)level.time / 1000, &bi );
			AICast_InputToUserCommand( cs, &bi, &ucmd, cs->clientState->cur_ps.delta_angles );
			AICast_PredictMovement( cs, 1, 0.6, &move, &ucmd,
									( goal && goal->entitynum > -1 ) ? goal->entitynum : cs->entityNum );

			// blocked if we hit a client (or non-stationary mover) other than our enemy or goal
			if ( move.stopevent != PREDICTSTOP_HITCLIENT ) {
				// not blocked
				cs->navigation.blockedAvoidTime = level.time - 1;
				return;
			}

			// if we stopped passed our goal, ignore it
			if ( goal ) {
				if ( VectorDistance( cs->clientState->origin, goal->origin ) <
					 VectorDistance( cs->clientState->origin, move.endpos ) ) {
					vec3_t v1, v2;
					VectorSubtract( goal->origin, cs->clientState->origin, v1 );
					VectorSubtract( goal->origin, move.endpos, v2 );
					VectorNormalize( v1 );
					VectorNormalize( v2 );
					if ( DotProduct( v1, v2 ) < 0 ) {
						// we went passed the goal, so assume we can reach it
						cs->navigation.blockedAvoidTime = level.time - 1;
						return;
					}
				}
			}

			// try and get them to move, in case we can't get around them
			blockEnt = -1;
			for ( i = 0; i < move.numtouch; i++ ) {
				if ( move.touchents[i] >= MAX_CLIENTS ) {
					if ( !Q_stricmp( g_entities[move.touchents[i]].classname, "script_mover" ) ) {
						// avoid script_mover's
						blockEnt = move.touchents[i];
					}
					// if we are close to the impact point, then avoid this entity
					else if ( VectorDistance( cs->clientState->origin, move.endpos ) < 10 ) {
						//G_Printf("AI (%s) avoiding %s\n", g_entities[cs->entityNum].aiName, g_entities[move.touchents[i]].classname );
						blockEnt = move.touchents[i];
					}
					continue;
				}
				//
				ocs = AICast_GetCastState( move.touchents[i] );
				if ( !ocs->clientState ) {
					blockEnt = move.touchents[i];
				}
				// reject this blocker if we are following or going to them
				else if ( cs->navigation.followEntity != ocs->entityNum ) {
					// if they are moving away from us already, let them go
					if ( VectorLength( ocs->clientState->cur_ps.velocity ) > 10 ) {
						vec3_t v1, v2;

						VectorSubtract( ocs->clientState->origin, cs->clientState->origin, v2 );
						VectorNormalize( v2 );
						VectorNormalize2( ocs->clientState->cur_ps.velocity, v1 );

						if ( DotProduct( v1, v2 ) > 0.0 ) {
							continue;
						}
					}
					//
					// if they recently were asked to avoid us, then they're probably not listening
					if ( ocs->navigation.obstructingTime > level.time - 500 ) {
						blockEnt = move.touchents[i];
					}
					//
					// if they are not avoiding, ignore
					if ( !( ocs->aiFlags & AIFL_NOAVOID ) ) {
						continue;
					}
					//
					// they should avoid us
					if ( ocs->leaderNum >= 0 ) {
						ogoal.entitynum = ocs->leaderNum;
						VectorCopy( g_entities[ocs->leaderNum].r.currentOrigin, ogoal.origin );
						if ( AICast_GetAvoid( ocs, &ogoal, ocs->navigation.obstructingPos, qfalse, cs->entityNum ) ) {
							// give them time to move somewhere else
							ocs->navigation.obstructingTime = level.time + 1000;
						} else {
							// make sure they don't call GetAvoid() for another few frames to let others avoid also
							ocs->navigation.obstructingTime = level.time - 1;
							blockEnt = move.touchents[i];
						}
					} else {
						if ( AICast_GetAvoid( ocs, NULL, ocs->navigation.obstructingPos, qfalse, cs->entityNum ) ) {
							// give them time to move somewhere else
							ocs->navigation.obstructingTime = level.time + 1000;
						} else {
							// make sure they don't call GetAvoid() for another few frames to let others avoid also
							ocs->navigation.obstructingTime = level.time - 1;
							blockEnt = move.touchents[i];
						}
					}
				}
			}

		} else {
			return;
		}

		if ( blockEnt < 0 ) {
			// nothing found to be worth avoding
			cs->navigation.blockedAvoidTime = level.time - 1;
			return;
		}

		// something is blocking our path
		if ( g_entities[blockEnt].aiName && g_entities[blockEnt].client && cs->clientState &&
			 VectorDistance( cs->clientState->origin, g_entities[blockEnt].r.currentOrigin ) < 128 ) {
			int oldId = cs->script.castScriptStatus.scriptId;
			AICast_ScriptEvent( cs, "blocked", g_entities[blockEnt].aiName );
			if ( ( oldId != cs->script.castScriptStatus.scriptId ) || ( cs->aiFlags & AIFL_DENYACTION ) ) {
				// the script has changed, so assume the scripting is handling the avoidance
				return;
			}
		}

		// avoid geometry and props, but assume clients will get out the way
		if ( /*blockEnt > MAX_CLIENTS &&*/ AICast_GetAvoid( cs, goal, pos, qfalse, blockEnt ) ) {
			VectorSubtract( pos, cs->clientState->cur_ps.origin, dir );
			VectorNormalize( dir );
			cs->navigation.blockedAvoidYaw = vectoyaw( dir );
			if ( blockEnt >= MAX_CLIENTS ) {
				cs->navigation.blockedAvoidTime = level.time + 100 + rand() % 200;
			} else {
				cs->navigation.blockedAvoidTime = level.time + 300 + rand() % 400;
			}
		} else {
			cs->navigation.blockedAvoidTime = level.time - 1; // don't look again for another few frames
			return;
		}
	}

	VectorClear( pos );
	pos[YAW] = cs->navigation.blockedAvoidYaw;
	AngleVectors( pos, dir, NULL, NULL );

	if ( moveresult->flags & MOVERESULT_ONTOPOFOBSTACLE ) {
		trap_EA_Jump( cs->clientState->entitynum );
	}

	trap_EA_Move( cs->clientState->entitynum, dir, 200 ); //400);

	vectoangles( dir, cs->ideal_viewangles );
	cs->ideal_viewangles[2] *= 0.5;
}

/*
================
AICast_EvaluatePmove

  Avoidance after the event (leaders instruct AI's to get out the way, AI's instruct other non-moving AI's to get out the way)
================
*/
void AICast_EvaluatePmove( int clientnum, pmove_t *pm ) {
	cast_state_t *cs, *ocs;
	int i, ent;
	bot_goal_t ogoal;

	//vec3_t pos, dir;
	cs = AICast_GetCastState( clientnum );
	// make sure we are using the right AAS data for this entity (one's that don't get set will default to the player's AAS data)
	trap_AAS_SetCurrentWorld( cs->aasWorldIndex );

	// NOTE: this is only enabled for real clients, so their followers get out of their way
	//if (cs->clientState)
	//	return;

	// look through the touchent's to see if we've bumped into something we should avoid, or react to
	for ( i = 0; i < pm->numtouch; i++ ) {
		// mark the time, so they can deal with the obstruction in their own think functions
		cs->navigation.blockedTime = level.time;

		if ( pm->touchents[i] == pm->ps->groundEntityNum ) {
			continue;
		}

		// if they are an AI Cast, inform them of our disposition, and hope that they are reasonable
		// enough to assist us in our desire to move beyond our current position
		if ( pm->touchents[i] < aicast_maxclients ) {
			if ( !AICast_EntityVisible( cs, pm->touchents[i], qtrue ) ) {
				continue;
			}

			// if we are inspecting the body, abort if we touch anything
			if ( cs->clientState && cs->enemyNum >= 0 && g_entities[cs->enemyNum].health <= 0 ) {
				cs->enemyNum = -1;
			}

			// anything we touch, should see us
			AICast_UpdateVisibility( &g_entities[pm->touchents[i]], &g_entities[cs->entityNum], qfalse, qtrue );

			ocs = AICast_GetCastState( pm->touchents[i] );
			if ( ( ocs->clientState ) && ( AICast_SameTeam( cs, ocs->entityNum ) ) &&
				 ( !( ocs->aiFlags & AIFL_NOAVOID ) ) &&
				 ( ( ocs->leaderNum == cs->entityNum ) || ( VectorLength( ocs->clientState->velocity ) < 5 ) ) &&
				 ( ocs->navigation.obstructingTime < ( level.time + 100 ) ) ) {
				// if they are moving away from us already, let them go
				if ( VectorLength( ocs->clientState->cur_ps.velocity ) > 10 ) {
					vec3_t v1, v2;

					VectorSubtract( ocs->clientState->origin, g_entities[clientnum].client->ps.velocity, v2 );
					VectorNormalize( v2 );
					VectorNormalize2( ocs->clientState->cur_ps.velocity, v1 );

					if ( DotProduct( v1, v2 ) > 0.0 ) {
						continue;
					}
				}
				if ( ocs->leaderNum >= 0 ) {
					VectorCopy( g_entities[ocs->leaderNum].r.currentOrigin, ogoal.origin );
					ogoal.areanum = AI_PointAreaNum( ogoal.origin );
					ogoal.entitynum = ocs->leaderNum;
					if ( ocs->clientState &&
						 AICast_GetAvoid( ocs, &ogoal, ocs->navigation.obstructingPos, qfalse,
										  cs->entityNum ) ) { // give them time to move somewhere else
						ocs->navigation.obstructingTime = level.time + 1000;
					}
				} else {
					if ( ocs->clientState &&
						 AICast_GetAvoid( ocs, NULL, ocs->navigation.obstructingPos, qfalse,
										  cs->entityNum ) ) { // give them time to move somewhere else
						ocs->navigation.obstructingTime = level.time + 1000;
					}
				}
			}
		} else if ( cs->clientState ) {
			// if we are blocked by a brush entity, see if we can activate it
			ent = pm->touchents[i];
			if ( g_entities[ent].s.modelindex > 0 && g_entities[ent].s.eType == ET_MOVER ) {
				//find the bsp entity which should be activated in order to remove
				//the blocking entity

				if ( !g_entities[ent].isProp && Q_stricmp( g_entities[ent].classname, "func_static" ) &&
					 Q_stricmp( g_entities[ent].classname, "func_button" ) &&
					 Q_stricmp( g_entities[ent].classname, "func_tram" ) ) {
					G_Activate( &g_entities[ent], &g_entities[cs->entityNum] );
				}
			}
		}
	}
}

/*
==============
AICast_RequestCrouchAttack
==============
*/
qboolean AICast_RequestCrouchAttack( cast_state_t *cs, vec3_t org, float time ) {
	if ( cs->attributes[ATTACK_CROUCH] > 0 &&
		 AICast_CheckAttackAtPos( cs->entityNum, cs->enemyNum, org, qtrue, qfalse ) ) {
		if ( time ) {
			cs->attackcrouch_time = level.time + (int)( time * 1000 );
		}
		return qtrue;
	}

	return qfalse;
}

/*
================
AICast_DeadClipWalls
================
*/
void AICast_DeadClipWalls( cast_state_t *cs ) {
	/*
	//animation_t *anim;
	orientation_t or;
	vec3_t	src, vel;
	trace_t	tr;

	// get the death animation we are currently playing
	//anim = BG_GetAnimationForIndex( cs->entityNum, (cs->clientState->cur_ps.torsoAnim & ~ANIM_TOGGLEBIT) );

	// find the head position
	trap_GetTag( cs->entityNum, "tag_head", &or );
	// move up a tad
	or.origin[2] += 3;

	// trace from the base of our bounding box, to the head
	VectorCopy( cs->clientState->origin, src );
	src[2] -= cs->clientState->cur_ps.mins[2] + 3;
	trap_Trace( &tr, src, vec3_origin, vec3_origin, or.origin, cs->entityNum, MASK_SOLID );

	// if we hit something, move away from it
	if (!tr.startsolid && !tr.allsolid && tr.fraction < 1.0) {
		VectorScale( tr.plane.normal, 80, vel );
		vel[2] = 0;
		VectorAdd( g_entities[cs->entityNum].client->ps.velocity, vel, g_entities[cs->entityNum].client->ps.velocity );
	}
*/
}
