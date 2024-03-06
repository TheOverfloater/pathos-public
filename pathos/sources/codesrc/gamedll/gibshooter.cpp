/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "gibshooter.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(gibshooter, CGibShooter);

//=============================================
// @brief
//
//=============================================
CGibShooter::CGibShooter( edict_t* pedict ):
	CPointEntity(pedict),
	m_numGibs(0),
	m_gibCapacity(0),
	m_gibMaterial(0),
	m_gibModelIndex(NO_PRECACHE),
	m_nbBodyVariations(0),
	m_gibVelocity(0),
	m_variance(0),
	m_gibLifetime(0),
	m_delay(0)
{
}

//=============================================
// @brief
//
//=============================================
CGibShooter::~CGibShooter( void )
{
}

//=============================================
// @brief
//
//=============================================
void CGibShooter::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CGibShooter, m_numGibs, EFIELD_UINT32));	
	DeclareSaveField(DEFINE_DATA_FIELD(CGibShooter, m_gibCapacity, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CGibShooter, m_gibMaterial, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CGibShooter, m_gibVelocity, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CGibShooter, m_variance, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CGibShooter, m_gibLifetime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CGibShooter, m_delay, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CGibShooter::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "m_iGibs"))
	{
		m_numGibs = m_gibCapacity = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "m_flVelocity"))
	{
		m_gibVelocity = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "m_flVariance"))
	{
		m_variance = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "m_flGibLife"))
	{
		m_gibLifetime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "delay"))
	{
		m_delay = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CGibShooter::Precache( void )
{
	CPointEntity::Precache();

	m_gibModelIndex = gd_engfuncs.pfnPrecacheModel(HUMAN_GIBS_MODEL_FILENAME);
	
	if(m_gibModelIndex != NO_PRECACHE)
		m_nbBodyVariations = gd_engfuncs.pfnGetModelFrameCount(m_gibModelIndex);

	if(m_gibMaterial != MAT_NONE)
		Util::PrecacheDebrisSounds((breakmaterials_t)m_gibMaterial);
}

//=============================================
// @brief
//
//=============================================
bool CGibShooter::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(!m_delay)
		m_delay = 0.1;

	if(!m_gibLifetime)
		m_gibLifetime = CGib::GIB_DEFAULT_LIFETIME;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CGibShooter::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	SetThink(&CGibShooter::ShootThink);
	m_pState->nextthink = g_pGameVars->time + 0.1;
}

//=============================================
// @brief
//
//=============================================
CGib* CGibShooter::CreateGib( void )
{
	CGib* pGib = reinterpret_cast<CGib*>(CBaseEntity::CreateEntity("gib", nullptr));
	if(!pGib)
		return nullptr;

	if(!pGib->InitGib(HUMAN_GIBS_MODEL_FILENAME))
	{
		Util::RemoveEntity(pGib);
		return nullptr;
	}

	pGib->SetBloodColor(BLOOD_RED);

	if(m_nbBodyVariations > CGib::GIB_SKULL)
	{
		Uint32 randomGibsBegin = CGib::GIB_SKULL+1;
		pGib->SetBody(Common::RandomLong(randomGibsBegin, CGib::NB_GIBS-randomGibsBegin));
	}

	return pGib;
}

//=============================================
// @brief
//
//=============================================
void CGibShooter::ShootThink( void )
{
	Vector forward, right, up;
	Math::AngleVectors(m_pState->angles, &forward, &right, &up);

	Vector shootDirection = forward + right * Common::RandomFloat(-1, 1) * m_variance;
	shootDirection = shootDirection + up * Common::RandomFloat(-1, 1) * m_variance;

	shootDirection.Normalize();
	CGib* pGib = CreateGib();
	if(pGib)
	{
		pGib->SetOrigin(m_pState->origin);
		pGib->SetVelocity(shootDirection * m_gibVelocity);

		Vector avelocity;
		avelocity[0] = Common::RandomFloat(-200, 200);
		avelocity[1] = Common::RandomFloat(-200, 200);
		pGib->SetAngularVelocity(avelocity);

		// Set think time
		Double thinktime = pGib->GetNextThinkTime() - g_pGameVars->time;
		Double lifetime = m_gibLifetime * Common::RandomFloat(0.9, 1.1);
		if(lifetime > thinktime)
		{
			pGib->SetNextThink(lifetime);
			lifetime = 0;
		}

		pGib->SetLifeTime(lifetime);
	}

	// Avoid going over
	if(m_numGibs > 0)
		m_numGibs--;

	if(!m_numGibs)
	{
		if(HasSpawnFlag(FL_REPEATABLE))
		{
			m_numGibs = m_gibCapacity;
			ClearThinkFunctions();
		}
		else
		{
			SetThink(&CBaseEntity::RemoveThink);
			m_pState->nextthink = g_pGameVars->time + 0.1;
		}
	}
	else
	{
		// Spawn again after delay
		m_pState->nextthink = g_pGameVars->time + m_delay;
	}
}