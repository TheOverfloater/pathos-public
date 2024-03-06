/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerauto.h"
#include "globalstate.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_auto, CTriggerAuto);

//=============================================
// @brief
//
//=============================================
CTriggerAuto::CTriggerAuto( edict_t* pedict ):
	CDelayEntity(pedict),
	m_globalState(NO_STRING_VALUE),
	m_triggerMode(USE_OFF),
	m_p1GlobalState(NO_STRING_VALUE),
	m_p1Target(NO_STRING_VALUE),
	m_p2GlobalState(NO_STRING_VALUE),
	m_p2Target(NO_STRING_VALUE),
	m_p3GlobalState(NO_STRING_VALUE),
	m_p3Target(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerAuto::~CTriggerAuto( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerAuto::DeclareSaveFields( void )
{
	// Call base class to do it first
	CDelayEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerAuto, m_globalState, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerAuto, m_triggerMode, EFIELD_INT32));

	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerAuto, m_p1GlobalState, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerAuto, m_p1Target, EFIELD_STRING));

	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerAuto, m_p2GlobalState, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerAuto, m_p2Target, EFIELD_STRING));

	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerAuto, m_p3GlobalState, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerAuto, m_p3Target, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerAuto::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "globalstate"))
	{
		m_globalState = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "p1globalstate"))
	{
		m_p1GlobalState = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "p1target"))
	{
		m_p1Target = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "p2globalstate"))
	{
		m_p2GlobalState = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "p2target"))
	{
		m_p2Target = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "p3globalstate"))
	{
		m_p3GlobalState = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "p3target"))
	{
		m_p3Target = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "triggerstate"))
	{
		Int32 state = SDL_atoi(kv.value);
		switch(state)
		{
		case 0:
			m_triggerMode = USE_OFF;
			break;
		case 2:
			m_triggerMode = USE_TOGGLE;
			break;
		case 1:
		default:
			m_triggerMode = USE_ON;
			break;
		}
		return true;
	}
	else
		return CDelayEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerAuto::Spawn( void )
{
	if(!CDelayEntity::Spawn())
		return false;

	m_pState->flags |= FL_INITIALIZE;
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CTriggerAuto::Restore( void )
{
	if(!CDelayEntity::Restore())
		return false;

	m_pState->flags |= FL_INITIALIZE;
	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerAuto::InitEntity( void )
{
	if(m_pFields->target != NO_STRING_VALUE && (m_globalState == NO_STRING_VALUE || gGlobalStates.GetState(gd_engfuncs.pfnGetString(m_globalState)) == GLOBAL_ON))
	{
		UseTargets(this, (usemode_t)m_triggerMode, 0);
		if(HasSpawnFlag(FL_REMOVE_ON_FIRE))
			Util::RemoveEntity(this);
	}

	if(m_p3Target != NO_STRING_VALUE && (m_p3GlobalState == NO_STRING_VALUE || gGlobalStates.GetState(gd_engfuncs.pfnGetString(m_p3GlobalState)) == GLOBAL_ON))
	{
		UseTargets(this, (usemode_t)m_triggerMode, 0, m_p3Target);
		
		m_p3GlobalState = NO_STRING_VALUE;
		m_p3Target = NO_STRING_VALUE;

		m_p2GlobalState = NO_STRING_VALUE;
		m_p2Target = NO_STRING_VALUE;

		m_p1GlobalState = NO_STRING_VALUE;
		m_p1Target = NO_STRING_VALUE;
	}
	else if(m_p2Target != NO_STRING_VALUE && (m_p2GlobalState == NO_STRING_VALUE || gGlobalStates.GetState(gd_engfuncs.pfnGetString(m_p2GlobalState)) == GLOBAL_ON))
	{
		UseTargets(this, (usemode_t)m_triggerMode, 0, m_p2Target);

		m_p2GlobalState = NO_STRING_VALUE;
		m_p2Target = NO_STRING_VALUE;

		m_p1GlobalState = NO_STRING_VALUE;
		m_p1Target = NO_STRING_VALUE;
	}
	else if(m_p1Target != NO_STRING_VALUE && (m_p1GlobalState == NO_STRING_VALUE || gGlobalStates.GetState(gd_engfuncs.pfnGetString(m_p1GlobalState)) == GLOBAL_ON))
	{
		UseTargets(this, (usemode_t)m_triggerMode, 0, m_p1Target);

		m_p1GlobalState = NO_STRING_VALUE;
		m_p1Target = NO_STRING_VALUE;
	}
}
