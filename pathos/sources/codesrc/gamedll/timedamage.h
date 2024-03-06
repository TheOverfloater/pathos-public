/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TIMEDAMAGE_H
#define TIMEDAMAGE_H

#include "animatingentity.h"

//=============================================
//
//=============================================
class CTimeDamage : public CBaseEntity
{
public:
	enum damage_type_t
	{
		TD_BBOX = 0,
		TD_FOLLOW_ENTITY,
		TD_BBOX_FOLLOW_ENTITY
	};

public:
	explicit CTimeDamage( edict_t* pedict );
	virtual ~CTimeDamage( void );

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Declares save-restore fields
	virtual void DeclareSaveFields( void ) override;
	// Calld when aiment is freed
	virtual void OnAimentFreed( void ) override;
	// Returns entity's flags
	virtual Int32 GetEntityFlags( void ) override;
	// Called when the entity is freed
	virtual void FreeEntity( void ) override;

public:
	// Think function for follow-type damage infliction
	void EXPORTFN DamageThink( void );

public:
	// Creates bbox-based timedamage entity
	static CTimeDamage* CreateTimeDamageBox( CBaseEntity* pInflictor, const Vector& origin, const Vector& mins, const Vector& maxs, Int32 dmgTypeFlags, Float dmgDelay, Float dmgAmount, Float life, CBaseEntity* pFollow = nullptr );
	// Creates timedamage entity that follows another entity
	static CTimeDamage* CreateTimeDamageFollowEntity( CBaseEntity* pInflictor, CBaseEntity* pTarget, Int32 dmgTypeFlags, Int32 hitgroup, Float dmgDelay, Float dmgAmount, Float life );

public:
	// Set the type
	void SetType( damage_type_t type );
	// Set the damage delay
	void SetDamageDelay( Float delay );
	// Set damage type flags
	void SetDamageTypeFlags( Int32 dmgFlags );
	// Set lifetime of entity
	void SetLifetime( Float life );
	// Set damage amount
	void SetDamageAmount( Float dmg );
	// Set attacker entity
	void SetAttacker( CBaseEntity* pAttacker );
	// Set hurt target entity
	void SetHurtTarget( CBaseEntity* pTarget );
	// Set hitgroup
	void SetHitgroup( Int32 hitgroup );

	// Get the type
	damage_type_t GetDmgType( void );

public:
	// Adds entity to the list
	static void AddTimeDamageToList( CTimeDamage* pDamage );
	// Removes entity from list
	static void RemoveTimeDamageFromList( const CTimeDamage* pDamage );
	// Clears all timedamage entities from list
	static void ClearTimeDamageList( void );

	// Tells if there's another timedamage with type tied to an entity
	static bool EntityHasTimeDamageTied( const CBaseEntity* pEntity, damage_type_t type );
	// Tells if there's a timedamage in the bbox
	static bool HasTimeDamageInBBox( const Vector& mins, const Vector& maxs, damage_type_t type );

private:
	// Damage type
	Int32 m_dmgType;
	// Damage delay
	Float m_dmgDelay;
	// Damage type flags
	Int32 m_dmgTypeFlags;
	// Lifetime of entity
	Float m_lifetime;
	// Damage amount
	Float m_dmgAmount;
	// Hitgroup
	Int32 m_hitgroup;

	// Next damage time
	Double m_nextDmgTime;
	// Death time
	Double m_deathTime;

	// Attacker entity
	CEntityHandle m_attacker;
	// Target entity
	CEntityHandle m_hurtTarget;

public:
	// Linked list of all timedamage entities
	static CLinkedList<CTimeDamage*> g_timeDamageEntityList;
};
#endif //TIMEDAMAGE_H