/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envspritetrain.h"

// env_elight renderfx value
const Int32 CEnvSpriteTrain::ELIGHT_RENDERFX_VALUE = 69;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_spritetrain, CEnvSpriteTrain);

//=============================================
// @brief
//
//=============================================
CEnvSpriteTrain::CEnvSpriteTrain( edict_t* pedict ):
	CFuncTrain(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSpriteTrain::~CEnvSpriteTrain( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvSpriteTrain::Precache( void )
{
	gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_pFields->modelname));

	CFuncTrain::Precache();
}

//=============================================
// @brief
//
//=============================================
void CEnvSpriteTrain::SetSpawnProperties( void )
{
	// Movetype needs to be MOVETYPE_PUSH so ltime is valid
	m_pState->movetype = MOVETYPE_PUSH;
	m_pState->solid = SOLID_NOT;

	// Hack: Create a new env_elighttrain entity
	if(m_pState->renderfx == ELIGHT_RENDERFX_VALUE)
		m_pState->rendertype = RT_ENVELIGHT;
}

//=============================================
// @brief
//
//=============================================
Vector CEnvSpriteTrain::GetDestinationVector( const Vector& destOrigin )
{
	// env_spritetrain only uses the basic origin
	return destOrigin;
}
