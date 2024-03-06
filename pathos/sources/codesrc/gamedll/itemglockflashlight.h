/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ITEMGLOCKFLASHLIGHT_H
#define ITEMGLOCKFLASHLIGHT_H

#include "playeritem.h"

//=============================================
//
//=============================================
class CItemGlockFlashlight : public CPlayerItem
{
public:
	explicit CItemGlockFlashlight( edict_t* pedict );
	virtual ~CItemGlockFlashlight( void );

public:
	virtual void SetSpawnProperties( void ) override;
	virtual bool AddToPlayer( CBaseEntity* pPlayer ) override;
};
#endif //ITEMGLOCKFLASHLIGHT_H