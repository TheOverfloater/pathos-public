/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcnpcclip.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_npcclip, CFuncNPCClip);

//=============================================
// @brief
//
//=============================================
CFuncNPCClip::CFuncNPCClip( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncNPCClip::~CFuncNPCClip( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncNPCClip::Spawn( void )
{
	m_pState->angles = ZERO_VECTOR;
	m_pState->movetype = MOVETYPE_PUSH;
	m_pState->effects |= EF_NODRAW;
	m_pState->flags |= FL_WORLDBRUSH;

	if(!HasSpawnFlag(FL_START_OFF))
	{
		m_pState->solid = SOLID_BSP;
		m_pState->flags |= FL_NPC_CLIP;
	}
	else
	{
		m_pState->solid = SOLID_NOT;
	}

	if(!SetModel(m_pFields->modelname))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncNPCClip::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(useMode == USE_OFF)
	{
		m_pState->flags &= ~FL_NPC_CLIP;
		m_pState->solid = SOLID_NOT;
	}
	else if(useMode == USE_ON)
	{
		m_pState->flags |= FL_NPC_CLIP;
		m_pState->solid = SOLID_BSP;
	}
	else
	{
		if(m_pState->flags & FL_NPC_CLIP)
		{
			m_pState->flags &= ~FL_NPC_CLIP;
			m_pState->solid = SOLID_NOT;
		}
		else
		{
			m_pState->flags |= FL_NPC_CLIP;
			m_pState->solid = SOLID_BSP;
		}
	}

	// ALWAYS link to world
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
}