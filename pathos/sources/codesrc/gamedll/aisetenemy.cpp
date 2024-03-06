/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "aisetenemy.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(ai_setenemy, CAISetEnemy);

//=============================================
// @brief
//
//=============================================
CAISetEnemy::CAISetEnemy( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CAISetEnemy::~CAISetEnemy( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CAISetEnemy::Spawn( void )
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
void CAISetEnemy::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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

	CBaseEntity* pEnemyEntity = nullptr;
	const Char* pstrEnemyEntityName = gd_engfuncs.pfnGetString(m_pFields->netname);

	if(!qstrcmp(pstrEnemyEntityName, "player"))
	{
		pEnemyEntity = Util::GetHostPlayer();
		if(!pEnemyEntity)
		{
			Util::EntityConPrintf(m_pEdict, "Could not find enemy '%s'.\n", pstrEnemyEntityName);
			return;
		}
	}
	else
	{
		edict_t* pEnemyEdict = Util::FindEntityByTargetName(nullptr, pstrEnemyEntityName);
		if(Util::IsNullEntity(pEnemyEdict))
		{
			Util::EntityConPrintf(m_pEdict, "Could not find enemy '%s'.\n", pstrEnemyEntityName);
			return;
		}

		pEnemyEntity = CBaseEntity::GetClass(pEnemyEdict);
	}

	if(!pEnemyEntity->IsPlayer() && !pEnemyEntity->IsNPC())
	{
		Util::EntityConDPrintf(m_pEdict, "Entity '%s' is not a player or an NPC.\n", pstrEnemyEntityName);
		return;
	}

	pTargetEntity->PushEnemy(pEnemyEntity, pEnemyEntity->GetNavigablePosition(), pEnemyEntity->GetAngles());
}