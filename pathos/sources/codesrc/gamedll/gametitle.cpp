/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "gametitle.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(game_title, CGameTitle);

// Game title message name
const Char CGameTitle::TITLE_MESSAGE_NAME[] = "GAMETITLE";

//=============================================
// @brief
//
//=============================================
CGameTitle::CGameTitle( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CGameTitle::~CGameTitle( void )
{
}

//=============================================
// @brief
//
//=============================================
void CGameTitle::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.showmessage, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteString(TITLE_MESSAGE_NAME);
	gd_engfuncs.pfnUserMessageEnd();

	Util::RemoveEntity(this);
}