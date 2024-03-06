/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envsetangles.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_setangles, CEnvSetAngles);

//=============================================
// @brief
//
//=============================================
CEnvSetAngles::CEnvSetAngles( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSetAngles::~CEnvSetAngles( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CEnvSetAngles::Spawn( void )
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
void CEnvSetAngles::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	edict_t* ptarget = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_pFields->target));
	if(!ptarget)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find target '%s'.\n", gd_engfuncs.pfnGetString(m_pFields->target));
		return;
	}

	CBaseEntity* pEntity = CBaseEntity::GetClass(ptarget);
	pEntity->SetAngles(m_pState->angles);
}