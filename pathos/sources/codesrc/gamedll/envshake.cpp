/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envshake.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_shake, CEnvShake);

//=============================================
// @brief
//
//=============================================
CEnvShake::CEnvShake( edict_t* pedict ):
	CPointEntity(pedict),
	m_amplitude(0),
	m_frequency(0),
	m_duration(0),
	m_radius(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvShake::~CEnvShake( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvShake::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvShake, m_amplitude, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvShake, m_frequency, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvShake, m_duration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvShake, m_radius, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CEnvShake::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "amplitude"))
	{
		m_amplitude = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "frequency"))
	{
		m_frequency = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "duration"))
	{
		m_duration = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "radius"))
	{
		m_radius = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvShake::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(HasSpawnFlag(FL_SHAKE_EVERYWHERE))
		m_radius = 0;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvShake::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	Util::ScreenShake(m_pState->origin,
		m_amplitude,
		m_frequency,
		m_duration,
		m_radius,
		HasSpawnFlag(FL_SHAKE_INAIR),
		HasSpawnFlag(FL_DISRUPT_CONTROLS));
}