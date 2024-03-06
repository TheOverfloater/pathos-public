/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "playerweaponstrip.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(player_weaponstrip, CPlayerWeaponStrip);

//=============================================
// @brief
//
//=============================================
CPlayerWeaponStrip::CPlayerWeaponStrip( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CPlayerWeaponStrip::~CPlayerWeaponStrip( void )
{
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeaponStrip::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	CPlayerEntity* pPlayer = nullptr;
	if(pActivator && pActivator->IsPlayer())
		pPlayer = reinterpret_cast<CPlayerEntity*>(pActivator);
	else
		pPlayer = reinterpret_cast<CPlayerEntity*>(Util::GetHostPlayer());

	if(!pPlayer)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find player.");
		return;
	}

	pPlayer->RemoveAllWeapons();
}