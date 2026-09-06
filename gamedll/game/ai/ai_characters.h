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

#ifndef ICEWOLF_AI_CHARACTERS_H
#define ICEWOLF_AI_CHARACTERS_H

#include "ai_actor.h"

void AIChar_SetBBox( gentity_t *ent, cast_state_t *cs, qboolean useHeadTag );
void AIChar_Death( gentity_t *ent, gentity_t *attacker, int damage, int mod );
int AIChar_GetPainLocation( gentity_t *ent, vec3_t point );
void AIChar_Pain( gentity_t *ent, gentity_t *attacker, int damage, vec3_t point );
void AIChar_Sight( gentity_t *ent, gentity_t *other, int lastSight );
void AIChar_AttackSound( cast_state_t *cs );

#endif
