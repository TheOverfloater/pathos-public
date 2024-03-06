/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggersaveweapons.h"
#include "playerweapon.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_saveweapons, CTriggerSaveWeapons);

//=============================================
// @brief
//
//=============================================
CTriggerSaveWeapons::CTriggerSaveWeapons( edict_t* pedict ):
	CPointEntity(pedict),
	m_numWeapons(0),
	m_activeWeaponId(0),
	m_playerHealth(0),
	m_playerArmor(0),
	m_pPlayer(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerSaveWeapons::~CTriggerSaveWeapons( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerSaveWeapons::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CTriggerSaveWeapons, m_weaponInfos, EFIELD_BYTE, sizeof(m_weaponInfos)));
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CTriggerSaveWeapons, m_pWeaponEntities, EFIELD_EHANDLE, MAX_WEAPONS));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSaveWeapons, m_numWeapons, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSaveWeapons, m_activeWeaponId, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSaveWeapons, m_playerHealth, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSaveWeapons, m_playerArmor, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerSaveWeapons, m_pPlayer, EFIELD_ENTPOINTER));
}

//=============================================
// @brief
//
//=============================================
void CTriggerSaveWeapons::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!m_pPlayer)
	{
		// Either use activator, or assume it's local player
		CBaseEntity* pEntity;
		if(pActivator && pActivator->IsPlayer())
			pEntity = pActivator;
		else
			pEntity = Util::GetHostPlayer();

		if(!pEntity || !pEntity->IsPlayer())
		{
			Util::EntityConPrintf(m_pEdict, "Not a player entity.\n");
			return;
		}

		m_pPlayer = pEntity;
	}

	if(m_numWeapons > 0)
	{
		// Verify that the weapon entities still exist
		for(Int32 i = 0; i < m_numWeapons; i++)
		{
			if(!m_pWeaponEntities[i] || (m_pWeaponEntities[i]->GetFlags() & FL_KILLME))
			{
				Util::EntityConDPrintf(m_pEdict, "A weapon entity was deleted.\n");
				return;
			}
		}

		RestoreWeapons();
		m_numWeapons = 0;

		// Save the active weapon
		m_pPlayer->SelectWeapon((weaponid_t)m_activeWeaponId);

		m_pPlayer->SetHealth(m_playerHealth);
		m_pPlayer->SetArmorValue(m_playerArmor);
	}
	else
	{
		SaveWeapons();

		// Switch to previous weapon
		CPlayerWeapon* pWeapon = m_pPlayer->GetActiveWeapon();
		if(pWeapon)
			m_activeWeaponId = pWeapon->GetId();

		m_playerHealth = m_pPlayer->GetHealth();
		m_playerArmor = m_pPlayer->GetArmorValue();
	}
}

//=============================================
// @brief
//
//=============================================
void CTriggerSaveWeapons::SaveWeapons( void )
{
	CPlayerWeapon* pWeapon = m_pPlayer->GetWeaponList();
	if (!pWeapon)
		return;

	while (pWeapon)
	{
		// Save the item
		weaponinfo_t* psave = &m_weaponInfos[m_numWeapons];
		m_pWeaponEntities[m_numWeapons] = pWeapon;
		m_numWeapons++;

		Int32 ammoId = pWeapon->GetAmmoIndex();
		if(ammoId != -1)
			psave->ammo = m_pPlayer->GetAmmoCount(ammoId);

		psave->clip = pWeapon->GetClip();
		psave->leftclip = pWeapon->GetLeftClip();
		psave->rightclip = pWeapon->GetRightClip();
		psave->id = pWeapon->GetId();

		// Go onto the next
		pWeapon = pWeapon->GetNextWeapon();
	}
}

//=============================================
// @brief
//
//=============================================
void CTriggerSaveWeapons::RestoreWeapons( void )
{
	// Remove weapons not on the list
	CPlayerWeapon* pPlayerWeapon = m_pPlayer->GetWeaponList();
	while(pPlayerWeapon)
	{
		Int32 i = 0;
		for(; i < m_numWeapons; i++)
		{
			if(m_pWeaponEntities[i] == reinterpret_cast<const CBaseEntity*>(pPlayerWeapon))
				break;
		}

		// Save this before potentially removing
		CPlayerWeapon* pNext = pPlayerWeapon->GetNextWeapon();

		if(i == m_numWeapons)
			pPlayerWeapon->DropWeapon(false);

		pPlayerWeapon = pNext;
	}

	for(Int32 i = 0; i < m_numWeapons; i++)
	{
		weaponinfo_t& info = m_weaponInfos[i];
		CPlayerWeapon* pWeapon = reinterpret_cast<CPlayerWeapon*>((CBaseEntity*)m_pWeaponEntities[i]);

		// Save the item
		Int32 ammoId = pWeapon->GetAmmoIndex();
		if(ammoId != -1)
			m_pPlayer->SetAmmoCount(ammoId, info.ammo);

		pWeapon->SetClip(info.clip);
		pWeapon->SetLeftClip(info.leftclip);
		pWeapon->SetRightClip(info.rightclip);

		if(!pWeapon->GetPlayer())
		{
			if(m_pPlayer->AddPlayerWeapon(pWeapon))
				pWeapon->AttachToPlayer(m_pPlayer);
		}
	}
}