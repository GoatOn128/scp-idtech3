/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

// ui_players.c

#include "../precompiled.h"

#include "ui_local.h"


#define UI_TIMER_GESTURE        2300
#define UI_TIMER_JUMP           1000
#define UI_TIMER_LAND           130
#define UI_TIMER_WEAPON_SWITCH  300
#define UI_TIMER_ATTACK         500
#define UI_TIMER_MUZZLE_FLASH   20
#define UI_TIMER_WEAPON_DELAY   250

#define JUMP_HEIGHT             56

#define SWINGSPEED              0.3

#define SPIN_SPEED              0.9
#define COAST_TIME              1000


static int dp_realtime;
static float jumpHeight;


/*
===============
UI_PlayerInfo_SetWeapon
===============
*/
static void UI_PlayerInfo_SetWeapon( playerInfo_t *pi, weapon_t weaponNum ) {
	gitem_t *   item;
	char path[MAX_QPATH];

	pi->currentWeapon = weaponNum;
tryagain:
	pi->realWeapon = weaponNum;
	pi->weaponModel = 0;
	pi->barrelModel = 0;
	pi->flashModel = 0;

	if ( weaponNum == WP_NONE ) {
		return;
	}

	for ( item = bg_itemlist + 1; item->classname ; item++ ) {
		if ( item->giType != IT_WEAPON ) {
			continue;
		}
		if ( item->giTag == weaponNum ) {
			break;
		}
	}

	if ( item->classname ) {
		pi->weaponModel = trap_R_RegisterModel( item->world_model[0] );
	}

	if ( pi->weaponModel == 0 ) {
//		if( weaponNum == WP_MACHINEGUN ) {	//----(SA)	removing old weapon references
		if ( weaponNum == WP_MP40 ) {
			weaponNum = WP_NONE;
			goto tryagain;
		}
//		weaponNum = WP_MACHINEGUN;	//----(SA)	removing old weapon references
		weaponNum = WP_MP40;
		goto tryagain;
	}

	strcpy( path, item->world_model[0] );
	idStr stripped( path );
	stripped.StripFileExtension();
	idStr::Copynz( path, stripped.c_str(), sizeof( path ) );
	strcat( path, "_flash.md3" );
	pi->flashModel = trap_R_RegisterModel( path );

	switch ( weaponNum ) {
	case WP_GAUNTLET:
		MAKERGB( pi->flashDlightColor, 0.6, 0.6, 1 );
		break;

//	case WP_MACHINEGUN:
//		MAKERGB( pi->flashDlightColor, 1, 1, 0 );
//		break;

//	case WP_SHOTGUN:
//		MAKERGB( pi->flashDlightColor, 1, 1, 0 );
//		break;

	case WP_GRENADE_LAUNCHER:
		MAKERGB( pi->flashDlightColor, 1, 0.7, 0.5 );
		break;

	case WP_FLAMETHROWER:
		MAKERGB( pi->flashDlightColor, 0.6, 0.6, 1 );
		break;

//	case WP_RAILGUN:
//		MAKERGB( pi->flashDlightColor, 1, 0.5, 0 );
//		break;

//	case WP_BFG:
//		MAKERGB( pi->flashDlightColor, 1, 0.7, 1 );
//		break;

//	case WP_GRAPPLING_HOOK:
//		MAKERGB( pi->flashDlightColor, 0.6, 0.6, 1 );
//		break;

	default:
		MAKERGB( pi->flashDlightColor, 1, 1, 1 );
		break;
	}
}


/*
===============
UI_ForceLegsAnim
===============
*/
static void UI_ForceLegsAnim( playerInfo_t *pi, int anim ) {
	pi->legsAnim = ( ( pi->legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

	if ( anim == LEGS_JUMP ) {
		pi->legsAnimationTimer = UI_TIMER_JUMP;
	}
}


/*
===============
UI_SetLegsAnim
===============
*/
// TTimo: unused
/*
static void UI_SetLegsAnim( playerInfo_t *pi, int anim ) {
	if ( pi->pendingLegsAnim ) {
		anim = pi->pendingLegsAnim;
		pi->pendingLegsAnim = 0;
	}
	UI_ForceLegsAnim( pi, anim );
}
*/

/*
===============
UI_ForceTorsoAnim
===============
*/
static void UI_ForceTorsoAnim( playerInfo_t *pi, int anim ) {
	pi->torsoAnim = ( ( pi->torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

	if ( anim == TORSO_GESTURE ) {
		pi->torsoAnimationTimer = UI_TIMER_GESTURE;
	}

	if ( anim == TORSO_ATTACK || anim == TORSO_ATTACK2 ) {
		pi->torsoAnimationTimer = UI_TIMER_ATTACK;
	}
}


/*
===============
UI_SetTorsoAnim
===============
*/
// TTimo: unused
/*
static void UI_SetTorsoAnim( playerInfo_t *pi, int anim ) {
	if ( pi->pendingTorsoAnim ) {
		anim = pi->pendingTorsoAnim;
		pi->pendingTorsoAnim = 0;
	}

	UI_ForceTorsoAnim( pi, anim );
}
*/

/*
===============
UI_TorsoSequencing
===============
*/
// TTimo: unused
/*
static void UI_TorsoSequencing( playerInfo_t *pi ) {
	int				currentAnim;
	animNumber_t	raisetype;	//----(SA) added

	currentAnim = pi->torsoAnim & ~ANIM_TOGGLEBIT;

	if ( pi->weapon != pi->currentWeapon ) {
		if ( currentAnim != TORSO_DROP ) {
			pi->torsoAnimationTimer = UI_TIMER_WEAPON_SWITCH;
			UI_ForceTorsoAnim( pi, TORSO_DROP );
		}
	}

	if ( pi->torsoAnimationTimer > 0 ) {
		return;
	}

	if( currentAnim == TORSO_GESTURE ) {
		UI_SetTorsoAnim( pi, TORSO_STAND );
		return;
	}

	if( currentAnim == TORSO_ATTACK	|| currentAnim == TORSO_ATTACK2 ||
		currentAnim == TORSO_ATTACK3 || currentAnim == TORSO_ATTACK4 ||
		currentAnim == TORSO_ATTACK5 || currentAnim == TORSO_ATTACK5B) {
		UI_SetTorsoAnim( pi, TORSO_STAND );
		return;
	}

	if ( currentAnim == TORSO_DROP ) {
		UI_PlayerInfo_SetWeapon( pi, pi->weapon );
		pi->torsoAnimationTimer = UI_TIMER_WEAPON_SWITCH;

//----(SA) added
		switch(pi->weapon)
		{
			case WP_MAUSER:
				raisetype = TORSO_RAISE2;	// (high)
				break;

			case WP_GAUNTLET:
			case WP_SILENCER:
			case WP_LUGER:
			case WP_KNIFE:
				raisetype = TORSO_RAISE3;	// (pistol)
				break;

			case WP_GRENADE_LAUNCHER:
				raisetype = TORSO_RAISE5;	// (throw)
				break;

			default:
				raisetype = TORSO_RAISE;	// (low)
				break;
		}

		UI_ForceTorsoAnim( pi, raisetype );

		return;
	}

	if (	currentAnim == TORSO_RAISE || currentAnim == TORSO_RAISE2 ||
			currentAnim == TORSO_RAISE3 || currentAnim == TORSO_RAISE4 ||
			currentAnim == TORSO_RAISE5) {
		UI_SetTorsoAnim( pi, TORSO_STAND );
		return;
	}
//----(SA) end
}
*/

/*
===============
UI_LegsSequencing
===============
*/
// TTimo: unused
/*
static void UI_LegsSequencing( playerInfo_t *pi ) {
	int		currentAnim;

	currentAnim = pi->legsAnim & ~ANIM_TOGGLEBIT;

	if ( pi->legsAnimationTimer > 0 ) {
		if ( currentAnim == LEGS_JUMP ) {
			jumpHeight = JUMP_HEIGHT * sin( M_PI * ( UI_TIMER_JUMP - pi->legsAnimationTimer ) / UI_TIMER_JUMP );
		}
		return;
	}

	if ( currentAnim == LEGS_JUMP ) {
		UI_ForceLegsAnim( pi, LEGS_LAND );
		pi->legsAnimationTimer = UI_TIMER_LAND;
		jumpHeight = 0;
		return;
	}

	if ( currentAnim == LEGS_LAND ) {
		UI_SetLegsAnim( pi, LEGS_IDLE );
		return;
	}
}
*/

/*
======================
UI_PositionEntityOnTag
======================
*/
static void UI_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
									clipHandle_t parentModel, char *tagName ) {
	int i;
	orientation_t lerped;

	// lerp the tag
	trap_CM_LerpTag( &lerped, (const refEntity_t *)parent, (const char *)tagName, 0 );

	// FIXME: allow origin offsets along tag?
	entity->origin = parent->origin;
	for ( i = 0 ; i < 3 ; i++ ) {
		entity->origin = entity->origin + parent->axis[i] * lerped.origin[i];
	}

	idMat3 lerpedAxis( lerped.axis[0], lerped.axis[1], lerped.axis[2] );
	idMat3 parentAxis( parent->axis[0], parent->axis[1], parent->axis[2] );
	idMat3 entityAxis = lerpedAxis * parentAxis;
	entity->axis[0] = entityAxis[0];
	entity->axis[1] = entityAxis[1];
	entity->axis[2] = entityAxis[2];
	entity->backlerp = parent->backlerp;
}


/*
======================
UI_PositionRotatedEntityOnTag
======================
*/
static void UI_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
										   clipHandle_t parentModel, char *tagName ) {
	int i;
	orientation_t lerped;

	// lerp the tag
	trap_CM_LerpTag( &lerped, parent, tagName, 0 );

	// FIXME: allow origin offsets along tag?
	entity->origin = parent->origin;
	for ( i = 0 ; i < 3 ; i++ ) {
		entity->origin = entity->origin + parent->axis[i] * lerped.origin[i];
	}

	idMat3 entityMatrix( entity->axis[0], entity->axis[1], entity->axis[2] );
	idMat3 parentMatrix( parent->axis[0], parent->axis[1], parent->axis[2] );
	idMat3 tempMatrix = entityMatrix * parentMatrix;
	idMat3 lerpedMatrix( lerped.axis[0], lerped.axis[1], lerped.axis[2] );
	idMat3 resultMatrix = lerpedMatrix * tempMatrix;
	entity->axis[0] = resultMatrix[0];
	entity->axis[1] = resultMatrix[1];
	entity->axis[2] = resultMatrix[2];
}


/*
===============
UI_SetLerpFrameAnimation
===============
*/
// TTimo: unused
/*
static void UI_SetLerpFrameAnimation( playerInfo_t *ci, lerpFrame_t *lf, int newAnimation ) {
	animation_t	*anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 0 || newAnimation >= MAX_ANIMATIONS ) {
		trap_Error( idStr::FormatString("Bad animation number (UI_SLFA): %i", newAnimation) );
	}

	anim = &ci->animations[ newAnimation ];

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;
}
*/

/*
===============
UI_RunLerpFrame
===============
*/
// TTimo: unused
/*
static void UI_RunLerpFrame( playerInfo_t *ci, lerpFrame_t *lf, int newAnimation ) {
	int			f;
	animation_t	*anim;

	// see if the animation sequence is switching
	if ( newAnimation != lf->animationNumber || !lf->animation ) {
		UI_SetLerpFrameAnimation( ci, lf, newAnimation );
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if ( dp_realtime >= lf->frameTime ) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		if ( dp_realtime < lf->animationTime ) {
			lf->frameTime = lf->animationTime;		// initial lerp
		} else {
			lf->frameTime = lf->oldFrameTime + anim->frameLerp;
		}
		f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
		if ( f >= anim->numFrames ) {
			f -= anim->numFrames;
			if ( anim->loopFrames ) {
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			} else {
				f = anim->numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				lf->frameTime = dp_realtime;
			}
		}
		lf->frame = anim->firstFrame + f;
		if ( dp_realtime > lf->frameTime ) {
			lf->frameTime = dp_realtime;
		}
	}

	if ( lf->frameTime > dp_realtime + 200 ) {
		lf->frameTime = dp_realtime;
	}

	if ( lf->oldFrameTime > dp_realtime ) {
		lf->oldFrameTime = dp_realtime;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) {
		lf->backlerp = 0;
	} else {
		lf->backlerp = 1.0 - (float)( dp_realtime - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
	}
}
*/

/*
===============
UI_PlayerAnimation
===============
*/
// TTimo: unused
/*
static void UI_PlayerAnimation( playerInfo_t *pi, int *legsOld, int *legs, float *legsBackLerp,
						int *torsoOld, int *torso, float *torsoBackLerp ) {

	// legs animation
	pi->legsAnimationTimer -= uis.frametime;
	if ( pi->legsAnimationTimer < 0 ) {
		pi->legsAnimationTimer = 0;
	}

	UI_LegsSequencing( pi );

	if ( pi->legs.yawing && ( pi->legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_IDLE ) {
		UI_RunLerpFrame( pi, &pi->legs, LEGS_TURN );
	} else {
		UI_RunLerpFrame( pi, &pi->legs, pi->legsAnim );
	}
	*legsOld = pi->legs.oldFrame;
	*legs = pi->legs.frame;
	*legsBackLerp = pi->legs.backlerp;

	// torso animation
	pi->torsoAnimationTimer -= uis.frametime;
	if ( pi->torsoAnimationTimer < 0 ) {
		pi->torsoAnimationTimer = 0;
	}

	UI_TorsoSequencing( pi );

	UI_RunLerpFrame( pi, &pi->torso, pi->torsoAnim );
	*torsoOld = pi->torso.oldFrame;
	*torso = pi->torso.frame;
	*torsoBackLerp = pi->torso.backlerp;
}
*/

/*
==================
UI_SwingAngles
==================
*/
static void UI_SwingAngles( float destination, float swingTolerance, float clampTolerance,
							float speed, float *angle, qboolean *swinging ) {
	float swing;
	float move;
	float scale;

	if ( !*swinging ) {
		// see if a swing should be started
		swing = idMath::AngleDelta( *angle, destination );
		if ( swing > swingTolerance || swing < -swingTolerance ) {
			*swinging = qtrue;
		}
	}

	if ( !*swinging ) {
		return;
	}

	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = idMath::AngleDelta( destination, *angle );
	scale = fabs( swing );
	if ( scale < swingTolerance * 0.5 ) {
		scale = 0.5;
	} else if ( scale < swingTolerance ) {
		scale = 1.0;
	} else {
		scale = 2.0;
	}

	// swing towards the destination angle
	if ( swing >= 0 ) {
		move = uis.frametime * scale * speed;
		if ( move >= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = idMath::AngleNormalize360( *angle + move );
	} else if ( swing < 0 ) {
		move = uis.frametime * scale * -speed;
		if ( move <= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = idMath::AngleNormalize360( *angle + move );
	}

	// clamp to no more than tolerance
	swing = idMath::AngleDelta( destination, *angle );
	if ( swing > clampTolerance ) {
		*angle = idMath::AngleNormalize360( destination - ( clampTolerance - 1 ) );
	} else if ( swing < -clampTolerance ) {
		*angle = idMath::AngleNormalize360( destination + ( clampTolerance - 1 ) );
	}
}


/*
======================
UI_MovedirAdjustment
======================
*/
static float UI_MovedirAdjustment( playerInfo_t *pi ) {
	idVec3 relativeAngles;
	idVec3 moveVector;

	relativeAngles = pi->viewAngles - pi->moveAngles;
	idAngles( relativeAngles ).ToVectors( &moveVector, NULL, NULL );
	if ( fabs( moveVector[0] ) < 0.01 ) {
		moveVector[0] = 0.0;
	}
	if ( fabs( moveVector[1] ) < 0.01 ) {
		moveVector[1] = 0.0;
	}

	if ( moveVector[1] == 0 && moveVector[0] > 0 ) {
		return 0;
	}
	if ( moveVector[1] < 0 && moveVector[0] > 0 ) {
		return 22;
	}
	if ( moveVector[1] < 0 && moveVector[0] == 0 ) {
		return 45;
	}
	if ( moveVector[1] < 0 && moveVector[0] < 0 ) {
		return -22;
	}
	if ( moveVector[1] == 0 && moveVector[0] < 0 ) {
		return 0;
	}
	if ( moveVector[1] > 0 && moveVector[0] < 0 ) {
		return 22;
	}
	if ( moveVector[1] > 0 && moveVector[0] == 0 ) {
		return -45;
	}

	return -22;
}


/*
===============
UI_PlayerAngles
===============
*/
static void UI_PlayerAngles( playerInfo_t *pi, idVec3 legs[3], idVec3 torso[3], idVec3 head[3] ) {
	idVec3 legsAngles, torsoAngles, headAngles;
	float dest;
	float adjust;

	headAngles = pi->viewAngles;
	headAngles[YAW] = idMath::AngleNormalize360( headAngles[YAW] );
	legsAngles.Zero();
	torsoAngles.Zero();

	// --------- yaw -------------

	// allow yaw to drift a bit
	if ( ( pi->legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_IDLE
		 || ( pi->torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_STAND  ) {
		// if not standing still, always point all in the same direction
		pi->torso.yawing = qtrue;   // always center
		pi->torso.pitching = qtrue; // always center
		pi->legs.yawing = qtrue;    // always center
	}

	// adjust legs for movement dir
	adjust = UI_MovedirAdjustment( pi );
	legsAngles[YAW] = headAngles[YAW] + adjust;
	torsoAngles[YAW] = headAngles[YAW] + 0.25 * adjust;


	// torso
	UI_SwingAngles( torsoAngles[YAW], 25, 90, SWINGSPEED, &pi->torso.yawAngle, &pi->torso.yawing );
	UI_SwingAngles( legsAngles[YAW], 40, 90, SWINGSPEED, &pi->legs.yawAngle, &pi->legs.yawing );

	torsoAngles[YAW] = pi->torso.yawAngle;
	legsAngles[YAW] = pi->legs.yawAngle;

	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if ( headAngles[PITCH] > 180 ) {
		dest = ( -360 + headAngles[PITCH] ) * 0.75;
	} else {
		dest = headAngles[PITCH] * 0.75;
	}
	UI_SwingAngles( dest, 15, 30, 0.1, &pi->torso.pitchAngle, &pi->torso.pitching );
	torsoAngles[PITCH] = pi->torso.pitchAngle;

	// pull the angles back out of the hierarchial chain
	AnglesSubtract( headAngles, torsoAngles, headAngles );
	AnglesSubtract( torsoAngles, legsAngles, torsoAngles );

	AnglesSubtract( legsAngles, pi->moveAngles, legsAngles );       // NERVE - SMF

	idAngles( legsAngles ).ToVectors( &legs[0], &legs[1], &legs[2] );
	idAngles( torsoAngles ).ToVectors( &torso[0], &torso[1], &torso[2] );
	idAngles( headAngles ).ToVectors( &head[0], &head[1], &head[2] );
}


/*
===============
UI_PlayerFloatSprite
===============
*/
static void UI_PlayerFloatSprite( playerInfo_t *pi, idVec3 origin, qhandle_t shader ) {
	refEntity_t ent;

	memset( &ent, 0, sizeof( ent ) );
	ent.origin = origin;
	ent.origin[2] += 48;
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 10;
	ent.renderfx = 0;
	trap_R_AddRefEntityToScene( &ent );
}


/*
======================
UI_MachinegunSpinAngle
======================
*/
float   UI_MachinegunSpinAngle( playerInfo_t *pi ) {
	int delta;
	float angle;
	float speed;
	int torsoAnim;

	delta = dp_realtime - pi->barrelTime;
	if ( pi->barrelSpinning ) {
		angle = pi->barrelAngle + delta * SPIN_SPEED;
	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = 0.5 * ( SPIN_SPEED + (float)( COAST_TIME - delta ) / COAST_TIME );
		angle = pi->barrelAngle + delta * speed;
	}

	torsoAnim = pi->torsoAnim  & ~ANIM_TOGGLEBIT;
	if ( torsoAnim == TORSO_ATTACK2 ) {
		torsoAnim = TORSO_ATTACK;
	}
	if ( pi->barrelSpinning == !( torsoAnim == TORSO_ATTACK ) ) {
		pi->barrelTime = dp_realtime;
		pi->barrelAngle = idMath::AngleNormalize360( angle );
		pi->barrelSpinning = !!( torsoAnim == TORSO_ATTACK );
	}

	return angle;
}

// NERVE - SMF
/*
===============
UI_GetAnimation
===============
*/
static int UI_GetAnimation( playerInfo_t *pi, const char *name ) {
	int i;

	for ( i = 0; i < pi->numAnimations; i++ ) {
		if ( !idStr::Icmp( pi->animations[i].name, name ) ) {
			return pi->animations[i].firstFrame;
		}
	}

	return 0;
}

/*
===============
UI_DrawPlayer
===============
*/
void WM_getWeaponAnim( const char **torso_anim, const char **legs_anim );       // NERVE - SMF

void UI_DrawPlayer( float x, float y, float w, float h, playerInfo_t *pi, int time ) {
	refdef_t refdef;
	refEntity_t legs;
	refEntity_t torso;
	refEntity_t head;
	refEntity_t gun;
	refEntity_t backpack;
	refEntity_t helmet;
//	refEntity_t		barrel;
	refEntity_t flash;
	idVec3 origin;
	int renderfx;
	idVec3 mins( -16, -16, -24 );
	idVec3 maxs( 16, 16, 32 );
	float len;
	float xx;
	idVec4 hcolor( 1, 0, 0, 0.5 );
	const char      *torso_anim = NULL, *legs_anim = NULL;

	if ( !pi->legsModel || !pi->torsoModel || !pi->headModel || !pi->animations[0].numFrames ) {
		return;
	}

	dp_realtime = time;

	if ( pi->pendingWeapon != -1 && dp_realtime > pi->weaponTimer ) {
		pi->weapon = pi->pendingWeapon;
		pi->lastWeapon = pi->pendingWeapon;
		pi->pendingWeapon = (weapon_t)-1;
		pi->weaponTimer = 0;
		if ( pi->currentWeapon != pi->weapon ) {
			trap_S_StartLocalSound( trap_S_RegisterSound( "sound/weapons/change.wav" ), CHAN_LOCAL );
		}
	}

	UI_AdjustFrom640( &x, &y, &w, &h );

	y -= jumpHeight;

	memset( &refdef, 0, sizeof( refdef ) );
	memset( &legs, 0, sizeof( legs ) );
	memset( &torso, 0, sizeof( torso ) );
	memset( &head, 0, sizeof( head ) );

	refdef.rdflags = RDF_NOWORLDMODEL;

	refdef.viewaxis[0] = mat3_identity[0];
	refdef.viewaxis[1] = mat3_identity[1];
	refdef.viewaxis[2] = mat3_identity[2];

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.fov_x = (int)( (float)refdef.width / 640.0f * 90.0f );
	xx = refdef.width / tan( refdef.fov_x / 360 * M_PI );
	refdef.fov_y = atan2( refdef.height, xx );
	refdef.fov_y *= ( 360 / M_PI );

	// calculate distance so the player nearly fills the box
	len = 1.01 * ( maxs[2] - mins[2] );                         // NERVE - SMF - changed from 0.7
	origin[0] = len / tan( DEG2RAD( refdef.fov_x ) * 0.5 );
	origin[1] = 0.5 * ( mins[1] + maxs[1] );
	origin[2] = -0.5 * ( mins[2] + maxs[2] );

	refdef.time = dp_realtime;

	trap_R_SetColor( hcolor.ToFloatPtr() );
	trap_R_ClearScene();
	trap_R_SetColor( NULL );

	// get the rotation information
	UI_PlayerAngles( pi, legs.axis, torso.axis, head.axis );

	// get the animation state (after rotation, to allow feet shuffle)
//	UI_PlayerAnimation( pi, &legs.oldframe, &legs.frame, &legs.backlerp,
//		 &torso.oldframe, &torso.frame, &torso.backlerp );

	renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;

	//
	// add the body
	//
	legs.hModel = pi->legsModel;
	legs.customSkin = pi->legsSkin;
	legs.renderfx = renderfx;

	legs.origin = origin;
	legs.lightingOrigin = origin;
	legs.oldorigin = legs.origin;

	WM_getWeaponAnim( &torso_anim, &legs_anim );

	if ( torso_anim ) {
		legs.torsoFrame = UI_GetAnimation( pi, torso_anim );
		legs.oldTorsoFrame = UI_GetAnimation( pi, torso_anim );
	}
	legs.torsoBacklerp = 0; //torso.backlerp;

	if ( legs_anim ) {
		legs.frame = UI_GetAnimation( pi, legs_anim );
		legs.oldframe = UI_GetAnimation( pi, legs_anim );
	}
	legs.backlerp = 0;

	memcpy( legs.torsoAxis, torso.axis, sizeof( torso.axis ) );
	torso = legs;

	trap_R_AddRefEntityToScene( &torso );

	//
	// add the head
	//
	head.hModel = pi->headModel;
	if ( !head.hModel ) {
		return;
	}
	head.customSkin = pi->headSkin;

	head.lightingOrigin = origin;

	UI_PositionRotatedEntityOnTag( &head, &torso, pi->torsoModel, "tag_head" );

	head.renderfx = renderfx;

	trap_R_AddRefEntityToScene( &head );

	//
	// add the gun
	//
	if ( pi->currentWeapon != WP_NONE ) {
		memset( &gun, 0, sizeof( gun ) );
		gun.hModel = pi->weaponModel;
		gun.lightingOrigin = origin;
		UI_PositionEntityOnTag( &gun, &torso, pi->torsoModel, "tag_weapon" );
		gun.renderfx = renderfx;
		trap_R_AddRefEntityToScene( &gun );
	}

	//
	// add muzzle flash
	//
	if ( dp_realtime <= pi->muzzleFlashTime ) {
		if ( pi->flashModel ) {
			memset( &flash, 0, sizeof( flash ) );
			flash.hModel = pi->flashModel;
			flash.lightingOrigin = origin;
			UI_PositionEntityOnTag( &flash, &gun, pi->weaponModel, "tag_flash" );
			flash.renderfx = renderfx;
			trap_R_AddRefEntityToScene( &flash );
		}

		// make a dlight for the flash
		if ( pi->flashDlightColor[0] || pi->flashDlightColor[1] || pi->flashDlightColor[2] ) {
			trap_R_AddLightToScene( flash.origin, 200 + ( rand() & 31 ), pi->flashDlightColor[0],
									pi->flashDlightColor[1], pi->flashDlightColor[2], 0 );
		}
	}

	//
	// add the backpack
	//
	if ( pi->backpackModel ) {
		memset( &backpack, 0, sizeof( backpack ) );
		backpack.hModel = pi->backpackModel;
		backpack.lightingOrigin = origin;
		UI_PositionEntityOnTag( &backpack, &torso, pi->torsoModel, "tag_back" );
		backpack.renderfx = renderfx;
		trap_R_AddRefEntityToScene( &backpack );
	}

	//
	// add the helmet
	//
	if ( pi->helmetModel ) {
		memset( &helmet, 0, sizeof( helmet ) );
		helmet.hModel = pi->helmetModel;
		helmet.lightingOrigin = origin;
		UI_PositionEntityOnTag( &helmet, &head, pi->headModel, "tag_mouth" );
		helmet.renderfx = renderfx;
		trap_R_AddRefEntityToScene( &helmet );
	}

	//
	// add the chat icon
	//
	if ( pi->chat ) {
		UI_PlayerFloatSprite( pi, origin, trap_R_RegisterShaderNoMip( "sprites/balloon3" ) );
	}

	//
	// add an accent light
	//
//	origin[0] -= 100;	// + = behind, - = in front
//	origin[1] += 100;	// + = left, - = right
//	origin[2] += 100;	// + = above, - = below
	trap_R_AddLightToScene( origin, 500, 1.0, 1.0, 1.0, 0 );

	origin[0] -= 100;
	origin[1] -= 100;
	origin[2] -= 100;
	trap_R_AddLightToScene( origin, 500, 1.0, 0.0, 0.0, 0 );

	trap_R_RenderScene( &refdef );
}


/*
==========================
UI_RegisterClientSkin
==========================
*/
static qboolean UI_RegisterClientSkin( playerInfo_t *pi, const char *modelName, const char *skinName ) {
	char filename[MAX_QPATH];

//	Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower_%s.skin", modelName, skinName );
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/body_%s.skin", modelName, skinName );      // NERVE - SMF - make this work with wolf
	pi->legsSkin = trap_R_RegisterSkin( filename );

//	Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper_%s.skin", modelName, skinName );
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/body_%s.skin", modelName, skinName );  // NERVE - SMF - make this work with wolf
	pi->torsoSkin = trap_R_RegisterSkin( filename );

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/head_%s.skin", modelName, skinName );
	pi->headSkin = trap_R_RegisterSkin( filename );

	if ( !pi->legsSkin || !pi->torsoSkin || !pi->headSkin ) {
		return qfalse;
	}

	return qtrue;
}

/*
================
return a hash value for the given string
================
*/
static long BG_StringHashValue( const char *fname ) {
	int i;
	long hash;
	char letter;

	hash = 0;
	i = 0;
	while ( fname[i] != '\0' ) {
		letter = tolower( fname[i] );
		hash += (long)( letter ) * ( i + 119 );
		i++;
	}
	if ( hash == -1 ) {
		hash = 0;   // never return -1
	}
	return hash;
}

/*
============
AnimParseAnimConfig

  returns qfalse if error, qtrue otherwise
============
*/
static qboolean AnimParseAnimConfig( playerInfo_t *animModelInfo, const char *filename, const char *input ) {
	idLexer lexer( LEXFL_ONLYSTRINGS | LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWDOLLARNAMES | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS );
	idToken token, pending;
	qboolean havePending = qfalse;
	animation_t *animations;
	int i, fps, skip = -1;

	animations = animModelInfo->animations;
	animModelInfo->numAnimations = 0;

	lexer.LoadMemory( input, strlen( input ), filename );

	animModelInfo->footsteps = FOOTSTEP_NORMAL;
	animModelInfo->headOffset.Zero();
	animModelInfo->gender = GENDER_MALE;
	animModelInfo->isSkeletal = qfalse;
	animModelInfo->version = 0;

	// read optional parameters
	while ( 1 ) {
		if ( havePending ) {
			token = pending;
			havePending = qfalse;
		} else if ( !lexer.ReadToken( &token ) ) {
			break;
		}
		if ( token.Icmp( "footsteps" ) == 0 ) {
			if ( !lexer.ReadToken( &token ) ) {
				break;
			}
			if ( token.Icmp( "default" ) == 0 || token.Icmp( "normal" ) == 0 ) {
				animModelInfo->footsteps = FOOTSTEP_NORMAL;
			} else if ( token.Icmp( "boot" ) == 0 ) {
				animModelInfo->footsteps = FOOTSTEP_BOOT;
			} else if ( token.Icmp( "flesh" ) == 0 ) {
				animModelInfo->footsteps = FOOTSTEP_FLESH;
			} else if ( token.Icmp( "mech" ) == 0 ) {
				animModelInfo->footsteps = FOOTSTEP_MECH;
			} else if ( token.Icmp( "energy" ) == 0 ) {
				animModelInfo->footsteps = FOOTSTEP_ENERGY;
			} else {
//				BG_AnimParseError( "Bad footsteps parm '%s'\n", token );
			}
			continue;
		} else if ( token.Icmp( "headoffset" ) == 0 ) {
			for ( i = 0 ; i < 3 ; i++ ) {
				if ( !lexer.ReadToken( &token ) ) {
					break;
				}
				animModelInfo->headOffset[i] = atof( token.c_str() );
			}
			continue;
		} else if ( token.Icmp( "sex" ) == 0 ) {
			if ( !lexer.ReadToken( &token ) ) {
				break;
			}
			if ( token[0] == 'f' || token[0] == 'F' ) {
				animModelInfo->gender = GENDER_FEMALE;
			} else if ( token[0] == 'n' || token[0] == 'N' ) {
				animModelInfo->gender = GENDER_NEUTER;
			} else {
				animModelInfo->gender = GENDER_MALE;
			}
			continue;
		} else if ( token.Icmp( "version" ) == 0 ) {
			if ( !lexer.ReadToken( &token ) ) {
				break;
			}
			animModelInfo->version = atoi( token.c_str() );
			continue;
		} else if ( token.Icmp( "skeletal" ) == 0 ) {
			animModelInfo->isSkeletal = qtrue;
			continue;
		}

		if ( animModelInfo->version < 2 ) {
			// if it is a number, start parsing animations
			if ( idStr::CharIsNumeric( token[0] ) ) {
				pending = token;      // unget the token
				havePending = qtrue;
				break;
			}
		}

		// STARTANIMS marks the start of the animations
		if ( token.Icmp( "STARTANIMS" ) == 0 ) {
			break;
		}
//		BG_AnimParseError( "unknown token '%s'", token );
	}

	// read information for each frame
	for ( i = 0 ; ( animModelInfo->version > 1 ) || ( i < MAX_ANIMATIONS ) ; i++ ) {

		if ( havePending ) {
			token = pending;
			havePending = qfalse;
		} else if ( !lexer.ReadToken( &token ) ) {
			break;
		}

		if ( animModelInfo->version > 1 ) {   // includes animation names at start of each line

			if ( token.Icmp( "ENDANIMS" ) == 0 ) {   // end of animations
				break;
			}

			idStr::Copynz( animations[i].name, token.c_str(), sizeof( animations[i].name ) );
			// convert to all lower case
			idStr::Copynz( animations[i].name, idStr( animations[i].name ).ToLower().c_str(), sizeof( animations[i].name ) );
			//
			if ( !lexer.ReadTokenOnLine( &token ) || !token[0] ) {
//				BG_AnimParseError( "end of file without ENDANIMS" );
			}
		} else {
			// just set it to the equivalent animStrings[]
			idStr::Copynz( animations[i].name, animStrings[i], sizeof( animations[i].name ) );
			// convert to all lower case
			idStr::Copynz( animations[i].name, idStr( animations[i].name ).ToLower().c_str(), sizeof( animations[i].name ) );
		}

		animations[i].firstFrame = atoi( token.c_str() );

		if ( !animModelInfo->isSkeletal ) { // skeletal models dont require adjustment

			// leg only frames are adjusted to not count the upper body only frames
			if ( i == LEGS_WALKCR ) {
				skip = animations[LEGS_WALKCR].firstFrame - animations[TORSO_GESTURE].firstFrame;
			}
			if ( i >= LEGS_WALKCR ) {
				animations[i].firstFrame -= skip;
			}

		}

		if ( !lexer.ReadTokenOnLine( &token ) || !token[0] ) {
//			BG_AnimParseError( "end of file without ENDANIMS" );
		}
		animations[i].numFrames = atoi( token.c_str() );

		if ( !lexer.ReadTokenOnLine( &token ) || !token[0] ) {
//			BG_AnimParseError( "end of file without ENDANIMS: line %i" );
		}
		animations[i].loopFrames = atoi( token.c_str() );

		if ( !lexer.ReadTokenOnLine( &token ) || !token[0] ) {
//			BG_AnimParseError( "end of file without ENDANIMS: line %i" );
		}
		fps = atof( token.c_str() );
		if ( fps == 0 ) {
			fps = 1;
		}
		animations[i].frameLerp = 1000 / fps;
		animations[i].initialLerp = 1000 / fps;

		// movespeed
		if ( !lexer.ReadTokenOnLine( &token ) || !token[0] ) {
//			BG_AnimParseError( "end of file without ENDANIMS" );
		}
		animations[i].moveSpeed = atoi( token.c_str() );

		// animation blending
		if ( !lexer.ReadTokenOnLine( &token ) ) {    // must be on same line
			animations[i].animBlend = 0;
		} else {
			animations[i].animBlend = atoi( token.c_str() );
		}

		// calculate the duration
		animations[i].duration = animations[i].initialLerp
								 + animations[i].frameLerp * animations[i].numFrames
								 + animations[i].animBlend;

		// get the nameHash
		animations[i].nameHash = BG_StringHashValue( animations[i].name );

		if ( idStr::Cmpn( animations[i].name, "climb", 5 ) == 0 ) {
			animations[i].flags |= ANIMFL_LADDERANIM;
		}
		if ( strstr( animations[i].name, "firing" ) ) {
			animations[i].flags |= ANIMFL_FIRINGANIM;
			animations[i].initialLerp = 40;
		}

	}

	animModelInfo->numAnimations = i;

	if ( animModelInfo->version < 2 && i != MAX_ANIMATIONS ) {
//		BG_AnimParseError( "Incorrect number of animations" );
		return qfalse;
	}

	return qtrue;           // NERVE - SMF - blah
}

/*
======================
UI_ParseAnimationFile
======================
*/
static qboolean UI_ParseAnimationFile( const char *filename, playerInfo_t *pi ) {
	int len;
	char text[20000];
	fileHandle_t f;

	memset( pi->animations, 0, sizeof( animation_t ) * MAX_ANIMATIONS );

	// load the file
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		return qfalse;
	}
	if ( len >= ( sizeof( text ) - 1 ) ) {
		Com_Printf( "File %s too long\n", filename );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	AnimParseAnimConfig( pi, filename, text );
	return qtrue;
}


/*
==========================
UI_RegisterClientModelname
==========================
*/
int WM_getWeaponIndex();

qboolean UI_RegisterClientModelname( playerInfo_t *pi, const char *modelSkinName ) {
	char modelName[MAX_QPATH];
	char skinName[MAX_QPATH];
	char filename[MAX_QPATH];
	char        *slash;
	const char* backpack = NULL;
	const char* helmet = NULL;

	pi->torsoModel = 0;
	pi->headModel = 0;

	if ( !modelSkinName[0] ) {
		return qfalse;
	}

	idStr::Copynz( modelName, modelSkinName, sizeof( modelName ) );

	slash = strchr( modelName, '/' );
	if ( !slash ) {
		// modelName did not include a skin name
		idStr::Copynz( skinName, "default", sizeof( skinName ) );
	} else {
		idStr::Copynz( skinName, slash + 1, sizeof( skinName ) );
		// truncate modelName
		*slash = 0;
	}

	// NERVE - SMF - set weapon
	pi->weapon = (weapon_t)WM_getWeaponIndex();
	UI_PlayerInfo_SetWeapon( pi, pi->weapon );

	// NERVE - SMF - determine skin
	{
		const char *team;
		const char *playerClass;
		int var, teamval;

		teamval = trap_Cvar_VariableValue( "mp_team" );

		if ( teamval == 1 ) {
			team = "blue";
		} else {
			team = "red";
		}

		var = trap_Cvar_VariableValue( "mp_playerType" );

		if ( var == 0 ) {
			playerClass = "soldier";

			if ( teamval == 1 ) {
				backpack = "acc/backpack/backpack_sol.md3";
				helmet = "acc/helmet_american/sol.md3";
			} else {
				backpack = "acc/backpack/backpack_german_sol.md3";
				helmet = "acc/helmet_german/helmet_german_sol.md3";
			}
		} else if ( var == 1 )   {
			playerClass = "medic";

			if ( teamval == 1 ) {
				backpack = "acc/backpack/backpack_med.md3";
				helmet = "acc/helmet_american/med.md3";
			} else {
				backpack = "acc/backpack/backpack_german_med.md3";
				helmet = "acc/helmet_german/helmet_german_med.md3";
			}
		} else if ( var == 2 )   {
			playerClass = "engineer";

			if ( teamval == 1 ) {
				backpack = "acc/backpack/backpack_eng.md3";
				helmet = "acc/helmet_american/eng.md3";
			} else {
				backpack = "acc/backpack/backpack_german_eng.md3";
				helmet = "acc/helmet_german/helmet_german_eng.md3";
			}
		} else {
			playerClass = "lieutenant";

			if ( teamval == 1 ) {
				backpack = "acc/backpack/backpack_lieu.md3";
				helmet = "acc/helmet_american/lieu.md3";
			} else {
				backpack = "acc/backpack/backpack_german_lieu.md3";
				helmet = "acc/helmet_american/lieu.md3";
			}
		}

		strcpy( skinName, idStr::FormatString( "%s%s1", team, playerClass ) );
	}
	// -NERVE - SMF

//		idStr::Copynz( skinName, "bluesoldier1", sizeof( skinName ) );		// NERVE - SMF - make this work with wolf - TESTING!!!
//	}
//	else {
//		idStr::Copynz( skinName, "redsoldier1", sizeof( skinName ) );		// NERVE - SMF - make this work with wolf - TESTING!!!
//	}

	// load cmodels before models so filecache works

//	Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower.md3", modelName );
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/body.mds", modelName ); // NERVE - SMF - make this work with wolf
	pi->legsModel = trap_R_RegisterModel( filename );
	if ( !pi->legsModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

//	Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper.md3", modelName );
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/body.mds", modelName ); // NERVE - SMF - make this work with wolf
	pi->torsoModel = trap_R_RegisterModel( filename );
	if ( !pi->torsoModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/head.md3", modelName );
	pi->headModel = trap_R_RegisterModel( filename );
	if ( !pi->headModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	// NERVE - SMF - load backpack and helmet
	if ( backpack ) {
		pi->backpackModel = trap_R_RegisterModel( idStr::FormatString( "models/players/%s/%s", modelName, backpack ) );
	}

	if ( helmet ) {
		pi->helmetModel = trap_R_RegisterModel( idStr::FormatString( "models/players/%s/%s", modelName, helmet ) );
	}

	// if any skins failed to load, fall back to default
	if ( !UI_RegisterClientSkin( pi, modelName, skinName ) ) {
		if ( !UI_RegisterClientSkin( pi, modelName, "default" ) ) {
			Com_Printf( "Failed to load skin file: %s : %s\n", modelName, skinName );
			return qfalse;
		}
	}

	// load the animations
//----(SA) changing name of config file to avoid backwards or alternate compatibility confustion
//	Com_sprintf( filename, sizeof( filename ), "models/players/%s/animation.cfg", modelName );
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/wolfanim.cfg", modelName );
//----(SA) end
	if ( !UI_ParseAnimationFile( filename, pi ) ) {         // NERVE - SMF - make this work with wolf
		Com_Printf( "Failed to load animation file %s\n", filename );
		return qfalse;
	}

	return qtrue;
}


/*
===============
UI_PlayerInfo_SetModel
===============
*/
void UI_PlayerInfo_SetModel( playerInfo_t *pi, const char *model ) {
	memset( pi, 0, sizeof( *pi ) );
	UI_RegisterClientModelname( pi, model );
//	pi->weapon = WP_MACHINEGUN;
//	pi->weapon = WP_MP40;
	pi->currentWeapon = pi->weapon;
	pi->lastWeapon = pi->weapon;
	pi->pendingWeapon = (weapon_t)-1;
	pi->weaponTimer = 0;
	pi->chat = qfalse;
	pi->newModel = qtrue;
	UI_PlayerInfo_SetWeapon( pi, pi->weapon );
}


/*
===============
UI_PlayerInfo_SetInfo
===============
*/
void UI_PlayerInfo_SetInfo( playerInfo_t *pi, int legsAnim, int torsoAnim, const idVec3 &viewAngles, const idVec3 &moveAngles, weapon_t weaponNumber, qboolean chat ) {
	int currentAnim;
	weapon_t weaponNum;

	pi->chat = chat;

	// view angles
	pi->viewAngles = viewAngles;

	// move angles
	pi->moveAngles = moveAngles;

	if ( pi->newModel ) {
		pi->newModel = qfalse;

		jumpHeight = 0;
		pi->pendingLegsAnim = 0;
		UI_ForceLegsAnim( pi, legsAnim );
		pi->legs.yawAngle = viewAngles[YAW];
		pi->legs.yawing = qfalse;

		pi->pendingTorsoAnim = 0;
		UI_ForceTorsoAnim( pi, torsoAnim );
		pi->torso.yawAngle = viewAngles[YAW];
		pi->torso.yawing = qfalse;

		if ( weaponNumber != -1 ) {
			pi->weapon = weaponNumber;
			pi->currentWeapon = weaponNumber;
			pi->lastWeapon = weaponNumber;
			pi->pendingWeapon = (weapon_t)-1;
			pi->weaponTimer = 0;
			UI_PlayerInfo_SetWeapon( pi, pi->weapon );
		}

		return;
	}

	// weapon
	if ( weaponNumber == -1 ) {
		pi->pendingWeapon = (weapon_t)-1;
		pi->weaponTimer = 0;
	} else if ( weaponNumber != WP_NONE )   {
		pi->pendingWeapon = weaponNumber;
		pi->weaponTimer = dp_realtime + UI_TIMER_WEAPON_DELAY;
	}
	weaponNum = pi->lastWeapon;
	pi->weapon = weaponNum;

	if ( torsoAnim == BOTH_DEATH1 || legsAnim == BOTH_DEATH1 ) {
		torsoAnim = legsAnim = BOTH_DEATH1;
		pi->weapon = pi->currentWeapon = WP_NONE;
		UI_PlayerInfo_SetWeapon( pi, pi->weapon );

		jumpHeight = 0;
		pi->pendingLegsAnim = 0;
		UI_ForceLegsAnim( pi, legsAnim );

		pi->pendingTorsoAnim = 0;
		UI_ForceTorsoAnim( pi, torsoAnim );

		return;
	}

	// leg animation
	currentAnim = pi->legsAnim & ~ANIM_TOGGLEBIT;
	if ( legsAnim != LEGS_JUMP && ( currentAnim == LEGS_JUMP || currentAnim == LEGS_LAND ) ) {
		pi->pendingLegsAnim = legsAnim;
	} else if ( legsAnim != currentAnim )   {
		jumpHeight = 0;
		pi->pendingLegsAnim = 0;
		UI_ForceLegsAnim( pi, legsAnim );
	}

	// torso animation
	if ( torsoAnim == TORSO_STAND || torsoAnim == TORSO_STAND2 ) {
		if ( weaponNum == WP_NONE || weaponNum == WP_GAUNTLET ) {
			torsoAnim = TORSO_STAND2;
		} else {
			torsoAnim = TORSO_STAND;
		}
	}

	if ( torsoAnim == TORSO_ATTACK || torsoAnim == TORSO_ATTACK2 ) {
		if ( weaponNum == WP_NONE || weaponNum == WP_GAUNTLET ) {
			torsoAnim = TORSO_ATTACK2;
		} else {
			torsoAnim = TORSO_ATTACK;
		}
		pi->muzzleFlashTime = dp_realtime + UI_TIMER_MUZZLE_FLASH;
		//FIXME play firing sound here
	}

	currentAnim = pi->torsoAnim & ~ANIM_TOGGLEBIT;

	if ( weaponNum != pi->currentWeapon || currentAnim == TORSO_RAISE || currentAnim == TORSO_DROP ) {
		pi->pendingTorsoAnim = torsoAnim;
	} else if ( ( currentAnim == TORSO_GESTURE || currentAnim == TORSO_ATTACK ) && ( torsoAnim != currentAnim ) )   {
		pi->pendingTorsoAnim = torsoAnim;
	} else if ( torsoAnim != currentAnim )   {
		pi->pendingTorsoAnim = 0;
		UI_ForceTorsoAnim( pi, torsoAnim );
	}
}
