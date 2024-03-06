/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "delayentity.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(delayeduse, CDelayEntity);

//=============================================
// @brief
//
//=============================================
CDelayEntity::CDelayEntity( edict_t* pedict ):
	CBaseEntity(pedict),
	m_delay(0),
	m_killTarget(NO_STRING_VALUE),
	m_useMode(USE_TOGGLE)
{
}

//=============================================
// @brief
//
//=============================================
CDelayEntity::~CDelayEntity( void )
{
}

//=============================================
// @brief
//
//=============================================
void CDelayEntity::DeclareSaveFields( void )
{
	// Call base class first
	CBaseEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CDelayEntity, m_delay, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CDelayEntity, m_killTarget, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CDelayEntity, m_useMode, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CDelayEntity::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "delay"))
	{
		m_delay = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "killtarget"))
	{
		m_killTarget = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}


//=============================================
// @brief
//
//=============================================
void CDelayEntity::UseTargets( CBaseEntity* pActivator, usemode_t useMode, Float value, string_t target )
{
	string_t targetentityname;
	if(target != NO_STRING_VALUE)
		targetentityname = target;
	else
		targetentityname = m_pFields->target;

	// Do nothing if both target and killtarget are null
	if(targetentityname == NO_STRING_VALUE && m_killTarget == NO_STRING_VALUE)
		return;

	if(m_delay > 0)
	{
		edict_t *pentity = gd_engfuncs.pfnCreateEntity("delayeduse");
		if(Util::IsNullEntity(pentity))
			return;

		CDelayEntity *pdelayentity = reinterpret_cast<CDelayEntity*>(CBaseEntity::GetClass(pentity));
		
		pentity->state.nextthink = g_pGameVars->time + m_delay;
		pdelayentity->SetThink(&CDelayEntity::DelayThink);

		// Set variables
		pdelayentity->m_useMode = m_useMode;
		pdelayentity->m_killTarget = m_killTarget;
		pdelayentity->m_delay = 0;
		pentity->fields.target = targetentityname;

		if(pActivator && pActivator->IsPlayer())
			pentity->state.owner = pActivator->GetEntityIndex();
		else
			pentity->state.owner = NO_ENTITY_INDEX;
	}
	else
	{
		if(m_killTarget != NO_STRING_VALUE)
		{
			const Char* pstrkilltargetname = gd_engfuncs.pfnGetString(m_killTarget);
			if(pstrkilltargetname)
			{
				if(g_pCvarTriggerDebug->GetValue() >= 1)
					gd_engfuncs.pfnCon_DPrintf("Killing entities with name: '%s'.\n", pstrkilltargetname);

				edict_t* pkillentity = Util::FindEntityByTargetName(nullptr, pstrkilltargetname);
				while(!Util::IsNullEntity(pkillentity))
				{
					Util::RemoveEntity(pkillentity);
					pkillentity = Util::FindEntityByTargetName(pkillentity, pstrkilltargetname);
				}
			}
		}

		// Call base class to manage the firing
		CBaseEntity::UseTargets(pActivator, useMode, value, targetentityname);
	}
}

//=============================================
// @brief
//
//=============================================
void CDelayEntity::DelayThink( void )
{
	CBaseEntity* pActivator = nullptr;
	if(m_pState->owner != NO_ENTITY_INDEX)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(m_pState->owner);
		if(!Util::IsNullEntity(pedict))
			pActivator = CBaseEntity::GetClass(pedict);
	}

	// Call CBaseEntity function
	UseTargets(pActivator, m_useMode, 0);

	// Remove this entity
	Util::RemoveEntity(this);
}
