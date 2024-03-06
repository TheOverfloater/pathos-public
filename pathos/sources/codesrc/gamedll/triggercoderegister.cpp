/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggercoderegister.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_coderegister, CTriggerCodeRegister);

//=============================================
// @brief
//
//=============================================
CTriggerCodeRegister::CTriggerCodeRegister( edict_t* pedict ):
	CPointEntity(pedict),
	m_codeId(NO_STRING_VALUE),
	m_code(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerCodeRegister::~CTriggerCodeRegister( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerCodeRegister::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCodeRegister, m_codeId, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCodeRegister, m_code, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerCodeRegister::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "keypadid"))
	{
		m_codeId = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "code"))
	{
		m_code = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerCodeRegister::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_codeId == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(HasSpawnFlag(FL_AUTO_GEN_CODE))
	{
		CString code;
		// Add in existing code bit
		if(m_code != NO_STRING_VALUE)
			code << gd_engfuncs.pfnGetString(m_code);

		// Generate random number
		code << (Int32)Common::RandomLong(10, 999999);
		m_code = gd_engfuncs.pfnAllocString(code.c_str());	

		RemoveSpawnFlag(FL_AUTO_GEN_CODE);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCodeRegister::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Either use activator, or assume it's local player
	CBaseEntity* pEntity;
	if(pActivator && pActivator->IsPlayer())
		pEntity = pActivator;
	else
		pEntity = Util::GetHostPlayer();

	if(!pEntity || !pEntity->IsPlayer())
		return;

	const Char* pstrCodeId = gd_engfuncs.pfnGetString(m_codeId);
	const Char* pstrCode = gd_engfuncs.pfnGetString(m_code);

	pEntity->AddPasscode(pstrCodeId, pstrCode); 
}