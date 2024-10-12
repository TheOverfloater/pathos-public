/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "light.h"
#include "lightstyles.h"

// Starting index for switchable lightstyles
const Uint32 CLight::SWITCHABLE_LIGHT_FIRST_STYLEINDEX = 32;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(light, CLight);

//=============================================
// @brief
//
//=============================================
CLight::CLight( edict_t* pedict ):
	CPointEntity(pedict),
	m_styleIndex(0),
	m_stylePattern(NO_STRING_VALUE),
	m_interpolatePattern(false),
	m_patternFramerate(0),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CLight::~CLight( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CLight::Spawn( void )
{
	if(m_styleIndex < SWITCHABLE_LIGHT_FIRST_STYLEINDEX)
	{
		Util::RemoveEntity(this);
		return true;
	}

	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE || !HasSpawnFlag(FL_LIGHT_START_OFF))
		m_isActive = true;
	else
		m_isActive = false;

	// Set current style value
	SetCurrentStyle();

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CLight::Restore( void )
{
	if(!CPointEntity::Restore())
		return false;

	// Set current style value
	SetCurrentStyle();

	return true;
}

//=============================================
// @brief
//
//=============================================
void CLight::DeclareSaveFields( void )
{
	// Call base class to handle it's own
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CLight, m_styleIndex, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CLight, m_stylePattern, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CLight, m_interpolatePattern, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CLight, m_patternFramerate, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CLight, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CLight::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "style"))
	{
		m_styleIndex = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "pattern"))
	{
		m_stylePattern = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "pitch"))
	{
		m_pState->angles.x = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "interp"))
	{
		m_interpolatePattern = (SDL_atoi(kv.value) == 1) ? true : false;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "framerate"))
	{
		m_patternFramerate = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CLight::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool desiredState = false;
	switch(useMode)
	{
	case USE_ON:
		desiredState = true;
		break;
	case USE_OFF:
		desiredState = false;
		break;
	case USE_TOGGLE:
		desiredState = m_isActive ? false : true;
		break;
	}

	if(m_isActive == desiredState)
		return;

	m_isActive = desiredState;
	SetCurrentStyle();
}

//=============================================
// @brief
//
//=============================================
void CLight::SetCurrentStyle( void )
{
	if(m_isActive)
	{
		if(m_stylePattern != NO_STRING_VALUE)
		{
			// Send the pattern itself
			const Char* pstrPattern = gd_engfuncs.pfnGetString(m_stylePattern);
			gSVLightStyles.SetLightStyle(pstrPattern, m_interpolatePattern, m_patternFramerate, m_styleIndex);
		}
		else
		{
			// Just set the style to fully lit
			gSVLightStyles.SetLightStyle("m", false, 0, m_styleIndex);
		}
	}
	else
	{
		// Set to completely dark
		gSVLightStyles.SetLightStyle("a", false, 0, m_styleIndex);
	}
}