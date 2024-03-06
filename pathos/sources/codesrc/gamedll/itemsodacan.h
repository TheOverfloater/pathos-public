/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ITEMSODACAN_H
#define ITEMSODACAN_H

#include "playeritem.h"

//=============================================
//
//=============================================
class CItemSodaCan : public CPlayerItem
{
public:
	// Sodacan model
	static const Char SODACAN_MODEL_FILENAME[];

public:
	explicit CItemSodaCan( edict_t* pedict );
	virtual ~CItemSodaCan( void );

public:
	virtual void Precache( void ) override;
	virtual void SetSpawnProperties( void ) override;
	virtual bool AddToPlayer( CBaseEntity* pPlayer ) override;
	virtual void SetModel( void ) override;
	virtual void PlayClatterSound( void ) override;
};
#endif //ITEMSODACAN_H