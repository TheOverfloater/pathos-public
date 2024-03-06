/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef LADDER_SHARED_H
#define LADDER_SHARED_H

// Basic ladder definitions
static const Float LADDER_STEP_TIME					= (30.0f/45.0f);
static const Float LADDER_STEP_SIZE					= 32;
static const Float LADDER_FORWARD_OFFSET			= 16;
static const Float LADDER_PIECE_HEIGHT				= 32;
static const Float PLAYER_ORIGIN_OFFSET				= 36;

// View interpolation timings
static const Float LADDER_ENTER_LERP_TIME			= 0.665;
static const Float LADDER_LEAVE_LERP_TIME			= 0.5;
static const Float LADDER_LEAVE_VIEW_LERP_TIME		= 0.5;

// Enter/leave timings
static const Float LADDER_ENTER_LEFT_TIME			= 2.34;
static const Float LADDER_ENTER_RIGHT_TIME			= 2.67;
static const Float LADDER_ENTER_BOTTOM_TIME			= 2;
static const Float LADDER_ENTER_TOP_TIME			= 4.67;

static const Float LADDER_LEAVE_BOTTOM_TIME			= 1;
static const Float LADDER_LEAVE_LEFT_TIME			= 1.67;
static const Float LADDER_LEAVE_RIGHT_TIME			= 1.67;
static const Float LADDER_LEAVE_TOP_TIME			= 2.34;

// Entry points
enum ladder_entrypoints_t
{
	LADDER_ENTRYPOINT_UNAVAILABLE = -1,
	LADDER_ENTER_TOP = 0,
	LADDER_ENTER_LEFT,
	LADDER_ENTER_RIGHT,
	LADDER_ENTER_BOTTOM
};

// Exit points
enum ladder_exitpoints_t
{
	LADDER_EXIT_UNAVAILABLE	= -1,
	LADDER_EXIT_TOP	= 0,
	LADDER_EXIT_LEFT,
	LADDER_EXIT_RIGHT,
	LADDER_EXIT_BOTTOM
};

// Exit offsets
static const Float LADDER_BT_EXIT_FW			= 32;
static const Float LADDER_RT_EXIT_RT			= -20;
static const Float LADDER_RT_EXIT_FW			= 25;
static const Float LADDER_LF_EXIT_RT			= 20;
static const Float LADDER_LF_EXIT_FW			= 25;
static const Float LADDER_TP_EXIT_FW			= -24;
static const Float LADDER_TP_EXIT_UP			= 100;

enum ladder_verify_codes_t
{
	LADDER_VR_NOMOVE = 0,
	LADDER_VR_MOVE_VALID,
	LADDER_VR_MOVE_EXIT_TOP,
	LADDER_VR_MOVE_EXIT_BOTTOM,
	LADDER_VR_MOVE_EXIT_USE,
};

enum ladder_move_state_t
{
	LADDER_RESET = 0,
	LADDER_RESTING,
	LADDER_MOVE_DOWN,
	LADDER_MOVE_UP
};

enum ladder_state_t
{
	LADDER_STATE_INACTIVE = 0,
	LADDER_STATE_ENTERING,
	LADDER_STATE_ACTIVE,
	LADDER_STATE_LEAVING,
	LADDER_STATE_CLEANUP,
	LADDER_STATE_CLEANUP_SV,
};

#endif