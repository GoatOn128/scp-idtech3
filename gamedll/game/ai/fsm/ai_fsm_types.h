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

#ifndef ICEWOLF_AI_FSM_TYPES_H
#define ICEWOLF_AI_FSM_TYPES_H

// Zero initialization produces an inactive machine. Legacy is an explicit migration boundary.
enum class AIStateId { None, Legacy, Idle, Combat, Chase, LoperLeap };
enum class AITransitionReason { None, Decision, Completed, TargetLost, Script, Death, Legacy };
enum class AILoperLeapPhase { None, FaceTarget, Airborne, Landing, Recovery };

struct ai_fsm_state_t {
	AIStateId state;
	AIStateId previous;
	AIStateId pending;
	AIStateId lastRequested;
	AITransitionReason reason;
	AITransitionReason pendingReason;
	int enteredAt;
	unsigned int transitions;
	unsigned int rejectedTransitions;
	bool updating;
	bool committing;
};

#endif
