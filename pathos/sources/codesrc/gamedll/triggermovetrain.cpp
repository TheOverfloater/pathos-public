/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggermovetrain.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_movetrain, CTriggerMoveTrain);

//=============================================
// @brief
//
//=============================================
CTriggerMoveTrain::CTriggerMoveTrain( edict_t* pedict ):
	CPointEntity(pedict),
	m_targetPathName(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerMoveTrain::~CTriggerMoveTrain( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerMoveTrain::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerMoveTrain, m_targetPathName, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerMoveTrain::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "pathtarget"))
	{
		m_targetPathName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerMoveTrain::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE
		|| m_pFields->target == NO_STRING_VALUE
		|| m_targetPathName == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerMoveTrain::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	const Char* pstrPathCornerName = gd_engfuncs.pfnGetString(m_targetPathName);
	if(!pstrPathCornerName || !qstrlen(pstrPathCornerName))
		return; // Spawn will already have warned about this

	edict_t* pTargetingNode = Util::FindEntityByTarget(nullptr, pstrPathCornerName);
	edict_t* pNode = Util::FindEntityByTargetName(nullptr, pstrPathCornerName);

	if(!pNode || Util::IsNullEntity(pNode))
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find path_corner entities.\n");
		return;
	}

	CBaseEntity* pTargetingPathCornerEntity = nullptr;
	if(pTargetingNode)
	{
		pTargetingPathCornerEntity = CBaseEntity::GetClass(pTargetingNode);
		if(!pTargetingPathCornerEntity || !pTargetingPathCornerEntity->IsPathCornerEntity())
		{
			Util::EntityConPrintf(m_pEdict, "Entity '%s' is not a valid path_corner entity.\n", pstrPathCornerName);
			return;
		}
	}

	CBaseEntity* pPathCornerEntity = CBaseEntity::GetClass(pNode);
	if(!pPathCornerEntity || !pPathCornerEntity->IsPathCornerEntity())
	{
		Util::EntityConPrintf(m_pEdict, "Entity '%s' is not a valid path_corner entity.\n", pstrPathCornerName);
		return;
	}

	// Get target train's name
	const Char* pstrTargetTrainName = gd_engfuncs.pfnGetString(m_pFields->target);
	if(!pstrTargetTrainName || !qstrlen(pstrTargetTrainName))
		return;

	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, pstrTargetTrainName);
		if(!pedict)
			break;

		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pTrainEntity = CBaseEntity::GetClass(pedict);
		if(!pTrainEntity || !pTrainEntity->IsFuncTrainEntity())
			continue;

		if(HasSpawnFlag(FL_REROUTE))
		{
			// Reroute the train
			pTrainEntity->Reroute(pPathCornerEntity, m_pState->speed);
		}
		else
		{
			// Move it to the destination
			pTrainEntity->MoveTrainToPathCorner(pPathCornerEntity, pTargetingPathCornerEntity);
		}
	}
}