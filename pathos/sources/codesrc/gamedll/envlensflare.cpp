/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envlensflare.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_lensflare, CEnvLensFlare);

//=============================================
// @brief
//
//=============================================
CEnvLensFlare::CEnvLensFlare( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvLensFlare::~CEnvLensFlare( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvLensFlare::Precache( void )
{
	CBaseEntity::Precache();

	gd_engfuncs.pfnPrecacheModel(NULL_SPRITE_FILENAME);
}

//=============================================
// @brief
//
//=============================================
bool CEnvLensFlare::Spawn( void )
{
	// Set modelname to null sprite model
	m_pFields->modelname = gd_engfuncs.pfnAllocString(NULL_SPRITE_FILENAME);

	if(!CBaseEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname, false))
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE || HasSpawnFlag(FL_START_ON))
		m_pState->effects &= ~EF_NODRAW;
	else
		m_pState->effects |= EF_NODRAW;

	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->rendertype = RT_LENSFLARE;
	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvLensFlare::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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