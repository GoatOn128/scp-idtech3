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
==============
AICast_ChangeViewAngles
==============
*/
void AICast_ChangeViewAngles( cast_state_t *cs, float thinktime ) {
	float diff, factor, maxchange, anglespeed;
	int i;
	ai_client_state_t *bs;

	bs = cs->clientState;
	//
	// restore locked viewangles if required
	if ( cs->aiFlags & AIFL_VIEWLOCKED ) {
		VectorCopy( cs->viewlock_viewangles, cs->ideal_viewangles );
	} else {
		// check for playanim angles
		if ( cs->script.castScriptStatus.playAnimViewlockTime >= level.time ) {
			// check to make sure we are still playing a legs animation
			if ( !g_entities[cs->entityNum].client->ps.legsTimer ) {
				cs->script.castScriptStatus.playAnimViewlockTime = 0;
			} else {
				VectorCopy( cs->script.castScriptStatus.playanim_viewangles, cs->ideal_viewangles );
			}
		}
	}
	//
	if ( cs->ideal_viewangles[PITCH] > 180 ) {
		cs->ideal_viewangles[PITCH] -= 360;
	}
	//
	maxchange = cs->attributes[YAW_SPEED]; //300;
	if ( cs->aiState >= AISTATE_COMBAT ) {
		factor = 2.0;
		maxchange *= 2.0;
	} else {
		factor = 0.7;
	}
	//
	if ( cs->lockViewAnglesTime < level.time ) {
		maxchange *= thinktime;
		for ( i = 0; i < 3; i++ ) {
			diff = fabs( AI_AngleDifference( cs->viewangles[i], cs->ideal_viewangles[i] ) );
			anglespeed = diff * factor;
			if ( cs->aiState >= AISTATE_COMBAT ) {
				if ( anglespeed < cs->attributes[YAW_SPEED] ) {
					anglespeed = cs->attributes[YAW_SPEED];
				}
			}
			if ( thinktime != 9999.0f ) {
				if ( anglespeed > maxchange ) {
					anglespeed = maxchange;
				}
			}
			cs->viewangles[i] = AI_ChangeViewAngle( cs->viewangles[i], cs->ideal_viewangles[i], anglespeed );

			//cs->viewangles[i] = cs->ideal_viewangles[i];
		}
	}
	if ( cs->viewangles[PITCH] > 180 ) {
		cs->viewangles[PITCH] -= 360;
	}
	//elementary action: view
	trap_EA_View( bs->client, cs->viewangles );
}

/*
==============
AICast_InputToUserCommand
==============
*/
static int serverTime;
void AICast_InputToUserCommand( cast_state_t *cs, bot_input_t *bi, usercmd_t *ucmd, int delta_angles[3] ) {
	vec3_t angles, forward, right, up;
	short temp;
	int j;
	signed char movechar;
	gentity_t *ent;

	ent = &g_entities[cs->entityNum];

	//clear the whole structure
	memset( ucmd, 0, sizeof( usercmd_t ) );
	//
	//Com_Printf("dir = %f %f %f speed = %f\n", bi->dir[0], bi->dir[1], bi->dir[2], bi->speed);
	//the duration for the user command in milli seconds
	ucmd->serverTime = serverTime;
	//crouch/movedown
	if ( aiDefaults[cs->aiCharacter]
			 .attributes[ATTACK_CROUCH] ) { // only crouch if this character is physically able to
		if ( cs->clientState->cur_ps.groundEntityNum != ENTITYNUM_NONE && bi->actionflags & ACTION_CROUCH ) {
			ucmd->upmove -= 127;
		}
	}
	//
	// actions not effected by script pausing
	//
	// set zoom button
	if ( cs->aiFlags & AIFL_ZOOMING ) {
		ucmd->wbuttons |= WBUTTON_ZOOM;
	}
	//set the buttons
	if ( bi->actionflags & ACTION_ATTACK ) {
		vec3_t ofs;
		// don't fire if we are not facing the right direction yet
		if ( ( cs->combat.triggerReleaseTime < level.time ) &&
			 ( ( cs->lockViewAnglesTime >= level.time ) ||
			   ( fabs( AI_AngleDifference( cs->ideal_viewangles[YAW], cs->viewangles[YAW] ) ) < 20 ) ) &&
			 // check for radid luger firing by skilled users (release fire between shots)
			 ( ( ( level.time + cs->entityNum * 500 ) / 2000 ) % 2 || !( rand() % ( 1 + g_gameskill.integer ) ) ||
			   ( cs->attributes[ATTACK_SKILL] < 0.5 ) || ( cs->weaponNum != WP_LUGER ) ||
			   ( cs->clientState->cur_ps.weaponTime == 0 ) || ( cs->clientState->cur_ps.releasedFire ) ) ) {
			ucmd->buttons |= BUTTON_ATTACK;
			// do some swaying around for some weapons
			AICast_WeaponSway( cs, ofs );
			VectorAdd( bi->viewangles, ofs, bi->viewangles );
		}
	}
	//
	//set the view angles
	//NOTE: the ucmd->angles are the angles WITHOUT the delta angles
	ucmd->angles[PITCH] = ANGLE2SHORT( bi->viewangles[PITCH] );
	ucmd->angles[YAW] = ANGLE2SHORT( bi->viewangles[YAW] );
	ucmd->angles[ROLL] = ANGLE2SHORT( bi->viewangles[ROLL] );
	//subtract the delta angles
	for ( j = 0; j < 3; j++ ) {
		temp = ucmd->angles[j] - delta_angles[j];
		ucmd->angles[j] = temp;
	}

	//----(SA)	modified slightly for DM/DK
	ucmd->weapon = bi->weapon;
	//
	// relaxed mode show no weapons
	if ( cs->aiState <= AISTATE_QUERY ) {
		if ( WEAPS_ONE_HANDED & ( 1 << ucmd->weapon ) ) { // one-handed wepons don't draw, others do
			ucmd->weapon = WP_NONE;
		}
	}
	//----(SA)	end

	//
	if ( bi->actionflags & ACTION_GESTURE ) {
		ucmd->buttons |= BUTTON_GESTURE;
	}
	if ( bi->actionflags & ACTION_RELOAD ) {
		ucmd->wbuttons |= WBUTTON_RELOAD;
	}
	//
	// if we are locked down, don't do anything
	//
	if ( cs->pauseTime > level.time ) {
		return;
	}
	//
	// if scripted pause, no movement
	//
	if ( cs->script.castScriptStatus.scriptNoMoveTime > level.time ) {
		return;
	}
	//
	// if viewlock, wait until we are facing ideal angles before we move
	if ( cs->aiFlags & AIFL_VIEWLOCKED ) {
		if ( fabs( AI_AngleDifference( cs->ideal_viewangles[YAW], cs->viewangles[YAW] ) ) > 10 ) {
			return;
		}
	}
	//
	if ( bi->actionflags & ACTION_DELAYEDJUMP ) {
		bi->actionflags |= ACTION_JUMP;
		bi->actionflags &= ~ACTION_DELAYEDJUMP;
	}
	//
	// only move if we are in combat or we are facing where our ideal angles
	if ( bi->speed ) {
		if ( ( !( cs->aiFlags & AIFL_WALKFORWARD ) && cs->enemyNum >= 0 ) ||
			 ( ( ucmd->forwardmove >= 0 ) && fabs( AngleNormalize180( AI_AngleDifference(
												 cs->ideal_viewangles[YAW], cs->viewangles[YAW] ) ) ) < 60 ) ) {
			//NOTE: movement is relative to the REAL view angles
			//get the horizontal forward and right vector
			//get the pitch in the range [-180, 180]
			if ( bi->dir[2] ) {
				angles[PITCH] = bi->viewangles[PITCH];
			} else {
				angles[PITCH] = 0;
			}
			angles[YAW] = bi->viewangles[YAW];
			angles[ROLL] = 0;
			AngleVectors( angles, forward, right, up );
			//bot input speed is in the range [0, 400]
			bi->speed = bi->speed * 127 / 400;
			//set the view independent movement
			ucmd->forwardmove = DotProduct( forward, bi->dir ) * bi->speed;
			ucmd->rightmove = DotProduct( right, bi->dir ) * bi->speed;

			// RF, changed this to fix stim soldier flying attack
			if ( !ucmd->upmove ) { // only change it if we don't already have an upmove set
				ucmd->upmove = DotProduct( up, bi->dir ) * bi->speed;
			}
			//if (!ucmd->upmove)	// only change it if we don't already have an upmove set
			//	ucmd->upmove = abs(forward[2]) * bi->dir[2] * bi->speed;
		}
	}
	//
	//normal keyboard movement
	if ( cs->actionFlags & CASTACTION_WALK ) {
		movechar = 70;
	} else {
		movechar = 127;
	}
	if ( bi->actionflags & ACTION_MOVEFORWARD ) {
		ucmd->forwardmove = movechar;
	}
	if ( !( cs->aiFlags & AIFL_WALKFORWARD ) ||
		 ( !cs->clientState->cur_ps.groundEntityNum ||
		   cs->clientState->cur_ps.groundEntityNum ==
			   ENTITYNUM_NONE ) ) { // only do other movements if we are allowed to
		if ( bi->actionflags & ACTION_MOVEBACK ) {
			ucmd->forwardmove = -movechar;
		}
		if ( bi->actionflags & ACTION_MOVELEFT ) {
			ucmd->rightmove = -movechar;
		}
		if ( bi->actionflags & ACTION_MOVERIGHT ) {
			ucmd->rightmove = movechar;
		}
	}
	// prevent WALKFORWARD AI from moving backwards
	if ( cs->aiFlags & AIFL_WALKFORWARD ) {
		if ( ucmd->forwardmove < 0 ) {
			ucmd->forwardmove = 0;
		}
	}
	//jump/moveup
	if ( bi->actionflags & ACTION_JUMP ) {
		ucmd->upmove = 127; // JUMP always takes preference over ducking
	}
	if ( bi->actionflags & ACTION_MOVEDOWN ) {
		ucmd->upmove = -127; // JUMP always takes preference over ducking
	}
	if ( bi->actionflags & ACTION_MOVEUP ) {
		ucmd->upmove = 127; // JUMP always takes preference over ducking
	}
	//
	//Com_Printf("forward = %d right = %d up = %d\n", ucmd.forwardmove, ucmd.rightmove, ucmd.upmove);
}

/*
==============
AICast_UpdateInput
==============
*/
void AICast_UpdateInput( cast_state_t *cs, int time ) {
	bot_input_t bi;
	ai_client_state_t *bs;
	int j;
	float speed;

	bs = cs->clientState;

	//add the delta angles to the bot's current view angles
	for ( j = 0; j < 3; j++ ) {
		cs->viewangles[j] = AngleMod( cs->viewangles[j] + SHORT2ANGLE( bs->cur_ps.delta_angles[j] ) );
	}
	//
	AICast_ChangeViewAngles( cs, (float)time / 1000 );
	//
	if ( cs->pauseTime > level.time ) {
		trap_EA_View( bs->client, cs->viewangles );
		trap_EA_GetInput( bs->client, (float)time / 1000, &bi );
		AICast_InputToUserCommand( cs, &bi, &cs->lastucmd, bs->cur_ps.delta_angles );
		g_entities[cs->clientState->entitynum].client->ps.pm_flags &= ~PMF_RESPAWNED;
		//
		//subtract the delta angles
		for ( j = 0; j < 3; j++ ) {
			cs->viewangles[j] = AngleMod( cs->viewangles[j] - SHORT2ANGLE( bs->cur_ps.delta_angles[j] ) );
		}
		//
		return;
	}
	//
	trap_EA_GetInput( bs->client, (float)time / 1000, &bi );
	//
	// restrict the speed according to the character and their current speedScale
	// HACK, don't slow down while crouching
	if ( bi.actionflags & ACTION_CROUCH && cs->speedScale < 1.0 ) {
		cs->speedScale = 1.0;
	}
	//
	// check some Cast AI specific movement flags
	if ( cs->actionFlags & CASTACTION_WALK ) {
		if ( cs->speedScale > ( cs->attributes[WALKING_SPEED] / cs->attributes[RUNNING_SPEED] ) ) {
			cs->speedScale = ( cs->attributes[WALKING_SPEED] / cs->attributes[RUNNING_SPEED] );
		}
	}
	// don't ever let the speed get too low
	if ( cs->speedScale < 0.25 ) {
		cs->speedScale = 0.25;
	}
	if ( cs->speedScale > 1.2 ) {
		cs->speedScale = 1.2;
	}
	//
	speed = cs->speedScale * cs->attributes[RUNNING_SPEED];
	//
	//if (speed <= (cs->attributes[WALKING_SPEED] + (cs->attributes[WALKING_SPEED] + 50 < cs->attributes[RUNNING_SPEED] ? 50 : -1)))	// do a fast shuffle if slightly over walking speed
	if ( speed <= cs->attributes[WALKING_SPEED] ) {
		cs->actionFlags |= CASTACTION_WALK;
	}
	//
	// we use 300 here, because the default player speed is 300, so Cast AI's can't move faster than that
	if ( ( bi.speed / 400.0 ) > ( speed / 300.0 ) ) {
		bi.speed = 400.0 * ( speed / 300.0 );
		if ( bi.speed > 400.0 ) {
			bi.speed = 400.0; // just in case, we should never exceed this
		}
	}
	//
	// do a fast shuffle if slightly over walking speed
	if ( bi.speed <=
		 ( 400.0 / 300.0 ) * ( cs->attributes[WALKING_SPEED] +
							   ( cs->attributes[WALKING_SPEED] + 50 < cs->attributes[RUNNING_SPEED] ? 50 : -1 ) ) ) {
		cs->actionFlags |= CASTACTION_WALK;
	}
	//
	AICast_InputToUserCommand( cs, &bi, &cs->lastucmd, bs->cur_ps.delta_angles );
	//
	// check some Cast AI specific movement flags
	if ( cs->actionFlags & CASTACTION_WALK ) {
		cs->lastucmd.buttons |= BUTTON_WALKING; // play the walking animation
	}
	//
	//subtract the delta angles
	for ( j = 0; j < 3; j++ ) {
		cs->viewangles[j] = AngleMod( cs->viewangles[j] - SHORT2ANGLE( bs->cur_ps.delta_angles[j] ) );
	}
	//
	// make sure the respawn flag is disabled (causes problems after multiple "map xxx" commands)
	g_entities[cs->clientState->entitynum].client->ps.pm_flags &= ~PMF_RESPAWNED;
	// set the aiState
	g_entities[cs->clientState->entitynum].client->ps.aiState = cs->aiState;
}

/*
============
AICast_StartServerFrame

  Do movements, sighting, etc
============
*/
void AICast_StartServerFrame( int time ) {
	int i, elapsed, count, clCount, activeCount;
	cast_state_t *cs;
	int castcount;
	static int lasttime;
	static vmCvar_t aicast_disable;
	gentity_t *ent;
	cast_state_t *pcs;
	qboolean highPriority;
	int oldLegsTimer;

	if ( trap_Cvar_VariableIntegerValue( "savegame_loading" ) ) {
		return;
	}

	if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
		return;
	}

	if ( saveGamePending ) {
		return;
	}

	// if waiting at intermission, don't think
	if ( strlen( g_missionStats.string ) > 1 ) {
		return;
	}

	if ( !aicast_disable.handle ) {
		trap_Cvar_Register( &aicast_disable, "aicast_disable", "0", CVAR_CHEAT );
	} else {
		trap_Cvar_Update( &aicast_disable );
		if ( aicast_disable.integer ) {
			return;
		}
	}

	trap_Cvar_Update( &aicast_debug );

	// no need to think during the intermission
	if ( level.intermissiontime ) {
		return;
	}
	//
	// make sure the AAS gets updated
	AI_UpdateNavigationEntities( time );
	//
	//
	elapsed = time - lasttime;
	if ( elapsed == 0 ) {
		return; // no time has elapsed
	}

	pcs = AICast_GetCastState( 0 );

	//G_Printf( "AI startserverframe: %i\n", time );

	AICast_AgePlayTime( 0 );

	if ( elapsed < 0 ) {
		elapsed = 0;
		lasttime = time;
	}
	// don't let the framerate drop below 10
	if ( elapsed > 100 ) {
		elapsed = 100;
	}
	//
	// process player's current script if it exists
	AICast_ScriptRun( AICast_GetCastState( 0 ), qfalse );
	//
	//AICast_SightUpdate( (int)((float)SIGHT_PER_SEC * ((float)elapsed / 1000)) );
	//
	count = 0;
	castcount = 0;
	clCount = 0;
	activeCount = 0;
	//
	//update the AI characters
	for ( i = 0, ent = g_entities; i < level.maxclients /*&& clCount < level.numPlayingClients*/; i++, ent++ ) {
		//if (ent->inuse && ent->client)
		//	clCount++;
		//
		cs = AICast_GetCastState( i );
		// is this a cast AI?
		if ( cs->clientState ) {
			if ( ent->aiInactive == qfalse && ent->inuse ) {
				//
				elapsed = level.time - cs->lastMoveThink;
				if ( cs->lastThink && elapsed > 0 ) {
					highPriority = qfalse;
					if ( ent->health > 0 ) {
						highPriority = qtrue;
					} else if ( cs->deathTime > ( level.time - 5000 ) ) {
						highPriority = qtrue;
					}
					//
					if ( highPriority ) {
						activeCount++;
					}
					//
					// optimization, if they're not in the player's PVS, and they aren't trying to move, then don't bother thinking
					if (
						( highPriority && ( elapsed > 300 ) ) ||
						( g_entities[0].client && g_entities[0].client->cameraPortal ) ||
						( highPriority && ( cs->perception.vislist[0].visible_timestamp ==
											cs->perception.vislist[0].lastcheck_timestamp ) ) ||
						( highPriority && ( pcs->perception.vislist[cs->entityNum].visible_timestamp ==
											pcs->perception.vislist[cs->entityNum].lastcheck_timestamp ) ) ||
						( VectorLength( ent->client->ps.velocity ) > 0 ) ||
						( highPriority && ( cs->lastucmd.forwardmove || cs->lastucmd.rightmove ||
											cs->lastucmd.upmove > 0 || cs->lastucmd.buttons || cs->lastucmd.wbuttons ) )
						// !!NOTE: always allow thinking if in PVS, otherwise bosses won't gib, and dead guys might not push away from clipped walls
						|| ( trap_InPVS(
							   cs->clientState->origin,
							   g_entities[0]
								   .s.pos.trBase ) ) ) { // do pvs check last, since it's the most expensive to call
						oldLegsTimer = ent->client->ps.legsTimer;
						//
						// send it's movement commands
						//
						serverTime = time;
						AICast_UpdateInput( cs, elapsed );
						trap_BotUserCommand( cs->clientState->client, &( cs->lastucmd ) );
						cs->lastMoveThink = level.time;
						//
						// check for anim changes that may require us to stay still
						//
						if ( oldLegsTimer < ent->client->ps.legsTimer &&
							 ent->client->ps.groundEntityNum == ENTITYNUM_WORLD ) {
							// dont move until they are finished
							if ( cs->script.castScriptStatus.scriptNoMoveTime <
								 level.time + ent->client->ps.legsTimer ) {
								cs->script.castScriptStatus.scriptNoMoveTime = level.time + ent->client->ps.legsTimer;
							}
						}
					}
				}
			} else {
				trap_UnlinkEntity( ent );
			}
			//
			// see if we've checked all cast AI's
			if ( ++castcount >= numcast ) {
				break;
			}
		}
	}
	//
	lasttime = time;
	//
	if ( aicast_debug.integer == 3 ) {
		G_Printf( "AI Active Count: %i\n", activeCount );
	}
}

float AI_AngleDifference( float ang1, float ang2 ) {
	float diff;

	diff = ang1 - ang2;
	if ( ang1 > ang2 ) {
		if ( diff > 180.0 ) {
			diff -= 360.0;
		}
	} else {
		if ( diff < -180.0 ) {
			diff += 360.0;
		}
	}
	return diff;
}

float AI_ChangeViewAngle( float angle, float ideal_angle, float speed ) {
	float move;

	angle = AngleMod( angle );
	ideal_angle = AngleMod( ideal_angle );
	if ( angle == ideal_angle ) {
		return angle;
	}
	move = ideal_angle - angle;
	if ( ideal_angle > angle ) {
		if ( move > 180.0 ) {
			move -= 360.0;
		}
	} else {
		if ( move < -180.0 ) {
			move += 360.0;
		}
	}
	if ( move > 0 ) {
		if ( move > speed ) {
			move = speed;
		}
	} else {
		if ( move < -speed ) {
			move = -speed;
		}
	}
	return AngleMod( angle + move );
}
