/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envposworld.h"
#include "skytexturesets.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(envpos_world, CEnvPosWorld);

//=============================================
// @brief
//
//=============================================
CEnvPosWorld::CEnvPosWorld( edict_t* pedict ):
	CPointEntity(pedict),
	m_fogStartDist(0),
	m_fogEndDist(0),
	m_dontAffectSky(false),
	m_isActive(false),
	m_skyTextureName(NO_STRING_VALUE),
	m_skyTextureSetIndex(NO_POSITION)
{
}

//=============================================
// @brief
//
//=============================================
CEnvPosWorld::~CEnvPosWorld( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvPosWorld::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvPosWorld, m_fogStartDist, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvPosWorld, m_fogEndDist, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvPosWorld, m_dontAffectSky, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvPosWorld, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvPosWorld, m_skyTextureName, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CEnvPosWorld::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "enddist"))
	{
		m_fogEndDist = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "startdist"))
	{
		m_fogStartDist = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "affectsky"))
	{
		m_dontAffectSky = (SDL_atoi(kv.value) == 1) ? true : false;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "skytexture"))
	{
		m_skyTextureName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvPosWorld::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE
		|| HasSpawnFlag(FL_START_ON))
		m_isActive = true;

	// Manage custom sky texture
	if(m_skyTextureName != NO_STRING_VALUE)
	{
		const Char* pstrSkyTexture = gd_engfuncs.pfnGetString(m_skyTextureName);
		if(pstrSkyTexture)
			m_skyTextureSetIndex = gSkyTextureSets.RegisterSkyTextureSet(pstrSkyTexture);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CEnvPosWorld::Restore( void )
{
	if(!CPointEntity::Restore())
		return false;

	// Manage custom sky texture
	if(m_skyTextureName != NO_STRING_VALUE)
	{
		const Char* pstrSkyTexture = gd_engfuncs.pfnGetString(m_skyTextureName);
		if(pstrSkyTexture)
			m_skyTextureSetIndex = gSkyTextureSets.RegisterSkyTextureSet(pstrSkyTexture);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvPosWorld::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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
void CEnvPosWorld::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(pPlayer && !m_isActive)
		return;

	if(!m_isActive && !HasSpawnFlag(FL_SEND_OFF_MESSAGE))
		return;

	if(pPlayer)
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.skyboxparameters, nullptr, pPlayer->GetEdict());
	else
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.skyboxparameters, nullptr, nullptr);

	gd_engfuncs.pfnMsgWriteByte(m_isActive);

	if(m_isActive)
	{
		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteFloat(m_pState->origin[i]);

		gd_engfuncs.pfnMsgWriteSmallFloat(m_pState->health);
		gd_engfuncs.pfnMsgWriteFloat(m_fogStartDist);
		gd_engfuncs.pfnMsgWriteFloat(m_fogEndDist);

		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteByte(m_pState->rendercolor[i]);

		gd_engfuncs.pfnMsgWriteByte(m_dontAffectSky);
		gd_engfuncs.pfnMsgWriteChar(m_skyTextureSetIndex);
	}

	gd_engfuncs.pfnUserMessageEnd();
}