/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerforceclose.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_forceclose, CTriggerForceClose);

//=============================================
// @brief
//
//=============================================
CTriggerForceClose::CTriggerForceClose( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerForceClose::~CTriggerForceClose( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerForceClose::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	const Char* pstrName = gd_engfuncs.pfnGetString(m_pFields->target);
	if(!pstrName || !qstrlen(pstrName))
	{
		Util::EntityConPrintf(m_pEdict, "Target was null.\n");
		return;
	}

	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, pstrName);
		if(!pedict)
			break;

		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(pEntity && pEntity->IsFuncDoorEntity())
			pEntity->SetForcedClose();
	}
}