/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/
// Code by valina354

#include "includes.h"
#include "gd_includes.h"
#include "envcredits.h"
#include "player.h"

// Default scroll speed
const Float CEnvCredits::DEFAULT_SCROLL_SPEED = 45.0f;

// Link the entity to its class
LINK_ENTITY_TO_CLASS(env_credits, CEnvCredits);

//=============================================
// @brief
//
//=============================================
CEnvCredits::CEnvCredits( edict_t* pedict ):
	CPointEntity(pedict),
	m_scrollSpeed(DEFAULT_SCROLL_SPEED)
{
}

//=============================================
// @brief
//
//=============================================
CEnvCredits::~CEnvCredits( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvCredits::DeclareSaveFields( void )
{
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvCredits, m_scrollSpeed, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CEnvCredits::KeyValue( const keyvalue_t& kv )
{
	if (!qstrcmp(kv.keyname, "scrollspeed"))
	{
		m_scrollSpeed = SDL_atof(kv.value);
		return true;
	}
	else
	{
		return CPointEntity::KeyValue(kv);
	}
}

//=============================================
// @brief
//
//=============================================
bool CEnvCredits::Spawn( void )
{
	if (!CPointEntity::Spawn())
		return false;

	if (m_scrollSpeed <= 0)
		m_scrollSpeed = DEFAULT_SCROLL_SPEED;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvCredits::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.showcredits, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteFloat(m_scrollSpeed);
	gd_engfuncs.pfnUserMessageEnd();
}