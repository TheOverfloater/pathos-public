/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "npcmaker.h"
#include "ai_basenpc.h"

// Proximity check boundary size
const Vector CNPCMaker::BOUNDARY_CHECK_SIZE = Vector(34, 34, 0);

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(npcmaker, CNPCMaker);

//=============================================
// @brief
//
//=============================================
CNPCMaker::CNPCMaker( edict_t* pedict ):
	CDelayEntity(pedict),
	m_npcClassName(NO_STRING_VALUE),
	m_numNPCsToCreate(-1),
	m_maxLiveChildren(-1),
	m_numLiveChildren(0),
	m_deathTriggerTarget(NO_STRING_VALUE),
	m_classHeadSetting(0),
	m_classHeadWasSet(false),
	m_classWeaponSetting(0),
	m_classWeaponWasSet(false),
	m_isActive(false),
	m_fadeChildren(false)
{
}

//=============================================
// @brief
//
//=============================================
CNPCMaker::~CNPCMaker( void )
{
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CNPCMaker::Spawn( void )
{
	if(!CDelayEntity::Spawn())
		return false;

	if(m_npcClassName == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->effects |= EF_NODRAW;

	if(m_pFields->targetname != NO_STRING_VALUE)
	{
		if(HasSpawnFlag(FL_CYCLIC))
			SetUse(&CNPCMaker::CyclicUse);
		else
			SetUse(&CNPCMaker::ToggleUse);

		if(HasSpawnFlag(FL_START_ON))
			m_isActive = true;
	}
	else
	{
		m_isActive = true;
	}

	m_pState->flags |= FL_INITIALIZE;

	m_fadeChildren = (m_numNPCsToCreate > 1) ? true : false;

	if(!m_classnameHeadSettingsArray.empty() && !m_classnameWeaponSettingsArray.empty())
	{
		// Get name of npc
		const Char* pstrEntityClassName = gd_engfuncs.pfnGetString(m_npcClassName);
		if(pstrEntityClassName && qstrlen(pstrEntityClassName))
		{
			// Legacy mod support
			CString makeName = pstrEntityClassName;
			if(!qstrncmp(makeName, "monster_", 8))
			{
				makeName.erase(0, 8);
				makeName.insert(0, "npc_");
			}

			if(!m_classnameHeadSettingsArray.empty())
			{
				// Set head setting
				for(Uint32 i = 0; i < m_classnameHeadSettingsArray.size(); i++)
				{
					const classnamesetting_t& setting = m_classnameHeadSettingsArray[i];
					if(!qstrcmp(setting.classname, makeName))
					{
						m_classHeadSetting = setting.settingvalue;
						m_classHeadWasSet = true;
						break;
					}
				}

				m_classnameHeadSettingsArray.clear();
			}

			if(!m_classnameWeaponSettingsArray.empty())
			{
				// Set weapon setting
				for(Uint32 i = 0; i < m_classnameWeaponSettingsArray.size(); i++)
				{
					const classnamesetting_t& setting = m_classnameWeaponSettingsArray[i];
					if(!qstrcmp(setting.classname, makeName))
					{
						m_classWeaponSetting = setting.settingvalue;
						m_classWeaponWasSet = true;
						break;
					}
				}

				m_classnameWeaponSettingsArray.empty();
			}
		}
	}

	return true;
}

//=============================================
// @brief Initializes the entity after map has done loading
//
//=============================================
void CNPCMaker::InitEntity( void )
{
	Vector traceStart = m_pState->origin + Vector(0, 0, 16);
	Vector traceEnd = m_pState->origin - Vector(0, 0, 2048);

	trace_t tr;
	Util::TraceLine(traceStart, traceEnd, true, false, m_pEdict, tr);
	m_groundPosition = tr.endpos;

	if(m_isActive)
	{
		SetThink(&CNPCMaker::MakerThink);
		m_pState->nextthink = g_pGameVars->time + m_delay;
	}
}

//=============================================
// @brief Performs precache functions
//
//=============================================
void CNPCMaker::Precache( void )
{
	CDelayEntity::Precache();

	if(m_npcClassName != NO_STRING_VALUE)
		Util::PrecacheEntity(gd_engfuncs.pfnGetString(m_npcClassName));
}

//=============================================
// @brief Manages a keyvalue
//
//=============================================
bool CNPCMaker::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "monstercount") || !qstrcmp(kv.keyname, "childcount"))
	{
		m_numNPCsToCreate = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "m_imaxlivechildren"))
	{
		m_maxLiveChildren = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "monstertype") || !qstrcmp(kv.keyname, "childtype"))
	{
		// Legacy mod support
		if(!qstrncmp(kv.value, "monster_", 8))
		{
			CString tmp(kv.value);
			tmp.erase(0, 8);
			tmp.insert(0, "npc_");
			m_npcClassName = gd_engfuncs.pfnAllocString(tmp.c_str());
		}
		else
		{
			m_npcClassName = gd_engfuncs.pfnAllocString(kv.value);
		}
		return true;
	}
	else if(!qstrcmp(kv.keyname, "deathtarget"))
	{
		m_deathTriggerTarget = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrncmp(kv.keyname, "weapons_", 8))
	{
		const Char* pstrClassnameBit = kv.keyname + 8;
		CString strClassname;
		strClassname << "npc_" << pstrClassnameBit;

		classnamesetting_t setting;
		setting.classname = strClassname;
		setting.settingvalue = SDL_atoi(kv.value);
		m_classnameWeaponSettingsArray.push_back(setting);

		return true;
	}
	else if(!qstrncmp(kv.keyname, "heads_", 6))
	{
		const Char* pstrClassnameBit = kv.keyname + 6;
		CString strClassname;
		strClassname << "npc_" << pstrClassnameBit;

		classnamesetting_t setting;
		setting.classname = strClassname;
		setting.settingvalue = SDL_atoi(kv.value);
		m_classnameHeadSettingsArray.push_back(setting);

		return true;
	}
	else
	{
		return CDelayEntity::KeyValue(kv);
	}
}

//=============================================
// @brief Calls for classes and their children
//
//=============================================
void CNPCMaker::DeclareSaveFields( void )
{
	CDelayEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CNPCMaker, m_npcClassName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCMaker, m_numNPCsToCreate, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCMaker, m_maxLiveChildren, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCMaker, m_groundPosition, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCMaker, m_numLiveChildren, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCMaker, m_deathTriggerTarget, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCMaker, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCMaker, m_fadeChildren, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCMaker, m_classHeadSetting, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCMaker, m_classHeadWasSet, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCMaker, m_classWeaponSetting, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCMaker, m_classWeaponWasSet, EFIELD_BOOLEAN));
}

//=============================================
// @brief Death notice from child entities
//
//=============================================
void CNPCMaker::ChildDeathNotice( CBaseEntity* pChild )
{
	if(m_numLiveChildren)
		m_numLiveChildren--;

	if(m_deathTriggerTarget != NO_STRING_VALUE)
		Util::FireTargets(gd_engfuncs.pfnGetString(m_deathTriggerTarget), this, this, USE_TOGGLE, 0);

	if(!m_fadeChildren)
		pChild->SetOwner(nullptr);
}

//=============================================
// @brief Called when npcmaker is toggled
//
//=============================================
void CNPCMaker::ToggleUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool prevState = m_isActive;
	switch(useMode)
	{
	case USE_ON:
		m_isActive = true;
		break;
	case USE_OFF:
		m_isActive = false;
		break;
	case USE_TOGGLE:
		{
			if(!m_isActive)
				m_isActive = true;
			else
				m_isActive = false;
		}
		break;
	}

	if(prevState == m_isActive)
		return;

	if(!m_isActive)
	{
		SetThink(nullptr);
		m_pState->nextthink = 0;
	}
	else
	{
		SetThink(&CNPCMaker::MakerThink);
		m_pState->nextthink = g_pGameVars->time;
	}
}

//=============================================
// @brief Called when npcmaker is cyclic
//
//=============================================
void CNPCMaker::CyclicUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	CreateNPC(true);
}

//=============================================
// @brief Think function for spawning npcs
//
//=============================================
void CNPCMaker::MakerThink( void )
{
	SetThink(&CNPCMaker::MakerThink);
	m_pState->nextthink = g_pGameVars->time + m_delay;

	CreateNPC(false);
}

//=============================================
// @brief Called to fire(duh)(I need better comments)
//
//=============================================
void CNPCMaker::FireThink( void )
{
	if(m_pFields->target != NO_STRING_VALUE)
		Util::FireTargets(gd_engfuncs.pfnGetString(m_pFields->target), this, this, USE_TOGGLE, 0);

	if(!m_numNPCsToCreate)
	{
		// Disable but don't destroy, as it'll still get notices
		SetThink(nullptr);
		SetUse(nullptr);
	}
}

//=============================================
// @brief Creates the NPC
//
//=============================================
void CNPCMaker::CreateNPC( bool isCyclic )
{
	// Don't spawn if not cyclic and we're full with live children
	if(!isCyclic && m_maxLiveChildren > 0 && (Int32)m_numLiveChildren >= m_maxLiveChildren)
		return;

	// Get name of npc
	const Char* pstrEntityClassName = gd_engfuncs.pfnGetString(m_npcClassName);
	if(!pstrEntityClassName || !qstrlen(pstrEntityClassName))
		return;

	// Legacy mod support
	CString makeName = pstrEntityClassName;
	if(!qstrncmp(makeName, "monster_", 8))
	{
		makeName.erase(0, 8);
		makeName.insert(0, "npc_");
	}

	// Don't do proximity checks for weapon entities, or when we're cyclic
	if(qstrncmp(makeName, "weapon_", 7) && !isCyclic)
	{
		Vector mins = m_pState->origin - BOUNDARY_CHECK_SIZE;
		Vector maxs = m_pState->origin + BOUNDARY_CHECK_SIZE;

		maxs.z = m_pState->origin.z;
		mins.z = m_groundPosition.z;

		edict_t* pedict = nullptr;
		while(true)
		{
			pedict = Util::FindEntityInBBox(pedict, mins, maxs);
			if(!pedict)
				break;

			if(Util::IsNullEntity(pedict))
				continue;

			CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
			if(!pEntity)
				continue;

			if((pEntity->IsPlayer() || pEntity->IsNPC()) && pEntity->IsAlive())
				return;
		}
	}

	Vector spawnOrigin = m_pState->origin + Vector(0, 0, 4);
	CBaseEntity* pEntity = CBaseEntity::CreateEntity(makeName.c_str(), spawnOrigin, m_pState->angles, this);
	if(!pEntity)
	{
		Util::EntityConPrintf(m_pEdict, "Null entity specified for creation.\n");
		return;
	}

	// If not present, use legacy
	Int32 weaponsValue;
	if(!m_classWeaponWasSet || m_classWeaponSetting == 0 && m_pState->weapons != 0)
		weaponsValue = m_pState->weapons;
	else
		weaponsValue = m_classWeaponSetting;

	pEntity->SetWeapons(weaponsValue);

	// Set head based on classname setting. If not present, use legacy.
	Int32 headValue = m_classHeadWasSet ? m_classHeadSetting : m_pState->body;
	pEntity->SetHead(headValue);

	// TODO: Why did I do this?
	pEntity->SetNetname(gd_engfuncs.pfnGetString(m_pFields->netname)); 

	if(m_fadeChildren)
		pEntity->SetSpawnFlag(CBaseNPC::FL_NPC_FADE_CORPSE);

	pEntity->SetSpawnFlag(CBaseNPC::FL_NPC_FALL_TO_GROUND);

	if(HasSpawnFlag(FL_NPC_CLIP))
		pEntity->SetSpawnFlag(CBaseNPC::FL_NPC_USE_NPC_CLIP);

	if(HasSpawnFlag(FL_GAG))
		pEntity->SetSpawnFlag(CBaseNPC::FL_NPC_GAG);

	if(HasSpawnFlag(FL_IDLE))
		pEntity->SetSpawnFlag(CBaseNPC::FL_NPC_IDLE);

	pEntity->SetAlwaysAlert(true);
	if(!pEntity->Spawn())
	{
		Util::RemoveEntity(pEntity);
		return;
	}

	// Set targetname if netname is not null
	if(m_pFields->netname != NO_STRING_VALUE)
		pEntity->SetTargetName(gd_engfuncs.pfnGetString(m_pFields->netname));

	if(pEntity->IsNPC())
		m_numLiveChildren++;

	if(m_numNPCsToCreate != -1)
		m_numNPCsToCreate--;

	if(!m_numNPCsToCreate)
	{
		if(m_pFields->target != NO_STRING_VALUE)
		{
			SetThink(&CNPCMaker::FireThink);
			m_pState->nextthink = g_pGameVars->time + 0.1;
		}
		else
		{
			// Disable but don't destroy, as it'll still get notices
			SetThink(nullptr);
			SetUse(nullptr);
		}
	}
}