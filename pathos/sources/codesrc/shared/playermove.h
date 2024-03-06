/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef NETWORKING_H
#define NETWORKING_H

#include "usercmd.h"
#include "pm_shared.h"
#include "textures_shared.h"

struct cache_model_t;
struct en_material_t;
enum cmodel_type_t;

// Waterjump height
static const Float WATERJUMP_HEIGHT = 8.0f;
// Waterjump wait time
static const Float WATERJUMP_WAIT_TIME = 2000; // two seconds
// Waterjump wait time
static const Float PLAYER_DUCK_WAIT_TIME = 1000; // Time until we can duck again
// Waterjump wait time
static const Float PLAYER_DUCK_TIME = 400; // Time until we can duck again
// Waterjump wait time
static const Float SWIM_SOUND_DELAY = 1000; // two seconds
// Number of bumps with flymove
static const Uint32 NUM_FLYMOVE_BUMPS = 4;
// Max planes we can collide against
static const Uint32 MAX_CLIP_PLANES = 5;
// Player fall treshold
static const Float PLAYER_FALL_TRESHOLD = 350.0f;
// Player's safe falling speed
static const Float PLAYER_SAFE_FALL_SPEED = 580.0f;
// Player's climbing speed
static const Float PLAYER_CLIMB_SPEED = 100.0f;
// Player downward drift speed in water
static const Float PLAYER_DOWNWARD_DRIFT_SPEED = 10.0f;
// Default plane normal treshold for being onground
static const Float DEFAULT_ONGROUND_LOWER_CAP = 0.7;

struct pm_interface_t
{
	void					(*pfnCon_Printf)( const Char *fmt, ... );
	void					(*pfnCon_DPrintf)( const Char *fmt, ... );
	void					(*pfnCon_VPrintf)( const Char *fmt, ... );
	void					(*pfnCon_EPrintf)( const Char *fmt, ... );

	Double					(*pfnGetTime)( void );

	cmodel_type_t			(*pfnGetModelType)( const cache_model_t& model );
	void					(*pfnGetModelBounds)( const cache_model_t& model, Vector& mins, Vector& maxs );
	const cache_model_t*	(*pfnGetModel)( Int32 modelindex );

	Int32					(*pfnGetNumEntities)( void );
	const entity_state_t*	(*pfnGetEntityState)( entindex_t entindex );

	const en_material_t*	(*pfnGetMapTextureMaterialScript)( const Char* pstrTextureName );

	void					(*pfnPlaySound)( entindex_t entindex, Int32 channel, const Char* psample, Float volume, Float attenuation, Int32 pitch, Int32 flags );
	void					(*pfnPlayStepSound)( entindex_t entindex, const Char* pstrMaterialName, bool stepleft, Float volume, const Vector& origin );
	void					(*pfnAddToTouched)( entindex_t hitent, trace_t& trace, const Vector& velocity );
};

/*
=======================
CPlayerMovement

=======================
*/
class CPlayerMovement
{
public:
	enum blockflags_t
	{
		BLOCKED_NOT = (1<<0),
		BLOCKED_FLOOR = (1<<1),
		BLOCKED_WALL = (1<<2)
	};

public:
	CPlayerMovement();
	~CPlayerMovement();

public:
	// Initializes function pointers, etc
	void Init( const trace_interface_t& traceFuncs, const pm_interface_t& pmFuncs, bool isServer );
	// Resets the class after game exit
	void Reset( void );
	
	// Runs movement logic for a player
	void RunMovement( const usercmd_t& cmd, pm_info_t* pminfo, bool playSounds, bool isMultiplayer );
	// Perform playermove operations
	void RunPlayerMove( void );

private:
	// Checks if the player is stuck
	bool CheckStuck( void );
	// Sets entities in traceline class
	void AddGravity( Float multiplier = 1.0 );
	// Sets blocked flags
	static Int32 ClipVelocity( const Vector& in, const Vector& normal, Vector& out, Float overbounce );

	// Checks water levels
	bool CheckWater( void );
	// Checks water jumping
	bool CheckWaterJump( bool dojump );
	// Do watermove stuff
	void WaterJump( void );
	// Checks velocity values
	void CheckVelocity( void );
	// Checks and adjusts parameters sent
	void CheckParameters( void );

	// Plays any step sounds
	void UpdateStepSound( void );
	// Checks if we can un-duck
	bool CanUnDuck( void );
	// Un-crouches player
	void UnDuck( void );
	// Manages crouching
	void Duck( void );
	// Friction
	void Friction( void );
	// Plays falling sounds
	void CheckFalling( void );
	// Budges player when crouching
	void FixCrouchStuck( const Vector& direction );

	// Calculates ground acceleration
	void Accelerate( const Vector& wishdir, Float wishspeed, Float acceleration );
	// Calculates air acceleration
	void AirAccelerate( const Vector& wishdir, Float wishspeed, Float acceleration );

	// Drops punch angle
	void DropPunchAngle( void );
	// Reduces timers
	void ReduceTimers( void );
	// Determines waterlevel, onground
	void CategorizePosition( void );
	// Fixes gravity on entity
	void FixupGravityVelocity( void );
	// Checks if the player is touching any ladders
	const entity_state_t* GetLadder( void );

	// Manages any impacts
	void PushEntity( const Vector& push, trace_t& trace );

	// Perform noclip movement
	void Move_Noclip( void );
	// Performs ladder movement
	void Move_Ladder( void );
	// Toss/Bounce movement
	void Move_Toss_Bounce( void );
	// Flying movement
	Int32 Move_Fly( void );
	// Water movement
	void Move_Water( void );
	// Air movement
	void Move_Air( void );
	// Walking movement
	void Move_Walk( void );
	// Move when FL_FROZEN is set
	void Move_Frozen( void );

	// Performs jumping
	void Jump( void );
	// Prevents mega-bunnyjumping
	void PreventMegaBunnyJumping( void );

	// Determines what texture we're on
	void DetermineTextureType( void );

private:
	// Current frametime
	Float m_frameTime;
	// Playermove info structure
	pm_info_t* m_pPMInfo;

	// Client state
	entity_state_t* m_pPlayerState;

	// Forward vector of angles
	Vector m_vForward;
	// Right vector of angles
	Vector m_vRight;
	// Up vector of angles
	Vector m_vUp;

	// Current hull index
	hull_types_t m_hullIndex;
	// Max speed
	Float m_maxSpeed;
	// Max forward speed
	Float m_maxForwardSpeed;
	// Previous water level
	Int32 m_oldWaterLevel;

	// Plane Z cap
	Float m_planeZCap;

	// TRUE if we're on a ladder
	bool m_isOnLadder;
	// Ladder entity
	const entity_state_t* m_pLadder;
	// TRUE if running on server side
	bool m_isServer;
	// Tells if sounds should be played
	bool m_playSounds;
	// TRUE if multiplayer
	bool m_isMultiplayer;

	// Material name for the texture
	const en_material_t* m_pTextureMaterial;
	// Default material script
	en_material_t m_defaultMaterial;

private:
	// Trace function interface
	trace_interface_t m_traceInterface;
	// Playermove interface
	pm_interface_t m_pmInterface;
	// Usercmd being processed
	usercmd_t m_userCmd;
	// Movevars pointer
	movevars_t* m_pMovevars;
};

#endif //NETWORKING_H