/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "infoplayerdeathmatch.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(info_player_deathmatch, CInfoPlayerDeathMatch);

//=============================================
// @brief
//
//=============================================
CInfoPlayerDeathMatch::CInfoPlayerDeathMatch( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CInfoPlayerDeathMatch::~CInfoPlayerDeathMatch( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CInfoPlayerDeathMatch::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "master"))
	{
		m_pFields->message = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CInfoPlayerDeathMatch::IsTriggered( const CBaseEntity* pentity ) const
{
	return Util::IsMasterTriggered(gd_engfuncs.pfnGetString(m_pFields->message), pentity, this);
}
