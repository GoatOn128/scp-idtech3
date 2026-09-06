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

#ifndef ICEWOLF_AI_BOSS_HEINRICH_H
#define ICEWOLF_AI_BOSS_HEINRICH_H

#include "../ai_actor.h"

void AICast_Heinrich_SoundPrecache( void );
void AICast_Heinrich_Taunt( cast_state_t *cs );
char *AIFunc_Heinrich_SwordLunge( cast_state_t *cs );
char *AIFunc_Heinrich_SwordLungeStart( cast_state_t *cs );
char *AIFunc_Heinrich_SwordKnockback( cast_state_t *cs );
char *AIFunc_Heinrich_SwordKnockbackStart( cast_state_t *cs );
char *AIFunc_Heinrich_SwordSideSlash( cast_state_t *cs );
char *AIFunc_Heinrich_SwordSideSlashStart( cast_state_t *cs );
char *AIFunc_Heinrich_Earthquake( cast_state_t *cs );
char *AIFunc_Heinrich_MeleeStart( cast_state_t *cs );
char *AIFunc_Heinrich_RaiseDead( cast_state_t *cs );
char *AIFunc_Heinrich_RaiseDeadStart( cast_state_t *cs );
char *AIFunc_Heinrich_SpawnSpiritsStart( cast_state_t *cs );

extern const ai_ability_set_t ai_HeinrichAbilities;

#endif
