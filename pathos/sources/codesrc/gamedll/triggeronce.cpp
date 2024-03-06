/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggeronce.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_once, CTriggerOnce);

//=============================================
// @brief
//
//=============================================
CTriggerOnce::CTriggerOnce( edict_t* pedict ):
	CTriggerEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerOnce::~CTriggerOnce( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CTriggerOnce::Spawn( void )
{
	if(!CTriggerEntity::Spawn())
		return false;

	// Set touch fn
	SetTouch(&CTriggerOnce::TriggerTouch);
	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerOnce::TriggerTouch( CBaseEntity* pOther )
{
	// Check for master
	if(!IsMasterTriggered())
		return;
	
	// Check for filter flags
	bool noClients = HasSpawnFlag(FL_NO_CLIENTS);
	bool allowNpcs = HasSpawnFlag(FL_ALLOW_NPCS);
	bool allowPushables = HasSpawnFlag(FL_PUSHABLES);
	if(!CheckFilterFlags(pOther, noClients, allowNpcs, allowPushables))
		return;

	// Play sound if needed
	if(m_pFields->noise != NO_STRING_VALUE)
		Util::EmitAmbientSound(GetCenter(), m_pFields->noise);

	// Set activator
	m_activator = pOther;
	
	// Trigger targets
	UseTargets(m_activator, USE_TOGGLE, 0);

	// Show message if needed
	if(m_activator->IsPlayer() && m_triggerMessage != NO_STRING_VALUE)
		Util::ShowMessage(gd_engfuncs.pfnGetString(m_triggerMessage), m_activator);

	// Handle post-trigger functions
	PostTrigger();
}

//=============================================
// @brief
//
//=============================================
void CTriggerOnce::PostTrigger( void )
{
	// Disable touch
	SetTouch(nullptr);

	SetThink(&CBaseEntity::RemoveThink);
	m_pState->nextthink = g_pGameVars->time + 0.1;
}