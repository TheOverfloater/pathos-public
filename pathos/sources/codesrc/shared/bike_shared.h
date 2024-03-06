/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef BIKE_SHARED_H
#define BIKE_SHARED_H

static const Double BIKE_ENTER_TIME							= 0.75;	// time for lerping in
static const Double BIKE_LEAVE_TIME							= 1;	// time it takes to lerp out

static const Float MOTORBIKE_ACCELERATION					= 150;
static const Float MOTORBIKE_DECELERATION					= 200;
static const Float MOTORBIKE_BRAKE_DECELERATION				= 650;

static const Float MOTORBIKE_FATAL_COLLISON_SPEED			= 650;
static const Float MOTORBIKE_MINIMUM_DAMAGE_COLLISON_SPEED	= 180;
static const Float MOTORBIKE_MINIMUM_KNOCK_SPEED			= 30;

static const Float MOTORBIKE_FRONT_WHEEL_DISTANCE			= 48;
static const Float MOTORBIKE_BACK_WHEEL_DISTANCE			= 24;

enum bike_anims_t
{
	BIKE_ANIM_STANDBY = 0,
	BIKE_ANIM_ENTER,
	BIKE_ANIM_EXIT,
	BIKE_ANIM_IDLE,
	BIKE_ANIM_FORWARD,
	BIKE_ANIM_TURNLEFT,
	BIKE_ANIM_LEFT,
	BIKE_ANIM_LEFTBACK,
	BIKE_ANIM_TURNRIGHT,
	BIKE_ANIM_RIGHT,
	BIKE_ANIM_RIGHTBACK,
	BIKE_ANIM_TRASHED,
	NB_BIKE_ANIMS
};

enum bike_sv_states_t
{
	BIKE_SV_INACTIVE = 0,
	BIKE_SV_ENTERING_LERP,
	BIKE_SV_ENTERING,
	BIKE_SV_ACTIVE,
	BIKE_SV_LEAVING,
	BIKE_SV_LEAVING_LERP,
	BIKE_SV_CLEANUP,
	BIKE_SV_RESTORE
};
#endif
