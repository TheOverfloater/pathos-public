/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggermove.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_move, CTriggerMove);

//=============================================
// @brief
//
//=============================================
CTriggerMove::CTriggerMove( edict_t* pedict ):
	CPointEntity(pedict),
	m_landmarkEntity(NO_STRING_VALUE),
	m_groundEntity(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerMove::~CTriggerMove( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerMove::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerMove, m_landmarkEntity, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerMove, m_groundEntity, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerMove::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "landmark"))
	{
		m_landmarkEntity = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "groundent"))
	{
		m_groundEntity = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerMove::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerMove::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	edict_t *pMoveEdict = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_pFields->target));
	if(!pMoveEdict)
		return;

	edict_t *pMoveDest = nullptr;
	if(!strcmp(gd_engfuncs.pfnGetString(m_pFields->netname), "player"))
		pMoveDest = gd_engfuncs.pfnGetEdictByIndex(HOST_CLIENT_ENTITY_INDEX);
	else
		pMoveDest = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_pFields->netname));

	if(!pMoveDest || Util::IsNullEntity(pMoveDest))
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find '%s'.\n", gd_engfuncs.pfnGetString(m_pFields->netname));
		return;
	}

	// Get entity pointers
	CBaseEntity* pMoveDestEntity = CBaseEntity::GetClass(pMoveDest);
	CBaseEntity* pMoveEntity = CBaseEntity::GetClass(pMoveEdict);

	if(m_landmarkEntity != NO_STRING_VALUE)
	{
		edict_t* pLandmarkEdict = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_landmarkEntity));
		if(!pLandmarkEdict)
		{
			Util::EntityConPrintf(m_pEdict, "Can't find landmark entity '%s'\n", gd_engfuncs.pfnGetString(m_landmarkEntity));
			return;
		}

		CBaseEntity* pLandmarkEntity = CBaseEntity::GetClass(pLandmarkEdict);
		Vector vecOffset = pMoveEntity->GetOrigin() - pLandmarkEntity->GetOrigin();
		Vector vecEntityOrigin = pMoveDestEntity->GetOrigin() + vecOffset;
		pMoveEntity->SetOrigin(vecEntityOrigin);

		if(HasSpawnFlag(FL_COPY_ANGLES))
			pMoveEntity->SetAngles(pMoveDestEntity->GetAngles());
	}
	else
	{
		CBaseEntity* pDestination = CBaseEntity::GetClass(pMoveDest);
		if(pDestination->IsPlayer())
		{
			trace_t tr;
			Util::TraceLine(pDestination->GetOrigin(), pDestination->GetOrigin() - Vector(0, 0, 1024), true, false, true, GetEdict(), tr);

			if(tr.allSolid() || tr.startSolid() || tr.noHit())
			{
				Util::EntityConPrintf(m_pEdict, "Invalid trace for entity.\n");
				return;
			}

			pMoveEntity->SetOrigin(tr.endpos);
		}
		else
			pMoveEntity->SetOrigin(pMoveDestEntity->GetOrigin());

		if(HasSpawnFlag(FL_COPY_ANGLES))
			pMoveEntity->SetAngles(pMoveDestEntity->GetAngles());
	}

	if(pMoveEntity->IsNPC())
	{
		// Move npc up a bit, and then drop him
		pMoveEntity->RemoveFlags(FL_ONGROUND);
		pMoveEntity->SetOrigin(pMoveEntity->GetOrigin() + Vector(0, 0, 8));
		pMoveEntity->DropToFloor();
		 
		// Set ground entity
		if(m_groundEntity != NO_STRING_VALUE)
		{
			edict_t* pGroundEntity = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_groundEntity));
			if(!pGroundEntity)
			{
				Util::EntityConPrintf(m_pEdict, "Can't find ground entity '%s'\n", gd_engfuncs.pfnGetString(m_groundEntity));
				return;
			}

			pMoveEntity->SetGroundEntity(CBaseEntity::GetClass(pGroundEntity));
		}
	}

	if(pMoveEntity->IsParented())
		pMoveEntity->SetParent(nullptr);
}