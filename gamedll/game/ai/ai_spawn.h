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

#ifndef ICEWOLF_AI_SPAWN_H
#define ICEWOLF_AI_SPAWN_H

#include "ai_actor.h"

void AIChar_spawn( gentity_t *ent );
void SP_ai_soldier( gentity_t *ent );
void SP_ai_american( gentity_t *ent );
void SP_ai_zombie( gentity_t *ent );
void SP_ai_warzombie( gentity_t *ent );
void SP_ai_venom( gentity_t *ent );
void SP_ai_loper( gentity_t *ent );
void SP_ai_boss_helga( gentity_t *ent );
void SP_ai_boss_heinrich( gentity_t *ent );
void SP_ai_partisan( gentity_t *ent );
void SP_ai_civilian( gentity_t *ent );
void SP_ai_eliteguard( gentity_t *ent );
void SP_ai_frogman( gentity_t *ent );
void SP_ai_stimsoldier_dual( gentity_t *ent );
void SP_ai_stimsoldier_rocket( gentity_t *ent );
void SP_ai_stimsoldier_tesla( gentity_t *ent );
void SP_ai_supersoldier( gentity_t *ent );
void SP_ai_protosoldier( gentity_t *ent );
void SP_ai_blackguard( gentity_t *ent );

#endif
