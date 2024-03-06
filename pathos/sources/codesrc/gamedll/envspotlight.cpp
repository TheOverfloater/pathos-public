/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envspotlight.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_spotlight, CEnvSpotlight);

//=============================================
// @brief
//
//=============================================
CEnvSpotlight::CEnvSpotlight( edict_t* pedict ):
	CEnvDLight(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSpotlight::~CEnvSpotlight( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvSpotlight::SetLightRenderFx( void )
{
	m_pState->rendertype = RT_ENVSPOTLIGHT;
}

//=============================================
// @brief
//
//=============================================
void CEnvSpotlight::SetMinsMaxs( void )
{
	Vector forward;
	Math::AngleVectors(m_pState->angles, &forward, nullptr, nullptr);

	Vector points[2];
	points[0] = Vector(0, 0, 0);
	Math::VectorMA(points[0], m_pState->renderamt, forward, points[1]);

	Vector mins(NULL_MINS);
	Vector maxs(NULL_MAXS);

	for(Uint32 i = 0; i < 2; i++)
	{
		for(Uint32 j = 0; j < 3; j++)
		{
			if(points[i][j] > maxs[j])
				maxs[j] = points[i][j];

			if(points[i][j] < mins[j])
				mins[j] = points[i][j];
		}
	}

	Math::VectorSubtract(mins, Vector(1, 1, 1), mins);
	Math::VectorAdd(maxs, Vector(1, 1, 1), maxs);

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, mins, maxs);
}

//=============================================
// @brief
//
//=============================================
void CEnvSpotlight::SetSpotlightValues( const Vector& origin, const Vector& angles, const Vector& color, Uint32 radius, Uint32 conesize )
{
	m_pState->origin = origin;
	m_pState->angles = angles;
	m_pState->rendercolor = color;
	m_pState->renderamt = radius;
	m_pState->scale = conesize;

	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
}

//=============================================
// @brief
//
//=============================================
CEnvSpotlight* CEnvSpotlight::SpawnSpotlight( const Vector& origin, const Vector& angles, const Vector& color, Uint32 radius, Uint32 conesize )
{
	edict_t* pedict = gd_engfuncs.pfnCreateEntity("env_spotlight");
	if(!pedict)
	{
		gd_engfuncs.pfnCon_Printf("Failed to create env_spotlight entity.\n");
		return nullptr;
	}

	CEnvSpotlight* pspotlight = reinterpret_cast<CEnvSpotlight*>(CBaseEntity::GetClass(pedict));
	if(!pspotlight)
	{
		gd_engfuncs.pfnCon_Printf("Failed to create env_spotlight entity.\n");
		return nullptr;
	}

	// Set values for lgiht
	pspotlight->SetSpotlightValues(origin, angles, color, radius, conesize);

	// Spawn the entity
	DispatchSpawn(pedict);
	return pspotlight;
}