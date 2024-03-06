/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerslowmove.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_slowmove, CTriggerSlowMove);

//=============================================
// @brief
//
//=============================================
CTriggerSlowMove::CTriggerSlowMove( edict_t* pedict ):
	CPointEntity(pedict),
	m_mode(MODE_OFF),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerSlowMove::~CTriggerSlowMove( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerSlowMove::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSlowMove, m_mode, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerSlowMove::KeyValue( const keyvalue_t& kv )
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
bool CTriggerSlowMove::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerSlowMove::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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

	bool slowmove = false;
	switch(m_mode)
	{
	case MODE_ON:
		slowmove = true;
		break;
	case MODE_OFF:
		slowmove = false;
		break;
	case MODE_TOGGLE:
		{
			if(pEntity->GetFlags() & FL_SLOWMOVE)
				slowmove = false;
			else
				slowmove = true;
		}
		break;
	}

	pEntity->SetForceSlowMove(slowmove, true);
}