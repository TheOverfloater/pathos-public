
/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggersubwaycontroller.h"
#include "player.h"

// Subway destination id for Bergen st.
const Char CTriggerSubwayController::BERGEN_ST_DEST_ID[] = "bergen";
// Subway destination id for I B Mann st.
const Char CTriggerSubwayController::IBMANN_ST_DEST_ID[] = "ibmann";
// Subway destination id for Marshall
const Char CTriggerSubwayController::MARSHALL_ST_DEST_ID[] = "marshall";
// Subway destination id for Eckhart st.
const Char CTriggerSubwayController::ECKHART_ST_DEST_ID[] = "eckhart";

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_subway_controller, CTriggerSubwayController);

//=============================================
// @brief
//
//=============================================
CTriggerSubwayController::CTriggerSubwayController( edict_t* pedict ):
	CPointEntity(pedict),
	m_destination1Target(NO_STRING_VALUE),
	m_destination2Target(NO_STRING_VALUE),
	m_destination3Target(NO_STRING_VALUE),
	m_destination4Target(NO_STRING_VALUE),
	m_subwayLine(SUBWAYLINE_BERGEN_ECKHART)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerSubwayController::~CTriggerSubwayController( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerSubwayController::DeclareSaveFields( void )
{
	// Call base class to handle it's own
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSubwayController, m_destination1Target, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSubwayController, m_destination2Target, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSubwayController, m_destination3Target, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSubwayController, m_destination4Target, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSubwayController, m_subwayLine, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerSubwayController::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CTriggerSubwayController::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "target1"))
	{
		m_destination1Target = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "target2"))
	{
		m_destination2Target = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "target3"))
	{
		m_destination3Target = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "target4"))
	{
		m_destination4Target = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "type"))
	{
		m_subwayLine = SDL_atoi(kv.value);
		if(m_subwayLine != SUBWAYLINE_BERGEN_ECKHART
			&& m_subwayLine != SUBWAYLINE_KASSAR_STILLWELL)
		{
			gd_engfuncs.pfnCon_Printf("Invalid subway line specified for %s.\n", gd_engfuncs.pfnGetString(m_pFields->targetname));
			m_subwayLine = SUBWAYLINE_BERGEN_ECKHART;
		}
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CTriggerSubwayController::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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

	pEntity->SpawnSubwayWindow((subwayline_t)m_subwayLine, this, HasSpawnFlag(FL_SUBWAY_DUMMY) ? true : false);
}

//=============================================
// @brief
//
//=============================================
void CTriggerSubwayController::FireTarget( CBaseEntity* pPlayer, const Char* pstrdestinationid )
{
	if(!pstrdestinationid)
		return;

	if(!pPlayer)
		return;

	string_t targetentity = NO_STRING_VALUE;
	if(!qstrcmp(pstrdestinationid, BERGEN_ST_DEST_ID))
		targetentity = m_destination1Target;
	else if(!qstrcmp(pstrdestinationid, IBMANN_ST_DEST_ID))
		targetentity = m_destination2Target;
	else if(!qstrcmp(pstrdestinationid, MARSHALL_ST_DEST_ID))
		targetentity = m_destination4Target;
	else if(!qstrcmp(pstrdestinationid, ECKHART_ST_DEST_ID))
		targetentity = m_destination3Target;
	else
	{
		gd_engfuncs.pfnClientPrintf(pPlayer->GetEdict(), "Unknown destination id '%s' specified.\n", pstrdestinationid);
		return;
	}

	if(targetentity == NO_STRING_VALUE)
	{
		gd_engfuncs.pfnClientPrintf(pPlayer->GetEdict(), "No target specified for destination '%s'.\n", pstrdestinationid);
		return;
	}

	// Fire the target
	Util::FireTargets(gd_engfuncs.pfnGetString(targetentity), pPlayer, this, USE_TOGGLE, 0);
}