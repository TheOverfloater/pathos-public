/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "infonodeair.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(info_node_air, CInfoNodeAir);

//=============================================
// @brief
//
//=============================================
CInfoNodeAir::CInfoNodeAir( edict_t* pedict ):
	CInfoNode(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CInfoNodeAir::~CInfoNodeAir( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CInfoNodeAir::IsAirNode( void )
{
	return true;
}
