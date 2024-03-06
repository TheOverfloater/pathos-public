/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerrelay.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_relay, CTriggerRelay);

//=============================================
// @brief
//
//=============================================
CTriggerRelay::CTriggerRelay( edict_t* pedict ):
	CDelayEntity(pedict),
	m_triggerMode(USE_OFF)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerRelay::~CTriggerRelay( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerRelay::DeclareSaveFields( void )
{
	// Call base class to do it first
	CDelayEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerRelay, m_triggerMode, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerRelay::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "triggerstate"))
	{
		Int32 state = SDL_atoi(kv.value);
		switch(state)
		{
		case 0:
			m_triggerMode = USE_OFF;
			break;
		case 2:
			m_triggerMode = USE_TOGGLE;
			break;
		case 1:
		default:
			m_triggerMode = USE_ON;
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
bool CTriggerRelay::Spawn( void )
{
	if(!CDelayEntity::Spawn())
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
void CTriggerRelay::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	UseTargets(this, (usemode_t)m_triggerMode, 0);
	if(HasSpawnFlag(FL_REMOVE_ON_FIRE))
		Util::RemoveEntity(this);
}