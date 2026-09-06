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

#ifndef ICEWOLF_AI_MOVEMENT_H
#define ICEWOLF_AI_MOVEMENT_H

#include "ai_actor.h"

void AICast_SpecialFunc( cast_state_t *cs );
void AICast_PredictMovement( cast_state_t *cs, int numframes, float frametime, aicast_predictmove_t *move,
							 usercmd_t *ucmd, int checkHitEnt );
qboolean AICast_GetAvoid( cast_state_t *cs, bot_goal_t *goal, vec3_t outpos, qboolean reverse, int blockEnt );
void AICast_Blocked( cast_state_t *cs, bot_moveresult_t *moveresult, int activate, bot_goal_t *goal );
void AICast_EvaluatePmove( int clientnum, pmove_t *pm );
qboolean AICast_RequestCrouchAttack( cast_state_t *cs, vec3_t org, float time );
void AICast_DeadClipWalls( cast_state_t *cs );

#endif
