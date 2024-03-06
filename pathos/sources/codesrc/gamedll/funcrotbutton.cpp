/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcrotbutton.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_rot_button, CFuncRotButton);

//=============================================
// @brief
//
//=============================================
CFuncRotButton::CFuncRotButton( edict_t* pedict ):
	CFuncButton(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncRotButton::~CFuncRotButton( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncRotButton::SetSpawnProperties( void )
{
	// Set axis of rotation
	Util::SetAxisDirection(*m_pState, m_pState->spawnflags, FL_ROTATE_Z, FL_ROTATE_X);

	// Reverse rotation of needed
	if(HasSpawnFlag(FL_ROTATE_REVERSE))
		Math::VectorScale(m_pState->movedir, -1, m_pState->movedir);

	if(HasSpawnFlag(FL_NOT_SOLID))
		m_pState->solid = SOLID_NOT;
	else
		m_pState->solid = SOLID_BSP;

	m_angle1 = m_pState->angles;
	m_angle2 = m_pState->angles + m_pState->movedir * m_moveDistance;

	if(m_angle2 == m_angle1)
		Util::EntityConPrintf(m_pEdict, "Rotating button start and end angles are equal.\n");
}

//=============================================
// @brief
//
//=============================================
void CFuncRotButton::BeginPressedMovement( void )
{
	AngularMove(m_angle2, m_pState->speed);
}

//=============================================
// @brief
//
//=============================================
void CFuncRotButton::BeginReturnMovement( void )
{
	AngularMove(m_angle1, m_pState->speed);
}