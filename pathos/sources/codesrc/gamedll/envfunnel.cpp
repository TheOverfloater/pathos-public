/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envfunnel.h"

// The default sprite model
const Char CEnvFunnel::DEFAULT_SPRITE_NAME[] = "sprites/flare6.spr";

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_funnel, CEnvFunnel);

//=============================================
// @brief
//
//=============================================
CEnvFunnel::CEnvFunnel( edict_t* pedict ):
	CPointEntity(pedict),
	m_spriteName(NO_STRING_VALUE),
	m_spriteModel(NO_PRECACHE)
{
}

//=============================================
// @brief
//
//=============================================
CEnvFunnel::~CEnvFunnel( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvFunnel::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvFunnel, m_spriteName, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
void CEnvFunnel::Precache( void )
{
	if(m_spriteName != NO_STRING_VALUE)
		m_spriteModel = gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_spriteName));
}

//=============================================
// @brief
//
//=============================================
bool CEnvFunnel::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "spritemodel"))
	{
		m_spriteName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvFunnel::Spawn( void )
{
	if(m_spriteName == NO_STRING_VALUE)
		m_spriteName = gd_engfuncs.pfnAllocString(DEFAULT_SPRITE_NAME);

	if(!CPointEntity::Spawn())
		return false;

	if(m_spriteModel == NO_PRECACHE)
	{
		Util::RemoveEntity(this);
		return false;
	}

	if(m_pState->renderamt <= 0)
		m_pState->renderamt = 255;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvFunnel::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteByte(TE_FUNNELSPRITE);
		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteFloat(m_pState->origin[i]);
		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteSmallFloat(m_pState->rendercolor[i]);
		gd_engfuncs.pfnMsgWriteSmallFloat(m_pState->renderamt);
		gd_engfuncs.pfnMsgWriteUint16(m_spriteModel);
		gd_engfuncs.pfnMsgWriteByte(HasSpawnFlag(FL_REVERSE) ? true : false);
	gd_engfuncs.pfnUserMessageEnd();

	if(HasSpawnFlag(FL_REMOVE_ON_FIRE))
	{
		SetThink(&CBaseEntity::RemoveThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
}