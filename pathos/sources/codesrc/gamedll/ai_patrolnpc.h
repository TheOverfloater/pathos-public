/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef AI_PATROLNPC_H
#define AI_PATROLNPC_H

#include "ai_squadnpc.h"

//=============================================
// Patrol npc specific tasks
//=============================================
enum patrolnpc_tasks_t
{
	AI_PATROLNPC_TASK_FIND_DEST = LAST_SQUADNPC_TASK + 1,
	AI_PATROLNPC_TASK_PATROL_DONE,
	AI_PATROLNPC_TASK_PATROL_FLUSH,
	AI_PATROLNPC_TASK_MOVE_BUDGE,

	// Must be last
	LAST_PATROLNPC_TASK
};

//=============================================
// Patrol npc specific schedules
//=============================================
enum patrolnpc_schedules_t
{
	AI_PATROLNPC_SCHED_PATROL = LAST_SQUADNPC_SCHEDULE + 1,
	AI_PATROLNPC_SCHED_PATROL_FAIL,
	AI_PATROLNPC_SCHED_PATROL_PROMPT,

	LAST_PATROLNPC_SCHEDULE
};

//=============================================
// Patrol npc specific ai conditions
//=============================================
enum patrolnpc_aiconditions_t
{
	AI_COND_SUSPICIOUS = AI_COND_PROVOKED
};

//=============================================
//
//=============================================
class CPatrolNPC : public CSquadNPC
{
public:
	// Max number of patrol failures before the patrol history is flushed
	static const Uint32 MAX_PATROL_FAILURES;
	// Minimum patrol distance
	static const Float MIN_PATROL_DISTANCE;
	// Minimum visible patrol distance
	static const Float MIN_VISIBLE_PATROL_DISTANCE;
	// Default patrol radius
	static const Float DEFAULT_PATROL_RADIUS;
	// Maximum distance a destination can have from the player's position
	static const Float MAX_PATROL_DEST_PLAYER_DISTANCE;
	// Minimum distance from a budge attempt we'll accept
	static const Float MIN_BUDGE_MOVE_DISTANCE;
	// Distance to which we'll try to budge out of a stuck spot
	static const Float BUDGE_MOVE_DISTANCE;

public:
	// Maximum patrol history size
	static const Uint32 MAX_PATROL_HISTORY = 32;

public:
	explicit CPatrolNPC( edict_t* pedict );
	virtual ~CPatrolNPC( void );

public:
	// Declares save-restore fields
	virtual void DeclareSaveFields( void ) override;
	// Manages keyvalues
	virtual bool KeyValue( const keyvalue_t& kv ) override;

	// Initializes the NPC
	virtual void InitNPC( void ) override;

	// Validates whether an intended patrol destination should be visited
	virtual bool ValidatePatrolDestination( const Vector& destPosition ) override;
	// Pushes a patrol destination to the history stack
	virtual void PushPatrolDestination( const Vector& destPosition ) override;
	// Prompts a patrol error
	virtual void PatrolErrorPrompt( CBaseEntity* pEntity ) override;
	// Returns the last node index a squad member chose to visit
	virtual void SetLastPatrolNodeIndex( Int32 nodeIndex ) override;
	// Returns the last node index a squad member chose to visit
	virtual Int32 GetLastPatrolNodeIndex( void ) override;
	// Clears patrol history stack
	virtual void ClearPatrolHistory( void ) override;

	// Returns the ideal schedule
	virtual const CAISchedule* GetSchedule( void ) override;
	// Returns a schedule by it's index
	virtual const CAISchedule* GetScheduleByIndex( Int32 scheduleIndex ) override;

	// Cleans up the current scripted_sequence
	virtual void CleanupScriptedSequence( void ) override;

	// Starts a task
	virtual void StartTask( const ai_task_t* pTask ) override;

	// Set by an NPC if this NPC is blocking them
	virtual void SetPathBlocked( CBaseEntity* pBlockedEntity, const Vector& destination ) override;

	// Sets the NPC state
	virtual void SetNPCState( npcstate_t state ) override;

public:
	// Returns the minimum patrol distance for non-visible destinations
	virtual Float GetMinimumPatrolDistance( void ) const;
	// Reurns the minimum patrol distance for visible destinations
	virtual Float GetMinimumVisiblePatrolDistance( void ) const;

public:
	// Builds a patrol path
	bool BuildPatrolPath( Float minDistance, Float maxDistance );

protected:
	// Tells if NPC should patrol(so non-patrolling npcs that still inherit from this class can override it)
	virtual bool ShouldPatrol( void );

protected:
	// Tells if the entity should patrol actively
	bool m_shouldPatrol;

	// History of patrol destinations
	Vector m_patrolHistoryArray[MAX_PATROL_HISTORY];
	// Number of entries in patrol history
	Uint32 m_numPatrolHistory;

	// Entities that prompted an error
	CEntityHandle m_squadMembersWithErrorsArray[MAX_SQUAD_MEMBERS-1];
	// Number of error prompts
	Uint32 m_numPatrolErrorPrompts;

	// Last patrol node of squad leader
	Int32 m_lastPatrolNodeIndex;

	// My last patrol node index
	Int32 m_myLastPatrolNodeIndex;
	// Number of patrol failures
	Uint32 m_numPatrolFailures;
	// Next patrol time
	Double m_nextPatrolTime;
	// Max patrol radius
	Float m_patrolRadius;
	// Node region I am restricted to
	string_t m_nodeRegionName;
};
#endif //AI_PATROLNPC_H