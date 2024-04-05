/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcclipeconomy.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_clipeconomy, CFuncClipEconomy);

//=============================================
// @brief
//
//=============================================
CFuncClipEconomy::CFuncClipEconomy( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncClipEconomy::~CFuncClipEconomy( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncClipEconomy::Spawn( void )
{
	if(!HasSpawnFlag(FL_TAKE_ANGLES))
		m_pState->angles = ZERO_VECTOR;

	m_pState->movetype = MOVETYPE_PUSH;
	m_pState->solid = SOLID_BSP;

	if(m_pFields->targetname == NO_STRING_VALUE)
		m_pState->effects |= EF_STATICENTITY;

	if(m_pState->rendermode == RENDER_NORMAL
		|| (m_pState->rendermode & RENDERMODE_BITMASK) == RENDER_TRANSALPHA)
		m_pState->flags |= FL_WORLDBRUSH;

	if(m_pState->renderamt == 0 
		&& (m_pState->rendermode & RENDERMODE_BITMASK) == RENDER_TRANSCOLOR)
	{
		m_pState->rendermode = RENDER_NORMAL;
		m_pState->effects |= EF_COLLISION;
	}

	if(!SetModel(m_pFields->modelname))
		return false;

	return true;
}
