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

#ifndef ICEWOLF_AI_CLIENT_STATE_H
#define ICEWOLF_AI_CLIENT_STATE_H

#include "ai_engine_types.h"

// Campaign actors use player physics and a navigation movement handle.
// Arena inventory, chat, team goals and decision nodes do not belong here.
struct ai_client_state_t {
	int inuse;
	int client;
	int entitynum;
	playerState_t cur_ps;
	float thinktime;
	vec3_t origin;
	vec3_t velocity;
	vec3_t eye;
	int areanum;
	int ms;
};

#define AI_FRAME_ATTACKED 2

#endif
