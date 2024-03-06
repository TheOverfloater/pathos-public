
/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggersubwayflagger.h"
#include "player.h"
#include "gameui_shared.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_subway_flagger, CTriggerSubwayFlagger);

//=============================================
// @brief
//
//=============================================
CTriggerSubwayFlagger::CTriggerSubwayFlagger( edict_t* pedict ):
	CPointEntity(pedict),
	m_flag(0)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerSubwayFlagger::~CTriggerSubwayFlagger( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerSubwayFlagger::DeclareSaveFields( void )
{
	// Call base class to handle it's own
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSubwayFlagger, m_flag, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerSubwayFlagger::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "swflag"))
	{
		m_flag = SDL_atoi(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerSubwayFlagger::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerSubwayFlagger::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Either use activator, or assume it's local player
	CBaseEntity* pEntity;
	if(pActivator && pActivator->IsPlayer())
		pEntity = pActivator;
	else
		pEntity = Util::GetHostPlayer();

	// Safely get player pointer
	if(!pEntity || !pEntity->IsPlayer())
	{
		Util::EntityConPrintf(m_pEdict, "Not a player entity.\n");
		return;
	}

	Int32 flag = FL_SUBWAY_NONE;
	switch(m_flag)
	{
	case SW_FLAG_BERGEN_ST:
		flag = FL_SUBWAY_GOT_BERGENST;
		break;
	case SW_FLAG_IBMANN_ST:
		flag = FL_SUBWAY_GOT_IBMANNST;
		break;
	case SW_FLAG_ECKHART_ST:
		flag = FL_SUBWAY_GOT_ECKHARTST;
		break;
	case SW_FLAG_MARSHALL_ST:
		flag = FL_SUBWAY_GOT_MARSHALLST;
		break;
	case SW_FLAG_HACKED:
		flag = FL_SUBWAY_DISABLED;
		break;
	default:
		gd_engfuncs.pfnClientPrintf(pEntity->GetEdict(), "Unknown subway flag %d.\n", m_flag);
	}

	// Set the flag
	pEntity->SetSubwayFlag(flag);

	// Won't be used anymore
	Util::RemoveEntity(this);
}
