/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "ai_squadnpc.h"
#include "cplane.h"

// Number of slots
const Uint32 CSquadNPC::NUM_SLOTS = 5;
// Search radius for squad members to recruit
const Float CSquadNPC::SQUADINIT_SEARCH_RADIUS = 1024;
// Radius to check for cover validation
const Float CSquadNPC::COVER_VALIDATE_RADIUS = 128;
// Elude time for enemies
const Float CSquadNPC::ENEMY_ELUDE_TIME = 5;

//==========================================================================
//
// SCHEDULES FOR CSQUADNPC CLASS
//
//==========================================================================

//=============================================
// @brief Found Enemy
//
//=============================================
ai_task_t taskListScheduleFoundEnemy[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_PLAY_SEQUENCE_FACE_ENEMY,	(Float)ACT_SIGNAL1)
};

const CAISchedule scheduleFoundEnemy(
	// Task list
	taskListScheduleFoundEnemy, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleFoundEnemy),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Found Enemy"
);

//=============================================
// @brief Combat Face
//
//=============================================
ai_task_t taskListScheduleSquadNPCCombatFace[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_WAIT,						1.5),
	AITASK(AI_TASK_SET_SCHEDULE,				(Float)AI_SCHED_SWEEP)
};

const CAISchedule scheduleSquadNPCCombatFace(
	// Task list
	taskListScheduleSquadNPCCombatFace, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleSquadNPCCombatFace),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_ENEMY_DEAD |
	AI_COND_CAN_ATTACK,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Combat Face"
);

//=============================================
// @brief Signal Suppressing Fire
//
//=============================================
ai_task_t taskListScheduleSignalSuppressingFire[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_PLAY_SEQUENCE_FACE_ENEMY,	(Float)ACT_SIGNAL2)
};

const CAISchedule scheduleSignalSuppressingFire(
	// Task list
	taskListScheduleSignalSuppressingFire, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleSignalSuppressingFire),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_ENEMY_DEAD |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_HEAR_SOUND |
	AI_COND_NO_AMMO_LOADED,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Signal Suppressing Fire"
);

//=============================================
// @brief Take cover
//
//=============================================
ai_task_t taskListScheduleTakeCover[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_FAIL_SCHEDULE,			(Float)AI_SQUADNPC_SCHED_TAKE_COVER_FAILED),
	AITASK(AI_TASK_WAIT,						0.2f),
	AITASK(AI_TASK_FIND_COVER_FROM_ENEMY,		0),
	AITASK(AI_TASK_RUN_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_REMEMBER,					(Float)AI_MEMORY_IN_COVER),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_SET_SCHEDULE,				(Float)AI_SCHED_WAIT_FACE_ENEMY)
};

const CAISchedule scheduleTakeCover(
	// Task list
	taskListScheduleTakeCover, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleTakeCover),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_IN_DANGER,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Take Cover"
);

//==========================================================================
//
// SCHEDULES FOR CSQUADNPC CLASS
//
//==========================================================================

//=============================================
// @brief Constructor
//
//=============================================
CSquadNPC::CSquadNPC( edict_t* pedict ):
	CBaseNPC(pedict),
	m_squadSlots(0),
	m_lastEnemySightTime(0),
	m_enemyEluded(false),
	m_mySlot(0),
	m_signalledSuppressingFire(false)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CSquadNPC::~CSquadNPC( void )
{
}

//=============================================
// @brief Declares saved variables for the class
//
//=============================================
void CSquadNPC::DeclareSaveFields( void )
{
	// Call base class to do it first
	CBaseNPC::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CSquadNPC, m_squadLeaderNPC, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CSquadNPC, m_squadMembers, EFIELD_EHANDLE, MAX_SQUAD_MEMBERS - 1));
	DeclareSaveField(DEFINE_DATA_FIELD(CSquadNPC, m_squadSlots, EFIELD_UINT64));
	DeclareSaveField(DEFINE_DATA_FIELD(CSquadNPC, m_lastEnemySightTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CSquadNPC, m_enemyEluded, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CSquadNPC, m_mySlot, EFIELD_UINT64));
	DeclareSaveField(DEFINE_DATA_FIELD(CSquadNPC, m_signalledSuppressingFire, EFIELD_BOOLEAN));
}

//=============================================
// @brief Initializes squad related data
//
//=============================================
void CSquadNPC::InitSquad( void )
{
	// Don't try if we can't form squads, or are already in one
	if(!(m_capabilityBits & AI_CAP_SQUAD) || IsInSquad())
		return;

	// If in a designated squad
	if(HasNetname())
	{
		// only squadleader can recruit
		if(!HasSpawnFlag(CBaseNPC::FL_NPC_SQUADLEADER))
			return;

		Uint32 squadSize = SquadRecruit(SQUADINIT_SEARCH_RADIUS, (MAX_SQUAD_MEMBERS-1));
		if(squadSize > 1)
		{
			CString classificationName;
			switch(GetClassification())
			{
			case CLASS_UNDEFINED:
				classificationName = "CLASS_UNDEFINED";
				break;
			case CLASS_NONE:
				classificationName = "CLASS_NONE";
				break;
			case CLASS_MACHINE:
				classificationName = "CLASS_MACHINE";
				break;
			case CLASS_PLAYER:
				classificationName = "CLASS_PLAYER";
				break;
			case CLASS_HUMAN_HOSTILE:
				classificationName = "CLASS_HUMAN_HOSTILE";
				break;
			case CLASS_HUMAN_FRIENDLY:
				classificationName = "CLASS_HUMAN_FRIENDLY";
				break;
			case CLASS_UNUSED:
				classificationName = "CLASS_UNUSED";
				break;
			}

			gd_engfuncs.pfnCon_DPrintf("Squad of %d npcs of type '%s' formed.\n", squadSize, classificationName.c_str());
			return;
		}
	}

	// Try to join an existing squad
	Vector mins, maxs;
	for(Uint32 i = 0; i < 3; i++)
	{
		mins[i] = m_pState->origin[i] - SQUADINIT_SEARCH_RADIUS;
		maxs[i] = m_pState->origin[i] + SQUADINIT_SEARCH_RADIUS;
	}

	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityInBBox(pedict, mins, maxs);
		if(!pedict)
			break;

		if(Util::IsNullEntity(pedict))
			continue;

		if(pedict == m_pEdict)
			continue;

		// See if this is a squad entity we can join squads with
		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(pEntity 
			&& pEntity->IsAlive() 
			&& pEntity->IsSquadNPC() 
			&& pEntity->IsSquadLeader() 
			&& pEntity->GetClassification() == GetClassification()
			&& !pEntity->HasNetname())
		{
			trace_t tr;
			Util::TraceLine(GetEyePosition(), pEntity->GetEyePosition(), true, false, pEntity->GetEdict(), tr);
			if(tr.noHit() && pEntity->AddToSquad(this))
			{
				CString classificationName;
				switch(GetClassification())
				{
				case CLASS_UNDEFINED:
					classificationName = "CLASS_UNDEFINED";
					break;
				case CLASS_NONE:
					classificationName = "CLASS_NONE";
					break;
				case CLASS_MACHINE:
					classificationName = "CLASS_MACHINE";
					break;
				case CLASS_PLAYER:
					classificationName = "CLASS_PLAYER";
					break;
				case CLASS_HUMAN_HOSTILE:
					classificationName = "CLASS_HUMAN_HOSTILE";
					break;
				case CLASS_HUMAN_FRIENDLY:
					classificationName = "CLASS_HUMAN_FRIENDLY";
					break;
				case CLASS_UNUSED:
					classificationName = "CLASS_UNUSED";
					break;
				}

				Util::EntityConDPrintf(m_pEdict, "%s joined existing squad of '%s'.\n", pEntity->GetClassName(), classificationName.c_str());
				return;
			}
		}
	}

	if(!IsInSquad())
	{
		// Set myself as leader
		m_squadLeaderNPC = this;
	}
}

//=============================================
// @brief Tells if this npc is the leader
//
//=============================================
bool CSquadNPC::IsSquadLeader( void ) const
{
	return (m_squadLeaderNPC == this) ? true : false;
}

//=============================================
// @brief Checks the current enemy's state
//
//=============================================
bool CSquadNPC::CheckEnemy( void )
{
	// Call base class to manage
	bool updatedEnemy = CBaseNPC::CheckEnemy();

	// Check for errors
	CBaseEntity* pSquadLeader = GetSquadLeader();
	if(!pSquadLeader)
	{
		Util::EntityConDPrintf(m_pEdict, "%s - Squad leader was null.\n", __FUNCTION__);
		return updatedEnemy;
	}

	// Tell other squad members about the enemy info, if the enemy is our squad leader's enemy
	if(IsInSquad() && m_enemy == reinterpret_cast<const CBaseEntity*>(pSquadLeader->GetEnemy()))
	{
		if(updatedEnemy)
			SetSquadEnemyInfo();
		else
			CopySquadEnemyInfo();
	}

	return updatedEnemy;
}

//=============================================
// @brief Called when a schedule is changed
//
//=============================================
void CSquadNPC::OnScheduleChange( void )
{
	// Vacate slot on schedule change
	VacateSlot();

	// Call base class to manage
	CBaseNPC::OnScheduleChange();
}

//=============================================
// @brief Manages getting killed
//
//=============================================
void CSquadNPC::Killed( CBaseEntity* pAttacker, gibbing_t gibbing, deathmode_t deathMode )
{
	// Vacate our slot
	VacateSlot();

	if(IsInSquad())
	{
		CBaseEntity* pLeader = GetSquadLeader();
		pLeader->RemoveNPCFromSquad(this);
	}

	CBaseNPC::Killed(pAttacker, gibbing, deathMode);
}

//=============================================
// @brief Forgets the player
//
//=============================================
void CSquadNPC::ForgetPlayer( CBaseEntity* pPlayer )
{
	if(IsSquadLeader())
	{
		for(Uint32 i = 0; i < (MAX_SQUAD_MEMBERS-1); i++)
		{
			CBaseEntity* pMember = GetSquadMember(i);
			if(!pMember || pMember == this)
				continue;

			pMember->ForgetPlayer(pPlayer);
		}
	}

	CBaseNPC::ForgetPlayer(pPlayer);
}
 
//=============================================
// @brief Sets the last enemy sight time
//
//=============================================
void CSquadNPC::SetLastEnemySightTime( Double time )
{
	m_lastEnemySightTime = time;
}

//=============================================
// @brief Sets the last enemy sight time
//
//=============================================
Double CSquadNPC::GetLastEnemySightTime( void )
{
	return m_lastEnemySightTime;
}

//=============================================
// @brief Adds a new enemy
//
//=============================================
void CSquadNPC::PushEnemy( CBaseEntity* pEnemy, const Vector& lastPosition, const Vector& lastAngles )
{
	CBaseEntity* pSquadLeader = GetSquadLeader();
	pSquadLeader->SetLastEnemySightTime(g_pGameVars->time);

	CBaseNPC::PushEnemy(pEnemy, lastPosition, lastAngles);
}

//=============================================
// @brief Allows squad members to validate a cover position
//
//=============================================
bool CSquadNPC::ValidateCover( const Vector& coverPosition )
{
	if(!IsInSquad())
		return true;

	if(IsSquadMemberInRange(coverPosition, COVER_VALIDATE_RADIUS))
		return false;
	else
		return true;
}

//=============================================
// @brief Returns the ideal NPC state
//
//=============================================
npcstate_t CSquadNPC::GetIdealNPCState( void )
{
	switch(m_npcState)
	{
	case NPC_STATE_IDLE:
	case NPC_STATE_ALERT:
		{
			if(CheckConditions(AI_COND_NEW_ENEMY) && IsInSquad())
				SetSquadEnemy(m_enemy);
		}
		break;
	}

	return CBaseNPC::GetIdealNPCState();
}

//=============================================
// @brief Returns a schedule by it's index
//
//=============================================
const CAISchedule* CSquadNPC::GetScheduleByIndex( Int32 scheduleIndex )
{
	switch(scheduleIndex)
	{
	case AI_SCHED_CHASE_ENEMY_FAILED:
		{
			return &scheduleChaseEnemyFailed;
		}
		break;
	case AI_SQUADNPC_SCHED_FOUND_ENEMY:
		{
			return &scheduleFoundEnemy;
		}
		break;
	case AI_SQUADNPC_SCHED_COMBAT_FACE:
		{
			return &scheduleSquadNPCCombatFace;
		}
		break;
	case AI_SCHED_INSPECT_ENEMY_CORPSE:
		{
			if(IsInSquad() && !IsSquadLeader() || IsFollowing())
				return CBaseNPC::GetScheduleByIndex(AI_SCHED_IDLE_STAND);
			else
				return CBaseNPC::GetScheduleByIndex(AI_SCHED_INSPECT_ENEMY_CORPSE);
		}
		break;
	case AI_SQUADNPC_SCHED_SIGNAL_SUPPRESSING_FIRE:
		{
			// So we don't call this schedule again
			m_signalledSuppressingFire = true;
			return &scheduleSignalSuppressingFire;
		}
		break;
	case AI_SQUADNPC_SCHED_TAKE_COVER_FAILED:
		{
			if(CheckConditions(AI_COND_CAN_RANGE_ATTACK1))
				return GetScheduleByIndex(AI_SCHED_RANGE_ATTACK1);
			else
				return GetScheduleByIndex(AI_SCHED_FAIL);
		}
		break;
	case AI_SQUADNPC_SCHED_TAKE_COVER:
		{
			return &scheduleTakeCover;
		}
		break;
	default:
		return CBaseNPC::GetScheduleByIndex(scheduleIndex);
	}
}

//=============================================
// @brief Returns the squad leader
//
//=============================================
CBaseEntity* CSquadNPC::GetSquadLeader( void )
{
	if(m_squadLeaderNPC)
		return m_squadLeaderNPC;
	else
		return this;
}

//=============================================
// @brief Returns the squad slots
//
//=============================================
Uint64 CSquadNPC::GetSquadSlots( void )
{
	return m_squadSlots;
}

//=============================================
// @brief Returns the squad slots
//
//=============================================
void CSquadNPC::SetSquadSlots( Uint64 squadSlots )
{
	m_squadSlots = squadSlots;
}

//=============================================
// @brief Vacates the slot occupied by this NPC
//
//=============================================
void CSquadNPC::VacateSlot( void )
{
	if(m_mySlot == NPC_SLOT_NONE)
		return;

	CBaseEntity* pSquadLeader = GetSquadLeader();
	Uint64 squadLeaderSlots = pSquadLeader->GetSquadSlots();
	squadLeaderSlots &= ~m_mySlot;

	pSquadLeader->SetSquadSlots(squadLeaderSlots);
	m_mySlot = NPC_SLOT_NONE;
}

//=============================================
// @brief Tells if we can occupy a given slot, and occupies if possible
//
//=============================================
bool CSquadNPC::OccupySlot( Uint64 desiredSlots )
{
	if(!IsInSquad())
		return true;

	if(IsSquadSplitOnEnemies())
	{
		m_mySlot = NPC_SLOT_SQUAD_SPLIT;
		return true;
	}

	CBaseEntity* pSquadLeader = GetSquadLeader();

	Uint64 squadLeaderSlots = pSquadLeader->GetSquadSlots();
	if(!(desiredSlots ^ squadLeaderSlots))
		return false;

	for(Uint32 i = 0; i < NUM_SLOTS; i++)
	{
		Uint64 slotMask = (1<<i);
		if((desiredSlots & slotMask) && !(squadLeaderSlots & slotMask))
		{
			pSquadLeader->SetSquadSlots((squadLeaderSlots|slotMask));
			m_mySlot = slotMask;
			return true;
		}
	}

	return false;
}

//=============================================
// @brief Tells if the NPC is in a squad
//
//=============================================
bool CSquadNPC::IsInSquad( void ) const
{
	return (m_squadLeaderNPC) ? true : false;
}

//=============================================
// @brief Tries to recruit any squad members
//
//=============================================
Uint32 CSquadNPC::SquadRecruit( Float searchRadius, Uint32 maxMembers )
{
	if(IsInSquad())
		return 0;

	if(maxMembers < 2)
		return 0;

	m_squadLeaderNPC = this;
	Uint32 nbSquadMembers = 1;

	if(HasNetname())
	{
		edict_t* pedict = nullptr;
		while(true)
		{
			pedict = Util::FindEntityByString(pedict, "netname", GetNetname());
			if(!pedict)
				break;

			if(Util::IsNullEntity(pedict))
				continue;

			CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
			if(pEntity && pEntity->GetClassification() == GetClassification())
			{
				if(!AddToSquad(pEntity))
					break;

				nbSquadMembers++;
			}
		}
	}
	else
	{
		Vector mins, maxs;
		for(Uint32 i = 0; i < 3; i++)
		{
			mins[i] = m_pState->origin[i] - searchRadius;
			maxs[i] = m_pState->origin[i] + searchRadius;
		}

		edict_t* pedict = nullptr;
		while(true)
		{
			pedict = Util::FindEntityInBBox(pedict, mins, maxs);
			if(!pedict)
				break;

			if(Util::IsNullEntity(pedict))
				continue;

			CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
			if(pEntity 
				&& pEntity->IsSquadNPC() 
				&& pEntity->IsAlive() 
				&& pEntity->GetClassification() == GetClassification() 
				&& !pEntity->HasNetname())
			{
				trace_t tr;
				Util::TraceLine(GetEyePosition(), pEntity->GetEyePosition(), true, false, pEntity->GetEdict(), tr);
				if(tr.noHit())
				{
					if(!AddToSquad(pEntity))
						break;

					nbSquadMembers++;
				}
			}
		}
	}

	// Clear squadleader ptr if only one member is present
	if(nbSquadMembers <= 1)
		m_squadLeaderNPC.reset();

	return nbSquadMembers;
}

//=============================================
// @brief Returns the number of squad members
//
//=============================================
Uint32 CSquadNPC::GetNbSquadMembers( void )
{
	if(!IsInSquad())
		return 0;

	CBaseEntity* pSquadLeader = GetSquadLeader();
	if(!pSquadLeader)
	{
		Util::EntityConDPrintf(m_pEdict, "%s - Squad leader was null.\n", __FUNCTION__);
		return 0;
	}

	Uint32 memberCount = 0;
	for(Uint32 i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CBaseEntity* pMember = pSquadLeader->GetSquadMember(i);
		if(pMember)
			memberCount++;
	}

	return memberCount;
}

//=============================================
// @brief Removes a squad member
//
//=============================================
void CSquadNPC::RemoveNPCFromSquad( CBaseEntity* pRemoveNPC )
{
	if(!IsSquadLeader() || m_squadLeaderNPC != reinterpret_cast<const CBaseEntity*>(this))
	{
		Util::EntityConPrintf(m_pEdict, "%s - Can only be called by squad leader.\n", __FUNCTION__);
		return;
	}

	if(pRemoveNPC == GetSquadLeader())
	{
		// if the squad leader is removed, then disband the squad
		for(Uint32 i = 0; i < (MAX_SQUAD_MEMBERS-1); i++)
		{
			if(m_squadMembers[i])
			{
				m_squadMembers[i]->SetSquadLeader(nullptr);
				m_squadMembers[i].reset();
			}
		}
	}
	else
	{
		CBaseEntity* pSquadLeader = GetSquadLeader();
		if(pSquadLeader)
			pSquadLeader->RemoveSquadMember(this);
	}

	pRemoveNPC->SetSquadLeader(nullptr);
}

//=============================================
// @brief Sets the squad leader
//
//=============================================
void CSquadNPC::SetSquadLeader( CBaseEntity* pSquadLeader )
{
	if(!pSquadLeader)
		m_squadLeaderNPC.reset();
	else
		m_squadLeaderNPC = pSquadLeader;
}

//=============================================
// @brief Removes a member of the squad
//
//=============================================
void CSquadNPC::RemoveSquadMember( CBaseEntity* pRemoveNPC )
{
	for(Uint32 i = 0; i < (MAX_SQUAD_MEMBERS-1); i++)
	{
		if(m_squadMembers[i] == reinterpret_cast<const CBaseEntity*>(pRemoveNPC))
		{
			m_squadMembers[i].reset();
			return;
		}
	}
}

//=============================================
// @brief Sets the enemy for squad members
//
//=============================================
void CSquadNPC::SetSquadEnemy( CBaseEntity* pEnemy )
{
	if(!IsInSquad())
		return;

	if(!pEnemy)
	{
		Util::EntityConDPrintf(m_pEdict, "%s - Enemy was null.\n", __FUNCTION__);
		return;
	}
	
	if(pEnemy->GetClassification() == GetClassification())
		return;

	CBaseEntity* pSquadLeader = GetSquadLeader();
	if(!pSquadLeader)
	{
		Util::EntityConDPrintf(m_pEdict, "%s - Squad leader was null.\n", __FUNCTION__);
		return;
	}

	for(Uint32 i = 0; i < (MAX_SQUAD_MEMBERS-1); i++)
	{
		CBaseEntity* pMember = pSquadLeader->GetSquadMember(i);
		if(!pMember)
			continue;

		CBaseEntity* pMemberEnemy = pMember->GetEnemy();
		if(pMemberEnemy != pEnemy && !pMember->CheckConditions(AI_COND_SEE_ENEMY))
		{
			// Remember squad member's enemy if needed
			if(pMemberEnemy != nullptr)
			{
				// Remember previous enemy
				Vector enemyLKP, enemyLKA;
				pMember->GetEnemyInfo(enemyLKP, enemyLKA);
				pMember->PushEnemy(pMemberEnemy, enemyLKP, enemyLKA);
			}

			// Set new enemy
			pMember->SetEnemy(pEnemy);
			pMember->SetEnemyInfo(pEnemy->GetNavigablePosition(), pEnemy->GetAngles());
			pMember->SetConditions(AI_COND_NEW_ENEMY);

			if(pEnemy->IsPlayer())
				pMember->SetLastPlayerSightTime(g_pGameVars->time);
		}
	}
}

//=============================================
// @brief Pastes enemy info to other squad members
//
//=============================================
void CSquadNPC::SetSquadEnemyInfo( void )
{
	CBaseEntity* pLeader = GetSquadLeader();
	if(!pLeader)
		return;

	pLeader->SetEnemyInfo(m_enemyLastKnownPosition, m_enemyLastKnownAngles);
}

//=============================================
// @brief Copies enemy info for squad members
//
//=============================================
void CSquadNPC::CopySquadEnemyInfo( void )
{
	CBaseEntity* pLeader = GetSquadLeader();
	if(!pLeader)
		return;

	if(pLeader == this)
		return;

	Vector enemyPosition, enemyAngles;
	pLeader->GetEnemyInfo(enemyPosition, enemyAngles);

	if(CheckConditions(AI_COND_ENEMY_NOT_FOUND) 
		&& !pLeader->CheckConditions(AI_COND_ENEMY_NOT_FOUND))
		ClearConditions(AI_COND_ENEMY_NOT_FOUND);

	m_enemyLastKnownPosition = enemyPosition;
	m_enemyLastKnownAngles = enemyAngles;
}

//=============================================
// @brief Splits enemies to squad members
//
//=============================================
bool CSquadNPC::IsSquadSplitOnEnemies( void )
{
	if(!IsInSquad())
		return false;

	CBaseEntity* pSquadLeader = GetSquadLeader();
	if(!pSquadLeader)
	{
		Util::EntityConDPrintf(m_pEdict, "%s - Squad leader was null.\n", __FUNCTION__);
		return false;
	}

	for(Uint32 i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CBaseEntity* pMember = pSquadLeader->GetSquadMember(i);
		if(!pMember)
			continue;

		CBaseEntity* pMemberEnemy = pMember->GetEnemy();
		if(pMemberEnemy && pMemberEnemy != m_enemy)
			return true;
	}

	return false;
}

//=============================================
// @brief Tells if a squad member is in range
//
//=============================================
bool CSquadNPC::IsSquadMemberInRange( const Vector& position, Float distance )
{
	Vector mins, maxs;
	for(Uint32 i = 0; i < 3; i++)
	{
		mins[i] = position[i] - distance;
		maxs[i] = position[i] + distance;
	}

	CBaseEntity* pSquadLeader = GetSquadLeader();
	if(!pSquadLeader)
	{
		Util::EntityConDPrintf(m_pEdict, "%s - Squad leader was null.\n", __FUNCTION__);
		return false;
	}

	for(Uint32 i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CBaseEntity* pMember = pSquadLeader->GetSquadMember(i);
		if(!pMember || pMember == this)
			continue;

		if(!Math::CheckMinsMaxs(mins, maxs, pMember->GetAbsMins(), pMember->GetAbsMaxs()))
			return true;
	}

	return false;
}

//=============================================
// @brief Tells if any squad members see us
//
//=============================================
bool CSquadNPC::IsVisibleBySquadMembers( void )
{
	CBaseEntity* pSquadLeader = GetSquadLeader();
	if(!pSquadLeader)
	{
		Util::EntityConDPrintf(m_pEdict, "%s - Squad leader was null.\n", __FUNCTION__);
		return false;
	}

	trace_t tr;
	Vector start = GetEyePosition();
	for(Uint32 i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CBaseEntity* pMember = pSquadLeader->GetSquadMember(i);
		if(!pMember || pMember == this)
			continue;

		Vector end = pMember->GetEyePosition();
		Util::TraceLine(start, end, true, false, m_pEdict, tr);
		if(tr.noHit() && !tr.allSolid() && !tr.startSolid())
			return true;
	}

	return false;
}

//=============================================
// @brief Tells if we can toss a grenade
//
//=============================================
bool CSquadNPC::CheckGrenadeToss( Double& nextGrenadeCheckTime, bool &tossGrenade, Vector& grenadeTossVelocity )
{
	if(nextGrenadeCheckTime > g_pGameVars->time)
	{
		tossGrenade = false;
		return false;
	}

	// No grenade throw while moving
	if(IsMoving())
	{
		nextGrenadeCheckTime = g_pGameVars->time + 1;
		tossGrenade = false;
		return false;
	}

	// Check if there's a ceiling above us
	Vector ceilingCheckPos = m_pState->origin + Vector(0, 0, 255);
	
	trace_t tr;
	Util::TraceLine(m_pState->origin, ceilingCheckPos, true, false, m_pEdict, tr);
	if(!tr.noHit())
	{
		nextGrenadeCheckTime = g_pGameVars->time + 1;
		tossGrenade = false;
		return false;
	}

	if(m_enemy && !(m_enemy->GetFlags() & FL_ONGROUND) && m_enemy->GetWaterLevel() == WATERLEVEL_NONE && m_enemyLastKnownPosition.z > m_pState->absmax.z)
	{
		tossGrenade = false;
		return false;
	}

	Vector targetVector;
	if(CheckConditions(AI_COND_SEE_ENEMY) || Common::RandomLong(0, 1) == 1)
		targetVector = m_enemy->GetNavigablePosition();
	else
		targetVector = m_enemyLastKnownPosition;

	// Do not blow teammates up
	Float grenadeDamageRadius = gSkillData.GetSkillCVarSetting(g_skillcvars.skillGrenadeRadius);
	if(IsInSquad() && IsSquadMemberInRange(targetVector, grenadeDamageRadius))
	{
		nextGrenadeCheckTime = g_pGameVars->time + 1;
		tossGrenade = false;
		return false;
	}

	// Don't blow yourself up either
	if((m_pState->origin - targetVector).Length() <= grenadeDamageRadius)
	{
		nextGrenadeCheckTime = g_pGameVars->time + 1;
		tossGrenade = false;
		return false;
	}

	Vector tossVector;
	if(!Util::CheckToss(this, GetGunPosition(), targetVector, 0.5, tossVector))
	{
		nextGrenadeCheckTime = g_pGameVars->time + 1;
		tossGrenade = false;
		return false;
	}

	tossGrenade = true;
	nextGrenadeCheckTime = g_pGameVars->time;
	grenadeTossVelocity = tossVector;

	return true;
}

//=============================================
// @brief Returns the squad leader
//
//=============================================
CBaseEntity* CSquadNPC::GetSquadMember( Uint32 index )
{
	if(index >= (MAX_SQUAD_MEMBERS-1))
		return this;
	else
		return m_squadMembers[index];
}

//=============================================
// @brief Adds an NPC to the squad
//
//=============================================
bool CSquadNPC::AddToSquad( CBaseEntity* pAddNPC )
{
	if(!pAddNPC)
		return false;

	if(!IsSquadLeader())
	{
		Util::EntityConPrintf(m_pEdict, "%s - Only squad leaders can add new squad members.\n", __FUNCTION__);
		return false;
	}

	// Avoid adding a sqquad member twice
	for(Uint32 i = 0; i < (MAX_SQUAD_MEMBERS-1); i++)
	{
		if(m_squadMembers[i] == reinterpret_cast<const CBaseEntity*>(pAddNPC))
		{
			Util::EntityConPrintf(m_pEdict, "%s - Already a member.\n", __FUNCTION__);
			return true;
		}
	}

	for(Uint32 i = 0; i < (MAX_SQUAD_MEMBERS-1); i++)
	{
		// If a slot is empty, add a squad member
		if(!m_squadMembers[i])
		{
			m_squadMembers[i] = pAddNPC;
			pAddNPC->SetSquadLeader(this);
			return true;
		}
	}

	return false;
}

//=============================================
// @brief Performs pre-schedule think functions
//
//=============================================
void CSquadNPC::PreScheduleThink( void )
{
	if(IsInSquad() && m_enemy)
	{
		CBaseEntity* pLeader = GetSquadLeader();
		if(pLeader)
		{
			// Update the squad's last sighting time
			if(CheckConditions(AI_COND_SEE_ENEMY))
				pLeader->SetLastEnemySightTime(g_pGameVars->time);
			else if(!pLeader->HasEnemyEluded() && (g_pGameVars->time - pLeader->GetLastEnemySightTime()) > ENEMY_ELUDE_TIME)
				pLeader->SetEnemyEluded(true);
		}
	}

	CBaseNPC::PreScheduleThink();
}

//=============================================
// @brief Sets whether the enemy has eluded us
//
//=============================================
void CSquadNPC::SetEnemyEluded( bool enemyEluded )
{
	m_enemyEluded = enemyEluded;
}

//=============================================
// @brief Tells whether the enemy has eluded us
//
//=============================================
bool CSquadNPC::HasEnemyEluded( void )
{
	return m_enemyEluded;
}

//=============================================
// @brief Sets the NPC state
//
//=============================================
void CSquadNPC::SetNPCState( npcstate_t state )
{
	// Reset the signal boolean
	if((m_npcState == NPC_STATE_COMBAT || m_npcState == NPC_STATE_SCRIPT)
		&& (state == NPC_STATE_IDLE || state == NPC_STATE_ALERT))
		m_signalledSuppressingFire = false;

	CBaseNPC::SetNPCState(state);
}