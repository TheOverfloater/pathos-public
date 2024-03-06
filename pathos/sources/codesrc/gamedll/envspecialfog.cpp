/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envspecialfog.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_specialfog, CEnvSpecialFog);

//=============================================
// @brief
//
//=============================================
CEnvSpecialFog::CEnvSpecialFog( edict_t* pedict ):
	CPointEntity(pedict),
	m_triggerMode(MODE_ON)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSpecialFog::~CEnvSpecialFog( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CEnvSpecialFog::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "mode"))
	{
		Int32 mode = SDL_atoi(kv.value);
		switch(mode)
		{
		case 1:
			m_triggerMode = MODE_OFF;
			break;
		case 0:
		default:
			m_triggerMode = MODE_ON;
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
void CEnvSpecialFog::DeclareSaveFields( void )
{
	// Call base class to handle it's own
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSpecialFog, m_triggerMode, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
void CEnvSpecialFog::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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

	pEntity->SetSpecialFog((m_triggerMode == MODE_ON) ? true : false);
}
