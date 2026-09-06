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
Contains the code to handle the various commands available with an event script.

These functions will return true if the action has been performed, and the script
should proceed to the next item on the list.
*/

/*
===============
AICast_NoAttackIfNotHurtSinceLastScriptAction

  Not an actual command, this is just used by the script code
===============
*/
void AICast_NoAttackIfNotHurtSinceLastScriptAction( cast_state_t *cs ) {
	if ( cs->script.castScriptStatus.scriptNoAttackTime > level.time ) {
		return;
	}

	// if not moving, we should attack
	if ( VectorLength( cs->clientState->velocity ) < 10 ) {
		return;
	}

	// if our enemy is in the direction we are moving, don't hold back
	if ( cs->enemyNum >= 0 && cs->script.castScriptStatus.scriptGotoEnt >= 0 ) {
		vec3_t v;

		VectorSubtract( g_entities[cs->enemyNum].r.currentOrigin, cs->clientState->origin, v );
		if ( DotProduct( cs->clientState->velocity, v ) > 0 ) {
			return;
		}
	}

	if ( cs->lastPain < cs->script.castScriptStatus.castScriptStackChangeTime ) {
		cs->script.castScriptStatus.scriptNoAttackTime = level.time + FRAMETIME;
	}
}

/*
=================
AICast_ScriptAction_NoAttack

  syntax: noattack <duration>
=================
*/
qboolean AICast_ScriptAction_NoAttack( cast_state_t *cs, char *params ) {
	if ( !params ) {
		G_Error( "AI Scripting: syntax error\n\nnoattack <duration>\n" );
	}

	cs->script.castScriptStatus.scriptNoAttackTime = level.time + atoi( params );

	return qtrue;
}

/*
=================
AICast_ScriptAction_Attack

  syntax: attack [ainame]

  Resumes attacking after a noattack was issued

  if ainame is given, we will attack only that entity as long as they are alive
=================
*/
qboolean AICast_ScriptAction_Attack( cast_state_t *cs, char *params ) {
	gentity_t *ent;

	cs->script.castScriptStatus.scriptNoAttackTime = 0;

	// if we have specified an aiName, then we should attack only this person
	if ( params ) {
		ent = AICast_FindEntityForName( params );
		if ( !ent ) {
			G_Error( "AI Scripting: \"attack\" command unable to find aiName \"%s\"", params );
		}
		cs->script.castScriptStatus.scriptAttackEnt = ent->s.number;
		cs->enemyNum = ent->s.number;
	} else {
		cs->script.castScriptStatus.scriptAttackEnt = -1;
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_SetAmmo

  syntax: setammo <pickupname> <count>
=================
*/
qboolean AICast_ScriptAction_SetAmmo( cast_state_t *cs, char *params ) {
	char *pString, *token;
	int weapon;
	int i;

	pString = params;

	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI Scripting: setammo without ammo identifier\n" );
	}

	weapon = WP_NONE;

	for ( i = 1; bg_itemlist[i].classname; i++ ) {
		//----(SA)	first try the name they see in the editor, then the pickup name
		if ( !Q_strcasecmp( token, bg_itemlist[i].classname ) ) {
			weapon = bg_itemlist[i].giTag;
			break;
		}

		if ( !Q_strcasecmp( token, bg_itemlist[i].pickup_name ) ) {
			weapon = bg_itemlist[i].giTag;
			break;
		}
	}

	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI Scripting: setammo without ammo count\n" );
	}

	if ( weapon != WP_NONE ) {
		// give them the ammo

		if ( atoi( token ) ) {
			int amt;
			amt = atoi( token );
			if ( amt > 50 + ammoTable[BG_FindAmmoForWeapon( weapon )].maxammo ) {
				amt = 999; // unlimited
			}
			Add_Ammo( &g_entities[cs->entityNum], weapon, amt, qtrue );
		} else {
			// remove ammo for this weapon
			g_entities[cs->entityNum].client->ps.ammo[BG_FindAmmoForWeapon( weapon )] = 0;
			g_entities[cs->entityNum].client->ps.ammoclip[BG_FindClipForWeapon( weapon )] = 0;
		}

	} else {

		if ( g_cheats.integer ) {
			G_Printf( "--SCRIPTER WARNING-- AI Scripting: setammo: unknown ammo \"%s\"", params );
		}
		return qfalse; // (SA) temp as scripts transition to new names
	}

	return qtrue;
};

/*
=================
AICast_ScriptAction_SetClip

  syntax: setclip <weapon name> <count>

=================
*/
qboolean AICast_ScriptAction_SetClip( cast_state_t *cs, char *params ) {
	char *pString, *token;
	int weapon;
	int i;

	pString = params;

	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI Scripting: setclip without weapon identifier\n" );
	}

	weapon = WP_NONE;

	for ( i = 1; bg_itemlist[i].classname; i++ ) {
		//----(SA)	first try the name they see in the editor, then the pickup name
		if ( !Q_strcasecmp( token, bg_itemlist[i].classname ) ) {
			weapon = bg_itemlist[i].giTag;
			break;
		}

		if ( !Q_strcasecmp( token, bg_itemlist[i].pickup_name ) ) {
			weapon = bg_itemlist[i].giTag;
			break;
		}
	}

	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI Scripting: setclip without ammo count\n" );
	}

	if ( weapon != WP_NONE ) {

		int spillover = atoi( token ) - ammoTable[weapon].maxclip;

		if ( spillover > 0 ) {
			// there was excess, put it in storage and fill the clip
			g_entities[cs->entityNum].client->ps.ammo[BG_FindAmmoForWeapon( weapon )] += spillover;
			g_entities[cs->entityNum].client->ps.ammoclip[BG_FindClipForWeapon( weapon )] = ammoTable[weapon].maxclip;
		} else {
			// set the clip amount to the exact specified value
			g_entities[cs->entityNum].client->ps.ammoclip[weapon] = atoi( token );
		}

	} else {
		//		G_Printf( "--SCRIPTER WARNING-- AI Scripting: setclip: unknown weapon \"%s\"", params );
		return qfalse; // (SA) temp as scripts transition to new names
		//		G_Error( "AI Scripting: setclip: unknown weapon \"%s\"", params );
	}

	return qtrue;
};

/*
==============
AICast_ScriptAction_SuggestWeapon
==============
*/
qboolean AICast_ScriptAction_SuggestWeapon( cast_state_t *cs, char *params ) {
	int weapon;
	int i;
	//int		suggestedweaps = 0; // TTimo: unused

	weapon = WP_NONE;

	for ( i = 1; bg_itemlist[i].classname; i++ ) {
		//----(SA)	first try the name they see in the editor, then the pickup name
		if ( !Q_strcasecmp( params, bg_itemlist[i].classname ) ) {
			weapon = bg_itemlist[i].giTag;
			break;
		}

		if ( !Q_strcasecmp( params, bg_itemlist[i].pickup_name ) ) {
			weapon = bg_itemlist[i].giTag;
			break;
		}
	}

	if ( weapon != WP_NONE ) {
		G_AddEvent( &g_entities[cs->entityNum], EV_SUGGESTWEAP, weapon );
	} else {
		G_Error( "AI Scripting: suggestweapon: unknown weapon \"%s\"", params );
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_SelectWeapon

  syntax: selectweapon <pickupname>
=================
*/
qboolean AICast_ScriptAction_SelectWeapon( cast_state_t *cs, char *params ) {
	int weapon;
	int i;

	weapon = WP_NONE;

	for ( i = 1; bg_itemlist[i].classname; i++ ) {
		//----(SA)	first try the name they see in the editor, then the pickup name
		if ( !Q_strcasecmp( params, bg_itemlist[i].classname ) ) {
			weapon = bg_itemlist[i].giTag;
			break;
		}

		if ( !Q_strcasecmp( params, bg_itemlist[i].pickup_name ) ) {
			weapon = bg_itemlist[i].giTag;
			break;
		}
	}

	if ( weapon != WP_NONE ) {
		if ( cs->clientState ) {
			cs->weaponNum = weapon;
		}
		cs->script.castScriptStatus.scriptFlags |= SFL_NOCHANGEWEAPON;

		g_entities[cs->entityNum].client->ps.weapon = weapon;
		g_entities[cs->entityNum].client->ps.weaponstate = WEAPON_READY;

		if ( !cs->aiCharacter ) { // only do this for player
			//vmCvar_t cvar;
			//
			g_entities[cs->entityNum].client->ps.weaponTime =
				750; // (SA) HACK: FIXME: TODO: delay to catch initial weapon reload
			// tell it which weapon to use after spawning in
			//trap_Cvar_Register( &cvar, "cg_loadWeaponSelect", "0", CVAR_ROM );
			//trap_Cvar_Set( "cg_loadWeaponSelect", va("%i", g_entities[cs->entityNum].client->ps.weapon ) );
		}

	} else {
		//		G_Printf( "--SCRIPTER WARNING-- AI Scripting: selectweapon: unknown weapon \"%s\"", params );
		//		return qfalse;	// (SA) temp as scripts transition to new names
		G_Error( "AI Scripting: selectweapon: unknown weapon \"%s\"", params );
	}

	return qtrue;
};

//----(SA)	added
/*
==============
AICast_ScriptAction_GiveArmor
	syntax: setarmor <amount>

==============
*/
qboolean AICast_ScriptAction_SetArmor( cast_state_t *cs, char *params ) {
	if ( !params || !params[0] ) {
		G_Error( "AI Scripting: setarmor requires an armor value" );
	}

	g_entities[cs->entityNum].client->ps.stats[STAT_ARMOR] += atoi( params );

	return qtrue;
}

/*
==============
AICast_ScriptAction_GiveArmor

	syntax: givearmor <pickupname>

	will probably be more like:
		syntax: givearmor <type> <amount>
==============
*/
qboolean AICast_ScriptAction_GiveArmor( cast_state_t *cs, char *params ) {
	int i;
	gitem_t *item = 0;

	for ( i = 1; bg_itemlist[i].classname; i++ ) {
		//----(SA)	first try the name they see in the editor, then the pickup name
		if ( !Q_strcasecmp( params, bg_itemlist[i].classname ) ) {
			item = &bg_itemlist[i];
		}

		if ( !Q_strcasecmp( params, bg_itemlist[i].pickup_name ) ) {
			item = &bg_itemlist[i];
		}
	}

	if ( !item ) { // item not found
		G_Error( "AI Scripting: givearmor%s, unknown item", params );
	}

	if ( item->giType == IT_ARMOR ) {
		g_entities[cs->entityNum].client->ps.stats[STAT_ARMOR] += item->quantity;
		if ( g_entities[cs->entityNum].client->ps.stats[STAT_ARMOR] > 100 ) {
			g_entities[cs->entityNum].client->ps.stats[STAT_ARMOR] = 100;
		}
	}

	return qtrue;
}

//----(SA)	end

/*
=================
AICast_ScriptAction_GiveWeapon

  syntax: giveweapon <pickupname>
=================
*/
qboolean AICast_ScriptAction_GiveWeapon( cast_state_t *cs, char *params ) {
	int weapon;
	int i;
	gentity_t *ent = &g_entities[cs->entityNum];

	weapon = WP_NONE;

	for ( i = 1; bg_itemlist[i].classname; i++ ) {
		//----(SA)	first try the name they see in the editor, then the pickup name
		if ( !Q_strcasecmp( params, bg_itemlist[i].classname ) ) {
			weapon = bg_itemlist[i].giTag;
			break;
		}

		if ( !Q_strcasecmp( params, bg_itemlist[i].pickup_name ) ) {
			weapon = bg_itemlist[i].giTag;
		}
	}

	if ( weapon == WP_COLT ) {
		// if you had the colt already, now you've got two!
		if ( COM_BitCheck( g_entities[cs->entityNum].client->ps.weapons, WP_COLT ) ) {
			weapon = WP_AKIMBO;
		}
	}

	if ( weapon != WP_NONE ) {
		COM_BitSet( g_entities[cs->entityNum].client->ps.weapons, weapon );

		//----(SA)	some weapons always go together (and they share a clip, so this is okay)
		if ( weapon == WP_GARAND ) {
			COM_BitSet( g_entities[cs->entityNum].client->ps.weapons, WP_SNOOPERSCOPE );
		}
		if ( weapon == WP_SNOOPERSCOPE ) {
			COM_BitSet( g_entities[cs->entityNum].client->ps.weapons, WP_GARAND );
		}
		if ( weapon == WP_FG42 ) {
			COM_BitSet( g_entities[cs->entityNum].client->ps.weapons, WP_FG42SCOPE );
		}
		if ( weapon == WP_SNIPERRIFLE ) {
			COM_BitSet( g_entities[cs->entityNum].client->ps.weapons, WP_MAUSER );
		}
		//----(SA)	end

		// monsters have full ammo for their attacks
		// knife gets infinite ammo too
		if ( !Q_strncasecmp( params, "monsterattack", 13 ) || weapon == WP_KNIFE ) {
			g_entities[cs->entityNum].client->ps.ammo[BG_FindAmmoForWeapon( weapon )] = 999;
			Fill_Clip( &g_entities[cs->entityNum].client->ps, weapon ); //----(SA)	added
		}
		// conditional flags
		if ( ent->aiCharacter == AICHAR_ZOMBIE ) {
			if ( COM_BitCheck( ent->client->ps.weapons, WP_MONSTER_ATTACK1 ) ) {
				cs->aiFlags |= AIFL_NO_FLAME_DAMAGE;
				SET_FLAMING_ZOMBIE( ent->s, 1 );
			}
		}
	} else {
		G_Error( "AI Scripting: giveweapon %s, unknown weapon", params );
	}

	return qtrue;
};

/*
=================
AICast_ScriptAction_TakeWeapon

  syntax: takeweapon <pickupname>
=================
*/
qboolean AICast_ScriptAction_TakeWeapon( cast_state_t *cs, char *params ) {
	int weapon;
	int i;

	weapon = WP_NONE;

	if ( !Q_stricmp( params, "all" ) ) {

		// clear out all weapons
		memset( g_entities[cs->entityNum].client->ps.weapons, 0,
				sizeof( g_entities[cs->entityNum].client->ps.weapons ) );
		memset( g_entities[cs->entityNum].client->ps.ammo, 0, sizeof( g_entities[cs->entityNum].client->ps.ammo ) );
		memset( g_entities[cs->entityNum].client->ps.ammoclip, 0,
				sizeof( g_entities[cs->entityNum].client->ps.ammoclip ) );
		cs->weaponNum = WP_NONE;

	} else {

		for ( i = 1; bg_itemlist[i].classname; i++ ) {
			//----(SA)	first try the name they see in the editor, then the pickup name
			if ( !Q_strcasecmp( params, bg_itemlist[i].classname ) ) {
				weapon = bg_itemlist[i].giTag;
				break;
			}

			if ( !Q_strcasecmp( params, bg_itemlist[i].pickup_name ) ) {
				weapon = bg_itemlist[i].giTag;
				break;
			}
		}

		if ( weapon != WP_NONE ) {
			qboolean clear;

			if ( weapon == WP_AKIMBO ) {
				// take both the colt /and/ the akimbo weapons when 'akimbo' is specified
				COM_BitClear( g_entities[cs->entityNum].client->ps.weapons, WP_COLT );
			} else if ( weapon == WP_COLT ) {
				// take 'akimbo' first if it's there, then take 'colt'
				if ( COM_BitCheck( g_entities[cs->entityNum].client->ps.weapons, WP_AKIMBO ) ) {
					weapon = WP_AKIMBO;
				}
			}

			//
			COM_BitClear( g_entities[cs->entityNum].client->ps.weapons, weapon );
			// also remove the ammo for this weapon
			// but first make sure we dont have any other weapons that use the same ammo
			clear = qtrue;
			for ( i = 0; i < WP_NUM_WEAPONS; i++ ) {
				if ( BG_FindAmmoForWeapon( weapon ) != BG_FindAmmoForWeapon( i ) ) {
					continue;
				}
				if ( COM_BitCheck( g_entities[cs->entityNum].client->ps.weapons, i ) ) {
					clear = qfalse;
				}
			}
			if ( clear ) {
				// (SA) temp only.  commented out for pistol guys in escape1
				//				g_entities[cs->entityNum].client->ps.ammo[BG_FindAmmoForWeapon(weapon)] = 0;
			}
		} else {
			G_Error( "AI Scripting: takeweapon %s, unknown weapon", params );
		}
	}

	if ( !g_entities[cs->entityNum].client->ps.weapons ) {
		if ( cs->clientState ) {
			cs->weaponNum = WP_NONE;
		} else {
			g_entities[cs->entityNum].client->ps.weapon = WP_NONE;
		}
	}

	return qtrue;
};

//----(SA)	added

/*
==============
AICast_ScriptAction_GiveInventory
==============
*/
qboolean AICast_ScriptAction_GiveInventory( cast_state_t *cs, char *params ) {
	int i;
	gitem_t *item = 0;

	for ( i = 1; bg_itemlist[i].classname; i++ ) {
		//----(SA)	first try the name they see in the editor, then the pickup name
		if ( !Q_strcasecmp( params, bg_itemlist[i].classname ) ) {
			item = &bg_itemlist[i];
		}

		if ( !Q_strcasecmp( params, bg_itemlist[i].pickup_name ) ) {
			item = &bg_itemlist[i];
		}
	}

	if ( !item ) { // item not found
		G_Error( "AI Scripting: giveinventory %s, unknown item", params );
	}

	if ( item->giType == IT_KEY ) {
		g_entities[cs->entityNum].client->ps.stats[STAT_KEYS] |= ( 1 << item->giTag );
	} else if ( item->giType == IT_HOLDABLE ) {
		// (SA) TODO
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_FireAtTarget

  syntax: fireattarget <targetname> [duration]
=================
*/
qboolean AICast_ScriptAction_FireAtTarget( cast_state_t *cs, char *params ) {
	gentity_t *ent;
	vec3_t vec, org, src;
	char *pString, *token;
	float diff;
	int i;

	pString = params;

	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		G_Error( "AI Scripting: fireattarget without a targetname\n" );
	}

	if ( !cs->clientState ) {
		G_Error( "AI Scripting: fireattarget called for non-AI character\n" );
	}

	// find this targetname
	ent = G_Find( NULL, FOFS( targetname ), token );
	if ( !ent ) {
		ent = AICast_FindEntityForName( token );
		if ( !ent ) {
			G_Error( "AI Scripting: fireattarget cannot find targetname/aiName \"%s\"\n", token );
		}
	}

	// if this is our first call for this fireattarget, record the ammo count
	if ( cs->script.castScriptStatus.scriptFlags & SFL_FIRST_CALL ) {
		cs->combat.lastWeaponFired = 0;
	}
	// make sure we don't move or shoot while turning to our target
	if ( cs->script.castScriptStatus.scriptNoAttackTime < level.time ) {
		cs->script.castScriptStatus.scriptNoAttackTime = level.time + 500;
	}
	// dont reload prematurely
	cs->noReloadTime = level.time + 1000;
	// don't move while firing at all
	//if (cs->script.castScriptStatus.scriptNoMoveTime < level.time) {
	cs->script.castScriptStatus.scriptNoMoveTime = level.time + 500;
	//}
	// let us move our view, whether it looks bad or not
	cs->script.castScriptStatus.playAnimViewlockTime = 0;
	// set the view angle manually
	BG_EvaluateTrajectory( &ent->s.pos, level.time, org );
	VectorCopy( cs->clientState->origin, src );
	src[2] += cs->clientState->cur_ps.viewheight;
	VectorSubtract( org, src, vec );
	VectorNormalize( vec );
	vectoangles( vec, cs->ideal_viewangles );
	for ( i = 0; i < 2; i++ ) {
		diff = fabs( AI_AngleDifference( cs->clientState->cur_ps.viewangles[i], cs->ideal_viewangles[i] ) );
		if ( VectorCompare( vec3_origin, ent->s.pos.trDelta ) ) {
			if ( diff ) {
				return qfalse; // not facing yet
			}
		} else {
			if ( diff > 25 ) { // allow some slack when target is moving
				return qfalse;
			}
		}
	}

	// force fire
	trap_EA_Attack( cs->clientState->client );
	//
	cs->bFlags |= AI_FRAME_ATTACKED;
	//
	// if we haven't fired yet
	if ( !cs->combat.lastWeaponFired ) {
		return qfalse;
	}
	//
	// do we need to stay and fire for a duration?
	token = COM_ParseExt( &pString, qfalse );
	if ( !token[0] ) {
		return qtrue; // no need to wait around
	}
	// only return true if we've been firing for long enough
	return ( ( cs->script.castScriptStatus.castScriptStackChangeTime + atoi( token ) ) < level.time );
}

/*
=================
AICast_ScriptAction_GodMode

  syntax: godmode <on/off>
=================
*/
qboolean AICast_ScriptAction_GodMode( cast_state_t *cs, char *params ) {
	if ( !params || !params[0] ) {
		G_Error( "AI Scripting: godmode requires an on/off specifier\n" );
	}

	if ( !Q_stricmp( params, "on" ) ) {
		g_entities[cs->clientState->entitynum].flags |= FL_GODMODE;
	} else if ( !Q_stricmp( params, "off" ) ) {
		g_entities[cs->clientState->entitynum].flags &= ~FL_GODMODE;
	} else {
		G_Error( "AI Scripting: godmode requires an on/off specifier\n" );
	}

	return qtrue;
}

/*
=================
AICast_ScriptAction_NoAIDamage

  syntax: noaidamage <on/off>
=================
*/
qboolean AICast_ScriptAction_NoAIDamage( cast_state_t *cs, char *params ) {
	if ( !params || !params[0] ) {
		G_Error( "AI Scripting: noaidamage requires an on/off specifier\n" );
	}

	if ( !Q_stricmp( params, "on" ) ) {
		cs->script.castScriptStatus.scriptFlags |= SFL_NOAIDAMAGE;
	} else if ( !Q_stricmp( params, "off" ) ) {
		cs->script.castScriptStatus.scriptFlags &= ~SFL_NOAIDAMAGE;
	} else {
		G_Error( "AI Scripting: noaidamage requires an on/off specifier\n" );
	}
	return qtrue;
}

/*
=================
AICast_ScriptAction_LightningDamage
=================
*/
qboolean AICast_ScriptAction_LightningDamage( cast_state_t *cs, char *params ) {
	Q_strlwr( params );
	if ( !Q_stricmp( params, "on" ) ) {
		cs->aiFlags |= AIFL_ROLL_ANIM; // hijacking this since the player doesn't use it
	} else {
		cs->aiFlags &= ~AIFL_ROLL_ANIM;
	}
	return qtrue;
}

/*
==================
AICast_ScriptAction_CatchFire
==================
*/
qboolean AICast_ScriptAction_CatchFire( cast_state_t *cs, char *params ) {
	gentity_t *ent = &g_entities[cs->entityNum];
	//
	ent->s.onFireEnd = level.time + 99999; // make sure it goes for longer than they need to die
	ent->flameBurnEnt = ENTITYNUM_WORLD;
	// add to playerState for client-side effect
	ent->client->ps.onFireStart = level.time;
	//
	return qtrue;
}
