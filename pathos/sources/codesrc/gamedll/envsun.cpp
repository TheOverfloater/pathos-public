/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envsun.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_sun, CEnvSun);

//=============================================
// @brief
//
//=============================================
CEnvSun::CEnvSun( edict_t* pedict ):
	CPointEntity(pedict),
	m_isActive(false),
	m_pitch(0),
	m_roll(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSun::~CEnvSun( void )
{
}

//=============================================
// @brief Manages keyvalues
//
//=============================================
bool CEnvSun::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "pitch"))
	{
		m_pitch = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "roll"))
	{
		m_roll = SDL_atof(kv.value);
		return true;
	}
	else 
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief Calls for classes and their children
//
//=============================================
void CEnvSun::DeclareSaveFields( void )
{
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSun, m_pitch, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSun, m_roll, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSun, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CEnvSun::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	if(HasSpawnFlag(FL_START_ON) || m_pFields->targetname == NO_STRING_VALUE)
		m_isActive = true;

	if(!m_pState->scale)
		m_pState->scale = 1.0;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvSun::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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

//=============================================
// @brief
//
//=============================================
void CEnvSun::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(pPlayer && !m_isActive)
		return;

	if(pPlayer)
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.sunflare, nullptr, pPlayer->GetEdict());
	else
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.sunflare, nullptr, nullptr);

	gd_engfuncs.pfnMsgWriteInt16(m_pEdict->entindex);
	gd_engfuncs.pfnMsgWriteByte(m_isActive);
	if(m_isActive)
	{
		gd_engfuncs.pfnMsgWriteSmallFloat(m_pitch);
		gd_engfuncs.pfnMsgWriteSmallFloat(m_roll);
		gd_engfuncs.pfnMsgWriteSmallFloat(m_pState->scale);

		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteByte(m_pState->rendercolor[i]);

		gd_engfuncs.pfnMsgWriteByte(HasSpawnFlag(FL_PORTAL_SUN));
	}
	gd_engfuncs.pfnUserMessageEnd();
}