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
AICast_Think

  entry point for all cast AI
============
*/
void AICast_Think( int client, float thinktime ) {
	gentity_t *ent;
	cast_state_t *cs;
	int i;
	int animIndex;
	animation_t *anim;

	//	if (saveGamePending || (strlen( g_missionStats.string ) > 2 )) {
	//		return;
	//	}

	//
	// get the cast ready for processing
	//
	cs = AICast_GetCastState( client );
	ent = &g_entities[client];
	//
	// make sure we are using the right AAS data for this entity (one's that don't get set will default to the player's AAS data)
	trap_AAS_SetCurrentWorld( cs->aasWorldIndex );
	//
	// make sure we have a valid navigation system
	//
	if ( !trap_AAS_Initialized() ) {
		return;
	}
	//
	trap_EA_ResetInput( client, NULL );
	cs->aiFlags &= ~AIFL_VIEWLOCKED;
	cs->aiFlags &= ~AIFL_SPECIAL_FUNC;
	//cs->weaponNum = ent->client->ps.weapon;
	//
	// turn off flags that are set each frame if needed
	ent->client->ps.eFlags &= ~( EF_NOSWINGANGLES | EF_MONSTER_EFFECT | EF_MONSTER_EFFECT2 | EF_MONSTER_EFFECT3 );
	// conditional flags
	if ( ent->aiCharacter == AICHAR_ZOMBIE ) {
		if ( COM_BitCheck( ent->client->ps.weapons, WP_MONSTER_ATTACK1 ) ) {
			cs->aiFlags |= AIFL_NO_FLAME_DAMAGE;
			SET_FLAMING_ZOMBIE( ent->s, 1 );
		} else {
			SET_FLAMING_ZOMBIE( ent->s, 0 );
		}
	}
	//
	// update bounding box
	AIChar_SetBBox( ent, cs, qtrue );
	//
	// set/disable these each frame as required
	//ent->r.svFlags |= SVF_BROADCAST;
	ent->client->ps.eFlags &= ~EF_FORCE_END_FRAME;
	//origin of the cast
	VectorCopy( ent->client->ps.origin, cs->clientState->origin );
	//eye coordinates of the cast
	VectorCopy( ent->client->ps.origin, cs->clientState->eye );
	cs->clientState->eye[2] += ent->client->ps.viewheight;
	//get the area the cast is in
	cs->clientState->areanum = AI_PointAreaNum( cs->clientState->origin );
	if ( cs->clientState->areanum ) {
		cs->navigation.lastValidAreaNum[cs->aasWorldIndex] = cs->clientState->areanum;
		cs->navigation.lastValidAreaTime[cs->aasWorldIndex] = level.time;
	}
	// if we're dead, do special stuff only
	if ( ent->health <= 0 || cs->revivingTime || cs->rebirthTime ) {
		if ( cs->fsm.state == AIStateId::LoperLeap ) {
			AI_CancelState( cs, AITransitionReason::Death );
		}
		//
		if ( cs->revivingTime && cs->revivingTime < level.time ) {
			// start us thinking again
			ent->client->ps.pm_type = PM_NORMAL;
			cs->revivingTime = 0;
		}
		//
		if ( cs->rebirthTime && cs->rebirthTime < level.time ) {
			vec3_t mins, maxs;
			int touch[10], numTouch;
			float oldmaxZ;

			oldmaxZ = ent->r.maxs[2];

			// make sure the area is clear
			AIChar_SetBBox( ent, cs, qfalse );

			VectorAdd( ent->r.currentOrigin, ent->r.mins, mins );
			VectorAdd( ent->r.currentOrigin, ent->r.maxs, maxs );
			trap_UnlinkEntity( ent );

			numTouch = trap_EntitiesInBox( mins, maxs, touch, 10 );

			if ( numTouch ) {
				for ( i = 0; i < numTouch; i++ ) {
					//if (!g_entities[touch[i]].client || g_entities[touch[i]].r.contents == CONTENTS_BODY)
					if ( g_entities[touch[i]].r.contents & MASK_PLAYERSOLID ) {
						break;
					}
				}
				if ( i == numTouch ) {
					numTouch = 0;
				}
			}

			if ( numTouch == 0 ) { // ok to spawn

				// give them health when they start reviving, so we won't gib after
				// just a couple shots while reviving
				ent->health = ent->client->ps.stats[STAT_HEALTH] = ent->client->ps.stats[STAT_MAX_HEALTH] =
					( ( cs->attributes[STARTING_HEALTH] - 50 ) > 30 ? ( cs->attributes[STARTING_HEALTH] - 50 ) : 30 );

				ent->r.contents = CONTENTS_BODY;
				ent->clipmask = MASK_PLAYERSOLID | CONTENTS_MONSTERCLIP;
				ent->takedamage = qtrue;
				ent->waterlevel = 0;
				ent->watertype = 0;
				ent->flags = 0;
				ent->die = AICast_Die;
				ent->client->ps.eFlags &= ~EF_DEAD;
				ent->s.eFlags &= ~EF_DEAD;

				cs->rebirthTime = 0;
				cs->deathTime = 0;

				ent->client->ps.eFlags &= ~EF_DEATH_FRAME;
				ent->client->ps.eFlags &= ~EF_FORCE_END_FRAME;
				ent->client->ps.eFlags |= EF_NO_TURN_ANIM;

				// play the revive animation
				cs->revivingTime = level.time + BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_REVIVE, qfalse, qtrue );
				;
			} else {
				// can't spawn yet, so set bbox back, and wait
				ent->r.maxs[2] = oldmaxZ;
				ent->client->ps.maxs[2] = ent->r.maxs[2];
			}
			trap_LinkEntity( ent );
		}
		// ZOMBIE should set effect flag if really dead
		if ( cs->aiCharacter == AICHAR_ZOMBIE && !ent->r.contents ) {
			ent->client->ps.eFlags |= EF_MONSTER_EFFECT2;
		}
		//
		if ( ent->health > GIB_HEALTH && cs->deathTime && cs->deathTime < ( level.time - 1000 ) ) {
			//ent->r.svFlags &= ~SVF_BROADCAST;
			if ( !ent->client->ps.torsoTimer && !ent->client->ps.legsTimer ) {
				ent->client->ps.eFlags |= EF_FORCE_END_FRAME;
			}
			// sink?
			if ( cs->deadSinkStartTime ) {
				ent->s.effect3Time = cs->deadSinkStartTime;
				// if they are gone
				if ( cs->deadSinkStartTime + DEAD_SINK_DURATION < level.time ) {
					trap_DropClient( cs->entityNum, "" );
					return;
				}
			}
			// if we've been dead for a while, stop head-checking
			if ( cs->deathTime < ( level.time - 5000 ) ) {
				ent->flags |= FL_NO_HEADCHECK;
			}
		}
		//
		// do some special handling for this character
		AICast_SpecialFunc( cs );
		//
		// no more thinking required
		return;
	}
	//
	// set some anim conditions
	if ( cs->secondDeadTime ) {
		BG_UpdateConditionValue( cs->entityNum, ANIM_COND_SECONDLIFE, qtrue, qfalse );
	} else {
		BG_UpdateConditionValue( cs->entityNum, ANIM_COND_SECONDLIFE, qfalse, qfalse );
	}
	// set health value
	if ( ent->health <= 0.25 * cs->attributes[STARTING_HEALTH] ) {
		BG_UpdateConditionValue( cs->entityNum, ANIM_COND_HEALTH_LEVEL, 2, qfalse );
	} else if ( ent->health <= 0.5 * cs->attributes[STARTING_HEALTH] ) {
		BG_UpdateConditionValue( cs->entityNum, ANIM_COND_HEALTH_LEVEL, 1, qfalse );
	} else {
		BG_UpdateConditionValue( cs->entityNum, ANIM_COND_HEALTH_LEVEL, 0, qfalse );
	}
	// set enemy position
	if ( cs->enemyNum >= 0 ) {
		if ( infront( ent, &g_entities[cs->enemyNum] ) ) {
			BG_UpdateConditionValue( ent->s.number, ANIM_COND_ENEMY_POSITION, POSITION_INFRONT, qtrue );
		} else {
			BG_UpdateConditionValue( ent->s.number, ANIM_COND_ENEMY_POSITION, POSITION_BEHIND, qtrue );
		}
	} else {
		BG_UpdateConditionValue( ent->s.number, ANIM_COND_ENEMY_POSITION, POSITION_UNUSED, qtrue );
	}
	// set defense pose
	if ( ent->flags & FL_DEFENSE_CROUCH ) {
		BG_UpdateConditionValue( ent->s.number, ANIM_COND_DEFENSE, qtrue, qfalse );
	} else {
		BG_UpdateConditionValue( ent->s.number, ANIM_COND_DEFENSE, qfalse, qfalse );
	}
	//
	cs->speedScale = 1.0; // reset each frame, set if required
	cs->actionFlags = 0; // FIXME: move this to a Cast AI movement init function!
	//retrieve the current client state
	AI_GetClientState( client, &( cs->clientState->cur_ps ) );
	//
	// setup movement speeds for the given state
	// walking
	animIndex = BG_GetAnimScriptAnimation( cs->entityNum, ent->client->ps.aiState, ANIM_MT_WALK );
	if ( animIndex >= 0 ) {
		anim = BG_GetAnimationForIndex( cs->entityNum, animIndex );
		cs->attributes[WALKING_SPEED] = anim->moveSpeed;
	}
	// crouching
	animIndex = BG_GetAnimScriptAnimation( cs->entityNum, ent->client->ps.aiState, ANIM_MT_WALKCR );
	if ( animIndex >= 0 ) {
		anim = BG_GetAnimationForIndex( cs->entityNum, animIndex );
		cs->attributes[CROUCHING_SPEED] = anim->moveSpeed;
	}
	// running
	animIndex = BG_GetAnimScriptAnimation( cs->entityNum, ent->client->ps.aiState, ANIM_MT_RUN );
	if ( animIndex >= 0 ) {
		anim = BG_GetAnimationForIndex( cs->entityNum, animIndex );
		cs->attributes[RUNNING_SPEED] = anim->moveSpeed;
	}
	// update crouch speed scale
	ent->client->ps.crouchSpeedScale = cs->attributes[CROUCHING_SPEED] / cs->attributes[RUNNING_SPEED];
	//
	// only enable headlook if we want to this frame
	ent->client->ps.eFlags &= ~EF_HEADLOOK;
	if ( cs->enemyNum >= 0 ) {
		ent->client->ps.eFlags &= ~EF_STAND_IDLE2; // never use alt idle if fighting
	}
	//
	// check for dead leader
	if ( cs->leaderNum >= 0 && g_entities[cs->leaderNum].health <= 0 ) {
		cs->leaderNum = -1;
	}
	//
#if 0
	// HACK for village2, if they are stuck, find a good position (there is a friendly guy placed inside a table)
	{
		trace_t tr;
		vec3_t org;
		trap_Trace( &tr, cs->clientState->cur_ps.origin, cs->clientState->cur_ps.mins, cs->clientState->cur_ps.maxs, cs->clientState->cur_ps.origin, cs->entityNum, CONTENTS_SOLID );
		while ( tr.startsolid ) {
			VectorCopy( cs->clientState->cur_ps.origin, org );
			org[0] += 96 * crandom();
			org[1] += 96 * crandom();
			org[2] += 16 * crandom();
			trap_Trace( &tr, org, cs->clientState->cur_ps.mins, cs->clientState->cur_ps.maxs, org, cs->entityNum, CONTENTS_SOLID );
			G_SetOrigin( &g_entities[cs->entityNum], org );
			VectorCopy( org, g_entities[cs->entityNum].client->ps.origin );
		}
	}
#endif
	//add the delta angles to the cast's current view angles
	for ( i = 0; i < 3; i++ ) {
		cs->viewangles[i] = AngleMod( cs->viewangles[i] + SHORT2ANGLE( cs->clientState->cur_ps.delta_angles[i] ) );
	}
	//
	//increase the local time of the cast
	//
	cs->clientState->thinktime = thinktime;
	// clear flags each frame
	cs->bFlags = 0;
	//
	// check enemy health
	if ( cs->enemyNum >= 0 && g_entities[cs->enemyNum].health <= 0 ) {
		cs->enemyNum = -1;
	}
	//
	// if the previous movetype was temporary, set it back
	if ( cs->movestateType == MSTYPE_TEMPORARY ) {
		cs->movestate = MS_DEFAULT;
		cs->movestateType = MSTYPE_NONE;
	}
	// crouching?
	if (    ( cs->attackcrouch_time >= level.time ) /*&&
			((cs->lastAttackCrouch > level.time - 500) || (cs->thinkFuncChangeTime < level.time - 1000))*/) {
		// if we are not moving, and we are firing, always stand, unless we are allowed to crouch + fire
		if ( ( cs->lastucmd.forwardmove || cs->lastucmd.rightmove ) ||
			 ( cs->combat.lastWeaponFired < level.time - 2000 ) || ( cs->aiFlags & AIFL_ATTACK_CROUCH ) ) {
			cs->lastAttackCrouch = level.time;
			trap_EA_Crouch( cs->clientState->client );
		}
	}
	//
	//if (cs->enemyNum >= 0) {
	//update the attack inventory values
	AICast_UpdateBattleInventory( cs, cs->enemyNum );
	//}
	//
	// if we don't have ammo for the current weapon, get rid of it
	if ( !( COM_BitCheck( cs->clientState->cur_ps.weapons, cs->weaponNum ) ) ||
		 !AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
		// select a weapon
		AICast_ChooseWeapon( cs, qfalse );
		// if still no ammo, select a blank weapon
		//if (!AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum )) {
		//	cs->weaponNum = WP_NONE;
		//}
	}
	//
	// in query mode, we do special handling (pause scripting, check for transition to alert/combat, etc)
	if ( cs->aiState == AISTATE_QUERY ) {
		AICast_QueryThink( cs );
	} else if ( cs->pauseTime < level.time ) {
		// do the thinking
		AICast_ProcessAIFunctions( cs, thinktime );
		//
		// make sure the correct weapon is selected
		trap_EA_SelectWeapon( cs->clientState->client, cs->weaponNum );
		//
		// process current script if it exists
		cs->script.castScriptStatusCurrent = cs->script.castScriptStatus;
		AICast_ScriptRun( cs, qfalse );
		//
		// do some special handling for this character
		AICast_SpecialFunc( cs );
	}
	//
	// any anim playing on legs should prevent turn anims being played in the cgame
	if ( ent->client->ps.legsTimer ) {
		ent->client->ps.eFlags |= EF_NO_TURN_ANIM;
	} else {
		ent->client->ps.eFlags &= ~EF_NO_TURN_ANIM;
	}
	//
	// set special movestate if necessary
	if ( cs->movestateType != MSTYPE_NONE ) {
		switch ( cs->movestate ) {
		case MS_WALK:
			cs->actionFlags |= CASTACTION_WALK;
			break;
		case MS_CROUCH:
			trap_EA_Crouch( cs->entityNum );
			break;
		default:
			break;
		}
	}
	//
	// see if we were recently firing
	if ( cs->combat.lastWeaponFired && cs->combat.lastWeaponFired > level.time - 2000 ) {
		ent->client->ps.eFlags |= EF_RECENTLY_FIRING;
	} else {
		ent->client->ps.eFlags &= ~EF_RECENTLY_FIRING;
	}
	//
	//subtract the delta angles
	for ( i = 0; i < 3; i++ ) {
		cs->viewangles[i] = AngleMod( cs->viewangles[i] - SHORT2ANGLE( cs->clientState->cur_ps.delta_angles[i] ) );
	}
}

/*
==============
AICast_QueryThink
==============
*/
void AICast_QueryThink( cast_state_t *cs ) {
	gentity_t *ent;
	qboolean visible;
	cast_state_t *ocs;
	vec3_t vec;

	ent = &g_entities[cs->entityNum];
	ocs = AICast_GetCastState( cs->enemyNum );

	// never crouch while in this state (by choice anyway)
	cs->attackcrouch_time = 0;

	// look at where we last (thought we) saw them
	VectorSubtract( cs->perception.vislist[cs->enemyNum].visible_pos, cs->clientState->origin, vec );
	VectorNormalize( vec );
	vectoangles( vec, cs->ideal_viewangles );

	// are they visible now?
	visible = AICast_VisibleFromPos( cs->clientState->origin, cs->entityNum, g_entities[cs->enemyNum].r.currentOrigin,
									 cs->enemyNum, qfalse );

	// make sure we dont process the sighting of this enemy by going into query mode again, without them being visible again after we leave here
	cs->perception.vislist[cs->enemyNum].flags &= ~AIVIS_PROCESS_SIGHTING;

	// look towards where we last saw them
	AICast_AimAtEnemy( cs );

	// if visible and alert time has expired, go POSTAL
	if ( ( cs->perception.queryAlertSightTime < 0 ) ||
		 ( ( cs->perception.queryAlertSightTime < level.time ) && visible ) ) {
		if ( !cs->perception.queryAlertSightTime ) {
			// set the "short reaction" condition
			BG_UpdateConditionValue( cs->entityNum, ANIM_COND_SHORT_REACTION, qtrue, qfalse );
		}
		AICast_StateChange( cs, AISTATE_COMBAT );
		BG_UpdateConditionValue( cs->entityNum, ANIM_COND_SHORT_REACTION, qfalse, qfalse );
		AIFunc_BattleStart( cs );
		return;
	}

	// if they've fired since the start of the query mode, go POSTAL
	if ( ocs->combat.lastWeaponFired > cs->perception.queryStartTime ) {
		// set the "short reaction" condition
		BG_UpdateConditionValue( cs->entityNum, ANIM_COND_SHORT_REACTION, qtrue, qfalse );
		AICast_StateChange( cs, AISTATE_COMBAT );
		BG_UpdateConditionValue( cs->entityNum, ANIM_COND_SHORT_REACTION, qfalse, qfalse );
		AIFunc_BattleStart( cs );
		return;
	}

	// if not visible, then kill the Lock On timer
	if ( ( cs->perception.queryAlertSightTime > 0 ) && !visible ) {
		cs->perception.queryAlertSightTime = 0;
	}

	// if the query has expired, go back to relaxed
	if ( !ent->client->ps.legsTimer ) {
		AICast_StateChange( cs, AISTATE_RELAXED );
	}
}

/*
==================
AICast_IdleReload
==================
*/
void AICast_IdleReload( cast_state_t *cs ) {
	if ( AICast_NoReload( cs->entityNum ) ) {
		return;
	}
	if ( cs->noReloadTime >= level.time ) {
		return;
	}
	if ( !( ( cs->clientState->cur_ps.ammoclip[BG_FindClipForWeapon( cs->clientState->cur_ps.weapon )] <
			  (int)( 0.75 * ammoTable[cs->clientState->cur_ps.weapon].maxclip ) ) &&
			cs->clientState->cur_ps.ammo[BG_FindAmmoForWeapon( cs->clientState->cur_ps.weapon )] ) ) {
		return;
	}
	//
	trap_EA_Reload( cs->entityNum );
}

int AI_GetClientState( int clientNum, playerState_t *state ) {
	if ( !state || clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		return qfalse;
	}
	gentity_t *ent;

	ent = &g_entities[clientNum];
	if ( !ent->inuse ) {
		return qfalse;
	}
	if ( !ent->client ) {
		return qfalse;
	}

	memcpy( state, &ent->client->ps, sizeof( playerState_t ) );
	return qtrue;
}
