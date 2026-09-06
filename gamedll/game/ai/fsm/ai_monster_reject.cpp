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

//=================================================================================
//
// REJECT X-CREATURE
//
//	Attacks are: backhand slap, blowtorch (small flamethrower)
//
//=================================================================================

////// Backhand attack (slap)

/*
==============
AIFunc_RejectBackhand
==============
*/
char *AIFunc_RejectBackhand( cast_state_t *cs ) {
	return AIFunc_DefaultStart( cs );
}

/*
==============
AIFunc_RejectBackhandStart
==============
*/
char *AIFunc_RejectBackhandStart( cast_state_t *cs ) {
	// Reserved until the backhand attack is implemented.
	return NULL;
}
