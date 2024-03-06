/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envearthquake.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_earthquake, CEnvEarthQuake);

//=============================================
// @brief
//
//=============================================
CEnvEarthQuake::CEnvEarthQuake( edict_t* pedict ):
	CPointEntity(pedict),
	m_minDelay(0),
	m_maxDelay(0),
	m_minForce(0),
	m_maxForce(0),
	m_duration(0),
	m_fadeTime(0),
	m_shakeBeginTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvEarthQuake::~CEnvEarthQuake( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvEarthQuake::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvEarthQuake, m_minDelay, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvEarthQuake, m_maxDelay, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvEarthQuake, m_minForce, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvEarthQuake, m_maxForce, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvEarthQuake, m_duration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvEarthQuake, m_fadeTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvEarthQuake, m_shakeBeginTime, EFIELD_TIME));
}

//=============================================
// @brief
//
//=============================================
bool CEnvEarthQuake::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_minDelay < 0)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid value %d for minimum delay.\n", m_minDelay);
		Util::RemoveEntity(this);
		return false;
	}
	else if(m_minDelay > m_maxDelay)
	{
		Util::EntityConPrintf(m_pEdict, "Minimum delay %f is larger than max delay %f.\n", m_minDelay, m_maxDelay);
		Util::RemoveEntity(this);
		return false;
	}

	if(m_maxDelay <= 0)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid value %f for maximum delay.\n", m_maxDelay);
		Util::RemoveEntity(this);
		return false;
	}

	if(m_duration <= 0)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid value %f for duration.\n", m_duration);
		Util::RemoveEntity(this);
		return false;
	}

	if(m_fadeTime < 0)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid value %f for fade out time.\n", m_fadeTime);
		Util::RemoveEntity(this);
		return false;
	}

	if(m_fadeTime > m_duration)
	{
		Util::EntityConPrintf(m_pEdict, "Fade out time %f is longer than shake duration %f\n", m_fadeTime, m_duration);
		Util::RemoveEntity(this);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CEnvEarthQuake::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "mindelay"))
	{
		m_minDelay = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "maxdelay"))
	{
		m_maxDelay = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "minforce"))
	{
		m_minForce = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "maxforce"))
	{
		m_maxForce = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "duration"))
	{
		m_duration = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "fadetime"))
	{
		m_fadeTime = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CEnvEarthQuake::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool currentstate = false;
	switch(useMode)
	{
	case USE_ON:
		currentstate = true;
		break;
	case USE_OFF:
		currentstate = false;
		break;
	case USE_TOGGLE:
		currentstate = (m_shakeBeginTime != 0 ? false : true);
		break;
	}

	if(!currentstate)
	{
		SetThink(nullptr);
		m_pState->nextthink = 0;
		m_shakeBeginTime = 0;
	}
	else
	{
		SetThink(&CEnvEarthQuake::QuakeThink);
		m_pState->nextthink = g_pGameVars->time + Common::RandomFloat(m_minDelay, m_maxDelay);
		m_shakeBeginTime = g_pGameVars->time;
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvEarthQuake::QuakeThink( void )
{
	Float nextdelay = Common::RandomFloat(m_minDelay, m_maxDelay);
	if((g_pGameVars->time + nextdelay) > (m_shakeBeginTime + m_duration))
	{
		SetThink(nullptr);
		m_pState->nextthink = 0;
		m_shakeBeginTime = 0;
		return;
	}
	
	Vector shakeDirection = Vector(
		Common::RandomFloat(-1, 1),
		Common::RandomFloat(-1, 1),
		0);
	shakeDirection.Normalize();

	Float shakeForce = Common::RandomFloat(m_minForce, m_maxForce);
	if(m_fadeTime)
	{
		// Calculate fade factor
		Double fadeBeginTime = m_shakeBeginTime + m_duration - m_fadeTime;
		if(fadeBeginTime <= g_pGameVars->time)
		{
			Float factor = (g_pGameVars->time - fadeBeginTime) / m_duration;
			factor = 1.0 - clamp(factor, 0.0, 1.0);
			shakeForce *= factor;
		}
	}

	for(Int32 i = 0; i < g_pGameVars->maxclients; i++)
	{
		CBaseEntity* pPlayer = Util::GetPlayerByIndex(i);
		if(!pPlayer)
			continue;

		Vector vecPush;
		Math::VectorScale(shakeDirection, shakeForce, vecPush);
		if(pPlayer->GetFlags() & FL_BASEVELOCITY)
			Math::VectorAdd(vecPush, pPlayer->GetVelocity(), vecPush);

		pPlayer->SetVelocity(vecPush);

		if(Common::RandomLong(0, 1) == 0)
		{
			Float angleForce = Common::RandomFloat(0.5, 0.8);
			pPlayer->SetPunchAmount(vecPush * angleForce);
		}
	}

	m_pState->nextthink = g_pGameVars->time + nextdelay;
}