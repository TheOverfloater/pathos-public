/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "aisettriggercondition.h"

// Number of AI condition triggers
const Uint32 CAISetTriggerCondition::NUM_CONDITION_TRIGGERS = 2;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(ai_settriggercondition, CAISetTriggerCondition);

//=============================================
// @brief
//
//=============================================
CAISetTriggerCondition::CAISetTriggerCondition( edict_t* pedict ):
	CPointEntity(pedict),
	m_triggerCondition(0),
	m_triggerTarget(NO_STRING_VALUE),
	m_conditionIndex(0)
{
}

//=============================================
// @brief
//
//=============================================
CAISetTriggerCondition::~CAISetTriggerCondition( void )
{
}

//=============================================
// @brief
//
//=============================================
void CAISetTriggerCondition::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CAISetTriggerCondition, m_triggerCondition, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CAISetTriggerCondition, m_triggerTarget, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CAISetTriggerCondition, m_conditionIndex, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CAISetTriggerCondition::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "triggercondition"))
	{
		m_triggerCondition = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "triggertarget"))
	{
		m_triggerTarget = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "index"))
	{
		m_conditionIndex = SDL_atoi(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CAISetTriggerCondition::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->target == NO_STRING_VALUE
		|| m_triggerTarget == NO_STRING_VALUE
		|| !m_triggerCondition)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(m_conditionIndex < 0 || m_conditionIndex >= NUM_CONDITION_TRIGGERS)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid 'index' value of %d.\n", m_conditionIndex);
		m_conditionIndex = 0;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CAISetTriggerCondition::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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

	pTargetEntity->SetAITriggerCondition(m_conditionIndex, m_triggerCondition, gd_engfuncs.pfnGetString(m_triggerTarget));
}