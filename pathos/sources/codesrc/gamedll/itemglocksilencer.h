/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ITEMGLOCKSILENCER_H
#define ITEMGLOCKSILENCER_H

#include "playeritem.h"

//=============================================
//
//=============================================
class CItemGlockSilencer : public CPlayerItem
{
public:
	explicit CItemGlockSilencer( edict_t* pedict );
	virtual ~CItemGlockSilencer( void );

public:
	virtual void SetSpawnProperties( void ) override;
	virtual bool AddToPlayer( CBaseEntity* pPlayer ) override;
};
#endif //ITEMGLOCKSILENCER_H