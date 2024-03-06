/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerzoom.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_zoom, CTriggerZoom);

//=============================================
// @brief
//
//=============================================
CTriggerZoom::CTriggerZoom( edict_t* pedict ):
	CPointEntity(pedict),
	m_targetFOV(0),
	m_startFOV(0),
	m_state(STATE_OFF),
	m_duration(0),
	m_beginTime(0),
	m_pPlayer(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerZoom::~CTriggerZoom( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerZoom::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerZoom, m_targetFOV, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerZoom, m_startFOV, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerZoom, m_state, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerZoom, m_duration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerZoom, m_beginTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerZoom, m_pPlayer, EFIELD_ENTPOINTER));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerZoom::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "targetsize"))
	{
		m_targetFOV = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "startsize"))
	{
		m_startFOV = SDL_atoi(kv.value);
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
bool CTriggerZoom::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(HasSpawnFlag(FL_START_ON))
		m_state = STATE_INITIAL;

	if(m_startFOV == -1)
		m_startFOV = gd_engfuncs.pfnGetCvarFloatValue(REFERENCE_FOV_CVAR_NAME);

	if(m_targetFOV == -1)
		m_targetFOV = gd_engfuncs.pfnGetCvarFloatValue(REFERENCE_FOV_CVAR_NAME);

	if(m_startFOV == m_targetFOV)
		Util::EntityConPrintf(m_pEdict, "Target FOV and start FOV are the same.\n");


	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerZoom::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Either use activator, or assume it's local player
	CBaseEntity* pEntity;
	if(pActivator && pActivator->IsPlayer())
		pEntity = pActivator;
	else
		pEntity = Util::GetHostPlayer();

	if(!pEntity || !pEntity->IsPlayer())
	{
		gd_engfuncs.pfnCon_Printf("%s - Failed to get player.\n", gd_engfuncs.pfnGetString(m_pFields->classname));
		return;
	}

	m_pPlayer = pEntity;

	if(m_state == STATE_OFF)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.triggerzoom, nullptr, m_pPlayer->GetEdict());
			gd_engfuncs.pfnMsgWriteByte(m_startFOV);
			gd_engfuncs.pfnMsgWriteSmallFloat(0);
		gd_engfuncs.pfnUserMessageEnd();

		m_state = STATE_INITIAL;
	}
	else if(m_state == STATE_INITIAL)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.triggerzoom, nullptr, m_pPlayer->GetEdict());
			gd_engfuncs.pfnMsgWriteByte(m_targetFOV);
			gd_engfuncs.pfnMsgWriteSmallFloat(m_duration);
		gd_engfuncs.pfnUserMessageEnd();

		// Set the time
		m_beginTime = g_pGameVars->time;
		m_state = STATE_BLENDING;
	}
	else if(m_state == STATE_BLENDING)
	{
		// Disable the entity
		m_state = STATE_OFF;
	}
}

//=============================================
// @brief
//
//=============================================
void CTriggerZoom::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(m_state == STATE_INITIAL && !m_pPlayer)
		m_pPlayer = Util::GetHostPlayer();

	if(pPlayer != m_pPlayer)
		return;

	if((m_beginTime+m_duration) <= g_pGameVars->time)
	{
		// Return, but do not alter state of entity for
		// legacy support on existing levels
		return;
	}

	if(m_state == STATE_INITIAL)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.triggerzoom, nullptr, m_pPlayer->GetEdict());
			gd_engfuncs.pfnMsgWriteByte(m_startFOV);
			gd_engfuncs.pfnMsgWriteSmallFloat(0);
		gd_engfuncs.pfnUserMessageEnd();
	}
	else if(m_state == STATE_BLENDING)
	{
		// Calculate expected FOV and the delta
		Float timedelta = (m_beginTime+m_duration) - g_pGameVars->time;
		Float timefrac = timedelta/m_duration;

		Uint32 currentFOV = m_startFOV*(timefrac) + m_targetFOV*(1.0 - timefrac);

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.triggerzoom, nullptr, m_pPlayer->GetEdict());
			gd_engfuncs.pfnMsgWriteByte(currentFOV);
			gd_engfuncs.pfnMsgWriteSmallFloat(timedelta);
		gd_engfuncs.pfnUserMessageEnd();
	}
}
