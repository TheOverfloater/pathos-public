/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerentity.h"

//=============================================
// @brief
//
//=============================================
CTriggerEntity::CTriggerEntity( edict_t* pedict ):
	CDelayEntity(pedict),
	m_filterEntityName(NO_STRING_VALUE),
	m_masterEntityName(NO_STRING_VALUE),
	m_triggerMessage(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerEntity::~CTriggerEntity( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerEntity::DeclareSaveFields( void )
{
	// Call base class to do it first
	CDelayEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerEntity, m_filterEntityName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerEntity, m_masterEntityName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerEntity, m_triggerMessage, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerEntity::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "filtername"))
	{
		m_filterEntityName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "master"))
	{
		m_masterEntityName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "triggermsg"))
	{
		m_triggerMessage = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CDelayEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerEntity::IsMasterTriggered( void )
{
	if(m_masterEntityName != NO_STRING_VALUE)
	{
		const Char* pstrMasterName = gd_engfuncs.pfnGetString(m_masterEntityName);
		if(!Util::IsMasterTriggered(pstrMasterName, m_activator, this))
			return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CTriggerEntity::CheckFilterFlags( CBaseEntity* pOther, bool noclients, bool allownpcs, bool allowpushables )
{
	if(!pOther)
		return false;

	if((pOther->GetFlags() & FL_CLIENT && !noclients)
		|| (pOther->GetFlags() & FL_NPC && allownpcs)
		|| (pOther->IsFuncPushableEntity() && allowpushables))
	{
		if(m_filterEntityName != NO_STRING_VALUE && !pOther->IsPlayer())
		{
			const Char* pstrFilterName = gd_engfuncs.pfnGetString(m_filterEntityName);
			if(qstrcmp(pstrFilterName, pOther->GetTargetName()))
				return false;
		}

		return true;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CTriggerEntity::Spawn( void )
{
	if(!CDelayEntity::Spawn())
		return false;

	m_pState->solid = SOLID_TRIGGER;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->effects |= EF_NODRAW;

	if(!SetModel(m_pFields->modelname))
		return false;

	return true;
}
