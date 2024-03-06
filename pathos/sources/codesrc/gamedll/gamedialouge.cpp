/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "gamedialouge.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(game_dialouge, CGameDialouge);

//=============================================
// @brief
//
//=============================================
CGameDialouge::CGameDialouge( edict_t* pedict ):
	CPointEntity(pedict),
	m_radius(0),
	m_isActive(false),
	m_beginTime(0),
	m_duration(0)
{
}

//=============================================
// @brief
//
//=============================================
CGameDialouge::~CGameDialouge( void )
{
}

//=============================================
// @brief
//
//=============================================
void CGameDialouge::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CGameDialouge, m_radius, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameDialouge, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameDialouge, m_beginTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameDialouge, m_duration, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CGameDialouge::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "radius"))
	{
		m_radius = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CGameDialouge::Spawn( void )
{
	if(m_pFields->message == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CPointEntity::Spawn())
		return false;

	// Set duration
	m_duration = gd_engfuncs.pfnGetSoundDuration(gd_engfuncs.pfnGetString(m_pFields->message), PITCH_NORM);

	if(!HasSpawnFlag(FL_START_OFF))
		m_isActive = true;

	if(m_isActive && (m_radius || HasSpawnFlag(FL_LOOK_AT)))
	{
		SetThink(&CGameDialouge::DialougeThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CGameDialouge::Precache( void )
{
	if(m_pFields->message == NO_STRING_VALUE)
		return;

	gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(m_pFields->message));
}

//=============================================
// @brief
//
//=============================================
void CGameDialouge::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!m_isActive)
	{
		// Begin thinking if needed
		if( m_radius || HasSpawnFlag(FL_LOOK_AT) )
		{
			SetThink( &CGameDialouge::DialougeThink );
			m_pState->nextthink = g_pGameVars->time + 0.1;
		}

		m_isActive = true;
		return;
	}
	else
	{
		m_beginTime = g_pGameVars->time;

		CBaseEntity* pPlayer;
		if(pActivator && pActivator->IsPlayer())
			pPlayer = pActivator;
		else
			pPlayer = Util::GetHostPlayer();

		Util::EmitEntitySound(pPlayer, m_pFields->message, SND_CHAN_STATIC, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_DIALOUGE);
		pPlayer->SetDialougeDuration(m_duration);

		if(!HasSpawnFlag(FL_REPEATABLE))
		{
			SetThink(&CBaseEntity::RemoveThink);
			m_pState->nextthink = g_pGameVars->time + m_duration;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CGameDialouge::DialougeThink( void )
{
	if(!m_radius && !HasSpawnFlag(FL_LOOK_AT))
		return;

	CBaseEntity* pPlayer = Util::GetHostPlayer();

	if( m_radius )
	{
		float flDist = (m_pState->origin - pPlayer->GetOrigin()).Length();
		if(flDist > m_radius)
		{
			SetThink( &CGameDialouge::DialougeThink );
			m_pState->nextthink = g_pGameVars->time + 0.1;
			return;
		}
	}

	if( HasSpawnFlag(FL_LOOK_AT) )
	{
		if(!(pPlayer->GetFlags() & FL_ONGROUND))
		{
			SetThink( &CGameDialouge::DialougeThink );
			m_pState->nextthink = g_pGameVars->time + 0.1;
			return;
		}

		Vector playerEyes = pPlayer->GetEyePosition();
		Vector vecDirToPlayer = m_pState->origin - pPlayer->GetEyePosition(true);
		vecDirToPlayer = vecDirToPlayer.Normalize();

		Vector vecPlayerForward;
		Math::AngleVectors(pPlayer->GetViewAngles(), &vecPlayerForward, nullptr, nullptr);
		vecPlayerForward = vecPlayerForward.Normalize();

		Float dotProduct = Math::DotProduct( vecPlayerForward, vecDirToPlayer );
		if ( dotProduct < 0.8 )
		{
			SetThink( &CGameDialouge::DialougeThink );
			m_pState->nextthink = g_pGameVars->time + 0.1;
			return;
		}

		trace_t tr;
		Util::TraceLine(playerEyes, m_pState->origin, true, false, true, pPlayer->GetEdict(), tr);
		if(tr.fraction != 1.0)
		{
			SetThink( &CGameDialouge::DialougeThink );
			m_pState->nextthink = g_pGameVars->time + 0.1;
			return;
		}
	}

	m_beginTime = g_pGameVars->time;
	Util::EmitEntitySound(pPlayer, m_pFields->message, SND_CHAN_STATIC, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_DIALOUGE);
	pPlayer->SetDialougeDuration(m_duration);

	if(m_pFields->target != NO_STRING_VALUE)
		Util::FireTargets(gd_engfuncs.pfnGetString(m_pFields->target), this, this, USE_TOGGLE, 0);

	SetThink(&CBaseEntity::RemoveThink);
	m_pState->nextthink = g_pGameVars->time + m_duration;
}

//=============================================
// @brief
//
//=============================================
void CGameDialouge::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(!m_beginTime)
		return;

	if(m_beginTime + m_duration <= g_pGameVars->time)
	{
		Util::RemoveEntity(this);
		return;
	}

	const Char* pstrFilepath = gd_engfuncs.pfnGetString(m_pFields->message);
	gd_engfuncs.pfnPlayEntitySound(pPlayer->GetEntityIndex(), pstrFilepath, SND_FL_DIALOUGE, SND_CHAN_VOICE, VOL_NORM, ATTN_NONE, PITCH_NORM, (g_pGameVars->time - m_beginTime), pPlayer->GetClientIndex());
}

//=============================================
// @brief
//
//=============================================
CGameDialouge* CGameDialouge::CreateDialouge( const Char* pstrPath, const Vector& origin, Float radius, bool visibleOnly )
{
	edict_t *pEntity = gd_engfuncs.pfnCreateEntity("game_dialouge");
	if(!pEntity)
		return nullptr;

	CGameDialouge *pDialouge = reinterpret_cast<CGameDialouge *>(CBaseEntity::GetClass( pEntity ));

	gd_engfuncs.pfnSetOrigin(pEntity, origin);
	pEntity->fields.message = gd_engfuncs.pfnAllocString(pstrPath);
	pDialouge->m_radius = radius;

	if(visibleOnly)
		pEntity->state.spawnflags |= FL_LOOK_AT;

	DispatchSpawn( pEntity );
	return pDialouge;
}