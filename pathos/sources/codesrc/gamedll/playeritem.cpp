/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "playeritem.h"
#include "player.h"
#include "weapons_shared.h"

//=============================================
// @brief
//
//=============================================
CPlayerItem::CPlayerItem( edict_t* pedict ):
	CAnimatingEntity(pedict),
	m_playImpactSound(false)
{
}

//=============================================
// @brief
//
//=============================================
CPlayerItem::~CPlayerItem( void )
{
}

//=============================================
// @brief
//
//=============================================
void CPlayerItem::SetEnableImpactSound( bool enable )
{
	m_playImpactSound = true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerItem::SetModel( void )
{
	m_pFields->modelname = gd_engfuncs.pfnAllocString(W_OBJECTS_MODEL_FILENAME);
}

//=============================================
// @brief
//
//=============================================
bool CPlayerItem::Spawn( void )
{
	SetModel();

	if(!CAnimatingEntity::Spawn())
		return false;

	m_pState->movetype = MOVETYPE_TOSS;
	m_pState->forcehull = HULL_POINT;
	m_pState->solid = SOLID_TRIGGER;

	// Set collision box
	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, ITEM_HULL_MIN, ITEM_HULL_MAX);

	// Set glow and color of glow
	m_pState->renderfx = RenderFx_GlowAura;
	m_pState->rendercolor = WEAPON_GLOW_COLOR;

	// Set body
	SetSpawnProperties();

	// Set touch function
	SetTouch(&CPlayerItem::DefaultTouch);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerItem::DefaultTouch( CBaseEntity* pOther )
{
	if(!pOther->IsPlayer())
	{
		if(m_playImpactSound)
		{
			PlayClatterSound();
			m_playImpactSound = false;
		}

		Util::AlignEntityToSurface(m_pEdict);
		return;
	}

	if(AddToPlayer(pOther))
	{
		// Trigger any targets
		if(m_pFields->target != NO_STRING_VALUE)
		{
			UseTargets(pOther, USE_TOGGLE, 0);
			m_pFields->target = NO_STRING_VALUE;
		}

		SetTouch(nullptr);
		Util::RemoveEntity(m_pEdict);
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerItem::PlayClatterSound( void )
{
	Util::PlayWeaponClatterSound(m_pEdict);
}