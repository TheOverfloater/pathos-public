/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "aisetfollowtarget.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(ai_setfollowtarget, CAISetFollowTarget);

//=============================================
// @brief
//
//=============================================
CAISetFollowTarget::CAISetFollowTarget( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CAISetFollowTarget::~CAISetFollowTarget( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CAISetFollowTarget::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->target == NO_STRING_VALUE
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
void CAISetFollowTarget::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	const Char* pstrTargetEntityName = gd_engfuncs.pfnGetString(m_pFields->target);

	edict_t* pTargetEdict = Util::FindEntityByTargetName(nullptr, pstrTargetEntityName);
	if(Util::IsNullEntity(pTargetEdict))
	{
		Util::EntityConPrintf(m_pEdict, "Could not find target '%s'.\n", pstrTargetEntityName);
		return;
	}

	CBaseEntity* pTargetEntity = CBaseEntity::GetClass(pTargetEdict);
	if(!pTargetEntity->IsNPC())
	{
		Util::EntityConPrintf(m_pEdict, "Entity '%s' was not an NPC.\n", pstrTargetEntityName);
		return;
	}

	CBaseEntity* pFollowEntity = nullptr;
	const Char* pstrFollowEntityName = gd_engfuncs.pfnGetString(m_pFields->netname);

	if(!qstrcmp(pstrFollowEntityName, "player"))
	{
		pFollowEntity = Util::GetHostPlayer();
		if(!pFollowEntity)
		{
			Util::EntityConPrintf(m_pEdict, "Could not find follow target '%s'.\n", pstrFollowEntityName);
			return;
		}
	}
	else
	{
		edict_t* pFollowEdict = Util::FindEntityByTargetName(nullptr, pstrFollowEntityName);
		if(Util::IsNullEntity(pFollowEdict))
		{
			Util::EntityConPrintf(m_pEdict, "Could not find follow target '%s'.\n", pstrFollowEntityName);
			return;
		}

		pFollowEntity = CBaseEntity::GetClass(pFollowEdict);
	}

	if(!pFollowEntity->IsPlayer() && !pFollowEntity->IsNPC())
	{
		Util::EntityConDPrintf(m_pEdict, "Entity '%s' is not a player or an NPC.\n", pstrFollowEntityName);
		return;
	}

	pTargetEntity->SetFollowTarget(pFollowEntity);
}