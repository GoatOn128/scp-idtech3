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
AICast_StartFrame

  Think any clients that need thinking
============
*/
void CopyToBodyQue( gentity_t *ent );

void AICast_StartFrame( int time ) {
	int i, elapsed, count, clCount;
	cast_state_t *cs;
	int castcount;
	static int lasttime, lastthink;
	static vmCvar_t aicast_disable;
	gentity_t *ent;

	if ( trap_Cvar_VariableIntegerValue( "savegame_loading" ) ) {
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
	trap_Cvar_Update( &aicast_debugname );
	trap_Cvar_Update( &aicast_scripts );

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

	//G_Printf( "AI startframe: %i\n", time );

	if ( elapsed < 0 ) {
		elapsed = 0;
		lasttime = time;
	}
	// don't let the SIGHTING framerate drop below 10 (too much sighting to process at once)
	if ( elapsed > 100 ) {
		elapsed = 100;
	}
	AICast_SightUpdate( (int)( (float)SIGHT_PER_SEC * ( (float)elapsed / 1000 ) ) );
	//
	// update the player's area, only update if it's valid
	for ( i = 0; i < 2; i++ ) {
		trap_AAS_SetCurrentWorld( i );
		castcount = AI_PointAreaNum( g_entities[0].s.pos.trBase );
		if ( castcount ) {
			caststates[0].navigation.lastValidAreaNum[i] = castcount;
			caststates[0].navigation.lastValidAreaTime[i] = level.time;
		}
	}
	//
	count = 0;
	castcount = 0;
	clCount = 0;
	//
	if ( ++lastthink > level.maxclients ) {
		lastthink = 0;
	}
	//update the AI characters
	for ( i = lastthink, ent = &g_entities[lastthink]; clCount < level.numPlayingClients && count < aicast_maxthink;
		  i++, ent++ ) {
		if ( i >= level.maxclients ) {
			// rewind back to the start
			i = 0;
			ent = g_entities;
		}
		//
		lastthink = i;
		if ( !ent->inuse ) {
			continue;
		}
		if ( ent->client ) {
			clCount++;
		}
		//
		cs = AICast_GetCastState( i );
		// is this a cast AI?
		if ( cs->clientState ) {
			if ( ent->aiInactive == qfalse ) {
				//
				elapsed = time - cs->lastThink;
				//
				// if they're moving/firing think every frame
				if ( ( elapsed && cs->script.scriptAnimTime && cs->script.scriptAnimTime >= ( level.time - 1000 ) ) ||
					 (   ( elapsed >= 50 ) &&
						 (   (   (   ( !VectorCompare( ent->client->ps.velocity, vec3_origin ) ) ||
									 ( cs->enemyNum >= 0 ) ||
									 ( cs->aiState >= AISTATE_COMBAT ) ||
									 ( cs->perception.vislist[0].visible_timestamp && cs->perception.vislist[0].visible_timestamp > level.time - 4000 ) ||
									 ( ent->client->buttons ) ||
									 ( elapsed >= aicast_thinktime ) ) /*&&
								(count <= aicast_maxthink)*/) ||
							( elapsed >= aicast_thinktime * 2 ) ) ) ) {
					// make it think now
					AICast_Think( i, (float)elapsed / 1000 );
					// did they drop?
					if ( !cs->clientState || !cs->clientState->inuse ) {
						break; // get out of here, to be safe
					}
					cs->lastThink =
						time + rand() % 20; // randomize this slightly to spread out thinks during high framerates
					//
					// only count live guys
					if ( ent->health > 0 ) {
						count++;
					}
				}
				// check for any debug info updates
				AICast_DebugFrame( cs );
			} else if ( cs->aiFlags & AIFL_WAITINGTOSPAWN ) {
				// check f the space is clear yet
				ent->AIScript_AlertEntity( ent );
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
}

// The navigation backend invalidates entity snapshots on StartFrame. Publish
// clients immediately afterwards, including when campaign thinking starts later
// than the server's navigation callback.
int AI_UpdateNavigationEntities( int time ) {
	trap_BotLibStartFrame( (float)time / 1000.0f );
	trap_AAS_SetCurrentWorld( 0 );
	if ( !trap_AAS_Initialized() ) {
		return BLERR_NOERROR;
	}
	for ( int client = 0; client < level.maxclients && client < MAX_CLIENTS; client++ ) {
		const gentity_t *ent = &g_entities[client];
		if ( !ent->inuse || !ent->client || !ent->r.linked || ( ent->r.svFlags & SVF_NOCLIENT ) || ent->aiInactive ) {
			continue;
		}
		bot_entitystate_t state = {};
		VectorCopy( ent->r.currentOrigin, state.origin );
		VectorCopy( ent->r.currentAngles, state.angles );
		VectorCopy( ent->s.origin2, state.old_origin );
		VectorCopy( ent->r.mins, state.mins );
		VectorCopy( ent->r.maxs, state.maxs );
		state.type = ent->s.eType;
		state.flags = ent->s.eFlags;
		state.solid = ent->r.bmodel ? SOLID_BSP : SOLID_BBOX;
		state.groundent = ent->s.groundEntityNum;
		state.modelindex = ent->s.modelindex;
		state.modelindex2 = ent->s.modelindex2;
		state.frame = ent->s.frame;
		state.powerups = ent->s.powerups;
		state.legsAnim = ent->s.legsAnim;
		state.torsoAnim = ent->s.torsoAnim;
		state.weapon = ent->s.weapon;
		trap_BotLibUpdateEntity( client, &state );
	}
	return BLERR_NOERROR;
}
