/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envsyncanimation.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_syncanimation, CEnvSyncAnimation);

//=============================================
// @brief
//
//=============================================
CEnvSyncAnimation::CEnvSyncAnimation( edict_t* pedict ):
	CPointEntity(pedict),
	m_isSyncEnabled(false)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSyncAnimation::~CEnvSyncAnimation( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvSyncAnimation::DeclareSaveFields( void )
{
	// Call base class to handle it's own
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSyncAnimation, m_isSyncEnabled, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CEnvSyncAnimation::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE 
		|| m_pFields->target == NO_STRING_VALUE
		|| m_pFields->netname == NO_STRING_VALUE)
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
void CEnvSyncAnimation::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	const Char* pstrEntityName = gd_engfuncs.pfnGetString(m_pFields->target);
	const Char* pstrSyncTargetName = gd_engfuncs.pfnGetString(m_pFields->netname);

	edict_t* pTargetEdict = Util::FindEntityByTargetName(nullptr, pstrEntityName);
	if(!pTargetEdict)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find target '%s'.\n", pstrEntityName);
		return;
	}

	edict_t* pSyncTargetEdict = Util::FindEntityByTargetName(nullptr, pstrSyncTargetName);
	if(!pSyncTargetEdict)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find sync target '%s'.\n", pstrSyncTargetName);
		return;
	}

	CBaseEntity* pTargetEntity = CBaseEntity::GetClass(pTargetEdict);
	if(!pTargetEntity)
	{
		Util::EntityConPrintf(m_pEdict, "Target entity '%s' is not a valid entity.\n", pstrSyncTargetName);
		return;
	}

	if(!m_isSyncEnabled)
	{
		CBaseEntity* pSyncTargetEntity = CBaseEntity::GetClass(pSyncTargetEdict);
		if(!pSyncTargetEntity)
		{
			Util::EntityConPrintf(m_pEdict, "Sync target entity '%s' is not a valid entity.\n", pstrSyncTargetName);
			return;
		}

		pTargetEntity->SetAiment(pSyncTargetEntity);
		pTargetEntity->SetEffectFlag(EF_SYNCSEQUENCE);
	}
	else
	{
		pTargetEntity->SetAiment(nullptr);
		pTargetEntity->RemoveEffectFlag(EF_SYNCSEQUENCE);
	}
}
