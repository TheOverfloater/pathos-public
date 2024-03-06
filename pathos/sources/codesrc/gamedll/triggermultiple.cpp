/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggermultiple.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_multiple, CTriggerMultiple);

//=============================================
// @brief
//
//=============================================
CTriggerMultiple::CTriggerMultiple( edict_t* pedict ):
	CTriggerOnce(pedict),
	m_waitTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerMultiple::~CTriggerMultiple( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CTriggerMultiple::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "wait"))
	{
		m_waitTime = SDL_atof(kv.value);
		return true;
	}
	else
		return CTriggerOnce::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CTriggerMultiple::DeclareSaveFields( void )
{
	// Call base class to do it first
	CTriggerEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerMultiple, m_waitTime, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerMultiple::Spawn( void )
{
	if(!CTriggerOnce::Spawn())
		return false;

	// Don't allow zero wait time on trigger_multiple
	if(m_waitTime <= 0)
		m_waitTime = 0.1;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerMultiple::PostTrigger( void )
{
	// Disable touch
	SetTouch(nullptr);

	// Restore after delay
	SetThink(&CTriggerMultiple::RestoreThink);
	m_pState->nextthink = g_pGameVars->time + m_waitTime;
}

//=============================================
// @brief
//
//=============================================
void CTriggerMultiple::RestoreThink( void )
{
	SetTouch(&CTriggerOnce::TriggerTouch);
	SetThink(nullptr);
}

//=============================================
// @brief
//
//=============================================
void CTriggerMultiple::TriggerWait( CBaseEntity* pActivator )
{
	// Check for master
	if(!IsMasterTriggered())
		return;

	PostTrigger();
}

//=============================================
// @brief
//
//=============================================
void CTriggerMultiple::SetTriggerWait( void )
{
	// Check for master
	if(!IsMasterTriggered())
		return;

	PostTrigger();
}