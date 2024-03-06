/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envroomtype.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_roomtype, CEnvRoomType);

//=============================================
// @brief
//
//=============================================
CEnvRoomType::CEnvRoomType( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvRoomType::~CEnvRoomType( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvRoomType::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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

	pEntity->SetRoomType(m_pState->skin);
}