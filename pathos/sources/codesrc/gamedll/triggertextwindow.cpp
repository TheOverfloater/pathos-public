/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggertextwindow.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_textwindow, CTriggerTextWindow);

//=============================================
// @brief
//
//=============================================
CTriggerTextWindow::CTriggerTextWindow( edict_t* pedict ):
	CPointEntity(pedict),
	m_textFilePath(NO_STRING_VALUE),
	m_codeId(NO_STRING_VALUE),
	m_code(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerTextWindow::~CTriggerTextWindow( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerTextWindow::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerTextWindow, m_textFilePath, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerTextWindow, m_codeId, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerTextWindow, m_code, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerTextWindow::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "textfile"))
	{
		m_textFilePath = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "code"))
	{
		m_code = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "keypadid"))
	{
		m_codeId = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "windowtitle"))
	{
		// Shut up engine messages
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerTextWindow::Spawn( void )
{
	if(m_textFilePath == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CPointEntity::Spawn())
		return false;

	if(m_codeId != NO_STRING_VALUE && (HasSpawnFlag(FL_AUTO_GENERATE_CODE) || m_code == NO_STRING_VALUE))
	{
		CString code;
		// Add in existing code bit
		if(m_code != NO_STRING_VALUE)
			code << gd_engfuncs.pfnGetString(m_code);

		// Generate random number
		code << (Int32)Common::RandomLong(10, 999999);
		m_code = gd_engfuncs.pfnAllocString(code.c_str());	

		RemoveSpawnFlag(FL_AUTO_GENERATE_CODE);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerTextWindow::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Either use activator, or assume it's local player
	CBaseEntity* pEntity;
	if(pActivator && pActivator->IsPlayer())
		pEntity = pActivator;
	else
		pEntity = Util::GetHostPlayer();

	if(!pEntity || !pEntity->IsPlayer())
		return;

	const Char* pstrTextFile = gd_engfuncs.pfnGetString(m_textFilePath);
	const Char* pstrCodeId = gd_engfuncs.pfnGetString(m_codeId);
	const Char* pstrCode = gd_engfuncs.pfnGetString(m_code);

	pEntity->SpawnTextWindow(pstrTextFile, pstrCode, pstrCodeId); 
}