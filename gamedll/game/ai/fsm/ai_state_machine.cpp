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

#include "../ai_local.h"

// State metadata is immutable; actors store IDs and timers, never descriptor pointers.
struct AIStateDefinition {
	AIStateId id;
	AIStateId parent;
	const char *name;
	ai_state_callback_t update;
	ai_state_callback_t enter;
	void ( *exit )( cast_state_t *cs );
	bool ( *canEnter )( const cast_state_t *cs );
};

static const AIStateDefinition stateDefinitions[] = {
	{ AIStateId::None, AIStateId::None, "None", NULL, NULL, NULL, NULL },
	{ AIStateId::Legacy, AIStateId::None, "Legacy", NULL, NULL, NULL, NULL },
	{ AIStateId::Idle, AIStateId::None, "Idle", AIFunc_Idle, AI_EnterIdle, NULL, NULL },
	{ AIStateId::Combat, AIStateId::None, "Combat", AIFunc_Battle, AI_EnterCombat, NULL, AI_HasLivingTarget },
	{ AIStateId::Chase, AIStateId::Combat, "Chase", AIFunc_BattleChase, AI_EnterChase, NULL, NULL },
	{ AIStateId::LoperLeap, AIStateId::Combat, "LoperLeap", AIFunc_LoperLeap, AI_EnterLoperLeap, AI_ExitLoperLeap,
	  AI_CanStartLoperLeap } };

static const AIStateDefinition *AI_StateDefinitionFor( AIStateId state ) {
	for ( const AIStateDefinition &definition : stateDefinitions ) {
		if ( definition.id == state ) {
			return &definition;
		}
	}
	return NULL;
}

AIStateId AI_StateParent( AIStateId state ) {
	const AIStateDefinition *definition = AI_StateDefinitionFor( state );
	return definition ? definition->parent : AIStateId::None;
}

const char *AI_StateName( AIStateId state ) {
	const AIStateDefinition *definition = AI_StateDefinitionFor( state );
	return definition ? definition->name : "Invalid";
}

bool AI_HasLivingTarget( const cast_state_t *cs ) {
	return cs && cs->enemyNum >= 0 && cs->enemyNum < MAX_CLIENTS && g_entities[cs->enemyNum].inuse &&
		   g_entities[cs->enemyNum].client && g_entities[cs->enemyNum].health > 0;
}

static void AI_ExitState( cast_state_t *cs ) {
	const AIStateDefinition *definition = AI_StateDefinitionFor( cs->fsm.state );
	if ( definition && definition->exit ) {
		definition->exit( cs );
	}
}

void AI_CancelState( cast_state_t *cs, AITransitionReason reason ) {
	if ( !cs ) {
		return;
	}
	AI_ExitState( cs );
	cs->fsm.previous = cs->fsm.state;
	cs->fsm.state = AIStateId::None;
	cs->fsm.pending = AIStateId::None;
	cs->fsm.reason = reason;
	cs->fsm.enteredAt = level.time;
	cs->fsm.transitions++;
	cs->currentState = NULL;
	cs->previousState = NULL;
}

void AI_SetLegacyState( cast_state_t *cs, ai_state_callback_t callback ) {
	if ( cs->currentState == callback ) {
		if ( !cs->fsm.committing ) {
			cs->fsm.enteredAt = cs->thinkFuncChangeTime = level.time;
			cs->fsm.transitions++;
		}
		return;
	}
	AI_ExitState( cs );
	cs->fsm.previous = cs->fsm.state;
	cs->fsm.state = callback ? AIStateId::Legacy : AIStateId::None;
	for ( const AIStateDefinition &definition : stateDefinitions ) {
		if ( callback && definition.update == callback ) {
			cs->fsm.state = definition.id;
			break;
		}
	}
	cs->fsm.pending = AIStateId::None;
	cs->fsm.reason = AITransitionReason::Legacy;
	cs->fsm.enteredAt = cs->thinkFuncChangeTime = level.time;
	cs->fsm.transitions++;
	cs->currentState = callback;
}

static bool AI_CanEnterState( const cast_state_t *cs, AIStateId state ) {
	// Child states inherit eligibility from their ancestors.
	const AIStateDefinition *definition = AI_StateDefinitionFor( state );
	while ( definition && definition->id != AIStateId::None ) {
		if ( definition->canEnter && !definition->canEnter( cs ) ) {
			return false;
		}
		definition = AI_StateDefinitionFor( definition->parent );
	}
	return definition != NULL;
}

char *AI_RequestState( cast_state_t *cs, AIStateId state, AITransitionReason reason ) {
	const AIStateDefinition *definition = AI_StateDefinitionFor( state );
	if ( !cs || !cs->clientState || !definition || !definition->update ) {
		return NULL;
	}
	cs->fsm.lastRequested = state;
	if ( !AI_CanEnterState( cs, state ) ) {
		cs->fsm.rejectedTransitions++;
		return NULL;
	}
	cs->fsm.pending = state;
	cs->fsm.pendingReason = reason;
	if ( !cs->fsm.updating && !cs->fsm.committing ) {
		const unsigned int transitions = cs->fsm.transitions;
		AI_CommitState( cs );
		if ( cs->fsm.transitions == transitions ) {
			return NULL;
		}
	}
	return const_cast<char *>( definition->name );
}

void AI_CommitState( cast_state_t *cs ) {
	if ( cs->fsm.committing ) {
		return;
	}
	cs->fsm.committing = true;
	// Entry may redirect (for example idle -> inspect body or combat -> ability).
	// Bound entry redirects; never execute an update here.
	for ( int redirects = 0; redirects < 8 && cs->fsm.pending != AIStateId::None; redirects++ ) {
		const AIStateId next = cs->fsm.pending;
		const AITransitionReason reason = cs->fsm.pendingReason;
		cs->fsm.pending = AIStateId::None;
		const AIStateDefinition *definition = AI_StateDefinitionFor( next );
		const unsigned int revision = cs->fsm.transitions;
		if ( !definition || !definition->update || !AI_CanEnterState( cs, next ) ||
			 ( next != AIStateId::Idle && !AICast_StateChange( cs, AISTATE_COMBAT ) ) ) {
			cs->fsm.rejectedTransitions++;
			continue;
		}
		// Script actions can replace or cancel behavior while evaluating approval.
		if ( cs->fsm.transitions != revision || cs->fsm.pending != AIStateId::None || !AI_CanEnterState( cs, next ) ) {
			cs->fsm.rejectedTransitions++;
			continue;
		}
		AI_ExitState( cs );
		cs->fsm.previous = cs->fsm.state;
		cs->fsm.state = next;
		cs->fsm.reason = reason;
		cs->fsm.enteredAt = cs->thinkFuncChangeTime = level.time;
		cs->fsm.transitions++;
		cs->currentState = definition->update;
		if ( definition->enter ) {
			char *entered = definition->enter( cs );
			// A monster with no usable attack must continue pursuing, not run gun combat.
			if ( !entered && next == AIStateId::Combat ) {
				AI_RequestState( cs, AIStateId::Chase, AITransitionReason::Decision );
			}
		}
	}
	if ( cs->fsm.pending != AIStateId::None ) {
		cs->fsm.pending = AIStateId::None;
		cs->fsm.rejectedTransitions++;
	}
	cs->fsm.committing = false;
}

/*
The lowest level of Cast AI thinking.
*/

/*
============
AICast_ProcessAIFunctions
============
*/
void AICast_ProcessAIFunctions( cast_state_t *cs, float thinktime ) {
	if ( !cs || !cs->clientState ) {
		return;
	}
	AICast_DBG_InitAIFuncs();
	if ( !cs->currentState ) {
		AIFunc_DefaultStart( cs );
		return; // Entry and update happen on separate thinks.
	}
	cs->fsm.updating = true;
	const AIStateDefinition *definition = AI_StateDefinitionFor( cs->fsm.state );
	ai_state_callback_t update = definition && definition->update ? definition->update : cs->currentState;
	char *transition = update( cs );
	cs->fsm.updating = false;
	AI_CommitState( cs );
	if ( transition ) {
		// Legacy callbacks still report transitions by name.
		trap_BotResetAvoidReach( cs->clientState->ms );
		AICast_DBG_AddAIFunc( cs, transition );
	}
}

/*
Support routines for the Decision Making layer.
*/

// Creature ability rules live in ai_monster_* and ai_boss_* modules.
