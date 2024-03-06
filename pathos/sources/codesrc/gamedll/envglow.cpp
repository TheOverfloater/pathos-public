/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envglow.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_glow, CEnvGlow);

//=============================================
// @brief
//
//=============================================
CEnvGlow::CEnvGlow( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvGlow::~CEnvGlow( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvGlow::Precache( void )
{
	if(m_pFields->modelname != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_pFields->modelname));
}

//=============================================
// @brief
//
//=============================================
bool CEnvGlow::Spawn( void )
{
	// Remove immediately if it has no targetname
	// Means it's completely static
	if(m_pFields->targetname == NO_STRING_VALUE)
	{
		Util::RemoveEntity(this);
		return true;
	}

	// Check for model
	if(m_pFields->modelname == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CBaseEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname, false))
		return false;

	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_NONE;

	return true;
}
