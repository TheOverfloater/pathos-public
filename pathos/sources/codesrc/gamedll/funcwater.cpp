/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcwater.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_water, CFuncWater);

//=============================================
// @brief
//
//=============================================
CFuncWater::CFuncWater( edict_t* pedict ):
	CFuncDoor(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncWater::~CFuncWater( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncWater::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "skin"))
	{
		m_pState->skin = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "WaveHeight"))
	{
		m_pState->scale = SDL_atof(kv.value);
		return true;
	}
	else
		return CFuncDoor::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CFuncWater::SetSpawnProperties( void )
{
	// Set move direction
	Util::SetMoveDirection(*m_pState);

	if(m_pState->renderfx == RenderFx_SkyEnt)
	{
		m_pState->rendertype = RT_SKYWATERENT;
		m_pState->renderfx = RenderFx_None;
	}
	else
		m_pState->rendertype = RT_WATERSHADER;

	m_pState->solid = SOLID_NOT;
	m_isSilent = true;
}