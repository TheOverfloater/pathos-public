/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "gametimer.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(game_timer, CGameTimer);

//=============================================
// @brief
//
//=============================================
CGameTimer::CGameTimer( edict_t* pedict ):
	CPointEntity(pedict),
	m_mode(TIMER_MODE_SET),
	m_duration(0)
{
}

//=============================================
// @brief
//
//=============================================
CGameTimer::~CGameTimer( void )
{
}

//=============================================
// @brief
//
//=============================================
void CGameTimer::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CGameTimer, m_mode, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameTimer, m_duration, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CGameTimer::Spawn( void )
{
	if(m_mode == TIMER_MODE_SET 
		&& m_duration <= 0)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CGameTimer::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "mode"))
	{
		Int32 mode = SDL_atoi(kv.value);
		switch(mode)
		{
		case TIMER_MODE_SET:
		case TIMER_MODE_CLEAR:
			break;
		default:
			{
				Util::EntityConPrintf(m_pEdict, "Invalid value %d for field %d.\n", kv.value, kv.keyname);
				return false;
			}
			break;
		}

		m_mode = mode;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "duration"))
	{
		m_duration = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CGameTimer::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	CBaseEntity* pPlayer;
	if(pActivator && pActivator->IsPlayer())
		pPlayer = pActivator;
	else
		pPlayer = Util::GetHostPlayer();

	switch(m_mode)
	{
	case TIMER_MODE_SET:
		pPlayer->SetCountdownTimer(m_duration, gd_engfuncs.pfnGetString(m_pFields->netname));
		break;
	case TIMER_MODE_CLEAR:
		pPlayer->ClearCountdownTimer();
		break;
	}
}
