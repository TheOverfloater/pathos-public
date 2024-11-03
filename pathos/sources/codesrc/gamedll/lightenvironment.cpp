/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "lightenvironment.h"

// TRUE if we have an ALD file present for the map
bool CLightEnvironment::g_isALDFilePresent = false;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(light_environment, CLightEnvironment);

//=============================================
// @brief
//
//=============================================
CLightEnvironment::CLightEnvironment( edict_t* pedict ):
	CPointEntity(pedict),
	m_sunlightIntensity(0),
	m_lightEnvMode(MODE_NORMAL)
{
}

//=============================================
// @brief
//
//=============================================
CLightEnvironment::~CLightEnvironment( void )
{
}

//=============================================
// @brief
//
//=============================================
void CLightEnvironment::DeclareSaveFields(void)
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CLightEnvironment, m_sunlightDirection, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CLightEnvironment, m_sunlightColor, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CLightEnvironment, m_sunlightIntensity, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CLightEnvironment, m_lightEnvMode, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CLightEnvironment::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "_fade"))
	{
		return true;
	}
	else if(!qstrcmp(kv.keyname, "_light"))
	{
		Int32 colorr = 0;
		Int32 colorg = 0;
		Int32 colorb = 0;
		Int32 intensity = 0;

		Uint32 num = SDL_sscanf(kv.value, "%d %d %d %d", &colorr, &colorg, &colorb, &intensity);
		if(num == 1)
		{
			m_sunlightColor.x = clamp(colorr, 0, 255);
			m_sunlightColor.y = m_sunlightColor.x;
			m_sunlightColor.z = m_sunlightColor.y;
			m_sunlightIntensity = 0;
		}
		else
		{
			m_sunlightColor.x = clamp(colorr, 0, 255);
			m_sunlightColor.y = clamp(colorg, 0, 255);
			m_sunlightColor.z = clamp(colorb, 0, 255);
			m_sunlightIntensity = clamp(intensity, 0, 255);
		}
		return true;
	}
	else if (!qstrcmp(kv.keyname, "nightmode"))
	{
		if (SDL_atoi(kv.value) == 1)
		{
			if (m_lightEnvMode == MODE_NIGHT)
				m_lightEnvMode = MODE_DAYLIGHT_RETURN_AND_NIGHT;
			else
				m_lightEnvMode = MODE_NIGHT;
		}
		return true;
	}
	else if (!qstrcmp(kv.keyname, "daylightreturn"))
	{
		if (SDL_atoi(kv.value) == 1)
		{
			if (m_lightEnvMode == MODE_DAYLIGHT_RETURN)
				m_lightEnvMode = MODE_DAYLIGHT_RETURN_AND_NIGHT;
			else
				m_lightEnvMode = MODE_DAYLIGHT_RETURN;
		}
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CLightEnvironment::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	// Calculate final light values
	if(m_sunlightIntensity)
	{
		m_sunlightColor.x = ((Float)m_sunlightIntensity/255.0f) * m_sunlightColor.x;
		m_sunlightColor.y = ((Float)m_sunlightIntensity/255.0f) * m_sunlightColor.y;
		m_sunlightColor.z = ((Float)m_sunlightIntensity/255.0f) * m_sunlightColor.z;
	}

	// Flag entity for removal
	return true;
}

//=============================================
// @brief
//
//=============================================
void CLightEnvironment::SendInitMessage( const CBaseEntity* pPlayer )
{
	// Either use activator, or assume it's local player
	const CBaseEntity* pEntity;
	if (pPlayer)
		pEntity = pPlayer;
	else
		pEntity = Util::GetHostPlayer();

	if (!pEntity || !pEntity->IsPlayer())
	{
		Util::EntityConPrintf(m_pEdict, "Not a player entity.\n");
		return;
	}
	
	daystage_t dayStage = pEntity->GetDayStage();
	SetLightEnvValues(dayStage);
}

//=============================================
// @brief
//
//=============================================
bool CLightEnvironment::SetLightEnvValues( daystage_t daystage )
{
	if (g_isALDFilePresent)
	{
		switch (daystage)
		{
		case DAYSTAGE_NIGHTSTAGE:
		{
			if (m_lightEnvMode != MODE_NIGHT
				&& m_lightEnvMode != MODE_DAYLIGHT_RETURN_AND_NIGHT)
				return false;
		}
		break;
		case DAYSTAGE_DAYLIGHT_RETURN:
		{
			if (m_lightEnvMode != MODE_DAYLIGHT_RETURN
				&& m_lightEnvMode != MODE_DAYLIGHT_RETURN_AND_NIGHT)
				return false;
		}
		break;
		default:
		case DAYSTAGE_NORMAL:
		{
			if (m_lightEnvMode != MODE_NORMAL)
				return false;
		}
		break;
		}
	}
	else if (m_lightEnvMode != MODE_NORMAL)
	{
		// If no ALD is present, ignore anything but normal light_env
		return false;
	}

	// Modulate the color like RAD does with gamma adjustments
	Uint32 color_r = pow((Float)m_sunlightColor.x / 114.0, 0.6) * 264;
	Uint32 color_g = pow((Float)m_sunlightColor.y / 114.0, 0.6) * 264;
	Uint32 color_b = pow((Float)m_sunlightColor.z / 114.0, 0.6) * 264;

	gd_engfuncs.pfnSetCVarFloat("sv_skycolor_r", (Float)color_r);
	gd_engfuncs.pfnSetCVarFloat("sv_skycolor_g", (Float)color_g);
	gd_engfuncs.pfnSetCVarFloat("sv_skycolor_b", (Float)color_b);

	Vector forward;
	Math::AngleVectors(m_pState->angles, &forward, nullptr, nullptr);

	gd_engfuncs.pfnSetCVarFloat("sv_skyvec_x", forward.x);
	gd_engfuncs.pfnSetCVarFloat("sv_skyvec_y", forward.y);
	gd_engfuncs.pfnSetCVarFloat("sv_skyvec_z", forward.z);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CLightEnvironment::CheckALDFile( void )
{
	// Reset to default
	g_isALDFilePresent = false;

	const cache_model_t* pWorld = gd_engfuncs.pfnGetModel(1);
	if (!pWorld)
		return;

	CString aldPath = pWorld->name;
	Uint32 dotPos = aldPath.find(0, ".");
	if (dotPos == CString::CSTRING_NO_POSITION)
		return;

	Uint32 eraseCnt = aldPath.length() - dotPos;
	aldPath.erase(dotPos, eraseCnt);
	aldPath << ".ald";

	if (gd_filefuncs.pfnFileExists(aldPath.c_str()))
		g_isALDFilePresent = true;
}