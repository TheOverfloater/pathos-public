/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENCTRACEREFFECT_H
#define ENCTRACEREFFECT_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvTracerEffect : public CPointEntity
{
public:
	// Default tracer color
	static const Vector DEFAULT_COLOR;

public:
	// Spawnflags
	enum
	{
		FL_START_ON			= (1<<0)
	};

	// Tracer gravity settings
	enum tracergravity_t
	{
		TRACER_GRAVITY_NONE = 0,
		TRACER_GRAVITY_NORMAL,
		TRACER_GRAVITY_SLOW
	};

public:
	enum fxtype_t
	{
		EFFECT_TRACER = 0,
		EFFECT_SPARKSTREAK,
		EFFECT_STREAKSPLASH,

		// MUST BE LAST
		NB_EFFECT_TYPES
	};

public:
	explicit CEnvTracerEffect( edict_t* pedict );
	virtual ~CEnvTracerEffect( void );

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Calls for classes and their children
	virtual void DeclareSaveFields( void ) override;
	// Manages keyvalues
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	// Calls use function
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

public:
	// Repeat function
	void EXPORTFN RepeatThink( void );
	// Spawns the desired effect
	void CreateEffect( void );
	// Get direction
	Vector GetDirection( Float* plength );

private:
	// Beam type
	Int32 m_type;
	// Particle count
	Int32 m_tracerCount;
	// Min repeat delay
	Float m_minRepeatDelay;
	// Max repeat delay
	Float m_maxRepeatDelay;
	// Min velocity
	Float m_minVelocity;
	// Max velocity
	Float m_maxVelocity;
	// Minimum tracer lifetime
	Float m_minLifetime;
	// Minimum tracer lifetime
	Float m_maxLifetime;
	// Tracer length
	Float m_tracerLength;
	// Tracer width
	Float m_tracerWidth;
	// Gravity type
	Uint32 m_tracerGravityType;
	// True if we're actively repeating
	bool m_isActive;
};
#endif //ENCPARTICLEEFFECT_H