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

#ifndef ICEWOLF_AI_BEHAVIOR_PURSUIT_H
#define ICEWOLF_AI_BEHAVIOR_PURSUIT_H

#include "../ai_actor.h"

char *AIFunc_ChaseGoalIdle( cast_state_t *cs );
char *AIFunc_ChaseGoalIdleStart( cast_state_t *cs, int entitynum, float reachdist );
char *AIFunc_ChaseGoal( cast_state_t *cs );
char *AIFunc_ChaseGoalStart( cast_state_t *cs, int entitynum, float reachdist, qboolean slowApproach );
char *AIFunc_BattleHunt( cast_state_t *cs );
char *AIFunc_BattleHuntStart( cast_state_t *cs );
char *AIFunc_BattleChase( cast_state_t *cs );
char *AIFunc_BattleChaseStart( cast_state_t *cs );

#endif
