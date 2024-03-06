/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "multidamage.h"

// Object definition
CMultiDamage gMultiDamage;

//=============================================
// @brief
//
//=============================================
CMultiDamage::CMultiDamage( void ):
	m_damageFlags(0)
{
}

//=============================================
// @brief
//
//=============================================
CMultiDamage::~CMultiDamage( void )
{
}

//=============================================
// @brief
//
//=============================================
void CMultiDamage::Clear( void )
{
	if(!m_damagedEntities.empty())
		m_damagedEntities.clear();

	m_damageFlags = 0;
}

//=============================================
// @brief
//
//=============================================
void CMultiDamage::ApplyDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Int32 hitgroup )
{
	if(m_damagedEntities.empty())
		return;

	m_damagedEntities.begin();
	while(!m_damagedEntities.end())
	{
		entitydamage_t& dmg = m_damagedEntities.get();
		dmg.hitgroup = hitgroup;
		if(dmg.pentity)
			dmg.pentity->TakeDamage(pInflictor, pAttacker, dmg.dmgamount, dmg.dmgtype);

		m_damagedEntities.next();
	}

	// Clear the list
	m_damagedEntities.clear();
}

//=============================================
// @brief
//
//=============================================
void CMultiDamage::AddDamage( CBaseEntity* pentity, Float damage, Int32 dmgtype )
{
	if(!pentity)
		return;

	// See if it's already present
	entitydamage_t* pdamage = nullptr;
	if(!m_damagedEntities.empty())
	{
		m_damagedEntities.begin();
		while(!m_damagedEntities.end())
		{
			entitydamage_t& check = m_damagedEntities.get();
			if(check.pentity == pentity)
			{
				pdamage = &check;
				break;
			}

			m_damagedEntities.next();
		}
	}

	// Add a new one
	if(!pdamage)
	{
		pdamage = &m_damagedEntities.add(entitydamage_t())->_val;
		pdamage->pentity = pentity;
		pdamage->dmgtype |= m_damageFlags;
	}

	if(!(pdamage->dmgtype & dmgtype))
		pdamage->dmgtype |= dmgtype;

	pdamage->dmgamount += damage;
}

//=============================================
// @brief
//
//=============================================
void CMultiDamage::SetDamageFlags( Int32 dmgtype )
{
	m_damageFlags |= dmgtype;
}

//=============================================
// @brief
//
//=============================================
void CMultiDamage::SetAttackDirection( const Vector& direction )
{
	m_attackDirection = direction;
}

//=============================================
// @brief
//
//=============================================
const Vector& CMultiDamage::GetAttackDirection( void ) const
{
	return m_attackDirection;
}

//=============================================
// @brief
//
//=============================================
Int32 CMultiDamage::GetHitGroupForEntity( const CBaseEntity* pEntity )
{
	m_damagedEntities.begin();
	while(!m_damagedEntities.end())
	{
		const entitydamage_t& check = m_damagedEntities.get();
		if(check.pentity == pEntity)
			return check.hitgroup;

		m_damagedEntities.next();
	}

	return HITGROUP_GENERIC;
}