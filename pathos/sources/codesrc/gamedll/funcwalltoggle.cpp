/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcwalltoggle.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_wall_toggle, CFuncWallToggle);

//=============================================
// @brief
//
//=============================================
CFuncWallToggle::CFuncWallToggle( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncWallToggle::~CFuncWallToggle( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncWallToggle::Spawn( void )
{
	m_pState->movetype = MOVETYPE_PUSH;

	if(m_pFields->targetname == NO_STRING_VALUE)
		m_pState->effects |= EF_STATICENTITY;

	if(m_pState->renderamt == 0 
		&& (m_pState->rendermode & RENDERMODE_BITMASK) == RENDER_TRANSCOLOR)
	{
		m_pState->rendermode = RENDER_NORMAL;
		m_pState->effects |= EF_COLLISION;
	}

	if(HasSpawnFlag(FL_NOT_SOLID))
		m_pState->solid = SOLID_NOT;
	else
		m_pState->solid = SOLID_BSP;

	if(HasSpawnFlag(FL_START_OFF))
		TurnOff();

	if(HasSpawnFlag(FL_ALWAYS_INVISIBLE))
		m_pState->effects |= EF_NODRAW;

	if(!SetModel(m_pFields->modelname))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncWallToggle::TurnOff( void )
{
	if(!HasSpawnFlag(FL_NOT_SOLID))
		m_pState->solid = SOLID_NOT;

	if(!HasSpawnFlag(FL_ALWAYS_INVISIBLE))
		m_pState->effects |= EF_NODRAW;

	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
}

//=============================================
// @brief
//
//=============================================
void CFuncWallToggle::TurnOn( void )
{
	if(!HasSpawnFlag(FL_NOT_SOLID))
		m_pState->solid = SOLID_BSP;

	if(!HasSpawnFlag(FL_ALWAYS_INVISIBLE))
		m_pState->effects &= ~EF_NODRAW;

	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
}

//=============================================
// @brief
//
//=============================================
bool CFuncWallToggle::IsOn( void ) const
{
	if(!HasSpawnFlag(FL_ALWAYS_INVISIBLE) && (m_pState->effects & EF_NODRAW)
		|| HasSpawnFlag(FL_ALWAYS_INVISIBLE) && (m_pState->solid == SOLID_NOT))
		return false;
	else
		return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncWallToggle::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool state = IsOn();
	if(ShouldToggle(useMode, state))
	{
		if(state)
			TurnOff();
		else
			TurnOn();
	}
}