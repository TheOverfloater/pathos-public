/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcbikeblock.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_bike_block, CFuncBikeBlock);

//=============================================
// @brief
//
//=============================================
CFuncBikeBlock::CFuncBikeBlock( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncBikeBlock::~CFuncBikeBlock( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncBikeBlock::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	m_pState->angles.Clear();
	m_pState->solid = SOLID_BSP;
	m_pState->movetype = MOVETYPE_PUSH;
	m_pState->effects |= EF_NODRAW;

	if(!SetModel(m_pFields->modelname))
		return false;

	m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CFuncBikeBlock::Restore( void )
{
	m_pState->flags |= FL_INITIALIZE;

	return CBaseEntity::Restore();
}

//=============================================
// @brief
//
//=============================================
void CFuncBikeBlock::SetBikeBlock( bool enable )
{
	if(enable && m_pState->solid == SOLID_NOT)
	{
		m_pState->solid = SOLID_BSP;
		m_pState->movetype = MOVETYPE_PUSH;
	}
	else if(!enable && m_pState->solid == SOLID_BSP)
	{
		m_pState->solid = SOLID_NOT;
		m_pState->movetype = MOVETYPE_NONE;
	}

	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
}

//=============================================
// @brief
//
//=============================================
void CFuncBikeBlock::InitEntity( void )
{
	CBaseEntity* pPlayer = Util::GetHostPlayer();
	if(!pPlayer)
		return;

	if(pPlayer->IsOnMotorBike())
		SetBikeBlock(true);
	else
		SetBikeBlock(false);
}
