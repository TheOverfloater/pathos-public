/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "nullentity.h"

//=============================================
// @brief
//
//=============================================
CNullEntity::CNullEntity( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CNullEntity::~CNullEntity( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CNullEntity::Spawn( void )
{
	Util::RemoveEntity(this);
	return true;
}
