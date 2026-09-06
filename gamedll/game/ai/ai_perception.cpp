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
==================
AICast_EntityVisible
==================
*/
qboolean AICast_EntityVisible( cast_state_t *cs, int enemynum, qboolean directview ) {
	cast_visibility_t *vis;
	int last_visible;
	int reactionTime;
	float dist;

	if ( enemynum >= MAX_CLIENTS ) {
		return qtrue; // FIXME: do a visibility calculation on non-client entities?
	}
	vis = &cs->perception.vislist[enemynum];

	if ( !vis->visible_timestamp && !vis->real_visible_timestamp ) {
		return qfalse; // they are not visible at all
	}
	if ( directview ) {
		last_visible = vis->real_visible_timestamp;
	} else {
		last_visible = vis->visible_timestamp;
	}

	reactionTime = (int)( 1000 * cs->attributes[REACTION_TIME] );
	if ( cs->combat.startAttackCount > 1 ) {
		// we recently saw them, so we are more "aware" of their presence
		reactionTime /= 2;
	}

	// if they are close, we should react faster
	if ( cs->clientState && enemynum == cs->enemyNum ) {
		dist = cs->combat.enemyDist;
	} else {
		dist =
			VectorDistance( g_entities[cs->entityNum].client->ps.origin, cs->perception.vislist[enemynum].visible_pos );
	}
	if ( dist < 384 ) {
		reactionTime *= 0.5 + 0.5 * ( dist / 384 );
	}

	if ( vis->notvisible_timestamp < ( level.time - reactionTime ) ) {
		// make sure we've seen them since we've last not seen them (since the visibility checking is spread amongst server frames)
		if ( vis->notvisible_timestamp < last_visible ) {
			return qtrue;
		}
	}

	// we can't directly see them, but if they've just left our sight, pretend we can see them for another second or so

	if ( !directview && last_visible ) {
		if ( vis->notvisible_timestamp > last_visible ) {
			if ( vis->notvisible_timestamp < ( last_visible + 5000 ) ) {
				return qtrue;
			}
		}
	}

	return qfalse;
}

/*
===============
AICast_GetWeaponSoundRange
===============
*/
float AICast_GetWeaponSoundRange( int weapon ) {
	// NOTE: made this a case, that way changing the ordering of weapons won't cause problems, as it would
	// with an array lookup

	switch ( weapon ) {
	case WP_NONE:
		return 0;
	case WP_KNIFE:
	case WP_GAUNTLET:
	case WP_STEN:
	case WP_SILENCER:
		return 64;
	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
		return 1500;
	case WP_GARAND:
	case WP_SNOOPERSCOPE:
		return 128;
	case WP_LUGER:
	case WP_COLT:
	case WP_AKIMBO:
		return 700;

	case WP_MONSTER_ATTACK1:
	case WP_MONSTER_ATTACK2:
	case WP_MONSTER_ATTACK3:
		// TODO: case for each monster
		return 1000;

	case WP_MP40:
	case WP_THOMPSON:
		return 1000;

	case WP_FG42:
	case WP_FG42SCOPE:
		return 1500;

	case WP_SNIPERRIFLE:
	case WP_MAUSER:
		return 2000;

	case WP_DYNAMITE:
		return 3000;

	case WP_PANZERFAUST:
	case WP_VENOM:
	case WP_FLAMETHROWER:
	case WP_TESLA:
		return 1000;
	}

	G_Error( "AICast_GetWeaponSoundRange: unknown weapon index: %i\n", weapon );
	return 0; // shutup the compiler
}

/*
=============
AICast_ProcessBullet
=============
*/
void AICast_ProcessBullet( gentity_t *attacker, vec3_t start, vec3_t end ) {
	gentity_t *tent;
	int i;
	float dist;
	vec3_t vProj, vDir, dir;
	cast_state_t *cs;

	VectorSubtract( end, start, dir );

	// RF, AI should hear this pass by them really closely, or hitting a wall close by
	for ( cs = caststates, tent = g_entities, i = 0; i < level.maxclients; i++, tent++, cs++ ) {
		if ( !tent->inuse ) {
			continue;
		}
		if ( tent == attacker ) {
			continue;
		}
		if ( tent->aiInactive ) {
			continue;
		}
		if ( tent->health <= 0 ) {
			continue;
		}
		if ( cs->script.castScriptStatus.scriptNoSightTime > level.time ) {
			continue;
		}
		if ( !( tent->r.svFlags & SVF_CASTAI ) ) {
			continue;
		}
		if ( cs->aiState >= AISTATE_COMBAT ) { // RF add	// already fighting, not interested in bullet impacts
			continue;
		}
		if ( cs->perception.bulletImpactIgnoreTime > level.time ) {
			continue;
		}
		dist = Distance( tent->client->ps.origin, end );
		if ( dist <= cs->attributes[INNER_DETECTION_RADIUS] ) {
			// close enough to hear/see the impact?
			// first check pvs
			if ( !trap_InPVS( tent->client->ps.origin, end ) ) {
				continue;
			}
			// heard it
			goto heard;
		}
		// are they within radius of the bullet path to hear it travel through the air?
		ProjectPointOntoVector( tent->client->ps.origin, start, end, vProj );
		VectorSubtract( vProj, start, vDir );
		if ( DotProduct( vDir, dir ) < 0 ) { // they are behind the path of the bullet
			continue;
		}
		if ( Distance( vProj, tent->client->ps.origin ) > 0.5 * cs->attributes[INNER_DETECTION_RADIUS] ) {
			continue;
		}
	heard:
		// call the script event
		AICast_ScriptEvent( cs, "bulletimpact", "" );
		if ( cs->aiFlags & AIFL_DENYACTION ) {
			continue; // ignore the bullet
		}
		//
		cs->perception.bulletImpactTime = level.time + 100 + rand() % 200; // random reaction delay;
		VectorCopy( start, cs->perception.bulletImpactStart );
		VectorCopy( end, cs->perception.bulletImpactEnd );
		cs->perception.bulletImpactEntity = attacker->s.number;
	}
}

/*
================
AICast_AudibleEvent
================
*/
void AICast_AudibleEvent( int srcnum, vec3_t pos, float range ) {
	int i;
	cast_state_t *cs, *scs = 0;
	gentity_t *ent, *sent;
	float adjustedRange, localDist;

	// DHM - Nerve :: caststates are not initialized in multiplayer
	if ( g_gametype.integer != GT_SINGLE_PLAYER ) {
		return;
	}
	// dhm

	if ( g_debugAudibleEvents.integer ) {
		G_Printf( "AICast_AudibleEvent: (%0.1f %0.1f %0.1f) range: %0.0f\n", pos[0], pos[1], pos[2], range );
	}

	sent = &g_entities[srcnum];
	if ( sent->flags & FL_NOTARGET ) {
		if ( g_debugAudibleEvents.integer ) {
			G_Printf( "NOTARGET enabled, aborting\n" );
		}
		return;
	}
	if ( srcnum < level.maxclients ) {
		scs = AICast_GetCastState( srcnum );
	}

	for ( ent = g_entities, cs = caststates, i = 0; i < level.maxclients; i++, cs++, ent++ ) {
		if ( !cs->clientState ) {
			continue;
		}
		if ( ent == sent ) {
			continue;
		}
		if ( cs->script.castScriptStatus.scriptNoSightTime > level.time ) {
			continue;
		}
		if ( ent->health <= 0 ) {
			continue;
		}
		// if within range, and this sound was made by an enemy
		if ( scs ) {
			if ( ( srcnum < level.maxclients ) && scs->aiState < AISTATE_COMBAT && !AICast_QueryEnemy( cs, srcnum ) ) {
				continue;
			}
		}
		// calculate the adjusted range according to this AI's hearing abilities
		adjustedRange = range * cs->attributes[HEARING_SCALE];
		localDist = DistanceSquared( pos, ent->s.pos.trBase );
		if ( localDist > adjustedRange * adjustedRange ) { // fast out if already outside range
			continue;
		}
		if ( !trap_InPVS( pos, ent->s.pos.trBase ) ) {
			adjustedRange *= cs->attributes[HEARING_SCALE_NOT_PVS];
		}
		if ( localDist > adjustedRange * adjustedRange ) {
			continue;
		}
		// we heard it

		if ( g_debugAudibleEvents.integer ) {
			G_Printf( "AICast_AudibleEvent heard: %s \"%s\" (dist:%0.0f s:%0.2f pvss:%0.2f)\n", ent->classname,
					  ent->aiName, ( sqrt( localDist ) ), cs->attributes[HEARING_SCALE],
					  cs->attributes[HEARING_SCALE_NOT_PVS] );
		}

		cs->perception.audibleEventTime = level.time + 200 + rand() % 300; // random reaction delay
		VectorCopy( pos, cs->perception.audibleEventOrg );
		cs->perception.audibleEventEnt = ent->s.number;
	}
}

/*
Does sight checking for Cast AI's.
*/

static float aiStateFovScales[] = {
	1.0, // relaxed
	1.5, // query
	1.5, // alert
	2.0, // combat
};

orientation_t clientHeadTags[MAX_CLIENTS];
int clientHeadTagTimes[MAX_CLIENTS];

/*
==============
AICast_InFieldOfVision
==============
*/
qboolean AICast_InFieldOfVision( vec3_t viewangles, float fov, vec3_t angles ) {
	int i;
	float diff, angle;

	for ( i = 0; i < 2; i++ ) {
		angle = AngleMod( viewangles[i] );
		angles[i] = AngleMod( angles[i] );
		diff = angles[i] - angle;
		if ( angles[i] > angle ) {
			if ( diff > 180.0 ) {
				diff -= 360.0;
			}
		} else {
			if ( diff < -180.0 ) {
				diff += 360.0;
			}
		}
		if ( diff > 0 ) {
			if ( diff > fov * 0.5 ) {
				return qfalse;
			}
		} else {
			if ( diff < -fov * 0.5 ) {
				return qfalse;
			}
		}
	}
	return qtrue;
}

/*
==============
AICast_VisibleFromPos
==============
*/
qboolean AICast_VisibleFromPos( vec3_t srcpos, int srcnum, vec3_t destpos, int destnum, qboolean updateVisPos ) {
	int i, contents_mask, passent, hitent;
	trace_t trace;
	vec3_t start, end, middle, eye;
	cast_state_t *cs = NULL;
	int srcviewheight;
	vec3_t destmins, destmaxs;
	vec3_t right, vec;
	qboolean inPVS;

	if ( g_entities[destnum].flags & FL_NOTARGET ) {
		return qfalse;
	}

	if ( srcnum < aicast_maxclients ) {
		cs = AICast_GetCastState( srcnum );
	}
	//
	if ( cs && cs->clientState ) {
		srcviewheight = cs->clientState->cur_ps.viewheight;
	} else if ( g_entities[srcnum].client ) {
		srcviewheight = g_entities[srcnum].client->ps.viewheight;
	} else {
		srcviewheight = 0;
	}
	//
	VectorCopy( g_entities[destnum].r.mins, destmins );
	VectorCopy( g_entities[destnum].r.maxs, destmaxs );
	//
	//calculate middle of bounding box
	VectorAdd( destmins, destmaxs, middle );
	VectorScale( middle, 0.5, middle );
	VectorAdd( destpos, middle, middle );
	// calculate eye position
	VectorCopy( srcpos, eye );
	eye[2] += srcviewheight;
	//
	// set the right vector
	VectorSubtract( middle, eye, vec );
	VectorNormalize( vec );
	right[0] = vec[1];
	right[1] = vec[0];
	right[2] = 0;
	//
	inPVS = qfalse;
	//
	for ( i = 0; i < 5; i++ ) {
		if ( cs && updateVisPos ) { // if it's a grenade or something, PVS checks don't work very well
			//if the point is not in potential visible sight
			if ( i < 3 ) { // don't do PVS check for left/right checks
				if ( !trap_InPVS( eye, middle ) ) {
					continue;
				} else {
					inPVS = qtrue;
				}
			} else if ( !inPVS ) {
				break; // wasn't in potential view in either of the previous tests
			} // so don't bother doing left/right
		}
		//
		contents_mask =
			MASK_AISIGHT; //(MASK_SHOT | CONTENTS_AI_NOSIGHT) & ~(CONTENTS_BODY);	// we can see anything that a bullet can pass through
		passent = srcnum;
		hitent = destnum;
		VectorCopy( eye, start );
		VectorCopy( middle, end );
		//if the entity is in water, lava or slime
		if ( trap_PointContents( middle, destnum ) & ( CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER ) ) {
			contents_mask |= ( CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER );
		} //end if
		//if eye is in water, lava or slime
		if ( trap_PointContents( eye, srcnum ) & ( CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER ) ) {
			if ( !( contents_mask & ( CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER ) ) ) {
				passent = destnum;
				hitent = srcnum;
				VectorCopy( middle, start );
				VectorCopy( eye, end );
			} //end if
			contents_mask ^= ( CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER );
		} //end if
		//trace from start to end
		trap_Trace( &trace, start, NULL, NULL, end, ENTITYNUM_NONE /*passent*/, contents_mask );
		//if water was hit
		if ( trace.contents & ( CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER ) ) {
			//if the water surface is translucent
			//			if (trace.surface.flags & (SURF_TRANS33|SURF_TRANS66))
			{
				//trace through the water
				contents_mask &= ~( CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER );
				trap_Trace( &trace, trace.endpos, NULL, NULL, end, passent, contents_mask );
			} //end if
		} //end if
		//if a full trace or the hitent was hit
		if ( trace.fraction >= 1 || trace.entityNum == hitent ) {
			return qtrue;
		}
		//check bottom and top of bounding box as well
		if ( i == 0 ) {
			middle[2] -= ( destmaxs[2] - destmins[2] ) * 0.5;
		} else if ( i == 1 ) {
			middle[2] += destmaxs[2] - destmins[2];
		} else if ( i == 2 ) { // right side
			middle[2] -= ( destmaxs[2] - destmins[2] ) / 2.0;
			VectorMA( eye, destmaxs[0] - 0.5, right, eye );
		} else if ( i == 3 ) { // left side
			VectorMA( eye, -2.0 * ( destmaxs[0] - 0.5 ), right, eye );
		}
	} //end for

	return qfalse;
}

/*
==============
AICast_CheckVisibility
==============
*/
qboolean AICast_CheckVisibility( gentity_t *srcent, gentity_t *destent ) {
	vec3_t dir, entangles, middle, eye, viewangles;
	cast_state_t *cs, *ocs;
	float fov, dist;
	int viewer, ent;
	cast_visibility_t *vis;
	orientation_t or ;

	if ( destent->flags & FL_NOTARGET ) {
		return qfalse;
	}
	//
	viewer = srcent->s.number;
	ent = destent->s.number;
	//
	cs = AICast_GetCastState( viewer );
	ocs = AICast_GetCastState( ent );
	//
	vis = &cs->perception.vislist[ent];
	//
	// if the destent is the client, and they have just loaded a savegame, ignore them temporarily
	if ( !destent->aiCharacter && level.lastLoadTime && ( level.lastLoadTime > level.time - 2000 ) &&
		 !vis->visible_timestamp ) {
		return qfalse;
	}
	// if we heard them
	/*
	if (	(vis->lastcheck_timestamp) &&
			(ocs->combat.lastWeaponFired) &&
			(ocs->combat.lastWeaponFired >= vis->lastcheck_timestamp) &&
			(AICast_GetWeaponSoundRange( ocs->combat.lastWeaponFiredWeaponNum ) > Distance( srcent->r.currentOrigin, ocs->combat.lastWeaponFiredPos ))) {
		return qtrue;
	}
	*/
	//
	// set the FOV
	fov = cs->attributes[FOV] * aiStateFovScales[cs->aiState];
	if ( !fov ) { // assume it's a player, give them a generic fov
		fov = 180;
	}
	if ( cs->aiFlags & AIFL_ZOOMING ) {
		fov *= 0.8;
	} else {
		if ( cs->lastEnemy >= 0 ) { // they've already been in a fight, so give them a very large fov
			if ( fov < 270 ) {
				fov = 270;
			}
		}
	}
	// RF, if they were visible last check, then give us a full FOV, since we are aware of them
	if ( cs->aiState >= AISTATE_ALERT && vis->visible_timestamp == vis->lastcheck_timestamp ) {
		fov = 360;
	}
	//calculate middle of bounding box
	VectorAdd( destent->r.mins, destent->r.maxs, middle );
	VectorScale( middle, 0.5, middle );
	VectorAdd( destent->client->ps.origin, middle, middle );
	// calculate eye position
	if ( ( level.lastLoadTime < level.time - 4000 ) && ( srcent->r.svFlags & SVF_CASTAI ) ) {
		if ( clientHeadTagTimes[srcent->s.number] == level.time ) {
			// use the actual direction the head is facing
			vectoangles( clientHeadTags[srcent->s.number].axis[0], viewangles );
			// and the actual position of the head
			VectorCopy( clientHeadTags[srcent->s.number].origin, eye );
		} else if ( trap_GetTag( srcent->s.number, "tag_head", &or ) ) {
			// use the actual direction the head is facing
			vectoangles( or.axis[0], viewangles );
			// and the actual position of the head
			VectorCopy( or.origin, eye );
			VectorMA( eye, 12, or.axis[2], eye );
			// save orientation data
			memcpy( &clientHeadTags[srcent->s.number], &or, sizeof( orientation_t ) );
			clientHeadTagTimes[srcent->s.number] = level.time;
		} else {
			VectorCopy( srcent->client->ps.origin, eye );
			eye[2] += srcent->client->ps.viewheight;
			VectorCopy( srcent->client->ps.viewangles, viewangles );
			// save orientation data (so we dont keep checking for a tag when it doesn't exist)
			VectorCopy( eye, clientHeadTags[srcent->s.number].origin );
			AnglesToAxis( viewangles, clientHeadTags[srcent->s.number].axis );
			clientHeadTagTimes[srcent->s.number] = level.time;
		}
	} else {
		VectorCopy( srcent->client->ps.origin, eye );
		eye[2] += srcent->client->ps.viewheight;
		VectorCopy( srcent->client->ps.viewangles, viewangles );
	}
	//check if entity is within field of vision
	VectorSubtract( middle, eye, dir );
	vectoangles( dir, entangles );
	//
	dist = VectorLength( dir );
	//
	// alertness is visible range
	if ( cs->clientState && dist > cs->attributes[ALERTNESS] ) {
		return qfalse;
	}
	// check FOV
	if ( !AICast_InFieldOfVision( viewangles, fov, entangles ) ) {
		return qfalse;
	}
	//
	if ( !AICast_VisibleFromPos( srcent->client->ps.origin, srcent->s.number, destent->client->ps.origin,
								 destent->s.number, qtrue ) ) {
		return qfalse;
	}
	//
	return qtrue;
}

/*
==============
AICast_UpdateVisibility
==============
*/
void AICast_UpdateVisibility( gentity_t *srcent, gentity_t *destent, qboolean shareVis, qboolean directview ) {
	cast_visibility_t *vis, *ovis, *svis, oldvis;
	cast_state_t *cs, *ocs;
	qboolean shareRange;
	int cnt, i;

	if ( destent->flags & FL_NOTARGET ) {
		return;
	}

	cs = AICast_GetCastState( srcent->s.number );
	ocs = AICast_GetCastState( destent->s.number );

	if ( cs->script.castScriptStatus.scriptNoSightTime >= level.time ) {
		return; // absolutely no sight (or hear) information allowed
	}
	shareRange = ( VectorDistance( srcent->client->ps.origin, destent->client->ps.origin ) < AIVIS_SHARE_RANGE );

	vis = &cs->perception.vislist[destent->s.number];

	vis->chase_marker_count = 0;

	if ( aicast_debug.integer == 1 ) {
		if ( !vis->visible_timestamp || vis->visible_timestamp < level.time - 5000 ) {
			if ( directview ) {
				G_Printf( "SIGHT (direct): %s sees %s\n", srcent->aiName, destent->aiName );
			} else {
				G_Printf( "SIGHT (non-direct/audible): %s sees %s\n", srcent->aiName, destent->aiName );
			}
		}
	}

	// trigger the sight event
	AICast_Sight( srcent, destent, vis->visible_timestamp );

	// update the values
	vis->lastcheck_timestamp = level.time;
	vis->visible_timestamp = level.time;
	VectorCopy( destent->client->ps.origin, vis->visible_pos );
	VectorCopy( destent->client->ps.velocity, vis->visible_vel );
	vis->lastcheck_health = destent->health - 1;

	// we may need to process this visibility at some point, even after they become not visible again
	vis->flags |= AIVIS_PROCESS_SIGHTING;

	if ( directview ) {
		vis->real_visible_timestamp = level.time;
		VectorCopy( destent->client->ps.origin, vis->real_visible_pos );
		vis->real_update_timestamp = level.time;
	}

	// if we are on fire, then run away from anything we see
	if ( cs->attributes[AGGRESSION] < 1.0 && srcent->s.onFireEnd > level.time &&
		 ( !destent->s.number || cs->dangerEntityValidTime < level.time + 2000 ) &&
		 !( cs->aiFlags & AIFL_NO_FLAME_DAMAGE ) ) {
		cs->dangerEntity = destent->s.number;
		VectorCopy( destent->r.currentOrigin, cs->dangerEntityPos );
		cs->dangerEntityValidTime = level.time + 5000;
		cs->dangerDist = 99999;
		cs->dangerEntityTimestamp = level.time;
	}

	// Look for reasons to make this character an enemy of ours

	// if they are an enemy and inside the detection radius, go hostile
	if ( !( vis->flags & AIVIS_ENEMY ) && !AICast_SameTeam( cs, destent->s.number ) ) {
		float idr;

		idr = cs->attributes[INNER_DETECTION_RADIUS];
		if ( cs->aiFlags & AIFL_ZOOMING ) {
			idr *= 10;
		}
		if ( !( vis->flags & AIVIS_ENEMY ) &&
			 VectorDistance( vis->visible_pos, g_entities[cs->entityNum].r.currentOrigin ) < idr ) {
			// RF, moved them over to AICast_ScanForEnemies()
			//AICast_ScriptEvent( cs, "enemysight", destent->aiName );
			vis->flags |= AIVIS_ENEMY;
		}
		// if we are in (or above) ALERT mode, then we now know this is an enemy
		else if ( cs->aiState >= AISTATE_ALERT ) {
			// RF, moved them over to AICast_ScanForEnemies()
			//AICast_ScriptEvent( cs, "enemysight", destent->aiName );
			vis->flags |= AIVIS_ENEMY;
		}
	}

	// if they are friendly, then we should help them out if they are in trouble
	if ( AICast_SameTeam( cs, destent->s.number ) &&
		 ( srcent->aiTeam == AITEAM_ALLIES || srcent->aiTeam == AITEAM_NAZI ) ) {
		// if they are dead, we should check them out
		if ( destent->health <= 0 ) {
			// if we haven't already checked them out
			if ( !( vis->flags & AIVIS_INSPECTED ) ) {
				vis->flags |= AIVIS_INSPECT;
			}
			// if they are mad, we should help, or at least act concerned
		} else if ( cs->aiState < AISTATE_COMBAT && ocs->aiState >= AISTATE_COMBAT && ocs->clientState &&
					( ocs->enemyNum >= 0 ) ) {
			// if we haven't already checked them out
			if ( !( vis->flags & AIVIS_INSPECTED ) ) {
				vis->flags |= AIVIS_INSPECT;
			}
			// if they are alert, we should also go alert
		} else if ( cs->aiState < AISTATE_ALERT && ocs->aiState == AISTATE_ALERT && ocs->clientState ) {
			AICast_StateChange( cs, AISTATE_ALERT );
		}
	}

	// if this is a friendly, then check them for hostile's that we currently haven't upgraded so

	if ( ( destent->health > 0 ) &&
		 ( srcent->aiTeam == destent->aiTeam ) && // only share with exact same team, and non-neutrals
		 ( srcent->aiTeam != AITEAM_NEUTRAL ) ) {
		ocs = AICast_GetCastState( destent->s.number );
		cnt = 0;
		//
		for ( i = 0; i < aicast_maxclients && cnt < level.numPlayingClients; i++ ) {
			if ( !g_entities[i].inuse ) {
				continue;
			}
			//
			cnt++;
			//
			if ( i == srcent->s.number ) {
				continue;
			}
			if ( i == destent->s.number ) {
				continue;
			}
			//
			ovis = &ocs->perception.vislist[i];
			svis = &cs->perception.vislist[i];
			//
			// if we are close to the friendly, then we should share their visibility info
			if ( destent->health > 0 && shareRange ) {
				oldvis = *svis;
				// if they have seen this character more recently than us, share
				if ( ( ovis->visible_timestamp > svis->visible_timestamp ) ||
					 ( ( ovis->visible_timestamp > level.time - 5000 ) && ( ovis->flags & AIVIS_ENEMY ) &&
					   !( svis->flags & AIVIS_ENEMY ) ) ) {
					// trigger an EVENT

					// trigger the sight event
					AICast_Sight( srcent, destent, ovis->visible_timestamp );

					// we may need to process this visibility at some point, even after they become not visible again
					svis->flags |= AIVIS_PROCESS_SIGHTING;

					// if we are sharing information about an enemy, then trigger a scripted event
					if ( !svis->real_visible_timestamp && ovis->real_visible_timestamp &&
						 ( ovis->flags & AIVIS_ENEMY ) ) {
						// setup conditions
						BG_UpdateConditionValue( ocs->entityNum, ANIM_COND_ENEMY_TEAM, g_entities[i].aiTeam, qfalse );
						// call the event
						BG_AnimScriptEvent( &g_entities[ocs->entityNum].client->ps, ANIM_ET_INFORM_FRIENDLY_OF_ENEMY,
											qfalse, qfalse );
					}
					// copy the whole structure
					*svis = *ovis;
					// minus the flags
					svis->flags = oldvis.flags;
					// keep our sight time if it's sooner
					if ( oldvis.visible_timestamp > ovis->visible_timestamp ) {
						svis->visible_timestamp = oldvis.visible_timestamp;
					}
					// check to see if we just made this character an enemy of ours
					if ( /*(cs->aiState == AISTATE_COMBAT) &&*/ ( ovis->flags & AIVIS_ENEMY ) &&
						 !( oldvis.flags & AIVIS_ENEMY ) ) {
						svis->flags |= AIVIS_ENEMY;
						if ( !( cs->perception.vislist[i].flags & AIVIS_SIGHT_SCRIPT_CALLED ) ) {
							AICast_ScriptEvent( cs, "enemysight", g_entities[i].aiName );
							cs->perception.vislist[i].flags |= AIVIS_SIGHT_SCRIPT_CALLED;
							if ( !( cs->aiFlags & AIFL_DENYACTION ) ) {
								G_AddEvent(
									srcent, EV_GENERAL_SOUND,
									G_SoundIndex( aiDefaults[cs->aiCharacter].soundScripts[SIGHTSOUNDSCRIPT] ) );
							}
						}
					}
				}
			} else {
				// if either of us haven't seen this character yet, then ignore it
				if ( !svis->visible_timestamp || !ovis->visible_timestamp ) {
					continue;
				}
			}
			//
			// if they have marked this character as hostile, then we should also
			if ( ( cs->aiState == AISTATE_COMBAT ) && AICast_HostileEnemy( ocs, i ) && !AICast_HostileEnemy( cs, i ) ) {
				if ( !( cs->perception.vislist[i].flags & AIVIS_SIGHT_SCRIPT_CALLED ) ) {
					AICast_ScriptEvent( cs, "enemysight", g_entities[i].aiName );
					cs->perception.vislist[i].flags |= AIVIS_SIGHT_SCRIPT_CALLED;
					if ( !( cs->aiFlags & AIFL_DENYACTION ) ) {
						G_AddEvent( srcent, EV_GENERAL_SOUND,
									G_SoundIndex( aiDefaults[cs->aiCharacter].soundScripts[SIGHTSOUNDSCRIPT] ) );
					}
				}
				svis->flags |= AIVIS_ENEMY;
			}
		}
	}
}

/*
==============
AICast_UpdateNonVisibility
==============
*/
void AICast_UpdateNonVisibility( gentity_t *srcent, gentity_t *destent, qboolean directview ) {
	cast_visibility_t *vis;
	cast_state_t *cs;

	cs = AICast_GetCastState( srcent->s.number );

	vis = &cs->perception.vislist[destent->s.number];

	// update the values
	vis->lastcheck_timestamp = level.time;
	vis->notvisible_timestamp = level.time;

	if ( directview ) {
		vis->real_update_timestamp = level.time;
		vis->real_notvisible_timestamp = level.time;
	}

	// if enough time has passed, and still within chase period, drop a marker
	if ( vis->chase_marker_count < MAX_CHASE_MARKERS ) {
		if ( ( level.time - vis->visible_timestamp ) > ( vis->chase_marker_count + 1 ) * CHASE_MARKER_INTERVAL ) {
			VectorCopy( destent->client->ps.origin, vis->chase_marker[vis->chase_marker_count] );
			vis->chase_marker_count++;
		}
	}
}

/*
==============
AICast_SightUpdate
==============
*/
static int lastsrc = 0, lastdest = 0;

void AICast_SightUpdate( int numchecks ) {
	int count = 0, destcount, srccount;
	int src, dest;
	gentity_t *srcent, *destent;
	cast_state_t *cs, *dcs;
	//static int	lastNumUpdated; // TTimo: unused
	cast_visibility_t *vis;
#define SIGHT_MIN_DELAY 200

	src = 0;
	dest = 0;
	if ( numchecks < 5 ) {
		numchecks = 5;
	}

	if ( trap_Cvar_VariableIntegerValue( "savegame_loading" ) ) {
		return;
	}

	if ( saveGamePending ) {
		return;
	}

	// First, check all REAL clients, so sighting player is only effected by reaction_time, not
	// effected by framerate also
	for ( srccount = 0, src = 0, srcent = &g_entities[0]; src < aicast_maxclients && srccount < level.numPlayingClients;
		  src++, srcent++ ) {
		if ( !srcent->inuse ) {
			continue;
		}

		srccount++;

		if ( srcent->aiInactive ) {
			continue;
		}
		if ( srcent->health <= 0 ) {
			continue;
		}
		if ( !( srcent->r.svFlags & SVF_CASTAI ) ) { // only source check AI Cast
			continue;
		}

		cs = AICast_GetCastState( src );

		if ( cs->script.castScriptStatus.scriptNoSightTime >= level.time ) {
			continue;
		}

		// make sure we are using the right AAS data for this entity (one's that don't get set will default to the player's AAS data)
		trap_AAS_SetCurrentWorld( cs->aasWorldIndex );

		for ( destcount = 0, dest = 0, destent = g_entities;
			  //dest < aicast_maxclients && destcount < level.numPlayingClients;
			  destent == g_entities; // only check the player
			  dest++, destent++ ) {
			if ( !destent->inuse ) {
				continue;
			}

			destcount++;

			if ( destent->health <= 0 ) {
				continue;
			}
			if ( destent->r.svFlags & SVF_CASTAI ) { // only dest check REAL clients
				continue;
			}

			if ( src == dest ) {
				continue;
			}

			vis = &cs->perception.vislist[destent->s.number];

			// OPTIMIZATION: if we have seen the player, abort checking each frame
			//if (vis->real_visible_timestamp && cs->aiState > AISTATE_QUERY && AICast_HostileEnemy(cs, destent->s.number))
			//	continue;

			// if we saw them last frame, skip this test, so we only check initial sightings each frame
			if ( vis->lastcheck_timestamp == vis->real_visible_timestamp ) {
				continue;
			}

			// if we recently checked this vis, skip
			if ( vis->lastcheck_timestamp >= level.time - ( 40 + rand() % 40 ) ) {
				continue;
			}

			// check for visibility
			if ( !( destent->flags & FL_NOTARGET ) && ( AICast_CheckVisibility( srcent, destent ) ) ) {
				// record the sighting
				AICast_UpdateVisibility( srcent, destent, qtrue, qtrue );
			} else // if (vis->lastcheck_timestamp == vis->real_update_timestamp)
			{
				AICast_UpdateNonVisibility( srcent, destent, qtrue );
			}
		}
	}

	// Now do the normal timeslice checks
	for ( srccount = 0, src = lastsrc, srcent = &g_entities[lastsrc];
		  src < aicast_maxclients; // && srccount < level.numPlayingClients;
		  src++, srcent++ ) {
		if ( !srcent->inuse ) {
			continue;
		}

		srccount++;

		if ( srcent->aiInactive ) {
			continue;
		}
		if ( srcent->health <= 0 ) {
			continue;
		}

		cs = AICast_GetCastState( src );

		if ( cs->script.castScriptStatus.scriptNoSightTime >= level.time ) {
			continue;
		}

		// make sure we are using the right AAS data for this entity (one's that don't get set will default to the player's AAS data)
		trap_AAS_SetCurrentWorld( cs->aasWorldIndex );

		if ( lastdest < 0 ) {
			lastdest = 0;
		}

		for ( destcount = 0, dest = lastdest, destent = &g_entities[lastdest];
			  dest < aicast_maxclients; // && destcount < level.numPlayingClients;
			  dest++, destent++ ) {
			if ( !destent->inuse ) {
				continue;
			}

			destcount++;

			if ( destent->aiInactive ) {
				continue;
			}
			if ( src == dest ) {
				continue;
			}

			dcs = AICast_GetCastState( destent->s.number );

			vis = &cs->perception.vislist[destent->s.number];

			// don't check too often
			if ( vis->lastcheck_timestamp > ( level.time - SIGHT_MIN_DELAY ) ) {
				continue;
			}

			if ( destent->health <= 0 ) {
				// only check dead guys until they are sighted
				if ( vis->lastcheck_health < 0 ) {
					continue;
				}
			}

			if ( vis->lastcheck_timestamp > level.time ) {
				continue; // let the loadgame settle down
			}
			// if they are friends, only check very infrequently
			if ( AICast_SameTeam( cs, destent->s.number ) && ( vis->lastcheck_timestamp == vis->visible_timestamp ) &&
				 ( destent->health == vis->lastcheck_health + 1 ) ) {
				if ( dcs->aiState < AISTATE_COMBAT ) {
					if ( vis->lastcheck_timestamp > ( level.time - ( 2000 + rand() % 1000 ) ) ) {
						continue; // dont check too often
					}
				} else { // check a little more frequently
					if ( vis->lastcheck_timestamp > ( level.time - ( 500 + rand() % 500 ) ) ) {
						continue; // dont check too often
					}
				}
			}

			// check for visibility
			if ( !( destent->flags & FL_NOTARGET ) && ( AICast_CheckVisibility( srcent, destent ) ) ) {
				// make sure they are still with us
				if ( destent->inuse ) {
					// record the sighting
					AICast_UpdateVisibility( srcent, destent, qtrue, qtrue );
				}
			} else // if (vis->lastcheck_timestamp == vis->real_update_timestamp)
			{
				AICast_UpdateNonVisibility( srcent, destent, qtrue );
			}

			// break if we've processed the maximum visibilities
			if ( ++count > numchecks ) {
				dest++;
				if ( dest >= aicast_maxclients ) {
					src++;
				}
				goto escape;
			}
		}

		lastdest = 0;
	}

escape:

	if ( src >= aicast_maxclients ) {
		src = 0;
	}
	lastsrc = src;
	if ( dest >= aicast_maxclients ) {
		dest = 0;
	}
	lastdest = dest;
}
