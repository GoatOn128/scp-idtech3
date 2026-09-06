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
AICast_WeaponRange
==================
*/
float AICast_WeaponRange( cast_state_t *cs, int weaponnum ) {
	switch ( weaponnum ) {
	case WP_TESLA:
		switch ( cs->aiCharacter ) {
		case AICHAR_SUPERSOLDIER: // BOSS2
			// if they have a panzer, give this weapon a shorter range
			if ( !COM_BitCheck( cs->clientState->cur_ps.weapons, WP_PANZERFAUST ) ) {
				return TESLA_SUPERSOLDIER_RANGE;
			}
		}
		return ( TESLA_RANGE * 0.9 ) - 50; // allow for bounding box
	case WP_FLAMETHROWER:
		return ( FLAMETHROWER_RANGE * 0.5 ) - 50; // allow for bounding box
	case WP_PANZERFAUST:
		return 8000;

	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
		return 800;
	case WP_MONSTER_ATTACK1:
	case WP_MONSTER_ATTACK2:
	case WP_MONSTER_ATTACK3: {
		const ai_ability_definition_t *ability = AI_FindAbility( cs, weaponnum );
		return ability ? ability->range( cs, weaponnum ) : 0;
	}

	case WP_MAUSER:
	case WP_GARAND:
	case WP_SNIPERRIFLE:
	case WP_SNOOPERSCOPE:
		return 8000;
		break;
	}
	// default range
	return 3000;
}

/*
==================
AICast_CheckAttack_real
==================
*/
qboolean AICast_CheckAttack_real( cast_state_t *cs, int enemy, qboolean allowHitWorld ) {
	//float points;
	vec3_t forward, right, start, end, dir, up, angles;
	weaponinfo_t wi;
	trace_t trace;
	float traceDist;
	static vec3_t smins = { -6, -6, -6 }, smaxs = { 6, 6, 6 };
	static vec3_t fmins = { -30, -30, -24 }, fmaxs = { 30, 30, 24 };
	float *mins, *maxs;
	float halfHeight;
	int traceMask;
	int fuzzyCount, i;
	gentity_t *ent, *enemyEnt;
	float dist;
	int passEnt;
	int weapnum;
	//
	if ( enemy < 0 ) {
		return qfalse;
	}
	ent = &g_entities[cs->entityNum];
	enemyEnt = &g_entities[enemy];
	//
	if ( cs->clientState ) {
		weapnum = cs->weaponNum;
	} else {
		weapnum = ent->client->ps.weapon;
	}
	//
	if ( !weapnum ) {
		return qfalse;
	}
	//
	// don't attack while in air (like on a ladder)
	if ( !ent->waterlevel && ent->client->ps.groundEntityNum == ENTITYNUM_NONE && !ent->active ) {
		// stim is allowed to fire while in air for flying attack
		if ( !ent->client->ps.powerups[PW_FLIGHT] ) {
			return qfalse;
		}
	}
	//
	if ( ent->health <= 0 ) {
		return qfalse;
	}
	// can't attack without any ammo
	if ( cs->clientState ) {
		if ( !AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) ) {
			return qfalse;
		}
	}
	// special case: warzombie should play laughing anim at first sight
	if ( cs->aiCharacter == AICHAR_WARZOMBIE && weapnum == WP_MONSTER_ATTACK2 ) {
		return qtrue;
	}
	//
	//if the enemy isn't directly visible
	if ( !allowHitWorld &&
		 cs->perception.vislist[enemy].real_visible_timestamp != cs->perception.vislist[enemy].real_update_timestamp ) {
		return qfalse;
	}
	//
	//get the weapon info (FIXME: hard-code the weapon info?)
	memset( &wi, 0, sizeof( weaponinfo_t ) );
	//
	traceMask = MASK_SHOT; // FIXME: assign mask's to different weapons
	//end point aiming at
	if ( !ent->active ) {
		//get the start point shooting from
		VectorCopy( enemyEnt->r.currentOrigin, start );
		start[2] += enemyEnt->client->ps.viewheight;
		VectorCopy( ent->r.currentOrigin, end );
		end[2] += ent->client->ps.viewheight;
		VectorSubtract( start, end, dir );
		vectoangles( dir, angles );
		AngleVectors( angles, forward, right, up );
		CalcMuzzlePoint( &g_entities[cs->entityNum], weapnum, forward, right, up, start );

		traceDist = AICast_WeaponRange( cs, weapnum );
		switch ( weapnum ) {
		case WP_GAUNTLET:
			mins = NULL;
			maxs = NULL;
			break;
		case WP_DYNAMITE:
		case WP_PANZERFAUST:
		case WP_GRENADE_LAUNCHER:
		case WP_GRENADE_PINEAPPLE:
			traceMask = MASK_MISSILESHOT;
			mins = smins;
			maxs = smaxs;
			break;
		case WP_FLAMETHROWER:
			mins = fmins;
			maxs = fmaxs;
			break;
		default:
			mins = smins;
			maxs = smaxs;
			break;
		}
		passEnt = cs->entityNum;

		// don't try too far
		dist = Distance( start, enemyEnt->r.currentOrigin );
		fuzzyCount = 6;
		if ( traceDist > dist ) {
			traceDist = dist;
		} else {
			dist -= enemyEnt->r.maxs[0]; // subtract distance to edge of bounding box edge
			if ( traceDist < dist ) {
				return qfalse;
			}
		}
	} else {
		gentity_t *mg42;
		// we are mounted on a weapon
		mg42 = &g_entities[cs->mountedEntity];
		VectorCopy( enemyEnt->r.currentOrigin, start );
		start[2] += enemyEnt->client->ps.viewheight;
		VectorCopy( mg42->r.currentOrigin, end );
		VectorSubtract( start, end, dir );
		vectoangles( dir, angles );
		AngleVectors( angles, forward, right, up );

		VectorCopy( mg42->r.currentOrigin, start );
		VectorMA( start, 16, forward, start );
		VectorMA( start, 16, up, start );
		// snap to integer coordinates for more efficient network bandwidth usage
		SnapVector( start );

		traceDist = 8192;
		mins = NULL;
		maxs = NULL;
		if ( mg42->mg42BaseEnt >= 0 ) {
			passEnt = mg42->mg42BaseEnt;
		} else {
			passEnt = cs->entityNum;
		}

		// don't try too far
		dist = Distance( start, enemyEnt->r.currentOrigin );
		if ( traceDist > dist ) {
			traceDist = dist;
			fuzzyCount = 6;
		} else { //if (dist > traceDist - 32) {
			return qfalse;
		}
		/*} else {
			fuzzyCount = 0;
		}*/
	}

	for ( i = 0; i <= fuzzyCount; i++ ) {
		VectorMA( start, traceDist, forward, end );

		// fuzzy end point
		if ( i > 0 ) {
			VectorMA( end, enemyEnt->r.maxs[0] * 0.9 * (float)( ( i % 2 ) * 2 - 1 ), right, end );
			halfHeight = ( enemyEnt->r.maxs[2] - enemyEnt->r.mins[2] ) / 2.0;
			end[2] = ( enemyEnt->r.currentOrigin[2] + enemyEnt->r.mins[2] ) + halfHeight;
			VectorMA( end, halfHeight * 0.9 * ( ( (float)( ( i - 1 ) - ( ( i - 1 ) % 2 ) ) / 2 - 1.0 ) ), up, end );
		}

		if ( /*allowHitWorld &&*/ !trap_InPVS( start, end ) ) {
			// not possibly attackable
			//continue;
			return qfalse;
		}

		trap_Trace( &trace, start, mins, maxs, end, passEnt, traceMask );
		if ( trace.fraction == 1.0 ) {
			if ( !trace.startsolid ) {
				return qtrue; // not sure why, but this fixes blackguards in chateau shooting through glass ceiling
			}
			//return qfalse;
			continue;
		}
		//if won't hit the enemy
		if ( trace.entityNum != enemy ) {

			// RF, assume we can shoot through props (chairs, etc)
			if ( g_entities[trace.entityNum].takedamage && g_entities[trace.entityNum].health > 0 &&
				 !Q_strncmp( g_entities[trace.entityNum].classname, "props_", 6 ) ) {
				return qtrue;
			}

			if ( !allowHitWorld ) {
				continue;
			}

			if ( trace.startsolid ) {
				continue;
			}

			//if the entity is a client
			if ( trace.entityNum >= 0 && trace.entityNum < MAX_CLIENTS ) {
				//if a teammate is hit
				if ( AICast_SameTeam( cs, trace.entityNum ) ) {
					return qfalse;
				}
			}
			//if the projectile does a radial damage
			if ( cs->weaponNum == WP_PANZERFAUST ) {
				if ( Distance( trace.endpos, g_entities[enemy].s.pos.trBase ) > 120 ) {
					continue;
				}
				//FIXME: check if a teammate gets radial damage
			}
		}
		// will successfully hit enemy
		return qtrue;
	}
	//
	return qfalse;
}

/*
==================
AICast_CheckAttackAtPos
==================
*/
qboolean AICast_CheckAttackAtPos( int entnum, int enemy, vec3_t pos, qboolean ducking, qboolean allowHitWorld ) {
	gentity_t *ent;
	vec3_t savepos;
	int saveview;
	qboolean rval;
	cast_state_t *cs;

	cs = AICast_GetCastState( entnum );
	ent = &g_entities[cs->clientState->entitynum];

	VectorCopy( ent->r.currentOrigin, savepos );
	VectorCopy( pos, ent->r.currentOrigin );

	saveview = ent->client->ps.viewheight;
	if ( ducking ) {
		if ( ent->client->ps.viewheight != ent->client->ps.crouchViewHeight ) {
			ent->client->ps.viewheight = ent->client->ps.crouchViewHeight;
		}
	} else {
		if ( ent->client->ps.viewheight != ent->client->ps.standViewHeight ) {
			ent->client->ps.viewheight = ent->client->ps.standViewHeight;
		}
	}

	rval = AICast_CheckAttack_real( cs, enemy, allowHitWorld );

	VectorCopy( savepos, ent->r.currentOrigin );
	ent->client->ps.viewheight = saveview;

	return rval;
}

/*
==================
AICast_CheckAttack

  optimization, uses the cache to avoid possible duplicate calls with same world paramaters
==================
*/
qboolean AICast_CheckAttack( cast_state_t *cs, int enemy, qboolean allowHitWorld ) {
	if ( cs->clientState ) {
		if ( ( cs->combat.checkAttackCache.time == level.time ) && ( cs->combat.checkAttackCache.enemy == enemy ) &&
			 ( cs->combat.checkAttackCache.weapon == cs->weaponNum ) &&
			 ( cs->combat.checkAttackCache.allowHitWorld == allowHitWorld ) ) {
			//G_Printf( "checkattack cache hit\n" );
			return ( cs->combat.checkAttackCache.result );
		} else {
			cs->combat.checkAttackCache.allowHitWorld = allowHitWorld;
			cs->combat.checkAttackCache.enemy = enemy;
			cs->combat.checkAttackCache.time = level.time;
			cs->combat.checkAttackCache.weapon = cs->weaponNum;
			return ( cs->combat.checkAttackCache.result = AICast_CheckAttack_real( cs, enemy, allowHitWorld ) );
		}
	} else {
		return AICast_CheckAttack_real( cs, enemy, allowHitWorld );
	}
}

/*
==============
AICast_WeaponWantScale
==============
*/
float AICast_WeaponWantScale( cast_state_t *cs, int weapon ) {
	switch ( weapon ) {
	case WP_GAUNTLET:
		return 0.1;
	case WP_FLAMETHROWER:
		return 2.0; // if we have this up close, definately use it
	default:
		return 1.0;
	}
}

/*
==============
AICast_GotEnoughAmmoForWeapon
==============
*/
qboolean AICast_GotEnoughAmmoForWeapon( cast_state_t *cs, int weapon ) {
	gentity_t *ent;
	int ammo, clip;

	ent = &g_entities[cs->entityNum];
	ammo = ent->client->ps.ammo[BG_FindAmmoForWeapon( weapon )];
	clip = ent->client->ps.ammoclip[BG_FindClipForWeapon( weapon )];

	// TODO!! check some kind of weapon list that holds the minimum requirements for each weapon
	switch ( weapon ) {
	case WP_GAUNTLET:
		return qtrue;
	default:
		return (qboolean)( ( clip >= ammoTable[weapon].uses ) || ( ammo >= ammoTable[weapon].uses ) ); //----(SA)
	}
}

/*
==============
AICast_WeaponUsable

  This is used to prevent weapons from being selected, even if they have ammo.

  This can be used to add a delay between firing for special attacks, or make certain
  that certain weapons are only selected within a certain range or under certain conditions.

  NOTE: that monster_attack2 will always override monster_attack1 if both are usable
==============
*/
qboolean AICast_WeaponUsable( cast_state_t *cs, int weaponNum ) {
	int delay, oldweap, hitclient;
	float dist = -1;
	gentity_t *ent, *grenade;

	if ( cs->enemyNum >= 0 ) {
		dist = Distance( cs->clientState->origin, g_entities[cs->enemyNum].s.pos.trBase );
	}

	oldweap = cs->weaponNum;
	ent = &g_entities[cs->entityNum];
	delay = -1;

	// just return qfalse if this weapon isn't ready for use
	switch ( weaponNum ) {
		// don't attempt to lob a grenade more than this often, since we will abort a grenade
		// throw if it's not safe, we shouldn't keep switching back too quickly
	case WP_DYNAMITE:
	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
		if ( cs->enemyNum < 0 ) {
			return qfalse;
		}
		delay = 5000;
		if ( dist > 0 && dist < 200 ) {
			return qfalse;
		}
		if ( cs->combat.weaponFireTimes[weaponNum] < level.time - delay ) {
			// make sure it's safe
			CalcMuzzlePoints( ent, weaponNum );
			grenade = weapon_grenadelauncher_fire( ent, weaponNum );
			hitclient = AICast_SafeMissileFire( grenade, grenade->nextthink - level.time, cs->enemyNum,
												g_entities[cs->enemyNum].s.pos.trBase, cs->entityNum, NULL );
			G_FreeEntity( grenade );
			if ( hitclient > -1 ) {
				return qtrue;
			} else {
				return qfalse; // it's not safe
			}
		}
		break;
	case WP_TESLA:
		switch ( cs->aiCharacter ) {
		case AICHAR_STIMSOLDIER3:
			if ( dist < 0 || dist >= TESLA_RANGE ) {
				return qfalse;
			}
		}
		break;
	case WP_MONSTER_ATTACK1:
	case WP_MONSTER_ATTACK2:
	case WP_MONSTER_ATTACK3: {
		const ai_ability_definition_t *ability = AI_FindAbility( cs, weaponNum );
		return ability && ability->available( cs, weaponNum );
	}
	default:
		delay = -1;
	}
	//
	return ( !cs->combat.weaponFireTimes[weaponNum] || ( cs->combat.weaponFireTimes[weaponNum] < level.time - delay ) );
}

/*
==============
AICast_ChooseWeapon
==============
*/
void AICast_ChooseWeapon( cast_state_t *cs, qboolean battleFunc ) {
	int i;
	int *ammo;
	float wantScale, bestWantScale, enemyDist = 0;
	qboolean inRange = qfalse, thisInRange, gotOne;

	AI_GetClientState( cs->entityNum, &( cs->clientState->cur_ps ) );
	ammo = cs->clientState->cur_ps.ammo;
	bestWantScale = 0.0;

	if ( cs->enemyNum >= 0 ) {
		enemyDist = VectorDistance( g_entities[cs->enemyNum].s.pos.trBase, cs->clientState->origin );
		// subtract distance to edge of bounding box
		enemyDist -= g_entities[cs->enemyNum].r.maxs[0];
	}

	if ( cs->clientState->cur_ps.weaponstate == WEAPON_RAISING ||
		 cs->clientState->cur_ps.weaponstate == WEAPON_RAISING_TORELOAD || //----(SA)	added
		 cs->clientState->cur_ps.weaponstate == WEAPON_DROPPING ||
		 cs->clientState->cur_ps.weaponstate == WEAPON_DROPPING_TORELOAD ) { //----(SA)	added
		return;
	}

	// disabled this, makes grenade guy keep trying to throw a grenade he doesn't have
	//	if (cs->clientState->cur_ps.weaponDelay || cs->clientState->cur_ps.weaponTime)
	//		return;

	if ( cs->weaponNum && ( cs->script.castScriptStatus.scriptFlags & SFL_NOCHANGEWEAPON ) ) {
		if ( AICast_GotEnoughAmmoForWeapon( cs, cs->weaponNum ) && AICast_WeaponUsable( cs, cs->weaponNum ) ) {
			return;
		} else {
			cs->script.castScriptStatus.scriptFlags &= ~SFL_NOCHANGEWEAPON;
		}
	} else {
		if ( cs->weaponNum == WP_GRENADE_LAUNCHER || cs->weaponNum == WP_GRENADE_PINEAPPLE ) {
			cs->weaponNum = WP_NONE; // dont use grenades at will
		}
	}

	gotOne = qfalse;

	// choose the best weapon to fight with
	for ( i = 0; i < WP_NUM_WEAPONS; i++ ) {
		if ( i == WP_GRENADE_LAUNCHER || i == WP_GRENADE_PINEAPPLE ) {
			continue; // never choose grenades at will, only when going into grenade flush mode
		}

		if ( !battleFunc && AI_FindAbility( cs, i ) ) {
			continue; // Special abilities start through the combat state.
		}

		if ( COM_BitCheck( cs->clientState->cur_ps.weapons, i ) ) {
			gotOne = qtrue;
			// check that our ammo is enough
			if ( !AICast_GotEnoughAmmoForWeapon( cs, i ) || !AICast_WeaponUsable( cs, i ) ) {
				continue;
			}
			// get the wantScale for this weapon given the current circumstances (0.0 - 1.0)
			wantScale = AICast_WeaponWantScale( cs, i );
			thisInRange = qfalse;
			// in range?
			if ( enemyDist && AICast_WeaponRange( cs, i ) > enemyDist ) {
				thisInRange = qtrue;
			}
			//
			if ( ( !inRange && thisInRange ) || ( ( !inRange || thisInRange ) && ( wantScale >= bestWantScale ) ) ) {
				cs->weaponNum = i;
				bestWantScale = wantScale;
				if (
					thisInRange ) { // we have found a weapon inside attackable range, don't override with one outside range
					inRange = qtrue;
				}
			}
		}
	}

	if ( !gotOne && ( cs->weaponNum < WP_MONSTER_ATTACK1 || cs->weaponNum > WP_MONSTER_ATTACK3 ) ) {
		if ( g_cheats.integer && ( !cs->clientState->cur_ps.weapons[0] && !cs->clientState->cur_ps.weapons[1] ) ) {
			// (SA) the print statement is a bit much.  lots of actors have no ammo...
			//			G_Printf( "AI: %s has no ammo\n", g_entities[cs->entityNum].aiName);
		}
		// select no weapon
		cs->weaponNum = WP_NONE;
		// if we have no weapons at all, we dont need to switch
		if ( !cs->clientState->cur_ps.weapons[0] && !cs->clientState->cur_ps.weapons[1] ) {
			g_entities[cs->entityNum].client->ps.weapon = WP_NONE;
		}
	}
}

/*
==================
AICast_WeaponSway

  Some weapons should be "sprayed" around a bit while firing
==================
*/
void AICast_WeaponSway( cast_state_t *cs, vec3_t ofs ) {
	VectorClear( ofs );
	switch ( cs->weaponNum ) {
	case WP_MONSTER_ATTACK1:
		if ( cs->aiCharacter != AICHAR_ZOMBIE ) {
			break; // only allow flaming zombie beyond here
		}
	case WP_FLAMETHROWER:
		ofs[PITCH] = ( 3.0 + 4.0 * sin( ( (float)level.time / 320.0 ) ) ) * sin( ( (float)level.time / 500.0 ) );
		ofs[YAW] = ( 6.0 + 8.0 * sin( ( (float)level.time / 250.0 ) ) ) * sin( ( (float)level.time / 400.0 ) );
		ofs[ROLL] = 0;
		break;
	case WP_VENOM:
		ofs[PITCH] = 2 * (float)cos( ( level.time / 200 ) );
		ofs[YAW] = 10 * (float)sin( ( level.time / 150 ) ) * (float)sin( ( level.time / 100 ) );
		ofs[ROLL] = 0;
		break;
	}
}

/*
==================
AICast_AimAtEnemy
==================
*/
qboolean AICast_AimAtEnemy( cast_state_t *cs ) {
	ai_client_state_t *bs;
	float aim_skill, aim_accuracy;
	vec3_t dir, bestorigin, start, enemyOrg;
	//	vec3_t mins = {-4,-4,-4}, maxs = {4, 4, 4};
	float dist;
	cast_visibility_t *vis;

	//
	if ( cs->script.castScriptStatus.scriptNoAttackTime >= ( level.time + 500 ) ) {
		return qfalse;
	}
	//
	if ( cs->combat.noAttackTime >= level.time ) {
		return qfalse;
	}
	//
	bs = cs->clientState;
	//
	//if the bot has no enemy
	if ( cs->enemyNum < 0 ) {
		return qfalse;
	}
	//
	aim_skill = cs->attributes[AIM_SKILL];
	aim_accuracy = AICast_GetAccuracy( cs->entityNum );
	if ( aim_accuracy <= 0 ) {
		aim_accuracy = 0.0001;
	}

	// StimSoldier is very good at firing Rocket Launcher
	if ( cs->aiCharacter == AICHAR_STIMSOLDIER2 && cs->weaponNum == WP_PANZERFAUST ) {
		aim_skill = 1;
		aim_accuracy = 1;
	}

	//get the weapon information

	//get the enemy entity information
	vis = &cs->perception.vislist[cs->enemyNum];
	if ( vis->visible_timestamp < vis->lastcheck_timestamp ) {
		// use our last visible position of them
		if ( vis->real_visible_timestamp == vis->lastcheck_timestamp ) {
			VectorCopy( vis->real_visible_pos, bestorigin );
		} else {
			VectorCopy( vis->visible_pos, bestorigin );
		}
	} else {
		// we can see them, if this weapon isn't a direct-hit weapon (bullets),
		// then predict where they are going to be
		if ( cs->weaponNum == WP_GRENADE_LAUNCHER || cs->weaponNum == WP_GRENADE_PINEAPPLE ) {
			aicast_predictmove_t move;
			AICast_PredictMovement( AICast_GetCastState( cs->enemyNum ), 1, 1.0, &move,
									&g_entities[cs->enemyNum].client->pers.cmd, -1 );
			VectorCopy( move.endpos, bestorigin );
		} else { // they are visible, use actual position
			VectorCopy( g_entities[cs->enemyNum].client->ps.origin, bestorigin );
		}
	}

	bestorigin[2] += g_entities[cs->enemyNum].client->ps.viewheight;
	//get the start point shooting from
	//NOTE: the x and y projectile start offsets are ignored
	VectorCopy( bs->origin, start );
	start[2] += bs->cur_ps.viewheight;
	//
	VectorCopy( bestorigin, enemyOrg );
	//
	// grenade hack: aim grenades at their feet if they are close
	if ( cs->weaponNum == WP_GRENADE_LAUNCHER || cs->weaponNum == WP_GRENADE_PINEAPPLE ) {
		if ( Distance( start, bestorigin ) < 180 ) {
			bestorigin[2] = enemyOrg[2] + g_entities[cs->enemyNum].r.mins[2] + crandom() * 20;
		} else if ( Distance( start, bestorigin ) > 400 ) { // aim up a bit for distance
			bestorigin[2] += 12 + Distance( start, bestorigin ) / 50 + crandom() * 20;
		}
	}
	dist = Distance( bs->eye, bestorigin );
	// rocket launcher should aim ahead
	if ( cs->weaponNum == WP_PANZERFAUST ) {
		VectorMA( bestorigin, aim_skill * aim_skill * ( dist / 900 ), g_entities[cs->enemyNum].client->ps.velocity,
				  bestorigin );
		// if they are close, aim down at their feet
		if ( dist < 512 ) {
			bestorigin[2] -=
				( VectorLength( g_entities[cs->enemyNum].client->ps.velocity ) / 500.0 ) * ( 1.0 - dist / 2048 ) *
				( bestorigin[2] -
				  ( g_entities[cs->enemyNum].client->ps.origin[2] + g_entities[cs->enemyNum].client->ps.mins[2] ) );
		}
	}
	// if the enemy is moving, they are harder to hit
	if ( dist > 256 ) {
		VectorMA( bestorigin,
				  ( 0.3 + 0.7 * ( 1 - aim_accuracy ) ) * 0.4 *
					  sin( (float)level.time / ( 500.0 + ( 100.0 * ( ( cs->entityNum + 3 ) % 4 ) ) ) ),
				  g_entities[cs->enemyNum].client->ps.velocity, bestorigin );
	}
	// if we are good at aiming, we should aim ahead of where they are now
	// since by the time we have rotated to that direction, some time will have passed
	if ( aim_skill > 0.2 ) {
		VectorMA( bestorigin, aim_skill * 0.2, g_entities[cs->enemyNum].client->ps.velocity, bestorigin );
	}
	//get aim direction
	VectorSubtract( bestorigin, bs->eye, dir );
	//set the ideal view angles
	vectoangles( dir, cs->ideal_viewangles );

	return qtrue; // do real aim checking after we've moved the angles
}

/*
==================
AICast_CanMoveWhileFiringWeapon
==================
*/
qboolean AICast_CanMoveWhileFiringWeapon( int weaponnum ) {
	switch ( weaponnum ) {
	case WP_MAUSER:
	case WP_GARAND:
	case WP_SNIPERRIFLE: //----(SA)	added
	case WP_SNOOPERSCOPE: //----(SA)	added
		//case WP_FG42SCOPE:		//----(SA)	added
	case WP_PANZERFAUST:
		return qfalse;
	default:
		return qtrue;
	}
}

/*
================
AICast_RandomTriggerRelease
================
*/
qboolean AICast_RandomTriggerRelease( cast_state_t *cs ) {
	// some characters override all weapon settings for trigger release
	switch ( cs->aiCharacter ) {
	case AICHAR_BLACKGUARD: // this is here since his "ready" frame is different to his firing frame, so it looks wierd to keep swapping between them
	case AICHAR_STIMSOLDIER1:
	case AICHAR_STIMSOLDIER2:
	case AICHAR_STIMSOLDIER3:
		return qfalse;
	}

	switch ( cs->weaponNum ) {
	case WP_MP40:
	case WP_VENOM:
	case WP_FG42SCOPE:
	case WP_FG42:
		//case WP_FLAMETHROWER:
		return qtrue;
	default:
		return qfalse;
	}
}

/*
==================
AICast_ProcessAttack

  NOTE: this should always be called after the movement has been processed
==================
*/
void AICast_ProcessAttack( cast_state_t *cs ) {
	ai_client_state_t *bs;

	// if our enemy is dead, stop attacking them
	if ( cs->enemyNum >= 0 && g_entities[cs->enemyNum].health <= 0 ) {
		return;
	}
	//
	if ( cs->script.castScriptStatus.scriptNoAttackTime >= level.time ) {
		return;
	}
	//
	if ( cs->combat.noAttackTime >= level.time ) {
		return;
	}
	// select a weapon
	AICast_ChooseWeapon( cs, qfalse );
	//
	if ( cs->weaponNum == WP_NONE ) {
		return;
	}
	// never fire grenades from within here (needs special AIFunc_GrenadeFlush() code)
	if ( cs->weaponNum == WP_GRENADE_LAUNCHER || cs->weaponNum == WP_GRENADE_PINEAPPLE ) {
		return;
	}
	//
	bs = cs->clientState;
	// check for not firing while moving flag, if present, abort attack if any movement has been issued
	if ( !AICast_CanMoveWhileFiringWeapon( cs->weaponNum ) ) {
		// if we are moving, don't fire
		bot_input_t bi;
		if ( cs->clientState->cur_ps.weaponTime > 200 ) {
			// if we recently fired, don't let us move for a bit
			cs->speedScale = 0;
			AICast_AimAtEnemy( cs ); // keep looking at them regardless
		}
		// if we're trying to move somewhere, don't let us shoot, until we've arrived
		trap_EA_GetInput( bs->client, (float)level.time / 1000, &bi );
		if ( ( cs->script.castScriptStatus.scriptNoMoveTime < level.time ) &&
			 ( ( bi.actionflags & ACTION_MOVEFORWARD ) || ( bi.actionflags & ACTION_MOVEBACK ) ||
			   ( bi.actionflags & ACTION_MOVELEFT ) || ( bi.actionflags & ACTION_MOVERIGHT ) || ( bi.speed ) ) ) {
			return;
		}
	}
	//
	//if not a "walk forward" AI, then aim at the enemy regardless of whether we can attack them or not
	if ( !( cs->aiFlags & AIFL_WALKFORWARD ) ) {
		if ( !AICast_AimAtEnemy( cs ) ) {
			return;
		}
	}
	//
	// if we are stuck in this position, we should duck if we can't hit them
	if ( !AICast_CheckAttack( cs, cs->enemyNum, qfalse ) ) {
		// we should duck if the enemy is shooting at us, and we can't hit them
		if ( cs->attributes[ATTACK_CROUCH] && ( cs->script.castScriptStatus.scriptNoMoveTime >= level.time ) ) {
			if ( !AICast_CheckAttackAtPos( cs->entityNum, cs->enemyNum, cs->clientState->origin, qfalse, qfalse ) ) {
				cs->attackcrouch_time = level.time + 2000;
			} else {
				cs->attackcrouch_time = 0; // we can attack them if we stand, so go for it
			}
		}
		return;
	}
	//
	//if we are a "walk forward" AI, then aim at the enemy only if we can attack them
	if ( cs->aiFlags & AIFL_WALKFORWARD ) {
		if ( !AICast_AimAtEnemy( cs ) ) {
			return;
		}
	}
	//
	// release the trigger every now and then
	if ( AICast_RandomTriggerRelease( cs ) && cs->combat.triggerReleaseTime < ( level.time - 500 ) ) {
		if ( rand() % 5 == 0 ) {
			cs->combat.triggerReleaseTime = level.time + 100 + rand() % 100;
			return;
		}
	}
	//
	if ( cs->combat.triggerReleaseTime > level.time ) {
		return;
	}
	//
	// FIXME: handle fire-on-release weapons?
	trap_EA_Attack( bs->client );
	//
	cs->bFlags |= AI_FRAME_ATTACKED;
}

/*
===============
AICast_RecordWeaponFire

  used for scripting, so we know when the weapon has been fired
===============
*/
void AICast_RecordWeaponFire( gentity_t *ent ) {
	cast_state_t *cs;
	float range;

	cs = AICast_GetCastState( ent->s.number );
	cs->combat.lastWeaponFired = level.time;
	cs->combat.lastWeaponFiredWeaponNum = ent->client->ps.weapon;
	VectorCopy( ent->r.currentOrigin, cs->combat.lastWeaponFiredPos );

	cs->combat.weaponFireTimes[cs->combat.lastWeaponFiredWeaponNum] = level.time;

	// do sighting
	range = AICast_GetWeaponSoundRange( cs->combat.lastWeaponFiredWeaponNum );

	AICast_AudibleEvent( cs->entityNum, cs->combat.lastWeaponFiredPos, range );

	if ( cs->clientState ) { // real player's don't need to play AI sounds
		AIChar_AttackSound( cs );
	}
}

/*
===============
AICast_StopAndAttack

  returns qtrue if they should go back to a battle state to attack,
  qfalse if they should keep chasing while they attack (like the Zombie)
===============
*/
qboolean AICast_StopAndAttack( cast_state_t *cs ) {
	float dist = -1;
	cast_state_t *ecs;

	if ( cs->enemyNum >= 0 ) {
		dist = Distance( cs->clientState->origin, g_entities[cs->enemyNum].r.currentOrigin );
	}

	switch ( cs->weaponNum ) {

		// if they are using Venom, and are too far away to be effective, then keep chasing
	case WP_VENOM:
		if ( dist > 300 ) {
			return qfalse;
		}
		// if we haven't injured them in a while, advance
		if ( cs->enemyNum >= 0 ) {
			ecs = AICast_GetCastState( cs->enemyNum );
			if ( ecs->lastPain < level.time - 3000 ) {
				return qfalse;
			}
		}
		break;
		// if they are using tesla (SUPERSOLDIER / BOSS2) try and get close
	case WP_TESLA:
		if ( dist > 128 /*&& (level.time%10000 < 8000)*/ ) {
			return qfalse;
		}
		// if we haven't injured them in a while, advance
		if ( cs->enemyNum >= 0 ) {
			ecs = AICast_GetCastState( cs->enemyNum );
			if ( ecs->lastPain < level.time - 3000 ) {
				return qfalse;
			}
		}
		break;
	case WP_PANZERFAUST:
		// if we haven't injured them in a while, advance
		if ( ( cs->aiCharacter == AICHAR_PROTOSOLDIER || cs->aiCharacter == AICHAR_SUPERSOLDIER ) &&
			 cs->enemyNum >= 0 ) {
			if ( dist > 300 ) {
				return qfalse;
			}
			ecs = AICast_GetCastState( cs->enemyNum );
			if ( ecs->lastPain < level.time - 3000 ) {
				return qfalse;
			}
		}
		break;
	case WP_FLAMETHROWER:
		// if we haven't injured them in a while, advance
		if ( cs->aiCharacter == AICHAR_VENOM && cs->enemyNum >= 0 ) {
			ecs = AICast_GetCastState( cs->enemyNum );
			if ( ecs->lastPain < level.time - 3000 ) {
				return qfalse;
			}
		}
		break;
	}

	return qtrue;
}

/*
===============
AICast_GetAccuracy
===============
*/
float AICast_GetAccuracy( int entnum ) {
#define AICAST_VARIABLE_ACC_ENABLED 1
#define AICAST_ACC_VISTIME ( 500 + ( 3500 * ( 1.0 - aicast_skillscale ) ) )
#define AICAST_ACC_SCALE 0.4
	cast_state_t *cs;
	float acc;

	cs = AICast_GetCastState( entnum );
	// the more they stay in our sights, the more accurate we get
	acc = cs->attributes[AIM_ACCURACY];

	if ( AICAST_VARIABLE_ACC_ENABLED ) {
		if ( cs->enemyNum >= 0 ) {
			if ( cs->perception.vislist[cs->enemyNum].real_notvisible_timestamp < level.time - AICAST_ACC_VISTIME ) {
				acc += 0.5 * AICAST_ACC_SCALE;
			} else {
				acc += AICAST_ACC_SCALE * ( (float)( -0.5 * AICAST_ACC_VISTIME + level.time -
													 cs->perception.vislist[cs->enemyNum].real_notvisible_timestamp ) /
											(float)( AICAST_ACC_VISTIME ) );
			}

			if ( acc > 1.0 ) {
				acc = 1.0;
			} else if ( acc < 0.0 ) {
				acc = 0.0;
			}
		}
	}
	return ( acc );
}

qboolean AICast_HasFiredWeapon( int entNum, int weapon ) {
	if ( AICast_GetCastState( entNum )->combat.weaponFireTimes[weapon] ) {
		return qtrue;
	}

	return qfalse;
}
