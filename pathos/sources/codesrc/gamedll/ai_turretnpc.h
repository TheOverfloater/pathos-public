/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef AI_TURRETNPC_H
#define AI_TURRETNPC_H

//
// I'd like to thank Valve for the AI code in the Half-Life SDK,
// which I used as a reference while writing this implementation
//

#include "ai_basenpc.h"

class CEnvSprite;

//=============================================
//
//=============================================
class CTurretNPC : public CBaseNPC
{
public:
	enum
	{
		FL_TURRET_AUTO_ACTIVATE		= (1<<5),
		FL_TURRET_START_INACTIVE	= (1<<6)
	};

	enum turret_animations_t
	{
		TURRET_ANIM_NONE = 0,
		TURRET_ANIM_FIRE,
		TURRET_ANIM_SPIN,
		TURRET_ANIM_DEPLOY,
		TURRET_ANIM_RETIRE,
		TURRET_ANIM_DIE
	};

	enum turret_orientation_t
	{
		ORIENTATION_FLOOR = 0,
		ORIENTATION_CEILING
	};

	enum spin_state_t
	{
		SPIN_STATE_DOWN = 0,
		SPIN_STATE_UP
	};

	enum turret_sounds_t
	{
		TURRET_SND_FIRE = 0,
		TURRET_SND_PING,
		TURRET_SND_ACTIVATE,
		TURRET_SND_DIE1,
		TURRET_SND_DIE2,
		TURRET_SND_DIE3,
		TURRET_SND_DEPLOY,
		TURRET_SND_SPINUP,
		TURRET_SND_SPINDOWN,
		TURRET_SND_SEARCH,
		TURRET_SND_ALERT,

		NB_TURRET_SOUNDS
	};
	
public:
	// Array of turret sounds
	static const Char* TURRET_SOUNDS[NB_TURRET_SOUNDS];
	// Turret glow sprite name
	static const Char TURRET_GLOW_SPRITE_NAME[];

	// Metal impact sound pattern
	static const Char TURRET_METAL_IMPACT_SOUND_PATTERN[];
	// Number of Metal impact sounds
	static const Uint32 TURRET_NB_METAL_IMPACT_SOUNDS;
	// Metal impact particle script
	static const Char TURRET_METAL_IMPACT_PARTICLE_SCRIPT[];
	// Impact decal name
	static const Char TURRET_IMPACT_DECAL_NAME[];
	// Maximum firing range of turret
	static const Float TURRET_MAX_RANGE;
	// Default turn rate
	static const Float TURRET_DEFAULT_TURN_RATE;
	// Default max wait time
	static const Float TURRET_DEFAULT_MAX_WAIT_TIME;

public:
	explicit CTurretNPC( edict_t* pedict );
	virtual ~CTurretNPC( void );

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Performs precache functions
	virtual void Precache( void ) override;
	// Manages a keyvalue
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	// Declares save-restore fields
	virtual void DeclareSaveFields( void ) override;

	// Makes the entity take on damage
	virtual bool TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags ) override;
	// Handles damage calculation for a hitscan
	virtual void TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags ) override;

	// Returns the classification
	virtual Int32 GetClassification( void ) const override;
	// Returns blood color setting
	virtual bloodcolor_t GetBloodColor( void ) override { return BLOOD_NONE; }
	// Gibs the NPC
	virtual void GibNPC( void ) override { };

	// Sets the ideal yaw speed
	virtual void SetYawSpeed( void ) override { };
	// Returns the sound mask for the NPC
	virtual Uint64 GetSoundMask( void ) override { return AI_SOUND_NONE; };

public:
	// Calls use function
	void EXPORTFN TurretUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value );

	// Called when active
	void EXPORTFN ActiveThink( void );
	// Called when searching for enemies
	void EXPORTFN SearchThink( void );
	// Called when auto searching for enemies
	void EXPORTFN AutoSearchThink( void );
	// Called when turret dies
	void EXPORTFN TurretDeathThink( void );

	// Called when spinning down
	void EXPORTFN SpinDownCall( void );
	// Called when spinning up
	void EXPORTFN SpinUpCall( void );

	// Called when deploying
	void EXPORTFN DeployThink( void );
	// Called when retiring
	void EXPORTFN RetireThink( void );
	// Called when initializing
	void EXPORTFN InitializeThink( void );

public:
	// Called to play ping sound
	virtual void Ping( void );
	// Called to turn eye on
	virtual void EyeOn( void );
	// Called to turn eye off
	virtual void EyeOff( void );
	// Called on initialization
	virtual void InitializeTurret( void ) { };

	// Sets the turret animation
	void SetAnimation( turret_animations_t animation );
	// Called when turret moves
	bool MoveTurret( void );
	// Called when the turret dies
	void Die( void );

public:
	// Called when firing
	virtual void TurretShoot( const Vector& shootOrigin, const Vector& enemyDirection) = 0;

protected:
	// Maximum time to spin the barrel without targets
	Float m_maxSpinTime;
	// Spin state
	Int32 m_spinState;

	// Turret eye glow sprite
	CEnvSprite* m_pEyeGlowSprite;
	// Eye brightness
	Int32 m_eyeBrightness;

	// Deploy height
	Float m_deployHeight;
	// Retract height
	Float m_retractHeight;
	// Minimum pitch
	Int32 m_minPitch;

	// Base turn rate
	Int32 m_baseTurnRate;
	// Turn rate
	Float m_turnRate;
	// Orientation
	Int32 m_orientation;
	// TRUE if on
	bool m_isOn;
	// TRUE if going berserk
	bool m_isBerserk;
	// TRUE if we should auto-start
	bool m_shouldAutoStart;

	// Last sight position
	Vector m_lastSightPosition;
	// Last sight time
	Double m_lastSightTime;
	// Max wait time
	Float m_maxWaitTime;
	// Search speed
	Int32 m_searchSpeed;

	// Start yaw
	Float m_startYaw;
	// Current angles
	Vector m_currentAngles;
	// Goal angles
	Vector m_goalAngles;

	// Ping time
	Double m_pingTime;
	// Spinup time
	Double m_spinUpTime;
	// Last time we took damage
	Double m_damageTime;
};
#endif //AI_TURRETNPC_H