/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envbubbles.h"

// Bubble sprite path
const Char CEnvBubbles::BUBBLE_SPRITE_PATH[] = "sprites/bubble.spr";
// Maximum frequency
const Int32 CEnvBubbles::MAX_FREQUENCY = 20;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_bubbles, CEnvBubbles);

//=============================================
// @brief
//
//=============================================
CEnvBubbles::CEnvBubbles( edict_t* pedict ):
	CBaseEntity(pedict),
	m_density(0),
	m_frequency(0),
	m_bubbleSprite(0),
	m_current(0),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CEnvBubbles::~CEnvBubbles( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvBubbles::DeclareSaveFields( void )
{
	// Call base class to do it first
	CBaseEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBubbles, m_density, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBubbles, m_frequency, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBubbles, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CEnvBubbles::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "density"))
	{
		m_density = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "frequency"))
	{
		m_frequency = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "current"))
	{
		m_current = SDL_atof(kv.value);
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvBubbles::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname))
		return false;

	m_pState->solid = SOLID_NOT;
	m_pState->effects |= EF_NODRAW;

	if(!HasSpawnFlag(FL_START_OFF))
	{
		SetThink(&CEnvBubbles::BubblingThink);
		m_pState->nextthink = g_pGameVars->time + 1.0;
		m_isActive = true;
	}
	else
		m_isActive = false;

	if(m_frequency > MAX_FREQUENCY)
		m_frequency = MAX_FREQUENCY;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvBubbles::Precache( void )
{
	CBaseEntity::Precache();

	m_bubbleSprite = gd_engfuncs.pfnPrecacheModel(BUBBLE_SPRITE_PATH);
	if(m_bubbleSprite == NO_PRECACHE)
	{
		Util::RemoveEntity(this);
		return;
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvBubbles::BubblingThink( void )
{
	Float bubbleHeight = m_pState->absmax[2] - m_pState->absmin[2];

	gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteByte(TE_BUBBLES);
		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteFloat(m_pState->absmin[i]);
		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteFloat(m_pState->absmax[i]);
		gd_engfuncs.pfnMsgWriteSmallFloat(bubbleHeight);
		gd_engfuncs.pfnMsgWriteUint16(m_bubbleSprite);
		gd_engfuncs.pfnMsgWriteUint16(m_density);
		gd_engfuncs.pfnMsgWriteSmallFloat(m_current);
	gd_engfuncs.pfnUserMessageEnd();

	m_pState->nextthink = g_pGameVars->time + 2.5 - (0.1*m_frequency);
}

//=============================================
// @brief
//
//=============================================
void CEnvBubbles::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	switch(useMode)
	{
	case USE_OFF:
		m_isActive = false;
		break;
	case USE_ON:
		m_isActive = true;
		break;
	case USE_TOGGLE:
	default:
		m_isActive = !m_isActive;
		break;
	}

	if(m_isActive)
	{
		SetThink(&CEnvBubbles::BubblingThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
	else
	{
		SetThink(nullptr);
		m_pState->nextthink = 0;
	}
}