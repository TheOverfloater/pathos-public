/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerkeypad.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_keypad, CTriggerKeypad);

//=============================================
// @brief
//
//=============================================
CTriggerKeypad::CTriggerKeypad( edict_t* pedict ):
	CPointEntity(pedict),
	m_keypadId(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerKeypad::~CTriggerKeypad( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerKeypad::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerKeypad, m_keypadId, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerKeypad::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "keypadid"))
	{
		m_keypadId = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerKeypad::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerKeypad::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Either use activator, or assume it's local player
	CBaseEntity* pEntity;
	if(pActivator && pActivator->IsPlayer())
		pEntity = pActivator;
	else
		pEntity = Util::GetHostPlayer();

	if(!pEntity || !pEntity->IsPlayer())
	{
		Util::EntityConPrintf(m_pEdict, "Not a player entity.\n");
		return;
	}

	const Char* pstrPasscode = nullptr;
	if(m_pFields->message != NO_STRING_VALUE)
		pstrPasscode = gd_engfuncs.pfnGetString(m_pFields->message);

	pEntity->SpawnKeypadWindow(gd_engfuncs.pfnGetString(m_keypadId), pstrPasscode, this, HasSpawnFlag(FL_STAY_TILL_NEXT) ? true : false);
}

//=============================================
// @brief
//
//=============================================
void CTriggerKeypad::FireTarget( CBaseEntity* pPlayer )
{
	if(m_pFields->target == NO_STRING_VALUE)
	{
		Util::EntityConPrintf(m_pEdict, "No target specified.\n");
		return;
	}

	Util::FireTargets(gd_engfuncs.pfnGetString(m_pFields->target), pPlayer, this, USE_TOGGLE, 0);
}