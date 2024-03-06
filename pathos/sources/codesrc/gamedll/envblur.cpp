/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envblur.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_blur, CEnvBlur);

//=============================================
// @brief
//
//=============================================
CEnvBlur::CEnvBlur( edict_t* pedict ):
	CPointEntity(pedict),
	m_isActive(false),
	m_blurFade(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvBlur::~CEnvBlur( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvBlur::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlur, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlur, m_blurFade, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CEnvBlur::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "fade"))
	{
		m_blurFade = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvBlur::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(HasSpawnFlag(FL_START_ON))
		m_isActive = true;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvBlur::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(pPlayer && !m_isActive)
		return;

	if(pPlayer)
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.motionblur, nullptr, pPlayer->GetEdict());
	else
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.motionblur, nullptr, nullptr);

	gd_engfuncs.pfnMsgWriteByte(m_isActive);
	if(m_isActive)
		gd_engfuncs.pfnMsgWriteSmallFloat(m_blurFade);
	gd_engfuncs.pfnMsgWriteByte(false);
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief
//
//=============================================
void CEnvBlur::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool prevstate = m_isActive;
	switch(useMode)
	{
	case USE_ON:
		m_isActive = true;
		break;
	case USE_OFF:
		m_isActive = false;
		break;
	case USE_TOGGLE:
		m_isActive = !m_isActive;
		break;
	}

	if(m_isActive != prevstate)
		SendInitMessage(nullptr);
}