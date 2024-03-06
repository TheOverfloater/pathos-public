/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef LADDER_H
#define LADDER_H

#include "ladder_shared.h"
#include "cl_entity.h"

struct ref_params_t;
struct cl_entity_t;
struct cache_model_t;

enum ladder_side_t
{
	SIDE_UNFEDINED = -1,
	SIDE_LEFT = 1,
	SIDE_RIGHT
};

enum ladder_animstate_t
{
	LADDER_ANIM_NONE = 0,
	LADDER_ANIM_DOWN_LUP,
	LADDER_ANIM_DOWN_RUP,
	LADDER_ANIM_UP_LUP,
	LADDER_ANIM_UP_RUP,
	LADDER_ANIM_IDLE_LUP,
	LADDER_ANIM_IDLE_RUP,
	LADDER_ANIM_ENTER_BOTTOM,
	LADDER_ANIM_ENTER_LEFT,
	LADDER_ANIM_ENTER_RIGHT,
	LADDER_ANIM_ENTER_TOP,
	LADDER_ANIM_LEAVE_BOTTOM_LUP,
	LADDER_ANIM_LEAVE_BOTTOM_RUP,
	LADDER_ANIM_LEAVE_LEFT_LUP,
	LADDER_ANIM_LEAVE_LEFT_RUP,
	LADDER_ANIM_LEAVE_RIGHT_LUP,
	LADDER_ANIM_LEAVE_RIGHT_RUP,
	LADDER_ANIM_LEAVE_TOP_LUP,
	LADDER_ANIM_LEAVE_TOP_RUP,
	NUM_LADDER_ANIMS
};

/*
====================
CLadder

====================
*/
class CLadder
{
public:
	// Max angle deviation
	static const Float LADDER_V_MAX_ADD;
	// Max add on X
	static const Float LADDER_MAX_ADD_X;
	// Max add on Y
	static const Float LADDER_MAX_ADD_Y;
	// Min add on Y
	static const Float LADDER_MIN_ADD_Y;
	// Mouse move timeout
	static const Double LADDER_MOUSEMOVE_TIMEOUT;
	// Mouse move timeout blend time
	static const Double LADDER_MOUSEMOVE_TIMEOUT_BLEND;
	// Model name for view entity
	static const Char LADDER_VIEWMODEL_NAME[];
	// Sequence names for ladder
	static const Char* LADDER_SEQUENCE_NAMES[];

public:
	CLadder( void );
	~CLadder( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes for the game
	bool InitGame( void );
	// Clears the game states
	void ClearGame( void );

public:
	// Performs think functions
	void Think( void );
	// Sets up view
	void CalcRefDef( ref_params_t& params );
	
	// Processes a ladder message
	void ProcessMessage( const byte* pdata, Uint32 msgsize );

	// Tells if the ladder is active
	bool IsActive( void ) const;
	// Plays a step sound
	static void PlayStepSound( void );
	// Manages mouse movement
	void MouseMove( Float mousex, Float mousey );

	// Draws ladders
	bool Draw( void );
	// Draws ladders for VSM
	bool DrawVSM( cl_dlight_t *dl ) const;

	// Gets the view origin and angles
	void GetViewInfo( Vector& origin, Vector& angles );

private:
	// Draws a single ladder
	static bool DrawLadder( cl_entity_t *pLadder );
	// Draws a single ladder for vsm
	static bool DrawLadderVSM( cl_entity_t *pLadder, cl_dlight_t *dl );
	// Sets a sequence
	void SetSequence( const Char *psequence );

private:
	// Pointer to ladder entity
	cl_entity_t *m_pLadderEntity;
	// View hands
	cl_entity_t m_viewEntity;

	// Model used to render hands
	const cache_model_t *m_pViewModel;

	// Current time
	Float m_time;

private:
	// Current ladder state
	ladder_state_t m_activeLadderState;
	// Ideal ladder state
	ladder_state_t m_idealLadderState;
	// Time when we need to update
	Double m_updateTime;

	// Current animation state
	ladder_animstate_t m_activeAnimState;
	// Ideal ladder animation state
	ladder_animstate_t m_idealAnimState;

	// Current movement state
	ladder_move_state_t m_activeMovementState;
	// Ideal movement state
	ladder_move_state_t m_idealMovementState;

	// Exit point
	ladder_exitpoints_t m_exitPoint;
	// Entry point
	ladder_entrypoints_t m_entryPoint;
	// Side
	ladder_side_t m_side;

private:
	// View angles
	Vector m_viewAngles;
	// View origin
	Vector m_viewOrigin;

	// Angles on server
	Vector m_svAngles;
	// Origin on server
	Vector m_svOrigin;

	// View entity origin
	Vector m_viewentOrigin;
	// View entity angles
	Vector m_viewentAngles;

private:
	// View blend origin
	Vector m_viewBlendOrigin;
	// View blend angles
	Vector m_viewBlendAngles;

	// View blend begin time
	Double m_viewBlendBeginTime;
	// View blend time delta
	Double m_viewBlendDelta;

	// Tells if view blending is active
	bool m_viewBlend;

private:
	// View entity blend origin
	Vector m_viewEntBlendOrigin;

	// View entity blend begin time
	Double m_viewEntBlendBeginTime;
	// View entity blend delta
	Double m_viewEntBlendDelta;
	// View exit blend time
	Float m_exitBlend;

	// Tells if view entity is being blended
	bool m_viewEntBlend;

private:
	// Deviation angles
	Vector	m_deviationAngles;
	// Add deviation angles
	Vector	m_addDeviationAngles;
	// Last mouse move time
	Double	m_lastMouseMove;

private:
	// Draw entities cvar
	CCVar* m_pCvarDrawEntities;
};
extern CLadder gLadder;
#endif