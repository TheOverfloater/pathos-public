/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef WEAPONGLOCK_H
#define WEAPONGLOCK_H

#include "playerweapon.h"

//=============================================
//
//=============================================
class CWeaponGlock : public CPlayerWeapon
{
public:
	enum weapon_anims_t
	{
		GLOCK_IDLE1 = 0,
		GLOCK_IDLE1_EMPTY,
		GLOCK_IDLE2,
		GLOCK_IDLE2_EMPTY,
		GLOCK_SHOOT,
		GLOCK_SHOOT_EMPTY,
		GLOCK_SHOOT_SILENCED,
		GLOCK_SHOOT_EMPTY_SILENCED,
		GLOCK_RELOAD,
		GLOCK_RELOAD_NOT_EMPTY,
		GLOCK_DEPLOY,
		GLOCK_DEPLOY_EMPTY,
		GLOCK_DEPLOY_FIRST,
		GLOCK_HOLSTER,
		GLOCK_HOLSTER_EMPTY,
		GLOCK_FLASHLIGHT_TOGGLE,
		GLOCK_FLASHLIGHT_TOGGLE_EMPTY,
		GLOCK_ADD_FLASHLIGHT,
		GLOCK_ADD_FLASHLIGHT_EMPTY,
		GLOCK_ADD_SILENCER,
		GLOCK_ADD_SILENCER_EMPTY,
		GLOCK_REMOVE_SILENCER,
		GLOCK_REMOVE_SILENCER_EMPTY,

		// Must be last
		NUM_WEAPON_ANIMATIONS
	};

public:
	// Weapon view model
	static const Char WEAPON_VIEWMODEL[];
	// Weapon weight
	static const Int32 WEAPON_WEIGHT;
	// Weapon slot
	static const Int32 WEAPON_SLOT;
	// Weapon slot position
	static const Int32 WEAPON_SLOT_POSITION;
	// Default ammo for weapon
	static const Uint32 WEAPON_DEFAULT_GIVE;
	// Max clip capacity for weapon
	static const Uint32 WEAPON_MAX_CLIP;
	// Weapon cone id
	static const Uint32 WEAPON_CONE_ID;
	// Silencer item name
	static const Char SILENCER_ENTITY_NAME[];
	// Flashlight item name
	static const Char FLASHLIGHT_ENTITY_NAME[];

	// w_ model groups
	static const Uint32 WGLOCK_SL_GROUP_INDEX;
	static const Uint32 WGLOCK_SL_OFF;
	static const Uint32 WGLOCK_SL_ON;
		
	static const Uint32 WGLOCK_FL_GROUP_INDEX;
	static const Uint32 WGLOCK_FL_OFF;
	static const Uint32 WGLOCK_FL_ON;

	// v_ model groups
	static const Uint32 VGLOCK_SL_GROUP_INDEX;
	static const Uint32 VGLOCK_SL_OFF;
	static const Uint32 VGLOCK_SL_ON;

	static const Uint32 VGLOCK_FL_GROUP_INDEX;
	static const Uint32 VGLOCK_FL_OFF;
	static const Uint32 VGLOCK_FL_ON;

	// Recoil degrade speed
	static const Float WEAPON_RECOIL_DEGRADE;

public:
	explicit CWeaponGlock( edict_t* pedict );
	virtual ~CWeaponGlock( void );

public:
	virtual void SetSpawnProperties( void ) override;
	virtual void Precache( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void DeclareSaveFields( void ) override;

	virtual bool GetWeaponInfo( weaponinfo_t* pWeapon ) override;

	virtual bool Deploy( void ) override;
	virtual void Holster( void ) override;

	virtual void PrimaryAttack( void ) override;
	virtual void SecondaryAttack( void ) override;
	virtual void WeaponSpecialFunction( void ) override;
	virtual void Idle( void ) override;
	virtual void Reload( void ) override;

	virtual void PostThink( void ) override;

	virtual void FlashlightToggle( bool drain = false ) override;
	virtual void SetFlashlightRequest( bool requested ) override;

	virtual Float GetRecoilDegradeFactor( void ) override;

	virtual bool HasSilencer( void ) override;
	virtual void SetHasSilencer( bool hasSilencer ) override;
	virtual bool HasFlashlight( void ) override;
	virtual void SetHasFlashlight( bool hasFlashlight ) override;
	virtual bool CanResetAttackTime( void ) override;
	virtual bool AddDuplicate( CPlayerWeapon* poriginal ) override;
	virtual void AddAccessories( CPlayerWeapon* pWeapon ) override;

private:
	bool m_hasSilencer;
	bool m_isSilenced;
	bool m_hasFlashlight;
	bool m_isFlashlightEquipped;
	bool m_isFlashlightActive;
	bool m_isFlashlightRequested;

	Double m_toggleTime;
	Double m_silencerTime;
	Double m_fireRateDelayTime;

private:
	// Sequence names for weapon
	static const Char* m_sequenceNames[NUM_WEAPON_ANIMATIONS];
};
#endif //WEAPONGLOCK_H