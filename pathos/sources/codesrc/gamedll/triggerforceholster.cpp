/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerforceholster.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_forceholster, CTriggerForceHolster);

//=============================================
// @brief
//
//=============================================
CTriggerForceHolster::CTriggerForceHolster( edict_t* pedict ):
	CPointEntity(pedict),
	m_mode(MODE_OFF)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerForceHolster::~CTriggerForceHolster( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerForceHolster::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerForceHolster, m_mode, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerForceHolster::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "mode"))
	{
		Int32 mode = SDL_atoi(kv.value);
		switch(mode)
		{
		case 1:
			m_mode = MODE_ON;
			break;
		case 2:
			m_mode = MODE_TOGGLE;
			break;
		default:
		case 0:
			m_mode = MODE_OFF;
			break;
		}
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerForceHolster::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerForceHolster::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Either use activator, or assume it's local player
	CBaseEntity* pEntity;
	if(pActivator && pActivator->IsPlayer())
		pEntity = pActivator;
	else
		pEntity = Util::GetHostPlayer();

	if(!pEntity || !pEntity->IsPlayer())
	{
		Util::EntityConPrintf(m_pEdict, "Not a player entity.\n");
		return;
	}

	bool forceholster = false;
	switch(m_mode)
	{
	case MODE_ON:
		forceholster = true;
		break;
	case MODE_OFF:
		forceholster = false;
		break;
	case MODE_TOGGLE:
		{
			if(pEntity->IsForceHolsterSet())
				forceholster = false;
			else
				forceholster = true;
		}
		break;
	}

	pEntity->SetForceHolster(forceholster);

	if(HasSpawnFlag(FL_IS_IN_DREAM))
	{
		if(forceholster)
			pEntity->SetIsInDream(true);
		else
			pEntity->SetIsInDream(false);
	}

	if(!HasSpawnFlag(FL_NO_SLOW_MOVE))
	{
		if(forceholster)
			pEntity->SetForceSlowMove(true, false);
		else
			pEntity->SetForceSlowMove(false, false);
	}
}