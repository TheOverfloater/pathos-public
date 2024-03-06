/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envfog.h"

// Current fog end distance
Uint32 CEnvFog::g_fogEndDist = 0;
// Fog ideal distance to set
Uint32 CEnvFog::g_fogIdealEndDist = 0;
// Time until fog blending is done
Double CEnvFog::g_fogBlendTime = 0;
// TRUE if fog should affect skybox
bool CEnvFog::g_fogAffectSky = true;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_fog, CEnvFog);

//=============================================
// @brief
//
//=============================================
CEnvFog::CEnvFog( edict_t* pedict ):
	CPointEntity(pedict),
	m_startDistance(0),
	m_endDistance(0),
	m_blendTime(0),
	m_isActive(false),
	m_dontAffectSky(false)
{
}

//=============================================
// @brief
//
//=============================================
CEnvFog::~CEnvFog( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvFog::DeclareSaveFields( void )
{
	// Call base class to handle it's own
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvFog, m_startDistance, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvFog, m_endDistance, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvFog, m_blendTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvFog, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvFog, m_dontAffectSky, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CEnvFog::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE
		|| HasSpawnFlag(FL_START_ON))
		m_isActive = true;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CEnvFog::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "startdist"))
	{
		m_startDistance = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "enddist"))
	{
		m_endDistance = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "blendtime"))
	{
		m_blendTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "affectsky"))
	{
		m_dontAffectSky = (SDL_atoi(kv.value) == 0) ? false : true;
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CEnvFog::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(pPlayer && !m_isActive)
		return;

	if(m_isActive)
	{
		if(HasSpawnFlag(FL_CLEAR_ON_FIRST))
		{
			if(pPlayer)
				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setfog, nullptr, pPlayer->GetEdict());
			else
				gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.setfog, nullptr, nullptr);

			gd_engfuncs.pfnMsgWriteInt16(NO_ENTITY_INDEX);
			gd_engfuncs.pfnMsgWriteByte(0);
			gd_engfuncs.pfnMsgWriteByte(0);
			gd_engfuncs.pfnMsgWriteByte(0);
			gd_engfuncs.pfnMsgWriteFloat(0);
			gd_engfuncs.pfnMsgWriteFloat(0);
			gd_engfuncs.pfnMsgWriteByte(0);
			gd_engfuncs.pfnMsgWriteSmallFloat(0);
			gd_engfuncs.pfnUserMessageEnd();

			m_pState->spawnflags &= ~FL_CLEAR_ON_FIRST;
		}

		if(pPlayer)
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setfog, nullptr, pPlayer->GetEdict());
		else
			gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.setfog, nullptr, nullptr);

		gd_engfuncs.pfnMsgWriteInt16(GetEntityIndex());
		gd_engfuncs.pfnMsgWriteByte(m_pState->rendercolor.x);
		gd_engfuncs.pfnMsgWriteByte(m_pState->rendercolor.y);
		gd_engfuncs.pfnMsgWriteByte(m_pState->rendercolor.z);
		gd_engfuncs.pfnMsgWriteFloat(m_startDistance);
		gd_engfuncs.pfnMsgWriteFloat(m_endDistance);
		gd_engfuncs.pfnMsgWriteByte(m_dontAffectSky);
		gd_engfuncs.pfnMsgWriteSmallFloat(m_blendTime);
		gd_engfuncs.pfnUserMessageEnd();

		if(!g_fogEndDist && !g_fogBlendTime)
			SetFogCullParams(m_endDistance, 0, m_dontAffectSky ? false : true);
	}
	else if(!HasSpawnFlag(FL_NO_OFF_MESSAGE))
	{
		if(pPlayer)
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setfog, nullptr, pPlayer->GetEdict());
		else
			gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.setfog, nullptr, nullptr);

		gd_engfuncs.pfnMsgWriteInt16(GetEntityIndex());
		gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnMsgWriteFloat(0);
		gd_engfuncs.pfnMsgWriteFloat(0);
		gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnMsgWriteSmallFloat(0);
		gd_engfuncs.pfnUserMessageEnd();
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvFog::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool prevstate = m_isActive;
	switch(useMode)
	{
	case USE_OFF:
		m_isActive = false;
		break;
	case USE_ON:
		m_isActive = true;
		break;
	case USE_TOGGLE:
	default:
		m_isActive = !m_isActive;
		break;
	}

	if(prevstate != m_isActive)
	{
		if(m_isActive)
			SetFogCullParams(m_endDistance, m_blendTime, m_dontAffectSky ? false : true);
		else if(!HasSpawnFlag(FL_NO_OFF_MESSAGE))
			SetFogCullParams(0, 0, false);

		SendInitMessage(nullptr);
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvFog::SetFogCullParams( Float endDistance, Float blendTime, bool affectSky )
{
	if(affectSky != g_fogAffectSky)
	{
		g_fogEndDist = 0;
		g_fogIdealEndDist = endDistance;
		g_fogBlendTime = g_pGameVars->time + blendTime;
	}
	else if(g_fogEndDist > endDistance)
	{
		g_fogIdealEndDist = endDistance;
		g_fogBlendTime = g_pGameVars->time + blendTime;
	}
	else
	{
		g_fogEndDist = endDistance;
		g_fogBlendTime = 0;
	}

	g_fogAffectSky = affectSky;
}

//=============================================
// @brief
//
//=============================================
void CEnvFog::ClearFogGlobals( void )
{
	g_fogEndDist = 0;
	g_fogIdealEndDist = 0;
	g_fogBlendTime = 0;
	g_fogAffectSky = true;
}

//=============================================
// @brief
//
//=============================================
void CEnvFog::UpdateFogGlobals( void )
{
	if(!g_fogBlendTime)
		return;

	if(g_fogBlendTime <= g_pGameVars->time)
	{
		g_fogEndDist = g_fogIdealEndDist;
		g_fogBlendTime = 0;
	}
}

//=============================================
// @brief
//
//=============================================
bool CEnvFog::FogCull( const edict_t& client, const edict_t& entity )
{
	if(!g_fogEndDist || !g_fogAffectSky)
		return false;

	const CBaseEntity* pEntity = CBaseEntity::GetClass(&client);
	if(!pEntity)
		return false;

	Vector playerViewOrigin = pEntity->GetVISEyePosition();

	Vector fogmins, fogmaxs;
	for(Uint32 i = 0; i < 3; i++)
	{
		fogmins[i] = playerViewOrigin[i]-g_fogEndDist;
		fogmaxs[i] = playerViewOrigin[i]+g_fogEndDist;
	}

	if(Math::CheckMinsMaxs(fogmins, fogmaxs, entity.state.absmin, entity.state.absmax))
		return true;
	else
		return false;
}