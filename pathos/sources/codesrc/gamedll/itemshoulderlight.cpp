/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "itemshoulderlight.h"
#include "weapons_shared.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(item_shoulderlight, CItemShoulderLight);

//=============================================
// @brief
//
//=============================================
CItemShoulderLight::CItemShoulderLight( edict_t* pedict ):
	CPlayerItem(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CItemShoulderLight::~CItemShoulderLight( void )
{
}

//=============================================
// @brief
//
//=============================================
void CItemShoulderLight::SetSpawnProperties( void )
{
	SetBodyGroup(WMODEL_BODY_BASE, WMODEL_SHOULDERLIGHT);
}

//=============================================
// @brief
//
//=============================================
bool CItemShoulderLight::AddToPlayer( CBaseEntity* pPlayer )
{
	if(!pPlayer || !pPlayer->IsPlayer())
		return false;

	return pPlayer->AddShoulderLight(gd_engfuncs.pfnGetString(m_pFields->classname), HasSpawnFlag(FL_ITEM_NO_NOTICE) ? true : false);
}
