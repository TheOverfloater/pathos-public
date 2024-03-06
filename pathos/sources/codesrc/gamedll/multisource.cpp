/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "multisource.h"
#include "globalstate.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(multisource, CMultiSource);

//=============================================
// @brief
//
//=============================================
CMultiSource::CMultiSource( edict_t* pedict ):
	CPointEntity(pedict),
	m_globalStateName(NO_STRING_VALUE),
	m_isTriggered(false)
{
}

//=============================================
// @brief
//
//=============================================
CMultiSource::~CMultiSource( void )
{
}

//=============================================
// @brief
//
//=============================================
void CMultiSource::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CMultiSource, m_globalStateName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CMultiSource, m_isTriggered, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CMultiSource::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "globalstate"))
	{
		m_globalStateName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CMultiSource::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(HasSpawnFlag(FL_START_ON))
		m_isTriggered = true;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CMultiSource::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	switch(useMode)
	{
	case USE_OFF:
		m_isTriggered = false;
		break;
	case USE_ON:
		m_isTriggered = true;
		break;
	case USE_TOGGLE:
	default:
		m_isTriggered = !m_isTriggered;
		break;
	}

	if(m_isTriggered)
	{
		usemode_t triggerMode = (m_globalStateName != NO_STRING_VALUE) ? USE_ON : USE_TOGGLE;
		UseTargets(nullptr, triggerMode, 0);
	}
}

//=============================================
// @brief
//
//=============================================
bool CMultiSource::IsGlobalEnabled( void ) const
{
	const Char* pstrGlobalName = gd_engfuncs.pfnGetString(m_globalStateName);
	if(!pstrGlobalName || !qstrlen(pstrGlobalName))
		return true;

	globalstate_state_t state = gGlobalStates.GetState(pstrGlobalName);
	if(state == GLOBAL_ON)
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
bool CMultiSource::IsTriggered( const CBaseEntity* pentity ) const
{
	if(m_globalStateName != NO_STRING_VALUE)
		return IsGlobalEnabled();
	else
		return m_isTriggered;
}