/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggercounterp.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_counter_p, CTriggerCounterP);

//=============================================
// @brief
//
//=============================================
CTriggerCounterP::CTriggerCounterP( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerCounterP::~CTriggerCounterP( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CTriggerCounterP::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pState->frags == 0)
	{
		Util::EntityConPrintf(m_pEdict, "No count limit specified.\n");
		return false;
	}

	// Save original count in skin
	m_pState->skin = m_pState->frags;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCounterP::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(useMode == USE_OFF)
	{
		m_pState->frags = m_pState->skin;
		Util::EntityConDPrintf(m_pEdict, "Was reset to %d.\n", m_pState->frags);
		return;
	}

	// Remove from the counts
	m_pState->frags--;
	Util::EntityConDPrintf(m_pEdict, "%d left.\n", m_pState->frags);

	if(m_pState->frags > 0)
		return;
	
	Util::FireTargets(gd_engfuncs.pfnGetString(m_pFields->target), pActivator, this, useMode, value);
	
	SetThink(&CBaseEntity::RemoveThink);
	m_pState->nextthink = g_pGameVars->time + 0.1;
}