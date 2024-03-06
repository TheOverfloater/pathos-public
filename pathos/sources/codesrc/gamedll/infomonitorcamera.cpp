/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "infomonitorcamera.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(info_monitorcamera, CInfoMonitorCamera);

//=============================================
// @brief
//
//=============================================
CInfoMonitorCamera::CInfoMonitorCamera( edict_t* pedict ):
	CPointEntity(pedict),
	m_pPVSData(nullptr),
	m_numCameraMonitors(0)
{
	for(Uint32 i = 0; i < MAX_MONITOR_ENTITIES; i++)
		m_pCameraMonitors[i] = nullptr;
}

//=============================================
// @brief
//
//=============================================
CInfoMonitorCamera::~CInfoMonitorCamera( void )
{
}

//=============================================
// @brief
//
//=============================================
void CInfoMonitorCamera::Precache( void )
{
	CPointEntity::Precache();

	gd_engfuncs.pfnPrecacheModel(NULL_SPRITE_FILENAME);
}

//=============================================
// @brief
//
//=============================================
bool CInfoMonitorCamera::Spawn( void )
{
	// Set modelname to null sprite model
	m_pFields->modelname = gd_engfuncs.pfnAllocString(NULL_SPRITE_FILENAME);

	if(!CPointEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname, false))
		return false;

	m_pState->effects &= ~EF_NODRAW;
	m_pState->rendertype = RT_MONITORCAMERA;

	// Make sure this gets set
	SetPVSData();

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CInfoMonitorCamera::Restore( void )
{
	if(!CPointEntity::Restore())
		return false;

	if(!m_pPVSData)
		SetPVSData();

	return true;
}

//=============================================
// @brief
//
//=============================================
void CInfoMonitorCamera::FreeEntity( void )
{
	if(m_pPVSData)
	{
		delete[] m_pPVSData;
		m_pPVSData = nullptr;
	}

	m_numCameraMonitors = 0;

	for(Uint32 i = 0; i < MAX_MONITOR_ENTITIES; i++)
		m_pCameraMonitors[i] = nullptr;
}

//=============================================
// @brief
//
//=============================================
void CInfoMonitorCamera::SetPVSData( void )
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
const byte* CInfoMonitorCamera::GetPVSData( void ) const
{
	return m_pPVSData;
}

//=============================================
// @brief
//
//=============================================
bool CInfoMonitorCamera::CheckCameraBBox( const edict_t* pedict ) const
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
void CInfoMonitorCamera::AddCameraMonitorEntity( const edict_t* pedict )
{
	for(Int32 i = 0; i < m_numCameraMonitors; i++)
	{
		if(m_pCameraMonitors[i] == pedict)
			return;
	}

	if(m_numCameraMonitors == MAX_MONITOR_ENTITIES)
	{
		Util::EntityConPrintf(m_pEdict, "Exceeded MAX_MONITOR_ENTITIES.\n");
		return;
	}

	m_pCameraMonitors[m_numCameraMonitors] = pedict;
	m_numCameraMonitors++;
}

//=============================================
// @brief
//
//=============================================
Uint32 CInfoMonitorCamera::GetNbCameraMonitors( void ) const
{
	return m_numCameraMonitors;
}

//=============================================
// @brief
//
//=============================================
const edict_t* CInfoMonitorCamera::GetMonitorByIndex( Uint32 index ) const
{
	assert(index < (Uint32)m_numCameraMonitors);
	return m_pCameraMonitors[index];
}