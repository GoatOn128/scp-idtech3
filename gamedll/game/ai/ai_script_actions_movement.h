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

#ifndef ICEWOLF_AI_SCRIPT_ACTIONS_MOVEMENT_H
#define ICEWOLF_AI_SCRIPT_ACTIONS_MOVEMENT_H

#include "ai_actor.h"

qboolean AICast_ScriptAction_GotoMarker( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_WalkToMarker( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_CrouchToMarker( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_WalkToCast( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_CrouchToCast( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_FollowCast( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Movetype( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_ExplicitRouting( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_PushAway( cast_state_t *cs, char *params );

#endif
