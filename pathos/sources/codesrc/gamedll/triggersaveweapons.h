/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERSAVEWEAPONS_H
#define TRIGGERSAVEWEAPONS_H

#include "pointentity.h"
#include "weapons_shared.h"

//=============================================
//
//=============================================
class CTriggerSaveWeapons : public CPointEntity
{
private:
	struct weaponinfo_t
	{
		weaponinfo_t():
			id(0),
			ammo(0),
			clip(0),
			leftclip(0),
			rightclip(0)
			{}

		byte id;
		Int16 ammo;
		Int16 clip;
		Int16 leftclip;
		Int16 rightclip;
	};

public:
	explicit CTriggerSaveWeapons( edict_t* pedict );
	virtual ~CTriggerSaveWeapons( void );

public:
	virtual void DeclareSaveFields( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
private:
	void SaveWeapons( void );
	void RestoreWeapons( void );

private:
	weaponinfo_t m_weaponInfos[MAX_WEAPONS];
	CEntityHandle m_pWeaponEntities[MAX_WEAPONS];
	Int32 m_numWeapons;

	Int32 m_activeWeaponId;
	Int32 m_playerHealth;
	Int32 m_playerArmor;

private:
	class CBaseEntity* m_pPlayer;
};
#endif //TRIGGERSAVEWEAPONS_H