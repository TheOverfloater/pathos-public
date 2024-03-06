/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerparalyzeplayer.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_paralyzeplayer, CTriggerParalyzePlayer);

//=============================================
// @brief
//
//=============================================
CTriggerParalyzePlayer::CTriggerParalyzePlayer( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerParalyzePlayer::~CTriggerParalyzePlayer( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerParalyzePlayer::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	CBaseEntity* pPlayer = Util::GetHostPlayer();
	if(!pPlayer)
		return;

	if(pPlayer->GetIsPlayerParalyzed())
		pPlayer->SetPlayerParalyzed(false);
	else
		pPlayer->SetPlayerParalyzed(true);
}