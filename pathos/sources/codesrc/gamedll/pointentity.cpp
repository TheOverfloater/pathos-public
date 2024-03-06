/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "pointentity.h"

//=============================================
// @brief
//
//=============================================
CPointEntity::CPointEntity( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CPointEntity::~CPointEntity( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CPointEntity::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->effects |= EF_NODRAW;
	return true;
}
