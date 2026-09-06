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

#ifndef ICEWOLF_AI_COMBAT_H
#define ICEWOLF_AI_COMBAT_H

#include "../ai_actor.h"

char *AIFunc_Battle( cast_state_t *cs );
char *AIFunc_BattleStart( cast_state_t *cs );
void AICast_UpdateBattleInventory( cast_state_t *cs, int enemy );
float AICast_Aggression( cast_state_t *cs );
int AICast_WantsToChase( cast_state_t *cs );
int AICast_WantsToTakeCover( cast_state_t *cs, qboolean attacking );
qboolean AICast_AIDamageOK( cast_state_t *cs, cast_state_t *ocs );
qboolean AICast_WantToRetreat( cast_state_t *cs );
qboolean AICast_AllowFlameDamage( int entNum );

#endif
