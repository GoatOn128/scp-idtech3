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

#ifndef ICEWOLF_AI_LOCAL_H
#define ICEWOLF_AI_LOCAL_H

#include "ai_actor.h"
#include "fsm/ai_ability.h"

#define AICAST_LEADERDIST_MAX 240
#define AICAST_LEADERDIST_MIN 64

#include "ai_awareness.h"
#include "fsm/ai_state_machine.h"
#include "fsm/ai_behavior_doors.h"
#include "ai_navigation.h"
#include "ai_movement.h"
#include "fsm/ai_behavior_idle.h"
#include "fsm/ai_behavior_investigate.h"
#include "fsm/ai_behavior_pursuit.h"
#include "fsm/ai_behavior_evasion.h"
#include "fsm/ai_behavior_cover.h"
#include "fsm/ai_behavior_grenades.h"
#include "fsm/ai_behavior_mounted.h"
#include "fsm/ai_combat.h"
#include "fsm/ai_monster_zombie.h"
#include "fsm/ai_monster_loper.h"
#include "fsm/ai_monster_stimsoldier.h"
#include "fsm/ai_monster_blackguard.h"
#include "fsm/ai_monster_reject.h"
#include "fsm/ai_monster_warzombie.h"
#include "fsm/ai_boss_helga.h"
#include "fsm/ai_boss_heinrich.h"
#include "ai_input.h"
#include "ai_scheduler.h"
#include "ai_targeting.h"
#include "ai_perception.h"
#include "ai_weapons.h"
#include "ai_danger.h"
#include "ai_events.h"
#include "ai_debug.h"
#include "ai_script.h"
#include "ai_script_entities.h"
#include "ai_system.h"
#include "ai_characters.h"
#include "ai_spawn.h"
#include "ai_script_actions_combat.h"
#include "ai_script_actions_movement.h"
#include "ai_script_actions_mission.h"
#include "ai_script_actions_animation.h"

extern cast_weapon_info_t weaponInfo;
void GibEntity( gentity_t *self, int killer );
void GibHead( gentity_t *self, int killer );

#endif
