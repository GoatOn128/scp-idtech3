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
AIFunc_BattleMG42()
============
*/
char *AIFunc_BattleMG42( cast_state_t *cs ) {
	int enemies[MAX_CLIENTS];
	int numEnemies;
	ai_client_state_t *bs;
	gentity_t *mg42, *ent;
	vec3_t angles, vec, bestangles;
	qboolean unmount = qfalse;

	mg42 = &g_entities[cs->mountedEntity];
	ent = &g_entities[cs->entityNum];
	bs = cs->clientState;

	// have we dismounted the MG42?
	if ( !ent->active ) {
		return AIFunc_DefaultStart( cs );
	}

	// are we waiting to dismount
	if ( cs->aiFlags & AIFL_DISMOUNTING ) {
		// face straight forward
		VectorCopy( mg42->s.angles, cs->ideal_viewangles );
		// only dismount when facing forwards
		if ( fabs( AI_AngleDifference( mg42->s.angles[YAW], cs->viewangles[YAW] ) ) < 10 ) {
			// try and unmount
			Cmd_Activate_f( ent );
		}
		return NULL;
	}

	/* RF, disabled this since the gun will unmount them when it is destroyed
	// if the mg42 is dead, dismount
	if (mg42->health <= 0) {
		cs->aiFlags |= AIFL_DISMOUNTING;
		AICast_ScriptEvent( cs, "forced_mg42_unmount", "destroyed" );
		return NULL;
	}
*/
	// if enemy is dead, stop attacking them
	if ( g_entities[cs->enemyNum].health <= 0 ) {
		cs->enemyNum = -1;
	}

	//if no enemy, or our current enemy isn't attackable, look for a better enemy
	if ( cs->enemyNum >= 0 ) {
		if ( cs->perception.vislist[cs->enemyNum].real_visible_timestamp &&
			 cs->perception.vislist[cs->enemyNum].real_visible_timestamp > ( level.time - 5000 ) ) {
			VectorSubtract( cs->perception.vislist[cs->enemyNum].real_visible_pos, mg42->r.currentOrigin, vec );
		} else if ( cs->perception.vislist[cs->enemyNum].visible_timestamp &&
					cs->perception.vislist[cs->enemyNum].visible_timestamp > ( level.time - 5000 ) ) {
			VectorSubtract( cs->perception.vislist[cs->enemyNum].visible_pos, mg42->r.currentOrigin, vec );
		} else { // just aim straight forward
			AngleVectors( mg42->s.angles, vec, NULL, NULL );
		}

		VectorNormalize( vec );
		vectoangles( vec, angles );
		angles[PITCH] = AngleNormalize180( angles[PITCH] );
		VectorCopy( angles, bestangles );
	}

	// check for enemy outside harc
	if ( cs->enemyNum < 0 || !AICast_CheckAttack( cs, cs->enemyNum, qfalse ) ||
		 ( fabs( AI_AngleDifference( angles[YAW], mg42->s.angles[YAW] ) ) > mg42->harc ) ||
		 ( angles[PITCH] < 0 && angles[PITCH] + 5 < -mg42->varc ) ||
		 ( angles[PITCH] > 0 && angles[PITCH] - 5 > 5.0 ) ) {
		qboolean shouldAttack;

		// look for a better enemy
		numEnemies = AICast_ScanForEnemies( cs, enemies );
		shouldAttack = qfalse;
		if ( numEnemies > 0 ) {
			int i;
			// default to the first known enemy, overwrite if we find a clearer shot
			cs->enemyNum = enemies[0];
			//
			// unmount unless we find an enemy within harc
			unmount = qtrue;
			//
			for ( i = 0; i < numEnemies; i++ ) {
				/*
				if (!cs->perception.vislist[enemies[i]].real_visible_timestamp ||
					(cs->perception.vislist[enemies[i]].real_visible_timestamp < level.time - 2000)) {
					// we can't see them, ignore them
					continue;
				}
				*/

				// if they are in the range
				if ( cs->perception.vislist[enemies[i]].real_visible_timestamp > ( level.time - 5000 ) ) {
					VectorSubtract( cs->perception.vislist[enemies[i]].real_visible_pos, mg42->r.currentOrigin, vec );
				} else {
					VectorSubtract( cs->perception.vislist[enemies[i]].visible_pos, mg42->r.currentOrigin, vec );
				}
				VectorNormalize( vec );
				vectoangles( vec, angles );
				angles[PITCH] = AngleNormalize180( angles[PITCH] );
				if ( !( ( fabs( AI_AngleDifference( angles[YAW], mg42->s.angles[YAW] ) ) > mg42->harc ) ||
						( angles[YAW] < 0 && angles[YAW] + 2 < -mg42->varc ) ||
						( angles[YAW] > 0 && angles[YAW] - 2 > 5.0 ) ) ) {
					//
					// found someone inside harc, so dont unmount
					unmount = qfalse;
					//
					if ( AICast_CheckAttack( cs, enemies[i], qfalse ) ) {
						VectorCopy( angles, bestangles );
						cs->enemyNum = enemies[i];
						shouldAttack = qtrue;
						break;
					} else if ( AICast_CheckAttack( cs, enemies[i], qtrue ) ) {
						// keep firing at anything behind solids, in case they find a position where they can shoot us, but our checkattack() doesn't find a clear shot
						VectorCopy( angles, bestangles );
						cs->enemyNum = enemies[i];
						shouldAttack = qtrue;
					}
				}
			}
		}

		if ( !shouldAttack ) {
			// keep firing at anything behind solids, in case they find a position where they can shoot us, but our checkattack() doesn't find a clear shot
			if ( cs->enemyNum < 0 || !AICast_CheckAttack( cs, cs->enemyNum, qtrue ) ||
				 ( !cs->perception.vislist[cs->enemyNum].real_visible_timestamp ||
				   ( cs->perception.vislist[cs->enemyNum].real_visible_timestamp < level.time - 2000 ) ) ) {
				// face straight forward
				cs->ideal_viewangles[PITCH] = 0;
				return NULL;
			}
		}

		// if we had possible enemies, but couldnt find one to attack, then dismount now
		if ( unmount ) {
			AICast_ScriptEvent( cs, "forced_mg42_unmount", NULL );
			if ( !( cs->aiFlags & AIFL_DENYACTION ) ) {
				cs->aiFlags |= AIFL_DISMOUNTING;
				return NULL;
			}
		}
	}
	//
	// hold down fire, and track them
	//
	// TODO: play a special "holding mg42" torso animation
	//
	VectorCopy( angles, cs->ideal_viewangles );
	if ( cs->combat.triggerReleaseTime < level.time ) {
		trap_EA_Attack( bs->client );
		cs->bFlags |= AI_FRAME_ATTACKED;

		if ( cs->combat.triggerReleaseTime < level.time - 3000 ) {
			cs->combat.triggerReleaseTime = level.time + 700 + rand() % 700;
		}
	}
	//
	return NULL;
}

/*
============
AIFunc_BattleMG42Start()
============
*/
char *AIFunc_BattleMG42Start( cast_state_t *cs ) {
	cs->aiFlags &= ~AIFL_DISMOUNTING;
	//
	AI_SetLegacyState( cs, AIFunc_BattleMG42 );
	return "AIFunc_BattleMG42";
}
