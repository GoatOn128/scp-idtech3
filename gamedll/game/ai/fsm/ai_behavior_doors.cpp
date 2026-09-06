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
AIFunc_DoorMarker()
============
*/
char *AIFunc_DoorMarker( cast_state_t *cs ) {
	bot_moveresult_t *moveresult;
	gentity_t *followent, *door;
	ai_client_state_t *bs;
	vec3_t destorg;
	float dist;
	//
	// do we need to avoid a danger?
	if ( cs->dangerEntityValidTime >= level.time ) {
		if ( !AICast_GetTakeCoverPos( cs, cs->dangerEntity, cs->dangerEntityPos, cs->combat.takeCoverPos ) ) {
			// shit??
		}
		// go to a position that cannot be seen from the dangerPos
		cs->combat.takeCoverTime = cs->dangerEntityValidTime + 1000;
		cs->attackcrouch_time = 0;
		return AIFunc_AvoidDangerStart( cs );
	}

	followent = &g_entities[cs->navigation.doorMarker];

	// if the entity is not ready yet
	if ( !followent->inuse ) {
		cs->navigation.doorMarkerTime = 0;
		//return AIFunc_DefaultStart( cs );
		return AIFunc_Restore( cs );
	}

	// if the door is open or idle
	door = &g_entities[cs->navigation.doorEntNum];
	if ( ( !door->key ) && ( door->s.apos.trType == TR_STATIONARY && door->s.pos.trType == TR_STATIONARY ) ) {
		cs->navigation.doorMarkerTime = 0;
		//return AIFunc_DefaultStart( cs );
		return AIFunc_Restore( cs );
	}

	// if we have an enemy, fire if they're visible
	if ( cs->enemyNum >= 0 ) { //attack the enemy if possible
		AICast_ProcessAttack( cs );
	}

	// they are ready, are they inside range? FIXME: make configurable
	dist = Distance( destorg, cs->clientState->origin );
	if ( dist < 12 ) {
		// check for a movement we should be making
		if ( cs->navigation.obstructingTime > level.time ) {
			AICast_MoveToPos( cs, cs->navigation.obstructingPos, -1 );
		}
		// if the door is locked, resume
		if ( followent->key ) {
			return AIFunc_Restore( cs );
		}
		return NULL;
	}

	// go to it
	//
	bs = cs->clientState;
	//
	moveresult = AICast_MoveToPos( cs, followent->r.currentOrigin, followent->s.number );
	// if we cant get there, forget it
	if ( moveresult && moveresult->failure ) {
		return AIFunc_Restore( cs );
	}
	// should we slow down?
	if ( cs->navigation.followDist ) {
		cs->speedScale = AICast_SpeedScaleForDistance( cs, dist, cs->navigation.followDist );
	}
	// reload?
	AICast_IdleReload( cs );
	return NULL;
}

/*
============
AIFunc_DoorMarkerStart()
============
*/
char *AIFunc_DoorMarkerStart( cast_state_t *cs, int doornum, int markernum ) {
	cs->navigation.doorEntNum = doornum;
	cs->navigation.doorMarker = markernum;
	cs->previousState = cs->currentState;
	AI_SetLegacyState( cs, AIFunc_DoorMarker );
	return "AIFunc_DoorMarker";
}
