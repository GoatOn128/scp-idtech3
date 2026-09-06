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

#ifndef ICEWOLF_AI_SCRIPT_H
#define ICEWOLF_AI_SCRIPT_H

#include "ai_actor.h"

qboolean AICast_EventMatch_StringEqual( cast_script_event_t *event, char *eventParm );
qboolean AICast_EventMatch_IntInRange( cast_script_event_t *event, char *eventParm );
int AICast_EventForString( char *string );
void AICast_ScriptLoad( void );
void AICast_ScriptParse( cast_state_t *cs );
void AICast_ScriptChange( cast_state_t *cs, int newScriptNum );
void AICast_ScriptEvent( struct cast_state_s *cs, char *eventStr, char *params );
void AICast_ForceScriptEvent( struct cast_state_s *cs, char *eventStr, char *params );
qboolean AICast_ScriptRun( cast_state_t *cs, qboolean force );

#endif
