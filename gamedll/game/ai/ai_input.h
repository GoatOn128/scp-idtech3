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

#ifndef ICEWOLF_AI_INPUT_H
#define ICEWOLF_AI_INPUT_H

#include "ai_actor.h"

void AICast_ChangeViewAngles( cast_state_t *cs, float thinktime );
void AICast_InputToUserCommand( cast_state_t *cs, bot_input_t *bi, usercmd_t *ucmd, int delta_angles[3] );
void AICast_UpdateInput( cast_state_t *cs, int time );
void AICast_StartServerFrame( int time );

float AI_AngleDifference( float angle, float other );
float AI_ChangeViewAngle( float angle, float ideal, float speed );

#endif
