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

#include "../../precompiled.h"

#include "ai_local.h"

/*
===============
AICast_ScriptAction_GotoMarker

  syntax: gotomarker <targetname> [firetarget [noattack]] [nostop] OR runtomarker <targetname> [firetarget [noattack]] [nostop]
===============
*/
qboolean AICast_ScriptAction_GotoMarker( cast_state_t *cs, char *params ) {
#define SCRIPT_REACHGOAL_DIST 8
	idLexer lexer( LEXFL_ONLYSTRINGS | LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWDOLLARNAMES | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS );
	idToken token;
	gentity_t *ent;
	idVec3 vec, org;
	int i, diff;
	qboolean slowApproach;

	ent = NULL;

	// if we are avoiding danger, then wait for the danger to pass
	if ( cs->script.castScriptStatus.scriptGotoId < 0 && cs->dangerEntityValidTime > level.time ) {
		return qfalse;
	}
	// if we are in a special func, then wait
	if ( cs->aiFlags & AIFL_SPECIAL_FUNC ) {
		return qfalse;
	}

	lexer.LoadMemory( params ? params : "", params ? strlen( params ) : 0, "ai_script_actions_movement" );
	lexer.ReadTokenOnLine( &token );
	if ( !token[0] ) {
		G_Error( "AI scripting: gotomarker must have an targetname\n" );
	}

	// if we already are going to the marker, just use that, and check if we're in range
	if ( cs->script.castScriptStatus.scriptGotoEnt >= 0 &&
		 cs->script.castScriptStatus.scriptGotoId == cs->thinkFuncChangeTime ) {
		ent = &g_entities[cs->script.castScriptStatus.scriptGotoEnt];
		if ( ent->targetname && !idStr::Icmp( ent->targetname, token.c_str() ) ) {
			// if we're not slowing down, then check for passing the marker, otherwise check distance only
			vec = ent->r.currentOrigin - cs->clientState->origin;
			//
			if ( cs->navigation.followSlowApproach && vec.Length() < cs->navigation.followDist ) {
				cs->navigation.followTime = 0;
				AIFunc_IdleStart( cs ); // resume normal AI
				return qtrue;
			} else if ( !cs->navigation.followSlowApproach &&
						vec.Length() < 64 /*&& ( cs->clientState->cur_ps.velocity * vec ) < 0*/ ) {
				cs->navigation.followTime = 0;
				AIFunc_IdleStart( cs ); // resume normal AI
				return qtrue;
			} else {
				// do we have a firetarget ?
				lexer.ReadTokenOnLine( &token );
				if ( !token[0] || !idStr::Icmp( token.c_str(), "nostop" ) ) {
					AICast_NoAttackIfNotHurtSinceLastScriptAction( cs );
				} else { // yes we do
					// find this targetname
					ent = G_Find( NULL, FOFS( targetname ), ( char * ) token.c_str() );
					if ( !ent ) {
						ent = AICast_FindEntityForName( ( char * ) token.c_str() );
						if ( !ent ) {
							G_Error( "AI Scripting: gotomarker cannot find targetname \"%s\"\n", token.c_str() );
						}
					}
					// set the view angle manually
					BG_EvaluateTrajectory( &ent->s.pos, level.time, org );
					vec = org - cs->clientState->origin;
					vec.Normalize();
					{
						const idAngles convertedAngles = vec.ToAngles();
						cs->ideal_viewangles.Set( convertedAngles.pitch, convertedAngles.yaw, convertedAngles.roll );
					}
					// noattack?
					lexer.ReadTokenOnLine( &token );
					if ( !token[0] || idStr::Icmp( token.c_str(), "noattack" ) ) {
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

	// find the ai_marker with the given "targetname"
	// TTimo: gcc: suggest () around assignment used as truth value
	while ( ( ent = G_Find( ent, FOFS( classname ), "ai_marker" ) ) ) {
		if ( ent->targetname && !idStr::Icmp( ent->targetname, token.c_str() ) ) {
			break;
		}
	}

	if ( !ent ) {
		G_Error( "AI Scripting: can't find ai_marker with \"targetname\" = \"%s\"\n", token.c_str() );
	}

	if ( ( cs->clientState->origin - ent->r.currentOrigin ).Length() < SCRIPT_REACHGOAL_DIST ) { // we made it
		return qtrue;
	}

	cs->script.castScriptStatus.scriptNoMoveTime = 0;
	cs->script.castScriptStatus.scriptGotoEnt = ent->s.number;
	//
	// slow approach to the goal?
	if ( !params || !strstr( params, " nostop" ) ) {
		slowApproach = qtrue;
	} else {
		slowApproach = qfalse;
	}
	//
	AIFunc_ChaseGoalStart( cs, ent->s.number, ( slowApproach ? SCRIPT_REACHGOAL_DIST : 32 ), slowApproach );
	cs->navigation.followIsGoto = qtrue;
	cs->navigation.followTime = 0x7fffffff; // make sure it gets through for the first frame
	cs->script.castScriptStatus.scriptGotoId = cs->thinkFuncChangeTime;

	AICast_NoAttackIfNotHurtSinceLastScriptAction( cs );
	return qfalse;
}

/*
===============
AICast_ScriptAction_WalkToMarker

  syntax: walktomarker <targetname> [firetarget [noattack]] [nostop]
===============
*/
qboolean AICast_ScriptAction_WalkToMarker( cast_state_t *cs, char *params ) {
	// if we are avoiding danger, then wait for the danger to pass
	if ( cs->script.castScriptStatus.scriptGotoId < 0 && cs->dangerEntityValidTime > level.time ) {
		return qfalse;
	}
	// if we are in a special func, then wait
	if ( cs->aiFlags & AIFL_SPECIAL_FUNC ) {
		return qfalse;
	}
	if ( !AICast_ScriptAction_GotoMarker( cs, params ) ||
		 ( !strstr( params, " nostop" ) && cs->clientState->cur_ps.velocity.Length() ) ) {
		cs->movestate = MS_WALK;
		cs->movestateType = MSTYPE_TEMPORARY;
		AICast_NoAttackIfNotHurtSinceLastScriptAction( cs );
		return qfalse;
	}

	return qtrue;
}

/*
===============
AICast_ScriptAction_CrouchToMarker

  syntax: crouchtomarker <targetname> [firetarget [noattack]] [nostop]
===============
*/
qboolean AICast_ScriptAction_CrouchToMarker( cast_state_t *cs, char *params ) {
	// if we are avoiding danger, then wait for the danger to pass
	if ( cs->script.castScriptStatus.scriptGotoId < 0 && cs->dangerEntityValidTime > level.time ) {
		return qfalse;
	}
	// if we are in a special func, then wait
	if ( cs->aiFlags & AIFL_SPECIAL_FUNC ) {
		return qfalse;
	}
	if ( !AICast_ScriptAction_GotoMarker( cs, params ) ||
		 ( !strstr( params, " nostop" ) && cs->clientState->cur_ps.velocity.Length() ) ) {
		cs->movestate = MS_CROUCH;
		cs->movestateType = MSTYPE_TEMPORARY;
		AICast_NoAttackIfNotHurtSinceLastScriptAction( cs );
		return qfalse;
	}

	return qtrue;
}

/*
===============
AICast_ScriptAction_WalkToCast

  syntax: walktocast <ainame> [firetarget [noattack]]
===============
*/
qboolean AICast_ScriptAction_WalkToCast( cast_state_t *cs, char *params ) {
	// if we are avoiding danger, then wait for the danger to pass
	if ( cs->script.castScriptStatus.scriptGotoId < 0 && cs->dangerEntityValidTime > level.time ) {
		return qfalse;
	}
	// if we are in a special func, then wait
	if ( cs->aiFlags & AIFL_SPECIAL_FUNC ) {
		return qfalse;
	}
	if ( !AICast_ScriptAction_GotoCast( cs, params ) ) {
		cs->movestate = MS_WALK;
		cs->movestateType = MSTYPE_TEMPORARY;
		AICast_NoAttackIfNotHurtSinceLastScriptAction( cs );
		return qfalse;
	}

	return qtrue;
}

/*
===============
AICast_ScriptAction_CrouchToCast

  syntax: crouchtocast <ainame> [firetarget [noattack]]
===============
*/
qboolean AICast_ScriptAction_CrouchToCast( cast_state_t *cs, char *params ) {
	// if we are avoiding danger, then wait for the danger to pass
	if ( cs->script.castScriptStatus.scriptGotoId < 0 && cs->dangerEntityValidTime > level.time ) {
		return qfalse;
	}
	// if we are in a special func, then wait
	if ( cs->aiFlags & AIFL_SPECIAL_FUNC ) {
		return qfalse;
	}
	if ( !AICast_ScriptAction_GotoCast( cs, params ) ) {
		cs->movestate = MS_CROUCH;
		cs->movestateType = MSTYPE_TEMPORARY;
		AICast_NoAttackIfNotHurtSinceLastScriptAction( cs );
		return qfalse;
	}

	return qtrue;
}

/*
===================
AICast_ScriptAction_FollowCast

  syntax: followcast <ainame>
===================
*/
qboolean AICast_ScriptAction_FollowCast( cast_state_t *cs, char *params ) {
	gentity_t *ent;

	// find the cast/player with the given "name"
	ent = AICast_FindEntityForName( params );
	if ( !ent ) {
		G_Error( "AI Scripting: can't find AI cast with \"ainame\" = \"%s\"\n", params );
	}

	AIFunc_ChaseGoalStart( cs, ent->s.number, 64, qtrue );

	return qtrue;
};

//----(SA)	end

/*
=================
AICast_ScriptAction_Movetype

  syntax: movetype <walk/run/crouch/default>

  Sets this character's movement type, will exist until another movetype command is called
=================
*/
qboolean AICast_ScriptAction_Movetype( cast_state_t *cs, char *params ) {
	if ( !idStr::Icmp( params, "walk" ) ) {
		cs->movestate = MS_WALK;
		cs->movestateType = MSTYPE_PERMANENT;
		return qtrue;
	}
	if ( !idStr::Icmp( params, "run" ) ) {
		cs->movestate = MS_RUN;
		cs->movestateType = MSTYPE_PERMANENT;
		return qtrue;
	}
	if ( !idStr::Icmp( params, "crouch" ) ) {
		cs->movestate = MS_CROUCH;
		cs->movestateType = MSTYPE_PERMANENT;
		return qtrue;
	}
	if ( !idStr::Icmp( params, "default" ) ) {
		cs->movestate = MS_DEFAULT;
		cs->movestateType = MSTYPE_NONE;
		return qtrue;
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_ExplicitRouting

  syntax: explicit_routing <on/off>
=================
*/

qboolean AICast_ScriptAction_ExplicitRouting( cast_state_t *cs, char *params ) {
	if ( !params || !params[0] ) {
		G_Error( "AI Scripting: explicit_routing requires an on/off specifier\n" );
	}

	if ( !idStr::Icmp( params, "on" ) ) {
		cs->aiFlags |= AIFL_EXPLICIT_ROUTING;
	} else if ( !idStr::Icmp( params, "off" ) ) {
		cs->aiFlags &= ~AIFL_EXPLICIT_ROUTING;
	} else {
		G_Error( "AI Scripting: explicit_routing requires an on/off specifier\n" );
	}

	return qtrue;
}

/*
================
AICast_ScriptAction_PushAway

  syntax: pushaway <ainame>
================
*/
qboolean AICast_ScriptAction_PushAway( cast_state_t *cs, char *params ) {
	gentity_t *pushed;
	idVec3 v, f, r;
	idAngles ang;

	if ( !params || !params[0] ) {
		G_Error( "AI_Scripting: syntax: pushaway <ainame>" );
	}
	// find them
	pushed = AICast_FindEntityForName( params );
	if ( !pushed ) {
		G_Error( "AI_Scripting: pushaway: cannot find \"%s\"", params );
	}
	// calc the vecs
	v = pushed->s.pos.trBase - cs->clientState->origin;
	v.Normalize();
	ang = v.ToAngles();
	ang.ToVectors( &f, &r, NULL );
	// push them away and to the side
	v = f * 200;
	v = v + ( level.time % 5000 < 2500 ? 200 : 200 ) * r;
	pushed->client->ps.velocity = pushed->client->ps.velocity + v;
	//
	return qtrue;
}
