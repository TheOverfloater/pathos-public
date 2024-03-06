/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envbeamfx.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_beamfx, CEnvBeamFX);

//=============================================
// @brief
//
//=============================================
CEnvBeamFX::CEnvBeamFX( edict_t* pedict ):
	CPointEntity(pedict),
	m_beamType(FX_BEAMDISK),
	m_spriteModelName(NO_STRING_VALUE),
	m_beamRadius(0),
	m_startFrame(0),
	m_life(0),
	m_width(0),
	m_beamNoise(0),
	m_noiseSpeed(0),
	m_beamSpeed(0),
	m_repeatMaxDelay(0),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CEnvBeamFX::~CEnvBeamFX( void )
{
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CEnvBeamFX::Spawn( void )
{
	// Check for model
	if(m_spriteModelName == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!m_noiseSpeed)
		m_noiseSpeed = 1.0;

	if(!CPointEntity::Spawn())
		return false;

	if(m_repeatMaxDelay)
	{
		if(m_repeatMinDelay > m_repeatMaxDelay)
		{
			Util::EntityConPrintf(m_pEdict, "Min repeat rate greather than max repeat rate.\n");
			Util::RemoveEntity(m_pEdict);
			return false;
		}

		if(m_pFields->targetname == NO_STRING_VALUE || HasSpawnFlag(FL_START_ON))
		{
			SetThink(&CEnvBeamFX::RepeatThink);
			m_pState->nextthink = g_pGameVars->time + 0.1;
			m_isActive = true;
		}
	}

	return true;
}

//=============================================
// @brief Performs precache functions
//
//=============================================
void CEnvBeamFX::Precache( void )
{
	CPointEntity::Precache();

	if(m_spriteModelName != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_spriteModelName));
}

//=============================================
// @brief Calls for classes and their children
//
//=============================================
void CEnvBeamFX::DeclareSaveFields( void )
{
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFX, m_beamType, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFX, m_spriteModelName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFX, m_beamRadius, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFX, m_startFrame, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFX, m_life, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFX, m_width, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFX, m_beamNoise, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFX, m_noiseSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFX, m_beamSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFX, m_repeatMinDelay, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFX, m_repeatMaxDelay, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFX, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief Manages keyvalues
//
//=============================================
bool CEnvBeamFX::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "type"))
	{
		m_beamType = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "radius"))
	{
		m_beamRadius = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "startframe"))
	{
		m_startFrame = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "life"))
	{
		m_life = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "width"))
	{
		m_width = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "beamnoise"))
	{
		m_beamNoise = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "noisespeed"))
	{
		m_noiseSpeed = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "beamspeed"))
	{
		m_beamSpeed = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "texture"))
	{
		m_spriteModelName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "mindelay"))
	{
		m_repeatMinDelay = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "maxdelay"))
	{
		m_repeatMaxDelay = SDL_atof(kv.value);
		return true;
	}
	else 
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief Calls use function
//
//=============================================
void CEnvBeamFX::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!m_repeatMaxDelay)
	{
		CreateEffect();
		return;
	}
	else
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
			SetThink(&CEnvBeamFX::RepeatThink);
		}
		else
		{
			m_pState->nextthink = 0;
			SetThink(nullptr);
		}
	}
}

//=============================================
// @brief Calls use function
//
//=============================================
void CEnvBeamFX::RepeatThink( void )
{
	CreateEffect();

	m_pState->nextthink = g_pGameVars->time + Common::RandomFloat(m_repeatMinDelay, m_repeatMaxDelay);
	SetThink(&CEnvBeamFX::RepeatThink);
}

//=============================================
// @brief Spawns the effect
//
//=============================================
void CEnvBeamFX::CreateEffect( void )
{
	Vector endPosition = m_pState->origin;

	Float realDuration = m_life * 0.1;
	endPosition.z += m_beamRadius * (1 / realDuration);

	const Char* pstrModelName = gd_engfuncs.pfnGetString(m_spriteModelName);
	if(!pstrModelName)
		return;

	switch(m_beamType)
	{
	case FX_BEAMDISK:
		Util::CreateBeamDisk(m_pState->origin, endPosition, pstrModelName, m_startFrame, m_pState->framerate,
			m_life, m_width, m_beamNoise, m_pState->renderamt, m_beamSpeed, m_noiseSpeed, m_pState->rendercolor.x,
			m_pState->rendercolor.y, m_pState->rendercolor.z);
		break;
	case FX_BEAMTORUS:
		Util::CreateBeamTorus(m_pState->origin, endPosition, pstrModelName, m_startFrame, m_pState->framerate,
			m_life, m_width, m_beamNoise, m_pState->renderamt, m_beamSpeed, m_noiseSpeed, m_pState->rendercolor.x,
			m_pState->rendercolor.y, m_pState->rendercolor.z);
		break;
	case FX_BEAMCYLINDER:
		Util::CreateBeamCylinder(m_pState->origin, endPosition, pstrModelName, m_startFrame, m_pState->framerate,
			m_life, m_width, m_beamNoise, m_pState->renderamt, m_beamSpeed, m_noiseSpeed, m_pState->rendercolor.x,
			m_pState->rendercolor.y, m_pState->rendercolor.z);
		break;
	}
}