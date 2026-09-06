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

#ifndef ICEWOLF_AI_EVENTS_H
#define ICEWOLF_AI_EVENTS_H

#include "ai_actor.h"

void AICast_Sight( gentity_t *ent, gentity_t *other, int lastSight );
void AICast_Pain( gentity_t *targ, gentity_t *attacker, int damage, vec3_t point );
void AICast_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
void AICast_EndChase( cast_state_t *cs );
void AICast_AIDoor_Touch( gentity_t *ent, gentity_t *aidoor_trigger, gentity_t *door );
void AICast_ProcessActivate( int entNum, int activatorNum );
void AICast_RecordScriptSound( int client );

#endif
