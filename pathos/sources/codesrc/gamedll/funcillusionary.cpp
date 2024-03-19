/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcillusionary.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_illusionary, CFuncIllusionary);

//=============================================
// @brief
//
//=============================================
CFuncIllusionary::CFuncIllusionary( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncIllusionary::~CFuncIllusionary( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncIllusionary::Spawn( void )
{
	if(!HasSpawnFlag(FL_TAKE_ANGLES))
		m_pState->angles = ZERO_VECTOR;
	
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->solid = SOLID_NOT;

	if(m_pFields->targetname == NO_STRING_VALUE)
		m_pState->effects |= EF_STATICENTITY;

	if(!SetModel(m_pFields->modelname))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CFuncIllusionary::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "skin"))
	{
		m_pState->skin = SDL_atoi(kv.value);
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}