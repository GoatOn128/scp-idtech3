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

static int numaifuncs;
static char *aifuncs[MAX_AIFUNCS];

/*
==========
AICast_DBG_InitAIFuncs
==========
*/
void AICast_DBG_InitAIFuncs( void ) {
	numaifuncs = 0;
}

/*
==========
AICast_DBG_AddAIFunc
==========
*/
void AICast_DBG_AddAIFunc( cast_state_t *cs, char *funcname ) {
	if ( aicast_debug.integer ) {
		if ( aicast_debug.integer != 2 || ( g_entities[cs->entityNum].aiName &&
											!strcmp( aicast_debugname.string, g_entities[cs->entityNum].aiName ) ) ) {
			G_Printf( "%s: %s\n", g_entities[cs->entityNum].aiName, funcname );
		}
	}
	if ( numaifuncs < MAX_AIFUNCS ) {
		aifuncs[numaifuncs++] = funcname;
	}
}

/*
==========
AICast_DBG_ListAIFuncs
==========
*/
void AICast_DBG_ListAIFuncs( cast_state_t *cs, int numprint ) {
	int i;

	if ( aicast_debug.integer != 2 || ( g_entities[cs->entityNum].aiName &&
										!strcmp( aicast_debugname.string, g_entities[cs->entityNum].aiName ) ) ) {
		AICast_Printf( AICAST_PRT_DEBUG, S_COLOR_RED "AICast_ProcessAIFunctions: recorded %d AI transitions\n",
					   numaifuncs );
		for ( i = ( numaifuncs > numprint ? numaifuncs - numprint : 0 ); i < numaifuncs; i++ )
			AICast_Printf( AICAST_PRT_DEBUG, "%s, ", aifuncs[i] );
		AICast_Printf( AICAST_PRT_DEBUG, "\n" );
	}
}

/*
==========
AICast_DebugFrame
==========
*/
void AICast_DebugFrame( cast_state_t *cs ) {
	gentity_t *ent;

	if ( aicast_debug.integer ) {
		ent = &g_entities[cs->entityNum];

		if ( cs->script.castScriptStatus.castScriptEventIndex >= 0 ) {
			ent->client->ps.eFlags |= EF_TALK;
		} else {
			ent->client->ps.eFlags &= ~EF_TALK;
		}
	}
}

/*
===========
AICast_DBG_RouteTable_f
===========
*/
void AICast_DBG_RouteTable_f( vec3_t org, char *param ) {
	static int srcarea = 0, dstarea = 0;
	//	extern botlib_export_t botlib; // TTimo: unused

	if ( !param || strlen( param ) < 1 ) {
		trap_Printf( "You must specify 'src', 'dest' or 'show'\n" );
		return;
	}

	trap_AAS_SetCurrentWorld( 0 ); // use the default world, which should have a routetable

	if ( Q_stricmp( param, "toggle" ) == 0 ) {
		trap_AAS_RT_ShowRoute( vec3_origin, -666, -666 ); // stupid toggle hack
		return;
	}

	if ( Q_stricmp( param, "src" ) == 0 ) { // set the src
		srcarea = 1 + trap_AAS_PointAreaNum( org );
		return;
	} else if ( Q_stricmp( param, "dest" ) == 0 ) {
		dstarea = 1 + trap_AAS_PointAreaNum( org );
	}

	if ( srcarea && dstarea ) { // show the path
		trap_AAS_RT_ShowRoute( org, srcarea - 1, dstarea - 1 );
	} else {
		trap_Printf( "You must specify 'src' & 'dest' first\n" );
	}
}

/*
===============
AICast_DBG_Spawn_f
===============
*/
void AICast_DBG_Spawn_f( gclient_t *client, char *cmd ) {
	extern qboolean G_CallSpawn( gentity_t * ent );
	gentity_t *ent;
	vec3_t dir;

	ent = G_Spawn();
	ent->classname = (char *)G_Alloc( strlen( cmd ) + 1 );
	strcpy( ent->classname, cmd );
	AngleVectors( client->ps.viewangles, dir, NULL, NULL );
	VectorMA( client->ps.origin, 96, dir, ent->s.origin );

	if ( !G_CallSpawn( ent ) ) {
		G_Printf( "Error: unable to spawn \"%s\" entity\n", cmd );
	}
}

/*
===============
AICast_DBG_Cmd_f

  General entry point for all "aicast ..." commands
===============
*/
void AICast_DBG_Cmd_f( int clientNum ) {
	gentity_t *ent;
	char cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return; // not fully in game yet
	}

	// get the first word following "aicast"
	trap_Argv( 1, cmd, sizeof( cmd ) );

	if ( Q_stricmp( cmd, "dbg_routetable" ) == 0 ) {
		trap_Argv( 2, cmd, sizeof( cmd ) );
		AICast_DBG_RouteTable_f( ent->client->ps.origin, cmd );
		return;
	}
	if ( Q_stricmp( cmd, "spawn" ) == 0 ) {
		// spawn a given character
		trap_Argv( 2, cmd, sizeof( cmd ) );
		AICast_DBG_Spawn_f( ent->client, cmd );
		return;
	}
	if ( Q_stricmp( cmd, "getname" ) == 0 ) {
		// get name of character we're looking at
		//		AICast_DBG_GetName_f(ent);
		return;
	}
	if ( Q_stricmp( cmd, "followme" ) == 0 ) {
		// tell character to follow us
		trap_Argv( 2, cmd, sizeof( cmd ) );
		//		AICast_DBG_FollowMe_f(ent->client, cmd);
		return;
	}
}
