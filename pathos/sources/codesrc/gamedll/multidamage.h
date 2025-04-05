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
#include "constants.h"

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
			hitcount(0)
		{
			memset(grouphitcounts, 0, sizeof(grouphitcounts));
		}

		CBaseEntity* pentity;
		Float dmgamount;
		Int32 dmgtype;
		Uint32 hitcount;
		
		Uint32 grouphitcounts[NB_HITGROUPS];
	};

public:
	CMultiDamage( void );
	~CMultiDamage( void );

public:
	void Prepare( bullet_types_t bulletType, const Vector& shotOrigin );
	void Prepare( bullet_types_t bulletType, const Vector& shotOrigin, const Vector& shotDirection );
	void ApplyDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker );
	void AddDamage( CBaseEntity* pentity, Float damage, Int32 dmgtype, hitgroups_t hitgroup = HITGROUP_GENERIC );

	void SetDamageFlags( Int32 dmgtype );
	const Vector& GetAttackDirection( void ) const;
	Uint32 GetShotCount( void ) const;
	bullet_types_t GetBulletType( void ) const;
	const Vector& GetShotDirection( void ) const;
	const Vector& GetDamageDirection( void ) const;
	const Vector& GetAttackOrigin( void ) const;

	Uint32 GetEntityHitCount( const CBaseEntity* pEntity );
	Uint32 GetHitGroupHitCountForEntity( const CBaseEntity* pEntity, hitgroups_t hitgroup );
	hitgroups_t GetHitHighestCountGroupForEntity( const CBaseEntity* pEntity );

private:
	// Linked list of affected entities
	CLinkedList<entitydamage_t> m_damagedEntities;

	// Bullet type
	bullet_types_t m_bulletType;
	// Base damage flags
	Int32 m_damageFlags;
	// Number of total shots
	Uint32 m_nbTotalShots;
	// Direction of the shot(aka gun's angle)
	Vector m_shotDirection;

	// Attack origin
	Vector m_attackOrigin;
	// Attack direction for current entity(angle gun was shot at)
	Vector m_attackDirection;
	// Damage direction(dir from attack origin to target center)
	Vector m_damageDirection;
};
extern CMultiDamage gMultiDamage;
#endif //MULTIDAMAGE_H