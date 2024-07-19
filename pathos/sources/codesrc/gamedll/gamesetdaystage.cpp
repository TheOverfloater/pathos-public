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

	// True if any light_env set their values
	bool valuesset = false;

	// Update all light_env entities
	Uint32 numEntities = gd_engfuncs.pfnGetNbEdicts();
	for (Uint32 i = 1; i < numEntities; i++)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
		if (pedict->free || !pedict->pprivatedata)
			continue;

		pEntity = CBaseEntity::GetClass(pedict);
		if (!pEntity->IsLightEnvironment())
			continue;

		bool result = pEntity->SetLightEnvValues((daystage_t)m_dayStage);
		if(result)
			valuesset = true;
	}

	// If no light_env set values, then clear them
	if (!valuesset)
	{
		gd_engfuncs.pfnSetCVarFloat("sv_skycolor_r", 0);
		gd_engfuncs.pfnSetCVarFloat("sv_skycolor_g", 0);
		gd_engfuncs.pfnSetCVarFloat("sv_skycolor_b", 0);

		gd_engfuncs.pfnSetCVarFloat("sv_skyvec_x", 0);
		gd_engfuncs.pfnSetCVarFloat("sv_skyvec_y", 0);
		gd_engfuncs.pfnSetCVarFloat("sv_skyvec_z", 0);
	}
}
