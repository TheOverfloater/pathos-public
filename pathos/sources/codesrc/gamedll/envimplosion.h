/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVIMPLOSION_H
#define ENVIMPLOSION_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvImplosion : public CPointEntity
{
public:
	// Default tracer color
	static const Vector DEFAULT_COLOR;

public:
	enum
	{
		FL_REVERSE_DIRECTION	= (1<<0),
		FL_REVERSE_INTENSITY	= (1<<1)
	};

public:
	explicit CEnvImplosion( edict_t* pedict );
	virtual ~CEnvImplosion( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
private:
	void EXPORTFN SpawnThink( void );

private:
	Float m_radius;
	Uint32 m_tracerCount;
	Float m_life;
	Float m_duration;
	Double m_lastSpawnTime;
	Uint32 m_spawnCount;
	Double m_spawnBeginTime;
	Float m_tracerLength;
	Float m_tracerWidth;
};
#endif //ENVIMPLOSION_H