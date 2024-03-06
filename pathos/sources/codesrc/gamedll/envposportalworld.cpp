/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envposportalworld.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(envpos_portal_world, CEnvPosPortalWorld);

//=============================================
// @brief
//
//=============================================
CEnvPosPortalWorld::CEnvPosPortalWorld( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvPosPortalWorld::~CEnvPosPortalWorld( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvPosPortalWorld::Precache( void )
{
	CPointEntity::Precache();

	gd_engfuncs.pfnPrecacheModel(NULL_SPRITE_FILENAME);
}

//=============================================
// @brief
//
//=============================================
bool CEnvPosPortalWorld::Spawn( void )
{
	// Set modelname to null sprite model
	m_pFields->modelname = gd_engfuncs.pfnAllocString(NULL_SPRITE_FILENAME);

	if(!CPointEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname, false))
		return false;

	m_pState->effects &= ~EF_NODRAW;
	m_pState->effects |= EF_NOVIS;

	m_pState->rendertype = RT_ENVPOSPORTALWORLD;
	return true;
}
