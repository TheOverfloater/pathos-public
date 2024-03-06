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

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(light_environment, CLightEnvironment);

//=============================================
// @brief
//
//=============================================
CLightEnvironment::CLightEnvironment( edict_t* pedict ):
	CPointEntity(pedict),
	m_sunlightIntensity(0)
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
			m_sunlightColor.r = clamp(colorr, 0, 255);
			m_sunlightColor.g = m_sunlightColor.r;
			m_sunlightColor.b = m_sunlightColor.r;
			m_sunlightIntensity = 0;
		}
		else
		{
			m_sunlightColor.r = clamp(colorr, 0, 255);
			m_sunlightColor.g = clamp(colorg, 0, 255);
			m_sunlightColor.b = clamp(colorb, 0, 255);
			m_sunlightIntensity = clamp(intensity, 0, 255);
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
		m_sunlightColor.r = ((Float)m_sunlightIntensity/255.0f) * m_sunlightColor.r;
		m_sunlightColor.g = ((Float)m_sunlightIntensity/255.0f) * m_sunlightColor.g;
		m_sunlightColor.b = ((Float)m_sunlightIntensity/255.0f) * m_sunlightColor.b;
	}
	
	// Modulate the color like RAD does with gamma adjustments
	Uint32 color_r = pow((Float)m_sunlightColor.r / 114.0, 0.6) * 264;
	Uint32 color_g = pow((Float)m_sunlightColor.g / 114.0, 0.6) * 264;
	Uint32 color_b = pow((Float)m_sunlightColor.b / 114.0, 0.6) * 264;

	gd_engfuncs.pfnSetCVarFloat("sv_skycolor_r", (Float)color_r);
	gd_engfuncs.pfnSetCVarFloat("sv_skycolor_g", (Float)color_g);
	gd_engfuncs.pfnSetCVarFloat("sv_skycolor_b", (Float)color_b);

	Vector forward;
	Math::AngleVectors(m_pState->angles, &forward, nullptr, nullptr);

	gd_engfuncs.pfnSetCVarFloat("sv_skyvec_x", forward.x);
	gd_engfuncs.pfnSetCVarFloat("sv_skyvec_y", forward.y);
	gd_engfuncs.pfnSetCVarFloat("sv_skyvec_z", forward.z);

	// Flag entity for removal
	Util::RemoveEntity(this);
	return true;
}
