/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envelighttrain.h"
#include "animatingentity.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_elighttrain, CEnvELightTrain);

//=============================================
// @brief
//
//=============================================
CEnvELightTrain::CEnvELightTrain( edict_t* pedict ):
	CEnvSpriteTrain(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvELightTrain::~CEnvELightTrain( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvELightTrain::Precache( void )
{
	gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_pFields->modelname));

	CEnvSpriteTrain::Precache();
}

//=============================================
// @brief
//
//=============================================
bool CEnvELightTrain::Spawn( void )
{
	m_pFields->modelname = gd_engfuncs.pfnAllocString(NULL_SPRITE_FILENAME);

	return CEnvSpriteTrain::Spawn();
}

//=============================================
// @brief
//
//=============================================
bool CEnvELightTrain::TrainSetModel( void )
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
void CEnvELightTrain::SetSpawnProperties( void )
{
	// Movetype needs to be MOVETYPE_PUSH so ltime is valid
	m_pState->movetype = MOVETYPE_PUSH;
	m_pState->solid = SOLID_NOT;
	m_pState->rendertype = RT_ENVELIGHT;
}