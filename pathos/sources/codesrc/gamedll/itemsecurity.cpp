/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "itemsecurity.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(item_security, CItemSecurity);

//=============================================
// @brief
//
//=============================================
CItemSecurity::CItemSecurity( edict_t* pedict ):
	CPlayerItem(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CItemSecurity::~CItemSecurity( void )
{
}

//=============================================
// @brief
//
//=============================================
void CItemSecurity::SetSpawnProperties( void )
{
	SetBodyGroup(WMODEL_BODY_BASE, WMODEL_SECURITY);
}

//=============================================
// @brief
//
//=============================================
bool CItemSecurity::AddToPlayer( CBaseEntity* pPlayer )
{
	// Always "adds" to player
	return true;
}
