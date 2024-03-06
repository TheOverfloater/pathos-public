/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envbeverage.h"

// Default number of cans in an env_beverage
const Uint32 CEnvBeverage::DEFAULT_NB_BEVERAGES = 10;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_beverage, CEnvBeverage);

//=============================================
// @brief
//
//=============================================
CEnvBeverage::CEnvBeverage( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvBeverage::~CEnvBeverage( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvBeverage::Precache( void )
{
	Util::PrecacheEntity("item_sodacan");
}

//=============================================
// @brief
//
//=============================================
bool CEnvBeverage::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	m_pState->frags = 0;
	if(!m_pState->health)
		m_pState->health = DEFAULT_NB_BEVERAGES;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvBeverage::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(m_pState->frags != 0 || m_pState->health <= 0)
		return;

	CBaseEntity* pBeverage = CBaseEntity::CreateEntity("item_sodacan", m_pState->origin, m_pState->angles, this);
	if(!pBeverage)
		return;

	if(m_pState->skin == CAN_RANDOM)
		pBeverage->SetSkin(Common::RandomLong(0, CAN_RANDOM-1));
	else
		pBeverage->SetSkin(m_pState->skin);

	// Call to spawn entity
	if(!pBeverage->Spawn())
	{
		Util::RemoveEntity(pBeverage);
		return;
	}

	pBeverage->SetOwner(this);

	m_pState->frags = 1;
	m_pState->health -= 1;
}

//=============================================
// @brief
//
//=============================================
void CEnvBeverage::ChildEntityRemoved( CBaseEntity* pEntity )
{
	m_pState->frags = 0;
}