/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerrepeat.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_repeat, CTriggerRepeat);

//=============================================
// @brief
//
//=============================================
CTriggerRepeat::CTriggerRepeat( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerRepeat::~CTriggerRepeat( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CTriggerRepeat::Spawn( void )
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
void CTriggerRepeat::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	SetThink(&CTriggerRepeat::RepeatThink);
	m_pState->nextthink = g_pGameVars->time + 0.1;
}

//=============================================
// @brief
//
//=============================================
void CTriggerRepeat::RepeatThink( void )
{
	Util::FireTargets(gd_engfuncs.pfnGetString(m_pFields->target), nullptr, this, USE_TOGGLE, 0);
	if(m_pState->frags != -1)
		m_pState->frags--;

	if(m_pState->frags == 0)
	{
		Util::RemoveEntity(this);
	}
	else
	{
		SetThink(&CTriggerRepeat::RepeatThink);
		m_pState->nextthink = g_pGameVars->time + m_pState->health;
	}
}