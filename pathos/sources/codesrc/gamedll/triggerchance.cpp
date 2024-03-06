/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerchance.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_chance, CTriggerChance);

//=============================================
// @brief
//
//=============================================
CTriggerChance::CTriggerChance( edict_t* pedict ):
	CPointEntity(pedict),
	m_chancePercentage(0)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerChance::~CTriggerChance( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerChance::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerChance, m_chancePercentage, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerChance::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "chancepercentage"))
	{
		m_chancePercentage = SDL_atoi(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerChance::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->target == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(m_chancePercentage <= 0 || m_chancePercentage >= 100)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid percentage '%d' specified.\n", m_chancePercentage);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerChance::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	Int32 random = Common::RandomLong(0, 100);
	Int32 treshold = (100-m_chancePercentage);
	if(random >= treshold)
		Util::FireTargets(gd_engfuncs.pfnGetString(m_pFields->target), pActivator, this, useMode, value);

	SetThink(&CBaseEntity::RemoveThink);
	m_pState->nextthink = g_pGameVars->time + 0.1;
}