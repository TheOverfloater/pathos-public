/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envblood.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_blood, CEnvBlood);

//=============================================
// @brief
//
//=============================================
CEnvBlood::CEnvBlood( edict_t* pedict ):
	CPointEntity(pedict),
	m_bloodColor(BLOOD_RED),
	m_bloodAmount(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvBlood::~CEnvBlood( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvBlood::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlood, m_bloodColor, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlood, m_bloodAmount, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CEnvBlood::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "color"))
	{
		Int32 color = SDL_atoi(kv.value);
		switch(color)
		{
		case COLOR_YELLOW:
			m_bloodColor = COLOR_YELLOW;
			break;
		case COLOR_RED:
		default:
			m_bloodColor = COLOR_RED;
			break;
		}
		return true;
	}
	else if(!qstrcmp(kv.keyname, "amount"))
	{
		m_bloodAmount = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvBlood::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	Util::SetMoveDirection(*m_pState);
	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvBlood::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Spawn blood particles
	Util::SpawnBloodParticles(GetBloodPosition(pActivator), GetBloodDirection(), GetBloodColor(), false);

	// Spawn decal if needed
	if(HasSpawnFlag(FL_DECAL))
	{
		Vector forward = GetBloodDirection();
		Vector traceStart = GetBloodPosition(pActivator);
		Vector traceEnd = traceStart + forward * GetBloodAmount() * 2;

		trace_t tr;
		Util::TraceLine(traceStart, traceEnd, true, false, nullptr, tr);
		if(!tr.noHit())
			Util::SpawnBloodDecal(tr, GetBloodColor(), true);
	}
}

//=============================================
// @brief
//
//=============================================
bloodcolor_t CEnvBlood::GetBloodColor( void )
{
	return (bloodcolor_t)m_bloodColor;
}

//=============================================
// @brief
//
//=============================================
Float CEnvBlood::GetBloodAmount( void ) const
{
	return m_bloodAmount;
}

//=============================================
// @brief
//
//=============================================
void CEnvBlood::SetBloodColor( bloodcolor_t color )
{
	m_bloodColor = color;
}

//=============================================
// @brief
//
//=============================================
void CEnvBlood::SetBloodAmount( Float amount )
{
	m_bloodAmount = amount;
}

//=============================================
// @brief
//
//=============================================
Vector CEnvBlood::GetBloodDirection( void ) const
{
	if(HasSpawnFlag(FL_RANDOM))
		return Util::GetRandomBloodVector();
	else
		return m_pState->movedir;
}

//=============================================
// @brief
//
//=============================================
Vector CEnvBlood::GetBloodPosition( CBaseEntity* pActivator ) const
{
	Vector result;
	if(HasSpawnFlag(FL_PLAYER))
	{
		CBaseEntity* pPlayer;
		if(pActivator && pActivator->IsPlayer())
			pPlayer = pActivator;
		else
			pPlayer = Util::GetHostPlayer();

		if(pPlayer)
		{
			Vector randomoffs;
			for(Uint32 i = 0; i < 3; i++)
				randomoffs[i] = Common::RandomFloat(-10, 10);

			result = pPlayer->GetEyePosition() + randomoffs;
		}
	}
	else
	{
		// Just return the origin
		result = m_pState->origin;
	}

	return result;
}