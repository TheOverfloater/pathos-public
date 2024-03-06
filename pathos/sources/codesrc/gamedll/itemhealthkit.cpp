/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "itemhealthkit.h"
#include "weapons_shared.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(item_healthkit, CItemHealthkit);

//=============================================
// @brief
//
//=============================================
CItemHealthkit::CItemHealthkit( edict_t* pedict ):
	CPlayerItem(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CItemHealthkit::~CItemHealthkit( void )
{
}

//=============================================
// @brief
//
//=============================================
void CItemHealthkit::SetSpawnProperties( void )
{
	SetBodyGroup(WMODEL_BODY_BASE, WMODEL_MEDKIT);
}

//=============================================
// @brief
//
//=============================================
bool CItemHealthkit::AddToPlayer( CBaseEntity* pPlayer )
{
	if(!pPlayer || !pPlayer->IsPlayer())
		return false;

	return pPlayer->AddMedkit(gd_engfuncs.pfnGetString(m_pFields->classname), HasSpawnFlag(FL_ITEM_NO_NOTICE) ? true : false);
}
