/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENCPARTICLEEFFECT_H
#define ENCPARTICLEEFFECT_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvParticleEffect : public CPointEntity
{
public:
	// Max colors that can be addressed
	static const Uint32 MAX_COLORS;

	// Spawnflags
	enum
	{
		FL_START_ON = (1<<0)
	};

public:
	enum fxtype_t
	{
		EFFECT_PARTICLEEXPLOSION1 = 0,
		EFFECT_PARTICLEEXPLOSION2,
		EFFECT_BLOBEXPLOSION,
		EFFECT_ROCKETEXPLOSION,
		EFFECT_PARTICLEEFFECT,
		EFFECT_LAVASPLASH,
		EFFECT_TELEPORTSPLASH,
		EFFECT_ROCKETTRAIL,
		NB_EFFECT_TYPES
	};

public:
	explicit CEnvParticleEffect( edict_t* pedict );
	virtual ~CEnvParticleEffect( void );

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

private:
	// Beam type
	Int32 m_type;
	// Start color
	Int32 m_startColor;
	// End color
	Int32 m_endColor;
	// Particle count
	Int32 m_particleCount;
	// Rocket trail type
	Int32 m_trailType;
	// Min repeat delay
	Float m_minRepeatDelay;
	// Max repeat delay
	Float m_maxRepeatDelay;
	// True if we're actively repeating
	bool m_isActive;
};
#endif //ENCPARTICLEEFFECT_H