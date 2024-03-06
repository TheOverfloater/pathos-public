/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCROTATING_H
#define FUNCROTATING_H

// Number of fan sound options
static const Uint32 NUM_FAN_SOUND_OPTIONS = 6;

//=============================================
//
//=============================================
class CFuncRotating : public CBaseEntity
{
public:
	// Array of fan sounds
	static const Char* g_pFanSounds[NUM_FAN_SOUND_OPTIONS];

public:
	// Fan minimum pitch
	static const Float FAN_MIN_PITCH;
	// Fan maximum pitch
	static const Float FAN_MAX_PITCH;

public:
	enum
	{
		FL_ROTATE_Y_AXIS			= 0,
		FL_ROTATE_START_ON			= (1<<0),
		FL_ROTATE_REVERSE			= (1<<1),
		FL_ROTATE_Z_AXIS			= (1<<2),
		FL_ROTATE_X_AXIS			= (1<<3),
		FL_ACCELERATE_DECELERATE	= (1<<4),
		FL_INFLICT_DAMAGE			= (1<<5),
		FL_ROTATING_NOT_SOLID		= (1<<6),
		FL_SND_SMALL_RADIUS			= (1<<7),
		FL_SND_MEDIUM_RADIUS		= (1<<8),
		FL_SND_LARGE_RADIUS			= (1<<9)
	};

	enum fanstate_t
	{
		FAN_STATE_OFF = 0,
		FAN_STATE_SPINDOWN,
		FAN_STATE_SPINUP,
		FAN_STATE_ON
	};

public:
	explicit CFuncRotating( edict_t* pedict );
	virtual ~CFuncRotating( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallBlocked( CBaseEntity* pOther ) override;
	virtual void CallUse( CBaseEntity* pacticator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
	virtual void DeclareSaveFields( void ) override;
	virtual void InitEntity( void ) override;

public:
	void RampPitchAndVolume( void );

public:
	void EXPORTFN SpinUpThink( void );
	void EXPORTFN SpinDownThink( void );
	void EXPORTFN HurtTouch( CBaseEntity* pOther );
	void EXPORTFN RotateThink( void );

protected:
	Float m_fanFriction;
	Float m_fanAttenuation;
	Float m_volume;
	Float m_pitch;
	Float m_damage;
	Int32 m_sounds;
	Double m_lastThinkTime;
	Int32 m_fanState;

	string_t m_fanSound;
};
#endif //FUNCWALL_H