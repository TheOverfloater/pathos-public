/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerspeedup.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_speedup, CTriggerSpeedup);

//=============================================
// @brief
//
//=============================================
CTriggerSpeedup::CTriggerSpeedup( edict_t* pedict ):
	CPointEntity(pedict),
	m_targetSpeed(0),
	m_startSpeed(0),
	m_duration(0),
	m_beginTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerSpeedup::~CTriggerSpeedup( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerSpeedup::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSpeedup, m_targetSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSpeedup, m_startSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSpeedup, m_duration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSpeedup, m_beginTime, EFIELD_TIME));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerSpeedup::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "targetspeed"))
	{
		m_targetSpeed = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "startspeed"))
	{
		m_startSpeed = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "duration"))
	{
		m_duration = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerSpeedup::Spawn( void )
{
	if(m_pFields->target == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerSpeedup::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	m_pState->flags |= FL_ALWAYSTHINK;
	m_pState->nextthink = m_pState->ltime + 0.1;
	SetThink(&CTriggerSpeedup::SpeedupThink);
	m_beginTime = g_pGameVars->time;
}

//=============================================
// @brief
//
//=============================================
void CTriggerSpeedup::SpeedupThink( void )
{
	Float speed;
	if(m_beginTime + m_duration <= g_pGameVars->time)
	{
		speed = m_targetSpeed;
		m_pState->flags &= ~FL_ALWAYSTHINK;
		ClearThinkFunctions();
	}
	else
	{
		// Calculate speed
		Float frac = (g_pGameVars->time-m_beginTime)/m_duration;
		speed = m_startSpeed+(m_targetSpeed-m_startSpeed)*frac;

		// Set nextthink
		m_pState->nextthink = m_pState->ltime + 0.1;
	}

	const Char* pstrTarget = gd_engfuncs.pfnGetString(m_pFields->target);
	if(!pstrTarget || !qstrlen(pstrTarget))
		return;

	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, pstrTarget);
		if(Util::IsNullEntity(pedict))
			break;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(!pEntity || !pEntity->IsFuncTrainEntity())
			continue;

		pEntity->SetSpeed(speed);
	}
}