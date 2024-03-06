/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "ammoglockclip.h"
#include "weaponglock.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(ammo_glock_clip, CAmmoGlockClip);

//=============================================
// @brief
//
//=============================================
CAmmoGlockClip::CAmmoGlockClip( edict_t* pedict ):
	CPlayerAmmo(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CAmmoGlockClip::~CAmmoGlockClip( void )
{
}

//=============================================
// @brief
//
//=============================================
void CAmmoGlockClip::SetSpawnProperties( void )
{
	SetBodyGroup(WMODEL_BODY_BASE, WMODEL_GLOCK_CLIP);
}

//=============================================
// @brief
//
//=============================================
Int32 CAmmoGlockClip::GetAmmoAmount( void )
{
	return CWeaponGlock::WEAPON_MAX_CLIP;
}

//=============================================
// @brief
//
//=============================================
Int32 CAmmoGlockClip::GetMaxAmmo( void )
{
	return MAX_9MM_AMMO;
}

//=============================================
// @brief
//
//=============================================
const Char* CAmmoGlockClip::GetAmmoTypeName( void )
{
	return AMMOTYPE_9MM_NAME;
}