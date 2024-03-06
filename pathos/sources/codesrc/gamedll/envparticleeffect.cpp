/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envparticleeffect.h"

// Max colors that can be addressed
const Uint32 CEnvParticleEffect::MAX_COLORS = 255;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_particleeffect, CEnvParticleEffect);

//=============================================
// @brief
//
//=============================================
CEnvParticleEffect::CEnvParticleEffect( edict_t* pedict ):
	CPointEntity(pedict),
	m_type(EFFECT_PARTICLEEXPLOSION1),
	m_startColor(0),
	m_endColor(0),
	m_particleCount(0),
	m_trailType(0),
	m_minRepeatDelay(0),
	m_maxRepeatDelay(0),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CEnvParticleEffect::~CEnvParticleEffect( void )
{
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CEnvParticleEffect::Spawn( void )
{
	if(m_startColor < 0 || m_startColor > MAX_COLORS)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid start color index %d specified.\n", m_startColor);
		if(m_startColor < 0)
			m_startColor = 0;
		else if(m_startColor > MAX_COLORS)
			m_startColor = MAX_COLORS;
	}
	
	if(m_endColor < 0 || m_endColor > MAX_COLORS)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid end color index %d specified.\n", m_endColor);
		if(m_endColor < 0)
			m_endColor = 0;
		else if(m_endColor > MAX_COLORS)
			m_endColor = MAX_COLORS;
	}

	if(m_type < 0 || m_type >= NB_EFFECT_TYPES)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid particle effect type %d specified.\n", m_type);
		Util::RemoveEntity(m_pEdict);
		return false;
	}

	if(m_type == EFFECT_PARTICLEEFFECT)
	{
		if(m_particleCount <= 0)
		{
			Util::EntityConPrintf(m_pEdict, "Invalid particle count specified.\n");
			Util::RemoveEntity(m_pEdict);
			return false;
		}
	}
	else if(m_type == EFFECT_ROCKETTRAIL)
	{
		if(m_trailType < 0 || m_trailType > nb_trail_types)
		{
			Util::EntityConPrintf(m_pEdict, "Invalid rocket trail type %d specified.\n", m_trailType);
			Util::RemoveEntity(m_pEdict);
			return false;
		}

		if(m_pFields->target == NO_STRING_VALUE)
		{
			Util::EntityConPrintf(m_pEdict, "No target specified for rocket trail.\n");
			Util::RemoveEntity(m_pEdict);
			return false;
		}
	}

	if(m_maxRepeatDelay)
	{
		if(m_minRepeatDelay > m_maxRepeatDelay)
		{
			Util::EntityConPrintf(m_pEdict, "Min repeat rate greather than max repeat rate.\n");
			Util::RemoveEntity(m_pEdict);
			return false;
		}

		if(m_pFields->targetname == NO_STRING_VALUE || HasSpawnFlag(FL_START_ON))
		{
			m_pState->nextthink = g_pGameVars->time + 0.1;
			SetThink(&CEnvParticleEffect::RepeatThink);
			m_isActive = true;
		}
	}

	if(m_pState->speed <= 0)
		m_pState->speed = 1.0;

	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief Calls for classes and their children
//
//=============================================
void CEnvParticleEffect::DeclareSaveFields( void )
{
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvParticleEffect, m_type, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvParticleEffect, m_startColor, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvParticleEffect, m_endColor, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvParticleEffect, m_particleCount, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvParticleEffect, m_trailType, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvParticleEffect, m_minRepeatDelay, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvParticleEffect, m_maxRepeatDelay, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvParticleEffect, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief Manages keyvalues
//
//=============================================
bool CEnvParticleEffect::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "type"))
	{
		m_type = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "startcolor"))
	{
		m_startColor = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "endcolor"))
	{
		m_endColor = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "count"))
	{
		m_particleCount = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "trailtype"))
	{
		m_trailType = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "mindelay"))
	{
		m_minRepeatDelay = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "maxdelay"))
	{
		m_maxRepeatDelay = SDL_atof(kv.value);
		return true;
	}
	else 
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief Calls use function
//
//=============================================
void CEnvParticleEffect::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(m_maxRepeatDelay > 0)
	{
		bool desiredState = false;
		switch(useMode)
		{
		case USE_OFF:
			desiredState = false;
			break;
		case USE_ON:
			desiredState = true;
			break;
		case USE_TOGGLE:
		default:
			if(m_isActive)
				desiredState = false;
			else
				desiredState = true;
			break;
		}

		if(desiredState == m_isActive)
			return;

		m_isActive = desiredState;

		if(m_isActive)
		{
			m_pState->nextthink = g_pGameVars->time + 0.1;
			SetThink(&CEnvParticleEffect::RepeatThink);
		}
		else
		{
			m_pState->nextthink = 0;
			SetThink(nullptr);
		}
	}
	else
	{
		// Single-use only
		CreateEffect();
	}
}

//=============================================
// @brief Repeat function
//
//=============================================
void CEnvParticleEffect::RepeatThink( void )
{
	CreateEffect();

	m_pState->nextthink = g_pGameVars->time + Common::RandomFloat(m_minRepeatDelay, m_maxRepeatDelay);
	SetThink(&CEnvParticleEffect::RepeatThink);
}
 
//=============================================
// @brief Spawns the desired effect
//
//=============================================
void CEnvParticleEffect::CreateEffect( void )
{
	switch(m_type)
	{
	case EFFECT_PARTICLEEXPLOSION1:
		{
			Util::CreateParticleExplosion1(m_pState->origin);
		}
		break;
	case EFFECT_PARTICLEEXPLOSION2:
		{
			Util::CreateParticleExplosion2(m_pState->origin, clamp(m_startColor, 0, 255), clamp(m_endColor, 0, 255));
		}
		break;
	case EFFECT_BLOBEXPLOSION:
		{
			Util::CreateBlobExplosion(m_pState->origin);
		}
		break;
	case EFFECT_ROCKETEXPLOSION:
		{
			Util::CreateRocketExplosion(m_pState->origin, clamp(m_startColor, 0, 255));
		}
		break;
	case EFFECT_PARTICLEEFFECT:
		{
			Vector forward;
			Math::AngleVectors(m_pState->angles, &forward);

			Util::CreateParticleEffect(m_pState->origin, forward*m_pState->speed, clamp(m_startColor, 0, 255), m_particleCount);
		}
		break;
	case EFFECT_LAVASPLASH:
		{
			Util::CreateLavaSplash(m_pState->origin);
		}
		break;
	case EFFECT_TELEPORTSPLASH:
		{
			Util::CreateTeleportSplash(m_pState->origin);
		}
		break;
	case EFFECT_ROCKETTRAIL:
		{
			if(m_pFields->target == NO_STRING_VALUE)
			{
				Util::EntityConPrintf(m_pEdict, "No target specified for rocket trail.\n");
				return;
			}

			const Char* pstrTargetName = gd_engfuncs.pfnGetString(m_pFields->target);
			edict_t* pEdict = Util::FindEntityByTargetName(nullptr, pstrTargetName);
			if(!pEdict)
			{
				Util::EntityConPrintf(m_pEdict, "Target entity '%s' could not be found.\n", pstrTargetName);
				return;
			}

			Util::CreateRocketTrail(m_pState->origin, pEdict->state.origin, m_trailType);
		}
		break;
	}
}