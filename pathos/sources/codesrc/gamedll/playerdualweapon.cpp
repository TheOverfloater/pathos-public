/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "playerdualweapon.h"
#include "player.h"
#include "buttonbits.h"
#include "game.h"

//=============================================
// @brief
//
//=============================================
CPlayerDualWeapon::CPlayerDualWeapon( edict_t* pedict ):
	CPlayerWeapon(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CPlayerDualWeapon::~CPlayerDualWeapon( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CPlayerDualWeapon::AddDuplicate( CPlayerWeapon* pOriginal )
{
	bool hadDual = pOriginal->HasDual();
	if(!hadDual)
		pOriginal->SetDuplicate(true);

	return (ExtractAmmo(pOriginal) || !hadDual) ? true : false;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerDualWeapon::AddFullAmmoDual( CPlayerWeapon* pcheckweapon )
{
	if(m_hasDual)
		return false;

	// Set dual stuff
	m_leftClip = pcheckweapon->GetDefaultAmmo();
	m_hasDual = true;

	// Play notifications if no notice is not set
	if(!pcheckweapon->HasSpawnFlag(CPlayerWeapon::FL_WEAPON_NO_NOTICE))
	{
		// Play sound
		Util::EmitEntitySound(this, AMMO_PICKUP_SOUND, SND_CHAN_ITEM);

		// Tell client
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudweaponpickup, nullptr, m_pPlayer->GetEdict());
			gd_engfuncs.pfnMsgWriteByte(m_weaponId);
		gd_engfuncs.pfnUserMessageEnd();
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerDualWeapon::AddAmmo( Int32 count, const Char* pstrname, Int32 maxclip, Int32 maxcarry, CBaseEntity* pWeapon )
{
	Int32 ammoid;
	if(maxclip < 1)
	{
		m_clip = WEAPON_NO_CLIP;
		ammoid = m_pPlayer->GiveAmmo(count, pstrname, maxcarry, true, pWeapon);
	}
	else if(m_clip == 0)
	{
		Int32 clipgive = m_clip+count;
		if(clipgive > maxclip)
			clipgive = maxclip;
		clipgive -= m_clip;

		if(m_isDuplicate && !m_hasDual)
		{
			m_leftClip += clipgive;
			m_isDuplicate = false;
			m_hasDual = true;

			if(!pWeapon->HasSpawnFlag(CPlayerWeapon::FL_WEAPON_NO_NOTICE))
			{
				// Tell client
				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudweaponpickup, nullptr, m_pPlayer->GetEdict());
					gd_engfuncs.pfnMsgWriteByte(m_weaponId);
				gd_engfuncs.pfnUserMessageEnd();

				// Play sound
				Util::EmitEntitySound(m_pPlayer, WEAPON_PICKUP_SOUND, SND_CHAN_ITEM);
			}
		}
		else
		{
			m_clip += clipgive;
			m_rightClip += clipgive;

			if(m_hasDual)
			{
				Int32 maxClipSingle = (Int32)GetMaxClipSingle();
				Int32 leftgive = (clipgive > maxClipSingle) ? (clipgive - maxClipSingle) : 0;
				m_leftClip += leftgive;

				if(leftgive > 0 && !pWeapon->HasSpawnFlag(CPlayerWeapon::FL_WEAPON_NO_NOTICE))
					Util::EmitEntitySound(m_pPlayer, AMMO_PICKUP_SOUND, SND_CHAN_ITEM);
			}
		
		}

		ammoid = m_pPlayer->GiveAmmo(count-clipgive, pstrname, maxcarry, true, pWeapon);

		// Re-deploy gun
		if(m_isDeployed)
			Deploy();
	}
	else
	{
		if(m_isDuplicate && !m_hasDual)
		{
			m_isDuplicate = false;
			m_hasDual = true;

			m_leftClip += count;
			count = 0;

			if(!pWeapon->HasSpawnFlag(CPlayerWeapon::FL_WEAPON_NO_NOTICE))
			{
				// Tell client
				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudweaponpickup, nullptr, m_pPlayer->GetEdict());
					gd_engfuncs.pfnMsgWriteByte(m_weaponId);
				gd_engfuncs.pfnUserMessageEnd();

				// Play sound
				Util::EmitEntitySound(m_pPlayer, WEAPON_PICKUP_SOUND, SND_CHAN_ITEM);
			}
		}

		// Just give the ammo
		ammoid = m_pPlayer->GiveAmmo(count, pstrname, maxcarry, true, pWeapon);
	}

	// Set ammo type
	if(ammoid > 0)
		m_ammoType = ammoid;

	return (ammoid > 0) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CPlayerDualWeapon::FinishReload( void )
{
	// Manage reload for other clips
	Int32 maxClipSingle = (Int32)GetMaxClipSingle();

	if(m_inDual)
	{
		Int32 rightNeeded = maxClipSingle-m_rightClip;
		Int32 leftNeeded = maxClipSingle-m_leftClip;

		Int32 ammoCount = m_pPlayer->GetAmmoCount(m_ammoType);
		if(rightNeeded > ammoCount)
			rightNeeded = ammoCount;

		if(leftNeeded > (ammoCount-rightNeeded))
			leftNeeded = (ammoCount-rightNeeded);

		m_leftClip += leftNeeded;
		m_rightClip += rightNeeded;
	}
	else
	{
		Int32 rightNeeded = maxClipSingle-m_rightClip;

		Int32 ammoCount = m_pPlayer->GetAmmoCount(m_ammoType);
		if(rightNeeded > ammoCount)
			rightNeeded = ammoCount;

		m_rightClip += rightNeeded;
	}

	// Allow base class to manage the main clip, but
	// only after left and right have been updated,
	// or the count will be wrong
	CPlayerWeapon::FinishReload();
}
