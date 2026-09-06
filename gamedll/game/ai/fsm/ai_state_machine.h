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

#ifndef ICEWOLF_AI_STATE_MACHINE_H
#define ICEWOLF_AI_STATE_MACHINE_H

#include "../ai_actor.h"

char *AIFunc_Restore( cast_state_t *cs );
char *AIFunc_DefaultStart( cast_state_t *cs );
void AICast_ProcessAIFunctions( cast_state_t *cs, float thinktime );

AIStateId AI_StateParent( AIStateId state );
const char *AI_StateName( AIStateId state );
bool AI_HasLivingTarget( const cast_state_t *cs );
char *AI_RequestState( cast_state_t *cs, AIStateId state, AITransitionReason reason );
void AI_CommitState( cast_state_t *cs );
void AI_SetLegacyState( cast_state_t *cs, ai_state_callback_t callback );
void AI_CancelState( cast_state_t *cs, AITransitionReason reason );

// Entry implementations are invoked only by the transition boundary.
char *AI_EnterIdle( cast_state_t *cs );
char *AI_EnterCombat( cast_state_t *cs );
char *AI_EnterChase( cast_state_t *cs );
char *AI_EnterLoperLeap( cast_state_t *cs );
bool AI_CanStartLoperLeap( const cast_state_t *cs );
void AI_ExitLoperLeap( cast_state_t *cs );

#endif
