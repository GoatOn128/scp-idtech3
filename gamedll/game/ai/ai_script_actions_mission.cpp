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
===============
AICast_ScriptAction_GotoCast

  syntax: gotocast <ainame> [firetarget [noattack]] OR runtocast <ainame> [firetarget [noattack]]
===============
*/
qboolean AICast_ScriptAction_GotoCast( cast_state_t *cs, char *params ) {
#define SCRIPT_REACHCAST_DIST 64
	char *pString, *token;
	gentity_t *ent;
	vec3_t vec, org;
	int i, diff;

	ent = NULL;

	// if we are avoiding danger, then wait for the danger to pass
	if ( cs->script.castScriptStatus.scriptGotoId < 0 && cs->dangerEntityValidTime > level.time ) {
		return qfalse;
	}
	// if we are in a special func, then wait
	if ( cs->aiFlags & AIFL_SPECIAL_FUNC ) {
		return qfalse;
	}

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI scripting: gotocast must have an ainame\n" );
	}

	// if we already are going to the marker, just use that, and check if we're in range
	if ( cs->script.castScriptStatus.scriptGotoEnt >= 0 &&
		 cs->script.castScriptStatus.scriptGotoId == cs->thinkFuncChangeTime ) {
		ent = &g_entities[cs->script.castScriptStatus.scriptGotoEnt];
		if ( ent->targetname && !Q_strcasecmp( ent->targetname, token ) ) {
			if ( Distance( cs->clientState->origin, ent->r.currentOrigin ) < cs->navigation.followDist ) {
				cs->navigation.followTime = 0;
				AIFunc_IdleStart( cs ); // resume normal AI
				return qtrue;
			} else {
				// do we have a firetarget ?
				token = COM_ParseExt( &pString, qfalse );
				if ( !token[0] ) {
					AICast_NoAttackIfNotHurtSinceLastScriptAction( cs );
				} else { // yes we do
					// find this targetname
					ent = G_Find( NULL, FOFS( targetname ), token );
					if ( !ent ) {
						ent = AICast_FindEntityForName( token );
						if ( !ent ) {
							G_Error( "AI Scripting: gotomarker cannot find targetname \"%s\"\n", token );
						}
					}

					// set the view angle manually
					BG_EvaluateTrajectory( &ent->s.pos, level.time, org );
					VectorSubtract( org, cs->clientState->origin, vec );
					VectorNormalize( vec );
					vectoangles( vec, cs->ideal_viewangles );
					// noattack?
					token = COM_ParseExt( &pString, qfalse );
					if ( !token[0] || Q_stricmp( token, "noattack" ) ) {
						qboolean fire = qtrue;
						// if it's an AI, and they aren't visible, dont shoot
						if ( ent->r.svFlags & SVF_CASTAI ) {
							if ( cs->perception.vislist[ent->s.number].real_visible_timestamp !=
								 cs->perception.vislist[ent->s.number].lastcheck_timestamp ) {
								fire = qfalse;
							}
						}
						if ( fire ) {
							for ( i = 0; i < 2; i++ ) {
								diff = fabs( AI_AngleDifference( cs->viewangles[i], cs->ideal_viewangles[i] ) );
								if ( diff < 20 ) {
									// dont reload prematurely
									cs->noReloadTime = level.time + 1000;
									// force fire
									trap_EA_Attack( cs->clientState->client );
									//
									cs->bFlags |= AI_FRAME_ATTACKED;
									// dont reload prematurely
									cs->noReloadTime = level.time + 200;
								}
							}
						}
					}
				}
				cs->navigation.followTime = level.time + 500;
				return qfalse;
			}
		} else {
			ent = NULL;
		}
	}

	// find the cast/player with the given "name"
	ent = AICast_FindEntityForName( token );
	if ( !ent ) {
		G_Error( "AI Scripting: can't find AI cast with \"ainame\" = \"%s\"\n", token );
	}

	if ( Distance( cs->clientState->origin, ent->r.currentOrigin ) < SCRIPT_REACHCAST_DIST ) { // we made it
		return qtrue;
	}

	if ( !ent ) {
		G_Error( "AI Scripting: can't find ai_marker with \"targetname\" = \"%s\"\n", token );
	}

	cs->script.castScriptStatus.scriptNoMoveTime = 0;
	cs->script.castScriptStatus.scriptGotoEnt = ent->s.number;
	//
	AIFunc_ChaseGoalStart( cs, ent->s.number, SCRIPT_REACHCAST_DIST, qtrue );
	cs->navigation.followTime = 0x7fffffff;
	AICast_NoAttackIfNotHurtSinceLastScriptAction( cs );
	cs->script.castScriptStatus.scriptGotoId = cs->thinkFuncChangeTime;

	return qfalse;
}

/*
==============
AICast_ScriptAction_AbortIfLoadgame
==============
*/
qboolean AICast_ScriptAction_AbortIfLoadgame( cast_state_t *cs, char *params ) {
	char loading[4];

	trap_Cvar_VariableStringBuffer( "savegame_loading", loading, sizeof( loading ) );

	if ( strlen( loading ) > 0 && atoi( loading ) != 0 ) {
		// abort the current script
		cs->script.castScriptStatus.castScriptStackHead =
			cs->script.castScriptEvents[cs->script.castScriptStatus.castScriptEventIndex].stack.numItems;
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_Wait

  syntax: wait <duration/FOREVER> [moverange] [facetarget]

  moverange defaults to 200, allows some monouverability to avoid fire or attack
=================
*/
qboolean AICast_ScriptAction_Wait( cast_state_t *cs, char *params ) {
	char *pString, *token, *facetarget;
	int duration;
	float moverange;
	float dist;
	gentity_t *ent;
	vec3_t org, vec;

	// if we are in a special func, then wait
	if ( cs->aiFlags & AIFL_SPECIAL_FUNC ) {
		return qfalse;
	}
	// EXPERIMENTAL: if they are on fire, or avoiding danger, let them loose until it passes (or they die)
	if ( cs->dangerEntityValidTime > level.time ) {
		cs->script.castScriptStatus.scriptNoMoveTime = -1;
		return qfalse;
	}

	if ( ( cs->script.castScriptStatus.scriptFlags & SFL_FIRST_CALL ) && cs->clientState ) {
		// first call, init the waitPos
		VectorCopy( cs->clientState->origin, cs->script.castScriptStatus.scriptWaitPos );
	}

	// get the duration
	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI scripting: wait must have a duration\n" );
	}
	if ( !Q_stricmp( token, "forever" ) ) {
		duration = level.time + 10000;
	} else {
		duration = atoi( token );
	}

	// if this is for the player, don't worry about enforcing the moverange
	if ( !cs->clientState ) {
		return ( cs->script.castScriptStatus.castScriptStackChangeTime + duration < level.time );
	}

	token = COM_ParseExt( &pString, qfalse );
	// if this token is a number, then assume it is the moverange, otherwise we have a default moverange with a facetarget
	moverange = -999;
	facetarget = NULL;
	if ( token[0] ) {
		if ( toupper( token[0] ) >= 'A' && toupper( token[0] ) <= 'Z' ) {
			facetarget = token;
		} else { // we found a moverange
			moverange = atof( token );
			token = COM_ParseExt( &pString, qfalse );
			if ( token[0] ) {
				facetarget = token;
			}
		}
	}

	// default to no moverange
	//if (moverange == -999) {
	//	moverange = 200;
	//}

	if ( moverange != 0 ) { // default to 200 if no range given
		if ( moverange > 0 ) {
			dist = Distance( cs->clientState->origin, cs->script.castScriptStatus.scriptWaitPos );
			// if we are able to move, and have an enemy
			if ( ( cs->script.castScriptStatus.scriptWaitMovetime < level.time ) && ( cs->enemyNum >= 0 ) ) {

				// if we can attack them, or they can't attack us, stay here
				// TTimo: gcc: warning: suggest parentheses around && within ||
				if ( AICast_CheckAttack( cs, cs->enemyNum, qfalse ) ||
					 ( !AICast_EntityVisible( AICast_GetCastState( cs->enemyNum ), cs->entityNum, qfalse ) &&
					   !AICast_CheckAttack( AICast_GetCastState( cs->enemyNum ), cs->entityNum, qfalse ) ) ) {
					cs->script.castScriptStatus.scriptNoMoveTime = level.time + 200;
				}
			}
			// if outside range, move towards waitPos
			if ( ( !cs->clientState || !cs->clientState->cur_ps.legsTimer ) &&
				 ( ( ( cs->script.castScriptStatus.scriptWaitMovetime > level.time ) && ( dist > 32 ) ) ||
				   ( dist > moverange ) ) ) {
				cs->script.castScriptStatus.scriptNoMoveTime = 0;
				AICast_MoveToPos( cs, cs->script.castScriptStatus.scriptWaitPos, 0 );
				if ( dist > 64 ) {
					cs->script.castScriptStatus.scriptWaitMovetime = level.time + 600;
				}
			} else
				// if we are reloading, look for somewhere to hide
				if ( cs->script.castScriptStatus.scriptWaitHideTime > level.time ||
					 cs->clientState->cur_ps.weaponTime > 500 ) {
					if ( cs->script.castScriptStatus.scriptWaitHideTime < level.time ) {
						// look for a hide pos within the wait range
					}
					cs->script.castScriptStatus.scriptWaitHideTime = level.time + 500;
				}
		}
	} else {
		cs->script.castScriptStatus.scriptNoMoveTime = cs->script.castScriptStatus.castScriptStackChangeTime + duration;
	}

	// do we have a facetarget ?
	if ( facetarget ) { // yes we do
		// find this targetname
		ent = G_Find( NULL, FOFS( targetname ), facetarget );
		if ( !ent ) {
			ent = AICast_FindEntityForName( facetarget );
			if ( !ent ) {
				G_Error( "AI Scripting: wait cannot find targetname \"%s\"\n", token );
			}
		}
		// set the view angle manually
		BG_EvaluateTrajectory( &ent->s.pos, level.time, org );
		VectorSubtract( org, cs->clientState->origin, vec );
		VectorNormalize( vec );
		vectoangles( vec, cs->ideal_viewangles );
	}

	return ( cs->script.castScriptStatus.castScriptStackChangeTime + duration < level.time );
}

/*
=================
AICast_ScriptAction_Trigger

  syntax: trigger <ainame> <trigger>

  Calls the specified trigger for the given ai character
=================
*/
qboolean AICast_ScriptAction_Trigger( cast_state_t *cs, char *params ) {
	gentity_t *ent;
	char *pString, *token;
	int oldId;

	// get the cast name
	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI scripting: trigger must have a name and an identifier\n" );
	}

	ent = AICast_FindEntityForName( token );
	if ( !ent ) {
		ent = G_Find( &g_entities[MAX_CLIENTS], FOFS( scriptName ), token );
		if ( !ent ) {
			if ( trap_Cvar_VariableIntegerValue( "developer" ) ) {
				G_Printf( "AI Scripting: can't find AI cast with \"ainame\" = \"%s\"\n", params );
			}
			return qtrue;
		}
	}

	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI scripting: trigger must have a name and an identifier\n" );
	}

	oldId = cs->script.castScriptStatus.scriptId;
	if ( ent->client ) {
		AICast_ScriptEvent( AICast_GetCastState( ent->s.number ), "trigger", token );
	} else {
		G_Script_ScriptEvent( ent, "trigger", token );
	}

	// if the script changed, return false so we don't muck with it's variables
	return ( oldId == cs->script.castScriptStatus.scriptId );
}

/*
=================
AICast_ScriptAction_AlertEntity

  syntax: alertentity <targetname>
=================
*/
qboolean AICast_ScriptAction_AlertEntity( cast_state_t *cs, char *params ) {
	gentity_t *ent;

	if ( !params || !params[0] ) {
		G_Error( "AI Scripting: alertentity without targetname\n" );
	}

	// find this targetname
	ent = G_Find( NULL, FOFS( targetname ), params );
	if ( !ent ) {
		ent = G_Find( NULL, FOFS( aiName ), params ); // look for an AI
		if ( !ent || !ent->client ) { // accept only AI for aiName check
			G_Error( "AI Scripting: alertentity cannot find targetname \"%s\"\n", params );
		}
	}

	// call this entity's AlertEntity function
	if ( !ent->AIScript_AlertEntity ) {
		if ( !ent->client && ent->use && !Q_stricmp( ent->classname, "ai_trigger" ) ) {
			ent->use( ent, NULL, NULL );
			return qtrue;
		}

		if ( aicast_debug.integer ) {
			G_Printf(
				"AI Scripting: alertentity \"%s\" (classname = %s) doesn't have an \"AIScript_AlertEntity\" function\n",
				params, ent->classname );
		}
		//G_Error( "AI Scripting: alertentity \"%s\" (classname = %s) doesn't have an \"AIScript_AlertEntity\" function\n", params, ent->classname );
		return qtrue;
	}

	ent->AIScript_AlertEntity( ent );

	return qtrue;
}

/*
=================
AICast_ScriptAction_SaveGame

  NOTE: only use this command in "player" scripts, not for AI

  syntax: savegame
=================
*/
qboolean AICast_ScriptAction_SaveGame( cast_state_t *cs, char *params ) {
	char *pString, *saveName;
	pString = params;

	if ( cs->clientState ) {
		G_Error( "AI Scripting: savegame attempted on a non-player" );
	}

	//----(SA)	check for parameter
	saveName = COM_ParseExt( &pString, qfalse );
	if ( !saveName[0] ) {
		G_SaveGame( NULL ); // save the default "current" savegame
	} else {
		G_SaveGame( saveName );
	}
	//----(SA)	end

	return qtrue;
}

/*
=================
AICast_ScriptAction_Accum

  syntax: accum <buffer_index> <command> <paramater>

  Commands:

	accum <n> inc <m>
	accum <n> abort_if_less_than <m>
	accum <n> abort_if_greater_than <m>
	accum <n> abort_if_not_equal <m>
	accum <n> abort_if_equal <m>
	accum <n> set <m>
	accum <n> random <m>
	accum <n> bitset <m>
	accum <n> bitreset <m>
	accum <n> abort_if_bitset <m>
	accum <n> abort_if_not_bitset <m>
=================
*/
qboolean AICast_ScriptAction_Accum( cast_state_t *cs, char *params ) {
	char *pString, *token, lastToken[MAX_QPATH];
	int bufferIndex;

	pString = params;

	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI Scripting: accum without a buffer index\n" );
	}

	bufferIndex = atoi( token );
	if ( bufferIndex >= MAX_SCRIPT_ACCUM_BUFFERS ) {
		G_Error( "AI Scripting: accum buffer is outside range (0 - %i)\n", MAX_SCRIPT_ACCUM_BUFFERS );
	}

	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI Scripting: accum without a command\n" );
	}

	Q_strncpyz( lastToken, token, sizeof( lastToken ) );
	token = COM_ParseExt( &pString, qfalse );

	if ( !Q_stricmp( lastToken, "inc" ) ) {
		if ( !token[0] ) {
			G_Error( "AI Scripting: accum %s requires a parameter\n", lastToken );
		}
		cs->script.scriptAccumBuffer[bufferIndex] += atoi( token );
	} else if ( !Q_stricmp( lastToken, "abort_if_less_than" ) ) {
		if ( !token[0] ) {
			G_Error( "AI Scripting: accum %s requires a parameter\n", lastToken );
		}
		if ( cs->script.scriptAccumBuffer[bufferIndex] < atoi( token ) ) {
			// abort the current script
			cs->script.castScriptStatus.castScriptStackHead =
				cs->script.castScriptEvents[cs->script.castScriptStatus.castScriptEventIndex].stack.numItems;
		}
	} else if ( !Q_stricmp( lastToken, "abort_if_greater_than" ) ) {
		if ( !token[0] ) {
			G_Error( "AI Scripting: accum %s requires a parameter\n", lastToken );
		}
		if ( cs->script.scriptAccumBuffer[bufferIndex] > atoi( token ) ) {
			// abort the current script
			cs->script.castScriptStatus.castScriptStackHead =
				cs->script.castScriptEvents[cs->script.castScriptStatus.castScriptEventIndex].stack.numItems;
		}
	} else if ( !Q_stricmp( lastToken, "abort_if_not_equal" ) ) {
		if ( !token[0] ) {
			G_Error( "AI Scripting: accum %s requires a parameter\n", lastToken );
		}
		if ( cs->script.scriptAccumBuffer[bufferIndex] != atoi( token ) ) {
			// abort the current script
			cs->script.castScriptStatus.castScriptStackHead =
				cs->script.castScriptEvents[cs->script.castScriptStatus.castScriptEventIndex].stack.numItems;
		}
	} else if ( !Q_stricmp( lastToken, "abort_if_equal" ) ) {
		if ( !token[0] ) {
			G_Error( "AI Scripting: accum %s requires a parameter\n", lastToken );
		}
		if ( cs->script.scriptAccumBuffer[bufferIndex] == atoi( token ) ) {
			// abort the current script
			cs->script.castScriptStatus.castScriptStackHead =
				cs->script.castScriptEvents[cs->script.castScriptStatus.castScriptEventIndex].stack.numItems;
		}
	} else if ( !Q_stricmp( lastToken, "bitset" ) ) {
		if ( !token[0] ) {
			G_Error( "AI Scripting: accum %s requires a parameter\n", lastToken );
		}
		cs->script.scriptAccumBuffer[bufferIndex] |= ( 1 << atoi( token ) );
	} else if ( !Q_stricmp( lastToken, "bitreset" ) ) {
		if ( !token[0] ) {
			G_Error( "AI Scripting: accum %s requires a parameter\n", lastToken );
		}
		cs->script.scriptAccumBuffer[bufferIndex] &= ~( 1 << atoi( token ) );
	} else if ( !Q_stricmp( lastToken, "abort_if_bitset" ) ) {
		if ( !token[0] ) {
			G_Error( "AI Scripting: accum %s requires a parameter\n", lastToken );
		}
		if ( cs->script.scriptAccumBuffer[bufferIndex] & ( 1 << atoi( token ) ) ) {
			// abort the current script
			cs->script.castScriptStatus.castScriptStackHead =
				cs->script.castScriptEvents[cs->script.castScriptStatus.castScriptEventIndex].stack.numItems;
		}
	} else if ( !Q_stricmp( lastToken, "abort_if_not_bitset" ) ) {
		if ( !token[0] ) {
			G_Error( "AI Scripting: accum %s requires a parameter\n", lastToken );
		}
		if ( !( cs->script.scriptAccumBuffer[bufferIndex] & ( 1 << atoi( token ) ) ) ) {
			// abort the current script
			cs->script.castScriptStatus.castScriptStackHead =
				cs->script.castScriptEvents[cs->script.castScriptStatus.castScriptEventIndex].stack.numItems;
		}
	} else if ( !Q_stricmp( lastToken, "set" ) ) {
		if ( !token[0] ) {
			G_Error( "AI Scripting: accum %s requires a parameter\n", lastToken );
		}
		cs->script.scriptAccumBuffer[bufferIndex] = atoi( token );
	} else if ( !Q_stricmp( lastToken, "random" ) ) {
		if ( !token[0] ) {
			G_Error( "AI Scripting: accum %s requires a parameter\n", lastToken );
		}
		cs->script.scriptAccumBuffer[bufferIndex] = rand() % atoi( token );
	} else {
		G_Error( "AI Scripting: accum %s: unknown command\n", params );
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_SpawnCast

  syntax: spawncast <classname> <targetname> <ainame>

  <targetname> is the entity marker which has the position and angles of where we want the new
  cast AI to spawn
=================
*/
qboolean AICast_ScriptAction_SpawnCast( cast_state_t *cs, char *params ) {
	//	char	*pString, *token;
	//	char	*classname;
	//	gentity_t	*targetEnt, *newCast;

	G_Error( "AI Scripting: spawncast is no longer functional. Use trigger_spawn instead.\n" );
	return qfalse;
	/*
	if (!params[0]) {
		G_Error( "AI Scripting: spawncast without a classname\n" );
	}

	pString = params;

	token = COM_ParseExt( &pString, qfalse );
	if (!token[0]) {
		G_Error( "AI Scripting: spawncast without a classname\n" );
	}

	classname = G_Alloc( strlen(token)+1 );
	Q_strncpyz( classname, token, strlen(token)+1 );

	token = COM_ParseExt( &pString, qfalse );
	if (!token[0]) {
		G_Error( "AI Scripting: spawncast without a targetname\n" );
	}

	targetEnt = G_Find( NULL, FOFS(targetname), token );
	if (!targetEnt) {
		G_Error( "AI Scripting: cannot find targetname \"%s\"\n", token );
	}

	token = COM_ParseExt( &pString, qfalse );
	if (!token[0]) {
		G_Error( "AI Scripting: spawncast without an ainame\n" );
	}

	newCast = G_Spawn();
	newCast->classname = classname;
	VectorCopy( targetEnt->s.origin, newCast->s.origin );
	VectorCopy( targetEnt->s.angles, newCast->s.angles );
	newCast->aiName = G_Alloc( strlen(token)+1 );
	Q_strncpyz( newCast->aiName, token, strlen(token)+1 );

	if (!G_CallSpawn( newCast )) {
		G_Error( "AI Scripting: spawncast for unknown entity \"%s\"\n", newCast->classname );
	}

	return qtrue;
*/
}

/*
=================
AICast_ScriptAction_MissionFailed

  syntax: missionfailed  <time>
=================
*/
qboolean AICast_ScriptAction_MissionFailed( cast_state_t *cs, char *params ) {
	char *pString, *token;
	int time = 6, mof = 0;

	pString = params;

	token = COM_ParseExt( &pString, qfalse ); // time
	if ( token && token[0] ) {
		time = atoi( token );
	}

	token = COM_ParseExt( &pString, qfalse ); // mof (means of failure)
	if ( token && token[0] ) {
		mof = atoi( token );
	}

	// play mission fail music
	trap_SendServerCommand( -1, "mu_play sound/music/l_failed_1.wav 0\n" );
	// clear queue so it'll be quiet after failed stinger
	trap_SetConfigstring( CS_MUSIC_QUEUE, "" );

	// fade all sound out
	trap_SendServerCommand( -1, va( "snd_fade 0 %d", time * 1000 ) );

	if ( mof < 0 ) {
		mof = 0;
	}
	trap_SendServerCommand( -1, va( "cp missionfail%d", mof ) );

	// reload the current savegame, after a delay
	trap_SetConfigstring( CS_SCREENFADE, va( "1 %i %i", level.time + 250, time * 1000 ) );
	//	reloading = RELOAD_FAILED;
	trap_Cvar_Set( "g_reloading", va( "%d", RELOAD_FAILED ) );

	level.reloadDelayTime = level.time + 1000 + time * 1000;

	return qtrue;
}

/*
=================
AICast_ScriptAction_ObjectivesNeeded

  syntax: objectivesneeded <num_objectives>
=================
*/
qboolean AICast_ScriptAction_ObjectivesNeeded( cast_state_t *cs, char *params ) {
	char *pString, *token;

	pString = params;

	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI Scripting: objectivesneeded requires a num_objectives identifier\n" );
	}

	level.numObjectives = atoi( token );

	return qtrue;
}

/*
=================
AICast_ScriptAction_ObjectiveMet

  syntax: objectivemet <num_objective>
  also (for backwards compaiblity): missionsuccess <num_objective> [nodisplay]
=================
*/
qboolean AICast_ScriptAction_ObjectiveMet( cast_state_t *cs, char *params ) {
	gentity_t *player;
	vmCvar_t cvar;
	int lvl;
	char *pString, *token;

	pString = params;

	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI Scripting: missionsuccess requires a num_objective identifier\n" );
	}

	player = AICast_FindEntityForName( "player" );
	// double check that they are still alive
	if ( player->health <= 0 ) {
		return qfalse; // hold the script here
	}
	lvl = atoi( token );

	// if you've already got it, just return.  don't need to set 'yougotmail'
	if ( player->missionObjectives & ( 1 << ( lvl - 1 ) ) ) {
		return qtrue;
	}

	player->missionObjectives |= ( 1 << ( lvl - 1 ) ); // make this bitwise

	//set g_objective<n> cvar
	trap_Cvar_Register( &cvar, va( "g_objective%i", lvl ), "1", CVAR_ROM );
	// set it to make sure
	trap_Cvar_Set( va( "g_objective%i", lvl ), "1" );

	token = COM_ParseExt( &pString, qfalse );
	if ( token[0] ) {
		if ( Q_strcasecmp( token, "nodisplay" ) ) { // unknown command
			G_Error( "AI Scripting: missionsuccess with unknown parameter: %s\n", token );
		}
	} else { // show on-screen information
		trap_Cvar_Set( "cg_youGotMail", "2" ); // set flag to draw icon
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_Print

  syntax: print <text>

  Mostly for debugging purposes
=================
*/
qboolean AICast_ScriptAction_Print( cast_state_t *cs, char *params ) {
	if ( !params || !params[0] ) {
		G_Error( "AI Scripting: print requires some text\n" );
	}

	G_Printf( "(AI) %s-> %s\n", g_entities[cs->entityNum].aiName, params );
	return qtrue;
}

/*
===================
AICast_ScriptAction_ResetScript

	causes any currently running scripts to abort, in favour of the current script
===================
*/
qboolean AICast_ScriptAction_ResetScript( cast_state_t *cs, char *params ) {
	gclient_t *client;

	client = &level.clients[cs->entityNum];

	// stop any anim from playing
	if ( client->ps.torsoTimer && ( client->ps.torsoTimer > ( level.time - cs->script.scriptAnimTime ) ) ) {
		if ( ( client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) == cs->script.scriptAnimNum ) {
			client->ps.torsoTimer = 0;
		}
	}
	if ( client->ps.legsTimer && ( client->ps.legsTimer > ( level.time - cs->script.scriptAnimTime ) ) ) {
		if ( ( client->ps.legsAnim & ~ANIM_TOGGLEBIT ) == cs->script.scriptAnimNum ) {
			client->ps.legsTimer = 0;
		}
	}

	// stop playing voice channel sounds
	G_AddEvent( &g_entities[cs->clientState->entitynum], EV_GENERAL_SOUND, G_SoundIndex( "Blank" ) );

	cs->script.castScriptStatus.scriptNoMoveTime = 0;
	cs->script.castScriptStatus.playAnimViewlockTime = 0;
	// stop following anything that we don't need to be following
	cs->navigation.followEntity = -1;
	if ( cs->script.castScriptStatus.scriptFlags & SFL_FIRST_CALL ) {
		return qfalse;
	}

	// make sure zoom is off
	cs->aiFlags &= ~AIFL_ZOOMING;
	g_entities[cs->entityNum].client->ps.eFlags &= ~EF_CIG; //----(SA)	added

	return qtrue;
}

/*
===================
AICast_ScriptAction_Mount

	syntax: mount <targetname>

  Used to an AI to mount the MG42
===================
*/
qboolean AICast_ScriptAction_Mount( cast_state_t *cs, char *params ) {
	gentity_t *targetEnt, *ent;
	vec3_t vec;
	float dist;

	if ( !params || !params[0] ) {
		G_Error( "AI Scripting: mount requires a targetname\n" );
	}

	targetEnt = G_Find( NULL, FOFS( targetname ), params );
	if ( !targetEnt ) {
		G_Error( "AI Scripting: cannot find targetname \"%s\"\n", params );
	}

	VectorSubtract( targetEnt->r.currentOrigin, cs->clientState->origin, vec );
	dist = VectorNormalize( vec );
	vectoangles( vec, cs->ideal_viewangles );

	if ( dist > 40 ) {
		// walk towards it
		trap_EA_Move( cs->entityNum, vec, 80 );
		return qfalse;
	}

	if ( !targetEnt->takedamage ) {
		// the gun has been destroyed
		return qtrue;
	}

	// if we are facing it, start holding activate
	if ( fabs( cs->ideal_viewangles[YAW] - cs->viewangles[YAW] ) < 10 ) {
		ent = &g_entities[cs->entityNum];
		Cmd_Activate_f( ent );
		// did we mount it?
		if ( ent->active && targetEnt->r.ownerNum == ent->s.number ) {
			cs->mountedEntity = targetEnt->s.number;
			AIFunc_BattleMG42Start( cs );
			return qtrue;
		}
	}

	return qfalse;
}

/*
===================
AICast_ScriptAction_Unmount

	syntax: unmount

  Stop using their current mounted entity
===================
*/
qboolean AICast_ScriptAction_Unmount( cast_state_t *cs, char *params ) {
	gentity_t *ent, *mg42;

	ent = &g_entities[cs->entityNum];
	mg42 = &g_entities[cs->mountedEntity];

	if ( !ent->active ) {
		return qtrue; // nothing mounted, just skip this command
	}
	// face straight forward
	VectorCopy( mg42->s.angles, cs->ideal_viewangles );
	// try and unmount
	Cmd_Activate_f( ent );
	if ( !ent->active ) {
		return qtrue;
	}
	// waiting to unmount still
	return qfalse;
}

/*
====================
AICast_ScriptAction_SavePersistant

	syntax: savepersistant <next_mapname>

  Saves out the data that should be retained between certain levels. Not
  calling this routine before changing levels, is the equivalent of resetting
  the player's inventory/health/etc.

  The <next_mapname> is used to identify the next map we don't
  accidentally read in persistant data that was intended for a different map.
====================
*/
qboolean AICast_ScriptAction_SavePersistant( cast_state_t *cs, char *params ) {
	G_SavePersistant( params );
	return qtrue;
}

/*
==============
AICast_ScriptAction_Teleport
==============
*/
qboolean AICast_ScriptAction_Teleport( cast_state_t *cs, char *params ) {
	gentity_t *dest;

	dest = G_PickTarget( params );
	if ( !dest ) {
		G_Error( "AI Scripting: couldn't find teleporter destination: '%s'\n", params );
	}

	TeleportPlayer( &g_entities[cs->entityNum], dest->s.origin, dest->s.angles );

	return qtrue;
}

extern void G_EndGame( void );

/*
==============
AICast_ScriptAction_EndGame
==============
*/
qboolean AICast_ScriptAction_EndGame( cast_state_t *cs, char *params ) {
	G_EndGame();
	return qtrue;
}

/*
====================
AICast_ScriptAction_ChangeLevel

	syntax: changelevel <mapname> [exitTime] <persistent> <silent>

  Issues an spdevmap/spmap to the consol.
  Optionally add
	"persistent" if you want the player to keep their inventory through the transition.
	"silent" if you want it to not play the mission success music

====================
*/
qboolean AICast_ScriptAction_ChangeLevel( cast_state_t *cs, char *params ) {
	int i;
	char *pch, *pch2, *newstr;
	gentity_t *player;
	qboolean silent = qfalse, endgame = qfalse, savepersist = qfalse;
	int exitTime = 8000;

	player = AICast_FindEntityForName( "player" );
	// double check that they are still alive
	if ( player->health <= 0 ) {
		return qtrue; // get out of here
	}
	// don't process if already changing
	//	if(reloading)
	if ( g_reloading.integer ) {
		return qtrue;
	}

	// save persistent data if required
	newstr = va( params );
	pch = strstr( newstr, " persistent" ); // (SA) whoops, this was mis-spelled
	if ( pch ) {
		pch = strstr( newstr, " " );
		*pch = '\0';
		savepersist = qtrue;
	}

	//
	newstr = va( params );
	pch = strstr( newstr, " silent" );
	if ( pch ) {
		pch = strstr( newstr, " " );
		*pch = '\0';
		silent = qtrue;
	}

	// make sure we strip any params after the mapname
	newstr = va( params );
	pch = strstr( newstr, " " );
	if ( pch ) {
		*( pch++ ) = '\0';
		//
		// see if there is a mission_level specified
		pch2 = strstr( pch, " " );
		if ( pch2 ) { // kill the space if exists
			*pch2 = '\0';
		}

		if ( atoi( pch ) ) { // there's a 'time' specified
			exitTime = atoi( pch );
		}
	}

	if ( !Q_stricmp( newstr, "gamefinished" ) ) { // 'gamefinished' is keyword for 'exit to credits'
		endgame = qtrue;
	}

	if ( !endgame ) {

		// check for missing objectives
		for ( i = 0; i < level.numObjectives; i++ ) {
			if ( !( player->missionObjectives & ( 1 << i ) ) ) {
				trap_SendServerCommand( -1, "cp objectivesnotcomplete" );
				return qtrue;
			}
		}

		if ( savepersist ) {
			G_SavePersistant( newstr ); // save persistent data if required
		}
	}

	if ( !silent && !endgame ) {
		trap_SendServerCommand( -1, "mu_play sound/music/l_complete_1.wav 0\n" ); // play mission success music
	}
	trap_SetConfigstring( CS_MUSIC_QUEUE, "" ); // don't try to start anything.  no level load music

	trap_SetConfigstring( CS_SCREENFADE, va( "1 %i %i", level.time + 250, 750 + exitTime ) ); // fade out screen

	trap_SendServerCommand( -1, va( "snd_fade 0 %d", 1000 + exitTime ) ); //----(SA)	added

	// load the next map, after a delay
	level.reloadDelayTime = level.time + 1000 + exitTime;
	trap_Cvar_Set( "g_reloading", va( "%d", RELOAD_NEXTMAP_WAITING ) );

	if ( endgame ) {
		trap_Cvar_Set( "g_reloading", va( "%d", RELOAD_ENDGAME ) );
		return qtrue;
	}

	Q_strncpyz( level.nextMap, newstr, sizeof( level.nextMap ) );

	//if (g_cheats.integer)
	//	trap_SendConsoleCommand( EXEC_APPEND, va("spdevmap %s\n", newstr) );
	//else
	//	trap_SendConsoleCommand( EXEC_APPEND, va("spmap %s\n", newstr ) );

	return qtrue;
}

/*
==================
AICast_ScriptAction_FoundSecret
==================
*/
qboolean AICast_ScriptAction_FoundSecret( cast_state_t *cs, char *params ) {
	gentity_t *player = AICast_FindEntityForName( "player" );
	//	level.numSecretsFound++;
	player->numSecretsFound++;
	trap_SendServerCommand( -1, "cp secretarea" );
	G_SendMissionStats();
	return qtrue;
}

/*
==================
AICast_ScriptAction_NoAvoid

  syntax: noavoid
==================
*/
qboolean AICast_ScriptAction_NoAvoid( cast_state_t *cs, char *params ) {
	cs->aiFlags |= AIFL_NOAVOID;
	return qtrue;
}

/*
==================
AICast_ScriptAction_Avoid

  syntax: avoid
==================
*/
qboolean AICast_ScriptAction_Avoid( cast_state_t *cs, char *params ) {
	cs->aiFlags &= ~AIFL_NOAVOID;
	return qtrue;
}

/*
==================
AICast_ScriptAction_Attrib

  syntax: attrib <attribute> <value>
==================
*/
qboolean AICast_ScriptAction_Attrib( cast_state_t *cs, char *params ) {
	char *pString, *token;
	int i;

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: attrib <attribute> <value>" );
	}

	for ( i = 0; i < AICAST_MAX_ATTRIBUTES; i++ ) {
		if ( !Q_strcasecmp( token, castAttributeStrings[i] ) ) {
			// found a match, read in the value
			token = COM_ParseExt( &pString, qfalse );
			if ( !token[0] ) {
				G_Error( "AI_Scripting: syntax: attrib <attribute> <value>" );
			}
			// set the attribute
			cs->attributes[i] = atof( token );
			break;
		}
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_DenyAction

  syntax: deny
=================
*/
qboolean AICast_ScriptAction_DenyAction( cast_state_t *cs, char *params ) {
	cs->aiFlags |= AIFL_DENYACTION;
	return qtrue;
}

/*
=================
AICast_ScriptAction_Headlook
=================
*/
qboolean AICast_ScriptAction_Headlook( cast_state_t *cs, char *params ) {
	char *pString, *token;

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: headlook <ON/OFF>" );
	}
	Q_strlwr( token );

	if ( !Q_stricmp( token, "on" ) ) {
		cs->aiFlags &= ~AIFL_NO_HEADLOOK;
	} else if ( !Q_stricmp( token, "off" ) ) {
		cs->aiFlags |= AIFL_NO_HEADLOOK;
	} else {
		G_Error( "AI_Scripting: syntax: headlook <ON/OFF>" );
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_BackupScript

  backs up the current state of the scripting, so we can restore it later and resume
  were we left off (useful if player gets in our way)
=================
*/
qboolean AICast_ScriptAction_BackupScript( cast_state_t *cs, char *params ) {

	if ( !( cs->script.castScriptStatus.scriptFlags & SFL_WAITING_RESTORE ) ) {
		cs->script.castScriptStatusBackup = cs->script.castScriptStatusCurrent;
		cs->script.castScriptStatus.scriptFlags |= SFL_WAITING_RESTORE;
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_RestoreScript

  restores the state of the scripting to the previous backup
=================
*/
qboolean AICast_ScriptAction_RestoreScript( cast_state_t *cs, char *params ) {

	cs->script.castScriptStatus = cs->script.castScriptStatusBackup;

	// make sure we restart any goto's
	cs->script.castScriptStatus.scriptGotoId = -1;
	cs->script.castScriptStatus.scriptGotoEnt = -1;

	return qfalse; // dont continue scripting until next frame
}

/*
=================
AICast_ScriptAction_StateType

  set the current state for this character
=================
*/
qboolean AICast_ScriptAction_StateType( cast_state_t *cs, char *params ) {

	if ( !Q_stricmp( params, "alert" ) ) {
		cs->aiState = AISTATE_ALERT;
	} else if ( !Q_stricmp( params, "relaxed" ) ) {
		cs->aiState = AISTATE_RELAXED;
	}

	return qtrue;
}

/*
================
AICast_ScriptAction_KnockBack

  syntax: knockback [ON/OFF]
================
*/
qboolean AICast_ScriptAction_KnockBack( cast_state_t *cs, char *params ) {

	char *pString, *token;

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: knockback <ON/OFF>" );
	}
	Q_strlwr( token );

	if ( !Q_stricmp( token, "on" ) ) {
		g_entities[cs->entityNum].flags &= ~FL_NO_KNOCKBACK;
	} else if ( !Q_stricmp( token, "off" ) ) {
		g_entities[cs->entityNum].flags |= FL_NO_KNOCKBACK;
	} else {
		G_Error( "AI_Scripting: syntax: knockback <ON/OFF>" );
	}

	return qtrue;
}

/*
================
AICast_ScriptAction_Zoom

  syntax: zoom [ON/OFF]
================
*/
qboolean AICast_ScriptAction_Zoom( cast_state_t *cs, char *params ) {

	char *pString, *token;

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: zoom <ON/OFF>" );
	}
	Q_strlwr( token );

	// give them the inventory item
	g_entities[cs->entityNum].client->ps.stats[STAT_KEYS] |= ( 1 << INV_BINOCS );

	if ( !Q_stricmp( token, "on" ) ) {
		cs->aiFlags |= AIFL_ZOOMING;
	} else if ( !Q_stricmp( token, "off" ) ) {
		cs->aiFlags &= ~AIFL_ZOOMING;
	} else {
		G_Error( "AI_Scripting: syntax: zoom <ON/OFF>" );
	}

	return qtrue;
}

/*
===================
AICast_ScriptAction_StartCam

  syntax: startcam <camera filename>
===================
*/
qboolean ScriptStartCam( cast_state_t *cs, char *params, qboolean black ) {
	char *pString, *token;
	gentity_t *ent;

	ent = &g_entities[cs->entityNum];

	pString = params;
	token = COM_Parse( &pString );
	if ( !token[0] ) {
		G_Error( "G_ScriptAction_Cam: filename parameter required\n" );
	}

	// turn off noclient flag
	ent->r.svFlags &= ~SVF_NOCLIENT;

	// issue a start camera command to the client
	trap_SendServerCommand( cs->entityNum, va( "startCam %s %d", token, (int)black ) );

	return qtrue;
}

qboolean AICast_ScriptAction_StartCam( cast_state_t *cs, char *params ) {
	return ScriptStartCam( cs, params, qfalse );
}

qboolean AICast_ScriptAction_StartCamBlack( cast_state_t *cs, char *params ) {
	return ScriptStartCam( cs, params, qtrue );
}

//----(SA)	added
qboolean AICast_ScriptAction_StopCamBlack( cast_state_t *cs, char *params ) {
	trap_SendServerCommand( cs->entityNum, "stopCamblack" );
	return qtrue;
}

qboolean AICast_ScriptAction_StopCam( cast_state_t *cs, char *params ) {
	trap_SendServerCommand( cs->entityNum, "stopCam" );
	return qtrue;
}

//----(SA)	end

//----(SA)	added
qboolean AICast_ScriptAction_Cigarette( cast_state_t *cs, char *params ) {

	char *pString, *token;
	gentity_t *ent;

	ent = &g_entities[cs->entityNum];

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: cigarette <ON/OFF>" );
	}
	Q_strlwr( token );

	if ( !Q_stricmp( token, "on" ) ) {
		g_entities[cs->entityNum].client->ps.eFlags |= EF_CIG;
	} else if ( !Q_stricmp( token, "off" ) ) {
		g_entities[cs->entityNum].client->ps.eFlags &= ~EF_CIG;
	} else {
		G_Error( "AI_Scripting: syntax: cigarette <ON/OFF>" );
	}

	return qtrue;
}

//----(SA)	end

/*
=================
AICast_ScriptAction_Parachute

  syntax: parachute [ON/OFF]
=================
*/
qboolean AICast_ScriptAction_Parachute( cast_state_t *cs, char *params ) {

	char *pString, *token;
	gentity_t *ent;

	ent = &g_entities[cs->entityNum];

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: parachute <ON/OFF>" );
	}
	Q_strlwr( token );

	if ( !Q_stricmp( token, "on" ) ) {
		ent->flags |= FL_PARACHUTE;
	} else if ( !Q_stricmp( token, "off" ) ) {
		ent->flags &= ~FL_PARACHUTE;
	} else {
		G_Error( "AI_Scripting: syntax: parachute <ON/OFF>" );
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_EntityScriptName
=================
*/
qboolean AICast_ScriptAction_EntityScriptName( cast_state_t *cs, char *params ) {
	trap_Cvar_Set( "g_scriptName", params );
	return qtrue;
}

/*
=================
AICast_ScriptAction_AIScriptName
=================
*/
qboolean AICast_ScriptAction_AIScriptName( cast_state_t *cs, char *params ) {
	trap_Cvar_Set( "ai_scriptName", params );
	return qtrue;
}

/*
=================
AICast_ScriptAction_SetHealth
=================
*/
qboolean AICast_ScriptAction_SetHealth( cast_state_t *cs, char *params ) {
	if ( !params || !params[0] ) {
		G_Error( "AI Scripting: sethealth requires a health value" );
	}

	g_entities[cs->entityNum].health = atoi( params );
	g_entities[cs->entityNum].client->ps.stats[STAT_HEALTH] = atoi( params );

	return qtrue;
}

/*
=================
AICast_ScriptAction_NoTarget

  syntax: notarget ON/OFF
=================
*/
qboolean AICast_ScriptAction_NoTarget( cast_state_t *cs, char *params ) {
	if ( !params || !params[0] ) {
		G_Error( "AI Scripting: notarget requires ON or OFF as parameter" );
	}

	if ( !Q_strcasecmp( params, "ON" ) ) {
		g_entities[cs->entityNum].flags |= FL_NOTARGET;
	} else if ( !Q_strcasecmp( params, "OFF" ) ) {
		g_entities[cs->entityNum].flags &= ~FL_NOTARGET;
	} else {
		G_Error( "AI Scripting: notarget requires ON or OFF as parameter" );
	}

	return qtrue;
}

/*
==================
AICast_ScriptAction_Cvar
==================
*/
qboolean AICast_ScriptAction_Cvar( cast_state_t *cs, char *params ) {
	vmCvar_t cvar;
	char *pString, *token;
	char cvarName[MAX_QPATH];

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: cvar <cvarName> <cvarValue>" );
	}
	Q_strncpyz( cvarName, token, sizeof( cvarName ) );

	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: cvar <cvarName> <cvarValue>" );
	}

	if ( !strcmp( cvarName, "objective" ) ) {
		G_Printf( "WARNING: 'objective' cvar set from script.  Do not set directly.  Use 'missionsuccess <num>'\n" );
		return qtrue;
	}

	trap_Cvar_Register( &cvar, cvarName, token, CVAR_ROM );
	// set it to make sure
	trap_Cvar_Set( cvarName, token );
	return qtrue;
}

/*
=================
AICast_ScriptAction_LockPlayer

  syntax: lockplayer <ON/OFF>
=================
*/
qboolean AICast_ScriptAction_LockPlayer( cast_state_t *cs, char *params ) {
	gentity_t *ent;

	ent = &g_entities[cs->entityNum];

	if ( !params || !params[0] ) {
		G_Error( "AI Scripting: lockplayer requires an on/off specifier\n" );
	}

	if ( !Q_stricmp( params, "on" ) ) {
		ent->client->ps.pm_flags |= PMF_IGNORE_INPUT;
	} else if ( !Q_stricmp( params, "off" ) ) {
		ent->client->ps.pm_flags &= ~PMF_IGNORE_INPUT;
	} else {
		G_Error( "AI Scripting: lockplayer requires an on/off specifier\n" );
	}

	return qtrue;
}
