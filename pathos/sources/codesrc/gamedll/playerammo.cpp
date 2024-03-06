/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "playerammo.h"
#include "player.h"
#include "weapons_shared.h"

//=============================================
// @brief
//
//=============================================
CPlayerAmmo::CPlayerAmmo( edict_t* pedict ):
	CAnimatingEntity(pedict),
	m_playImpactSound(false)
{
}

//=============================================
// @brief
//
//=============================================
CPlayerAmmo::~CPlayerAmmo( void )
{
}

//=============================================
// @brief
//
//=============================================
void CPlayerAmmo::SetEnableImpactSound( bool enable )
{
	m_playImpactSound = true;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerAmmo::Spawn( void )
{
	m_pFields->modelname = gd_engfuncs.pfnAllocString(W_OBJECTS_MODEL_FILENAME);

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
	SetTouch(&CPlayerAmmo::DefaultTouch);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerAmmo::DefaultTouch( CBaseEntity* pOther )
{
	if(!pOther->IsPlayer())
	{
		if(m_playImpactSound)
		{
			Util::PlayWeaponClatterSound(m_pEdict);
			m_playImpactSound = false;
		}

		Util::AlignEntityToSurface(m_pEdict);
		return;
	}

	if(pOther->GiveAmmo(GetAmmoAmount(), GetAmmoTypeName(), GetMaxAmmo(), HasSpawnFlag(FL_AMMO_NO_NOTICE) ? false : true, this) != NO_AMMO_INDEX)
	{
		// Trigger any targets
		if(m_pFields->target != NO_STRING_VALUE)
		{
			UseTargets(pOther, USE_TOGGLE, 0);
			m_pFields->target = NO_STRING_VALUE;
		}

		// Play sound if needed
		if(!HasSpawnFlag(CPlayerAmmo::FL_AMMO_NO_NOTICE))
			Util::EmitEntitySound(pOther, AMMO_PICKUP_SOUND, SND_CHAN_ITEM);

		// Disable touch and remove
		SetTouch(nullptr);
		Util::RemoveEntity(m_pEdict);
	}
}