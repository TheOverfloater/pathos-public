/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envsetbody.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_setbody, CEnvSetBody);

//=============================================
// @brief
//
//=============================================
CEnvSetBody::CEnvSetBody( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSetBody::~CEnvSetBody( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CEnvSetBody::Spawn( void )
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
void CEnvSetBody::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool isFirst = false;
	edict_t* pedict = nullptr;
	const Char* pstrEntityName = gd_engfuncs.pfnGetString(m_pFields->target);

	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, pstrEntityName);
		if(!pedict)
		{
			if(isFirst)
				Util::EntityConPrintf(m_pEdict, "Couldn't find target '%s'.\n", pstrEntityName);

			return;
		}

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		pEntity->SetBody(m_pState->body);
		isFirst = false;
	}
}