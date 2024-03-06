/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerchangelevel.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_changelevel, CTriggerChangeLevel);

//=============================================
// @brief
//
//=============================================
CTriggerChangeLevel::CTriggerChangeLevel( edict_t* pedict ):
	CBaseEntity(pedict),
	m_nextLevelName(NO_STRING_VALUE),
	m_landmarkName(NO_STRING_VALUE),
	m_lastFiredtime(0)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerChangeLevel::~CTriggerChangeLevel( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerChangeLevel::DeclareSaveFields( void )
{
	// Call base class to do it first
	CBaseEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerChangeLevel, m_nextLevelName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerChangeLevel, m_landmarkName, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerChangeLevel::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "map"))
	{
		m_nextLevelName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "landmark"))
	{
		m_landmarkName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerChangeLevel::Spawn( void )
{
	if(m_nextLevelName == NO_STRING_VALUE 
		|| m_landmarkName == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	const Char* pstrLevelname = gd_engfuncs.pfnGetString(m_nextLevelName);
	if(qstrlen(pstrLevelname) >= SAVE_FILE_STRING_MAX_LENGTH)
	{
		Util::EntityConPrintf(m_pEdict, "Level name '%s' is too long.\n", pstrLevelname);
		return false;
	}

	const Char* pstrLandmarkName = gd_engfuncs.pfnGetString(m_landmarkName);
	if(qstrlen(pstrLandmarkName) >= SAVE_FILE_STRING_MAX_LENGTH)
	{
		Util::EntityConPrintf(m_pEdict, "Landmark name '%s' is too long.\n", pstrLandmarkName);
		return false;
	}

	if(!CBaseEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname, true))
		return false;

	// Set basic properties
	m_pState->solid = SOLID_TRIGGER;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->effects |= EF_NODRAW;

	// Link it
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);

	// Set to trigger on touch if not trigger only
	if(!HasSpawnFlag(FL_TRIGGER_ONLY))
		SetTouch(&CTriggerChangeLevel::ChangeLevelTouch);

	// Add this as a connection to the list
	gd_engfuncs.pfnAddLevelConnection(g_pGameVars->levelname.c_str(), pstrLevelname, pstrLandmarkName, nullptr);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerChangeLevel::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Do the level change
	BeginLevelChange();
}

//=============================================
// @brief
//
//=============================================
void CTriggerChangeLevel::BeginLevelChange( void )
{
	// Do not allow multiple clients to fire
	if(m_lastFiredtime == g_pGameVars->time)
		return;

	// Set last fired time
	m_lastFiredtime = g_pGameVars->time;

	// Get params
	const Char* pstrLevelname = gd_engfuncs.pfnGetString(m_nextLevelName);
	const Char* pstrLandmarkName = gd_engfuncs.pfnGetString(m_landmarkName);

	// Find landmark entity
	edict_t* plandmark = nullptr;
	while(true)
	{
		plandmark = Util::FindEntityByTargetName(plandmark, pstrLandmarkName);
		if(!plandmark)
			break;

		if(!qstrcmp(gd_engfuncs.pfnGetString(plandmark->fields.classname), "info_landmark"))
			break;
	}

	if(!plandmark)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find landmark '%s'.\n", pstrLandmarkName);
		return;
	}

	// Perform the level change
	gd_engfuncs.pfnBeginLevelChange(pstrLevelname, pstrLandmarkName, plandmark->state.origin);
}

//=============================================
// @brief
//
//=============================================
void CTriggerChangeLevel::ChangeLevelTouch( CBaseEntity* pOther )
{
	if(!pOther->IsPlayer())
		return;

	BeginLevelChange();
}