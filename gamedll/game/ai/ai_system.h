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

#ifndef ICEWOLF_AI_SYSTEM_H
#define ICEWOLF_AI_SYSTEM_H

#include "ai_actor.h"

// Deliberate encounter-wide attack staggering, reset at map initialization.
typedef struct {
	int lastSpiritAttackTime;
	int lastGrenadeFlushTime;
} ai_encounter_state_t;

extern ai_encounter_state_t ai_encounter;

void AICast_Printf( int type, const char *fmt, ... );
cast_state_t *AICast_GetCastState( int entitynum );
int AICast_SetupClient( int client );
int AICast_ShutdownClient( int client );
gentity_t *AICast_AddCastToGame( gentity_t *ent, char *castname, char *model, char *head, char *sex, char *color,
								 char *handicap );
void AICast_CheckLevelAttributes( cast_state_t *cs, gentity_t *ent, char **ppStr );
void AICast_SetAASIndex( cast_state_t *cs );
gentity_t *AICast_CreateCharacter( gentity_t *ent, float *attributes, cast_weapon_info_t *weaponInfo, char *castname,
								   char *model, char *head, char *sex, char *color, char *handicap );
void AICast_Init( void );
gentity_t *AICast_FindEntityForName( char *name );
gentity_t *AICast_TravEntityForName( gentity_t *startent, char *name );
void AIChar_AIScript_AlertEntity( gentity_t *ent );
void AICast_DelayedSpawnCast( gentity_t *ent, int castType );
void AICast_CastScriptThink( void );
void AICast_EnableRenderingThink( gentity_t *ent );
void AICast_CheckLoadGame( void );
qboolean AICast_SolidsInBBox( vec3_t pos, vec3_t mins, vec3_t maxs, int entnum, int mask );
void AICast_Activate( int activatorNum, int entNum );
qboolean AICast_NoFlameDamage( int entNum );
void AICast_SetFlameDamage( int entNum, qboolean status );
void G_SetAASBlockingEntity( gentity_t *ent, qboolean blocking );
void AICast_AdjustIdealYawForMover( int entnum, float yaw );
void AICast_AgePlayTime( int entnum );
int AICast_NoReload( int entnum );
int AICast_PlayTime( int entnum );
int AICast_NumAttempts( int entnum );
void AICast_RegisterPain( int entnum );

#endif
