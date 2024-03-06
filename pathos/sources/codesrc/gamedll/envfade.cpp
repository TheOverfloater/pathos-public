/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envfade.h"
#include "screenfade.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_fade, CEnvFade);

//=============================================
// @brief
//
//=============================================
CEnvFade::CEnvFade( edict_t* pedict ):
	CPointEntity(pedict),
	m_duration(0),
	m_holdtime(0),
	m_useTime(-1),
	m_layer(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvFade::~CEnvFade( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvFade::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvFade, m_duration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvFade, m_holdtime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvFade, m_useTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvFade, m_layer, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CEnvFade::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "duration"))
	{
		m_duration = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "holdtime"))
	{
		m_holdtime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "layer"))
	{
		m_layer = SDL_atoi(kv.value);
		m_layer = clamp(m_layer, 0, (MAX_FADE_LAYERS-1));
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvFade::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(HasSpawnFlag(FL_ACTIVATE_ON_START))
		m_useTime = g_pGameVars->time;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvFade::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	Int32 flags = 0;
	if(!HasSpawnFlag(FL_FADE_IN))
		flags |= FL_FADE_OUT;
	if(HasSpawnFlag(FL_FADE_IN))
		flags |= FL_FADE_MODULATE;
	if(HasSpawnFlag(FL_PERMANENT))
		flags |= FL_FADE_PERMANENT;
	if(HasSpawnFlag(FL_STAYOUT))
		flags |= FL_FADE_STAYOUT;

	if(HasSpawnFlag(FL_ONLY_ONE_CLIENT))
	{
		if(!pActivator->IsPlayer())
		{
			Util::EntityConPrintf(m_pEdict, "Not a player.\n");
			return;
		}

		m_useTime = g_pGameVars->time;
		Util::ScreenFadePlayer(pActivator->GetEdict(), 
			m_pState->rendercolor, 
			m_duration, 
			m_holdtime, 
			m_pState->renderamt, 
			flags, 
			m_layer, 
			0);
	}
	else
	{
		m_useTime = g_pGameVars->time;
		Util::ScreenFadeAllPlayers(m_pState->rendercolor, 
			m_duration, 
			m_holdtime, 
			m_pState->renderamt, 
			flags, 
			m_layer, 
			0);
	}

	UseTargets(this, USE_TOGGLE, 0);
}

//=============================================
// @brief
//
//=============================================
void CEnvFade::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(m_useTime == -1)
		return;

	// Clear if we're over the time
	if(m_useTime+m_duration+m_holdtime <= g_pGameVars->time)
	{
		m_useTime = -1;
		return;
	}

	// Do not restore for MP
	if(HasSpawnFlag(FL_ONLY_ONE_CLIENT))
		return;

	Int32 flags = 0;
	if(!HasSpawnFlag(FL_FADE_IN))
		flags |= FL_FADE_OUT;
	if(HasSpawnFlag(FL_FADE_IN))
		flags |= FL_FADE_MODULATE;
	if(HasSpawnFlag(FL_PERMANENT))
		flags |= FL_FADE_PERMANENT;
	if(HasSpawnFlag(FL_FADE_STAYOUT))
		flags |= FL_FADE_STAYOUT;

	Float timeoffset = g_pGameVars->time - m_useTime;
	Util::ScreenFadeAllPlayers(m_pState->rendercolor, 
		m_duration, 
		m_holdtime, 
		m_pState->renderamt, 
		flags, 
		m_layer, 
		timeoffset);
}