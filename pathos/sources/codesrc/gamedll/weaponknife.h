/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef WEAPONKNIFE_H
#define WEAPONKNIFE_H

#include "playerweapon.h"

//=============================================
//
//=============================================
class CWeaponKnife : public CPlayerWeapon
{
public:
	enum weapon_anims_t
	{
		KNIFE_IDLE1 = 0,
		KNIFE_IDLE2,
		KNIFE_SWING_START,
		KNIFE_SWING_LEFT,
		KNIFE_SWING_LEFT_HIT_NEAR,
		KNIFE_SWING_LEFT_HIT_FAR,
		KNIFE_SWING_RIGHT,
		KNIFE_SWING_RIGHT_HIT_NEAR,
		KNIFE_SWING_RIGHT_HIT_FAR,
		KNIFE_RETURN_LEFT,
		KNIFE_RETURN_RIGHT,
		KNIFE_DRAW,
		KNIFE_HOLSTER,

		// Must be last
		NUM_WEAPON_ANIMATIONS
	};

	enum knife_swingstate_t
	{
		STATE_NONE = 0,
		STATE_FIRST_SWING,
		STATE_SWING_LEFT,
		STATE_SWING_RIGHT,
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

public:
	explicit CWeaponKnife( edict_t* pedict );
	virtual ~CWeaponKnife( void );

public:
	virtual void SetSpawnProperties( void ) override;
	virtual void Precache( void ) override;
	
	virtual bool GetWeaponInfo( weaponinfo_t* pWeapon ) override;

	virtual bool Deploy( void ) override;
	virtual void Holster( void ) override;

	virtual void PrimaryAttack( void ) override;
	virtual void Idle( void ) override;

	virtual void PostThink( void ) override;

private:
	void Swing( void );

private:
	Double m_impactTime;
	Double m_timeNext;
	
	knife_swingstate_t m_attackState;
	bool m_knifeHit;

private:
	// Sequence names for weapon
	static const Char* m_sequenceNames[NUM_WEAPON_ANIMATIONS];
};
#endif //WEAPONKNIFE_H