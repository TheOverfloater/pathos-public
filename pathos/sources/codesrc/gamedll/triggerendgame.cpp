/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerendgame.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_endgame, CTriggerEndGame);

//=============================================
// @brief
//
//=============================================
CTriggerEndGame::CTriggerEndGame( edict_t* pedict ):
	CPointEntity(pedict),
	m_endGameCode(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerEndGame::~CTriggerEndGame( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerEndGame::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerEndGame, m_endGameCode, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerEndGame::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "endgamecode"))
	{
		m_endGameCode = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerEndGame::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE)
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
void CTriggerEndGame::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	gd_engfuncs.pfnEndGame(gd_engfuncs.pfnGetString(m_endGameCode));
}