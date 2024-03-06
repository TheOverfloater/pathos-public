/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcportalsurface.h"
#include "envfog.h"

// Array of portal surfaces in global list
CArray<CBaseEntity*> CFuncPortalSurface::m_pPortalSurfacesArray;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_portal_surface, CFuncPortalSurface);

//=============================================
// @brief
//
//=============================================
CFuncPortalSurface::CFuncPortalSurface( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncPortalSurface::~CFuncPortalSurface( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncPortalSurface::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname))
		return false;

	if(!HasSpawnFlag(FL_NOT_SOLID))
		m_pState->solid = SOLID_BSP;
	else
		m_pState->solid = SOLID_NOT;

	m_pState->movetype = MOVETYPE_PUSH;
	m_pState->effects |= EF_STATICENTITY;
	m_pState->flags |= (FL_WORLDBRUSH|FL_INITIALIZE);
	m_pState->rendertype = RT_PORTALSURFACE;

	if(m_pFields->target == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(HasSpawnFlag(FL_START_OFF))
		m_pState->effects |= EF_NODRAW;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CFuncPortalSurface::Restore( void )
{
	if(!CBaseEntity::Restore())
		return false;

	m_pState->flags |= FL_INITIALIZE;
	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncPortalSurface::InitEntity( void )
{
	edict_t* pTargetEdict = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_pFields->target));
	if(!pTargetEdict)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find target '%s'.\n", gd_engfuncs.pfnGetString(m_pFields->target));
		Util::RemoveEntity(this);
		return;
	}

	CBaseEntity* pEntity = CBaseEntity::GetClass(pTargetEdict);
	if(!pEntity || !pEntity->IsEnvPosPortalEntity())
	{
		Util::EntityConPrintf(m_pEdict, "Target '%s' is not a valid envpos_portal entity.\n", gd_engfuncs.pfnGetString(m_pFields->target));
		Util::RemoveEntity(this);
		return;
	}

	// Set aiment
	SetAiment(pEntity);
	// Set this as a portal surface
	pEntity->AddPortalSurfaceEntity(m_pEdict);

	// Add as portal surface entity
	CFuncPortalSurface::AddPortalSurfaceEntity(this);
}

//=============================================
// @brief
//
//=============================================
const CBaseEntity* CFuncPortalSurface::GetEnvPosPortalEntity( void ) const
{
	if(m_pState->aiment == NO_ENTITY_INDEX)
		return nullptr;

	edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(m_pState->aiment);
	if(Util::IsNullEntity(pedict))
		return nullptr;

	return CBaseEntity::GetClass(pedict);
}

//=============================================
// @brief
//
//=============================================
void CFuncPortalSurface::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	switch(useMode)
	{
	case USE_ON:
		m_pState->effects &= ~EF_NODRAW;
		break;
	case USE_OFF:
		m_pState->effects |= EF_NODRAW;
		break;
	case USE_TOGGLE:
		{
			if(m_pState->effects & EF_NODRAW)
				m_pState->effects &= ~EF_NODRAW;
			else
				m_pState->effects |= EF_NODRAW;
		}
		break;
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncPortalSurface::ClearPortalSurfaceList( void )
{
	if(m_pPortalSurfacesArray.empty())
		return;

	m_pPortalSurfacesArray.clear();
}

//=============================================
// @brief
//
//=============================================
void CFuncPortalSurface::AddPortalSurfaceEntity( CBaseEntity* pPortalSurface )
{
	for(Uint32 i = 0; i < m_pPortalSurfacesArray.size(); i++)
	{
		if(m_pPortalSurfacesArray[i] == pPortalSurface)
			return;
	}

	m_pPortalSurfacesArray.push_back(pPortalSurface);
}

//=============================================
// @brief
//
//=============================================
Uint32 CFuncPortalSurface::GetNbPortalSurfaces( void )
{
	return m_pPortalSurfacesArray.size();
}

//=============================================
// @brief
//
//=============================================
const CBaseEntity* CFuncPortalSurface::GetPortalSurfaceByIndex( Uint32 index )
{
	assert(index < m_pPortalSurfacesArray.size());
	return m_pPortalSurfacesArray[index];
}

//=============================================
// @brief
//
//=============================================
bool CFuncPortalSurface::CheckVISForEntity( const edict_t* pclient, const edict_t* pedict, const byte* pset )
{
	if(m_pPortalSurfacesArray.empty())
		return false;

	const CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	if(!pEntity->IsEnvPosPortalEntity() && !pEntity->IsFuncPortalSurfaceEntity())
	{
		for(Uint32 i = 0; i < m_pPortalSurfacesArray.size(); i++)
		{
			CBaseEntity* pPortalSurfaceEntity = m_pPortalSurfacesArray[i];
			if(!pPortalSurfaceEntity || (pPortalSurfaceEntity->HasEffectFlag(EF_NODRAW)))
				continue;

			const edict_t* pPortalSurfaceEdict = pPortalSurfaceEntity->GetEdict();
			if(Common::CheckVisibility(pPortalSurfaceEdict->leafnums, pset))
			{
				const CBaseEntity* pPortalEntity = pPortalSurfaceEntity->GetEnvPosPortalEntity();
				if(!pPortalEntity)
					continue;

				const byte* pPortalPVS = pPortalEntity->GetPVSData();
				if(pPortalPVS && Common::CheckVisibility(pedict->leafnums, pPortalPVS) && pPortalEntity->CheckPortalBBox(pedict))
					return true;
			}
		}
	}
	else if(pEntity->IsEnvPosPortalEntity())
	{
		Uint32 nbportalsurfaces = pEntity->GetNbPortalSurfaces();
		for(Uint32 i = 0; i < nbportalsurfaces; i++)
		{
			const edict_t* pPortalSurface = pEntity->GetPortalSurfaceByIndex(i);
			if(pPortalSurface && !(pPortalSurface->state.effects & EF_NODRAW) 
				&& Common::CheckVisibility(pPortalSurface->leafnums, pset) 
				&& !CEnvFog::FogCull(*pclient, *pPortalSurface))
				return true;
		}
	}

	return false;
}