/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerglobaldelayed.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_globaldelayed, CTriggerGlobalDelayed);

//=============================================
// @brief
//
//=============================================
CTriggerGlobalDelayed::CTriggerGlobalDelayed( edict_t* pedict ):
	CPointEntity(pedict),
	m_mode(MODE_SET),
	m_duration(0)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerGlobalDelayed::~CTriggerGlobalDelayed( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerGlobalDelayed::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerGlobalDelayed, m_mode, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerGlobalDelayed, m_duration, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerGlobalDelayed::Spawn( void )
{
	if(m_mode == MODE_SET && (m_pFields->target == NO_STRING_VALUE|| m_duration <= 0))
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
bool CTriggerGlobalDelayed::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "mode"))
	{
		Int32 mode = SDL_atoi(kv.value);
		switch(mode)
		{
		case MODE_SET:
		case MODE_CLEAR:
			break;
		default:
			{
				Util::EntityConPrintf(m_pEdict, "Invalid value %d for field %d.\n", kv.value, kv.keyname);
				return false;
			}
			break;
		}

		m_mode = mode;
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
void CTriggerGlobalDelayed::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	CBaseEntity* pPlayer;
	if(pActivator && pActivator->IsPlayer())
		pPlayer = pActivator;
	else
		pPlayer = Util::GetHostPlayer();

	switch(m_mode)
	{
	case MODE_SET:
		pPlayer->SetGlobalDelayedTrigger(m_duration, gd_engfuncs.pfnGetString(m_pFields->target));
		break;
	case MODE_CLEAR:
		pPlayer->ClearGlobalDelayedTrigger();
		break;
	}
}
