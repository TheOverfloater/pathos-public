/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ITEMKEVLAR_H
#define ITEMKEVLAR_H

#include "playeritem.h"

//=============================================
//
//=============================================
class CItemKevlar : public CPlayerItem
{
public:
	explicit CItemKevlar( edict_t* pedict );
	virtual ~CItemKevlar( void );

public:
	virtual void SetSpawnProperties( void ) override;
	virtual bool AddToPlayer( CBaseEntity* pPlayer ) override;
};
#endif //ITEMKEVLAR_H