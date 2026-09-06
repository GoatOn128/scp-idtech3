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

#include "ai_local.h"

/*
==============
AICast_SafeMissileFire

  checks to see if firing the missile will be successful, neutral, or dangerous to us or a friendly
==============
*/
int AICast_SafeMissileFire( gentity_t *ent, int duration, int enemyNum, vec3_t enemyPos, int selfNum, vec3_t endPos ) {
	int rval;
	vec3_t org;
	gentity_t *trav;

	if ( !G_PredictMissile( ent, duration, org, qtrue ) ) {
		// not point firing, since it won't explode
		return 0;
	}

	if ( endPos ) {
		VectorCopy( org, endPos );
	}

	// at end of life, so do radius damage
	rval = ( Distance( org, enemyPos ) < ent->splashRadius ) &&
		   AICast_VisibleFromPos( org, ent->s.number, enemyPos, enemyNum, qfalse );
	if ( rval ) {
		// don't hurt ourselves
		if ( Distance( org, g_entities[selfNum].r.currentOrigin ) < ent->splashRadius * 1.5 ) {
			return -1;
		}
		// make sure we don't injure a friendly
		for ( trav = g_entities; trav < g_entities + g_maxclients.integer; trav++ ) {
			if ( !trav->inuse ) {
				continue;
			}
			if ( !trav->client ) {
				continue;
			}
			if ( trav->health <= 0 ) {
				continue;
			}
			if ( trav->s.number == selfNum ) {
				continue;
			}
			if ( AICast_SameTeam( AICast_GetCastState( selfNum ), trav->s.number ) ) {
				if ( Distance( org, trav->r.currentOrigin ) < ent->splashRadius ) {
					return -1;
				}
			}
		}
	}
	// if it overshot the mark
	if ( !rval && Distance( g_entities[ent->r.ownerNum].r.currentOrigin, org ) >
					  Distance( g_entities[ent->r.ownerNum].r.currentOrigin, enemyPos ) ) {
		return -2; // so the AI can try aiming down a bit next time
	}
	//
	return rval;
}

/*
=============
AICast_CheckDangerousEntity

  check to see if the given entity can harm an AI character, if so, informs them
  appropriately
=============
*/
void AICast_CheckDangerousEntity( gentity_t *ent, int dangerFlags, float dangerDist, float tacticalLevel,
								  float aggressionLevel, qboolean hurtFriendly ) {
	vec3_t org, fwd, vec;
	cast_state_t *cs, *dcs;
	gentity_t *trav;
	int i, endTime;
	float dist;
	//
	//
	if ( dangerFlags & DANGER_MISSILE ) {
		// predict where the entity will explode
		if ( !( endTime = G_PredictMissile( ent, ent->nextthink - level.time, org, qtrue ) ) ) {
			return; // missile won't explode, so no danger
		}
	} else {
		// just avoid it for a bit, then forget it
		endTime = level.time + 1000;
		VectorCopy( ent->r.currentOrigin, org );
	}
	if ( dangerFlags & DANGER_CLIENTAIM ) {
		AngleVectors( ent->client->ps.viewangles, fwd, NULL, NULL );
	}
	//
	if ( ent->client ) {
		dcs = AICast_GetCastState( ent->s.number );
	} else {
		dcs = NULL;
	}
	//
	// see if this will hurt anyone
	for ( trav = g_entities, cs = AICast_GetCastState( 0 ), i = 0; i < level.numPlayingClients; cs++, trav++ ) {
		if ( !trav->inuse || !trav->client ) {
			continue;
		}
		i++; // found a connected client
		if ( trav == ent ) { // don't be afraid of ourself
			continue;
		}
		if ( trav->health <= 0 ) {
			continue;
		}
		if ( !cs->clientState ) { // not an AI, they should look out for themselves
			continue;
		}
		if ( cs->script.castScriptStatus.scriptNoSightTime >= level.time ) {
			continue; // absolutely no sight (or hear) information allowed
		}
		if ( !hurtFriendly && ent->s.number < MAX_CLIENTS && AICast_SameTeam( cs, ent->s.number ) ) {
			continue; // trust that friends will not hurt us
		}
		if ( ( dangerFlags & DANGER_FLAMES ) && ( cs->aiFlags & AIFL_NO_FLAME_DAMAGE ) ) {
			continue; // venom not effected by flames
		}
		if ( cs->attributes[TACTICAL] < tacticalLevel ) { // not smart enough
			continue;
		}
		if ( cs->aiState >= AISTATE_COMBAT &&
			 cs->attributes[AGGRESSION] > aggressionLevel ) { // we are too aggressive to worry about being hurt by this
			continue;
		}
		// if they are below alert mode, and the danger is not in FOV, then ignore it
		if ( cs->aiState < AISTATE_ALERT ) {
			vec3_t ang, dir;
			VectorSubtract( ent->r.currentOrigin, cs->clientState->origin, dir );
			VectorNormalize( dir );
			vectoangles( dir, ang );
			if ( !AICast_InFieldOfVision( cs->viewangles, cs->attributes[FOV], ang ) ) {
				// can't see it
				continue;
			}
		}
		if ( ent->client && ( !cs->perception.vislist[ent->s.number].visible_timestamp ||
							  ( cs->perception.vislist[ent->s.number].visible_timestamp < level.time - 3000 ) ) ) {
			//	(!dcs->perception.vislist[trav->s.number].visible_timestamp || (dcs->perception.vislist[trav->s.number].visible_timestamp < level.time - 3000))))
			continue; // not aware of them, and they're not aware of us
		}
		// are they in danger?
		if ( cs->dangerEntityValidTime < level.time + 50 ) {
			VectorSubtract( cs->clientState->cur_ps.origin, org, vec );
			dist = VectorLength( vec );
			if ( dist < dangerDist ) {
				if ( dangerFlags & DANGER_CLIENTAIM ) {
					// also check aiming
					if ( DotProduct( vec, fwd ) < ( dist * 0.95 - 100 ) ) {
						continue;
					}
				}
				//
				cs->aiFlags &= ~AIFL_DENYACTION;
				AICast_ScriptEvent( cs, "avoiddanger", ent->classname );
				if ( cs->aiFlags & AIFL_DENYACTION ) {
					continue;
				}
				cs->dangerEntity = ent->s.number;
				VectorCopy( org, cs->dangerEntityPos );
				cs->dangerEntityValidTime = endTime + 50;
				cs->dangerDist = dangerDist * 1.5; // when we hide from it, get a good distance away
				cs->dangerEntityTimestamp = level.time;
			}
		}
	}
}
