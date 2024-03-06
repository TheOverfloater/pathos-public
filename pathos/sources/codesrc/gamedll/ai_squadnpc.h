/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef AI_SQUADNPC_H
#define AI_SQUADNPC_H

#include "ai_basenpc.h"

enum squadnpc_tasks_t
{
	// Must be last
	LAST_SQUADNPC_TASK = LAST_BASENPC_TASK + 1,
};

enum squadnpc_schedules_t
{
	AI_SQUADNPC_SCHED_FOUND_ENEMY = LAST_BASENPC_SCHEDULE + 1,
	AI_SQUADNPC_SCHED_COMBAT_FACE,
	AI_SQUADNPC_SCHED_SIGNAL_SUPPRESSING_FIRE,
	AI_SQUADNPC_SCHED_TAKE_COVER_FAILED,
	AI_SQUADNPC_SCHED_TAKE_COVER,

	// Must be last
	LAST_SQUADNPC_SCHEDULE
};

//=============================================
//
//=============================================
class CSquadNPC : public CBaseNPC
{
public:
	enum squadslots_t
	{
		NPC_SLOT_NONE			= 0,
		NPC_SLOT_ENGAGE1		= (1<<0),
		NPC_SLOT_ENGAGE2		= (1<<1),
		NPC_SLOT_ENGAGE			= (NPC_SLOT_ENGAGE1|NPC_SLOT_ENGAGE2),
		NPC_SLOT_GRENADE1		= (1<<2),
		NPC_SLOT_GRENADE2		= (1<<3),
		NPC_SLOT_GRENADE		= (NPC_SLOT_GRENADE1|NPC_SLOT_GRENADE2),
		NPC_SLOT_SQUAD_SPLIT	= (1<<4)
	};

public:
	// Number of slots
	static const Uint32 NUM_SLOTS;
	// Search radius for squad members to recruit
	static const Float SQUADINIT_SEARCH_RADIUS;
	// Radius to check for cover validation
	static const Float COVER_VALIDATE_RADIUS;
	// Elude time for enemies
	static const Float ENEMY_ELUDE_TIME;

public:
	// Maximum number of squad members
	static const Uint32 MAX_SQUAD_MEMBERS = 32;

public:
	explicit CSquadNPC( edict_t* pedict );
	virtual ~CSquadNPC( void );

public:
	// Initializes squad related data
	virtual void InitSquad( void ) override;
	// Checks the current enemy's state
	virtual bool CheckEnemy( void ) override;
	// Called when a schedule is changed
	virtual void OnScheduleChange( void ) override;
	// Manages getting killed
	virtual void Killed( CBaseEntity* pAttacker, gibbing_t gibbing, deathmode_t deathMode ) override;
	// Forgets the player
	virtual void ForgetPlayer( CBaseEntity* pPlayer ) override;
	// Declares save-restore fields
	virtual void DeclareSaveFields( void ) override;
	// Adds a new enemy
	virtual void PushEnemy( CBaseEntity* pEnemy, const Vector& lastPosition, const Vector& lastAngles ) override;
	// Allows squad members to validate a cover position
	virtual bool ValidateCover( const Vector& coverPosition ) override;
	// Returns the ideal NPC state
	virtual npcstate_t GetIdealNPCState( void ) override;
	// Returns a schedule by it's index
	virtual const CAISchedule* GetScheduleByIndex( Int32 scheduleIndex ) override;
	// Returns the squad leader
	virtual CBaseEntity* GetSquadLeader( void ) override;
	// Returns the squad slots
	virtual Uint64 GetSquadSlots( void ) override;
	// Returns the squad slots
	virtual void SetSquadSlots( Uint64 squadSlots ) override;
	// Removes a squad member
	virtual void RemoveNPCFromSquad( CBaseEntity* pRemoveNPC ) override;
	// Sets the squad leader
	virtual void SetSquadLeader( CBaseEntity* pSquadLeader ) override;
	// Removes a member of the squad
	virtual void RemoveSquadMember( CBaseEntity* pRemoveNPC ) override;
	// Sets the last enemy sight time
	virtual void SetLastEnemySightTime( Double time ) override;
	// Sets the last enemy sight time
	virtual Double GetLastEnemySightTime( void ) override;
	// Sets whether the enemy has eluded us
	virtual void SetEnemyEluded( bool enemyEluded ) override;
	// Tells whether the enemy has eluded us
	virtual bool HasEnemyEluded( void ) override;
	// Returns the squad leader
	virtual CBaseEntity* GetSquadMember( Uint32 index ) override;
	// Tells if the NPC is of squadNPC type
	virtual bool IsSquadNPC( void ) const override { return true; }
	// Tells if this npc is the leader
	virtual bool IsSquadLeader( void ) const override;
	// Adds an NPC to the squad
	virtual bool AddToSquad( CBaseEntity* pAddNPC ) override;
	// Performs pre-schedule think functions
	virtual void PreScheduleThink( void ) override;
	// Sets the NPC state
	virtual void SetNPCState( npcstate_t state ) override;

public:
	// Vacates the slot occupied by this NPC
	void VacateSlot( void );
	// Tells if we can occupy a given slot, and occupies if possible
	bool OccupySlot( Uint64 desiredSlots );

	// Tells if the NPC is in a squad
	bool IsInSquad( void ) const;

	// Tries to recruit any squad members
	Uint32 SquadRecruit( Float searchRadius, Uint32 maxMembers );
	// Returns the number of squad members
	Uint32 GetNbSquadMembers( void );
	// Sets the enemy for squad members
	void SetSquadEnemy( CBaseEntity* pEnemy );
	// Pastes enemy info to other squad members
	void SetSquadEnemyInfo( void );
	// Copies enemy info for squad members
	void CopySquadEnemyInfo( void );
	// Splits enemies to squad members
	bool IsSquadSplitOnEnemies( void );
	// Tells if a squad member is in range
	bool IsSquadMemberInRange( const Vector& position, Float distance );
	// Tells if any squad members see us
	bool IsVisibleBySquadMembers( void );
	// Tells if we can toss a grenade
	bool CheckGrenadeToss( Double& nextGrenadeCheckTime, bool &tossGrenade, Vector& grenadeTossVelocity );

protected:
	// Squad leader NPC
	CEntityHandle	m_squadLeaderNPC;
	// Squad member NPCs
	CEntityHandle	m_squadMembers[MAX_SQUAD_MEMBERS-1];

	// Squad slots
	Uint64			m_squadSlots;
	// Last enemy sight time
	Double			m_lastEnemySightTime;
	// TRUE if enemy eluded us
	bool			m_enemyEluded;

	// The squad NPC's slot
	Uint64			m_mySlot;
	// True if we signalled suppressing fire
	bool			m_signalledSuppressingFire;
};

#endif //AI_SQUADNPC_H