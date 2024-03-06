/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "gamesetdaystage.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(game_setdaystage, CGameSetDayStage);

//=============================================
// @brief
//
//=============================================
CGameSetDayStage::CGameSetDayStage( edict_t* pedict ):
	CPointEntity(pedict),
	m_dayStage(DAYSTAGE_NORMAL)
{
}

//=============================================
// @brief
//
//=============================================
CGameSetDayStage::~CGameSetDayStage( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CGameSetDayStage::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "stage"))
	{
		Int32 value = SDL_atoi(kv.value);
		switch(value)
		{
		case 1:
			m_dayStage = DAYSTAGE_NIGHTSTAGE;
			break;
		case 2:
			m_dayStage = DAYSTAGE_DAYLIGHT_RETURN;
			break;
		default:
		case 0:
			m_dayStage = DAYSTAGE_NORMAL;
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
void CGameSetDayStage::DeclareSaveFields( void )
{
	// Call base class to handle it's own
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CGameSetDayStage, m_dayStage, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
void CGameSetDayStage::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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

	pEntity->SetDayStage((daystage_t)m_dayStage);
}
