/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "itemkevlar.h"
#include "weapons_shared.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(item_kevlar, CItemKevlar);

//=============================================
// @brief
//
//=============================================
CItemKevlar::CItemKevlar( edict_t* pedict ):
	CPlayerItem(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CItemKevlar::~CItemKevlar( void )
{
}

//=============================================
// @brief
//
//=============================================
void CItemKevlar::SetSpawnProperties( void )
{
	SetBodyGroup(WMODEL_BODY_BASE, WMODEL_KEVLAR);
}

//=============================================
// @brief
//
//=============================================
bool CItemKevlar::AddToPlayer( CBaseEntity* pPlayer )
{
	if(!pPlayer || !pPlayer->IsPlayer())
		return false;

	return pPlayer->AddKevlar(gd_engfuncs.pfnGetString(m_pFields->classname), HasSpawnFlag(FL_ITEM_NO_NOTICE) ? true : false);
}
