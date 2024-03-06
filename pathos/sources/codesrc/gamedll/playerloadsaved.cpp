/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "playerloadsaved.h"
#include "screenfade.h"

// True if we don't allow saving games at this time
bool CPlayerLoadSaved::m_isBlockingSaveGame = false;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(player_loadsaved, CPlayerLoadSaved);

//=============================================
// @brief
//
//=============================================
CPlayerLoadSaved::CPlayerLoadSaved( edict_t* pedict ):
	CPointEntity(pedict),
	m_messageTime(0),
	m_loadTime(0),
	m_duration(0),
	m_holdTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CPlayerLoadSaved::~CPlayerLoadSaved( void )
{
}

//=============================================
// @brief
//
//=============================================
void CPlayerLoadSaved::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerLoadSaved, m_messageTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerLoadSaved, m_loadTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerLoadSaved, m_duration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerLoadSaved, m_holdTime, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CPlayerLoadSaved::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "duration"))
	{
		m_duration = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "holdtime"))
	{
		m_holdTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "messagetime"))
	{
		m_messageTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "loadtime"))
	{
		m_loadTime = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CPlayerLoadSaved::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(m_messageTime > m_loadTime)
		Util::EntityConPrintf(m_pEdict, "Message time is set after load time.\n");

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerLoadSaved::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	Util::ScreenFadeAllPlayers(m_pState->rendercolor, m_duration, m_holdTime, m_pState->renderamt, FL_FADE_OUT|FL_FADE_CLEARGAME);

	// Set this to true
	m_isBlockingSaveGame = true;

	if(m_pFields->message != NO_STRING_VALUE)
	{
		SetThink(&CPlayerLoadSaved::MessageThink);
		m_pState->nextthink = g_pGameVars->time + m_messageTime;
	}
	else
	{
		SetThink(&CPlayerLoadSaved::LoadThink);
		m_pState->nextthink = g_pGameVars->time + m_loadTime;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerLoadSaved::MessageThink( void )
{
	Util::ShowMessageAllPlayers(gd_engfuncs.pfnGetString(m_pFields->message));

	Double loadTime = m_loadTime - m_messageTime;
	if(loadTime <= 0)
	{
		LoadThink();
		return;
	}

	SetThink(&CPlayerLoadSaved::LoadThink);
	m_pState->nextthink = g_pGameVars->time + loadTime;
}

//=============================================
// @brief
//
//=============================================
void CPlayerLoadSaved::LoadThink( void )
{
	if(g_pGameVars->maxclients > 1)
		return;

	gd_engfuncs.pfnServerCommand("reload\n");
}

//=============================================
// @brief
//
//=============================================
void CPlayerLoadSaved::ClearSaveGameBlock( void )
{
	m_isBlockingSaveGame = false;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerLoadSaved::IsBlockingSaving( void )
{
	return m_isBlockingSaveGame;
}