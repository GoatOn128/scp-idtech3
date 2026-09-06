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

#ifndef ICEWOLF_AI_SCRIPT_ENTITIES_H
#define ICEWOLF_AI_SCRIPT_ENTITIES_H

#include "ai_actor.h"

void SP_ai_marker( gentity_t *ent );
void ai_effect_think( gentity_t *ent );
void SP_ai_effect( gentity_t *ent );
void AICast_trigger_wait( gentity_t *ent );
void AICast_trigger_trigger( gentity_t *ent, gentity_t *activator );
void AICast_Touch_Trigger( gentity_t *self, gentity_t *other, trace_t *trace );
void ai_trigger_activate( gentity_t *self );
void ai_trigger_use( gentity_t *self, gentity_t *other, gentity_t *activator );
void SP_ai_trigger( gentity_t *ent );

#endif
