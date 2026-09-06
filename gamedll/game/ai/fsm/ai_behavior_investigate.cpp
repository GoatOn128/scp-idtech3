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
AIFunc_InspectFriendly()
============
*/
char *AIFunc_InspectFriendly( cast_state_t *cs ) {
	int enemies[MAX_CLIENTS];
	bot_moveresult_t *moveresult;
	gentity_t *followent, *ent;
	ai_client_state_t *bs;
	vec3_t destorg;
	float dist;
	qboolean moved = qfalse;

	ent = &g_entities[cs->entityNum];

	// if we have an enemy, attack now!
	if ( cs->enemyNum >= 0 ) {
		return AIFunc_BattleStart( cs );
	}

	cs->navigation.followEntity = cs->inspectNum;
	cs->navigation.followDist = 64;

	cs->script.scriptPauseTime =
		level.time + 4000; // wait for at least this long before resuming any scripted walking, etc

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

	if ( followent->client ) {
		VectorCopy( followent->client->ps.origin, destorg );
	} else {
		VectorCopy( followent->r.currentOrigin, destorg );
	}

	// they are ready, are they inside range? FIXME: make configurable
	dist = Distance( destorg, cs->clientState->origin );
	if ( !( dist < cs->navigation.followDist &&
			( ent->waterlevel || ( cs->clientState->cur_ps.groundEntityNum != ENTITYNUM_NONE ) ) ) ) {
		//
		// go to them
		//
		bs = cs->clientState;

		// set this flag so we know when we;ve just reached them
		cs->aiFlags |= AIFL_MISCFLAG1;

		// move straight to them if we can
		if ( !moved && ( cs->clientState->cur_ps.groundEntityNum != ENTITYNUM_NONE ||
						 g_entities[cs->entityNum].waterlevel > 1 ) ) {
			aicast_predictmove_t move;
			vec3_t dir;
			bot_input_t bi;
			usercmd_t ucmd;
			trace_t tr;
			qboolean simTest = qfalse;

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
				AICast_PredictMovement( cs, 10, 0.8, &move, &ucmd, cs->navigation.followEntity );

				if ( move.stopevent == PREDICTSTOP_HITENT ) { // success!
					trap_EA_Move( cs->entityNum, dir, 400 ); // set the movement
					vectoangles( dir, cs->ideal_viewangles );
					cs->ideal_viewangles[2] *= 0.5;
					moved = qtrue;
				} else { // clear movement
					//trap_EA_Move(cs->entityNum, dir, 0);
				}
			}
		}
		//
		if ( !moved ) {
			// use AAS routing
			moveresult = AICast_MoveToPos( cs, followent->r.currentOrigin, cs->navigation.followEntity );
			// if we cant get there, face the path to the enemy
			if ( !moveresult || moveresult->failure ) {
				// if we can get a visible target, then face it
				if ( !( cs->aiFlags & AIFL_MISCFLAG2 ) ) {
					if ( trap_AAS_GetRouteFirstVisPos( followent->r.currentOrigin, cs->clientState->origin,
													   cs->travelflags, cs->combat.takeCoverEnemyPos ) ) {
						cs->aiFlags |= AIFL_MISCFLAG2;
					} else {
						// if it failed, just use their origin for now, but keep checking
						VectorCopy( followent->r.currentOrigin, cs->combat.takeCoverEnemyPos );
					}
				}
				VectorSubtract( cs->combat.takeCoverEnemyPos, cs->clientState->origin, destorg );
				VectorNormalize( destorg );
				vectoangles( destorg, cs->ideal_viewangles );
			}
		}

		// should we slow down?
		if ( cs->navigation.followDist && cs->navigation.followSlowApproach ) {
			cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, cs->navigation.followDist );
		}
		/*
		// check for a movement we should be making
		if (cs->navigation.obstructingTime > level.time)
		{
			AICast_MoveToPos( cs, cs->navigation.obstructingPos, -1 );
			if (cs->movestate != MS_CROUCH) {
				cs->movestate = MS_WALK;
			}
			cs->movestateType = MSTYPE_TEMPORARY;
		}
*/
	} else if ( cs->aiFlags & AIFL_MISCFLAG1 ) {
		cs->aiFlags &= ~AIFL_MISCFLAG1;
		if ( g_entities[cs->inspectNum].health <= 0 ) {
			// call a script event
			cs->aiFlags &= ~AIFL_DENYACTION;
			AICast_ForceScriptEvent( cs, "inspectbodyend", g_entities[cs->inspectNum].aiName );
			if ( cs->aiFlags & AIFL_DENYACTION ) {
				// relinguish control back to scripting
				return AIFunc_DefaultStart( cs );
			}
		} else { // force a visibility update so we get their vis also
			AICast_UpdateVisibility( ent, &g_entities[cs->inspectNum], qtrue, qtrue );
		}
	}

	{
		int numEnemies;
		//
		// look for things we should attack
		numEnemies = AICast_ScanForEnemies( cs, enemies );
		if ( numEnemies == -1 ) { // query mode
			return NULL;
		} else if ( numEnemies == -2 ) { // inspection
			// only override current objective if we are inspecting a dead guy, and the new inspect target is fighting someone
			if ( ( g_entities[cs->inspectNum].health <= 0 ) && ( g_entities[enemies[0]].health > 0 ) ) {
				return AIFunc_InspectFriendlyStart( cs, enemies[0] );
			}
		}
		// RF, disabled this, if we are interrupted, scripts might not work right, and anyway, this is only a bullet, not as if it's a dead guy or anything
		//else if (numEnemies == -3)	// bullet impact
		//{
		//	if (cs->aiState < AISTATE_COMBAT) {
		//		return AIFunc_InspectBulletImpactStart( cs );
		//	}
		//}
		else if ( numEnemies > 0 ) {
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

			return AIFunc_BattleStart( cs );
		}
	}

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
AIFunc_InspectFriendlyStart
============
*/
char *AIFunc_InspectFriendlyStart( cast_state_t *cs, int entnum ) {
	cast_state_t *ocs;

	ocs = AICast_GetCastState( entnum );

	// we are about to deal with the request for inspection
	cs->perception.vislist[entnum].flags &= ~AIVIS_INSPECT;
	cs->script.scriptPauseTime =
		level.time + 4000; // wait for at least this long before resuming any scripted walking, etc

	//
	cs->aiFlags &= ~AIFL_MISCFLAG2;

	if ( ocs->aiState >= AISTATE_COMBAT || g_entities[entnum].health <= 0 ) {
		// mark this character as having been inspected
		cs->perception.vislist[entnum].flags |= AIVIS_INSPECTED;
	}

	// what should we do? wait here? hide? go see them?

	// if dead, go see them
	if ( g_entities[entnum].health <= 0 ) {
		cs->inspectNum = entnum;
		AI_SetLegacyState( cs, AIFunc_InspectFriendly );
		return "AIFunc_InspectFriendlyStart";
	}

	// not dead, so call scripting event
	AICast_ForceScriptEvent( cs, "inspectfriendlycombatstart", g_entities[entnum].aiName );
	if ( cs->aiFlags & AIFL_DENYACTION ) {
		// ignore this friendly forever and ever amen
		cs->perception.vislist[entnum].flags |= AIVIS_INSPECTED;
		return NULL;
	}

	// if they are in combat, then act according to aggressiveness
	if ( ocs->aiState >= AISTATE_COMBAT ) {
		if ( cs->attributes[AGGRESSION] < 0.3 ) {
			if ( AICast_GetTakeCoverPos( cs, entnum, g_entities[entnum].client->ps.origin, cs->combat.takeCoverPos ) ) {
				cs->combat.takeCoverTime = level.time + 10000; // hide for 10 seconds
				cs->script.scriptPauseTime = cs->combat.takeCoverTime;
				// crouch there if possible
				if ( cs->attributes[ATTACK_CROUCH] > 0.1 ) {
					cs->attackcrouch_time = level.time + 3000;
				}
				return AIFunc_BattleTakeCoverStart( cs );
			}
		}
	}

	// if still around, then we need to go to them
	cs->inspectNum = entnum;
	AI_SetLegacyState( cs, AIFunc_InspectFriendly );
	return "AIFunc_InspectFriendly";
}

/*
============
AIFunc_InspectBulletImpact()
============
*/
char *AIFunc_InspectBulletImpact( cast_state_t *cs ) {
	int enemies[MAX_CLIENTS];
	gentity_t *ent;
	vec3_t v1;
	gclient_t *client;
	//
	client = &level.clients[cs->entityNum];
	//
	ent = &g_entities[cs->entityNum];
	//
	cs->perception.bulletImpactIgnoreTime = level.time + 800;
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
	// wait until we are looking at the impact
	if ( cs->aiFlags & AIFL_MISCFLAG2 ) {
		// pause any scripting
		cs->script.scriptPauseTime = level.time + 1000;
		// look at bullet impact
		VectorSubtract( cs->perception.bulletImpactEnd, cs->clientState->origin, v1 );
		VectorNormalize( v1 );
		vectoangles( v1, cs->ideal_viewangles );
		//
		// if we are facing that direction, we've looked at the impact point
		if ( fabs( cs->ideal_viewangles[YAW] - cs->viewangles[YAW] ) < 1 ) {
			cs->aiFlags &= ~AIFL_MISCFLAG2;
		}
		return NULL;
	} else if ( cs->aiFlags & AIFL_MISCFLAG1 ) {
		// clear the flag now
		cs->aiFlags &= ~AIFL_MISCFLAG1;
		// start looking back at bullet
		VectorSubtract( cs->perception.bulletImpactStart, cs->clientState->origin, v1 );
		VectorNormalize( v1 );
		vectoangles( v1, cs->ideal_viewangles );
		if ( cs->aiState < AISTATE_ALERT ) {
			// change to alert state
			if ( !AICast_StateChange( cs, AISTATE_ALERT ) ) {
				if ( cs->lastEnemy < 0 && cs->enemyNum < 0 ) {
					// look back at our original angles
					VectorCopy( ent->s.angles, cs->ideal_viewangles );
				}
				// stop doing whatever we are doing, and return control to scripting
				cs->script.scriptPauseTime = 0;
				return AIFunc_IdleStart( cs );
			}
			// make sure we didnt change thinkfunc
			if ( cs->currentState != AIFunc_InspectBulletImpact ) {
				//G_Error( "scripting passed control out of AIFunc_InspectBulletImpact(), this is bad" );
				return NULL;
			}
		}
		// pause any scripting
		if ( ent->client->ps.legsTimer ) {
			cs->script.scriptPauseTime = level.time + ent->client->ps.legsTimer;
		} else { // just wait for a few seconds looking at the source
			cs->script.scriptPauseTime = level.time + 3500;
		}
	}
	// are we done?
	if ( cs->script.scriptPauseTime < level.time ) {
		if ( cs->lastEnemy < 0 && cs->enemyNum < 0 ) {
			// look back at our original angles
			VectorCopy( ent->s.angles, cs->ideal_viewangles );
		}
		return AIFunc_IdleStart( cs );
	}
	//
	// reload?
	AICast_IdleReload( cs );
	//
	// check for enemies
	{
		int numEnemies;
		//
		// look for things we should attack
		numEnemies = AICast_ScanForEnemies( cs, enemies );
		if ( numEnemies == -2 ) { // inspection
			// only override current objective if we are inspecting a dead guy, and the new inspect target is fighting someone
			if ( ( g_entities[cs->inspectNum].health <= 0 ) && ( g_entities[enemies[0]].health > 0 ) ) {

				return AIFunc_InspectFriendlyStart( cs, enemies[0] );
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

			return AIFunc_BattleStart( cs );
		}
	}
	//
	return NULL;
}

/*
============
AIFunc_InspectBulletImpactStart()
============
*/
char *AIFunc_InspectBulletImpactStart( cast_state_t *cs ) {
	int oldScriptIndex;
	// set the impact timer so we ignore bullets while inspecting this one
	cs->perception.bulletImpactIgnoreTime = level.time + 5000;
	// pause any scripting
	cs->script.scriptPauseTime = level.time + 1000;
	// set this so we know if we've started the trace back to the bullet origin
	cs->aiFlags |= AIFL_MISCFLAG1;
	cs->aiFlags |= AIFL_MISCFLAG2;
	//
	// call the script event
	oldScriptIndex = cs->script.scriptCallIndex;
	AICast_ScriptEvent( cs, "bulletimpactsound", "" );
	if ( oldScriptIndex == cs->script.scriptCallIndex ) {
		// no script event, so call the animation script
		BG_AnimScriptEvent( &g_entities[cs->entityNum].client->ps, ANIM_ET_BULLETIMPACT, qfalse, qtrue );
	}
	//
	// if the origin is not visible, set the bullet origin to the closest visible area from the src
	if ( !trap_InPVS( cs->perception.bulletImpactStart, cs->clientState->origin ) ) {
		// if it fails, then just look at the source
		trap_AAS_GetRouteFirstVisPos( g_entities[cs->perception.bulletImpactEntity].s.pos.trBase,
									  cs->clientState->origin, cs->travelflags, cs->perception.bulletImpactStart );
	}
	//
	AI_SetLegacyState( cs, AIFunc_InspectBulletImpact );
	return "AIFunc_InspectBulletImpact";
}

/*
============
AIFunc_InspectAudibleEvent()
============
*/
char *AIFunc_InspectAudibleEvent( cast_state_t *cs ) {
	int enemies[MAX_CLIENTS];
	bot_moveresult_t *moveresult;
	gentity_t *ent;
	ai_client_state_t *bs;
	vec3_t destorg, vec;
	float dist;
	qboolean moved = qfalse;

	ent = &g_entities[cs->entityNum];

	// if we have an enemy, attack now!
	if ( cs->enemyNum >= 0 ) {
		return AIFunc_BattleStart( cs );
	}

	cs->navigation.followDist = 64;

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

	// are we just looking for now?
	if ( cs->aiFlags & AIFL_MISCFLAG2 ) {
		if ( cs->script.scriptPauseTime <= level.time ) {
			return AIFunc_DefaultStart( cs );
		}
		return NULL;
	}

	VectorCopy( cs->perception.audibleEventOrg, destorg );

	// they are ready, are they inside range? FIXME: make configurable
	dist = Distance( destorg, cs->clientState->origin );
	if ( !( dist < cs->navigation.followDist &&
			( ent->waterlevel || ( cs->clientState->cur_ps.groundEntityNum != ENTITYNUM_NONE ) ) ) ) {
		//
		// go to them
		//
		bs = cs->clientState;

		// set this flag so we know when we;ve just reached them
		cs->aiFlags |= AIFL_MISCFLAG1;

		// if not overly aggressive, pursue with caution
		if ( cs->attributes[AGGRESSION] <= 0.8 ) {
			cs->movestate = MS_CROUCH;
			cs->movestateType = MSTYPE_TEMPORARY;
		}

		// move straight to them if we can
		if ( !moved && ( cs->clientState->cur_ps.groundEntityNum != ENTITYNUM_NONE ||
						 g_entities[cs->entityNum].waterlevel > 1 ) ) {
			aicast_predictmove_t move;
			vec3_t dir;
			bot_input_t bi;
			usercmd_t ucmd;
			trace_t tr;
			qboolean simTest = qfalse;

			if ( cs->attributes[RUNNING_SPEED] < 120 ) {
				simTest = qtrue;
			}

			if ( !simTest ) {
				// trace will eliminate most unsuccessful paths
				trap_Trace( &tr, cs->clientState->origin, NULL /*g_entities[cs->entityNum].r.mins*/,
							NULL /*g_entities[cs->entityNum].r.maxs*/, destorg, cs->entityNum,
							g_entities[cs->entityNum].clipmask );
				if ( tr.fraction == 1 ) {
					simTest = qtrue;
				}
			}

			if ( simTest ) {
				// try walking straight to them
				gentity_t *gent;
				//
				gent = G_Spawn();
				VectorCopy( destorg, gent->r.currentOrigin );
				//
				VectorSubtract( destorg, cs->clientState->origin, dir );
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
				AICast_PredictMovement( cs, 10, 0.8, &move, &ucmd, gent->s.number );
				//
				if ( move.stopevent == PREDICTSTOP_HITENT ) { // success!
					trap_EA_Move( cs->entityNum, dir, 400 );
					vectoangles( dir, cs->ideal_viewangles );
					cs->ideal_viewangles[2] *= 0.5;
					moved = qtrue;
				} else { // clear movement
					//trap_EA_Move(cs->entityNum, dir, 0);
				}
				//
				G_FreeEntity( gent );
			}
		}
		//
		if ( !moved ) {
			// use AAS routing
			moveresult = AICast_MoveToPos( cs, destorg, -1 );
			// if we cant get there, do something else
			if ( moveresult && moveresult->failure ) {

				// if we can get a visible target, then face it
				if ( trap_AAS_GetRouteFirstVisPos( cs->perception.audibleEventOrg, cs->clientState->origin,
												   cs->travelflags, destorg ) ) {
					cs->aiFlags |= AIFL_MISCFLAG2;
					VectorSubtract( destorg, cs->clientState->origin, destorg );
					VectorNormalize( destorg );
					vectoangles( destorg, cs->ideal_viewangles );
					return NULL;
				}

				if ( cs->lastEnemy < 0 && cs->enemyNum < 0 ) {
					// look back at our original angles
					VectorCopy( ent->s.angles, cs->ideal_viewangles );
				}
				return AIFunc_DefaultStart( cs );
			} else if ( !moveresult ) { // face it?
				if ( trap_InPVS( destorg, cs->clientState->origin ) ) {
					VectorSubtract( destorg, cs->clientState->origin, vec );
					VectorNormalize( vec );
					vectoangles( vec, cs->ideal_viewangles );
				}
			}
		}

		// should we slow down?
		if ( cs->navigation.followDist && cs->navigation.followSlowApproach ) {
			cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, cs->navigation.followDist );
		}
		/*
		// check for a movement we should be making
		if (cs->navigation.obstructingTime > level.time)
		{
			AICast_MoveToPos( cs, cs->navigation.obstructingPos, -1 );
			if (cs->movestate != MS_CROUCH) {
				cs->movestate = MS_WALK;
			}
			cs->movestateType = MSTYPE_TEMPORARY;
		}
*/
	} else if ( cs->aiFlags & AIFL_MISCFLAG1 ) {
		cs->aiFlags &= ~AIFL_MISCFLAG1;
		// call a script event
		cs->aiFlags &= ~AIFL_DENYACTION;
		AICast_ForceScriptEvent( cs, "inspectsoundend", g_entities[cs->perception.audibleEventEnt].aiName );
		if ( cs->aiFlags & AIFL_DENYACTION ) {
			// relinguish control back to scripting
			return AIFunc_DefaultStart( cs );
		}
	} else {
		// look around randomly
		if ( cs->combat.battleHuntViewTime < level.time ) {
			cs->combat.battleHuntViewTime = level.time + 700 + rand() % 1000;
			// set a random viewangle
			cs->ideal_viewangles[YAW] =
				AngleMod( cs->ideal_viewangles[YAW] + ( 45.0 + random() * 45.0 ) * ( 2 * ( rand() % 2 ) - 1 ) );
		}
		//
		if ( cs->script.scriptPauseTime < level.time ) {
			// we're done waiting around here
			if ( cs->lastEnemy < 0 && cs->enemyNum < 0 ) {
				// look back at our original angles
				VectorCopy( ent->s.angles, cs->ideal_viewangles );
			}
			return AIFunc_DefaultStart( cs );
		}
	}

	{
		int numEnemies;
		//
		// look for things we should attack
		numEnemies = AICast_ScanForEnemies( cs, enemies );
		if ( numEnemies == -1 ) { // query mode
			return NULL;
		} else if ( numEnemies == -2 ) { // inspection
			// only override current objective if we are inspecting a dead guy, and the new inspect target is fighting someone
			if ( ( g_entities[cs->inspectNum].health <= 0 ) && ( g_entities[enemies[0]].health > 0 ) ) {
				return AIFunc_InspectFriendlyStart( cs, enemies[0] );
			}
		} else if ( numEnemies == -4 ) { // NEW audible event
			//if (cs->aiState < AISTATE_COMBAT) {
			return AIFunc_InspectAudibleEventStart( cs, cs->perception.audibleEventEnt );
			//}
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

			return AIFunc_BattleStart( cs );
		}
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
AIFunc_InspectAudibleEventStart
============
*/
char *AIFunc_InspectAudibleEventStart( cast_state_t *cs, int entnum ) {
	cast_state_t *ocs;
	int oldScriptIndex;

	ocs = AICast_GetCastState( entnum );

	// we have now processed the audible event (whether we act on it or not)
	cs->perception.audibleEventTime = -9999;

	// trigger a script event, which has the ability to deny the request
	oldScriptIndex = cs->script.scriptCallIndex;
	AICast_ForceScriptEvent( cs, "inspectsoundstart", g_entities[cs->perception.audibleEventEnt].aiName );
	if ( cs->aiFlags & AIFL_DENYACTION ) {
		return NULL;
	}

	// if not in alert mode, go there now
	if ( cs->aiState < AISTATE_ALERT ) {
		AICast_StateChange( cs, AISTATE_ALERT );
	}

	if ( oldScriptIndex == cs->script.scriptCallIndex ) {
		BG_AnimScriptEvent( &g_entities[cs->entityNum].client->ps, ANIM_ET_INSPECTSOUND, qfalse, qtrue );
	}

	// pause the scripting for now
	cs->script.scriptPauseTime =
		level.time + 4000; // wait for at least this long before resuming any scripted walking, etc

	// set this when we decide to just look, rather than pursue
	cs->aiFlags &= ~AIFL_MISCFLAG2;

	// what should we do? wait here? hide? go see them?

	// if dead, go see them
	if ( g_entities[entnum].health <= 0 ) {
		cs->inspectNum = entnum;
		AI_SetLegacyState( cs, AIFunc_InspectFriendly );
		return "AIFunc_InspectFriendlyStart";
	}

	// if they are in combat, then act according to aggressiveness
	if ( ocs->aiState >= AISTATE_COMBAT ) {
		if ( cs->attributes[AGGRESSION] < 0.3 ) {
			if ( AICast_GetTakeCoverPos( cs, entnum, g_entities[entnum].client->ps.origin, cs->combat.takeCoverPos ) ) {
				cs->combat.takeCoverTime = level.time + 10000; // hide for 10 seconds
				cs->script.scriptPauseTime = cs->combat.takeCoverTime;
				// crouch there if possible
				if ( cs->attributes[ATTACK_CROUCH] > 0.1 ) {
					cs->attackcrouch_time = level.time + 3000;
				}
				return AIFunc_BattleTakeCoverStart( cs );
			}
		}
	}

	AI_SetLegacyState( cs, AIFunc_InspectAudibleEvent );
	return "AIFunc_InspectAudibleEvent";
}

/*
============
AIFunc_InspectBody()

  go up to the enemy, and have a good look at them, randomly taunt them
============
*/
char *AIFunc_InspectBody( cast_state_t *cs ) {
	int enemies[MAX_CLIENTS];
	int numEnemies;
	bot_moveresult_t *moveresult;
	ai_client_state_t *bs;
	vec3_t destorg, enemyOrg;
	//
	// stop crouching
	cs->attackcrouch_time = 0;
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
	// if running a script
	if ( cs->script.castScriptStatus.castScriptEventIndex >= 0 ) {
		cs->enemyNum = -1;
		return AIFunc_IdleStart( cs );
	}
	//
	bs = cs->clientState;
	//
	if ( cs->enemyNum < 0 ) {
		return AIFunc_IdleStart( cs );
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
		cs->enemyNum = enemies[0]; // just attack the first one
		return AIFunc_BattleStart( cs );
	}
	//
	VectorCopy( cs->perception.vislist[cs->enemyNum].visible_pos, enemyOrg );
	if ( ( cs->inspectBodyTime < 0 ) && ( Distance( cs->clientState->origin, enemyOrg ) > 64 ) ) {
		// if they were gibbed, don't go all the way
		if ( g_entities[cs->enemyNum].health < GIB_HEALTH && ( Distance( cs->clientState->origin, enemyOrg ) < 180 ) ) {
			cs->inspectBodyTime = level.time + 1000 + rand() % 1000;
			trap_EA_Gesture( cs->entityNum );
			G_AddEvent( &g_entities[cs->entityNum], EV_GENERAL_SOUND,
						G_SoundIndex( aiDefaults[cs->aiCharacter].soundScripts[ORDERSSOUNDSCRIPT] ) );
		}
		// walk to them
		if ( cs->movestate != MS_CROUCH ) {
			cs->movestate = MS_WALK;
		}
		cs->movestateType = MSTYPE_TEMPORARY;
		//
		moveresult = AICast_MoveToPos( cs, enemyOrg, -1 );
		//if the movement failed
		if ( moveresult && ( moveresult->failure || moveresult->blocked ) ) {
			//reset the avoid reach, otherwise bot is stuck in current area
			trap_BotResetAvoidReach( bs->ms );
			// couldn't get there, so stop trying to get there
			cs->enemyNum = -1;
			return AIFunc_IdleStart( cs );
		}
		if ( Distance( cs->clientState->origin, enemyOrg ) < 180 ) {
			// look down at them
			VectorSubtract( enemyOrg, cs->clientState->origin, destorg );
			destorg[2] -= 20;
			VectorNormalize( destorg );
			vectoangles( destorg, cs->ideal_viewangles );
		}
	} else if ( cs->inspectBodyTime < 0 ) {
		// just reached them
		cs->inspectBodyTime = level.time + 1000 + rand() % 1000;
		trap_EA_Gesture( cs->entityNum );
		G_AddEvent( &g_entities[cs->entityNum], EV_GENERAL_SOUND,
					G_SoundIndex( aiDefaults[cs->aiCharacter].soundScripts[ORDERSSOUNDSCRIPT] ) );
	} else if ( cs->inspectBodyTime < level.time ) {
		vec3_t vec;
		VectorSubtract( cs->startOrigin, cs->clientState->origin, vec );
		vec[2] = 0;
		// ready to go back to start position
		if ( VectorLength( vec ) > 64 ) {
			if ( cs->movestate != MS_CROUCH ) {
				cs->movestate = MS_WALK;
			}
			cs->movestateType = MSTYPE_TEMPORARY;
			moveresult = AICast_MoveToPos( cs, cs->startOrigin, -1 );
			//if the movement failed
			if ( moveresult && ( moveresult->failure || moveresult->blocked ) ) {
				//reset the avoid reach, otherwise bot is stuck in current area
				trap_BotResetAvoidReach( bs->ms );
				// couldn't get there, so stop trying to get there
				cs->enemyNum = -1;
				return AIFunc_IdleStart( cs );
			}
			// stay looking at them for a bit after starting to walk back
			if ( cs->inspectBodyTime + 750 > level.time ) {
				// look down at them
				VectorSubtract( enemyOrg, cs->clientState->origin, destorg );
				destorg[2] -= 20;
				VectorNormalize( destorg );
				vectoangles( destorg, cs->ideal_viewangles );
			}
		} else {
			cs->attackSNDtime = level.time;
			cs->enemyNum = -1;
			return AIFunc_IdleStart( cs );
		}
	}
	//
	return NULL;
}

/*
============
AIFunc_InspectBodyStart()
============
*/
char *AIFunc_InspectBodyStart( cast_state_t *cs ) {
	static int lastInspect;
	//
	// if an inspection was already started not long ago, forget it
	if ( lastInspect <= level.time && lastInspect > level.time - 1000 ) {
		cs->inspectBodyTime = 1; // go back to start position
	} else {
		lastInspect = level.time;
		cs->inspectBodyTime = -1;
	}
	AI_SetLegacyState( cs, AIFunc_InspectBody );
	return "AIFunc_InspectBody";
}
