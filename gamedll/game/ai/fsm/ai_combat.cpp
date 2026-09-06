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
AIFunc_Battle()
============
*/
char *AIFunc_Battle( cast_state_t *cs ) {
	bot_moveresult_t moveresult;
	int tfl;
	ai_client_state_t *bs;
	gentity_t *ent, *enemy;

	ent = &g_entities[cs->entityNum];
	enemy = &g_entities[cs->enemyNum];

	// if we are not in combat mode, then go there now!
	if ( cs->aiState < AISTATE_COMBAT ) {
		AICast_StateChange( cs, AISTATE_COMBAT ); // just go straight to combat mode
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
	// do we need to go to our leader?
	if ( cs->leaderNum >= 0 &&
		 Distance( cs->clientState->origin, g_entities[cs->leaderNum].r.currentOrigin ) > MAX_LEADER_DIST ) {
		return AIFunc_ChaseGoalStart( cs, cs->leaderNum, AICAST_LEADERDIST_MAX, qtrue );
	}
	bs = cs->clientState;
	//if no enemy
	if ( cs->enemyNum < 0 ) {
		// go back to whatever our default action is
		return AIFunc_DefaultStart( cs );
	}
	//
	if ( enemy->health <= 0 ) {
		// go back to whatever our default action is
		if ( g_entities[cs->entityNum].aiTeam == AITEAM_NAZI ) {
			return AIFunc_InspectBodyStart( cs );
		} else {
			return AIFunc_DefaultStart( cs );
		}
	}
	//
	// if we are not in a good attacking position, we should chase
	if ( !AICast_StopAndAttack( cs ) ) {
		return AIFunc_BattleChaseStart( cs );
	}
	//
	// if the enemy is no longer visible
	if ( ( cs->clientState->cur_ps.weaponTime < 100 ) // if reloading, don't chase until ready
		 && ( cs->script.castScriptStatus.scriptNoMoveTime < level.time ) &&
		 ( /*!AICast_EntityVisible( cs, cs->enemyNum, qtrue ) ||*/ !AICast_CheckAttack( cs, cs->enemyNum, qfalse ) ) ) {

		// if we are in a void, then try to avoid so we get out of it
		if ( !cs->clientState->areanum ) {
			if ( cs->navigation.obstructingTime >= level.time ) {
				// move there
				trap_EA_Move( cs->entityNum, cs->combat.takeCoverPos, 200 );
			} else if ( AICast_GetAvoid( cs, NULL, cs->combat.takeCoverPos, qtrue, cs->enemyNum ) ) {
				VectorSubtract( cs->combat.takeCoverPos, cs->clientState->origin, cs->combat.takeCoverPos );
				if ( VectorNormalize( cs->combat.takeCoverPos ) > 60 ) {
					cs->navigation.obstructingTime = level.time + 1000 + rand() % 600;
				}
				return NULL;
			}
		} else
			// if we are heading for a combatGoal, give us some time to get there
			if ( cs->navigation.combatGoalTime > level.time ) {
				if ( cs->navigation.combatGoalTime > level.time + 3000 ) {
					cs->navigation.combatGoalTime = level.time + 2000 + rand() % 1000;
					cs->combat.combatSpotDelayTime = level.time + 4000 + rand() % 3000;
				}
			} else if ( cs->leaderNum >= 0 ) {
				// chase them, nothing else to do
				return AIFunc_BattleChaseStart( cs );
			} else
				// if we weren't moving, it is likely they have dodged back behind something, ready to duck out and take another
				// shot. so, we could fool them by hiding from the position we last saw them from, in the hope that when they
				// return to fire at us, we won't be in their sight.
				if ( cs->attributes[TACTICAL] > 0.3 && cs->attributes[AGGRESSION] < 1.0 &&
					 cs->attributes[AGGRESSION] < ( random() + 0.5 * cs->attributes[TACTICAL] ) &&
					 ( cs->combat.takeCoverTime < level.time ) &&
					 AICast_GetTakeCoverPos( cs, cs->enemyNum, cs->perception.vislist[cs->enemyNum].real_visible_pos,
											 cs->combat.takeCoverPos ) ) {
					// start taking cover
					cs->combat.takeCoverTime = level.time + 2000 + rand() % 4000; // only move a little bit
					//cs->attackcrouch_time = 0;		// get out of here real quick
					return AIFunc_BattleTakeCoverStart( cs );
				} else
					// if we haven't thrown a grenade in a bit, go into "grenade flush mode"
					if ( ( ai_encounter.lastGrenadeFlushTime > level.time ||
						   ai_encounter.lastGrenadeFlushTime < level.time - 7000 ) &&
						 ( cs->aiState >= AISTATE_COMBAT ) &&
						 ( cs->script.castScriptStatus.castScriptEventIndex < 0 ) &&
						 ( ( ( COM_BitCheck( cs->clientState->cur_ps.weapons, WP_GRENADE_LAUNCHER ) ) &&
							 ( AICast_GotEnoughAmmoForWeapon( cs, WP_GRENADE_LAUNCHER ) ) &&
							 ( cs->combat.weaponFireTimes[WP_GRENADE_LAUNCHER] <
							   level.time - (int)( aicast_skillscale * 3000 ) ) ) ||
						   ( ( COM_BitCheck( cs->clientState->cur_ps.weapons, WP_GRENADE_PINEAPPLE ) ) &&
							 ( AICast_GotEnoughAmmoForWeapon( cs, WP_GRENADE_PINEAPPLE ) ) &&
							 ( cs->combat.weaponFireTimes[WP_GRENADE_PINEAPPLE] <
							   level.time - (int)( aicast_skillscale * 3000 ) ) ) ) &&
						 !( cs->weaponNum && ( cs->script.castScriptStatus.scriptFlags & SFL_NOCHANGEWEAPON ) ) &&
						 ( Distance( cs->clientState->origin, cs->perception.vislist[cs->enemyNum].real_visible_pos ) >
						   100 ) &&
						 ( Distance( cs->clientState->origin, cs->perception.vislist[cs->enemyNum].real_visible_pos ) <
						   1200 ) &&
						 ( AICast_WantsToChase( cs ) ) ) {
						// try and flush them out with a grenade
						//G_Printf("get outta there..\n");
						return AIFunc_GrenadeFlushStart( cs );
					} else
						// not visible, should we chase them?
						if ( AICast_WantsToChase( cs ) ) {
							// chase them
							return AIFunc_BattleChaseStart( cs );
						} else
							// Take Cover?
							if ( AICast_WantsToTakeCover( cs, qfalse ) && ( cs->combat.takeCoverTime < level.time ) &&
								 AICast_GetTakeCoverPos( cs, cs->enemyNum,
														 cs->perception.vislist[cs->enemyNum].real_visible_pos,
														 cs->combat.takeCoverPos ) ) {
								// go to a position that cannot be seen from the last place we saw the enemy, and wait there for some time
								cs->combat.takeCoverTime = level.time + 4000 + rand() % 2000;
								//cs->attackcrouch_time = 0;
								return AIFunc_BattleTakeCoverStart( cs );
							} else {
								// chase them, nothing else to do
								return AIFunc_BattleChaseStart( cs );
							}
	}
	// if we are obstructing someone else, move out the way
	if ( cs->navigation.obstructingTime > level.time ) {
		// setup a combatgoal in the obstructionYaw direction
		//cs->navigation.combatGoalTime = level.time + 10;
		//VectorCopy( cs->navigation.obstructingPos, cs->navigation.combatGoalOrigin );
		AICast_MoveToPos( cs, cs->navigation.obstructingPos, -1 );
		// if not crouching, walk instead of running
		cs->speedScale = cs->attributes[WALKING_SPEED] / cs->attributes[RUNNING_SPEED];
	}
	// if the enemy is really close, avoid them
	else if ( ( cs->navigation.obstructingTime < ( level.time - 500 + rand() % 300 ) ) &&
			  ( Distance( cs->clientState->origin, cs->perception.vislist[cs->enemyNum].real_visible_pos ) < 100 ) ) {
		if ( AICast_GetAvoid( cs, NULL, cs->navigation.obstructingPos, qtrue, cs->enemyNum ) ) {
			cs->navigation.obstructingTime = level.time + 500;
		} else {
			cs->navigation.obstructingTime = level.time - 1; // wait a bit before trying again
		}
	}
	//
	// setup for the fight
	//
	tfl = cs->travelflags;
	//if in lava or slime the bot should be able to get out
	if ( AI_InLava( bs ) ) {
		tfl |= TFL_LAVA;
	}
	if ( AI_InSlime( bs ) ) {
		tfl |= TFL_SLIME;
	}
	//
	/*
	moveresult = AICast_CombatMove(cs, tfl);
	//if the movement failed
	if (moveresult.failure) {
		//reset the avoid reach, otherwise bot is stuck in current area
		trap_BotResetAvoidReach(bs->ms);
		// reset the combatgoal
		cs->navigation.combatGoalTime = 0;
	} else if (cs->navigation.combatGoalTime > level.time && VectorLength(cs->clientState->cur_ps.velocity)) {	// crouch if moving?
		if (cs->attributes[ATTACK_CROUCH] > 0.1) {
			AICast_RequestCrouchAttack( cs, cs->clientState->origin, 0.5 );
		}
	}
	*/
	// if we are crouching, don't stay down for too long after we finish fighting
	if ( cs->aiFlags & AIFL_ATTACK_CROUCH ) {
		if ( cs->attackcrouch_time > level.time || ( cs->thinkFuncChangeTime < level.time - 1000 ) ) {
			cs->attackcrouch_time = level.time + 1000;
		}
	} else {
		cs->attackcrouch_time = 0; // only set it if we need it
	}
	//
	AICast_Blocked( cs, &moveresult, qfalse, NULL );
	//
	// Retreat?
	if ( cs->script.castScriptStatus.scriptNoMoveTime < level.time && AICast_WantToRetreat( cs ) ) {
		if ( AICast_GetTakeCoverPos( cs, cs->enemyNum, cs->perception.vislist[cs->enemyNum].visible_pos,
									 cs->combat.takeCoverPos ) ) {
			// go to a position that cannot be seen from the last place we saw the enemy, and wait there for some time
			cs->combat.takeCoverTime = level.time + 2000 + rand() % 3000;
			return AIFunc_BattleTakeCoverStart( cs );
		}
	}
	//
	// Lob a Grenade?
	// if we haven't thrown a grenade in a bit, go into "grenade flush mode"
	if ( ( ai_encounter.lastGrenadeFlushTime > level.time || ai_encounter.lastGrenadeFlushTime < level.time - 7000 ) &&
		 ( cs->aiState >= AISTATE_COMBAT ) && ( cs->script.castScriptStatus.castScriptEventIndex < 0 ) &&
		 ( cs->grenade.startGrenadeFlushTime < level.time - 3000 ) &&
		 ( COM_BitCheck( cs->clientState->cur_ps.weapons, WP_GRENADE_LAUNCHER ) ) &&
		 ( AICast_GotEnoughAmmoForWeapon( cs, WP_GRENADE_LAUNCHER ) ) &&
		 ( cs->combat.weaponFireTimes[WP_GRENADE_LAUNCHER] < level.time - (int)( aicast_skillscale * 3000 ) ) &&
		 ( ( cs->weaponNum == WP_GRENADE_LAUNCHER ) ||
		   !( cs->script.castScriptStatus.scriptFlags & SFL_NOCHANGEWEAPON ) ) &&
		 ( Distance( cs->clientState->origin, cs->perception.vislist[cs->enemyNum].real_visible_pos ) > 100 ) &&
		 ( Distance( cs->clientState->origin, cs->perception.vislist[cs->enemyNum].real_visible_pos ) < 2000 ) ) {
		// try and flush them out with a grenade
		//G_Printf("pineapple?\n");
		return AIFunc_GrenadeFlushStart( cs );
	}
	if ( ( ai_encounter.lastGrenadeFlushTime > level.time || ai_encounter.lastGrenadeFlushTime < level.time - 7000 ) &&
		 ( cs->aiState >= AISTATE_COMBAT ) && ( cs->script.castScriptStatus.castScriptEventIndex < 0 ) &&
		 ( cs->grenade.startGrenadeFlushTime < level.time - 3000 ) &&
		 ( COM_BitCheck( cs->clientState->cur_ps.weapons, WP_GRENADE_PINEAPPLE ) ) &&
		 ( AICast_GotEnoughAmmoForWeapon( cs, WP_GRENADE_PINEAPPLE ) ) &&
		 ( cs->combat.weaponFireTimes[WP_GRENADE_PINEAPPLE] < level.time - (int)( aicast_skillscale * 3000 ) ) &&
		 ( ( cs->weaponNum == WP_GRENADE_PINEAPPLE ) ||
		   !( cs->script.castScriptStatus.scriptFlags & SFL_NOCHANGEWEAPON ) ) &&
		 ( Distance( cs->clientState->origin, cs->perception.vislist[cs->enemyNum].real_visible_pos ) > 100 ) &&
		 ( Distance( cs->clientState->origin, cs->perception.vislist[cs->enemyNum].real_visible_pos ) < 2000 ) ) {
		// try and flush them out with a grenade
		//G_Printf("pineapple?\n");
		return AIFunc_GrenadeFlushStart( cs );
	}
	//
	// Dodge enemy aim?
	if ( ( cs->attributes[AGGRESSION] < 1.0 ) && ( ent->client->ps.weapon ) &&
		 ( ent->client->ps.groundEntityNum == ENTITYNUM_WORLD ) &&
		 ( !cs->lastRollMove || cs->lastRollMove < level.time - 4000 ) && ( cs->attributes[TACTICAL] > 0.5 ) &&
		 ( cs->aiFlags & AIFL_ROLL_ANIM ) && ( VectorLength( cs->clientState->cur_ps.velocity ) < 1 ) ) {
		vec3_t aim, enemyVec, right;
		// are they aiming at us?
		AngleVectors( enemy->client->ps.viewangles, aim, right, NULL );
		VectorSubtract( cs->clientState->origin, enemy->r.currentOrigin, enemyVec );
		VectorNormalize( enemyVec );
		// if they are looking at us, we should avoid them
		if ( DotProduct( aim, enemyVec ) > 0.97 ) {
			aicast_predictmove_t move;
			vec3_t dir;
			bot_input_t bi, bi_back;
			usercmd_t ucmd;
			float simTime = 0.8;

			cs->lastRollMove = level.time;

			trap_EA_GetInput( cs->entityNum, (float)level.time / 1000, &bi_back );
			trap_EA_ResetInput( cs->entityNum, NULL );
			if ( level.time % 200 < 100 ) {
				VectorNegate( right, dir );
			} else {
				VectorCopy( right, dir );
			}
			trap_EA_Move( cs->entityNum, dir, 400 );
			trap_EA_GetInput( cs->entityNum, (float)level.time / 1000, &bi );
			VectorCopy( dir, bi.dir );
			AICast_InputToUserCommand( cs, &bi, &ucmd, bs->cur_ps.delta_angles );
			AICast_PredictMovement( cs, 4, simTime / 4, &move, &ucmd, cs->enemyNum );

			trap_EA_ResetInput( cs->entityNum, &bi_back );

			if ( move.groundEntityNum == ENTITYNUM_WORLD && VectorDistance( move.endpos, cs->clientState->origin ) >
																simTime * cs->attributes[RUNNING_SPEED] * 0.8 ) {
				// good enough
				if ( AICast_CheckAttackAtPos(
						 cs->entityNum, cs->enemyNum, move.endpos,
						 cs->clientState->cur_ps.viewheight == cs->clientState->cur_ps.crouchViewHeight, qfalse ) ) {
					cs->combat.takeCoverTime = 0;
					return AIFunc_BattleRollStart( cs, dir );
				}
			}
		}
	}
	//
	// reload?
	if ( ( cs->clientState->cur_ps.weaponstate != WEAPON_RELOADING ) &&
		 ( cs->clientState->cur_ps.ammoclip[BG_FindClipForWeapon( cs->clientState->cur_ps.weapon )] <
		   (int)( ammoTable[cs->clientState->cur_ps.weapon].uses ) ) ) {
		if ( AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
			trap_EA_Reload( cs->entityNum );
		} else { // no ammo, switch?
			AICast_ChooseWeapon( cs, qfalse );
			if ( cs->weaponNum == WP_NONE ) {
				// no ammo, get out of here
				return AIFunc_DefaultStart( cs );
			}
			if ( !AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
				// no ammo, get out of here
				return AIFunc_DefaultStart( cs );
			}
		}
	} else {
		//attack the enemy if possible
		AICast_ProcessAttack( cs );
	}
	//
	return NULL;
}

/*
============
AIFunc_BattleStart()
============
*/
char *AI_EnterCombat( cast_state_t *cs ) {
	// make sure we don't avoid any areas when we start again
	trap_BotInitAvoidReach( cs->clientState->ms );
	// wait some time before taking cover again
	cs->combat.takeCoverTime = level.time + 300 + rand() % ( 2000 + (int)( 2000.0 * cs->attributes[AGGRESSION] ) );
	// wait some time before going to a combat spot
	cs->combat.combatSpotDelayTime = level.time + 1500 + rand() % 2500;
	//
	// start a crouch attack?
	if ( ( random() * 3.0 + 1.0 < cs->attributes[ATTACK_CROUCH] ) &&
		 AICast_RequestCrouchAttack( cs, cs->clientState->origin, 0.0 ) ) {
		cs->aiFlags |= AIFL_ATTACK_CROUCH;
	} else {
		cs->attackcrouch_time = 0;
		cs->aiFlags &= ~AIFL_ATTACK_CROUCH;
	}
	//
	cs->lastEnemy = cs->enemyNum;
	cs->combat.startAttackCount++;
	cs->crouchHideFlag = qfalse;
	//
	// get out of talking state
	cs->aiFlags &= ~AIFL_TALKING;
	//
	//update the attack inventory values
	AICast_UpdateBattleInventory( cs, cs->enemyNum );
	//
	const bool facingEnemy = fabs( AI_AngleDifference( cs->ideal_viewangles[YAW], cs->viewangles[YAW] ) ) < 10;
	// Every rejected slot is tried at most once, even if weapon selection returns it again.
	bool attempted[3] = {};
	for ( int attempt = 0; attempt < 4; attempt++ ) {
		AICast_ChooseWeapon( cs, facingEnemy ? qtrue : qfalse );
		const int weapon = cs->weaponNum;
		if ( weapon < WP_MONSTER_ATTACK1 || weapon > WP_MONSTER_ATTACK3 ) {
			return "Combat";
		}
		const int slot = weapon - WP_MONSTER_ATTACK1;
		if ( !facingEnemy || attempted[slot] ) {
			break;
		}
		attempted[slot] = true;
		if ( AI_FindAbility( cs, weapon ) ) {
			if ( !AICast_CheckAttack( cs, cs->enemyNum, qfalse ) ) {
				return AIFunc_BattleChaseStart( cs );
			}
			if ( char *transition = AI_StartAbility( cs, weapon ) ) {
				return transition;
			}
		}
		cs->combat.weaponFireTimes[weapon] = level.time;
	}
	return AIFunc_BattleChaseStart( cs );
}

/*
==================
AICast_UpdateBattleInventory
==================
*/
void AICast_UpdateBattleInventory( cast_state_t *cs, int enemy ) {
	vec3_t dir;
	int i;

	if ( enemy >= 0 ) {
		VectorSubtract( cs->perception.vislist[cs->enemyNum].visible_pos, cs->clientState->origin, dir );
		cs->combat.enemyHeight = (int)dir[2];
		cs->combat.enemyDist = (int)VectorLength( dir );
	}

	// stock up ammo that should never run out
	for ( i = 0; i < WP_NUM_WEAPONS; i++ ) {
		if ( ( i >= WP_MONSTER_ATTACK1 && i <= WP_MONSTER_ATTACK3 ) ||
			 ( g_entities[cs->clientState->entitynum].client->ps.ammo[BG_FindAmmoForWeapon( i )] > 800 ) ) {
			//g_entities[cs->clientState->entitynum].client->ps.ammo[ BG_FindAmmoForWeapon(i)] = 999;
			Add_Ammo( &g_entities[cs->entityNum], i, 999, qfalse );
		}
	}

	AI_GetClientState( cs->entityNum, &( cs->clientState->cur_ps ) );
}

/*
==================
AICast_Aggression

  Check all possible reasons why we shouldn't attack, returning a value from 1.0 (fully willing)
  to 0.0 (please don't hurt me).
==================
*/
float AICast_Aggression( cast_state_t *cs ) {
	ai_client_state_t *bs;
	float scale, dist;
	int painTime;
	int *ammo;

	bs = cs->clientState;

	// if we are out of ammo, we should never chase
	ammo = cs->clientState->cur_ps.ammo;
	if ( g_entities[cs->entityNum].aiTeam != AITEAM_MONSTER ) {
		if ( !AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
			return 0;
		}
	}

	// start fully willing to attack
	scale = 1.0;

	//if the enemy is located way higher
	//if (cs->combat.enemyHeight > 200)
	//	scale -= (cs->combat.enemyHeight)/800.0;

	//if very low on health
	if ( bs->cur_ps.stats[STAT_HEALTH] < 50 ) {
		scale -= ( 1.0 - cs->attributes[AGGRESSION] ) * ( 1.0 - ( (float)bs->cur_ps.stats[STAT_HEALTH] / 50.0 ) );
	}

	// if they've recently hit us, factor that in, so we get scared off by being
	// damaged, but later return once we've regained our confidence
	painTime = 15000 - (int)( 10000.0 * cs->attributes[AGGRESSION] * cs->attributes[AGGRESSION] );
	if ( cs->lastPain + painTime > level.time ) {
		scale -= 3 * ( 1.0 - cs->attributes[AGGRESSION] ) *
				 ( (float)( cs->lastPain + painTime - level.time ) / (float)painTime );
	}

	// if we just rolled, stay out of view if we jumped behind cover
	painTime = 10000 - (int)( 10000.0 * cs->attributes[AGGRESSION] * cs->attributes[AGGRESSION] );
	if ( cs->battleRollTime + painTime > level.time ) {
		scale -= 2 * ( 1.0 - cs->attributes[AGGRESSION] ) *
				 ( (float)( cs->battleRollTime + painTime - level.time ) / (float)painTime );
	}

	// gain in confidence the further we are away
	if ( cs->enemyNum >= 0 ) {
		dist = Distance( cs->clientState->origin, g_entities[cs->enemyNum].s.pos.trBase );
		//if (dist > 512) {
		scale += ( dist - 800.0 ) / ( 8000.0 );
		//}
	}

	// if our weapon is reloading, we should hide
	if ( cs->clientState->cur_ps.weaponTime > 0 ) {
		scale -= ( (float)cs->clientState->cur_ps.weaponTime / 1000.0 );
	}

	scale *= cs->attributes[AGGRESSION];

	// this should increase the chances of an ambush attack
	if ( cs->entityNum >= 0 && ( ( level.time + 2000 * g_entities[cs->entityNum].aiTeam ) %
								 ( 4000 + 500 * g_entities[cs->entityNum].aiTeam ) ) > 4000 ) {
		if ( cs->perception.vislist[cs->entityNum].visible_timestamp > level.time - 10000 ) {
			scale += 0.3 * (float)( level.time - cs->perception.vislist[cs->entityNum].visible_timestamp ) / 10000.0;
		}
	}

	if ( scale < 0 ) {
		scale = 0;
	}

	return scale;
}

/*
==================
AICast_WantsToChase
==================
*/
int AICast_WantsToChase( cast_state_t *cs ) {
	int *ammo;
	ammo = cs->clientState->cur_ps.ammo;
	if ( g_entities[cs->entityNum].aiTeam != AITEAM_MONSTER ) {
		if ( !AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
			return qfalse;
		}
	}
	if ( cs->attributes[AGGRESSION] == 1.0 ) {
		return qtrue;
	}
	if ( AICast_Aggression( cs ) > 0.6 ) {
		return qtrue;
	}
	return qfalse;
}

/*
==================
AICast_WantsToTakeCover
==================
*/
int AICast_WantsToTakeCover( cast_state_t *cs, qboolean attacking ) {
	float aggrScale;
	int *ammo;

	ammo = cs->clientState->cur_ps.ammo;
	if ( g_entities[cs->entityNum].aiTeam != AITEAM_MONSTER ) {
		if ( !cs->weaponNum ) {
			return qtrue;
		}
		if ( !AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
			return qtrue;
		}
	}
	if ( cs->attributes[AGGRESSION] == 1.0 ) {
		return qfalse;
	}
	// if currently attacking, we should stick around if not getting hurt
	if ( attacking ) {
		aggrScale = 1.2;
	} else {
		aggrScale = 0.8 /*+ 0.4 * random()*/;
	}
	//
	// if currently following someone, we should be more aggressive
	if ( cs->leaderNum >= 0 ) {
		aggrScale *= 3;
	}
	//
	// Dodge enemy aim?
	if ( cs->attributes[AGGRESSION] < 1.0 && attacking && ( cs->enemyNum >= 0 ) &&
		 ( g_entities[cs->enemyNum].client->ps.weapon ) && ( cs->attributes[TACTICAL] > 0.5 ) &&
		 ( cs->aiFlags & AIFL_ROLL_ANIM ) && ( VectorLength( cs->clientState->cur_ps.velocity ) < 1 ) ) {
		vec3_t aim, enemyVec;
		// are they aiming at us?
		AngleVectors( g_entities[cs->enemyNum].client->ps.viewangles, aim, NULL, NULL );
		VectorSubtract( cs->clientState->origin, g_entities[cs->enemyNum].r.currentOrigin, enemyVec );
		VectorNormalize( enemyVec );
		// if they are looking at us, we should avoid them
		if ( DotProduct( aim, enemyVec ) > 0.97 ) {
			//G_Printf("%s: I'm in danger, I should probably avoid\n", g_entities[cs->entityNum].aiName);
			aggrScale *= 0.6;
		}
	}
	//
	// FIXME: instead of a constant, call a "attack danger"
	// function, so we only attack if our aggression is greater than
	// the danger
	if ( AICast_Aggression( cs ) * aggrScale < 0.4 ) {
		//G_Printf("%s: run for your life!\n", g_entities[cs->entityNum].aiName);
		return qtrue;
	}
	//
	return qfalse;
}

/*
==============
AICast_AIDamageOK
==============
*/
qboolean AICast_AIDamageOK( cast_state_t *cs, cast_state_t *ocs ) {
	if ( cs->script.castScriptStatus.scriptFlags & SFL_NOAIDAMAGE ) {
		return qfalse;
	} else {

		if ( cs->aiCharacter == AICHAR_LOPER && ocs->aiCharacter == AICHAR_LOPER ) {
			return qfalse;
		}

		return qtrue;
	}
}

/*
==============
AICast_WantToRetreat
==============
*/
qboolean AICast_WantToRetreat( cast_state_t *cs ) {
	int *ammo;

	ammo = cs->clientState->cur_ps.ammo;
	if ( g_entities[cs->entityNum].aiTeam != AITEAM_MONSTER ) {
		if ( !cs->weaponNum ) {
			return qtrue;
		}
		if ( !AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
			return qtrue;
		}
	}

	if ( cs->attributes[AGGRESSION] >= 1.0 && cs->attributes[TACTICAL] <= 0.2 ) {
		return qfalse;
	}

	// RF, (Last Minute Hack) big dudes should never retreat
	if ( cs->aasWorldIndex != 0 ) {
		return qfalse;
	}

	if ( cs->leaderNum < 0 ) {
		if ( ( cs->attributes[TACTICAL] > 0.11 + random() * 0.5 ) &&
			 ( ( cs->clientState->cur_ps.weaponTime > 500 ) ||
			   ( ( cs->combat.takeCoverTime < level.time - 100 ) && ( AICast_WantsToTakeCover( cs, qtrue ) ) ) ) ) {
			return qtrue;
		}
	}
	//
	return qfalse;
}

qboolean AICast_AllowFlameDamage( int entNum ) {
	// DHM - Nerve :: caststates are not initialized in multiplayer
	if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
		return qtrue;
	}
	// dhm

	if ( caststates[entNum].aiFlags & AIFL_NO_FLAME_DAMAGE ) {
		return qfalse;
	}
	return qtrue;
}

char *AIFunc_BattleStart( cast_state_t *cs ) {
	return AI_RequestState( cs, AIStateId::Combat, AITransitionReason::Decision );
}
