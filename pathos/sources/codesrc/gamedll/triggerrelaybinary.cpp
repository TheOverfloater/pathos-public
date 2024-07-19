/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerrelaybinary.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_relay_binary, CTriggerRelayBinary);

//=============================================
// @brief
//
//=============================================
CTriggerRelayBinary::CTriggerRelayBinary(edict_t* pedict) :
	CDelayEntity(pedict),
	m_triggerOnMode(USE_OFF),
	m_triggerOffMode(USE_OFF),
	m_relayState(RELAY_STATE_OFF)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerRelayBinary::~CTriggerRelayBinary(void)
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerRelayBinary::DeclareSaveFields(void)
{
	// Call base class to do it first
	CDelayEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerRelayBinary, m_triggerOnMode, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerRelayBinary, m_triggerOffMode, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerRelayBinary, m_relayState, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerRelayBinary::KeyValue(const keyvalue_t& kv)
{
	if (!qstrcmp(kv.keyname, "triggeronstate"))
	{
		Int32 state = SDL_atoi(kv.value);
		switch (state)
		{
		case 0:
			m_triggerOnMode = USE_OFF;
			break;
		case 2:
			m_triggerOnMode = USE_TOGGLE;
			break;
		case 1:
		default:
			m_triggerOnMode = USE_ON;
			break;
		}
		return true;
	}
	else if (!qstrcmp(kv.keyname, "triggeroffstate"))
	{
		Int32 state = SDL_atoi(kv.value);
		switch (state)
		{
		case 0:
			m_triggerOffMode = USE_OFF;
			break;
		case 2:
			m_triggerOffMode = USE_TOGGLE;
			break;
		case 1:
		default:
			m_triggerOffMode = USE_ON;
			break;
		}
		return true;
	}
	else if (!qstrcmp(kv.keyname, "initialstate"))
	{
		Int32 state = SDL_atoi(kv.value);
		switch (state)
		{
		case 0:
			m_relayState = RELAY_STATE_OFF;
			break;
		case 1:
		default:
			m_relayState = RELAY_STATE_ON;
			break;
		}
		return true;
	}
	else
		return CDelayEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerRelayBinary::Spawn(void)
{
	if (!CDelayEntity::Spawn())
		return false;

	m_pState->solid = SOLID_NOT;
	m_pState->effects = EF_NODRAW;
	m_pState->movetype = MOVETYPE_NONE;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerRelayBinary::CallUse(CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value)
{
	switch (useMode)
	{
	case USE_OFF:
		m_relayState = RELAY_STATE_OFF;
		break;
	case USE_ON:
		m_relayState = RELAY_STATE_ON;
		break;
	case USE_TOGGLE:
		if(m_relayState == RELAY_STATE_OFF)
			m_relayState = RELAY_STATE_ON;
		else
			m_relayState = RELAY_STATE_OFF;
		break;
	}

	switch (m_relayState)
	{
	case RELAY_STATE_ON:
			UseTargets(this, (usemode_t)m_triggerOffMode, m_pFields->target);
		break;
	case RELAY_STATE_OFF:
			UseTargets(this, (usemode_t)m_triggerOnMode, m_pFields->netname);
		break;
	}
}