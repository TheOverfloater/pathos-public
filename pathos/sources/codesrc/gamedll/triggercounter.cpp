/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggercounter.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_counter, CTriggerCounter);

//=============================================
// @brief
//
//=============================================
CTriggerCounter::CTriggerCounter( edict_t* pedict ):
	CDelayEntity(pedict),
	m_nbTriggersLeft(0)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerCounter::~CTriggerCounter( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerCounter::DeclareSaveFields( void )
{
	// Call base class to do it first
	CDelayEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCounter, m_nbTriggersLeft, EFIELD_UINT32));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerCounter::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "count"))
	{
		Int32 nb = SDL_atoi(kv.value);
		if(!nb || nb < 0)
		{
			Util::EntityConPrintf(m_pEdict, "Invalid value '%d' for 'count'\n", nb);
			return false;
		}
		m_nbTriggersLeft = nb;
		return true;
	}
	else
		return CDelayEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerCounter::Spawn( void )
{
	if(!CDelayEntity::Spawn())
		return false;

	// Should still be a valid solid entity
	if(!SetModel(m_pFields->modelname, true))
		return false;

	// Despite being a trigger, this is more like a point entity
	// without a body or anything
	m_pState->effects = EF_NODRAW;
	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_NONE;

	if(!m_nbTriggersLeft)
	{
		Util::EntityConPrintf(m_pEdict, "No count set for entity.\n");
		m_nbTriggersLeft = 2;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCounter::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!m_nbTriggersLeft)
		return;

	m_nbTriggersLeft--;

	if(!m_nbTriggersLeft)
	{
		// Trigger targets
		UseTargets(pActivator, USE_TOGGLE, value);

		// Remove from world
		SetThink(&CBaseEntity::RemoveThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
}