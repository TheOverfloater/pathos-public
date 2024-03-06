/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ITEMSHOULDERLIGHT_H
#define ITEMSHOULDERLIGHT_H

#include "playeritem.h"

//=============================================
//
//=============================================
class CItemShoulderLight : public CPlayerItem
{
public:
	explicit CItemShoulderLight( edict_t* pedict );
	virtual ~CItemShoulderLight( void );

public:
	virtual void SetSpawnProperties( void ) override;
	virtual bool AddToPlayer( CBaseEntity* pPlayer ) override;
};
#endif //ITEMSHOULDERLIGHT_H