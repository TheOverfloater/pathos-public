/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ITEMSECURITY_H
#define ITEMSECURITY_H

#include "playeritem.h"

//=============================================
//
//=============================================
class CItemSecurity : public CPlayerItem
{
public:
	explicit CItemSecurity( edict_t* pedict );
	virtual ~CItemSecurity( void );

public:
	virtual void SetSpawnProperties( void ) override;
	virtual bool AddToPlayer( CBaseEntity* pPlayer ) override;
};
#endif //ITEMSECURITY_H