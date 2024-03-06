/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envsetsequence.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_setsequence, CEnvSetSequence);

//=============================================
// @brief
//
//=============================================
CEnvSetSequence::CEnvSetSequence( edict_t* pedict ):
	CPointEntity(pedict),
	m_seqDoneTarget(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSetSequence::~CEnvSetSequence( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CEnvSetSequence::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE 
		|| m_pFields->target == NO_STRING_VALUE
		|| m_pFields->message == NO_STRING_VALUE)
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
void CEnvSetSequence::DeclareSaveFields( void )
{
	// Call base class first
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSetSequence, m_seqDoneTarget, EFIELD_STRING));
}

//=============================================
// @brief Manages keyvalues
//
//=============================================
bool CEnvSetSequence::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "seqdonetarget"))
	{
		m_seqDoneTarget = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CEnvSetSequence::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	const Char* pstrEntityName = gd_engfuncs.pfnGetString(m_pFields->target);

	edict_t* pedict = Util::FindEntityByTargetName(nullptr, pstrEntityName);
	if(!pedict)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find target '%s'.\n", pstrEntityName);
		return;
	}

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);

	const Char* pstrSequenceName = gd_engfuncs.pfnGetString(m_pFields->message);
	if(pstrSequenceName)
	{
		Int32 sequenceIndex = pEntity->FindSequence(pstrSequenceName);
		if(sequenceIndex == NO_SEQUENCE_VALUE)
		{
			Util::EntityConPrintf(m_pEdict, "Could not find sequence named '%s'.\n", pstrSequenceName);
			return;
		}

		pEntity->SetSequence(sequenceIndex);
		pEntity->ResetSequenceInfo();

		if(m_pFields->target != NO_STRING_VALUE)
		{
			Float sequenceEndTime = pEntity->GetSequenceTime(sequenceIndex);
			m_pState->nextthink = g_pGameVars->time + sequenceEndTime;
			SetThink(&CEnvSetSequence::TriggerThink);
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvSetSequence::TriggerThink( void )
{
	if(m_seqDoneTarget == NO_STRING_VALUE)
		return;

	const Char* pstrName = gd_engfuncs.pfnGetString(m_seqDoneTarget);
	Util::FireTargets(pstrName, this, this, USE_TOGGLE, 0);

	m_pState->nextthink = 0;
	SetThink(nullptr);
}