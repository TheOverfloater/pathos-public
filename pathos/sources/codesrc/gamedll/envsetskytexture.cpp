/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "skytexturesets.h"
#include "envsetskytexture.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_setskytexture, CEnvSetSkyTexture);

//=============================================
// @brief
//
//=============================================
CEnvSetSkyTexture::CEnvSetSkyTexture( edict_t* pedict ):
	CPointEntity(pedict),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSetSkyTexture::~CEnvSetSkyTexture( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvSetSkyTexture::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSetSkyTexture, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CEnvSetSkyTexture::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->netname == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		Util::RemoveEntity(this);
		return false;
	}

	// Manage custom sky texture
	const Char* pstrSkyTexture = gd_engfuncs.pfnGetString(m_pFields->netname);
	if(pstrSkyTexture)
		m_pState->body = gSkyTextureSets.RegisterSkyTextureSet(pstrSkyTexture);

	// Check for start on flag
	if(HasSpawnFlag(FL_START_ON))
		m_isActive = true;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CEnvSetSkyTexture::Restore( void )
{
	if(!CPointEntity::Restore())
		return false;

	// Manage custom sky texture
	if(m_pFields->netname != NO_STRING_VALUE)
	{
		const Char* pstrSkyTexture = gd_engfuncs.pfnGetString(m_pFields->netname);
		if(pstrSkyTexture)
			m_pState->body = gSkyTextureSets.RegisterSkyTextureSet(pstrSkyTexture);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvSetSkyTexture::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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

	if(prevstate != m_isActive)
		SendInitMessage(nullptr);
}

//=============================================
// @brief
//
//=============================================
void CEnvSetSkyTexture::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(pPlayer && !m_isActive)
		return;

	if(!m_isActive && HasSpawnFlag(FL_NO_OFF_MESSAGE))
		return;

	if(pPlayer)
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setskytexture, nullptr, pPlayer->GetEdict());
	else
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.setskytexture, nullptr, nullptr);

	gd_engfuncs.pfnMsgWriteChar(m_isActive ? m_pState->body : NO_POSITION);
	gd_engfuncs.pfnUserMessageEnd();
}