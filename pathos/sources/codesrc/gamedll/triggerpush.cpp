/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerpush.h"

// Default speed
const Float CTriggerPush::DEFAULT_SPEED = 100;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_push, CTriggerPush);

//=============================================
// @brief
//
//=============================================
CTriggerPush::CTriggerPush( edict_t* pedict ):
	CTriggerEntity(pedict),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerPush::~CTriggerPush( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerPush::DeclareSaveFields( void )
{
	// Call base class to do it first
	CTriggerEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerPush, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerPush::Spawn( void )
{
	if(m_pState->angles.IsZero())
		m_pState->angles[YAW] = 360;

	if(!m_pState->angles.IsZero())
		Util::SetMoveDirection(*m_pState);

	if(!CTriggerEntity::Spawn())
		return false;

	if(!m_pState->speed)
		m_pState->speed = DEFAULT_SPEED;

	// Manage if start disabled
	if(!HasSpawnFlag(FL_START_OFF))
		m_isActive = true;
	else
		m_isActive = false;

	// Set friction modifier flag
	if(HasSpawnFlag(FL_FRICTION_MOD))
		m_pState->skin = CONTENT_FRICTIONMOD;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerPush::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	switch(useMode)
	{
	case USE_OFF:
		m_isActive = false;
		break;
	case USE_ON:
		m_isActive = true;
		break;
	case USE_TOGGLE:
	default:
		m_isActive = !m_isActive;
		break;
	}
}

//=============================================
// @brief
//
//=============================================
void CTriggerPush::CallTouch( CBaseEntity* pOther )
{
	// Don't damage if not active
	if(!m_isActive)
		return;

	// Check for master
	if(!IsMasterTriggered())
		return;

	// Check for movetypes
	movetype_t movetype = pOther->GetMoveType();
	if(movetype == MOVETYPE_NONE || movetype == MOVETYPE_PUSH
		|| movetype == MOVETYPE_NOCLIP || movetype == MOVETYPE_FOLLOW)
		return;

	solid_t solidity = pOther->GetSolidity();
	if(solidity == SOLID_NOT || solidity == SOLID_BSP)
		return;

	if(HasSpawnFlag(FL_PUSH_ONCE))
	{
		Vector velocity = pOther->GetVelocity();
		Math::VectorMA(velocity, m_pState->speed, m_pState->movedir, velocity);
		pOther->SetVelocity(velocity);

		if(velocity[2] > 0)
			pOther->RemoveFlags(FL_ONGROUND);

		SetThink(&CBaseEntity::RemoveThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
	else
	{
		Vector vecPush;
		Math::VectorScale(m_pState->movedir, m_pState->speed, vecPush);
		if(pOther->GetFlags() & FL_BASEVELOCITY)
			Math::VectorAdd(vecPush, pOther->GetBaseVelocity(), vecPush);

		pOther->SetBaseVelocity(vecPush);
		pOther->SetFlags(FL_BASEVELOCITY);
	}
}
