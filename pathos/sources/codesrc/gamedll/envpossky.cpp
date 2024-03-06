/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envpossky.h"

// Distance between PVS updates
const Float CEnvPosSky::PVS_UPDATE_DIST = 4.0f;

// Skybox entity used for VIS tests
CEnvPosSky* CEnvPosSky::g_pSkyEntity = nullptr;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(envpos_sky, CEnvPosSky);

//=============================================
// @brief
//
//=============================================
CEnvPosSky::CEnvPosSky( edict_t* pedict ):
	CFuncTrain(pedict),
	m_pPVSData(nullptr)
{
	// Complain if it's not cleared/was already set
	if(g_pSkyEntity)
		gd_engfuncs.pfnCon_Printf("Multiple %s entities defined in level!.\n", gd_engfuncs.pfnGetString(m_pFields->classname));

	// Set skybox pointer
	g_pSkyEntity = this;
}

//=============================================
// @brief
//
//=============================================
CEnvPosSky::~CEnvPosSky( void )
{
	if(m_pPVSData)
	{
		delete[] m_pPVSData;
		m_pPVSData = nullptr;
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvPosSky::Precache( void )
{
	gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_pFields->modelname));

	CFuncTrain::Precache();
}

//=============================================
// @brief
//
//=============================================
bool CEnvPosSky::Spawn( void )
{
	m_pFields->modelname = gd_engfuncs.pfnAllocString(NULL_SPRITE_FILENAME);

	return CFuncTrain::Spawn();
}

//=============================================
// @brief
//
//=============================================
void CEnvPosSky::SetSpawnProperties( void )
{
	// Movetype needs to be MOVETYPE_PUSH so ltime is valid
	m_pState->movetype = MOVETYPE_PUSH;
	m_pState->solid = SOLID_NOT;
	m_pState->rendertype = RT_ENVSKYENT;
	m_pState->effects |= EF_NOVIS;
}

//=============================================
// @brief
//
//=============================================
Vector CEnvPosSky::GetDestinationVector( const Vector& destOrigin )
{
	// env_spritetrain only uses the basic origin
	return destOrigin;
}

//=============================================
// @brief
//
//=============================================
bool CEnvPosSky::TrainSetModel( void )
{
	if(!SetModel(m_pFields->modelname, false))
		return false;

	// Reset size to zero
	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, ZERO_VECTOR, ZERO_VECTOR);

	return true;
}

//=============================================
// @brief
//
//=============================================
const byte* CEnvPosSky::GetPVSData( void )
{
	if(!m_pPVSData || (m_lastPVSOrigin-m_pState->origin).Length() > PVS_UPDATE_DIST)
		SetPVSData();

	return m_pPVSData;
}

//=============================================
// @brief
//
//=============================================
void CEnvPosSky::SetPVSData( void )
{
	Uint32 visBufferSize = gd_engfuncs.pfnGetVISBufferSize();
	if(!m_pPVSData)
	{
		// Allocate buffer
		m_pPVSData = new byte[visBufferSize];
	}

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

	gd_engfuncs.pfnLeafPVS(m_pPVSData, visBufferSize, *pleaf, *pbrushmodel);
}

//=============================================
// @brief
//
//=============================================
bool CEnvPosSky::CheckSkyboxVisibility( const edict_t* pedict )
{
	if(!g_pSkyEntity)
		return true;

	const byte* pPVS = g_pSkyEntity->GetPVSData();
	if(!pPVS)
		return false;

	return Common::CheckVisibility(pedict->leafnums, pPVS);
}

//=============================================
// @brief
//
//=============================================
void CEnvPosSky::ClearSkyboxEntity( void )
{
	g_pSkyEntity = nullptr;
}
