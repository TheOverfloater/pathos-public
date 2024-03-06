/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggersound.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_sound, CTriggerSound);

//=============================================
// @brief
//
//=============================================
CTriggerSound::CTriggerSound( edict_t* pedict ):
	CTriggerEntity(pedict),
	m_roomType(0),
	m_masterEntity(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerSound::~CTriggerSound( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerSound::DeclareSaveFields( void )
{
	// Call base class to do it first
	CTriggerEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSound, m_roomType, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerSound::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "roomtype"))
	{
		m_roomType = SDL_atoi(kv.value);
		return true;
	}
	else
		return CTriggerEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CTriggerSound::CallTouch( CBaseEntity* pOther )
{
	if(!IsMasterTriggered())
		return;

	if(pOther->IsPlayer())
		pOther->SetRoomType(m_roomType);
}
