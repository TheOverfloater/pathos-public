/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggersetsavetitle.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_setsavetitle, CTriggerSetSaveTitle);

//=============================================
// @brief
//
//=============================================
CTriggerSetSaveTitle::CTriggerSetSaveTitle( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerSetSaveTitle::~CTriggerSetSaveTitle( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CTriggerSetSaveTitle::Spawn( void )
{
	if(m_pFields->netname == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE || HasSpawnFlag(FL_START_ON))
		m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerSetSaveTitle::InitEntity( void )
{
	CallUse(this, this, USE_TOGGLE, 0);
}

//=============================================
// @brief
//
//=============================================
void CTriggerSetSaveTitle::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	CBaseEntity* pPlayer = Util::GetHostPlayer();
	if(!pPlayer)
	{
		Util::EntityConPrintf(m_pEdict, "Not a player entity.\n");
		return;
	}

	pPlayer->SetSaveGameTitle(gd_engfuncs.pfnGetString(m_pFields->netname));
}