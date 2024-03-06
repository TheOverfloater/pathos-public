/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcdoorrotating.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_door_rotating, CFuncDoorRotating);

//=============================================
// @brief
//
//=============================================
CFuncDoorRotating::CFuncDoorRotating( edict_t* pedict ):
	CFuncDoor(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncDoorRotating::~CFuncDoorRotating( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncDoorRotating::SetSpawnProperties( void )
{
	// Set move direction based on spawnflags
	Util::SetAxisDirection(*m_pState, m_pState->spawnflags, FL_ROTATE_Z, FL_ROTATE_X);

	// Reverse rotation of needed
	if(HasSpawnFlag(FL_ROTATE_REVERSE))
		Math::VectorScale(m_pState->movedir, -1, m_pState->movedir);

	if(HasSpawnFlag(FL_NOT_SOLID))
		m_pState->solid = SOLID_NOT;
	else
		m_pState->solid = SOLID_BSP;
}

//=============================================
// @brief
//
//=============================================
void CFuncDoorRotating::SetMovementVectors( void )
{
	m_angle1 = m_pState->angles;
	m_angle2 = m_pState->angles + m_pState->movedir * m_moveDistance;

	// Swap origins if starting open
	if(HasSpawnFlag(FL_START_OPEN))
	{
		m_pState->angles = m_angle2;
		Vector saveAngle = m_angle2;
		m_angle2 = m_angle1;
		m_angle1 = saveAngle;

		Math::VectorScale(m_pState->movedir, -1, m_pState->movedir);
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncDoorRotating::DoorBeginMoveUp( void )
{
	Float sign = 1.0;
	if(m_activator && (m_activator->IsPlayer() || m_activator->IsNPC()))
	{
		if(!HasSpawnFlag(FL_ONE_WAY) && m_pState->movedir[YAW])
		{
			Vector center = m_pState->origin + (m_pState->mins+m_pState->maxs)*0.5;
			Vector direction = (m_activatorOrigin - center).Normalize();

			// Ignore roll
			direction[ROLL] = 0;

			Vector next = (m_activatorOrigin+(direction*10))-m_pState->origin;
			Float comp = (direction.x*next.y - direction.y*next.x);
			if(comp < 0)
				sign = -1.0;
		}
	}

	AngularMove(m_angle2*sign, m_pState->speed);
}

//=============================================
// @brief
//
//=============================================
void CFuncDoorRotating::DoorBeginMoveDown( void )
{
	AngularMove(m_angle1, m_pState->speed);
}

//=============================================
// @brief
//
//=============================================
void CFuncDoorRotating::RealignRelatedDoor( CFuncDoor* pDoor )
{
	pDoor->SetAngles(m_pState->angles);
	pDoor->SetAngularVelocity(ZERO_VECTOR);
}

//=============================================
// @brief
//
//=============================================
void CFuncDoorRotating::SetToggleState( togglestate_t state, bool reverse )
{
	Vector setAngles;
	if(state == TS_AT_TOP)
		setAngles = m_angle2 * (reverse ? -1 : 1);
	else
		setAngles = m_angle1 * (reverse ? -1 : 1);

	m_pState->angles = setAngles;
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
	m_toggleState = state;
}