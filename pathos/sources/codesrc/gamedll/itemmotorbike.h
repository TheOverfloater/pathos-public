/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ITEMMOTORBIKE_H
#define ITEMMOTORBIKE_H

#include "animatingentity.h"

//=============================================
//
//=============================================
class CItemMotorBike : public CAnimatingEntity
{
public:
	// Trashed bike animation name
	static const Char BIKE_ANIMATION_TRASHED_NAME[];
	// Enter bike animation name
	static const Char BIKE_ANIMATION_ENTER_NAME[];
	// Exit bike animation name
	static const Char BIKE_ANIMATION_LEAVE_NAME[];
	// Motorbike model name
	static const Char BIKE_MODELNAME[];

public:
	enum
	{
		FL_LEAVE_DIALOUGE = (1<<0)
	};

public:
	explicit CItemMotorBike( edict_t* pedict );
	virtual ~CItemMotorBike( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual void TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags ) override;
	virtual bool Restore( void ) override;
	virtual Int32 GetEntityFlags( void ) override { return (CAnimatingEntity::GetEntityFlags() | FL_ENTITY_PLAYER_USABLE | FL_ENTITY_TRANSITION); }
	virtual bool ShouldSetBoundsOnRestore( void ) override { return false; }

public:
	void EXPORTFN UseBike( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value );

	void PlayerEnter( CBaseEntity* pEntity );
	void PlayerLeave( void );
	void SetFollow( void );

	Double GetLeaveTime( void );
	Double GetEnterTime( void );

public:
	// TRUE if trashed
	bool m_isTrashed;
	// Player using this bike
	CBaseEntity* m_pPlayer;
};

#endif //ITEMMOTORBIKE_H