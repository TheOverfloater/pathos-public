/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggergravity.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_gravity, CTriggerGravity);

//=============================================
// @brief
//
//=============================================
CTriggerGravity::CTriggerGravity( edict_t* pedict ):
	CTriggerEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerGravity::~CTriggerGravity( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerGravity::CallTouch( CBaseEntity* pOther )
{
	// Check for master
	if(!IsMasterTriggered())
		return;
	
	if(!pOther->IsPlayer())
		return;

	// Set player gravity
	pOther->SetGravity(m_pState->gravity);
}
