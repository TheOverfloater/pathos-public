/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcparticleblocker.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_particle_blocker, CFuncParticleBlocker);

//=============================================
// @brief
//
//=============================================
CFuncParticleBlocker::CFuncParticleBlocker( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncParticleBlocker::~CFuncParticleBlocker( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncParticleBlocker::Spawn( void )
{
	m_pState->angles = ZERO_VECTOR;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->solid = SOLID_BSP;

	m_pState->flags |= FL_PARTICLE_BLOCKER;

	if(m_pFields->targetname == NO_STRING_VALUE)
		m_pState->effects |= EF_STATICENTITY;

	if(!SetModel(m_pFields->modelname))
		return false;

	return true;
}
