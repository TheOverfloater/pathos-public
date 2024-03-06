/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcladder.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_ladder, CFuncLadder);

//=============================================
// @brief
//
//=============================================
CFuncLadder::CFuncLadder( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncLadder::~CFuncLadder( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncLadder::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname))
		return false;

	m_pState->effects |= (EF_NODRAW|EF_ALWAYS_SEND);
	m_pState->solid = SOLID_NOT;
	m_pState->skin = CONTENTS_LADDER;
	m_pState->movetype = MOVETYPE_PUSH;

	return true;
}
