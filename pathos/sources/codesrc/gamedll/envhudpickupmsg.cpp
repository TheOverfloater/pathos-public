/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envhudpickupmsg.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_hudpickupmsg, CEnvHudPickupMsg);

//=============================================
// @brief
//
//=============================================
CEnvHudPickupMsg::CEnvHudPickupMsg( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvHudPickupMsg::~CEnvHudPickupMsg( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CEnvHudPickupMsg::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	// Make sure it's valid
	if(m_pFields->message == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvHudPickupMsg::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	CBaseEntity* pPlayer;
	if(pActivator && pActivator->IsPlayer())
		pPlayer = pActivator;
	else
		pPlayer = Util::GetHostPlayer();

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.customitempickup, nullptr, pPlayer->GetEdict());
	gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_pFields->message));
	gd_engfuncs.pfnUserMessageEnd();

	if(HasSpawnFlag(FL_ONLY_ONCE))
	{
		Util::RemoveEntity(m_pEdict);
		return;
	}
}