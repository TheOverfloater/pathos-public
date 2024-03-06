/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "vector.h"
#include "edict.h"
#include "gamedll.h"
#include "gdll_interface.h"
#include "file_interface.h"
#include "entities.h"
#include "baseentity.h"
#include "com_math.h"
#include "constants.h"
#include "cache_model.h"
#include "studio.h"
#include "vbm_shared.h"
#include "gamedll.h"
#include "gamevars.h"
#include "util.h"
#include "contents.h"
#include "snd_shared.h"
#include "trace.h"
#include "util.h"
#include "activity.h"
#include "animevent.h"
#include "usermsgs.h"
#include "skilldata.h"
#include "worldspawn.h"
#include "envmessage.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(worldspawn, CWorldSpawn);

//=============================================
// @brief
//
//=============================================
CWorldSpawn::CWorldSpawn( edict_t* pedict ):
	CBaseEntity( pedict ),
	m_skyboxName(NO_STRING_VALUE),
	m_chapterTitle(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CWorldSpawn::~CWorldSpawn()
{
}

//=============================================
// @brief
//
//=============================================
bool CWorldSpawn::Spawn( void )
{
	// NOTE: Model is already set in SV_SpawnGame
	// Set origin via interface
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);

	m_pState->solid = SOLID_BSP;
	m_pState->movetype = MOVETYPE_PUSH;

	if(m_skyboxName != NO_STRING_VALUE)
	{
		const Char* pstrSkyName = gd_engfuncs.pfnGetString(m_skyboxName);
		gd_engfuncs.pfnSetCVarString("sv_skyname", pstrSkyName);
	}

	if(m_chapterTitle != NO_STRING_VALUE)
		m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CWorldSpawn::Precache( void )
{
}

//=============================================
// @brief
//
//=============================================
void CWorldSpawn::InitEntity( void )
{
	CEnvMessage* pMessage = reinterpret_cast<CEnvMessage*>(CBaseEntity::CreateEntity("env_message", nullptr));
	if(pMessage)
	{
		const Char* pstrMessage = gd_engfuncs.pfnGetString(m_chapterTitle);
		pMessage->SetMessage(pstrMessage);

		if(!pMessage->Spawn())
		{
			Util::RemoveEntity(pMessage);
			return;
		}

		pMessage->SetThink(&CBaseEntity::CallUseToggleThink);
		pMessage->SetNextThink(0.3);
		pMessage->SetSpawnFlag(CEnvMessage::FL_ONLY_ONCE);
	}
}

//=============================================
// @brief
//
//=============================================
bool CWorldSpawn::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "skyname"))
	{
		if(qstrlen(kv.value) > 0)
			m_skyboxName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "chaptertitle"))
	{
		m_chapterTitle = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "sounds")
		|| !qstrcmp(kv.keyname, "WaveHeight")
		|| !qstrcmp(kv.keyname, "MaxRange")
		|| !qstrcmp(kv.keyname, "chaptertitle")
		|| !qstrcmp(kv.keyname, "startdark")
		|| !qstrcmp(kv.keyname, "newunit")
		|| !qstrcmp(kv.keyname, "gametitle")
		|| !qstrcmp(kv.keyname, "mapteams")
		|| !qstrcmp(kv.keyname, "defaultteam")
		|| !qstrcmp(kv.keyname, "overdarken")
		|| !qstrcmp(kv.keyname, "specialfog"))
	{
		// Shut engine up about unhandled keyvalues
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CWorldSpawn::Restore( void )
{
	// Set origin via interface
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
	return true;
}