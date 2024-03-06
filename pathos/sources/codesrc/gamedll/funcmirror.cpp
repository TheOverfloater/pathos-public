/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcmirror.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_mirror, CFuncMirror);

//=============================================
// @brief
//
//=============================================
CFuncMirror::CFuncMirror( edict_t* pedict ):
	CFuncWall(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncMirror::~CFuncMirror( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncMirror::Spawn( void )
{
	if(!CFuncWall::Spawn())
		return false;

	// Set renderfx for id
	m_pState->rendertype = RT_MIRROR;

	if(HasSpawnFlag(FL_START_OFF))
		m_pState->effects |= EF_NODRAW;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncMirror::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	switch(useMode)
	{
	case USE_ON:
		m_pState->effects &= ~EF_NODRAW;
		break;
	case USE_OFF:
		m_pState->effects |= EF_NODRAW;
		break;
	case USE_TOGGLE:
		{
			if(m_pState->effects & EF_NODRAW)
				m_pState->effects &= ~EF_NODRAW;
			else
				m_pState->effects |= EF_NODRAW;
		}
		break;
	}
}