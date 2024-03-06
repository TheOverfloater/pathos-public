/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envelight.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_elight, CEnvELight);

//=============================================
// @brief
//
//=============================================
CEnvELight::CEnvELight( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvELight::~CEnvELight( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CEnvELight::Spawn( void )
{
	// If it has no targetname, then it'll be managed
	// by the clientside entity manager
	if(m_pFields->targetname == NO_STRING_VALUE)
	{
		Util::RemoveEntity(this);
		return true;
	}

	if(!CPointEntity::Spawn())
		return false;

	// Set empty sprite texture
	if(!SetModel(NULL_SPRITE_FILENAME, false))
		return false;

	if(HasSpawnFlag(FL_START_ON))
		m_pState->effects &= ~EF_NODRAW;
	else
		m_pState->effects |= EF_NODRAW;

	// Mark as an env_elight
	m_pState->rendertype = RT_ENVELIGHT;

	// Set mins/maxs
	Vector mins, maxs;
	for(Uint32 i = 0; i < 3; i++)
	{
		mins[i] = -m_pState->renderamt*ENV_ELIGHT_RADIUS_MULTIPLIER;
		maxs[i] = m_pState->renderamt*ENV_ELIGHT_RADIUS_MULTIPLIER;
	}

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, mins, maxs);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvELight::Precache( void )
{
	gd_engfuncs.pfnPrecacheModel(NULL_SPRITE_FILENAME);
}

//=============================================
// @brief
//
//=============================================
void CEnvELight::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	switch(useMode)
	{
	case USE_OFF:
		m_pState->effects |= EF_NODRAW;
		break;
	case USE_ON:
		m_pState->effects &= ~EF_NODRAW;
		break;
	case USE_TOGGLE:
	default:
		if(m_pState->effects & EF_NODRAW)
			m_pState->effects &= ~EF_NODRAW;
		else
			m_pState->effects |= EF_NODRAW;
		break;
	}
}
