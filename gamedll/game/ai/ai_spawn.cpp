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
============
AIChar_spawn
============
*/
void AIChar_spawn( gentity_t *ent ) {
	gentity_t *newent;
	cast_state_t *cs;
	AICharacterDefaults_t *aiCharDefaults;
	int i;
	static int lastCall;
	static int numCalls;

	// if there are other cast's waiting to spawn before us, wait for them
	for ( i = MAX_CLIENTS, newent = &g_entities[MAX_CLIENTS]; i < MAX_GENTITIES; i++, newent++ ) {
		if ( !newent->inuse ) {
			continue;
		}
		if ( newent->think != AIChar_spawn ) {
			continue;
		}
		if ( newent == ent ) {
			break; // we are the first in line
		}
		// still waiting for someone else
		ent->nextthink = level.time + FRAMETIME;
		return;
	}

	// if the client hasn't connected yet, wait around
	if ( !AICast_FindEntityForName( "player" ) ) {
		ent->nextthink = level.time + FRAMETIME;
		return;
	}

	if ( lastCall == level.time ) {
		if ( numCalls++ > 2 ) {
			ent->nextthink = level.time + FRAMETIME;
			return; // spawned enough this frame already
		}
	} else {
		numCalls = 0;
	}
	lastCall = level.time;

	aiCharDefaults = &aiDefaults[ent->aiCharacter];

	// ............................
	// setup weapon info
	//
	// starting weapons/ammo
	memset( &weaponInfo, 0, sizeof( weaponInfo ) );
	for ( i = 0; aiCharDefaults->weapons[i]; i++ ) {
		//weaponInfo.startingWeapons[(aiCharDefaults->weapons[i] / 32)] |= ( 1 << aiCharDefaults->weapons[i] );
		//weaponInfo.startingWeapons[0] |= ( 1 << aiCharDefaults->weapons[i] );

		COM_BitSet( weaponInfo.startingWeapons, aiCharDefaults->weapons[i] );
		if ( aiCharDefaults->weapons[i] ==
			 WP_GRENADE_LAUNCHER ) { // give them a bunch of grenades, but not an unlimited supply
			weaponInfo.startingAmmo[BG_FindAmmoForWeapon( aiCharDefaults->weapons[i] )] = 6;
		} else {
			weaponInfo.startingAmmo[BG_FindAmmoForWeapon( aiCharDefaults->weapons[i] )] = 999;
		}
	}
	//
	// use the default skin if nothing specified
	if ( !ent->aiSkin || !strlen( ent->aiSkin ) ) {
		ent->aiSkin = aiCharDefaults->skin;
	}
	// ............................
	//
	// create the character

	// (there will always be an ent->aiSkin (SA))
	newent = AICast_CreateCharacter( ent, aiCharDefaults->attributes, &weaponInfo, aiCharDefaults->name, ent->aiSkin,
									 ent->aihSkin, "m", "7", "100" );

	if ( !newent ) {
		G_FreeEntity( ent );
		return;
	}
	// copy any character-specific information to the new entity (like editor fields, etc)
	//
	// copy this across so killing ai can trigger a target
	newent->target = ent->target;
	//
	newent->classname = ent->classname;
	newent->r.svFlags |= ( ent->r.svFlags & SVF_NOFOOTSTEPS );
	newent->aiCharacter = ent->aiCharacter;
	newent->client->ps.aiChar = ent->aiCharacter;
	newent->spawnflags = ent->spawnflags;
	newent->aiTeam = ent->aiTeam;
	if ( newent->aiTeam < 0 ) {
		newent->aiTeam = aiCharDefaults->aiTeam;
	}
	newent->client->ps.teamNum = newent->aiTeam;
	//
	// kill the old entity
	G_FreeEntity( ent );
	// attach to the new entity
	ent = newent;
	//
	// precache any specific sounds
	//
	// ...
	//
	// get the cast state
	cs = AICast_GetCastState( ent->s.number );
	//
	// setup any character specific cast_state variables
	cs->deathfunc = AIChar_Death;
	cs->painfunc = AIChar_Pain;
	cs->aiFlags |= aiCharDefaults->aiFlags;
	cs->aiState = aiCharDefaults->aiState;
	//
	cs->perception.queryCountValidTime = -1;
	//
	// randomly choose idle animation
	if ( cs->aiFlags & AIFL_STAND_IDLE2 ) {
		newent->client->ps.eFlags |= EF_STAND_IDLE2;
	}
	//
	// attach any event specific functions (pain, death, etc)
	//
	//cs->getDeathAnim = AIChar_getDeathAnim;
	cs->sightfunc = AIChar_Sight;
	if ( ent->aiTeam == AITEAM_ALLIES || ent->aiTeam == AITEAM_NEUTRAL ) { // friendly
		cs->activate = AICast_ProcessActivate;
	} else {
		cs->activate = NULL;
	}
	//
	// looping sound?
	if ( aiCharDefaults->loopingSound ) {
		ent->s.loopSound = G_SoundIndex( aiCharDefaults->loopingSound );
	}
	//
	// precache sounds for this character
	for ( i = 0; i < MAX_AI_EVENT_SOUNDS; i++ ) {
		if ( aiDefaults[ent->aiCharacter].soundScripts[i] ) {
			G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[i] );
		}
	}
	//
	if ( ent->aiCharacter == AICHAR_HEINRICH ) {
		AICast_Heinrich_SoundPrecache();
	}
	//
	// special spawnflag stuff
	if ( ent->spawnflags & 2 ) {
		cs->secondDeadTime = qtrue;
	}
	//
	// init scripting
	cs->script.castScriptStatus.castScriptEventIndex = -1;
	cs->script.castScriptStatus.scriptAttackEnt = -1;
	//
	// set crouch move speed
	ent->client->ps.crouchSpeedScale = cs->attributes[CROUCHING_SPEED] / cs->attributes[RUNNING_SPEED];
	//
	// check for some anims which we can use for special behaviours
	if ( BG_GetAnimScriptEvent( &ent->client->ps, ANIM_ET_ROLL ) >= 0 ) {
		cs->aiFlags |= AIFL_ROLL_ANIM;
	}
	if ( BG_GetAnimScriptEvent( &ent->client->ps, ANIM_ET_FLIP ) >= 0 ) {
		cs->aiFlags |= AIFL_FLIP_ANIM;
	}
	if ( BG_GetAnimScriptEvent( &ent->client->ps, ANIM_ET_DIVE ) >= 0 ) {
		cs->aiFlags |= AIFL_DIVE_ANIM;
	}
	// HACK
	if ( ent->aiName && !Q_stricmp( ent->aiName, "deathshead" ) ) {
		cs->aiFlags |= AIFL_NO_FLAME_DAMAGE;
	}
	//
	// check for no headshot damage
	if ( cs->aiFlags & AIFL_NO_HEADSHOT_DMG ) {
		ent->headshotDamageScale = 0.0;
	}
	// set these values now so scripting system isn't relying on a Think having been run prior to running a script
	//origin of the cast
	VectorCopy( ent->client->ps.origin, cs->clientState->origin );
	//velocity of the cast
	VectorCopy( ent->client->ps.velocity, cs->clientState->velocity );
	//playerstate
	cs->clientState->cur_ps = ent->client->ps;
	//
	if ( !ent->aiInactive ) {
		// trigger a spawn script event
		AICast_ScriptEvent( cs, "spawn", "" );
	} else {
		trap_UnlinkEntity( ent );
	}
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_soldier (1 0.25 0) (-16 -16 -24) (16 16 64) TriggerSpawn NoRevive
soldier entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'infantryss/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/
/*
-------- MODEL FOR RADIANT ONLY - DO NOT SET THIS AS A KEY --------
model="models\mapobjects\characters\test\nazi.md3"
*/
/*
============
SP_ai_soldier
============
*/
void SP_ai_soldier( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_SOLDIER );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_american (1 0.25 0) (-16 -16 -24) (16 16 64) TriggerSpawn NoRevive
american entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'american/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_american
============
*/
void SP_ai_american( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_AMERICAN );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_zombie (1 0.25 0) (-16 -16 -24) (16 16 64) TriggerSpawn NoRevive PortalZombie
zombie entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'zombie/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_zombie
============
*/
void SP_ai_zombie( gentity_t *ent ) {
	ent->r.svFlags |= SVF_NOFOOTSTEPS;
	AICast_DelayedSpawnCast( ent, AICHAR_ZOMBIE );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_warzombie (1 0.25 0) (-16 -16 -24) (16 16 64) TriggerSpawn NoRevive PortalZombie
warrior zombie entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'warrior/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_warzombie
============
*/
void SP_ai_warzombie( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_WARZOMBIE );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_venom (1 0.25 0) (-16 -16 -24) (16 16 64) TriggerSpawn NoRevive
venom entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'venom/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_venom
============
*/
void SP_ai_venom( gentity_t *ent ) {
	ent->r.svFlags |= SVF_NOFOOTSTEPS;
	AICast_DelayedSpawnCast( ent, AICHAR_VENOM );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_loper (1 0.25 0) (-32 -32 -24) (32 32 48) TriggerSpawn NoRevive
loper entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'loper/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_loper
============
*/
void SP_ai_loper( gentity_t *ent ) {
	ent->r.svFlags |= SVF_NOFOOTSTEPS;
	AICast_DelayedSpawnCast( ent, AICHAR_LOPER );
	//
	level.loperZapSound = G_SoundIndex( "loperZap" );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_boss_helga (1 0.25 0) (-16 -16 -24) (16 16 64) TriggerSpawn NoRevive
helga entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'helga/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_boss_helga
============
*/
void SP_ai_boss_helga( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_HELGA );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_boss_heinrich (1 0.25 0) (-32 -32 -24) (32 32 156) TriggerSpawn NoRevive
heinrich entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'helga/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_boss_heinrich
============
*/
void SP_ai_boss_heinrich( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_HEINRICH );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_partisan (1 0.25 0) (-16 -16 -24) (16 16 64) TriggerSpawn NoRevive
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'partisan/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_partisan
============
*/
void SP_ai_partisan( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_PARTISAN );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_civilian (1 0.25 0) (-16 -16 -24) (16 16 64) TriggerSpawn NoRevive
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'civilian/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_civilian
============
*/
void SP_ai_civilian( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_CIVILIAN );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_eliteguard (1 0.25 0) (-16 -16 -24) (16 16 64) TriggerSpawn NoRevive
elite guard entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'eliteguard/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_eliteguard
============
*/
void SP_ai_eliteguard( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_ELITEGUARD );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_frogman (1 0.25 0) (-16 -16 -24) (16 16 64) TriggerSpawn NoRevive
elite guard entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'frogman/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_frogman
============
*/
void SP_ai_frogman( gentity_t *ent ) {
	ent->r.svFlags |= SVF_NOFOOTSTEPS;
	AICast_DelayedSpawnCast( ent, AICHAR_FROGMAN );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_stimsoldier_dual (1 0.25 0) (-32 -32 -24) (32 32 64) TriggerSpawn NoRevive
stim soldier entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'stim/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_stimsoldier_dual
============
*/
void SP_ai_stimsoldier_dual( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_STIMSOLDIER1 );
	//
	level.stimSoldierFlySound = G_SoundIndex( "sound/stimsoldier/flyloop.wav" );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_stimsoldier_rocket (1 0.25 0) (-32 -32 -24) (32 32 64) TriggerSpawn NoRevive
stim soldier entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'stim/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_stimsoldier_rocket
============
*/
void SP_ai_stimsoldier_rocket( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_STIMSOLDIER2 );
	//
	level.stimSoldierFlySound = G_SoundIndex( "sound/stimsoldier/flyloop.wav" );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_stimsoldier_tesla (1 0.25 0) (-32 -32 -24) (32 32 64) TriggerSpawn NoRevive
stim soldier entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'stim/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_stimsoldier_tesla
============
*/
void SP_ai_stimsoldier_tesla( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_STIMSOLDIER3 );
	//
	level.stimSoldierFlySound = G_SoundIndex( "sound/stimsoldier/flyloop.wav" );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_supersoldier (1 0.25 0) (-32 -32 -24) (32 32 64) TriggerSpawn NoRevive
supersoldier entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'supersoldier/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_supersoldier
============
*/
void SP_ai_supersoldier( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_SUPERSOLDIER );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_protosoldier (1 0.25 0) (-32 -32 -24) (32 32 64) TriggerSpawn NoRevive
protosoldier entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'protosoldier/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_protosoldier
============
*/
void SP_ai_protosoldier( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_PROTOSOLDIER );
}

//----------------------------------------------------------------------------------------------------------------------------
/*QUAKED ai_blackguard (1 0.25 0) (-16 -16 -24) (16 16 64) TriggerSpawn NoRevive
black guard entity
"skin" the .skin file to use for this character (must exist in the player characters directory, otherwise 'blackguard/default' is used)
"head" the .skin file to use for his head (must exist in the pc's dir, otherwise 'default' is used)
"ainame" name of AI
*/

/*
============
SP_ai_blackguard
============
*/
void SP_ai_blackguard( gentity_t *ent ) {
	AICast_DelayedSpawnCast( ent, AICHAR_BLACKGUARD );
}
