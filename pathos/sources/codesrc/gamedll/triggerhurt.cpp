/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerhurt.h"

// Damage delay time
const Float CTriggerHurt::DEFAULT_DMG_DELAY = 0.5;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_hurt, CTriggerHurt);

//=============================================
// @brief
//
//=============================================
CTriggerHurt::CTriggerHurt( edict_t* pedict ):
	CTriggerEntity(pedict),
	m_isActive(false),
	m_nextDamageTime(0),
	m_damageTime(0),
	m_dmgAmount(0),
	m_dmgDelay(0),
	m_bitsDamageInflict(0),
	m_playerDamageBits(0)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerHurt::~CTriggerHurt( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerHurt::DeclareSaveFields( void )
{
	// Call base class to do it first
	CTriggerEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerHurt, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerHurt, m_nextDamageTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerHurt, m_damageTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerHurt, m_dmgAmount, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerHurt, m_dmgDelay, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerHurt, m_bitsDamageInflict, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerHurt, m_playerDamageBits, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerHurt::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "dmg"))
	{
		m_dmgAmount = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "damagetype"))
	{
		m_bitsDamageInflict = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "dmgdelay"))
	{
		m_dmgDelay = SDL_atof(kv.value);
		return true;
	}
	else
		return CTriggerEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerHurt::Spawn( void )
{
	if(!CTriggerEntity::Spawn())
		return false;

	// Manage if start disabled
	if(!HasSpawnFlag(FL_START_OFF))
		m_isActive = true;
	else
		m_isActive = false;

	// Make sure this is valid
	if(m_dmgDelay <= 0)
		m_dmgDelay = DEFAULT_DMG_DELAY;

	// Set touch function
	SetTouch(&CTriggerHurt::HurtTouch);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerHurt::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
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
}

//=============================================
// @brief
//
//=============================================
void CTriggerHurt::HurtTouch( CBaseEntity* pOther )
{
	// Don't damage if not active
	if(!m_isActive)
		return;

	// Check for master
	if(!IsMasterTriggered())
		return;

	// Check take damage
	if(pOther->GetTakeDamage() == TAKEDAMAGE_NO)
		return;

	// Check if only allowed to touch clients or not allowed to touch clients
	if(HasSpawnFlag(FL_ONLY_CLIENTS) && !pOther->IsPlayer()
		|| HasSpawnFlag(FL_NO_CLIENTS) && pOther->IsPlayer())
		return;

	if(g_pGameVars->maxclients > 1)
	{
		if(m_nextDamageTime > g_pGameVars->time)
		{
			if(g_pGameVars->time != m_damageTime)
			{
				if(!pOther->IsPlayer())
					return;

				Int32 mask = 1<<pOther->GetClientIndex();
				if(m_playerDamageBits & mask)
					return;

				m_playerDamageBits |= mask;
			}
		}
		else
		{
			// Reset player bits
			m_playerDamageBits = 0;
			if(pOther->IsPlayer())
				m_playerDamageBits |= 1 << pOther->GetClientIndex();
		}
	}
	else
	{
		// SP is simpler
		if(m_nextDamageTime > g_pGameVars->time && g_pGameVars->time != m_damageTime)
			return;
	}

	Float dmgamount = m_dmgAmount * 0.5;
	if(dmgamount < 0)
		pOther->TakeHealth(-dmgamount, m_bitsDamageInflict);
	else
		pOther->TakeDamage(this, this, dmgamount, m_bitsDamageInflict);

	m_damageTime = g_pGameVars->time;
	m_nextDamageTime = g_pGameVars->time + m_dmgDelay;

	// Trigger targets if needed
	if(m_pFields->target != NO_STRING_VALUE)
	{
		// Check if only client touch should trigger
		if(HasSpawnFlag(FL_FIRE_ONLY_CLIENT) && !pOther->IsPlayer())
			return;

		UseTargets(pOther, USE_TOGGLE, 0);
		if(HasSpawnFlag(FL_TARGET_ONCE))
			m_pFields->target = NO_STRING_VALUE;
	}

	// Remove entity if flagged to do so
	if(HasSpawnFlag(FL_HURT_ONLY_ONCE) && pOther->IsPlayer())
	{
		SetTouch(nullptr);

		SetThink(&CBaseEntity::RemoveThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
}
