/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envglobal.h"
#include "globalstate.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_global, CEnvGobal);

//=============================================
// @brief
//
//=============================================
CEnvGobal::CEnvGobal( edict_t* pedict ):
	CPointEntity(pedict),
	m_globalStateName(NO_STRING_VALUE),
	m_triggerMode(0),
	m_initialState(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvGobal::~CEnvGobal( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvGobal::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvGobal, m_globalStateName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvGobal, m_triggerMode, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvGobal, m_initialState, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CEnvGobal::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "globalstate"))
	{
		m_globalStateName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "triggermode"))
	{
		m_triggerMode = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "initialstate"))
	{
		m_initialState = SDL_atoi(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvGobal::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_globalStateName == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(HasSpawnFlag(FL_SET_INITIAL_STATE))
	{
		const Char* pstrGlobalState = gd_engfuncs.pfnGetString(m_globalStateName);
		if(!gGlobalStates.IsGlobalStatePresent(pstrGlobalState))
			SetState(m_initialState);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvGobal::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	SetState(m_triggerMode);
}

//=============================================
// @brief
//
//=============================================
void CEnvGobal::SetState( Int32 newstate )
{
	const Char* pstrGlobalName = gd_engfuncs.pfnGetString(m_globalStateName);
	if(!pstrGlobalName || !qstrlen(pstrGlobalName))
		return;

	globalstate_state_t oldstate = gGlobalStates.GetState(pstrGlobalName);
	globalstate_state_t statetoset;
	switch(newstate)
	{
	case STATE_ON:
		statetoset = GLOBAL_ON;
		break;
	case STATE_DEAD:
		statetoset = GLOBAL_DEAD;
		break;
	case STATE_DELETED:
		statetoset = GLOBAL_DELETED;
		break;
	case STATE_TOGGLE:
		if(oldstate == GLOBAL_ON)
			statetoset = GLOBAL_OFF;
		else if(oldstate == GLOBAL_OFF)
			statetoset = GLOBAL_ON;
		else
			statetoset = oldstate;
		break;
	case STATE_OFF:
	default:
		statetoset = GLOBAL_OFF;
		break;
	}

	gGlobalStates.SetGlobalState(pstrGlobalName, statetoset);
}