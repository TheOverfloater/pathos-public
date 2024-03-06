/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef PLAYERAMMO_H
#define PLAYERAMMO_H

#include "animatingentity.h"

//=============================================
//
//=============================================
class CPlayerAmmo : public CAnimatingEntity
{
public:
	enum
	{
		FL_AMMO_NO_NOTICE = (1<<5)
	};
public:
	explicit CPlayerAmmo( edict_t* pedict );
	virtual ~CPlayerAmmo( void );

public:
	virtual bool Spawn( void ) override;
	virtual void SetEnableImpactSound( bool enable ) override;
	virtual bool ShouldSetBoundsOnRestore( void ) override { return false; }

public:
	virtual void SetSpawnProperties( void ) = 0;
	virtual Int32 GetAmmoAmount( void ) = 0;
	virtual Int32 GetMaxAmmo( void ) = 0;
	virtual const Char* GetAmmoTypeName( void ) = 0;

public:
	void EXPORTFN DefaultTouch( CBaseEntity* pOther );

private:
	bool m_playImpactSound;
};
#endif //PLAYERAMMO_H