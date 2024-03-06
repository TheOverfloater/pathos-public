/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerkillplayer.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_killplayer, CTriggerKillPlayer);

//=============================================
// @brief
//
//=============================================
CTriggerKillPlayer::CTriggerKillPlayer( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerKillPlayer::~CTriggerKillPlayer( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerKillPlayer::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	CBaseEntity* pPlayer = Util::GetHostPlayer();
	if(!pPlayer)
		return;

	pPlayer->TakeDamage(this, this, 1000, (DMG_GENERIC|DMG_NEVERGIB));
}