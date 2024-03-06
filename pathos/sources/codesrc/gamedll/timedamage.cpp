/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "timedamage.h"

// Linked list of all timedamage entities
CLinkedList<CTimeDamage*> CTimeDamage::g_timeDamageEntityList;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(timedamage, CTimeDamage);

//=============================================
// @brief
//
//=============================================
CTimeDamage::CTimeDamage( edict_t* pedict ):
	CBaseEntity(pedict),
	m_dmgType(TD_BBOX),
	m_dmgDelay(0),
	m_dmgTypeFlags(0),
	m_lifetime(0),
	m_dmgAmount(0),
	m_hitgroup(0),
	m_nextDmgTime(0),
	m_deathTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CTimeDamage::~CTimeDamage( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CTimeDamage::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	switch(m_dmgType)
	{
	case TD_BBOX:
	case TD_BBOX_FOLLOW_ENTITY:
		{
			if(m_dmgType == TD_BBOX_FOLLOW_ENTITY)
			{
				if(m_pState->aiment == NO_ENTITY_INDEX)
				{
					Util::WarnEmptyEntity(m_pEdict);
					return true;
				}

				m_pState->movetype = MOVETYPE_FOLLOW;
				m_pState->solid = SOLID_TRIGGER;
			}
			else
			{
				m_pState->movetype = MOVETYPE_NONE;
				m_pState->solid = SOLID_TRIGGER;
			}
		}
		break;
	case TD_FOLLOW_ENTITY:
		{
			if(m_pState->aiment == NO_ENTITY_INDEX)
			{
				Util::WarnEmptyEntity(m_pEdict);
				return true;
			}

			m_pState->movetype = MOVETYPE_FOLLOW;
			m_pState->solid = SOLID_NOT;

			gd_engfuncs.pfnSetMinsMaxs(m_pEdict, ZERO_VECTOR, ZERO_VECTOR);
		}
		break;
	}

	SetThink(&CTimeDamage::DamageThink);
	m_pState->nextthink = g_pGameVars->time + m_dmgDelay;
	m_deathTime = g_pGameVars->time + m_lifetime;

	if(!m_attacker)
		m_attacker = this;

	if(!m_dmgDelay)
		m_dmgDelay = 0.1;

	CTimeDamage::AddTimeDamageToList(this);

	return true;
}

//=============================================
// @brief Called when the entity is freed
//
//=============================================
void CTimeDamage::FreeEntity( void )
{
	CBaseEntity::FreeEntity();

	CTimeDamage::RemoveTimeDamageFromList(this);
}

//=============================================
// @brief
//
//=============================================
void CTimeDamage::DeclareSaveFields( void )
{
	CBaseEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CTimeDamage, m_dmgType, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTimeDamage, m_dmgDelay, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTimeDamage, m_dmgTypeFlags, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTimeDamage, m_lifetime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTimeDamage, m_dmgAmount, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTimeDamage, m_hitgroup, EFIELD_INT32));

	DeclareSaveField(DEFINE_DATA_FIELD(CTimeDamage, m_nextDmgTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTimeDamage, m_deathTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTimeDamage, m_attacker, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CTimeDamage, m_hurtTarget, EFIELD_EHANDLE));
}

//=============================================
// @brief
//
//=============================================
Int32 CTimeDamage::GetEntityFlags( void ) 
{
	Int32 flags = CBaseEntity::GetEntityFlags();

	// Do not transition this entity
	flags &= ~(FL_ENTITY_TRANSITION);

	// Do not save this entity
	flags |= FL_ENTITY_DONT_SAVE;

	return flags; 
}

//=============================================
// @brief
//
//=============================================
void CTimeDamage::DamageThink( void )
{
	if(m_deathTime <= g_pGameVars->time || (m_dmgType == TD_FOLLOW_ENTITY && !m_hurtTarget))
	{
		SetThink(&CBaseEntity::RemoveThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
		return;
	}

	if(m_dmgType == TD_FOLLOW_ENTITY)
	{
		Float dmg = m_dmgAmount * m_hurtTarget->GetHitgroupDmgMultiplier(m_hitgroup);
		m_hurtTarget->TakeDamage(this, m_attacker, dmg, m_dmgTypeFlags);
	}
	else
	{
		edict_t* pedict = nullptr;
		while(true)
		{
			pedict = Util::FindEntityInBBox(pedict, m_pState->absmin, m_pState->absmax);
			if(!pedict)
				break;

			CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
			if(!pEntity || pEntity->GetTakeDamage() == TAKEDAMAGE_NO)
				continue;

			// Do not hurt NPC we're tied to if we're a bbox
			if(m_dmgType == TD_BBOX_FOLLOW_ENTITY 
				&& pEntity->GetEntityIndex() == m_pState->aiment)
				continue;

			pEntity->TakeDamage(this, m_attacker, m_dmgAmount, m_dmgTypeFlags);
		}
	}

	SetThink(&CTimeDamage::DamageThink);
	m_pState->nextthink = g_pGameVars->time + m_dmgDelay;
}

//=============================================
// @brief
//
//=============================================
void CTimeDamage::SetType( damage_type_t type )
{
	m_dmgType = type;
}

//=============================================
// @brief
//
//=============================================
void CTimeDamage::SetDamageDelay( Float delay )
{
	m_dmgDelay = delay;
}

//=============================================
// @brief
//
//=============================================
void CTimeDamage::SetDamageTypeFlags( Int32 dmgFlags )
{
	m_dmgTypeFlags = dmgFlags;
}

//=============================================
// @brief
//
//=============================================
void CTimeDamage::SetLifetime( Float life )
{
	m_lifetime = life;
}

//=============================================
// @brief
//
//=============================================
void CTimeDamage::SetDamageAmount( Float dmg )
{
	m_dmgAmount = dmg;
}

//=============================================
// @brief
//
//=============================================
void CTimeDamage::SetAttacker( CBaseEntity* pAttacker )
{
	m_attacker = pAttacker;
}

//=============================================
// @brief
//
//=============================================
void CTimeDamage::SetHurtTarget( CBaseEntity* pTarget )
{
	m_hurtTarget = pTarget;
}

//=============================================
// @brief
//
//=============================================
void CTimeDamage::SetHitgroup( Int32 hitgroup )
{
	m_hitgroup = hitgroup;
}

//=============================================
// @brief Get the type
//
//=============================================
CTimeDamage::damage_type_t CTimeDamage::GetDmgType( void )
{
	return (damage_type_t)m_dmgType;
}

//=============================================
// @brief
//
//=============================================
void CTimeDamage::OnAimentFreed( void )
{
	CBaseEntity::OnAimentFreed();

	if(m_dmgType == TD_FOLLOW_ENTITY
		|| m_dmgType == TD_BBOX_FOLLOW_ENTITY)
	{
		m_pState->solid = SOLID_NOT;
		m_pState->movetype = MOVETYPE_NONE;

		SetThink(&CBaseEntity::RemoveThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
}

//=============================================
// @brief
//
//=============================================
CTimeDamage* CTimeDamage::CreateTimeDamageBox( CBaseEntity* pInflictor, const Vector& origin, const Vector& mins, const Vector& maxs, Int32 dmgTypeFlags, Float dmgDelay, Float dmgAmount, Float life, CBaseEntity* pFollow )
{
	CTimeDamage* pEntity = reinterpret_cast<CTimeDamage*>(CBaseEntity::CreateEntity("timedamage", pInflictor));

	pEntity->SetType(pFollow ? TD_BBOX_FOLLOW_ENTITY : TD_BBOX);
	pEntity->SetDamageAmount(dmgAmount);
	pEntity->SetLifetime(life);
	pEntity->SetDamageTypeFlags(dmgTypeFlags);
	pEntity->SetDamageDelay(dmgDelay);
	pEntity->SetAttacker(pInflictor);
	
	if(pFollow)
		pEntity->SetAiment(pFollow);

	if(!pEntity->Spawn())
	{
		Util::RemoveEntity(pEntity);
		return nullptr;
	}

	pEntity->SetOrigin(origin);
	pEntity->SetMinsMaxs(mins, maxs);

	return pEntity;
}

//=============================================
// @brief Adds entity to the list
//
//=============================================
void CTimeDamage::AddTimeDamageToList( CTimeDamage* pDamage )
{
	g_timeDamageEntityList.begin();
	while(!g_timeDamageEntityList.end())
	{
		if(g_timeDamageEntityList.get() == pDamage)
			return;

		g_timeDamageEntityList.next();
	}

	g_timeDamageEntityList.add(pDamage);
}

//=============================================
// @brief Removes entity from list
//
//=============================================
void CTimeDamage::RemoveTimeDamageFromList( const CTimeDamage* pDamage )
{
	g_timeDamageEntityList.begin();
	while(!g_timeDamageEntityList.empty())
	{
		if(g_timeDamageEntityList.get() == pDamage)
		{
			g_timeDamageEntityList.remove(g_timeDamageEntityList.get_link());
			return;
		}

		g_timeDamageEntityList.next();
	}
}

//=============================================
// @brief Clears all timedamage entities from list
//
//=============================================
void CTimeDamage::ClearTimeDamageList( void )
{
	if(g_timeDamageEntityList.empty())
		return;

	g_timeDamageEntityList.clear();
}

//=============================================
// @brief Tells if there's another timedamage with type tied to an entity
//
//=============================================
bool CTimeDamage::EntityHasTimeDamageTied( const CBaseEntity* pEntity, damage_type_t type )
{
	g_timeDamageEntityList.begin();
	while(!g_timeDamageEntityList.end())
	{
		CTimeDamage* pOther = g_timeDamageEntityList.get();

		if(pOther->GetAiment() == pEntity && pOther->GetDmgType())
			return true;

		g_timeDamageEntityList.next();
	}

	return false;
}

//=============================================
// @brief Tells if there's a timedamage in the bbox
//
//=============================================
bool CTimeDamage::HasTimeDamageInBBox( const Vector& mins, const Vector& maxs, damage_type_t type )
{
	g_timeDamageEntityList.begin();
	while(!g_timeDamageEntityList.end())
	{
		CTimeDamage* pOther = g_timeDamageEntityList.get();
		if(pOther->GetDmgType() == type)
		{
			if(!Math::CheckMinsMaxs(pOther->GetAbsMins(), pOther->GetAbsMaxs(), mins, maxs))
				return true;
		}

		g_timeDamageEntityList.next();
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
CTimeDamage* CTimeDamage::CreateTimeDamageFollowEntity( CBaseEntity* pInflictor, CBaseEntity* pTarget, Int32 dmgTypeFlags, Int32 hitgroup, Float dmgDelay, Float dmgAmount, Float life )
{
	CTimeDamage* pEntity = reinterpret_cast<CTimeDamage*>(CBaseEntity::CreateEntity("timedamage", pInflictor));

	pEntity->SetType(TD_FOLLOW_ENTITY);
	pEntity->SetDamageAmount(dmgAmount);
	pEntity->SetLifetime(life);
	pEntity->SetDamageTypeFlags(dmgTypeFlags);
	pEntity->SetDamageDelay(dmgDelay);
	pEntity->SetAttacker(pInflictor);
	pEntity->SetHitgroup(hitgroup);

	pEntity->SetAiment(pTarget);
	pEntity->SetHurtTarget(pTarget);

	if(!pEntity->Spawn())
	{
		Util::RemoveEntity(pEntity);
		return nullptr;
	}

	return pEntity;
}