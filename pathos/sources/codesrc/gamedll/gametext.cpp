/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "gametext.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(game_text, CGameText);

//=============================================
// @brief
//
//=============================================
CGameText::CGameText( edict_t* pedict ):
	CPointEntity(pedict),
	m_channel(0),
	m_positionX(0),
	m_positionY(0),
	m_effect(0),
	m_fadeInTime(0),
	m_fadeOutTime(0),
	m_holdTime(0),
	m_fxTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CGameText::~CGameText( void )
{
}

//=============================================
// @brief
//
//=============================================
void CGameText::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CGameText, m_channel, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameText, m_positionX, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameText, m_positionY, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameText, m_effect, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CGameText, m_color1, EFIELD_BYTE, sizeof(color32_t)));
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CGameText, m_color2, EFIELD_BYTE, sizeof(color32_t)));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameText, m_fadeInTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameText, m_fadeOutTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameText, m_holdTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameText, m_fxTime, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CGameText::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "channel"))
	{
		m_channel = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "x"))
	{
		m_positionX = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "y"))
	{
		m_positionY = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "effect"))
	{
		m_effect = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "color"))
	{
		Util::ReadColor32FromString(kv.value, m_color1);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "color2"))
	{
		Util::ReadColor32FromString(kv.value, m_color2);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "fadein"))
	{
		m_fadeInTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "fadeout"))
	{
		m_fadeOutTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "holdtime"))
	{
		m_holdTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "fxtime"))
	{
		m_fxTime = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CGameText::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	CBaseEntity* pPlayer = nullptr;
	if(!HasSpawnFlag(FL_ALL_PLAYERS))
	{
		if(pActivator && pActivator->IsPlayer())
			pPlayer = pActivator;
		else
			pPlayer = Util::GetHostPlayer();
	}
	else
	{
		// All players
		pPlayer = nullptr;
	}

	if(pPlayer)
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.showcustommessage, nullptr, pPlayer->GetEdict());
	else
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.showcustommessage, nullptr, nullptr);
	gd_engfuncs.pfnMsgWriteSmallFloat(m_positionX);
	gd_engfuncs.pfnMsgWriteSmallFloat(m_positionY);
	gd_engfuncs.pfnMsgWriteByte(m_effect);
	gd_engfuncs.pfnMsgWriteByte(m_color1.r);
	gd_engfuncs.pfnMsgWriteByte(m_color1.g);
	gd_engfuncs.pfnMsgWriteByte(m_color1.b);
	gd_engfuncs.pfnMsgWriteByte(m_color2.r);
	gd_engfuncs.pfnMsgWriteByte(m_color2.g);
	gd_engfuncs.pfnMsgWriteByte(m_color2.b);
	gd_engfuncs.pfnMsgWriteSmallFloat(m_fadeInTime);
	gd_engfuncs.pfnMsgWriteSmallFloat(m_fadeOutTime);
	gd_engfuncs.pfnMsgWriteSmallFloat(m_holdTime);
	gd_engfuncs.pfnMsgWriteSmallFloat(m_fxTime);
	gd_engfuncs.pfnMsgWriteByte(m_channel);
	gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_pFields->message));
	gd_engfuncs.pfnUserMessageEnd();
}