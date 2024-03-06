/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envspark.h"

// Default delay value
const Float CEnvSpark::DEFAULT_DELAY_TIME = 1.5;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_spark, CEnvSpark);

//=============================================
// @brief
//
//=============================================
CEnvSpark::CEnvSpark( edict_t* pedict ):
	CPointEntity(pedict),
	m_isActive(false),
	m_delay(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSpark::~CEnvSpark( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvSpark::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSpark, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSpark, m_delay, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CEnvSpark::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "MaxDelay"))
	{
		m_delay = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CEnvSpark::Precache( void )
{
	Util::PrecacheFixedNbSounds("misc/spark%d.wav", 6);
}

//=============================================
// @brief
//
//=============================================
bool CEnvSpark::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(!HasSpawnFlag(FL_TOGGLE) || HasSpawnFlag(FL_START_ON))
		m_isActive = true;

	if(m_isActive)
	{
		SetThink(&CEnvSpark::SparkThink);
		m_pState->nextthink = g_pGameVars->time + 0.1 + Common::RandomFloat(0, 1.5);
	}

	if(m_delay <= 0)
		m_delay = DEFAULT_DELAY_TIME;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvSpark::SparkThink( void )
{
	// Spawn spark effect
	Util::CreateSparks(m_pState->origin);

	CString soundfile;
	soundfile << "misc/spark" << (Int32)Common::RandomLong(1, 6) << ".wav";

	Float volume = Common::RandomFloat(0.1, 0.6);
	Util::EmitAmbientSound(m_pState->origin, soundfile.c_str(), volume);

	m_pState->nextthink = g_pGameVars->time + Common::RandomFloat(0, m_delay);
}

//=============================================
// @brief
//
//=============================================
void CEnvSpark::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool prevstate = m_isActive;
	switch(useMode)
	{
	case USE_OFF:
		m_isActive = false;
		break;
	case USE_ON:
		m_isActive = true;
		break;
	case USE_TOGGLE:
	default:
		m_isActive = !m_isActive;
		break;
	}

	if(prevstate == m_isActive)
		return;

	if(!m_isActive)
	{
		ClearThinkFunctions();
	}
	else
	{
		SetThink(&CEnvSpark::SparkThink);
		m_pState->nextthink = g_pGameVars->time + (0.1 + Common::RandomFloat(0, m_delay));
	}
}

