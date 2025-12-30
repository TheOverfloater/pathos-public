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
	m_bulletType(BULLET_NONE),
	m_damageFlags(0),
	m_nbTotalShots(0)
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
void CMultiDamage::Prepare( bullet_types_t bulletType, const Vector& shotOrigin )
{
	Prepare(bulletType, shotOrigin, ZERO_VECTOR);
}

//=============================================
// @brief
//
//=============================================
void CMultiDamage::Prepare( bullet_types_t bulletType, const Vector& shotOrigin, const Vector& shotDirection )
{
	if(!m_damagedEntities.empty())
		m_damagedEntities.clear();

	m_damageFlags = 0;
	m_nbTotalShots = 0;
	m_bulletType = bulletType;
	m_shotDirection = shotDirection;
	m_attackOrigin = shotOrigin;
}

//=============================================
// @brief
//
//=============================================
void CMultiDamage::ApplyDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker )
{
	if(m_damagedEntities.empty())
		return;

	m_damagedEntities.begin();
	while(!m_damagedEntities.end())
	{
		entitydamage_t& dmg = m_damagedEntities.get();

		if(dmg.pentity)
		{
			// Set damage direction based on vector from inflictor center to target's center
			m_damageDirection = (m_attackOrigin - dmg.pentity->GetCenter()).Normalize();
			// Apply damage to the target
			dmg.pentity->TakeDamage(pInflictor, pAttacker, dmg.dmgamount, dmg.dmgtype);
		}

		m_damagedEntities.next();
	}

	// Clear the list
	m_damagedEntities.clear();
}

//=============================================
// @brief
//
//=============================================
void CMultiDamage::AddDamage( CBaseEntity* pentity, Float damage, Int32 dmgtype, hitgroups_t hitgroup )
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
	pdamage->hitcount++;

	if(hitgroup >= 0 && hitgroup < NB_HITGROUPS)
	{
		// Apply to the specific hitgroup
		pdamage->grouphitcounts[hitgroup]++;
	}
	else
	{
		// Apply to generic instead
		pdamage->grouphitcounts[HITGROUP_GENERIC]++;

		// Warn developer
		gd_engfuncs.pfnCon_Printf("%s - Invalid hitgroup %d specified for entity '%s'.\n", __FUNCTION__, hitgroup, pentity->GetClassName());
	}

	m_nbTotalShots++;
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
const Vector& CMultiDamage::GetAttackOrigin( void ) const
{
	return m_attackOrigin;
}

//=============================================
// @brief
//
//=============================================
Uint32 CMultiDamage::GetShotCount( void ) const
{
	return m_nbTotalShots;
}

//=============================================
// @brief
//
//=============================================
bullet_types_t CMultiDamage::GetBulletType( void ) const
{
	return m_bulletType;
}

//=============================================
// @brief
//
//=============================================
const Vector& CMultiDamage::GetShotDirection( void ) const
{
	return m_shotDirection;
}

//=============================================
// @brief
//
//=============================================
const Vector& CMultiDamage::GetDamageDirection( void ) const
{
	return m_damageDirection;
}

//=============================================
// @brief
//
//=============================================
Uint32 CMultiDamage::GetEntityHitCount( const CBaseEntity* pEntity )
{
	m_damagedEntities.begin();
	while(!m_damagedEntities.end())
	{
		const entitydamage_t& check = m_damagedEntities.get();
		if(check.pentity == pEntity)
			return check.hitcount;

		m_damagedEntities.next();
	}

	return 0;
}

//=============================================
// @brief
//
//=============================================
Uint32 CMultiDamage::GetHitGroupHitCountForEntity( const CBaseEntity* pEntity, hitgroups_t hitgroup )
{
	if(hitgroup < 0 || hitgroup > NB_HITGROUPS)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid hitgroup '%d' specified.\n", __FUNCTION__, static_cast<Int32>(hitgroup));
		return 0;
	}

	m_damagedEntities.begin();
	while(!m_damagedEntities.end())
	{
		const entitydamage_t& check = m_damagedEntities.get();
		if(check.pentity == pEntity)
			return check.grouphitcounts[hitgroup];

		m_damagedEntities.next();
	}

	return 0;
}

//=============================================
// @brief
//
//=============================================
hitgroups_t CMultiDamage::GetHitHighestCountGroupForEntity( const CBaseEntity* pEntity )
{
	m_damagedEntities.begin();
	while(!m_damagedEntities.end())
	{
		const entitydamage_t& check = m_damagedEntities.get();
		if(check.pentity == pEntity)
		{
			hitgroups_t highestGrp = HITGROUP_NONE;
			Uint32 highestCount = 0;

			for(Uint32 i = 0; i < NB_HITGROUPS; i++)
			{
				if(check.grouphitcounts[i] > highestCount)
				{
					highestGrp = static_cast<hitgroups_t>(i);
					highestCount = check.grouphitcounts[i];
				}
			}

			if(highestGrp != HITGROUP_NONE)
				return highestGrp;
			else
				return HITGROUP_GENERIC;
		}

		m_damagedEntities.next();
	}

	return HITGROUP_GENERIC;
}