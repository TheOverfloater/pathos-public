/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerlogin.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_login, CTriggerLogin);

//=============================================
// @brief
//
//=============================================
CTriggerLogin::CTriggerLogin( edict_t* pedict ):
	CPointEntity(pedict),
	m_username(NO_STRING_VALUE),
	m_password(NO_STRING_VALUE),
	m_codeid(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerLogin::~CTriggerLogin( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerLogin::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerLogin, m_username, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerLogin, m_password, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerLogin, m_codeid, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerLogin::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "username"))
	{
		m_username = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "password"))
	{
		m_password = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "keypadid"))
	{
		m_codeid = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerLogin::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerLogin::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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

	const Char* pstruser = gd_engfuncs.pfnGetString(m_username);
	const Char* pstrpassword = gd_engfuncs.pfnGetString(m_password);
	const Char* pstrcodeid = gd_engfuncs.pfnGetString(m_codeid);

	pEntity->SpawnLoginWindow(pstruser, pstrpassword, pstrcodeid, this, HasSpawnFlag(FL_STAY_TILL_NEXT) ? true : false);
}

//=============================================
// @brief
//
//=============================================
void CTriggerLogin::FireTarget( CBaseEntity* pPlayer )
{
	if(m_pFields->target == NO_STRING_VALUE)
	{
		gd_engfuncs.pfnClientPrintf(nullptr, "%s - No target specified.\n", gd_engfuncs.pfnGetString(m_pFields->classname));
		return;
	}

	Util::FireTargets(gd_engfuncs.pfnGetString(m_pFields->target), pPlayer, this, USE_TOGGLE, 0);
}