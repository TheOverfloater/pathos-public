/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envdlighttrain.h"
#include "animatingentity.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_dlighttrain, CEnvDLightTrain);

//=============================================
// @brief
//
//=============================================
CEnvDLightTrain::CEnvDLightTrain( edict_t* pedict ):
	CEnvSpriteTrain(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvDLightTrain::~CEnvDLightTrain( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvDLightTrain::Precache( void )
{
	gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_pFields->modelname));

	CEnvSpriteTrain::Precache();
}

//=============================================
// @brief
//
//=============================================
bool CEnvDLightTrain::Spawn( void )
{
	m_pFields->modelname = gd_engfuncs.pfnAllocString(NULL_SPRITE_FILENAME);

	return CEnvSpriteTrain::Spawn();
}

//=============================================
// @brief
//
//=============================================
bool CEnvDLightTrain::TrainSetModel( void )
{
	if(!SetModel(m_pFields->modelname, false))
		return false;

	// Reset size to zero
	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, ZERO_VECTOR, ZERO_VECTOR);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvDLightTrain::SetSpawnProperties( void )
{
	// Movetype needs to be MOVETYPE_PUSH so ltime is valid
	m_pState->movetype = MOVETYPE_PUSH;
	m_pState->solid = SOLID_NOT;
	m_pState->rendertype = RT_ENVDLIGHT;
}