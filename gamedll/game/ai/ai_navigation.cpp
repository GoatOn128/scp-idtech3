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
AICast_GetRandomViewAngle()
============
*/
float AICast_GetRandomViewAngle( cast_state_t *cs, float tracedist ) {
	int cnt, passent, contents_mask;
	vec3_t vec, dir, start, end;
	trace_t trace;
	float bestdist, bestyaw;

	cnt = 0;
	VectorClear( vec );
	//
	VectorCopy( cs->clientState->origin, start );
	start[2] += cs->clientState->cur_ps.viewheight;
	//
	passent = cs->entityNum;
	contents_mask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_WATER | CONTENTS_SLIME;
	//	contents_mask = CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WATER;
	bestdist = 0;
	bestyaw = 0;
	//
	while ( cnt++ < 4 ) {
		vec[YAW] = random() * 360.0;
		//
		AngleVectors( vec, dir, NULL, NULL );
		VectorMA( start, tracedist, dir, end );
		//
		trap_Trace( &trace, start, NULL, NULL, end, passent, contents_mask );
		//
		if ( trace.fraction >= 1 ) {
			return vec[YAW];
		} else if ( !bestdist || bestdist < trace.fraction ) {
			bestdist = trace.fraction;
			bestyaw = vec[YAW];
		}
	}
	//
	if ( bestdist ) {
		return bestyaw;
	}
	// just return their current direction
	return cs->ideal_viewangles[YAW];
}

/*
============
AICast_MoveToPos()

  returns a pointer to the moveresult it used to make the move, so we can investigate it
  outside of this function
============
*/
bot_moveresult_t *AICast_MoveToPos( cast_state_t *cs, vec3_t pos, int entnum ) {
	bot_moveresult_t *moveresult;
	bot_goal_t goal;
	vec3_t /*target,*/ dir;
	bot_moveresult_t &lmoveresult = cs->navigation.lastMoveResult;
	int tfl;
	ai_client_state_t *bs;
	float dist;

	//int pretime = Sys_MilliSeconds();

	moveresult = NULL;

	if ( cs->script.castScriptStatus.scriptNoMoveTime > level.time ) {
		return NULL;
	}
	if ( cs->pauseTime > level.time ) {
		return NULL;
	}
	//
	bs = cs->clientState;
	tfl = cs->travelflags;
	//if in lava or slime the bot should be able to get out
	if ( AI_InLava( bs ) ) {
		tfl |= TFL_LAVA;
	}
	if ( AI_InSlime( bs ) ) {
		tfl |= TFL_SLIME;
	}
	//
	//create the chase goal
	memset( &goal, 0, sizeof( goal ) );
	goal.entitynum = entnum;
	if ( entnum >= 0 && entnum < level.maxclients &&
		 caststates[entnum].navigation.lastValidAreaTime[cs->aasWorldIndex] > level.time - 100 ) {
		goal.areanum = caststates[entnum].navigation.lastValidAreaNum[cs->aasWorldIndex];
	} else {
		goal.areanum = AI_PointAreaNum( pos );
		if ( entnum >= 0 && entnum < level.maxclients ) {
			if ( !goal.areanum ) {
				// use the last valid area
				goal.areanum = caststates[entnum].navigation.lastValidAreaNum[cs->aasWorldIndex];
			}
		}
	}
	VectorCopy( pos, goal.origin );
	VectorSet( goal.mins, -8, -8, -8 );
	VectorSet( goal.maxs, 8, 8, 8 );
	if ( entnum > -1 && entnum == cs->navigation.followEntity && !cs->navigation.followSlowApproach ) {
		goal.flags |= GFL_NOSLOWAPPROACH; // just speed right passed it
	}
	//
	// debugging, show the route
	if ( aicast_debug.integer == 2 && ( g_entities[cs->entityNum].aiName &&
										!strcmp( aicast_debugname.string, g_entities[cs->entityNum].aiName ) ) ) {
		trap_AAS_RT_ShowRoute( cs->clientState->origin, cs->clientState->areanum, goal.areanum );
	}
	//
	//initialize the movement state
	AI_SetupMovement( cs );
	//if this is a slow moving creature, don't use avoidreach
	if ( cs->attributes[RUNNING_SPEED] < 100 ) {
		//reset the avoid reach, otherwise bot is stuck in current area
		trap_BotResetAvoidReach( bs->ms );
	} else if ( !VectorCompare( cs->navigation.lastMoveToPosGoalOrg, pos ) ) {
		//reset the avoid reach, otherwise bot is stuck in current area
		trap_BotResetAvoidReach( bs->ms );
		VectorCopy( pos, cs->navigation.lastMoveToPosGoalOrg );
	}
	//move towards the goal
	if ( !( cs->aiFlags & AIFL_EXPLICIT_ROUTING ) || ( entnum < 0 ) ||
		 Q_strcasecmp( g_entities[entnum].classname, "ai_marker" ) ) {
		// use AAS routing
		trap_BotMoveToGoal( &lmoveresult, bs->ms, &goal, tfl );
		//if the movement failed
		if ( lmoveresult.failure ) {

			//reset the avoid reach, otherwise bot is stuck in current area
			trap_BotResetAvoidReach( bs->ms );

			// clear all movement
			trap_EA_Move( cs->entityNum, vec3_origin, 0 );

		} else {

			if ( entnum > 0 && goal.areanum && entnum >= 0 &&
				 entnum < level.maxclients ) { // NOTE: dont do this for the player
				// save this destination point
				caststates[entnum].navigation.lastValidAreaNum[cs->aasWorldIndex] = goal.areanum;
				caststates[entnum].navigation.lastValidAreaTime[cs->aasWorldIndex] = level.time;
			}

			if ( lmoveresult.flags & ( MOVERESULT_MOVEMENTVIEW | MOVERESULT_SWIMVIEW ) ) {
				VectorCopy( lmoveresult.ideal_viewangles, cs->ideal_viewangles );
				VectorCopy( cs->ideal_viewangles, cs->viewlock_viewangles );
				cs->aiFlags |= AIFL_VIEWLOCKED;
			} else if ( !( cs->bFlags & AI_FRAME_ATTACKED ) ) { // if we are attacking, don't change angles
				bot_input_t bi;

				trap_EA_GetInput( bs->client, 0.1, &bi );
				if ( VectorLength( lmoveresult.movedir ) < 0.5 ) {
					VectorSubtract( goal.origin, bs->origin, dir );
					vectoangles( dir, cs->ideal_viewangles );
				} else {
					// use our velocity if we are moving
					if ( VectorNormalize2( cs->clientState->cur_ps.velocity, dir ) > 1 ) {
						vectoangles( dir, cs->ideal_viewangles );
					} else {
						vectoangles( lmoveresult.movedir, cs->ideal_viewangles );
					}
				}
				cs->ideal_viewangles[2] *= 0.5;
				// look towards our future direction (like looking around a corner as we approach it)
				if ( !( cs->aiFlags & AIFL_WALKFORWARD ) && ( lmoveresult.flags & MOVERESULT_FUTUREVIEW ) ) {
					if ( AI_AngleDifference( cs->ideal_viewangles[1], lmoveresult.ideal_viewangles[1] ) > 45 ) {
						cs->ideal_viewangles[1] -= 45;
					} else if ( AI_AngleDifference( cs->ideal_viewangles[1], lmoveresult.ideal_viewangles[1] ) < -45 ) {
						cs->ideal_viewangles[1] += 45;
					} else {
						cs->ideal_viewangles[1] = lmoveresult.ideal_viewangles[1];
					}
					cs->ideal_viewangles[1] = AngleNormalize360( cs->ideal_viewangles[1] );
					cs->ideal_viewangles[0] = lmoveresult.ideal_viewangles[0];
					cs->ideal_viewangles[0] = 0.5 * AngleNormalize180( cs->ideal_viewangles[0] );
				}
			}
		}

	} else { // manual routing towards markers

		VectorSubtract( pos, cs->clientState->origin, dir );
		if ( ( dist = VectorNormalize( dir ) ) < 64 ) {
			trap_EA_Move( cs->entityNum, dir, 100.0 + 300.0 * ( dist / 64.0 ) );
		} else {
			trap_EA_Move( cs->entityNum, dir, 400 );
		}

		// look towards the marker also
		vectoangles( dir, cs->ideal_viewangles );
		cs->ideal_viewangles[2] *= 0.5;
	}
	// this must go last so we face the direction we avoid move
	AICast_Blocked( cs, &lmoveresult, qfalse, &goal );

	//G_Printf("MoveToPos: %i ms\n", -pretime + Sys_MilliSeconds() );
	/*
// debug, print movement info
if(0) // (SA) added to hide the print
{
bot_input_t bi;

trap_EA_GetInput(cs->clientState->client, (float) level.time / 1000, &bi);
G_Printf("spd: %i\n", (int)bi.speed );
}
*/
	return ( moveresult = &lmoveresult );
}

/*
============
AICast_SpeedScaleForDistance()
============
*/
float AICast_SpeedScaleForDistance( cast_state_t *cs, float startdist, float idealDist ) {
#define PREDICT_TIME_WALK 0.2
#define PREDICT_TIME_CROUCH 0.2
#define PREDICT_TIME_RUN 0.3
	float speed, dist;

	dist = startdist - idealDist;
	if ( dist < 1 ) {
		dist = 1;
	}

	// if walking
	if ( cs->movestate == MS_WALK ) {
		speed = cs->attributes[WALKING_SPEED];
		if ( speed * PREDICT_TIME_WALK > dist ) {
			return 0.2 + 0.8 * ( dist / ( speed * PREDICT_TIME_WALK ) );
		} else {
			return 1.0;
		}
	} else
		// if crouching
		if ( cs->movestate == MS_CROUCH || cs->attackcrouch_time > level.time ) {
			speed = cs->attributes[CROUCHING_SPEED];
			if ( speed * PREDICT_TIME_CROUCH > dist ) {
				return 0.3 + 0.7 * ( dist / ( speed * PREDICT_TIME_CROUCH ) );
			} else {
				return 1.0;
			}
		} else
		// running
		{
			speed = cs->attributes[RUNNING_SPEED];
			if ( speed * PREDICT_TIME_RUN > dist ) {
				return 0.2 + 0.8 * ( dist / ( speed * PREDICT_TIME_RUN ) );
			} else {
				return 1.0;
			}
		}
}

/*
==================
AICast_CombatMove
==================
*/
bot_moveresult_t AICast_CombatMove( cast_state_t *cs, int tfl ) {
	ai_client_state_t *bs;
	float attack_skill, croucher, dist;
	vec3_t forward, backward; //, up = {0, 0, 1};
	bot_moveresult_t moveresult;
	bot_goal_t goal;

	bs = cs->clientState;

	//get the enemy entity info
	memset( &moveresult, 0, sizeof( bot_moveresult_t ) );
	//
	attack_skill = cs->attributes[ATTACK_SKILL];
	croucher = ( cs->attributes[ATTACK_CROUCH] > 0.1 );

	//initialize the movement state
	AI_SetupMovement( cs );
	//direction towards the enemy
	VectorSubtract( cs->perception.vislist[cs->enemyNum].visible_pos, bs->origin, forward );
	//the distance towards the enemy
	dist = VectorNormalize( forward );
	VectorNegate( forward, backward );
	//
	// do we have somewhere we are trying to get to?
	if ( cs->navigation.combatGoalTime > level.time ) {
		if ( VectorLength( cs->navigation.combatGoalOrigin ) > 1 ) {
			//create the chase goal
			goal.areanum = AI_PointAreaNum( cs->navigation.combatGoalOrigin );
			VectorCopy( cs->navigation.combatGoalOrigin, goal.origin );

			// if we are really close, stop going for it
			// FIXME: a better way of doing this, so we don't stop short of the goal?
			if ( ( dist = Distance( goal.origin, cs->clientState->origin ) ) < 32 ) {
				if ( cs->navigation.combatGoalTime > level.time + 3000 ) {
					cs->navigation.combatGoalTime = level.time + 2000 + rand() % 1000;
					cs->combat.combatSpotDelayTime = level.time + 4000 + rand() % 3000;
				}
				VectorClear( cs->navigation.combatGoalOrigin );
			} else {
				aicast_predictmove_t move;
				//
				AICast_MoveToPos( cs, cs->navigation.combatGoalOrigin, -1 );
				cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, 32 );
				//
				// if we are going to move out of view very soon, stop moving
				AICast_PredictMovement( cs, 1, 0.8, &move, &cs->lastucmd, -1 );
				//
				if ( move.numtouch ||
					 !AICast_CheckAttackAtPos( cs->entityNum, cs->enemyNum, move.endpos, qfalse, qfalse ) ) {
					// abort the manouver, reached a good spot
					cs->navigation.combatGoalTime = 0;
					cs->combat.combatSpotAttackCount = cs->combat.startAttackCount;
				}
			}

			// if we are there, and the enemy can see us, but we cant hit them, abort immediately
		} else if ( !AICast_CheckAttack( cs, cs->enemyNum, qfalse ) &&
					AICast_VisibleFromPos( cs->perception.vislist[cs->enemyNum].visible_pos, cs->enemyNum,
										   cs->clientState->origin, cs->entityNum, qfalse ) ) {
			cs->navigation.combatGoalTime = 0;
			cs->combat.combatSpotAttackCount = cs->combat.startAttackCount;
		}
	} else { // look for a good position to move to?
		if ( ( ( cs->attributes[CAMPER] < random() ) && ( cs->combat.takeCoverTime < level.time ) &&
			   ( cs->combat.combatSpotAttackCount < cs->combat.startAttackCount ) &&
			   ( cs->combat.combatSpotDelayTime < level.time ) ) ) {

			if ( ( cs->attributes[TACTICAL] > 0.3 + random() * 0.5 ) &&
				 trap_AAS_RT_GetHidePos( cs->clientState->origin, cs->clientState->entitynum, cs->clientState->areanum,
										 cs->perception.vislist[cs->enemyNum].visible_pos, cs->enemyNum,
										 AI_PointAreaNum( cs->perception.vislist[cs->enemyNum].visible_pos ),
										 cs->navigation.combatGoalOrigin ) ) {
				cs->navigation.combatGoalTime = level.time + 10000; // give us plenty of time to get there
				//cs->combat.combatSpotAttackCount = cs->combat.startAttackCount + 3;	// don't keep moving around to different positions on our own
				cs->combat.combatSpotDelayTime = level.time + 3000 + rand() % 3000;
			} else {
				// don't look again until we've moved
				//cs->combat.combatSpotAttackCount = cs->combat.startAttackCount;
				cs->combat.combatSpotDelayTime = level.time + 3000 + rand() % 3000;
			}
		}
	}
	//
	return moveresult;
}

/*
==============
AICast_GetTakeCoverPos
==============
*/
qboolean AICast_GetTakeCoverPos( cast_state_t *cs, int enemyNum, vec3_t enemyPos, vec3_t returnPos ) {
	cs->crouchHideFlag = qfalse;
	//
	if ( cs->script.castScriptStatus.scriptNoMoveTime > level.time ) {
		return qfalse;
	}
	//
	cs->combat.lastGetHidePos = level.time;
	//
	// can we just crouch?
	if ( ( cs->attackcrouch_time < level.time ) && ( enemyNum < aicast_maxclients ) &&
		 AICast_CheckAttackAtPos( cs->entityNum, enemyNum, cs->clientState->origin, qfalse, qfalse ) &&
		 !AICast_CheckAttackAtPos( cs->entityNum, enemyNum, cs->clientState->origin, qtrue, qfalse ) ) {

		// do a more thorough check to see if the enemy can see us if we crouch
		vec3_t omaxs;
		gentity_t *ent;
		qboolean visible;

		ent = &g_entities[cs->entityNum];
		VectorCopy( ent->r.maxs, omaxs );
		ent->r.maxs[2] = ent->client->ps.crouchMaxZ + 4; // + 4 to be safe

		visible = AICast_VisibleFromPos( g_entities[enemyNum].r.currentOrigin, enemyNum, cs->clientState->origin,
										 cs->entityNum, qfalse );

		ent->r.maxs[2] = omaxs[2];

		if ( !visible ) {
			VectorCopy( enemyPos, cs->combat.takeCoverEnemyPos );
			VectorCopy( cs->clientState->origin, returnPos );
			cs->crouchHideFlag = qtrue;
			return qtrue;
		}
	}
	// if we are in a void, then we can't hide
	// look for a hiding spot
	if ( cs->clientState->areanum &&
		 trap_AAS_RT_GetHidePos( cs->clientState->origin, cs->clientState->entitynum, cs->clientState->areanum,
								 enemyPos, enemyNum, AI_PointAreaNum( enemyPos ), returnPos ) ) {
		return qtrue;
	}
	// if we are hiding from a dangerous entity, try and avoid it
	if ( cs->dangerEntity == enemyNum && cs->dangerEntityValidTime > level.time ) {
		if ( cs->navigation.dangerLastGetAvoid > level.time - 750 ) {
			return qtrue;
		} else if ( AICast_GetAvoid( cs, NULL, cs->combat.takeCoverPos, qtrue, cs->dangerEntity ) ) {
			cs->navigation.dangerLastGetAvoid = level.time;
			return qtrue;
		}
	}
	//
	return qfalse;
}

int AI_PointAreaNum( vec3_t origin ) {
	int areanum, numareas, areas[1];
	vec3_t end, ofs;
#define BOTAREA_JIGGLE_DIST 32

	areanum = trap_AAS_PointAreaNum( origin );
	if ( areanum ) {
		return areanum;
	}
	VectorCopy( origin, end );
	end[2] += 10;
	numareas = trap_AAS_TraceAreas( origin, end, areas, NULL, 1 );
	if ( numareas > 0 ) {
		return areas[0];
	}

	// Ridah, jiggle them around to look for a fuzzy area, helps LARGE characters reach destinations that are against walls
	ofs[2] = 10;
	for ( ofs[0] = -BOTAREA_JIGGLE_DIST; ofs[0] <= BOTAREA_JIGGLE_DIST; ofs[0] += BOTAREA_JIGGLE_DIST * 2 ) {
		for ( ofs[1] = -BOTAREA_JIGGLE_DIST; ofs[1] <= BOTAREA_JIGGLE_DIST; ofs[1] += BOTAREA_JIGGLE_DIST * 2 ) {
			VectorAdd( origin, ofs, end );
			numareas = trap_AAS_TraceAreas( origin, end, areas, NULL, 1 );
			if ( numareas > 0 ) {
				return areas[0];
			}
		}
	}

	return 0;
}

// Presence values are part of the navigation movement interface.
static const int AI_PRESENCE_NORMAL = 2;
static const int AI_PRESENCE_CROUCH = 4;

void AI_SetupMovement( cast_state_t *cs ) {
	ai_client_state_t *bs = cs->clientState;
	bot_initmove_t initmove;

	memset( &initmove, 0, sizeof( bot_initmove_t ) );
	VectorCopy( bs->cur_ps.origin, initmove.origin );
	VectorCopy( bs->cur_ps.velocity, initmove.velocity );
	VectorCopy( bs->cur_ps.origin, initmove.viewoffset );
	initmove.viewoffset[2] += bs->cur_ps.viewheight;
	initmove.entitynum = bs->entitynum;
	initmove.client = bs->client;
	initmove.thinktime = bs->thinktime;
	//set the onground flag
	if ( bs->cur_ps.groundEntityNum != ENTITYNUM_NONE ) {
		initmove.or_moveflags |= MFL_ONGROUND;
	}
	//set the teleported flag
	if ( ( bs->cur_ps.pm_flags & PMF_TIME_KNOCKBACK ) && ( bs->cur_ps.pm_time > 0 ) ) {
		initmove.or_moveflags |= MFL_TELEPORTED;
	}
	//set the waterjump flag
	if ( ( bs->cur_ps.pm_flags & PMF_TIME_WATERJUMP ) && ( bs->cur_ps.pm_time > 0 ) ) {
		initmove.or_moveflags |= MFL_WATERJUMP;
	}
	//set presence type
	if ( bs->cur_ps.pm_flags & PMF_DUCKED ) {
		initmove.presencetype = AI_PRESENCE_CROUCH;
	} else {
		initmove.presencetype = AI_PRESENCE_NORMAL;
	}
	//
	if ( cs->movestate == MS_WALK ) {
		initmove.or_moveflags |= MFL_WALK;
	}
	//
	VectorCopy( cs->viewangles, initmove.viewangles );
	//
	trap_BotInitMoveState( bs->ms, &initmove );
}

qboolean AI_InLava( ai_client_state_t *bs ) {
	vec3_t feet;

	VectorCopy( bs->origin, feet );
	feet[2] -= 23;
	return ( trap_AAS_PointContents( feet ) & CONTENTS_LAVA );
}

qboolean AI_InSlime( ai_client_state_t *bs ) {
	vec3_t feet;

	VectorCopy( bs->origin, feet );
	feet[2] -= 23;
	return ( trap_AAS_PointContents( feet ) & CONTENTS_SLIME );
}
