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

#ifndef ICEWOLF_AI_NAVIGATION_H
#define ICEWOLF_AI_NAVIGATION_H

#include "ai_actor.h"

float AICast_GetRandomViewAngle( cast_state_t *cs, float tracedist );
bot_moveresult_t *AICast_MoveToPos( cast_state_t *cs, vec3_t pos, int entnum );
float AICast_SpeedScaleForDistance( cast_state_t *cs, float startdist, float idealDist );
bot_moveresult_t AICast_CombatMove( cast_state_t *cs, int tfl );
qboolean AICast_GetTakeCoverPos( cast_state_t *cs, int enemyNum, vec3_t enemyPos, vec3_t returnPos );

int AI_PointAreaNum( vec3_t origin );
void AI_SetupMovement( cast_state_t *cs );
qboolean AI_InSlime( ai_client_state_t *state );
qboolean AI_InLava( ai_client_state_t *state );

#endif
