/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcmonitor.h"
#include "envfog.h"

// Array of monitors in global list
CArray<CBaseEntity*> CFuncMonitor::m_pMonitorsArray;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_monitor, CFuncMonitor);

//=============================================
// @brief
//
//=============================================
CFuncMonitor::CFuncMonitor( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncMonitor::~CFuncMonitor( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncMonitor::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname))
		return false;

	m_pState->movetype = MOVETYPE_PUSH;
	m_pState->solid = SOLID_BSP;
	m_pState->flags |= (FL_WORLDBRUSH|FL_INITIALIZE);
	m_pState->rendertype = RT_MONITORENTITY;

	if(m_pFields->targetname == NO_STRING_VALUE)
		m_pState->effects |= EF_STATICENTITY;

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
bool CFuncMonitor::Restore( void )
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
void CFuncMonitor::InitEntity( void )
{
	edict_t* pTargetEdict = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_pFields->target));
	if(!pTargetEdict)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find target '%s'.\n", gd_engfuncs.pfnGetString(m_pFields->target));
		Util::RemoveEntity(this);
		return;
	}

	CBaseEntity* pEntity = CBaseEntity::GetClass(pTargetEdict);
	if(!pEntity || !pEntity->IsInfoMonitorCameraEntity())
	{
		Util::EntityConPrintf(m_pEdict, "Target '%s' is not a valid info_monitorcamera entity.\n", gd_engfuncs.pfnGetString(m_pFields->target));
		Util::RemoveEntity(this);
		return;
	}

	// Set aiment
	SetAiment(pEntity);
	// Set this as a camera
	pEntity->AddCameraMonitorEntity(m_pEdict);

	// Add as monitor entity
	CFuncMonitor::AddMonitor(this);
}

//=============================================
// @brief
//
//=============================================
const CBaseEntity* CFuncMonitor::GetCameraEntity( void ) const
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
void CFuncMonitor::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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
void CFuncMonitor::ClearMonitorList( void )
{
	if(m_pMonitorsArray.empty())
		return;

	m_pMonitorsArray.clear();
}

//=============================================
// @brief
//
//=============================================
void CFuncMonitor::AddMonitor( CBaseEntity* pMonitor )
{
	for(Uint32 i = 0; i < m_pMonitorsArray.size(); i++)
	{
		if(m_pMonitorsArray[i] == pMonitor)
			return;
	}

	m_pMonitorsArray.push_back(pMonitor);
}

//=============================================
// @brief
//
//=============================================
Uint32 CFuncMonitor::GetNbMonitors( void )
{
	return m_pMonitorsArray.size();
}

//=============================================
// @brief
//
//=============================================
const CBaseEntity* CFuncMonitor::GetMonitorByIndex( Uint32 index )
{
	assert(index < m_pMonitorsArray.size());
	return m_pMonitorsArray[index];
}

//=============================================
// @brief
//
//=============================================
bool CFuncMonitor::CheckVISForEntity( const edict_t* pclient, const edict_t* pedict, const byte* pset )
{
	if(m_pMonitorsArray.empty())
		return false;

	const CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	if(!pEntity->IsInfoMonitorCameraEntity() && !pEntity->IsFuncMonitorEntity())
	{
		for(Uint32 i = 0; i < m_pMonitorsArray.size(); i++)
		{
			CBaseEntity* pMonitorEntity = m_pMonitorsArray[i];
			if(!pMonitorEntity || (pMonitorEntity->HasEffectFlag(EF_NODRAW)))
				continue;

			const edict_t* pMonitorEdict = pMonitorEntity->GetEdict();
			if(Common::CheckVisibility(pMonitorEdict->leafnums, pset))
			{
				const CBaseEntity* pCameraEntity = pMonitorEntity->GetCameraEntity();
				if(!pCameraEntity)
					continue;

				const byte* pCameraPVS = pCameraEntity->GetPVSData();
				if(pCameraPVS && Common::CheckVisibility(pedict->leafnums, pCameraPVS) && pCameraEntity->CheckCameraBBox(pedict))
					return true;
			}
		}
	}
	else if(pEntity->IsInfoMonitorCameraEntity())
	{
		Uint32 nbcameramonitors = pEntity->GetNbCameraMonitors();
		for(Uint32 i = 0; i < nbcameramonitors; i++)
		{
			const edict_t* pmonitor = pEntity->GetMonitorByIndex(i);
			if(pmonitor && !(pmonitor->state.effects & EF_NODRAW) 
				&& Common::CheckVisibility(pmonitor->leafnums, pset) 
				&& !CEnvFog::FogCull(*pclient, *pmonitor))
				return true;
		}
	}

	return false;
}