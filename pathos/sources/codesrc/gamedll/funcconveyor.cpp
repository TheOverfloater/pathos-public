/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcconveyor.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_conveyor, CFuncConveyor);

//=============================================
// @brief
//
//=============================================
CFuncConveyor::CFuncConveyor( edict_t* pedict ):
	CFuncWall(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncConveyor::~CFuncConveyor( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncConveyor::Spawn( void )
{
	// Set movement direction
	Util::SetMoveDirection(*m_pState);

	if(!CFuncWall::Spawn())
		return false;

	if(!HasSpawnFlag(FL_VISUAL_ONLY))
		m_pState->flags |= FL_CONVEYOR;

	if(!HasSpawnFlag(FL_INVISIBLE))
		m_pState->effects |= EF_CONVEYOR;
	else
		m_pState->effects |= EF_NODRAW;
	
	if(HasSpawnFlag(FL_NOT_SOLID))
	{
		m_pState->solid = SOLID_NOT;
		m_pState->skin = 0;
	}
	else
		m_pState->skin = CONTENTS_CONVEYOR;

	// Put speed in scale
	m_pState->scale = m_pState->speed;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncConveyor::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(HasSpawnFlag(FL_INVISIBLE))
	{
		m_pState->effects &= ~EF_NODRAW;
		m_pState->effects |= EF_CONVEYOR;
		return;
	}
	else
	{
		m_pState->speed = -m_pState->speed;
	}
}
