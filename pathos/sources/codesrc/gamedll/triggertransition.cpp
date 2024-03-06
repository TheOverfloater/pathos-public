/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggertransition.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_transition, CTriggerTransition);

//=============================================
// @brief
//
//=============================================
CTriggerTransition::CTriggerTransition( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerTransition::~CTriggerTransition( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CTriggerTransition::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname, true))
		return false;

	m_pState->effects |= EF_NODRAW;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->solid = SOLID_NOT;

	return true;
}
