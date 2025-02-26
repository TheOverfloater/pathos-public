/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MULTIDAMAGE_H
#define MULTIDAMAGE_H

#include "weapons_shared.h"

class CBaseEntity;

/*
====================
CMultiDamage

====================
*/
class CMultiDamage
{
public:
	struct entitydamage_t
	{
		entitydamage_t():
			pentity(nullptr),
			dmgamount(0),
			dmgtype(0),
			hitgroup(0),
			hitcount(0)
			{}

		CBaseEntity* pentity;
		Float dmgamount;
		Int32 dmgtype;
		Int32 hitgroup;
		Uint32 hitcount;
	};

public:
	CMultiDamage( void );
	~CMultiDamage( void );

public:
	void Clear( void );
	void ApplyDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Int32 hitgroup = HITGROUP_GENERIC, Int32 bullettype = BULLET_NONE, Uint32 shotcount = 0 );
	void AddDamage( CBaseEntity* pentity, Float damage, Int32 dmgtype );
	void SetDamageFlags( Int32 dmgtype );

	void SetAttackDirection( const Vector& direction );
	const Vector& GetAttackDirection( void ) const;
	Int32 GetHitGroupForEntity( const CBaseEntity* pEntity );

private:
	// Linked list of affected entities
	CLinkedList<entitydamage_t> m_damagedEntities;

	// Attack direction
	Vector m_attackDirection;
	// Base damage flags
	Int32 m_damageFlags;
};
extern CMultiDamage gMultiDamage;
#endif //MULTIDAMAGE_H