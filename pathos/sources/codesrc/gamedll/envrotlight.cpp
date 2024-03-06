/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envrotlight.h"

// Rotating light model
const Char CEnvRotLight::ENV_ROTLIGHT_MODEL_FILENAME[] = "models/props/emergency_light.mdl";

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_rot_light, CEnvRotLight);

//=============================================
// @brief
//
//=============================================
CEnvRotLight::CEnvRotLight( edict_t* pedict ):
	CAnimatingEntity(pedict),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CEnvRotLight::~CEnvRotLight( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvRotLight::DeclareSaveFields( void )
{
	// Call base class to do it first
	CAnimatingEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvRotLight, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
void CEnvRotLight::Precache( void )
{
	CAnimatingEntity::Precache();

	gd_engfuncs.pfnPrecacheModel("sprites/glow03.spr");
	gd_engfuncs.pfnPrecacheModel("sprites/emerg_flare.spr");
}

//=============================================
// @brief
//
//=============================================
bool CEnvRotLight::Spawn( void )
{
	m_pFields->modelname = gd_engfuncs.pfnAllocString(ENV_ROTLIGHT_MODEL_FILENAME);

	if(!CAnimatingEntity::Spawn())
		return false;

	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_NONE;

	ResetSequenceInfo();

	if(HasSpawnFlag(FL_START_ON) || m_pFields->targetname == NO_STRING_VALUE)
	{
		m_isActive = true;

		if(!HasSpawnFlag(FL_NO_SHADOWS))
			m_pState->renderfx = RenderFx_Rotlight;
		else
			m_pState->renderfx = RenderFx_RotlightNS;
	}
	else
	{
		m_pState->framerate = 0.0;
		m_isActive = false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvRotLight::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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

	if(m_isActive)
	{
		ResetSequenceInfo();

		if(!HasSpawnFlag(FL_NO_SHADOWS))
			m_pState->renderfx = RenderFx_Rotlight;
		else
			m_pState->renderfx = RenderFx_RotlightNS;
	}
	else
	{
		m_pState->framerate = 0;
		m_pState->renderfx = 0;
	}
}