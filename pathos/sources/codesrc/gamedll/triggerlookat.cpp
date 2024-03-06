/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerlookat.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_lookat, CTriggerLookAt);

//=============================================
// @brief
//
//=============================================
CTriggerLookAt::CTriggerLookAt( edict_t* pedict ):
	CPointEntity(pedict),
	m_radius(0),
	m_lookTime(0),
	m_accumulatedTime(0),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerLookAt::~CTriggerLookAt( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerLookAt::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerLookAt, m_radius, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerLookAt, m_lookTime, EFIELD_DOUBLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerLookAt, m_accumulatedTime, EFIELD_DOUBLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerLookAt, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerLookAt::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "radius"))
	{
		m_radius = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "time"))
	{
		m_lookTime = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerLookAt::Spawn( void )
{
	if(m_pFields->target == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE || !HasSpawnFlag(FL_START_OFF))
		m_isActive = true;

	if(m_isActive)
	{
		SetThink(&CTriggerLookAt::LookAtThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerLookAt::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(m_isActive)
		return;

	SetThink(&CTriggerLookAt::LookAtThink);
	m_pState->nextthink = g_pGameVars->time + 0.1;
	m_isActive = true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerLookAt::LookAtThink( void )
{
	CBaseEntity* pPlayer = Util::GetHostPlayer();

	if( m_radius > 0 )
	{
		Float flDist = (m_pState->origin - pPlayer->GetOrigin()).Length();
		if(flDist > m_radius)
		{
			if(m_accumulatedTime)
			{
				// Clear accumulated time if player is out of radius
				m_accumulatedTime = 0;
			}

			SetThink( &CTriggerLookAt::LookAtThink );
			m_pState->nextthink = g_pGameVars->time + 0.1;
			return;
		}
	}

	if(!(pPlayer->GetFlags() & FL_ONGROUND))
	{
		if(m_accumulatedTime)
		{
			// Clear accumulated time if player's not on ground
			m_accumulatedTime = 0;
		}

		SetThink( &CTriggerLookAt::LookAtThink );
		m_pState->nextthink = g_pGameVars->time + 0.1;
		return;
	}

	Vector vecDirToPlayer = m_pState->origin - pPlayer->GetEyePosition();
	vecDirToPlayer = vecDirToPlayer.Normalize();

	Vector vecPlayerForward;
	Math::AngleVectors(pPlayer->GetViewAngles(), &vecPlayerForward, nullptr, nullptr);
	vecPlayerForward = vecPlayerForward.Normalize();

	Float dotProduct = Math::DotProduct( vecPlayerForward, vecDirToPlayer );
	if ( dotProduct < 0.8 )	// +/- 15 degrees or so
	{
		if(m_accumulatedTime)
		{
			// Clear accumulated time if we're out of view
			m_accumulatedTime = 0;
		}

		SetThink( &CTriggerLookAt::LookAtThink );
		m_pState->nextthink = g_pGameVars->time + 0.1;
		return;
	}

	trace_t tr;
	Util::TraceLine(pPlayer->GetEyePosition(true), m_pState->origin, true, false, HasSpawnFlag(FL_IGNORE_GLASS) ? true : false, pPlayer->GetEdict(), tr);
	if(!tr.noHit())
	{
		if(m_accumulatedTime)
		{
			// Clear accumulated time if we're occluded
			m_accumulatedTime = 0;
		}

		SetThink( &CTriggerLookAt::LookAtThink );
		m_pState->nextthink = g_pGameVars->time + 0.1;
		return;
	}

	if(m_lookTime)
	{
		// Increment looked-at time
		m_accumulatedTime += 0.1;

		if(m_accumulatedTime < m_lookTime)
		{
			SetThink( &CTriggerLookAt::LookAtThink );
			m_pState->nextthink = g_pGameVars->time + 0.1;
			return;
		}
	}

	// Fire targets and remove
	Util::FireTargets(gd_engfuncs.pfnGetString(m_pFields->target), this, this, USE_TOGGLE, 0);
	Util::RemoveEntity(this);
}