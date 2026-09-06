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

#ifndef ICEWOLF_AI_SCRIPT_ACTIONS_COMBAT_H
#define ICEWOLF_AI_SCRIPT_ACTIONS_COMBAT_H

#include "ai_actor.h"

void AICast_NoAttackIfNotHurtSinceLastScriptAction( cast_state_t *cs );
qboolean AICast_ScriptAction_NoAttack( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_Attack( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_SetAmmo( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_SetClip( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_SuggestWeapon( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_SelectWeapon( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_SetArmor( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_GiveArmor( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_GiveWeapon( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_TakeWeapon( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_GiveInventory( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_FireAtTarget( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_GodMode( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_NoAIDamage( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_LightningDamage( cast_state_t *cs, char *params );
qboolean AICast_ScriptAction_CatchFire( cast_state_t *cs, char *params );

#endif
