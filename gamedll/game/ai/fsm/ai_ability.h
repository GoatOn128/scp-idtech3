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

#ifndef ICEWOLF_AI_ABILITY_H
#define ICEWOLF_AI_ABILITY_H

#include "../ai_engine_types.h"

struct cast_state_s;
typedef char *( *ai_state_callback_t )( struct cast_state_s *cs );

// Engine weapon slots are an adapter for existing animations and network effects.
// Gameplay code identifies an ability by name and owns its rules in the creature module.
typedef struct {
	const char *name;
	int engineWeapon;
	ai_state_callback_t start;
	float ( *range )( struct cast_state_s *cs, int weapon );
	qboolean ( *available )( struct cast_state_s *cs, int weapon );
} ai_ability_definition_t;

typedef struct {
	const ai_ability_definition_t *definitions;
	int count;
} ai_ability_set_t;

const ai_ability_definition_t *AI_FindAbility( const struct cast_state_s *cs, int weapon );
char *AI_StartAbility( struct cast_state_s *cs, int weapon );

#endif
