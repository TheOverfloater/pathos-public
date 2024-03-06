/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envposportal.h"
#include "skytexturesets.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(envpos_portal, CEnvPosPortal);

//=============================================
// @brief
//
//=============================================
CEnvPosPortal::CEnvPosPortal( edict_t* pedict ):
	CPointEntity(pedict),
	m_pPVSData(nullptr),
	m_numPortalSurfaces(0),
	m_skyTextureName(NO_STRING_VALUE)
{
	for(Uint32 i = 0; i < MAX_PORTAL_ENTITIES; i++)
		m_pPortalSurfaces[i] = nullptr;
}

//=============================================
// @brief
//
//=============================================
CEnvPosPortal::~CEnvPosPortal( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvPosPortal::Precache( void )
{
	CPointEntity::Precache();

	gd_engfuncs.pfnPrecacheModel(NULL_SPRITE_FILENAME);
}

//=============================================
// @brief
//
//=============================================
void CEnvPosPortal::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvPosPortal, m_skyTextureName, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CEnvPosPortal::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "skytexture"))
	{
		m_skyTextureName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvPosPortal::Spawn( void )
{
	// Set modelname to null sprite model
	m_pFields->modelname = gd_engfuncs.pfnAllocString(NULL_SPRITE_FILENAME);

	if(!CPointEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname, false))
		return false;

	m_pState->effects &= ~EF_NODRAW;
	m_pState->rendertype = RT_ENVPOSPORTAL;

	// Set this to it's default value
	m_pState->body = NO_POSITION;

	// Find the envpos_portal_world entity if we have a size set
	if(m_pState->scale != 0)
	{
		if(m_pFields->target == NO_STRING_VALUE)
		{
			Util::WarnEmptyEntity(m_pEdict);
			Util::RemoveEntity(this);
			return false;
		}

		// So we find our target after entities are done spawning
		m_pState->flags |= FL_INITIALIZE;
	}

	// Make sure this gets set
	SetPVSData();

	// Manage custom sky texture
	if(m_skyTextureName != NO_STRING_VALUE)
	{
		const Char* pstrSkyTexture = gd_engfuncs.pfnGetString(m_skyTextureName);
		if(pstrSkyTexture)
			m_pState->body = gSkyTextureSets.RegisterSkyTextureSet(pstrSkyTexture);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvPosPortal::InitEntity( void )
{
	if(m_pState->scale != 0)
	{
		edict_t* pTargetEdict = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_pFields->target));
		if(!pTargetEdict)
		{
			Util::EntityConPrintf(m_pEdict, "Couldn't find target '%s'.\n", gd_engfuncs.pfnGetString(m_pFields->target));
			Util::RemoveEntity(this);
			return;
		}

		CBaseEntity* pEntity = CBaseEntity::GetClass(pTargetEdict);
		if(!pEntity || !pEntity->IsEnvPosPortalWorldEntity())
		{
			Util::EntityConPrintf(m_pEdict, "Target '%s' is not a valid envpos_portal_world entity.\n", gd_engfuncs.pfnGetString(m_pFields->target));
			Util::RemoveEntity(this);
			return;
		}

		// Set aiment
		SetAiment(pEntity);
	}
}

//=============================================
// @brief
//
//=============================================
bool CEnvPosPortal::Restore( void )
{
	if(!CPointEntity::Restore())
		return false;

	if(!m_pPVSData)
		SetPVSData();

	// Re-register this
	if(m_skyTextureName != NO_STRING_VALUE)
	{
		const Char* pstrSkyTexture = gd_engfuncs.pfnGetString(m_skyTextureName);
		if(pstrSkyTexture)
			m_pState->body = gSkyTextureSets.RegisterSkyTextureSet(pstrSkyTexture);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvPosPortal::FreeEntity( void )
{
	if(m_pPVSData)
	{
		delete[] m_pPVSData;
		m_pPVSData = nullptr;
	}

	m_numPortalSurfaces = 0;

	for(Uint32 i = 0; i < MAX_PORTAL_ENTITIES; i++)
		m_pPortalSurfaces[i] = nullptr;
}

//=============================================
// @brief
//
//=============================================
void CEnvPosPortal::SetPVSData( void )
{
	const cache_model_t* pcache = gd_engfuncs.pfnGetModel(WORLD_MODEL_INDEX);
	if(pcache->type != MOD_BRUSH)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid model type for worldmodel.\n");
		return;
	}
	
	const brushmodel_t* pbrushmodel = pcache->getBrushmodel();
	const mleaf_t* pleaf = gd_engfuncs.pfnPointInLeaf(m_pState->origin, (*pbrushmodel));
	if(!pleaf)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't get leaf.\n");
		return;
	}

	Uint32 bufsize = gd_engfuncs.pfnGetVISBufferSize();
	if(!m_pPVSData)
	{
		// Allocate buffer
		m_pPVSData = new byte[gd_engfuncs.pfnGetVISBufferSize()];
	}

	gd_engfuncs.pfnLeafPVS(m_pPVSData, bufsize, (*pleaf), (*pbrushmodel));
}

//=============================================
// @brief
//
//=============================================
const byte* CEnvPosPortal::GetPVSData( void ) const
{
	return m_pPVSData;
}

//=============================================
// @brief
//
//=============================================
bool CEnvPosPortal::CheckPortalBBox( const edict_t* pedict ) const
{
	if(!m_pState->renderamt)
		return true;

	Vector bboxmins, bboxmaxs;
	for(Uint32 i = 0; i < 3; i++)
	{
		bboxmins[i] = m_pState->origin[i] - m_pState->renderamt;
		bboxmaxs[i] = m_pState->origin[i] + m_pState->renderamt;
	}

	if (bboxmins[0] > pedict->state.absmax[0]) 
		return false;
	if (bboxmins[1] > pedict->state.absmax[1]) 
		return false;
	if (bboxmins[2] > pedict->state.absmax[2]) 
		return false;
	if (bboxmaxs[0] < pedict->state.absmin[0]) 
		return false;
	if (bboxmaxs[1] < pedict->state.absmin[1]) 
		return false;
	if (bboxmaxs[2] < pedict->state.absmin[2]) 
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvPosPortal::AddPortalSurfaceEntity( const edict_t* pedict )
{
	for(Int32 i = 0; i < m_numPortalSurfaces; i++)
	{
		if(m_pPortalSurfaces[i] == pedict)
			return;
	}

	if(m_numPortalSurfaces == MAX_PORTAL_ENTITIES)
	{
		Util::EntityConPrintf(m_pEdict, "Exceeded MAX_PORTAL_ENTITIES.\n");
		return;
	}

	m_pPortalSurfaces[m_numPortalSurfaces] = pedict;
	m_numPortalSurfaces++;
}

//=============================================
// @brief
//
//=============================================
Uint32 CEnvPosPortal::GetNbPortalSurfaces( void ) const
{
	return m_numPortalSurfaces;
}

//=============================================
// @brief
//
//=============================================
const edict_t* CEnvPosPortal::GetPortalSurfaceByIndex( Uint32 index ) const
{
	assert(index < (Uint32)m_numPortalSurfaces);
	return m_pPortalSurfaces[index];
}