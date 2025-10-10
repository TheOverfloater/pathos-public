/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envimplosion.h"

// Default tracer color
const Vector CEnvImplosion::DEFAULT_COLOR = Vector(204, 204, 102);

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_implosion, CEnvImplosion);

//=============================================
// @brief
//
//=============================================
CEnvImplosion::CEnvImplosion( edict_t* pedict ):
	CPointEntity(pedict),
	m_radius(0),
	m_tracerCount(0),
	m_life(0),
	m_duration(0),
	m_lastSpawnTime(0),
	m_spawnCount(0),
	m_spawnBeginTime(0),
	m_tracerLength(0),
	m_tracerWidth(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvImplosion::~CEnvImplosion( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvImplosion::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvImplosion, m_radius, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvImplosion, m_tracerCount, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvImplosion, m_life, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvImplosion, m_duration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvImplosion, m_lastSpawnTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvImplosion, m_spawnCount, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvImplosion, m_spawnBeginTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvImplosion, m_tracerLength, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvImplosion, m_tracerWidth, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CEnvImplosion::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "effectradius"))
	{
		m_radius = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "count"))
	{
		Int32 value = SDL_atoi(kv.value);
		if(value <= 0)
		{
			Util::EntityConPrintf(m_pEdict, "Invalid setting %d for field '%s'.\n", value, kv.keyname);
			value = 1;
		}

		m_tracerCount = value;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "life"))
	{
		m_life = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "duration"))
	{
		m_duration = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "tracerlength"))
	{
		m_tracerLength = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "tracerwidth"))
	{
		m_tracerWidth = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvImplosion::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pState->renderamt <= 0)
		m_pState->renderamt = 255;

	if(m_pState->rendercolor.IsZero())
		m_pState->rendercolor = DEFAULT_COLOR;

	if(m_tracerLength <= 0)
		m_tracerLength = 1.5;

	if(m_tracerWidth <= 0)
		m_tracerWidth = 0.8;

	if(m_life <= 0)
		m_life = 1.5;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvImplosion::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(m_duration > 0)
	{
		m_spawnBeginTime = g_pGameVars->time;
		m_lastSpawnTime = g_pGameVars->time;
		m_spawnCount = 0;

		SetThink(&CEnvImplosion::SpawnThink);
		m_pState->nextthink = g_pGameVars->time;
	}
	else
	{
		bool reverse = HasSpawnFlag(FL_REVERSE_DIRECTION) ? true : false;
		Util::CreateTracerImplosion(m_pState->origin, m_radius, m_tracerCount, m_life, m_pState->rendercolor, m_pState->renderamt, m_tracerWidth, m_tracerWidth, reverse);
	}
}

//=============================================
// @brief
//
//=============================================
VOID CEnvImplosion::SpawnThink( void )
{
	Uint32 spawnRate = m_tracerCount / m_duration;
	Double intervalTime = g_pGameVars->time - m_lastSpawnTime;
	Uint32 nbSpawn = intervalTime * spawnRate;

	if(nbSpawn > 0)
	{
		Float durFraction = (g_pGameVars->time - m_spawnBeginTime) / m_duration;
		Float life;
		if(!HasSpawnFlag(FL_REVERSE_INTENSITY))
			life = m_life * (1.0 - durFraction);
		else
			life = m_life * durFraction;

		if(life < 0.5)
			life = 0.5;

		if((m_spawnCount + nbSpawn) > m_tracerCount)
			nbSpawn = m_tracerCount - m_spawnCount;

		bool reverse = HasSpawnFlag(FL_REVERSE_DIRECTION) ? true : false;
		Util::CreateTracerImplosion(m_pState->origin, m_radius, nbSpawn, life, m_pState->rendercolor, m_pState->renderamt, m_tracerWidth, m_tracerWidth, reverse);

		m_spawnCount += nbSpawn;
		m_lastSpawnTime = g_pGameVars->time;
	}

	if(m_spawnCount >= m_tracerCount)
	{
		// Disable think
		SetThink(nullptr);
		m_pState->nextthink = 0;

		// Reset these
		m_spawnCount = 0;
		m_lastSpawnTime = 0;
	}
	else
	{
		SetThink(&CEnvImplosion::SpawnThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
}