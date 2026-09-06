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

#ifndef ICEWOLF_AI_TARGETING_H
#define ICEWOLF_AI_TARGETING_H

#include "ai_actor.h"

int AICast_ScanForEnemies( cast_state_t *cs, int *enemies );
qboolean AICast_HostileEnemy( cast_state_t *cs, int enemynum );
qboolean AICast_QueryEnemy( cast_state_t *cs, int enemynum );
qboolean AICast_SameTeam( cast_state_t *cs, int enemynum );

#endif
