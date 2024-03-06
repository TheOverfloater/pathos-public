/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerchangetarget.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_changetarget, CTriggerChangeTarget);

//=============================================
// @brief
//
//=============================================
CTriggerChangeTarget::CTriggerChangeTarget( edict_t* pedict ):
	CDelayEntity(pedict),
	m_newTarget(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerChangeTarget::~CTriggerChangeTarget( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerChangeTarget::DeclareSaveFields( void )
{
	// Call base class to do it first
	CDelayEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerChangeTarget, m_newTarget, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerChangeTarget::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "m_iszNewTarget"))
	{
		m_newTarget = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CDelayEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerChangeTarget::Spawn( void )
{
	if(!CDelayEntity::Spawn())
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
void CTriggerChangeTarget::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(m_pFields->target == NO_STRING_VALUE)
		return;

	const Char* pstrTarget = gd_engfuncs.pfnGetString(m_pFields->target);

	edict_t* pEdict = nullptr;
	while(true)
	{
		pEdict = Util::FindEntityByTargetName(pEdict, pstrTarget);
		if(!pEdict)
			break;

		if(!Util::IsNullEntity(pEdict))
		{
			CBaseEntity* pEntity = CBaseEntity::GetClass(pEdict);
			if(pEntity)
				pEntity->SetTarget(m_newTarget);
		}
	}
}