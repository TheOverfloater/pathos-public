/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ITEMHEALTHKIT_H
#define ITEMHEALTHKIT_H

#include "playeritem.h"

//=============================================
//
//=============================================
class CItemHealthkit : public CPlayerItem
{
public:
	explicit CItemHealthkit( edict_t* pedict );
	virtual ~CItemHealthkit( void );

public:
	virtual void SetSpawnProperties( void ) override;
	virtual bool AddToPlayer( CBaseEntity* pPlayer ) override;
};
#endif //ITEMHEALTHKIT_H