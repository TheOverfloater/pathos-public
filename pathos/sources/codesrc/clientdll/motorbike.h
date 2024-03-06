/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MOTORBIKE_H
#define MOTORBIKE_H

struct ref_params_t;
class CCVar;

#include "cl_entity.h"
#include "bike_shared.h"

enum bike_states_t
{
	BIKE_STATE_STANDBY = 0,
	BIKE_STATE_ENTER_LERP,
	BIKE_STATE_ENTER,
	BIKE_STATE_LEAVE,
	BIKE_STATE_LEAVE_LERP,
	BIKE_STATE_READY,
	BIKE_STATE_FORWARD,
	BIKE_STATE_BACK,
	BIKE_STATE_TURNLEFT,
	BIKE_STATE_LEFT,
	BIKE_STATE_LEFTBACK,
	BIKE_STATE_TURNRIGHT,
	BIKE_STATE_RIGHT,
	BIKE_STATE_RIGHTBACK
};

enum bike_sounds_t
{
	BIKE_SOUND_NONE = 0,
	BIKE_SOUND_START,
	BIKE_SOUND_OFF,
	BIKE_SOUND_ACCEL_BEGIN,
	BIKE_SOUND_RUN,
	BIKE_SOUND_DECELERATE,
	BIKE_SOUND_IDLE
};

/*
====================
CMotorBike

====================
*/
class CMotorBike
{
public:
	// Turn angle
	static const Float MOTORBIKE_TURN_ANGLE;
	// View max deviation
	static const Float MOTORBIKE_MAX_ADD;
	// Mouse move timeout
	static const Float MOUSEMOVE_TIMEOUT;
	// Mouse move timeout blend time
	static const Float MOUSEMOVE_TIMEOUT_BLEND;
	// Surface align time
	static const Double ANGLE_ALIGN_TIME_ON_GROUND;
	// Surface align time
	static const Double ANGLE_ALIGN_TIME_IN_AIR;
	// Wheel blend time
	static const Double WHEEL_BLEND_TIME;
	// Motorbike volume
	static const Float MOTORBIKE_VOLUME;
	// Motorbike model name
	static const Char MOTORBIKE_MODEL_NAME[];
	// Motorbike sequence names(this needs to line up with bike_anims_t
	static const Char* MOTORBIKE_ANIM_NAMES[NB_BIKE_ANIMS];

public:
	CMotorBike( void );
	~CMotorBike( void );

public:
	// Initializes the class
	bool Init ( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes for game
	bool InitGame( void );
	// Clears game states
	void ClearGame( void );

public:
	// Tells if the motorbike is active
	bool IsActive( void ) const;
	// Returns the motorbike entity
	cl_entity_t *GetBikeEntity ( void );

	// Returns the turn amount
	Float GetTurnAmount( void );
	// Returns the acceleration
	Float GetAcceleration( void ) const;

	// Calculares view
	void CalcRefDef( ref_params_t& params );
	// Draws the bike
	bool Draw( void );

	// Performs think functions
	void Think( void );
	// Handles mouse movement
	void MouseMove( Float mousex, Float mousey );

public:
	// Processes a motorbike usermsg
	void ProcessMessage( const byte* pdata, Uint32 msgsize );

	// Gets the view origin and angles
	void GetViewInfo( Vector& origin, Vector& angles );

private:
	// Sets the sequence
	void SetSequence( Int32 sequence );
	// Returns the sequence time
	Float SequenceTime( Int32 sequence ) const;

	// Handles input
	void HandleInput( void );
	// Drops the punch angle
	void DropPunchAngle ( void );

	// Restores the bike entity
	void RestoreBike( void );
	// Updates sequence states
	void UpdateSequenceStates( void );
	// Updates sound states
	void UpdateSoundStates( void );

	// Handles states
	void HandleStates( void );
	// Performs thinking while active
	void ActiveThink( void );

	// Updates turning angles
	void UpdateTurnAngles ( void );
	// Updates flashlight
	void SetupFlashlight ( void );

	// Updates ground orientation
	void UpdateAngles( void );
	// Calculates ground angles
	void GetIdealUpVector( Vector& outup, Double* pblendtime );
	
private:
	// The entity which we render
	cl_entity_t			m_clientBikeEntity;

	// Pointer to engine bike entity
	cl_entity_t			*m_pBikeEntity;
	// Local player
	cl_entity_t			*m_pPlayer;

	// Our model
	const cache_model_t	*m_pBikeModel;

	// Current time
	Double				m_time;

	// Cvar for turn roll
	CCVar*				m_pCvarTurnRoll;

private:
	// Actual acceleration
	Float				m_acceleration;

	// For direction estimation
	Double				m_lastInputTime;

	// Previous ground entity
	Int32				m_prevPlayerGroundEntity;

	// handles exiting/entering and main state
	bike_sv_states_t	m_serverStatus;
	bike_sv_states_t	m_activeServerStatus;
	Double				m_nextUpdateTime;

	// animation state at the momment
	bike_states_t		m_animState;
	bike_states_t		m_idealAnimState;
	Double				m_nextSequenceTime;

	// should be taken from vuser
	Vector				m_exitOrigin;
	Vector				m_exitAngles;


	// Actual angles and origin
	Vector				m_viewAngles;
	Vector				m_viewOrigin;

	Vector				m_savedVelocity;

	Vector				m_punchAngle;
	Vector				m_punchAmount;

	// Interpolation
	bool				m_isViewSaved;
	Vector				m_lerpBeginOrigin;
	Vector				m_lerpBeginAngles;

	Vector				m_lerpEndAngles;
	Vector				m_lerpEndOrigin;

	Double				m_lerpBeginTime;
	Double				m_lerpEndTime;

	// Sounds
	Double				m_soundTime;
	bike_sounds_t		m_soundState;

	Float				m_direction;
	Float				m_prevDirection;

	Float				m_wheelAngle;
	Float				m_prevWheelAngle;

	bool				m_playBrakeSound;

	Int32				m_prevForwardState;
	Int32				m_prevBackState;

	// For turning around in our limited cone
	Vector				m_deviationAngles;
	Vector				m_addDeviationAngles;
	Double				m_lastMouseMove;

	// Variables for ground orientation
	Vector				m_currentBikeUpVector;
	Vector				m_idealBikeUpVector;
	Vector				m_prevBikeUpVector;

	Double				m_angleBlendTime;
	Double				m_angleBlendDuration;
};
extern CMotorBike gMotorBike;
#endif