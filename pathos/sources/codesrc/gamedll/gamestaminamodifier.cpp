/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "gamestaminamodifier.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(game_staminamodifier, CGameStaminaModifier);

//=============================================
// @brief
//
//=============================================
CGameStaminaModifier::CGameStaminaModifier( edict_t* pedict ):
	CPointEntity(pedict),
	m_sprintDrainMultiplier(0),
	m_normalMovemenetDrainFactor(0)
{
}

//=============================================
// @brief
//
//=============================================
CGameStaminaModifier::~CGameStaminaModifier( void )
{
}

//=============================================
// @brief
//
//=============================================
void CGameStaminaModifier::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CGameStaminaModifier, m_sprintDrainMultiplier, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CGameStaminaModifier, m_normalMovemenetDrainFactor, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CGameStaminaModifier::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "sprintmultiplier"))
	{
		m_sprintDrainMultiplier = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "normaldrainfactor"))
	{
		m_normalMovemenetDrainFactor = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CGameStaminaModifier::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	CBaseEntity* pPlayer;
	if(pActivator && pActivator->IsPlayer())
		pPlayer = pActivator;
	else
		pPlayer = Util::GetHostPlayer();

	pPlayer->SetStaminaModifiers(m_sprintDrainMultiplier, m_normalMovemenetDrainFactor);
}
