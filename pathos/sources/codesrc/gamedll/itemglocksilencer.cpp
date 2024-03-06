/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "itemglocksilencer.h"
#include "weapons_shared.h"
#include "player.h"
#include "playerweapon.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(item_glock_silencer, CItemGlockSilencer);

//=============================================
// @brief
//
//=============================================
CItemGlockSilencer::CItemGlockSilencer( edict_t* pedict ):
	CPlayerItem(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CItemGlockSilencer::~CItemGlockSilencer( void )
{
}

//=============================================
// @brief
//
//=============================================
void CItemGlockSilencer::SetSpawnProperties( void )
{
	SetBodyGroup(WMODEL_BODY_BASE, WMODEL_SEPARATE_GLOCK_SILENCER);
}

//=============================================
// @brief
//
//=============================================
bool CItemGlockSilencer::AddToPlayer( CBaseEntity* pPlayer )
{
	if(!pPlayer || !pPlayer->IsPlayer())
		return false;

	CPlayerWeapon* pWeapon = pPlayer->GetWeaponList();
	while(pWeapon)
	{
		if(pWeapon->GetId() == WEAPON_GLOCK && !pWeapon->HasSilencer())
		{
			// Play sound
			Util::EmitEntitySound(pPlayer, "items/pickup_weapon.wav", SND_CHAN_VOICE);

			// Send msg
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.huditempickup, nullptr, pPlayer->GetEdict());
				gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_pFields->classname));
			gd_engfuncs.pfnUserMessageEnd();

			// Set the flag to true
			pWeapon->SetHasSilencer(true);
			return true;
		}

		pWeapon = pWeapon->GetNextWeapon();
	}

	return false;
}
