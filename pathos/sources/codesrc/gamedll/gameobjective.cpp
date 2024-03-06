/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "gameobjective.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(game_objective, CGameObjective);

//=============================================
// @brief
//
//=============================================
CGameObjective::CGameObjective( edict_t* pedict ):
	CPointEntity(pedict),
	m_mode(OBJ_MODE_ADD)
{
}

//=============================================
// @brief
//
//=============================================
CGameObjective::~CGameObjective( void )
{
}

//=============================================
// @brief
//
//=============================================
void CGameObjective::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CGameObjective, m_mode, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CGameObjective::Spawn( void )
{
	if(m_pFields->message == NO_STRING_VALUE)
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
bool CGameObjective::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "mode"))
	{
		Int32 mode = SDL_atoi(kv.value);
		switch(mode)
		{
		case OBJ_MODE_ADD:
		case OBJ_MODE_REMOVE:
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
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CGameObjective::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	CBaseEntity* pPlayer;
	if(pActivator && pActivator->IsPlayer())
		pPlayer = pActivator;
	else
		pPlayer = Util::GetHostPlayer();

	bool notifyPlayer = HasSpawnFlag(FL_NO_NOTIFICATIONS) ? false : true;

	switch(m_mode)
	{
	case OBJ_MODE_ADD:
		pPlayer->AddMissionObjective(gd_engfuncs.pfnGetString(m_pFields->message), notifyPlayer);
		break;
	case OBJ_MODE_REMOVE:
		pPlayer->RemoveMissionObjective(gd_engfuncs.pfnGetString(m_pFields->message), notifyPlayer);
		break;
	}
}
