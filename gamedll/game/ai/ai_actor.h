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

#ifndef ICEWOLF_AI_ACTOR_H
#define ICEWOLF_AI_ACTOR_H

#include "ai_engine_types.h"
#include "fsm/ai_ability.h"
#include "fsm/ai_fsm_types.h"

//===========================================================================
//
// Name:			ai_actor.h
// Function:		Wolfenstein AI Character Routines
// Programmer:		Ridah
// Tab Size:		4 (real tabs)
//===========================================================================

#include "ai_combat_tuning.h"
#include "ai_client_state.h"

//
// constants/defines
//
#define MAX_AIFUNCS 15 // if we go over this per frame, likely to have an infinite loop
//
#define SIGHT_PER_SEC 50 // do this many sight iterations per second
//
// Cast AI specific action flags (get translated into ucmd's
#define CASTACTION_WALK 1
//
#define MAX_SCRIPT_ACCUM_BUFFERS 8
//
#define AICAST_PRT_ALWAYS 0
#define AICAST_PRT_DEBUG 1
//
#define DEBUG_FOLLOW_DIST 96
//
#define MAX_LEADER_DIST 256
//
#define AASWORLD_STANDARD 0
#define AASWORLD_LARGE 1
//
// use this for returning the length of an anim
#define ANIMLENGTH( frames, fps ) ( ( frames * 1000 ) / fps )
//
#define AICAST_TFL_DEFAULT                                                                                             \
	TFL_DEFAULT & ~( TFL_JUMPPAD | TFL_ROCKETJUMP | TFL_BFGJUMP | TFL_GRAPPLEHOOK | TFL_DOUBLEJUMP | TFL_RAMPJUMP |    \
					 TFL_STRAFEJUMP | TFL_LAVA ) //----(SA)	modified since slime is no longer deadly
//#define AICAST_TFL_DEFAULT	TFL_DEFAULT & ~(TFL_JUMPPAD|TFL_ROCKETJUMP|TFL_BFGJUMP|TFL_GRAPPLEHOOK|TFL_DOUBLEJUMP|TFL_RAMPJUMP|TFL_STRAFEJUMP|TFL_SLIME|TFL_LAVA)
//
// AI flags
#define AIFL_CATCH_GRENADE 0x1
#define AIFL_NO_FLAME_DAMAGE 0x2
#define AIFL_FIRED 0x4
#define AIFL_LAND_ANIM_PLAYED 0x8
#define AIFL_ROLL_ANIM 0x10
#define AIFL_FLIP_ANIM 0x20
#define AIFL_STAND_IDLE2 0x40
#define AIFL_NOAVOID 0x80 // if set, this AI will ignore requests for us to move out the way
#define AIFL_NOPAIN 0x100 // don't stop for pain anims
#define AIFL_WALKFORWARD 0x200 // only walk forward
#define AIFL_DENYACTION 0x400 // used by scripting to prevent dynamic code from executing certain behaviour
#define AIFL_VIEWLOCKED 0x800 // prevent anything outside movement routines from changing view
#define AIFL_CORPSESIGHTING 0x1000 // share information through friendly corpses
#define AIFL_WAITINGTOSPAWN 0x2000 // waiting until space is clear to spawn in to the game
#define AIFL_JUST_SPAWNED 0x4000
#define AIFL_NO_RELOAD 0x8000 // this character doesn't need to reload
#define AIFL_TALKING 0x10000
#define AIFL_NO_HEADLOOK 0x20000
#define AIFL_ATTACK_CROUCH 0x40000
#define AIFL_MISCFLAG1 0x80000 // used various bits of code, temporarily
#define AIFL_MISCFLAG2 0x100000 // used various bits of code, temporarily
#define AIFL_ZOOMING 0x200000
#define AIFL_NO_HEADSHOT_DMG 0x400000
#define AIFL_DIVE_ANIM 0x800000 // able to dive to cover
#define AIFL_NO_TESLA_DAMAGE 0x1000000
#define AIFL_EXPLICIT_ROUTING 0x2000000 // direct routing towards ai_markers, rather than using AAS
#define AIFL_DISMOUNTING 0x4000000
#define AIFL_SPECIAL_FUNC 0x8000000 // prevent external interuption of current think func

//
// predict events
typedef enum { PREDICTSTOP_NONE, PREDICTSTOP_HITENT, PREDICTSTOP_HITCLIENT } predictStop_t;
//
typedef enum {
	AITEAM_NAZI,
	AITEAM_ALLIES,
	AITEAM_MONSTER,
	AITEAM_SPARE1,
	AITEAM_SPARE2,
	AITEAM_SPARE3,
	AITEAM_SPARE4,
	AITEAM_NEUTRAL
} AITeam_t;
//
typedef enum { BBOX_SMALL, BBOX_LARGE } BBoxType_t;
//
// attributes
// !!! NOTE: any changes to this must be reflected in the castAttributeStrings in ai_system.cpp
typedef enum {
	RUNNING_SPEED, // max = 300	(running speed)
	WALKING_SPEED, // max = 300	(walking speed)
	CROUCHING_SPEED, // max = 300	(crouching speed)
	FOV, // max = 360	(field of view)
	YAW_SPEED, // max = 300	(yaw speed, so we can make zombie's turn slowly)
	LEADER, // max = 1.0	(ability to lead an AI squadron)
	AIM_SKILL, // max = 1.0	(skill while aiming)
	AIM_ACCURACY, // max = 1.0	(accuracy of firing)
	ATTACK_SKILL, // max = 1.0	(ability to attack and do other things, like retreat)
	REACTION_TIME, // max = 1.0	(upon seeing enemy, wait this long before reaction)
	ATTACK_CROUCH, // max = 1.0	(likely to crouch while firing)
	IDLE_CROUCH, // max = 1.0	(likely to crouch while idling)
	AGGRESSION, // max = 1.0	(willingness to fight till the death)
	TACTICAL, // max = 1.0	(ability to use strategy to their advantage, also behaviour whilst hunting enemy, more likely to creep around)
	CAMPER, // max = 1.0	(set this to make them stay in the spot they are spawned)
	ALERTNESS, // max = 1.0	(ability to notice enemies at long range)
	STARTING_HEALTH, // MAX = 999	(starting health)
	HEARING_SCALE, // max = 999	(multiply default hearing ranges by this)
	HEARING_SCALE_NOT_PVS, // max = 999 (multiply hearing range by this if outside PVS)
	INNER_DETECTION_RADIUS, // default = 512	(enemies within this range trigger immediate combat mode
	PAIN_THRESHOLD_SCALE, // default = 1.0

	AICAST_MAX_ATTRIBUTES

} castAttributes_t;
//
typedef enum {
	SIGHTSOUNDSCRIPT,
	ATTACKSOUNDSCRIPT,
	ORDERSSOUNDSCRIPT,
	DEATHSOUNDSCRIPT,
	QUIETDEATHSOUNDSCRIPT, //----(SA)	ADDED FOR SILENT DEATHS (SNIPER/KNIFE)
	FLAMEDEATHSOUNDSCRIPT, //----(SA)	ADDED FOR FLAMING
	PAINSOUNDSCRIPT,

	STAYSOUNDSCRIPT,
	FOLLOWSOUNDSCRIPT,
	ORDERSDENYSOUNDSCRIPT,
	MISC1SOUNDSCRIPT,

	MAX_AI_EVENT_SOUNDS
} AIEventSounds_t;
//
typedef struct {
	char *name;
	float attributes[AICAST_MAX_ATTRIBUTES];

	char *soundScripts[MAX_AI_EVENT_SOUNDS];

	int aiTeam;
	char *skin;
	int weapons[8];
	int bboxType;
	vec2_t crouchstandZ;
	int aiFlags;

	const ai_ability_set_t *abilities;

	char *loopingSound; // play this sound constantly while alive

	aistateEnum_t aiState;
} AICharacterDefaults_t;

//
// script flags
#define SFL_NOCHANGEWEAPON 0x1
#define SFL_NOAIDAMAGE 0x2
#define SFL_FRIENDLYSIGHTCORPSE_TRIGGERED 0x4
#define SFL_WAITING_RESTORE 0x8
#define SFL_FIRST_CALL 0x10
//
// attributes strings (used for per-entity attribute definitions)
// NOTE: these must match the attributes above)
extern char *castAttributeStrings[];
extern AICharacterDefaults_t aiDefaults[NUM_CHARACTERS];
//
// structure defines
//
#define AIVIS_ENEMY 1
#define AIVIS_INSPECTED 2 // we have inspected them once already
#define AIVIS_INSPECT 4 // we should inspect them when we get a chance
#define AIVIS_PROCESS_SIGHTING 8 // so we know if we have or haven't processed the sighting since they were last seen
#define AIVIS_SIGHT_SCRIPT_CALLED 0x10 // set once sight script has been called.. only call once
//
// share range
#define AIVIS_SHARE_RANGE 384 // if we are within this range of a friendly, share their vis info
//
#define MAX_CHASE_MARKERS 3
#define CHASE_MARKER_INTERVAL 1000
//
#define COMBAT_TIMEOUT 8000
// sight info
typedef struct {
	int flags;
	int lastcheck_timestamp;
	int real_visible_timestamp;
	int real_update_timestamp;
	int real_notvisible_timestamp;
	int visible_timestamp; // time we last recorded a sighting
	vec3_t visible_pos; // position we last knew of them being at (could be hearing, etc)
	vec3_t real_visible_pos; // position we last physically saw them
	vec3_t visible_vel; // velocity during last sighting
	int notvisible_timestamp; // last time we didn't see the entity (used for reaction delay)
	vec3_t chase_marker[MAX_CHASE_MARKERS];
	int chase_marker_count;
	int lastcheck_health;
} cast_visibility_t;
//
// starting weapons, ammo, etc
typedef struct {
	int startingWeapons[MAX_WEAPONS / ( sizeof( int ) * 8 )];
	int startingAmmo[MAX_WEAPONS]; // starting ammo values for each weapon (set to 999 for unlimited)
} cast_weapon_info_t;
//
// scripting
typedef struct {
	char *actionString;
	qboolean ( *actionFunc )( struct cast_state_s *cs, char *params );
} cast_script_stack_action_t;
//
typedef struct {
	//
	// set during script parsing
	cast_script_stack_action_t *action; // points to an action to perform
	char *params;
} cast_script_stack_item_t;
//
#define AICAST_MAX_SCRIPT_STACK_ITEMS 64
//
typedef struct {
	cast_script_stack_item_t items[AICAST_MAX_SCRIPT_STACK_ITEMS];
	int numItems;
} cast_script_stack_t;
//
typedef struct {
	int eventNum; // index in scriptEvents[]
	char *params; // trigger targetname, etc
	cast_script_stack_t stack;
} cast_script_event_t;
//
typedef struct {
	char *eventStr;
	qboolean ( *eventMatch )( cast_script_event_t *event, char *eventParm );
} cast_script_event_define_t;
//
typedef struct {
	int castScriptStackHead, castScriptStackChangeTime;
	int castScriptEventIndex; // current event containing stack of actions to perform
	// scripting system AI variables (set by scripting system, used directly by AI)
	int scriptId; // incremented each time the script changes
	int scriptFlags;
	int scriptNoAttackTime;
	int scriptNoMoveTime;
	int playAnimViewlockTime;
	int scriptGotoEnt; // just goto them, then resume normal behaviour (don't follow)
	int scriptGotoId;
	vec3_t scriptWaitPos;
	int scriptWaitMovetime;
	vec3_t scriptWaitHidePos;
	int scriptWaitHideTime;
	int scriptNoSightTime;
	int scriptAttackEnt; // we should always attack this AI if they are alive
	vec3_t playanim_viewangles;
} cast_script_status_t;
//
typedef struct {
	aistateEnum_t currentState;
	aistateEnum_t nextState;
	int nextStateTimer; // time left until "newState" is reached
} aistate_t;
//
typedef enum { MS_DEFAULT, MS_WALK, MS_RUN, MS_CROUCH } movestate_t;
//
typedef enum { MSTYPE_NONE, MSTYPE_TEMPORARY, MSTYPE_PERMANENT } movestateType_t;
//
//
typedef struct aicast_checkattack_cache_s {
	int enemy;
	qboolean allowHitWorld;
	int time;
	int weapon;
	qboolean result;
} aicast_checkattack_cache_t;
//
// --------------------------------------------------------------------------------
// the main cast structure
// Ability state belongs to each actor and is serialized with it.
typedef struct {
	vec3_t stimFlyAttackPos;
	int animHitCount; // for stepping through the frames on which to inflict damage
	vec3_t loperLeapVel;
	AILoperLeapPhase loperLeapPhase;
	int loperPhaseStartedAt;
	int loperLegAnimation;
	qboolean meleeHitApplied;
} ai_ability_state_t;

// Perception state belongs to each actor and is serialized with it.
typedef struct {
	cast_visibility_t vislist[MAX_CLIENTS]; // array of all other client entities, allocated at level start-up
	int queryStartTime, queryCountValidTime, queryCount, queryAlertSightTime;
	int lastScriptSound;
	int bulletImpactEntity;
	int bulletImpactTime; // last time we heard/saw a bullet impact
	int bulletImpactIgnoreTime;
	vec3_t bulletImpactStart, bulletImpactEnd;
	int audibleEventTime;
	vec3_t audibleEventOrg;
	int audibleEventEnt;
} ai_perception_state_t;

// Script state belongs to each actor and is serialized with it.
typedef struct {
	int numCastScriptEvents;
	cast_script_event_t *castScriptEvents; // contains a list of actions to perform for each event type
	cast_script_status_t castScriptStatus; // current status of scripting
	cast_script_status_t castScriptStatusCurrent; // scripting status to use for backups
	cast_script_status_t
		castScriptStatusBackup; // perm backup of status of scripting, only used by backup and restore commands
	int scriptCallIndex; // inc'd each time a script is called
	int scriptAnimTime, scriptAnimNum; // last time an anim was played using scripting
	int scriptAccumBuffer[MAX_SCRIPT_ACCUM_BUFFERS];
	int scriptPauseTime;
} ai_script_state_t;

// Navigation state belongs to each actor and is serialized with it.
typedef struct {
	int followEntity;
	float followDist;
	qboolean
		followIsGoto; // we are really just going to the entity, but should wait until scripting tells us we can stop
	int followTime; // if this runs out, the scripting has probably been interupted
	qboolean followSlowApproach;
	float combatGoalTime;
	vec3_t combatGoalOrigin;
	int blockedTime; // time they were last blocked by a solid entity
	int obstructingTime; // time that we should move so we are not obstructing someone else
	vec3_t obstructingPos;
	int blockedAvoidTime;
	float blockedAvoidYaw;
	int doorMarker, doorEntNum;
	int dangerLastGetAvoid;
	int lastAvoid;
	int doorMarkerTime, doorMarkerNum, doorMarkerDoor;
	vec3_t lastMoveToPosGoalOrg; // if this changes, we should reset the Bot Avoid Reach
	int lastValidAreaNum[2]; // last valid area within each AAS world
	int lastValidAreaTime[2]; // time we last got the area
	bot_moveresult_t lastMoveResult;
} ai_navigation_state_t;

// Combat state belongs to each actor and is serialized with it.
typedef struct {
	int weaponFireTimes[MAX_WEAPONS];
	int lastGetHidePos;
	int startAttackCount; // incremented each time we start a standing attack
	int combatSpotAttackCount;
	int combatSpotDelayTime;
	int startBattleChaseTime;
	int enemyHeight;
	int enemyDist;
	vec3_t takeCoverPos, takeCoverEnemyPos;
	int takeCoverTime;
	int attackSpotTime;
	int triggerReleaseTime;
	int lastWeaponFired; // set each time a weapon is fired. used to detect when a weapon has been fired from within scripting
	vec3_t lastWeaponFiredPos;
	int lastWeaponFiredWeaponNum;
	aicast_checkattack_cache_t checkAttackCache;
	int noAttackTime; // used by dynamic AI to stop attacking for set time
	int battleChaseMarker, battleChaseMarkerDir;
	int lastBattleHunted; // last time an enemy decided to hunt us
	int battleHuntPauseTime, battleHuntViewTime;
} ai_combat_state_t;

// Grenade state belongs to each actor and is serialized with it.
typedef struct {
	int startGrenadeFlushTime;
	int grenadeFlushEndTime;
	int grenadeFlushFiring;
	qboolean grenadeGrabFlag; // if this is set, we need to play the anim before we can grab it
	int grenadeKickWeapon;
} ai_grenade_state_t;

typedef struct cast_state_s {
	ai_ability_state_t ability;
	ai_perception_state_t perception;
	ai_script_state_t script;
	ai_navigation_state_t navigation;
	ai_combat_state_t combat;
	ai_grenade_state_t grenade;

	ai_client_state_t *clientState;
	int entityNum;

	int aasWorldIndex; // set this according to our bounding box type

	int aiCharacter;
	int aiFlags;
	int lastThink; // time they last thinked, so we can vary the think times
	int actionFlags; // cast AI specific movement flags
	int lastPain, lastPainDamage;
	int travelflags;
	int thinkFuncChangeTime;

	aistateEnum_t aiState;
	movestate_t movestate; // walk, run, crouch etc (can be specified in a script)
	movestateType_t movestateType; // temporary, permanent, etc

	float attributes[AICAST_MAX_ATTRIBUTES];
	cast_weapon_info_t *weaponInfo; // FIXME: make this a list, so they can have multiple weapons?

	ai_fsm_state_t fsm;
	ai_state_callback_t currentState;
	ai_state_callback_t previousState;

	void ( *painfunc )( gentity_t *ent, gentity_t *attacker, int damage, vec3_t point );
	void ( *deathfunc )( gentity_t *ent, gentity_t *attacker, int damage, int mod ); //----(SA)	added mod
	void ( *sightfunc )( gentity_t *ent, gentity_t *other, int lastSight );
	void ( *sightEnemy )( gentity_t *ent, gentity_t *other );
	void ( *sightFriend )( gentity_t *ent, gentity_t *other );

	void ( *activate )( int entNum, int activatorNum );

	int leaderNum; // entnum of player we are following

	float speedScale; // so we can vary movement speed

	int deathTime;
	int rebirthTime, revivingTime;
	int lastEnemy, nextIdleAngleChange;
	float idleYawChange, idleYaw;

	qboolean crouchHideFlag;
	int attackSNDtime;
	int attacksnd;
	int painSoundTime;
	int firstSightTime;
	qboolean secondDeadTime;

	int lockViewAnglesTime;

	int dangerEntity;
	int dangerEntityValidTime; // dangerEntity is valid until this time expires
	vec3_t dangerEntityPos; // dangerEntity is predicted to end up here
	int dangerEntityTimestamp; // time this danger was recorded
	float dangerDist;

	int mountedEntity; // mg42, etc that we have mounted
	int inspectBodyTime;
	vec3_t startOrigin;

	int damageQuota;
	int damageQuotaTime;

	int pauseTime; // absolutely don't move move while this is > level.time

	int secretsFound;

	int attempts;

	int lastRollMove;
	int lastFlipMove;

	int lastDodgeRoll; // last time we rolled to get out of our enemies direct aim
	int battleRollTime;

	vec3_t viewlock_viewangles;

	int totalPlayTime, lastLoadTime;

	int inspectNum;

	int lastAttackCrouch;

	int lastMoveThink; // last time we ran our ClientThink()

	int numEnemies; // last count of enemies that are currently pursuing us

	int noReloadTime; // dont reload prematurely until this time has expired

	int weaponNum; // our current weapon
	int enemyNum; // our current enemy
	vec3_t ideal_viewangles, viewangles;
	usercmd_t lastucmd;
	int attackcrouch_time;
	int bFlags;

	int deadSinkStartTime;

	int lastActivate;

} cast_state_t;
//
#define CSFOFS( x ) ( (int)&( ( (cast_state_t *)0 )->x ) )
//
typedef struct aicast_predictmove_s {
	vec3_t endpos; //position at the end of movement prediction
	vec3_t velocity; //velocity at the end of movement prediction
	int presencetype; //presence type at end of movement prediction
	int stopevent; //event that made the prediction stop
	float time; //time predicted ahead
	int frames; //number of frames predicted ahead
	int numtouch;
	int touchents[MAXTOUCH];
	int groundEntityNum;
} aicast_predictmove_t;
//
// variables/globals
//
//cast states
extern cast_state_t *caststates;
//number of characters
extern int numcast;
//
// minimum time between thinks (maximum is double this)
extern int aicast_thinktime;
// maximum number of character thinks at once
extern int aicast_maxthink;
// maximum clients
extern int aicast_maxclients;
// skill scale
extern float aicast_skillscale;
//
// cvar to enable aicast debugging, set higher for more levels of debugging
extern vmCvar_t aicast_debug;
extern vmCvar_t aicast_debugname;
extern vmCvar_t aicast_scripts;
//
//

// Per-actor update entry points; frame scheduling and input execution are separate.
void AICast_Think( int client, float thinktime );
void AICast_QueryThink( cast_state_t *cs );
void AICast_IdleReload( cast_state_t *cs );

int AI_GetClientState( int clientNum, playerState_t *state );

#endif
