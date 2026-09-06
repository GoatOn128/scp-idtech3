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

;

/*
================
AICast_ScriptAction_PlaySound

  syntax: playsound <soundname OR scriptname>

  Currently only allows playing on the VOICE channel, unless you use a sound script (yay)
================
*/
qboolean AICast_ScriptAction_PlaySound( cast_state_t *cs, char *params ) {
	if ( !params ) {
		G_Error( "AI Scripting: syntax error\n\nplaysound <soundname OR scriptname>\n" );
	}

	G_AddEvent( &g_entities[cs->clientState->entitynum], EV_GENERAL_SOUND, G_SoundIndex( params ) );

	// assume we are talking
	cs->aiFlags |= AIFL_TALKING;

	// randomly choose idle animation
	if ( cs->aiFlags & AIFL_STAND_IDLE2 ) {
		if ( cs->lastEnemy < 0 && cs->aiFlags & AIFL_TALKING ) {
			g_entities[cs->entityNum].client->ps.eFlags |= EF_STAND_IDLE2;
		} else {
			g_entities[cs->entityNum].client->ps.eFlags &= ~EF_STAND_IDLE2;
		}
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_PlayAnim

  syntax: playanim <animation> [legs/torso/both] [HOLDFRAME] [numLoops/FOREVER] [target]

  NOTE: any new animations that are needed by the scripting system, will need to be added here
=================
*/
qboolean AICast_ScriptAction_PlayAnim( cast_state_t *cs, char *params ) {
	char *pString, *token, tokens[3][MAX_QPATH];
	int i, endtime, duration, numLoops;
	gclient_t *client;
	gentity_t *ent;
	vec3_t org, vec;
	qboolean forever = qfalse, setAngles = qfalse;
	qboolean holdframe = qfalse;

	if ( cs->fsm.state == AIStateId::LoperLeap ) {
		AI_CancelState( cs, AITransitionReason::Script );
	}

	pString = params;

	client = &level.clients[cs->entityNum];

	if ( level.animScriptData.modelInfo[level.animScriptData.clientModels[cs->entityNum] - 1]->version >
		 1 ) { // new (scripted) model

		// read the name
		token = COM_ParseExt( &pString, qfalse );
		if ( !token || !token[0] ) {
			G_Error( "AI Scripting: syntax error\n\nplayanim <animation> <legs/torso/both>\n" );
		}
		Q_strncpyz( tokens[0], token, sizeof( tokens[0] ) );
		Q_strlwr( tokens[0] );

		// read the body part
		token = COM_ParseExt( &pString, qfalse );
		if ( !token || !token[0] ) {
			G_Error( "AI Scripting: syntax error\n\nplayanim <animation> <legs/torso/both>\n" );
		}
		Q_strncpyz( tokens[1], token, sizeof( tokens[1] ) );
		Q_strlwr( tokens[1] );

		// read the HOLDFRAME (optional)
		token = COM_ParseExt( &pString, qfalse );
		if ( token && token[0] && !Q_strcasecmp( token, "holdframe" ) ) {
			holdframe = qtrue;
			// read the numLoops (optional)
			token = COM_ParseExt( &pString, qfalse );
		}
		// token is parsed above
		if ( token && token[0] ) {
			if ( !Q_strcasecmp( token, "forever" ) ) {
				forever = qtrue;
				numLoops = -1;
				// read the target (optional)
				token = COM_ParseExt( &pString, qfalse );
			} else {
				numLoops = atoi( token );
				if ( !numLoops ) { // must be the target, so set loops to 1
					numLoops = 1;
				} else {
					// read the target (optional)
					token = COM_ParseExt( &pString, qfalse );
				}
			}

			if ( token && token[0] ) {
				// find this targetname
				ent = G_Find( NULL, FOFS( targetname ), token );
				if ( !ent ) {
					ent = AICast_FindEntityForName( token );
					if ( !ent ) {
						G_Error( "AI Scripting: playanim cannot find targetname \"%s\"\n", token );
					}
				}
				// set the view angle manually
				BG_EvaluateTrajectory( &ent->s.pos, level.time, org );
				VectorSubtract( org, cs->clientState->origin, vec );
				VectorNormalize( vec );
				vectoangles( vec, cs->ideal_viewangles );
				VectorCopy( cs->ideal_viewangles, cs->script.castScriptStatus.playanim_viewangles );
				setAngles = qtrue;
			}

		} else {
			numLoops = 1;
		}

		if ( cs->script.castScriptStatus.scriptFlags & SFL_FIRST_CALL ) {
			// first time in here, play the anim
			duration = BG_PlayAnimName( &( client->ps ), tokens[0],
										(animBodyPart_t)BG_IndexForString( tokens[1], animBodyPartsStr, qfalse ), qtrue,
										qfalse, qtrue );
			if ( numLoops == -1 ) {
				cs->script.scriptAnimTime = 0x7fffffff; // maximum time allowed
			} else {
				cs->script.scriptAnimTime = level.time + ( numLoops * duration );
			}
			if ( !strcmp( tokens[1], "torso" ) ) {
				cs->script.scriptAnimNum = client->ps.torsoAnim & ~ANIM_TOGGLEBIT;
				// adjust the duration according to numLoops
				if ( numLoops > 1 ) {
					client->ps.torsoTimer += duration * numLoops;
				} else if ( numLoops == -1 ) {
					client->ps.torsoTimer = 9999;
				}
			} else { // dont move
				cs->script.scriptAnimNum = client->ps.legsAnim & ~ANIM_TOGGLEBIT;
				cs->script.castScriptStatus.scriptNoMoveTime = cs->script.scriptAnimTime;
				// lock the viewangles
				if ( !cs->script.castScriptStatus.playAnimViewlockTime ||
					 cs->script.castScriptStatus.playAnimViewlockTime < level.time ) {
					VectorCopy( cs->ideal_viewangles, cs->script.castScriptStatus.playanim_viewangles );
				}
				cs->script.castScriptStatus.playAnimViewlockTime = cs->script.scriptAnimTime;
				// adjust the duration according to numLoops
				if ( numLoops > 1 ) {
					client->ps.legsTimer += duration * ( numLoops - 1 );
					if ( !strcmp( tokens[1], "both" ) ) {
						client->ps.torsoTimer += duration * ( numLoops - 1 );
					}
				} else if ( numLoops == -1 ) {
					client->ps.legsTimer = 9999;
					if ( !strcmp( tokens[1], "both" ) ) {
						client->ps.torsoTimer = 9999;
					}
				}
			}
		} else {
			if ( holdframe ) {
				// make sure it doesn't stop before the next command can be processed
				if ( !strcmp( tokens[1], "torso" ) ) {
					if ( client->ps.torsoTimer < 400 ) {
						client->ps.torsoTimer = 400;
					}
				} else if ( !strcmp( tokens[1], "legs" ) ) {
					if ( client->ps.legsTimer < 400 ) {
						client->ps.legsTimer = 400;
					}
				} else if ( !strcmp( tokens[1], "both" ) ) {
					if ( client->ps.torsoTimer < 400 ) {
						client->ps.torsoTimer = 400;
					}
					if ( client->ps.legsTimer < 400 ) {
						client->ps.legsTimer = 400;
					}
				}
			}
			// keep it looping if forever
			if ( forever ) {
				if ( !strcmp( tokens[1], "torso" ) ) {
					client->ps.torsoTimer = 9999;
				} else {
					client->ps.legsTimer = 9999;
					if ( !strcmp( tokens[1], "both" ) ) {
						client->ps.torsoTimer = 9999;
					}
				}
				return qfalse;
			}
			// wait for the anim to stop playing
			if ( cs->script.scriptAnimTime <= level.time ) {
				return qtrue;
			}
		}

		return qfalse;

	} else { // old model

		for ( i = 0; i < 3; i++ ) {
			token = COM_ParseExt( &pString, qfalse );
			if ( !token || !token[0] ) {
				//G_Error("AI Scripting: syntax error\n\nplayanim <animation> <pausetime> [legs/torso/both]\n");
				G_Printf( "AI Scripting: syntax error\n\nplayanim <animation> <pausetime> <legs/torso/both>\n" );
				return qtrue;
			} else {
				Q_strncpyz( tokens[i], token, sizeof( tokens[i] ) );
			}
		}

		Q_strlwr( tokens[2] );

		endtime = cs->script.castScriptStatus.castScriptStackChangeTime + atoi( tokens[1] );
		duration = endtime - level.time + 200; // so animations don't run out before starting a next animation
		if ( duration > 2000 ) {
			duration = 2000;
		}

		cs->script.scriptAnimTime = level.time;

		if ( duration < 200 ) {
			return qtrue; // done playing animation
		}
		// call the anims directly based on the animation token

		//if (cs->script.castScriptStatus.castScriptStackChangeTime == level.time) {
		for ( i = 0; i < MAX_ANIMATIONS; i++ ) {
			if ( !Q_strcasecmp( tokens[0], animStrings[i] ) ) {
				if ( !Q_strcasecmp( tokens[2], "torso" ) ) {
					if ( ( client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) != i ) {
						client->ps.torsoAnim = ( ( client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | i;
					}
					client->ps.torsoTimer = duration;
				} else if ( !Q_strcasecmp( tokens[2], "legs" ) ) {
					if ( ( client->ps.legsAnim & ~ANIM_TOGGLEBIT ) != i ) {
						client->ps.legsAnim = ( ( client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | i;
					}
					client->ps.legsTimer = duration;
				} else if ( !Q_strcasecmp( tokens[2], "both" ) ) {
					if ( ( client->ps.torsoAnim & ~ANIM_TOGGLEBIT ) != i ) {
						client->ps.torsoAnim = ( ( client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | i;
					}
					client->ps.torsoTimer = duration;
					if ( ( client->ps.legsAnim & ~ANIM_TOGGLEBIT ) != i ) {
						client->ps.legsAnim = ( ( client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | i;
					}
					client->ps.legsTimer = duration;
				} else {
					G_Printf( "AI Scripting: syntax error\n\nplayanim <animation> <pausetime> <legs/torso/both>\n" );
				}
				break;
			}
		}
		if ( i == MAX_ANIMATIONS ) {
			G_Printf( "AI Scripting: playanim has unknown or invalid animation \"%s\"\n", tokens[0] );
		}
		//}

		if ( !strcmp( tokens[2], "torso" ) ) {
			cs->script.scriptAnimNum = client->ps.torsoAnim & ~ANIM_TOGGLEBIT;
		} else {
			cs->script.scriptAnimNum = client->ps.legsAnim & ~ANIM_TOGGLEBIT;
		}

		if ( cs->script.castScriptStatus.scriptNoMoveTime < level.time + 300 ) {
			cs->script.castScriptStatus.scriptNoMoveTime = level.time + 300;
		}
		if ( cs->script.castScriptStatus.scriptNoAttackTime < level.time + 300 ) {
			cs->script.castScriptStatus.scriptNoAttackTime = level.time + 300;
		}
		return qfalse;
	}
};

/*
=================
AICast_ScriptAction_ClearAnim

  stops any animation that is currently playing
=================
*/
qboolean AICast_ScriptAction_ClearAnim( cast_state_t *cs, char *params ) {
	gclient_t *client;

	client = &level.clients[cs->entityNum];

	client->ps.torsoTimer = 0;
	client->ps.legsTimer = 0;

	// let us move again
	cs->script.castScriptStatus.scriptNoMoveTime = 0;

	return qtrue;
}

/*
=================
AICast_ScriptAction_FaceTargetAngles

  syntax: facetargetangles <targetname>

  The AI will face the same direction that the target entity is facing
=================
*/
qboolean AICast_ScriptAction_FaceTargetAngles( cast_state_t *cs, char *params ) {
	gentity_t *targetEnt;

	if ( !params || !params[0] ) {
		G_Error( "AI Scripting: facetargetangles requires a targetname\n" );
	}

	targetEnt = G_Find( NULL, FOFS( targetname ), params );
	if ( !targetEnt ) {
		G_Error( "AI Scripting: cannot find targetname \"%s\"\n", params );
	}

	VectorCopy( targetEnt->s.angles, cs->ideal_viewangles );

	return qtrue;
}

/*
==================
AICast_ScriptAction_NoSight

  syntax: nosight <duration>
==================
*/
qboolean AICast_ScriptAction_NoSight( cast_state_t *cs, char *params ) {
	if ( !params ) {
		G_Error( "AI Scripting: syntax error\n\nnosight <duration>\n" );
	}

	cs->script.castScriptStatus.scriptNoSightTime = level.time + atoi( params );

	return qtrue;
}

/*
==================
AICast_ScriptAction_Sight

  syntax: sight
==================
*/
qboolean AICast_ScriptAction_Sight( cast_state_t *cs, char *params ) {
	cs->script.castScriptStatus.scriptNoSightTime = 0;
	return qtrue;
}

//----(SA)	added some music interface

/*
==================
AICast_ScriptAction_MusicStart

==================
*/
qboolean AICast_ScriptAction_MusicStart( cast_state_t *cs, char *params ) {
	char *pString, *token;
	char cvarName[MAX_QPATH];
	int fadeupTime = 0;

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: mu_start <musicfile> <fadeuptime>" );
	}
	Q_strncpyz( cvarName, token, sizeof( cvarName ) );

	token = COM_ParseExt( &pString, qfalse );
	if ( token[0] ) {
		fadeupTime = atoi( token );
	}

	trap_SendServerCommand( cs->entityNum, va( "mu_start %s %d", cvarName, fadeupTime ) );

	return qtrue;
}

/*
==================
AICast_ScriptAction_MusicPlay

==================
*/
qboolean AICast_ScriptAction_MusicPlay( cast_state_t *cs, char *params ) {
	char *pString, *token;
	char cvarName[MAX_QPATH];
	int fadeupTime = 0;

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: mu_play <musicfile> [fadeup time]" );
	}
	Q_strncpyz( cvarName, token, sizeof( cvarName ) );

	trap_SendServerCommand( cs->entityNum, va( "mu_play %s %d", cvarName, fadeupTime ) );

	return qtrue;
}

/*
==================
AICast_ScriptAction_MusicStop
==================
*/
qboolean AICast_ScriptAction_MusicStop( cast_state_t *cs, char *params ) {
	char *pString, *token;
	int fadeoutTime = 0;

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( token[0] ) {
		fadeoutTime = atoi( token );
	}

	trap_SendServerCommand( cs->entityNum, va( "mu_stop %i", fadeoutTime ) );

	return qtrue;
}

/*
==================
AICast_ScriptAction_MusicFade
==================
*/
qboolean AICast_ScriptAction_MusicFade( cast_state_t *cs, char *params ) {
	char *pString, *token;
	float targetvol;
	int fadetime;

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: mu_fade <targetvol> <fadetime>" );
	}
	targetvol = atof( token );

	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: mu_fade <targetvol> <fadetime>" );
	}
	fadetime = atoi( token );

	trap_SendServerCommand( cs->entityNum, va( "mu_fade %f %i", targetvol, fadetime ) );

	return qtrue;
}

/*
==================
AICast_ScriptAction_MusicQueue
==================
*/
qboolean AICast_ScriptAction_MusicQueue( cast_state_t *cs, char *params ) {
	char *pString, *token;
	char cvarName[MAX_QPATH];

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: mu_queue <musicfile>" );
	}
	Q_strncpyz( cvarName, token, sizeof( cvarName ) );

	trap_SetConfigstring( CS_MUSIC_QUEUE, cvarName );

	return qtrue;
}

/*
==================
AICast_ScriptAction_AnimCondition

  syntax: anim_condition <condition> <string>
==================
*/
qboolean AICast_ScriptAction_AnimCondition( cast_state_t *cs, char *params ) {
	char *pString, *token;
	char condition[MAX_QPATH];

	pString = params;
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: anim_condition <condition> <string>" );
	}
	Q_strncpyz( condition, token, sizeof( condition ) );
	//
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI_Scripting: syntax: anim_condition <condition> <string>" );
	}
	//
	BG_UpdateConditionValueStrings( cs->entityNum, condition, token );
	//
	return qtrue;
}
