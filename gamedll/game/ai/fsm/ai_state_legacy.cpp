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

/*
============
AIFunc_Restore()

  restores the last aifunc that was backed up
============
*/
char *AIFunc_Restore( cast_state_t *cs ) {
	const ai_state_callback_t previous = cs->previousState;
	if ( !previous ) {
		return AIFunc_DefaultStart( cs );
	}
	if ( previous == AIFunc_BattleChase || previous == AIFunc_Battle ) {
		return AIFunc_BattleStart( cs );
	}
	if ( previous == AIFunc_Idle ) {
		return AIFunc_IdleStart( cs );
	}
	// Restore execution on the next think, never recursively update another state.
	AI_SetLegacyState( cs, previous );
	return "Restore";
}

/*
============
AIFunc_DefaultStart()
============
*/
char *AIFunc_DefaultStart( cast_state_t *cs ) {
	qboolean first = qfalse;
	char *rval = NULL;
	//
	if ( cs->aiFlags & AIFL_JUST_SPAWNED ) {
		first = qtrue;
		cs->aiFlags &= ~AIFL_JUST_SPAWNED;
	}
	//
	switch ( cs->aiCharacter ) {
	case AICHAR_ZOMBIE:
		// portal zombie, requires spawning effect
		if ( first && ( g_entities[cs->entityNum].spawnflags & 4 ) ) {
			return AIFunc_FlameZombie_PortalStart( cs );
		}
		break;
	}
	//
	// if they have an enemy, then pursue
	if ( cs->enemyNum >= 0 && ( cs->currentState != AIFunc_Battle ) ) { // make sure we haven't just come from there
		rval = AIFunc_BattleStart( cs );
	}
	//
	if ( !rval ) {
		return AIFunc_IdleStart( cs );
	}
	//
	return rval;
}
