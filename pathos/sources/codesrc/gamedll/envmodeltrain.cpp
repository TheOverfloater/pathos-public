/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envmodeltrain.h"
#include "animatingentity.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_modeltrain, CEnvModelTrain);

//=============================================
// @brief
//
//=============================================
CEnvModelTrain::CEnvModelTrain( edict_t* pedict ):
	CEnvSpriteTrain(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvModelTrain::~CEnvModelTrain( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CEnvModelTrain::TrainSetModel( void )
{
	if(!CEnvSpriteTrain::TrainSetModel())
		return false;

	Vector mins, maxs;
	if(CAnimatingEntity::GetSequenceBox(*m_pState, mins, maxs, false))
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, mins, maxs);

	return true;
}
