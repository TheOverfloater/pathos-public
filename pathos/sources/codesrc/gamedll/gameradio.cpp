/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "gameradio.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(game_radio, CGameRadio);

//=============================================
// @brief
//
//=============================================
CGameRadio::CGameRadio( edict_t* pedict ):
	CPointEntity(pedict),
	m_beginTime(0),
	m_pPlayer(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CGameRadio::~CGameRadio( void )
{
}

//=============================================
// @brief
//
//=============================================
void CGameRadio::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CGameRadio, m_beginTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameRadio, m_pPlayer, EFIELD_ENTPOINTER));
}

//=============================================
// @brief
//
//=============================================
bool CGameRadio::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CGameRadio::Precache( void )
{
	if(m_pFields->message == NO_STRING_VALUE)
		return;

	const Char* pstrsoundfile = gd_engfuncs.pfnGetString(m_pFields->message);
	gd_engfuncs.pfnPrecacheSound(pstrsoundfile);
}

//=============================================
// @brief
//
//=============================================
bool CGameRadio::Restore( void )
{
	if(!m_pPlayer)
	{
		m_beginTime = 0;
		m_pPlayer = nullptr;
	}

	if(m_pFields->message == NO_STRING_VALUE || !m_beginTime)
		return true;

	const Char* pstrsoundfile = gd_engfuncs.pfnGetString(m_pFields->message);
	Float duration = gd_engfuncs.pfnGetSoundDuration(pstrsoundfile, PITCH_NORM);

	if(m_beginTime + duration <= g_pGameVars->time)
	{
		m_beginTime = 0;
		m_pPlayer = nullptr;
		return true;
	}
	
	Float soundremainder = g_pGameVars->time - m_beginTime;
	gd_engfuncs.pfnPlayEntitySound(m_pPlayer->GetEntityIndex(), pstrsoundfile, HasSpawnFlag(FL_NO_MUTE) ? SND_FL_NONE : SND_FL_RADIO, SND_CHAN_STATIC, VOL_NORM, ATTN_NORM, PITCH_NORM, soundremainder, m_pPlayer->GetEntityIndex()-1);
	return true;
}

//=============================================
// @brief
//
//=============================================
void CGameRadio::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(m_pFields->message == NO_STRING_VALUE)
		return;

	// Either use activator, or assume it's local player
	CBaseEntity* pEntity;
	if(pActivator && pActivator->IsPlayer())
		pEntity = pActivator;
	else
		pEntity = Util::GetHostPlayer();

	// Get sound file path
	const Char* pstrsoundfile = gd_engfuncs.pfnGetString(m_pFields->message);
	gd_engfuncs.pfnPlayEntitySound(pEntity->GetEntityIndex(), pstrsoundfile, HasSpawnFlag(FL_NO_MUTE) ? SND_FL_NONE : SND_FL_RADIO, SND_CHAN_STATIC, VOL_NORM, ATTN_NORM, PITCH_NORM, 0, pEntity->GetClientIndex());

	// Set radio icon
	if(m_pFields->message != NO_STRING_VALUE)
	{
		const Char* pstrcallername = gd_engfuncs.pfnGetString(m_pFields->netname);

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.radiomessage, nullptr, pEntity->GetEdict());
			gd_engfuncs.pfnMsgWriteInt16(m_pEdict->entindex);
			gd_engfuncs.pfnMsgWriteString(pstrcallername);
			gd_engfuncs.pfnMsgWriteSmallFloat(m_pState->health);
			gd_engfuncs.pfnMsgWriteByte(m_pState->rendercolor.x);
			gd_engfuncs.pfnMsgWriteByte(m_pState->rendercolor.y);
			gd_engfuncs.pfnMsgWriteByte(m_pState->rendercolor.z);
			gd_engfuncs.pfnMsgWriteByte(m_pState->renderamt);
		gd_engfuncs.pfnUserMessageEnd();
	}

	// Remove entity if flagged
	if(HasSpawnFlag(FL_REMOVE_ON_FIRE))
	{
		Util::RemoveEntity(this);
		return;
	}

	m_beginTime = g_pGameVars->time;
	m_pPlayer = pEntity;
}