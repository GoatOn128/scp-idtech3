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

#ifndef ICEWOLF_AI_PERCEPTION_H
#define ICEWOLF_AI_PERCEPTION_H

#include "ai_actor.h"

qboolean AICast_EntityVisible( cast_state_t *cs, int enemynum, qboolean directview );
float AICast_GetWeaponSoundRange( int weapon );
void AICast_ProcessBullet( gentity_t *attacker, vec3_t start, vec3_t end );
void AICast_AudibleEvent( int srcnum, vec3_t pos, float range );
qboolean AICast_InFieldOfVision( vec3_t viewangles, float fov, vec3_t angles );
qboolean AICast_VisibleFromPos( vec3_t srcpos, int srcnum, vec3_t destpos, int destnum, qboolean updateVisPos );
qboolean AICast_CheckVisibility( gentity_t *srcent, gentity_t *destent );
void AICast_UpdateVisibility( gentity_t *srcent, gentity_t *destent, qboolean shareVis, qboolean directview );
void AICast_UpdateNonVisibility( gentity_t *srcent, gentity_t *destent, qboolean directview );
void AICast_SightUpdate( int numchecks );

#endif
