/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envtracereffect.h"

// Default tracer color
const Vector CEnvTracerEffect::DEFAULT_COLOR = Vector(204, 204, 102);

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_tracereffect, CEnvTracerEffect);

//=============================================
// @brief
//
//=============================================
CEnvTracerEffect::CEnvTracerEffect( edict_t* pedict ):
	CPointEntity(pedict),
	m_type(EFFECT_TRACER),
	m_tracerCount(0),
	m_minRepeatDelay(0),
	m_maxRepeatDelay(0),
	m_minVelocity(0),
	m_maxVelocity(0),
	m_tracerLength(0),
	m_tracerWidth(0),
	m_minLifetime(0),
	m_maxLifetime(0),
	m_tracerGravityType(TRACER_GRAVITY_NONE),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CEnvTracerEffect::~CEnvTracerEffect( void )
{
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CEnvTracerEffect::Spawn( void )
{
	if(m_type < 0 || m_type >= NB_EFFECT_TYPES)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid particle effect type %d specified.\n", m_type);
		Util::RemoveEntity(m_pEdict);
		return false;
	}

	if(m_type == EFFECT_SPARKSTREAK || m_type == EFFECT_STREAKSPLASH)
	{
		if(m_minVelocity > m_maxVelocity)
		{
			Util::EntityConPrintf(m_pEdict, "Min velocity greather than max velocity.\n");
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
			SetThink(&CEnvTracerEffect::RepeatThink);
			m_isActive = true;
		}
	}

	if(m_pState->speed <= 0)
		m_pState->speed = 1.0;

	if(m_tracerLength <= 0)
		m_tracerLength = 1.5;

	if(m_tracerWidth <= 0)
		m_tracerWidth = 0.8;
	
	if(m_type != EFFECT_TRACER || m_pFields->target == NO_STRING_VALUE)
	{
		if(m_minLifetime <= 0)
			m_minLifetime = 1.5;
	}

	if(m_pState->renderamt <= 0)
		m_pState->renderamt = 255;

	if(m_pState->rendercolor.IsZero())
		m_pState->rendercolor = DEFAULT_COLOR;

	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief Calls for classes and their children
//
//=============================================
void CEnvTracerEffect::DeclareSaveFields( void )
{
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvTracerEffect, m_type, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvTracerEffect, m_minRepeatDelay, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvTracerEffect, m_maxRepeatDelay, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvTracerEffect, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvTracerEffect, m_minVelocity, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvTracerEffect, m_maxVelocity, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvTracerEffect, m_tracerLength, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvTracerEffect, m_tracerWidth, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvTracerEffect, m_minLifetime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvTracerEffect, m_maxLifetime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvTracerEffect, m_tracerGravityType, EFIELD_UINT32));
}

//=============================================
// @brief Manages keyvalues
//
//=============================================
bool CEnvTracerEffect::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "type"))
	{
		m_type = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "count"))
	{
		m_tracerCount = SDL_atoi(kv.value);
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
	else if(!qstrcmp(kv.keyname, "minvelocity"))
	{
		m_minVelocity = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "maxvelocity"))
	{
		m_maxVelocity = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "tracerlength"))
	{
		m_tracerLength = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "tracerwidth"))
	{
		m_tracerWidth = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "minlife"))
	{
		m_minLifetime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "maxlife"))
	{
		m_maxLifetime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "gravtype"))
	{
		Int32 type = SDL_atoi(kv.value);
		switch(type)
		{
		case 0:
			m_tracerGravityType = TRACER_GRAVITY_NONE;
			break;
		case 1:
			m_tracerGravityType = TRACER_GRAVITY_NORMAL;
			break;
		case 2:
			m_tracerGravityType = TRACER_GRAVITY_SLOW;
			break;
		default:
			m_tracerGravityType = TRACER_GRAVITY_NONE;
			Util::EntityConPrintf(m_pEdict, "Unknown value %d specified for '%s'.\n", type, kv.keyname);
			break;
		}

		return true;
	}
	else 
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief Calls use function
//
//=============================================
void CEnvTracerEffect::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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
			SetThink(&CEnvTracerEffect::RepeatThink);
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
void CEnvTracerEffect::RepeatThink( void )
{
	CreateEffect();

	m_pState->nextthink = g_pGameVars->time + Common::RandomFloat(m_minRepeatDelay, m_maxRepeatDelay);
	SetThink(&CEnvTracerEffect::RepeatThink);
}

//=============================================
// @brief Get direction
//
//=============================================
Vector CEnvTracerEffect::GetDirection( Float* plength )
{
	if(m_pFields->target == NO_STRING_VALUE)
	{
		Vector forward;
		Math::AngleVectors(m_pState->angles, &forward);
		if(plength) (*plength) = 1;
		return forward;
	}
	else
	{
		const Char* pstrTarget = gd_engfuncs.pfnGetString(m_pFields->target);
		edict_t* pEdict = Util::FindEntityByTargetName(nullptr, pstrTarget);
		if(!pEdict)
		{
			Util::EntityConPrintf(m_pEdict, "Target '%s' not found.\n", pstrTarget);
			if(plength) (*plength) = 1;
			return Vector(0, 0, 1);
		}
		
		Vector vectotarget = (pEdict->state.origin - m_pState->origin);
		if(plength) (*plength) = vectotarget.Length();
		return vectotarget.Normalize();
	}
}

//=============================================
// @brief Spawns the desired effect
//
//=============================================
void CEnvTracerEffect::CreateEffect( void )
{
	switch(m_type)
	{
	case EFFECT_TRACER:
		{
			tracer_type_t gravType;
			switch(m_tracerGravityType)
			{
			case TRACER_GRAVITY_NORMAL:
				gravType = TRACER_GRAVITY;
				break;
			case TRACER_GRAVITY_SLOW:
				gravType = TRACER_SLOW_GRAVITY;
				break;
			case TRACER_GRAVITY_NONE:
			default:
				gravType = TRACER_NORMAL;
				break;
			}

			Float length;
			Vector forward = GetDirection(&length);
			for(Uint32 i = 0; i < m_tracerCount; i++)
			{
				Float speed;
				if(m_maxVelocity > 0 && m_minVelocity < m_maxVelocity)
					speed = Common::RandomFloat(m_minVelocity, m_maxVelocity);
				else
					speed = m_pState->speed;

				Vector velocity = forward * speed;

				Float life;
				if(m_minLifetime <= 0 && m_maxLifetime <= 0)
				{
					// If minlife is zero, then life comes from distance travelled
					life = length / speed;
				}
				else
				{
					// Depends on whether there's a valid max life value
					if(m_maxLifetime > m_minLifetime)
						life = Common::RandomFloat(m_minLifetime, m_maxLifetime);
					else
						life = m_minLifetime;
				}

				Util::CreateTracer(m_pState->origin, velocity, m_pState->rendercolor, m_pState->renderamt, m_tracerWidth, m_tracerLength, life, gravType);
			}
		}
		break;
	case EFFECT_SPARKSTREAK:
		{
			Util::CreateSparkStreak(m_pState->origin, m_tracerCount, m_pState->rendercolor, m_pState->renderamt, m_tracerWidth, m_tracerLength, m_minLifetime, m_maxLifetime, m_minVelocity, m_maxVelocity);
		}
		break;
	case EFFECT_STREAKSPLASH:
		{
			Vector forward = GetDirection(nullptr);
			Util::CreateStreakSplash(m_pState->origin, forward, m_pState->rendercolor, m_pState->renderamt, m_tracerWidth, m_tracerLength, m_tracerCount, m_pState->speed, m_minLifetime, m_maxLifetime, m_minVelocity, m_maxVelocity);
		}
		break;
	}
}