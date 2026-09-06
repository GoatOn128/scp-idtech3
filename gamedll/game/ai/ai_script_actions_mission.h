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

#ifndef ICEWOLF_AI_SCRIPT_ACTIONS_MISSION_H
#define ICEWOLF_AI_SCRIPT_ACTIONS_MISSION_H

#include "ai_actor.h"

qboolean AICast_ScriptAction_GotoCast( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_AbortIfLoadgame( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Wait( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Trigger( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_AlertEntity( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_SaveGame( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Accum( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_SpawnCast( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_MissionFailed( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_ObjectivesNeeded( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_ObjectiveMet( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Print( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_ResetScript( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Mount( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Unmount( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_SavePersistant( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Teleport( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_EndGame( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_ChangeLevel( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_FoundSecret( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_NoAvoid( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Avoid( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Attrib( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_DenyAction( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Headlook( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_BackupScript( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_RestoreScript( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_StateType( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_KnockBack( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Zoom( cast_state_t *cs, char *params );
qboolean ScriptStartCam( cast_state_t *cs, char *params, qboolean black );
qboolean AICast_ScriptAction_StartCam( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_StartCamBlack( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_StopCamBlack( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_StopCam( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Cigarette( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Parachute( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_EntityScriptName( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_AIScriptName( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_SetHealth( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_NoTarget( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Cvar( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_LockPlayer( cast_state_t *cs, char *params );

#endif
