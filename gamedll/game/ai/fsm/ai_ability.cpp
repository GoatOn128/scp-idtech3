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

#include "../ai_local.h"

const ai_ability_definition_t *AI_FindAbility( const cast_state_t *cs, int weapon ) {
	if ( !cs || cs->aiCharacter < 0 || cs->aiCharacter >= NUM_CHARACTERS ) {
		return NULL;
	}
	const ai_ability_set_t *set = aiDefaults[cs->aiCharacter].abilities;
	if ( !set ) {
		return NULL;
	}
	for ( int i = 0; i < set->count; i++ ) {
		if ( set->definitions[i].engineWeapon == weapon ) {
			return &set->definitions[i];
		}
	}
	return NULL;
}

char *AI_StartAbility( cast_state_t *cs, int weapon ) {
	const ai_ability_definition_t *ability = AI_FindAbility( cs, weapon );
	if ( !ability || !cs->clientState || !ability->available( cs, weapon ) ) {
		return NULL;
	}
	return ability->start( cs );
}
