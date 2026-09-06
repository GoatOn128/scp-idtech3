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

#ifndef ICEWOLF_AI_WEAPONS_H
#define ICEWOLF_AI_WEAPONS_H

#include "ai_actor.h"

float AICast_WeaponRange( cast_state_t *cs, int weaponnum );
qboolean AICast_CheckAttack_real( cast_state_t *cs, int enemy, qboolean allowHitWorld );
qboolean AICast_CheckAttackAtPos( int entnum, int enemy, vec3_t pos, qboolean ducking, qboolean allowHitWorld );
qboolean AICast_CheckAttack( cast_state_t *cs, int enemy, qboolean allowHitWorld );
float AICast_WeaponWantScale( cast_state_t *cs, int weapon );
qboolean AICast_GotEnoughAmmoForWeapon( cast_state_t *cs, int weapon );
qboolean AICast_WeaponUsable( cast_state_t *cs, int weaponNum );
void AICast_ChooseWeapon( cast_state_t *cs, qboolean battleFunc );
void AICast_WeaponSway( cast_state_t *cs, vec3_t ofs );
qboolean AICast_AimAtEnemy( cast_state_t *cs );
qboolean AICast_CanMoveWhileFiringWeapon( int weaponnum );
qboolean AICast_RandomTriggerRelease( cast_state_t *cs );
void AICast_ProcessAttack( cast_state_t *cs );
void AICast_RecordWeaponFire( gentity_t *ent );
qboolean AICast_StopAndAttack( cast_state_t *cs );
float AICast_GetAccuracy( int entnum );
qboolean AICast_HasFiredWeapon( int entNum, int weapon );

#endif
