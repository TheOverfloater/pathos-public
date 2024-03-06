/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef PLAYERITEM_H
#define PLAYERITEM_H

#include "animatingentity.h"

//=============================================
//
//=============================================
class CPlayerItem : public CAnimatingEntity
{
public:
	enum
	{
		FL_ITEM_NO_NOTICE = (1<<5)
	};

public:
	explicit CPlayerItem( edict_t* pedict );
	virtual ~CPlayerItem( void );

public:
	virtual bool Spawn( void ) override;

public:
	virtual void SetEnableImpactSound( bool enable ) override;
	virtual void SetModel( void );
	virtual void PlayClatterSound( void );
	virtual bool ShouldSetBoundsOnRestore( void ) override { return false; }

public:
	virtual void SetSpawnProperties( void ) = 0;
	virtual bool AddToPlayer( CBaseEntity* pPlayer ) = 0;

public:
	void EXPORTFN DefaultTouch( CBaseEntity* pOther );

protected:
	bool m_playImpactSound;
};
#endif //PLAYERITEM_H