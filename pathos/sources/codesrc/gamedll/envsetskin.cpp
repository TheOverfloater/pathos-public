/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envsetskin.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_setskin, CEnvSetSkin);

//=============================================
// @brief
//
//=============================================
CEnvSetSkin::CEnvSetSkin( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSetSkin::~CEnvSetSkin( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CEnvSetSkin::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE || m_pFields->target == NO_STRING_VALUE)
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
void CEnvSetSkin::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	Int32 count = 0;

	edict_t* ptarget = nullptr;
	while(true)
	{
		ptarget = Util::FindEntityByTargetName(ptarget, gd_engfuncs.pfnGetString(m_pFields->target));
		if(!ptarget)
			break;

		CBaseEntity* pEntity = CBaseEntity::GetClass(ptarget);
		pEntity->SetSkin(m_pState->skin);
		count++;
	}

	if(!count)
		Util::EntityConPrintf(m_pEdict, "Couldn't find target '%s'.\n", gd_engfuncs.pfnGetString(m_pFields->target));
}