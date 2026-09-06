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

#ifndef ICEWOLF_AI_DEBUG_H
#define ICEWOLF_AI_DEBUG_H

#include "ai_actor.h"

void AICast_DBG_InitAIFuncs( void );
void AICast_DBG_AddAIFunc( cast_state_t *cs, char *funcname );
void AICast_DBG_ListAIFuncs( cast_state_t *cs, int numprint );
void AICast_DebugFrame( cast_state_t *cs );
void AICast_DBG_RouteTable_f( vec3_t org, char *param );
void AICast_DBG_Spawn_f( gclient_t *client, char *cmd );
void AICast_DBG_Cmd_f( int clientNum );

#endif
