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

AICharacterDefaults_t aiDefaults[NUM_CHARACTERS] = {

	//AICHAR_NONE
	{ 0 },

	//AICHAR_SOLDIER
	{ "Soldier",
	  {
		  220, // running speed
		  90, // walking speed
		  80, // crouching speed
		  90, // Field of View
		  200, // Yaw Speed	// RF change
		  0.0, // leader
		  0.5, // aim skill
		  0.5, // aim accuracy
		  0.75, // attack skill
		  0.5, // reaction time
		  0.4, // attack crouch
		  0.0, // idle crouch
		  0.5, // aggression
		  0.8, // tactical
		  0.0, // camper
		  16000, // alertness
		  100, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "infantrySightPlayer",
		  "infantryAttackPlayer",
		  "infantryOrders",
		  "infantryDeath",
		  "infantrySilentDeath", //----(SA)	added
		  "infantryFlameDeath", //----(SA)	added
		  "infantryPain",
		  "infantryStay", // stay - you're told to stay put
		  "infantryFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "infantryOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_NAZI, // team
	  "infantryss/default", // default model/skin
	  { WP_MP40, WP_GRENADE_LAUNCHER }, // starting weapons
	  BBOX_SMALL,
	  { 32, 48 }, // bbox, crouch/stand height
	  AIFL_CATCH_GRENADE | AIFL_STAND_IDLE2, // flags
	  NULL,
	  NULL, // looping sound
	  AISTATE_RELAXED },

	//AICHAR_AMERICAN
	{ "American",
	  {
		  220, // running speed
		  90, // walking speed
		  80, // crouching speed
		  90, // Field of View
		  200, // Yaw Speed	// RF change
		  0.0, // leader
		  0.70, // aim skill
		  0.70, // aim accuracy
		  0.75, // attack skill
		  0.5, // reaction time
		  0.3, // attack crouch
		  0.0, // idle crouch
		  0.5, // aggression
		  0.8, // tactical
		  0.0, // camper
		  16000, // alertness
		  100, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "americanSightPlayer",
		  "americanAttackPlayer",
		  "americanOrders",
		  "americanDeath",
		  "americanSilentDeath", //----(SA)	added
		  "americanFlameDeath", //----(SA)	added
		  "americanPain",
		  "americanStay", // stay - you're told to stay put
		  "americanFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "americanOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_ALLIES,
	  "american/default",
	  { WP_THOMPSON, WP_GRENADE_PINEAPPLE },
	  BBOX_SMALL,
	  { 32, 48 },
	  AIFL_CATCH_GRENADE | AIFL_STAND_IDLE2,
	  NULL,
	  NULL,
	  AISTATE_RELAXED },

	//AICHAR_ZOMBIE
	{ "Zombie",
	  {
		  200, // running speed		//----(SA)	DK requested change
		  60, // walking speed		//----(SA)	DK requested change
		  80, // crouching speed
		  90, // Field of View
		  350, // Yaw Speed
		  0.0, // leader
		  0.70, // aim skill
		  0.70, // aim accuracy
		  0.75, // attack skill
		  0.1, // reaction time
		  0.0, // attack crouch
		  0.0, // idle crouch
		  1.0, // aggression
		  0.0, // tactical
		  0.0, // camper
		  16000, // alertness
		  180, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "zombieSightPlayer",
		  "zombieAttackPlayer",
		  "zombieOrders",
		  "zombieDeath",
		  "zombieSilentDeath", //----(SA)	added
		  "zombieFlameDeath", //----(SA)	added
		  "zombiePain",
		  "sound/weapons/melee/fstatck.wav", // stay - you're told to stay put
		  "sound/weapons/melee/fstmiss.wav", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "zombieOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_MONSTER,
	  "zombie/default",
	  { /*WP_GAUNTLET,*/ WP_MONSTER_ATTACK2, WP_MONSTER_ATTACK3 },
	  BBOX_SMALL,
	  { 32, 48 },
	  /*AIFL_NOPAIN|AIFL_WALKFORWARD|*/ AIFL_NO_RELOAD,
	  &ai_ZombieAbilities,
	  NULL,
	  AISTATE_ALERT },

	//AICHAR_WARZOMBIE
	{ "WarriorZombie",
	  {
		  250, // running speed	(SA) upped from 200->250 per Mike/DK
		  60, // walking speed
		  80, // crouching speed
		  90, // Field of View
		  350, // Yaw Speed
		  0.0, // leader
		  0.70, // aim skill
		  0.70, // aim accuracy
		  0.75, // attack skill
		  0.1, // reaction time
		  0.0, // attack crouch
		  0.0, // idle crouch
		  1.0, // aggression
		  0.0, // tactical
		  0.0, // camper
		  16000, // alertness
		  180, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "warzombieSightPlayer",
		  "warzombieAttackPlayer",
		  "warzombieOrders",
		  "warzombieDeath",
		  "warzombieSilentDeath", //----(SA)	added
		  "warzombieFlameDeath", //----(SA)	added
		  "warzombiePain",
		  //----(SA)	changed per DK
		  //		"sound/weapons/melee/fstatck.wav",		// stay - you're told to stay put
		  "sound/weapons/melee/warz_hit.wav",
		  //		"sound/weapons/melee/fstmiss.wav",		// follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "sound/weapons/melee/warz_miss.wav",
		  "warzombieOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_MONSTER,
	  "warrior/crypt2",
	  { WP_MONSTER_ATTACK1, WP_MONSTER_ATTACK2, WP_MONSTER_ATTACK3 },
	  BBOX_SMALL,
	  { 10, 48 }, // very low defense position
	  AIFL_NO_RELOAD,
	  &ai_WarZombieAbilities,
	  NULL,
	  AISTATE_ALERT },

	//AICHAR_VENOM
	{ "Venom",
	  {
		  110, // running speed
		  100, // walking speed
		  80, // crouching speed
		  90, // Field of View
		  200, // Yaw Speed
		  0.0, // leader
		  0.70, // aim skill
		  0.70, // aim accuracy
		  0.75, // attack skill
		  0.5, // reaction time
		  0.05, // attack crouch
		  0.0, // idle crouch
		  0.9, // aggression
		  0.2, // tactical
		  0.0, // camper
		  16000, // alertness
		  240, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "venomSightPlayer",
		  "venomAttackPlayer",
		  "venomOrders",
		  "venomDeath",
		  "venomSilentDeath", //----(SA)	added
		  "venomFlameDeath", //----(SA)	added
		  "venomPain",
		  "venomStay", // stay - you're told to stay put
		  "venomFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "venomOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_NAZI,
	  "venom/default",
	  { WP_FLAMETHROWER },
	  BBOX_SMALL,
	  { 32, 48 },
	  AIFL_NO_FLAME_DAMAGE | AIFL_WALKFORWARD | AIFL_NO_RELOAD, // |AIFL_NO_HEADSHOT_DMG,
	  NULL,
	  NULL,
	  AISTATE_RELAXED },

	//AICHAR_LOPER
	{ "Loper",
	  {
		  220, // running speed
		  70, // walking speed
		  220, // crouching speed
		  90, // Field of View
		  200, // Yaw Speed
		  0.0, // leader
		  0.70, // aim skill
		  0.70, // aim accuracy
		  0.75, // attack skill
		  0.8, // reaction time
		  0.05, // attack crouch
		  0.0, // idle crouch
		  1.0, // aggression
		  0.1, // tactical
		  0.0, // camper
		  16000, // alertness
		  500, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "loperSightPlayer",
		  "loperAttackPlayer",
		  "loperOrders",
		  "loperDeath",
		  "loperSilentDeath", //----(SA)	added
		  "loperFlameDeath", //----(SA)	added
		  "loperPain",
		  "loperAttack2Start", // stay - you're told to stay put
		  "loperAttackStart", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "loperHit1", // deny - refuse orders (doing something else)
		  "loperHit2", // misc1
	  },
	  AITEAM_MONSTER,
	  "loper/default",
	  { /*WP_MONSTER_ATTACK1,*/ WP_MONSTER_ATTACK2, WP_MONSTER_ATTACK3 },
	  BBOX_LARGE,
	  { 32, 32 }, // large is for wide characters
	  AIFL_NO_RELOAD,
	  &ai_LoperAbilities,
	  "sound/world/electloop.wav",
	  AISTATE_ALERT },

	//AICHAR_ELITEGUARD
	{ "Elite Guard",
	  {
		  230, // running speed
		  90, // walking speed
		  100, // crouching speed
		  90, // Field of View
		  200, // Yaw Speed	// RF change
		  0.0, // leader
		  0.5, // aim skill
		  1.0, // aim accuracy
		  0.9, // attack skill
		  0.3, // reaction time
		  0.4, // attack crouch
		  0.0, // idle crouch
		  0.5, // aggression
		  1.0, // tactical
		  0.0, // camper
		  16000, // alertness
		  120, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "eliteGuardSightPlayer",
		  "eliteGuardAttackPlayer",
		  "eliteGuardOrders",
		  "eliteGuardDeath",
		  "eliteGuardSilentDeath", //----(SA)	added
		  "eliteGuardFlameDeath", //----(SA)	added
		  "eliteGuardPain",
		  "eliteGuardStay", // stay - you're told to stay put
		  "eliteGuardFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "eliteGuardOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_NAZI,
	  "eliteguard/default",
	  { WP_SILENCER }, //----(SA)	TODO: replace w/ "silenced luger"
	  BBOX_SMALL,
	  { 32, 48 },
	  AIFL_CATCH_GRENADE | AIFL_STAND_IDLE2,
	  NULL,
	  NULL,
	  AISTATE_RELAXED },

	//AICHAR_STIMSOLDIER1
	{ "Stim Soldier",
	  {
		  170, // running speed
		  100, // walking speed
		  90, // crouching speed
		  90, // Field of View
		  150, // Yaw Speed
		  0.0, // leader
		  0.7, // aim skill
		  1.0, // aim accuracy
		  0.9, // attack skill
		  0.6, // reaction time
		  0.05, // attack crouch
		  0.0, // idle crouch
		  0.9, // aggression
		  0.1, // tactical
		  0.0, // camper
		  16000, // alertness
		  300, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "stimSoldierSightPlayer",
		  "stimSoldierAttackPlayer",
		  "stimSoldierOrders",
		  "stimSoldierDeath",
		  "stimSoldierSilentDeath", //----(SA)	added
		  "stimSoldeirFlameDeath", //----(SA)	added
		  "stimSoldierPain",
		  "stimSoldierStay", // stay - you're told to stay put
		  "stimSoldierFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "stimSoldierOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_NAZI,
	  "stim/default",
	  { WP_MONSTER_ATTACK2 }, // TODO: dual machinegun attack
	  BBOX_LARGE,
	  { 48, 64 },
	  AIFL_NO_RELOAD,
	  NULL,
	  NULL,
	  AISTATE_ALERT },

	//AICHAR_STIMSOLDIER2
	{ "Stim Soldier",
	  {
		  170, // running speed
		  100, // walking speed
		  90, // crouching speed
		  90, // Field of View
		  150, // Yaw Speed
		  0.0, // leader
		  0.7, // aim skill
		  1.0, // aim accuracy
		  0.9, // attack skill
		  0.6, // reaction time
		  0.05, // attack crouch
		  0.0, // idle crouch
		  0.9, // aggression
		  0.1, // tactical
		  0.0, // camper
		  16000, // alertness
		  300, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "stimSoldierSightPlayer",
		  "stimSoldierAttackPlayer",
		  "stimSoldierOrders",
		  "stimSoldierDeath",
		  "stimSoldierSilentDeath", //----(SA)	added
		  "stimSoldierFlameDeath", //----(SA)	added
		  "stimSoldierPain",
		  "stimSoldierStay", // stay - you're told to stay put
		  "stimSoldierFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "stimSoldierOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_NAZI,
	  "stim/default",
	  { WP_MP40, WP_MONSTER_ATTACK1 }, // attack1 is leaping rocket attack
	  BBOX_LARGE,
	  { 48, 64 },
	  AIFL_NO_RELOAD,
	  &ai_StimRocketAbilities,
	  NULL,
	  AISTATE_ALERT },

	//AICHAR_STIMSOLDIER3
	{ "Stim Soldier",
	  {
		  170, // running speed
		  100, // walking speed
		  90, // crouching speed
		  90, // Field of View
		  150, // Yaw Speed
		  0.0, // leader
		  0.7, // aim skill
		  1.0, // aim accuracy
		  0.9, // attack skill
		  0.6, // reaction time
		  0.05, // attack crouch
		  0.0, // idle crouch
		  0.9, // aggression
		  0.1, // tactical
		  0.0, // camper
		  16000, // alertness
		  300, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "stimSoldierSightPlayer",
		  "stimSoldierAttackPlayer",
		  "stimSoldierOrders",
		  "stimSoldierDeath",
		  "stimSoldierSilentDeath", //----(SA)	added
		  "stimSoldierFlameDeath", //----(SA)	added
		  "stimSoldierPain",
		  "stimSoldierStay", // stay - you're told to stay put
		  "stimSoldierFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "stimSoldierOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_NAZI,
	  "stim/default",
	  { WP_MP40, WP_TESLA }, // no monster_attack1, since that's only used for the jumping rocket attack
	  BBOX_LARGE,
	  { 48, 64 },
	  AIFL_NO_RELOAD,
	  &ai_StimTeslaAbilities,
	  NULL,
	  AISTATE_ALERT },

	//AICHAR_SUPERSOLDIER
	{ "Super Soldier",
	  {
		  170, // running speed
		  100, // walking speed
		  90, // crouching speed
		  90, // Field of View
		  150, // Yaw Speed
		  0.0, // leader
		  0.7, // aim skill
		  1.0, // aim accuracy
		  0.9, // attack skill
		  0.6, // reaction time
		  0.05, // attack crouch
		  0.0, // idle crouch
		  1.0, // aggression
		  0.0, // tactical
		  0.0, // camper
		  16000, // alertness
		  300, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  2.0, // pain threshold multiplier
	  },
	  {
		  "superSoldierSightPlayer",
		  "superSoldierAttackPlayer",
		  "superSoldierOrders",
		  "superSoldierDeath",
		  "superSoldierSilentDeath", //----(SA)	added
		  "superSoldierFlameDeath", //----(SA)	added
		  "superSoldierPain",
		  "superSoldierStay", // stay - you're told to stay put
		  "superSoldierFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "superSoldierOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_NAZI,
	  "supersoldier/default",
	  { WP_VENOM },
	  BBOX_LARGE,
	  { 48, 64 },
	  AIFL_NO_RELOAD | AIFL_NO_FLAME_DAMAGE | AIFL_NO_TESLA_DAMAGE,
	  NULL,
	  NULL,
	  AISTATE_ALERT },

	//AICHAR_BLACKGUARD
	{ "Black Guard",
	  {
		  220, // running speed
		  90, // walking speed
		  100, // crouching speed
		  90, // Field of View
		  300, // Yaw Speed
		  0.0, // leader
		  0.5, // aim skill
		  0.8, // aim accuracy
		  0.9, // attack skill
		  0.3, // reaction time
		  0.4, // attack crouch
		  0.0, // idle crouch
		  0.5, // aggression
		  1.0, // tactical
		  0.0, // camper
		  16000, // alertness
		  120, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "blackGuardSightPlayer",
		  "blackGuardAttackPlayer",
		  "blackGuardOrders",
		  "blackGuardDeath",
		  "blackGuardSilentDeath", //----(SA)	added
		  "blackGuardFlameDeath", //----(SA)	added
		  "blackGuardPain",
		  "blackGuardStay", // stay - you're told to stay put
		  "blackGuardFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "blackGuardOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_NAZI,
	  "blackguard/default",
	  //		{WP_MP40, WP_GRENADE_LAUNCHER, WP_MONSTER_ATTACK1},	// attack1 is melee kick
	  { WP_FG42, WP_FG42SCOPE, WP_GRENADE_LAUNCHER, WP_MONSTER_ATTACK1 }, // attack1 is melee kick
	  BBOX_SMALL,
	  { 32, 48 },
	  AIFL_CATCH_GRENADE | AIFL_FLIP_ANIM | AIFL_STAND_IDLE2,
	  &ai_BlackGuardAbilities,
	  NULL,
	  AISTATE_RELAXED },

	//AICHAR_PROTOSOLDIER
	{ "Protosoldier",
	  {
		  170, // running speed
		  100, // walking speed
		  90, // crouching speed
		  90, // Field of View
		  230, // Yaw Speed
		  0.0, // leader
		  0.7, // aim skill
		  1.0, // aim accuracy
		  0.9, // attack skill
		  0.2, // reaction time
		  0.05, // attack crouch
		  0.0, // idle crouch
		  0.9, // aggression
		  0.1, // tactical
		  0.0, // camper
		  16000, // alertness
		  300, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  2.0, // pain threshold multiplier
	  },
	  {
		  "protoSoldierSightPlayer",
		  "protoSoldierAttackPlayer",
		  "protoSoldierOrders",
		  "protoSoldierDeath",
		  "protoSoldierSilentDeath", //----(SA)	added
		  "protoSoldierFlameDeath", //----(SA)	added
		  "protoSoldierPain",
		  "protoSoldierStay", // stay - you're told to stay put
		  "protoSoldierFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "protoSoldierOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_NAZI,
	  "protosoldier/default",
	  { WP_VENOM },
	  BBOX_LARGE,
	  { 48, 64 },
	  AIFL_NO_TESLA_DAMAGE | AIFL_NO_FLAME_DAMAGE | AIFL_WALKFORWARD | AIFL_NO_RELOAD,
	  NULL,
	  NULL,
	  AISTATE_ALERT },

	// AICHAR_FROGMAN
	{ "Frogman",
	  {
		  170, // running speed
		  100, // walking speed
		  90, // crouching speed
		  90, // Field of View
		  150, // Yaw Speed
		  0.0, // leader
		  0.7, // aim skill
		  1.0, // aim accuracy
		  0.9, // attack skill
		  0.6, // reaction time
		  0.05, // attack crouch
		  0.0, // idle crouch
		  0.9, // aggression
		  0.1, // tactical
		  0.0, // camper
		  16000, // alertness
		  200, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "frogmanSightPlayer",
		  "frogmanAttackPlayer",
		  "frogmanOrders",
		  "frogmanDeath",
		  "frogmanSilentDeath", //----(SA)	added
		  "frogmanFlameDeath", //----(SA)	added
		  "frogmanPain",
		  "frogmanStay", // stay - you're told to stay put
		  "frogmanFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "frogmanOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_NAZI,
	  "frogman/default",
	  { 0 },
	  BBOX_SMALL,
	  { 32, 48 }, // bbox, crouch/stand height
	  0,
	  NULL,
	  NULL,
	  AISTATE_RELAXED },

	//AICHAR_HELGA
	{ "Helga",
	  {
		  140, // running speed
		  90, // walking speed
		  80, // crouching speed
		  90, // Field of View
		  200, // Yaw Speed
		  0.0, // leader
		  0.5, // aim skill
		  0.5, // aim accuracy
		  0.75, // attack skill
		  0.5, // reaction time
		  0.0, // attack crouch
		  0.0, // idle crouch
		  1.0, // aggression
		  0.0, // tactical
		  0.0, // camper
		  16000, // alertness
		  100, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  3.0, // pain threshold multiplier
	  },
	  {
		  "helgaAttackPlayer",
		  "helgaAttackPlayer",
		  "helgaOrders",
		  "helgaDeath",
		  "helgaSilentDeath", //----(SA)	added
		  "helgaFlameDeath", //----(SA)	added
		  "helgaAttackPlayer",
		  "sound/weapons/melee/fstatck.wav", // stay - you're told to stay put
		  "helgaFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "helgaOrdersDeny", // deny - refuse orders (doing something else)
	  },
	  AITEAM_MONSTER, // team
	  "beast/default", // default model/skin
	  { WP_MONSTER_ATTACK1, WP_MONSTER_ATTACK2 /*,WP_MONSTER_ATTACK3*/ }, // starting weapons
	  BBOX_LARGE,
	  { 90, 90 }, // bbox, crouch/stand height
	  AIFL_WALKFORWARD | AIFL_NO_RELOAD,
	  &ai_HelgaAbilities,
	  NULL,
	  AISTATE_ALERT },

	//AICHAR_HEINRICH
	{ "Heinrich",
	  {
		  170, // running speed
		  100, // walking speed
		  90, // crouching speed
		  90, // Field of View
		  130, // Yaw Speed
		  0.0, // leader
		  0.7, // aim skill
		  1.0, // aim accuracy
		  0.9, // attack skill
		  0.2, // reaction time
		  0.05, // attack crouch
		  0.0, // idle crouch
		  1.0, // aggression
		  0.0, // tactical
		  0.0, // camper
		  16000, // alertness
		  2000, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  5.0, // pain threshold multiplier
	  },
	  {
		  "heinrichSightPlayer",
		  "heinrichAttackPlayer",
		  "heinrichOrders",
		  "heinrichDeath",
		  "heinrichSilentDeath",
		  "heinrichFlameDeath", //----(SA)	added
		  "heinrichPain",
		  "heinrichStay", // stay - you're told to stay put
		  "heinrichFollow", // follow - go with ordering player ("i'm with you" rather than "yes sir!")
		  "heinrichStomp", // deny - refuse orders (doing something else)
	  },
	  AITEAM_NAZI,
	  "heinrich/default",
	  { WP_MONSTER_ATTACK1, WP_MONSTER_ATTACK2, WP_MONSTER_ATTACK3 }, // attack3 is given to him by scripting
	  BBOX_LARGE,
	  { 72, 72 }, // (SA) height is not exact.  just eyeballed.
	  AIFL_NO_FLAME_DAMAGE | AIFL_WALKFORWARD | AIFL_NO_RELOAD,
	  &ai_HeinrichAbilities,
	  NULL,
	  AISTATE_ALERT },

	//AICHAR_PARTISAN
	{ "Partisan",
	  {
		  220, // running speed
		  90, // walking speed
		  80, // crouching speed
		  90, // Field of View
		  300, // Yaw Speed
		  0.0, // leader
		  0.70, // aim skill
		  0.70, // aim accuracy
		  0.75, // attack skill
		  0.5, // reaction time
		  0.3, // attack crouch
		  0.0, // idle crouch
		  0.5, // aggression
		  0.8, // tactical
		  0.0, // camper
		  16000, // alertness
		  100, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "partisanSightPlayer",
		  "partisanAttackPlayer",
		  "partisanOrders",
		  "partisanDeath",
		  "partisanSilentDeath", //----(SA)	added
		  "partisanFlameDeath", //----(SA)	added
		  "partisanPain",
		  "partisanStay",
		  "partisanFollow",
		  "partisanOrdersDeny",
	  },
	  AITEAM_ALLIES, //----(SA)	changed affiliation for DK
	  "partisan/default",
	  { WP_THOMPSON },
	  BBOX_SMALL,
	  { 32, 48 },
	  AIFL_CATCH_GRENADE | AIFL_STAND_IDLE2,
	  NULL,
	  NULL,
	  AISTATE_RELAXED },

	//AICHAR_CIVILIAN
	{ "Civilian",
	  {
		  220, // running speed
		  90, // walking speed
		  80, // crouching speed
		  90, // Field of View
		  300, // Yaw Speed
		  0.0, // leader
		  0.70, // aim skill
		  0.70, // aim accuracy
		  0.75, // attack skill
		  0.5, // reaction time
		  0.3, // attack crouch
		  0.0, // idle crouch
		  0.5, // aggression
		  0.8, // tactical
		  0.0, // camper
		  16000, // alertness
		  100, // starting health
		  1.0, // hearing scale
		  0.9, // not in pvs hearing scale
		  512, // relaxed detection radius
		  1.0, // pain threshold multiplier
	  },
	  {
		  "civilianSightPlayer",
		  "civilianAttackPlayer",
		  "civilianOrders",
		  "civilianDeath",
		  "civilianSilentDeath", //----(SA)	added
		  "civilianFlameDeath", //----(SA)	added
		  "civilianPain",
		  "civilianStay",
		  "civilianFollow",
		  "civilianOrdersDeny",
	  },
	  AITEAM_NEUTRAL, //----(SA)	changed affiliation for DK
	  "civilian/default",
	  { 0 },
	  BBOX_SMALL,
	  { 32, 48 },
	  AIFL_CATCH_GRENADE | AIFL_STAND_IDLE2,
	  NULL,
	  NULL,
	  AISTATE_RELAXED },

};

//---------------------------------------------------------------------------
// Character specific attributes (defaults, these can be altered in the editor (TODO!))

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Bounding boxes
static vec3_t bbmins[2] = { { -18, -18, -24 }, { -32, -32, -24 } };
static vec3_t bbmaxs[2] = { { 18, 18, 48 }, { 32, 32, 68 } };
//static float crouchMaxZ[2] = {32,48};	// same as player, will head be ok? // TTimo: unused
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Weapon info
cast_weapon_info_t weaponInfo;
//---------------------------------------------------------------------------

/*
============
AIChar_SetBBox

  FIXME: pass a maxZ into this so we can tailor the height for each character,
  since height isn't important for the AAS routing (whereas width is very important)
============
*/
void AIChar_SetBBox( gentity_t *ent, cast_state_t *cs, qboolean useHeadTag ) {
	vec3_t bbox[2];
	trace_t tr;
	orientation_t or ;

	if ( !useHeadTag ) {
		VectorCopy( bbmins[cs->aasWorldIndex], ent->client->ps.mins );
		VectorCopy( bbmaxs[cs->aasWorldIndex], ent->client->ps.maxs );
		ent->client->ps.maxs[2] = aiDefaults[cs->aiCharacter].crouchstandZ[1];
		VectorCopy( ent->client->ps.mins, ent->r.mins );
		VectorCopy( ent->client->ps.maxs, ent->r.maxs );
		ent->client->ps.crouchMaxZ = aiDefaults[cs->aiCharacter].crouchstandZ[0];
		ent->s.density = cs->aasWorldIndex;
	} else if ( trap_GetTag( ent->s.number, "tag_head", &or ) ) { // if not found, then just leave it
		or.origin[2] -= ent->client->ps.origin[2]; // convert to local coordinates
		or.origin[2] += 11;
		if ( or.origin[2] < 0 ) {
			or.origin[2] = 0;
		}
		if ( or.origin[2] > aiDefaults[cs->aiCharacter].crouchstandZ[1] + 30 ) {
			or.origin[2] = aiDefaults[cs->aiCharacter].crouchstandZ[1] + 30;
		}

		memset( &tr, 0, sizeof( tr ) );

		// check that the new height is ok first, otherwise leave it alone
		VectorCopy( bbmins[cs->aasWorldIndex], bbox[0] );
		VectorCopy( bbmaxs[cs->aasWorldIndex], bbox[1] );
		// set the head tag height
		bbox[1][2] = or.origin[2];

		if ( bbox[1][2] > ent->client->ps.maxs[2] ) {
			// check this area is clear
			trap_TraceCapsule( &tr, ent->client->ps.origin, bbox[0], bbox[1], ent->client->ps.origin, ent->s.number,
							   ent->clipmask );
		}

		if ( !tr.startsolid && !tr.allsolid ) {
			VectorCopy( bbox[0], ent->client->ps.mins );
			VectorCopy( bbox[1], ent->client->ps.maxs );
			VectorCopy( ent->client->ps.mins, ent->r.mins );
			VectorCopy( ent->client->ps.maxs, ent->r.maxs );
			ent->client->ps.crouchMaxZ = aiDefaults[cs->aiCharacter].crouchstandZ[0];
			ent->s.density = cs->aasWorldIndex;
		}
	}

	// if they are linked, then relink to update bbox
	if ( ent->r.linked ) {
		trap_LinkEntity( ent );
	}
}

/*
============
AIChar_Death
============
*/
void AIChar_Death( gentity_t *ent, gentity_t *attacker, int damage, int mod ) { //----(SA)	added mod
	// need this check otherwise sound will overwrite gib message
	if ( ent->health > GIB_HEALTH ) {
		if ( ent->client->ps.eFlags & EF_HEADSHOT ) {
			G_AddEvent( ent, EV_GENERAL_SOUND,
						G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[QUIETDEATHSOUNDSCRIPT] ) );
		} else {
			switch ( mod ) { //----(SA)	modified to add 'quiet' deaths
			case MOD_KNIFE_STEALTH:
			case MOD_SNIPERRIFLE:
			case MOD_SNOOPERSCOPE:
				G_AddEvent( ent, EV_GENERAL_SOUND,
							G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[QUIETDEATHSOUNDSCRIPT] ) );
				break;
			case MOD_FLAMETHROWER:
				G_AddEvent(
					ent, EV_GENERAL_SOUND,
					G_SoundIndex(
						aiDefaults[ent->aiCharacter].soundScripts[FLAMEDEATHSOUNDSCRIPT] ) ); //----(SA)	added
				break;
			default:
				G_AddEvent( ent, EV_GENERAL_SOUND,
							G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[DEATHSOUNDSCRIPT] ) );
				break;
			}
		}
	}
}

/*
=============
AIChar_GetPainLocation
=============
*/
int AIChar_GetPainLocation( gentity_t *ent, vec3_t point ) {
	static char *painTagNames[] = {
		"tag_head",	   "tag_chest",	   "tag_torso",	  "tag_groin", "tag_armright",
		"tag_armleft", "tag_legright", "tag_legleft", NULL,
	};

	int tagIndex, bestTag;
	float bestDist, dist;
	orientation_t or ;

	// first make sure the client is able to retrieve tag information
	if ( !trap_GetTag( ent->s.number, painTagNames[0], &or ) ) {
		return 0;
	}

	// find a correct animation to play, based on the body orientation at previous frame
	for ( tagIndex = 0, bestDist = 0, bestTag = -1; painTagNames[tagIndex]; tagIndex++ ) {
		// grab the tag with this name
		if ( trap_GetTag( ent->s.number, painTagNames[tagIndex], &or ) ) {
			dist = VectorDistance( or.origin, point );
			if ( !bestDist || dist < bestDist ) {
				bestTag = tagIndex;
				bestDist = dist;
			}
		}
	}

	if ( bestTag >= 0 ) {
		return bestTag + 1;
	}

	return 0;
}

/*
============
AIChar_Pain
============
*/
void AIChar_Pain( gentity_t *ent, gentity_t *attacker, int damage, vec3_t point ) {
#define PAIN_THRESHOLD 25
#define STUNNED_THRESHOLD 30
	cast_state_t *cs;
	float dist;
	qboolean forceStun = qfalse;
	float painThreshold, stunnedThreshold;

	cs = AICast_GetCastState( ent->s.number );

	if ( g_testPain.integer == 1 ) {
		ent->health = ent->client->pers.maxHealth; // debugging
	}

	if ( g_testPain.integer != 2 ) {
		if ( level.time < cs->painSoundTime ) {
			return;
		}
	}

	painThreshold = PAIN_THRESHOLD * cs->attributes[PAIN_THRESHOLD_SCALE];
	stunnedThreshold = STUNNED_THRESHOLD * cs->attributes[PAIN_THRESHOLD_SCALE];

	// if they are already playing another animation, we might get confused and cut it off, so don't play a pain
	if ( ent->client->ps.torsoTimer || ent->client->ps.legsTimer ) {
		return;
	}

	// if we are waiting for our weapon to fire (throwing a grenade)
	if ( ent->client->ps.weaponDelay ) {
		return;
	}

	if (
		attacker->s.weapon == WP_FLAMETHROWER &&
		!( cs->aiFlags &
		   AIFL_NO_FLAME_DAMAGE ) ) { // flames should be recognized more often, since they stay onfire until they're dead anyway
		painThreshold = 1;
		stunnedThreshold = 99999; // dont be stunned
	}

	// HACK: if the attacker is using the flamethrower, don't do any special pain anim or sound
	// FIXME: we should pass in the MOD here, since they could have fired a grenade, then switched weapons
	//if (attacker->s.weapon == WP_FLAMETHROWER) {
	//	return;
	//}

	if ( !Q_stricmp( attacker->classname, "props_statue" ) ) {
		damage = 99999; // try and force a stun
		forceStun = qtrue;
	}

	if ( attacker->s.weapon == WP_TESLA ) {
		damage *= 2;
		if ( cs->attributes[PAIN_THRESHOLD_SCALE] <= 1.0 ) {
			damage = 99999;
		}
	}

	// now check the damageQuota to see if we should play a pain animation
	// first reduce the current damageQuota with time
	if ( cs->damageQuotaTime && cs->damageQuota > 0 ) {
		cs->damageQuota -= (int)( ( 1.0 + ( g_gameskill.value / GSKILL_MAX ) ) *
								  ( (float)( level.time - cs->damageQuotaTime ) / 1000 ) *
								  ( 7.5 + cs->attributes[ATTACK_SKILL] * 10.0 ) );
		if ( cs->damageQuota < 0 ) {
			cs->damageQuota = 0;
		}
	}

	// if it's been a long time since our last pain, scale it up
	if ( cs->painSoundTime < level.time - 1000 ) {
		float scale;
		scale = (float)( level.time - cs->painSoundTime - 1000 ) / 1000.0;
		if ( scale > 4.0 ) {
			scale = 4.0;
		}
		damage = (int)( (float)damage * ( 1.0 + ( scale * ( 1.0 - 0.5 * g_gameskill.value / GSKILL_MAX ) ) ) );
	}

	// adjust the new damage with distance, if they are really close, scale it down, to make it
	// harder to get through the game by continually rushing the enemies
	if ( ( attacker->s.weapon != WP_TESLA ) &&
		 ( ( dist = VectorDistance( ent->r.currentOrigin, attacker->r.currentAngles ) ) < 384 ) ) {
		damage -= (int)( (float)damage * ( 1.0 - ( dist / 384.0 ) ) * ( 0.5 + 0.5 * g_gameskill.value / GSKILL_MAX ) );
	}

	// add the new damage
	cs->damageQuota += damage;
	cs->damageQuotaTime = level.time;

	if ( forceStun ) {
		damage = 99999; // try and force a stun
		cs->damageQuota = painThreshold + 1;
	}

	// if it's over the threshold, play a pain

	// don't do this if crouching, or we might clip through the world

	if ( g_testPain.integer == 2 || ( cs->damageQuota > painThreshold ) ) {
		int delay;

		// stunned?
		if ( damage > stunnedThreshold && ( forceStun || ( rand() % 2 ) ) ) { // stunned
			BG_UpdateConditionValue( ent->s.number, ANIM_COND_STUNNED, qtrue, qfalse );
		}
		// enemy weapon
		if ( attacker->client ) {
			BG_UpdateConditionValue( ent->s.number, ANIM_COND_ENEMY_WEAPON, attacker->s.weapon, qtrue );
		}
		if ( point ) {
			// location
			BG_UpdateConditionValue( ent->s.number, ANIM_COND_IMPACT_POINT, AIChar_GetPainLocation( ent, point ),
									 qtrue );
		} else {
			BG_UpdateConditionValue( ent->s.number, ANIM_COND_IMPACT_POINT, 0, qfalse );
		}

		// pause while we play a pain
		delay = BG_AnimScriptEvent( &ent->client->ps, ANIM_ET_PAIN, qfalse, qtrue );

		// turn off temporary conditions
		BG_UpdateConditionValue( ent->s.number, ANIM_COND_STUNNED, 0, qfalse );
		BG_UpdateConditionValue( ent->s.number, ANIM_COND_ENEMY_WEAPON, 0, qfalse );
		BG_UpdateConditionValue( ent->s.number, ANIM_COND_IMPACT_POINT, 0, qfalse );

		if ( delay >= 0 ) {
			// setup game stuff to handle the character movements, etc
			cs->pauseTime = level.time + delay + 250;
			cs->lockViewAnglesTime = cs->pauseTime;
			// make sure we stop crouching
			cs->attackcrouch_time = 0;
			// don't fire while in pain?
			cs->combat.triggerReleaseTime = cs->pauseTime;
			// stay crouching if we were before the pain
			if ( cs->clientState->cur_ps.viewheight == cs->clientState->cur_ps.crouchViewHeight ) {
				cs->attackcrouch_time = level.time + (float)( cs->pauseTime - level.time ) + 500;
			}
		}

		// if we didnt just play a scripted sound, then play one of the default sounds
		if ( cs->perception.lastScriptSound < level.time ) {
			G_AddEvent( ent, EV_GENERAL_SOUND,
						G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[PAINSOUNDSCRIPT] ) );
		}

		// reset the quota
		cs->damageQuota = 0;
		cs->damageQuotaTime = 0;
		//
		cs->painSoundTime =
			cs->pauseTime +
			(int)( 1000 * ( g_gameskill.value / GSKILL_MAX ) ); // add a bit more of a buffer before the next one
	}
}

/*
============
AIChar_Sight
============
*/
void AIChar_Sight( gentity_t *ent, gentity_t *other, int lastSight ) {
	cast_state_t *cs;

	cs = AICast_GetCastState( ent->s.number );

	// if we are in noattack mode, don't make sounds
	if ( cs->script.castScriptStatus.scriptNoAttackTime >= level.time ) {
		return;
	}
	if ( cs->combat.noAttackTime >= level.time ) {
		return;
	}

	// if they have recently played a script sound, then ignore this
	if ( cs->perception.lastScriptSound > level.time - 4000 ) {
		return;
	}

	if ( !AICast_SameTeam( cs, other->s.number ) ) {
		if ( !cs->firstSightTime || cs->firstSightTime < ( level.time - 15000 ) ) {
			//G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( aiDefaults[ent->aiCharacter].sightSoundScript ) );
		}
		cs->firstSightTime = level.time;
	}
}

/*
=====================
AIChar_AttackSND

  NOTE: this should just lookup a sound script for this character/weapon combo
=====================
*/
void AIChar_AttackSound( cast_state_t *cs ) {

	gentity_t *ent;

	ent = &g_entities[cs->entityNum];

	if ( cs->attackSNDtime > level.time ) {
		return;
	}

	// if we are in noattack mode, don't make sounds
	if ( cs->script.castScriptStatus.scriptNoAttackTime >= level.time ) {
		return;
	}
	if ( cs->combat.noAttackTime >= level.time ) {
		return;
	}

	// Ridah, only yell when throwing grenades every now and then, since it's not very "stealthy"
	if ( cs->weaponNum == WP_GRENADE_LAUNCHER && rand() % 5 ) {
		return;
	}

	cs->attackSNDtime = level.time + 5000 + ( 1000 * rand() % 10 );

	AICast_ScriptEvent( cs, "attacksound", ent->aiName );
	if ( cs->aiFlags & AIFL_DENYACTION ) {
		return;
	}

	if ( cs->weaponNum == WP_LUGER ) {
		G_AddEvent( ent, EV_GENERAL_SOUND,
					G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[ORDERSSOUNDSCRIPT] ) );
	} else {
		G_AddEvent( ent, EV_GENERAL_SOUND,
					G_SoundIndex( aiDefaults[ent->aiCharacter].soundScripts[ATTACKSOUNDSCRIPT] ) );
	}
}
