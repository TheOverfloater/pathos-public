/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

//
// I'd like to thank Valve for the AI code in the Half-Life SDK,
// which I used as a reference while writing this implementation
//

#ifndef AI_BASENPC_H
#define AI_BASENPC_H


#include "animatingentity.h"
#include "ai_sounds.h"
#include "ai_schedule.h"
#include "ai_schedules.h"
#include "scriptedsequence.h"
#include "ai_common.h"
#include "weapons_shared.h"

// AI Conditions can't be enums, because they go over 32 bits
static const Uint64 AI_COND_NONE						= 0;
static const Uint64 AI_COND_NO_AMMO_LOADED				= (1ULL<<0);
static const Uint64 AI_COND_SEE_HATE					= (1ULL<<1);
static const Uint64 AI_COND_SEE_FEAR					= (1ULL<<2);
static const Uint64 AI_COND_SEE_DISLIKE					= (1ULL<<3);
static const Uint64 AI_COND_SEE_ENEMY					= (1ULL<<4);
static const Uint64 AI_COND_ENEMY_OCCLUDED				= (1ULL<<5);
static const Uint64 AI_COND_SMELL_FOOD					= (1ULL<<6);
static const Uint64 AI_COND_ENEMY_TOO_FAR				= (1ULL<<7);
static const Uint64 AI_COND_LIGHT_DAMAGE				= (1ULL<<8);
static const Uint64 AI_COND_HEAVY_DAMAGE				= (1ULL<<9);
static const Uint64 AI_COND_CAN_RANGE_ATTACK1			= (1ULL<<10);
static const Uint64 AI_COND_CAN_RANGE_ATTACK2			= (1ULL<<11);
static const Uint64 AI_COND_CAN_MELEE_ATTACK1			= (1ULL<<12);
static const Uint64 AI_COND_CAN_MELEE_ATTACK2			= (1ULL<<13);
static const Uint64 AI_COND_FOLLOW_TARGET_TOO_FAR		= (1ULL<<14);
static const Uint64 AI_COND_PROVOKED					= (1ULL<<15);
static const Uint64 AI_COND_NEW_ENEMY					= (1ULL<<16);
static const Uint64 AI_COND_HEAR_SOUND					= (1ULL<<17);
static const Uint64 AI_COND_SMELL						= (1ULL<<18);
static const Uint64 AI_COND_ENEMY_FACING_ME				= (1ULL<<19);
static const Uint64 AI_COND_ENEMY_DEAD					= (1ULL<<20);
static const Uint64 AI_COND_SEE_CLIENT					= (1ULL<<21);
static const Uint64 AI_COND_SEE_NEMESIS					= (1ULL<<22);
static const Uint64 AI_COND_ENEMY_NOT_FOUND				= (1ULL<<23);
static const Uint64 AI_COND_ENEMY_UNREACHABLE			= (1ULL<<24);
static const Uint64 AI_COND_IN_DANGER					= (1ULL<<25);
static const Uint64 AI_COND_DANGEROUS_ENEMY_CLOSE		= (1ULL<<26);
static const Uint64 AI_COND_BLOCKING_PATH				= (1ULL<<27);
static const Uint64 AI_COND_TASK_FAILED					= (1ULL<<28);
static const Uint64 AI_COND_SCHEDULE_DONE				= (1ULL<<29);
static const Uint64 AI_COND_CLIENT_UNSEEN				= (1ULL<<30);
static const Uint64 AI_COND_PLAYER_CLOSE				= (1ULL<<31);
static const Uint64 AI_COND_FRIENDLY_FIRE				= (1ULL<<32);
static const Uint64 AI_COND_ENEMY_NAVIGABLE				= (1ULL<<33);
static const Uint64 AI_COND_ELOF_FAILED					= (1ULL<<34);
static const Uint64 AI_COND_HEARD_ENEMY_NEW_POSITION	= (1ULL<<35);
static const Uint64 AI_COND_RESERVED1					= (1ULL<<36);
static const Uint64	AI_COND_RESERVED2					= (1ULL<<37);
static const Uint64 AI_COND_RESERVED3					= (1ULL<<38);
static const Uint64	AI_COND_RESERVED4					= (1ULL<<39);
static const Uint64	AI_COND_NOT_ONGROUND				= (1ULL<<40);
static const Uint64 AI_COND_SHOOT_VECTOR_VALID			= (1ULL<<41);
static const Uint64 AI_COND_CAN_SPECIAL_ATTACK1			= (1ULL<<42);
static const Uint64 AI_COND_CAN_SPECIAL_ATTACK2			= (1ULL<<43);
static const Uint64 AI_COND_SEE_HOSTILE_NPC				= (1ULL<<44);
static const Uint64 AI_COND_CAN_ATTACK					= (AI_COND_CAN_RANGE_ATTACK1|AI_COND_CAN_RANGE_ATTACK2|AI_COND_CAN_MELEE_ATTACK1|AI_COND_CAN_MELEE_ATTACK2|AI_COND_CAN_SPECIAL_ATTACK1|AI_COND_CAN_SPECIAL_ATTACK2);
static const Uint64 AI_SCRIPT_BREAK_CONDITIONS			= (AI_COND_LIGHT_DAMAGE|AI_COND_HEAVY_DAMAGE|AI_COND_HEAR_SOUND|AI_COND_SEE_ENEMY|AI_COND_SEE_HOSTILE_NPC);

enum ai_memory_t
{
	AI_MEMORY_PROVOKED				= (1<<0),
	AI_MEMORY_IN_COVER				= (1<<1),
	AI_MEMORY_SUSPICIOUS			= (1<<2),
	AI_MEMORY_PATH_FINISHED			= (1<<3),
	AI_MEMORY_ON_PATH				= (1<<4),
	AI_MEMORY_MOVE_FAILED			= (1<<5),
	AI_MEMORY_FLINCHED				= (1<<6),
	AI_MEMORY_KILLED				= (1<<7),
	AI_MEMORY_SOUGHT_TACTICAL		= (1<<8),
	AI_MEMORY_DODGE_ENEMY_FAILED	= (1<<9),
	AI_MEMORY_SAW_MEDKIT			= (1<<10),
	AI_MEMORY_CHECKING_DANGERS		= (1<<12),
	AI_MEMORY_SAW_NPC				= (1<<13),
	AI_MEMORY_SAW_PLAYER			= (1<<14),
	AI_MEMORY_SIGNALLED				= (1<<15),
	AI_MEMORY_DISTURBED				= (1<<16),
	AI_MEMORY_HIDING_SPOT_NOT_FOUND	= (1<<17)
};

enum bodytarget_t
{
	BODYTARGET_CENTER = 0,
	BODYTARGET_HEAD,
	BODYTARGET_LEGS
};

//=============================================
//
//=============================================
class CBaseNPC : public CAnimatingEntity
{
public:
	// Delay between npc thinking
	static const Float NPC_THINK_TIME;
	// Head turn speed on yaw
	static const Float NPC_HEAD_TURN_YAW_SPEED;
	// Head turn speed on pitch
	static const Float NPC_HEAD_TURN_PITCH_SPEED;
	// Maximum looking distance
	static const Float NPC_DEFAULT_MAX_LOOK_DISTANCE;
	// Maximum firing distance
	static const Float NPC_DEFAULT_MAX_FIRING_DISTANCE;
	// Default hearing sensitivity
	static const Float NPC_DEFAULT_HEARING_SENSITIVITY;
	// Hearing lean awareness gain
	static const Float NPC_HEAR_LEAN_AWARENESS_GAIN;
	// Lean awareness timeout
	static const Float NPC_LEANAWARENESS_TIMEOUT;
	// NPC step size
	static const Float NPC_STEP_SIZE;
	// Maximum danger exposure time
	static const Float NPC_MAX_DANGER_TIME;
	// Minimum enemy distance
	static const Float NPC_MINIMUM_ENEMY_DISTANCE;
	// Enemy intercept distance
	static const Float NPC_ENEMY_INTERCEPT_DISTANCE;
	// Enemy combat state timeout
	static const Float NPC_COMBATSTATE_TIMEOUT;
	// Distance beyond which we'll try to cut corners
	static const Float NPC_CORNER_CUT_MIN_DIST;
	// Enemy update distance
	static const Float NPC_ENEMY_UPDATE_DISTANCE;
	// Triangulation maximum height
	static const Float NPC_TRIANGULATION_MAX_HEIGHT;
	// Triangulation minimum x size
	static const Float NPC_TRIANGULATION_MIN_SIZE_X;
	// Triangulation maximum x size
	static const Float NPC_TRIANGULATION_MAX_SIZE_X;
	// Door search radius when looking for double doors
	static const Float NPC_DOOR_SEARCH_RADIUS;
	// Minimum localmove check distance
	static const Float MIN_LOCALMOVE_CHECK_DIST;
	// Maximum distance the NPC can simplify paths in
	static const Float NPC_MAX_SIMPLIFY_DISTANCE;
	// Maximum traces an NPC can do while simplifying routes
	static const Uint32 NPC_MAX_SIMPLIFY_TRACES;
	// Distance between corner cut checks
	static const Float NPC_SIMPLIFICATION_FIX_DISTANCE;
	// NPC error glow aura color
	static const Vector NPC_ERROR_GLOW_AURA_COLOR;
	// Default max distance for navigation
	static const Float NPC_MAX_NAVIGATION_DISTANCE;
	// Dangerous enemy minimum distance
	static const Float NPC_DANGEROUS_ENEMY_MIN_DISTANCE;
	// Dangerous enemy minimum cover distance
	static const Float NPC_DANGEROUS_ENEMY_MIN_COVER_DISTANCE;
	// Enemy search distance
	static const Float NPC_MAX_ENEMY_SEARCH_DISTANCE;
	// Number of lateral cover checks
	static const Float NPC_LATERAL_COVER_CHECK_NUM;
	// Lateral cover check distance
	static const Float NPC_LATERAL_COVER_CHECK_DISTANCE;
	// NPC follow walk distance
	static const Float NPC_FOLLOW_WALK_DISTANCE;
	// NPC follow run distance
	static const Float NPC_FOLLOW_RUN_DISTANCE;
	// NPC default cover distance
	static const Float NPC_DEFAULT_COVER_DISTANCE;
	// NPC reload cover distance
	static const Float NPC_RELOAD_COVER_DISTANCE;
	// Best sound cover minimum distance
	static const Float NPC_COVER_BESTSOUND_MIN_DISTANCE;
	// Best sound cover maximum distance
	static const Float NPC_COVER_BESTSOUND_MAX_DISTANCE;
	// Best sound cover optimal distance
	static const Float NPC_COVER_BESTSOUND_OPTIMAL_DISTANCE;
	// NPC move wait time
	static const Float NPC_DEFAULT_MOVE_WAIT_TIME;
	// Dodge min distance
	static const Float NPC_DODGE_MIN_DISTANCE;
	// Dodge max distance
	static const Float NPC_DODGE_MAX_DISTANCE;
	// Minimum health value for gibbing
	static const Float NPC_GIB_HEALTH_VALUE;
	// Bullet gibbing damage treshold
	static const Float NPC_BULLETGIB_DMG_TRESHOLD;
	// Bullet gibbing minimum health treshold
	static const Float NPC_BULLETGIB_MIN_HEALTH;
	// Light damage treshold
	static const Float NPC_LIGHT_DAMAGE_TRESHOLD;
	// Heavy damage treshold
	static const Float NPC_HEAVY_DAMAGE_TRESHOLD;
	// Default lean awareness time
	static const Float NPC_DEFAULT_LEAN_AWARE_TIME;
	// Script move minimum distance
	static const Float NPC_SCRIPT_MOVE_MIN_DIST;
	// NPC gun sound radius
	static const Float NPC_GUN_SOUND_RADIUS;
	// Minimum size of an enemy the NPC will kick
	static const Float NPC_ENEMY_MIN_KICK_SIZE;
	// NPC firing angle treshold
	static const Float NPC_FIRING_ANGLE_TRESHOLD;
	// Max localmove height diff in start and end
	static const Float NPC_MAX_LOCALMOVE_HEIGHT_DIFF;
	// Distance at which we can be decapitated
	static const Float NPC_DECAP_MAX_DISTANCE;
	// Distance at which we can be gibbed by a bullet
	static const Float NPC_BULLETGIB_MAX_DISTANCE;
	// Number of coverage checks
	static const Uint32 NPC_NUM_COVERAGE_CHECKS;
	// Max number of schedule changes per think
	static const Uint32 NPC_MAX_SCHEDULE_CHANGES;
	// Max number of tasks executed
	static const Uint32 NPC_MAX_TASK_EXECUTIONS;
	// Navigability minimum distance change
	static const Float NAVIGABILITY_CHECK_MIN_DISTANCE_CHANGE;

public:
	// Max backed up enemies
	static const Uint32 MAX_BACKED_UP_ENEMIES = 4;

public:
	enum
	{
		FL_NPC_WAIT_TILL_SEEN		= (1<<0),
		FL_NPC_GAG					= (1<<1),
		FL_NPC_USE_NPC_CLIP			= (1<<2),
		FL_NPC_DONT_FORGET_PLAYER	= (1<<3),
		FL_NPC_PRISONER				= (1<<4),
		FL_NPC_SQUADLEADER			= (1<<5),
		FL_NPC_NO_PUSHING			= (1<<6),
		FL_NPC_WAIT_FOR_SCRIPT		= (1<<7),
		FL_NPC_IDLE					= (1<<8),
		FL_NPC_FADE_CORPSE			= (1<<9),
		FL_NPC_IMMORTAL				= (1<<10),
		FL_NPC_DONT_FALL			= (1<<11),
		FL_NPC_FALL_TO_GROUND		= (1<<12)
	};
	enum aitrigger_t
	{
		AI_TRIGGER_NONE = 0,
		AI_TRIGGER_SEE_PLAYER_ANGRY_AT_PLAYER,
		AI_TRIGGER_TAKEDAMAGE,
		AI_TRIGGER_HALF_HEALTH,
		AI_TRIGGER_DEATH,
		AI_TRIGGER_SQUADMEMBER_DIE,
		AI_TRIGGER_SQUADLEADER_DIE,
		AI_TRIGGER_HEAR_WORLD,
		AI_TRIGGER_HEAR_PLAYER,
		AI_TRIGGER_HEAR_COMBAT,
		AI_TRIGGER_SEE_PLAYER_UNCONDITIONAL,
		AI_TRIGGER_SEE_PLAYER_NOT_IN_COMBAT,
		AI_TRIGGER_SEE_ENEMY
	};
	enum ai_capabilities_t
	{
		AI_CAP_NONE				= 0,
		AI_CAP_DUCK				= (1<<0),
		AI_CAP_JUMP				= (1<<1),
		AI_CAP_STRAFE			= (1<<2),
		AI_CAP_SQUAD			= (1<<3),
		AI_CAP_SWIM				= (1<<4),
		AI_CAP_CLIMB			= (1<<5),
		AI_CAP_USE				= (1<<6),
		AI_CAP_HEAR				= (1<<7),
		AI_CAP_AUTO_OPEN_DOORS	= (1<<8),
		AI_CAP_OPEN_DOORS		= (1<<9),
		AI_CAP_RANGE_ATTACK1	= (1<<10),
		AI_CAP_RANGE_ATTACK2	= (1<<11),
		AI_CAP_MELEE_ATTACK1	= (1<<12),
		AI_CAP_MELEE_ATTACK2	= (1<<13),
		AI_CAP_FLY				= (1<<14),
		AI_CAP_EXPRESSIONS		= (1<<17),
		AI_CAP_USE_MEDKITS		= (1<<18),
		AI_CAP_TURN_HEAD		= (1<<19),
		AI_CAP_TURN_HEAD_PITCH	= (1<<20),
		AI_CAP_ATTACK_BLEND_SEQ	= (1<<21),
		AI_CAP_SPECIAL_ATTACK1	= (1<<22),
		AI_CAP_SPECIAL_ATTACK2	= (1<<23),

		AI_CAP_GROUP_DOORS		= (AI_CAP_USE|AI_CAP_AUTO_OPEN_DOORS|AI_CAP_OPEN_DOORS)
	};
	enum movementflag_t
	{
		MF_NONE					= 0,
		MF_TO_TARGETENT			= (1<<0),
		MF_TO_ENEMY				= (1<<1),
		MF_TO_COVER				= (1<<2),
		MF_TO_DETOUR			= (1<<3),
		MF_TO_PATH_CORNER		= (1<<4),
		MF_TO_NODE				= (1<<5),
		MF_TO_LOCATION			= (1<<6),
		MF_IS_GOAL				= (1<<7),
		MF_DONT_SIMPLIFY		= (1<<8),
		MF_APPROXIMATE			= (1<<9),
		MF_DETOUR_PATH			= (1<<10),
		MF_CIRCLE_PATH			= (1<<11),

		MF_NOT_TO_MASK			= (MF_IS_GOAL|MF_DONT_SIMPLIFY|MF_DETOUR_PATH)
	};
	enum movegoal_t
	{
		MOVE_GOAL_NONE			= 0,
		MOVE_GOAL_TARGET_ENT	= (MF_TO_TARGETENT),
		MOVE_GOAL_ENEMY			= (MF_TO_ENEMY),
		MOVE_GOAL_PATH_CORNER	= (MF_TO_PATH_CORNER),
		MOVE_GOAL_LOCATION		= (MF_TO_LOCATION),
		MOVE_GOAL_NODE			= (MF_TO_NODE)
	};
	enum taskstatus_t
	{
		TASK_STATUS_NEW = 0,
		TASK_STATUS_RUNNING,
		TASK_STATUS_RUNNING_MOVEMENT,
		TASK_STATUS_RUNNING_TASK,
		TASK_STATUS_COMPLETE,
		TASK_STATUS_FAILED,
		TASK_STATUS_FAILED_NO_RETRY
	};
	enum npc_sightbits_t
	{
		AI_SIGHTED_NOTHING				= 0,
		AI_SIGHTED_PLAYER_FULL			= (1<<0),
		AI_SIGHTED_PLAYER_LEAN			= (1<<1),
		AI_SIGHTED_PLAYER_CLOSE			= (1<<2),
		AI_SIGHTED_PLAYER_FLASHLIGHT	= (1<<3),
		AI_SIGHTED_PLAYER_FAR			= (1<<4),
		AI_SIGHTED_PLAYER_PARTIAL		= (1<<5),
		AI_SIGHTED_NPC					= (1<<6),
		AI_SIGHTED_NPC_CLOSE			= (1<<7),
		AI_SIGHTED_NPC_FAR				= (1<<8),
		AI_SIGHTED_NPC_PARTIAL			= (1<<9)
	};
	enum npc_animevent_t
	{
		NPC_AE_DEAD					= 1000,
		NPC_AE_NO_INTERRUPT,
		NPC_AE_CAN_INTERRUPT,
		NPC_AE_FIRE_EVENT,
		NPC_AE_SOUND,
		NPC_AE_SENTENCE,
		NPC_AE_IN_AIR,
		NPC_AE_END_ANIMATION,
		NPC_AE_SOUND_VOICE,
		NPC_AE_SENTENCE_RANDOM1,
		NPC_AE_NOT_DEAD,
		NPC_AE_STEP,
		NPC_AE_BODYDROP_LIGHT = 2001,
		NPC_AE_BODYDROP_HEAVY,
		NPC_AE_SWISH_SOUND = 2010,
		NPC_AE_SETBODYGROUP
	};

	enum advance_result_t
	{
		ADVANCE_RESULT_SUCCESS = 0,
		ADVANCE_RESULT_FAILED,
		ADVANCE_RESULT_REACHED_GOAL
	};

protected:
	struct enemy_info_t
	{
		CEntityHandle enemy;
		Vector lastknownorigin;
		Vector lastknownangles;
	};

	struct route_point_t
	{
		route_point_t():
			type(MF_NONE),
			nodeindex(NO_POSITION)
			{}

		Vector position;
		Int32 type;
		Int32 nodeindex;
	};

	struct cleardamage_t
	{
		cleardamage_t():
			dmgbit(0),
			time(0)
			{}

		Int32 dmgbit;
		Double time;
	};

public:
	explicit CBaseNPC( edict_t* pedict );
	virtual ~CBaseNPC( void );

public:
	// Declares save-restore fields
	virtual void DeclareSaveFields( void ) override;
	// Manages keyvalues
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	// Restores the entity from a save file
	virtual bool Restore( void ) override;
	// Performs precache functions
	virtual void Precache( void ) override;
	// Tells if entity should set bounds on restore
	virtual bool ShouldSetBoundsOnRestore( void ) override { return false; }
	// Tells object's usability type
	virtual usableobject_type_t GetUsableObjectType( void ) override { return USABLE_OBJECT_NONE; }

	// Gets the relationship with the other entity
	virtual Int32 GetRelationship( CBaseEntity* pOther ) override;

	// Tells if an entity is in view of this NPC
	virtual bool IsInView( CBaseEntity* pOther ) const override;
	// Tells if a position is in view of this NPC
	virtual bool IsInView( const Vector& position ) const override;
	// Tells if this NPC is moving
	virtual bool IsMoving( void ) const override;
	// Tells if the other npc is an enemy of this NPC
	virtual bool IsEnemyOf( CBaseEntity* pNPC ) const override;
	// Tells if this NPC is aware of the other npc
	virtual bool IsAwareOf( CBaseEntity* pNPC ) const override;
	// Tells if the entity is an NPC
	virtual bool IsNPC( void ) const override { return true; }
	// Tells if the entity is alive
	virtual bool IsAlive( void ) const override { return (m_pState->deadstate == DEADSTATE_NONE && m_npcState != NPC_STATE_DEAD) ? true : false; };

	// Nudges an NPC off the ground by a few units, then puts them on the floor
	virtual void GroundEntityNudge( bool noExceptions = false ) override;
	
	// Handles an animation event
	virtual void HandleAnimationEvent( const mstudioevent_t* pevent ) override;

	// TRUE if a black hole can pull this entity
	virtual bool CanBlackHolePull( void ) const override;

	// Set by an NPC if this NPC is blocking them
	virtual void SetPathBlocked( CBaseEntity* pBlockedEntity, const Vector& destination ) override;
	// Returns the ideal NPC state
	virtual npcstate_t GetIdealNPCState( void ) override;
	// Sets the ideal NPC state
	virtual void SetIdealNPCState( npcstate_t state ) override;
	// Returns the current NPC state
	virtual npcstate_t GetNPCState( void ) override;

	// Manages getting killed
	virtual void Killed( CBaseEntity* pAttacker, gibbing_t gibbing, deathmode_t deathMode ) override;
	// Takes some amount of health
	virtual bool TakeHealth( Float amount, Int32 damageFlags ) override;

	// Manages taking damage
	virtual bool TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags ) override;
	// Manages a traceline from an attack
	virtual void TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags ) override;
	// Returns blood color setting
	virtual bloodcolor_t GetBloodColor( void ) override;

	// Returns the current enemy
	virtual CBaseEntity* GetEnemy( void ) const override;
	// Returns the entity's targeting origin
	virtual Vector GetBodyTarget( const Vector& targetingPosition ) override;
	// Tells if entity can be auto-aimed
	virtual bool IsAutoAimable( CBaseEntity* pAimerEntity ) override;

	// Performs a localmove check
	// Note: Keep startPosition as not a reference, because CheckLocalMove modifies m_pState->origin, which is used sometimes as a parameter
	virtual localmove_t CheckLocalMove( const Vector startPosition, const Vector& endPosition, const CBaseEntity* pTargetEntity, Float* pDistance = nullptr, bool isInitial = false ) override;

	// Sets the current script state
	virtual void SetScriptState( scriptstate_t state ) override;
	// Returns the current script state
	virtual scriptstate_t GetScriptState( void ) const override;
	// Returns the scripted_sequence controlling this entity
	virtual CScriptedSequence* GetScriptedSequence( void ) override;
	// Returns the entity's target entity
	virtual CBaseEntity* GetTargetEntity( void ) override;

	// Tells if the NPC can play a scripted_sequence
	virtual bool CanPlaySequence( bool disregardState, script_interrupt_level_t interruptLevel ) override;
	// Tells if the NPC can play a scripted_sentence
	virtual bool CanPlaySentence( bool disregardState ) override;

	// Sets the goal entity of this NPC
	virtual void SetGoalEntity( CBaseEntity* pEntity ) override;
	// Sets the script entity for this NPC
	virtual void SetScriptEntity( CScriptedSequence* pScriptEntity ) override;
	// Sets the target entity of this NPC
	virtual void SetTargetEntity( CBaseEntity* pEntity ) override;

	// Sets the activity time
	virtual void SetLastActivityTime( Double time ) override;
	// Sets the current activity
	virtual void SetCurrentActivity( activity_t activity ) override;
	// Returns the ideal activity
	virtual Int32 GetIdealActivity( void ) override;

	// Exits the current scripted_sequence
	virtual void ExitScriptedSequence( void ) override;
	// Cleans up the current scripted_sequence
	virtual void CleanupScriptedSequence( void ) override;

	// Clears the schedule
	virtual void ClearSchedule( void ) override;

	// Plays a sentence
	virtual void PlaySentence( const Char* pstrSentenceName, Float duration, Float volume, Float attenuation, Float timeOffset, bool subtitleOnlyInRadius, CBaseEntity* pPlayer = nullptr );
	// Plays a scripted_sentence
	virtual void PlayScriptedSentence( const Char* pstrSentenceName, Float duration, Float volume, Float attenuation, Float timeOffset, bool subtitleOnlyInRadius, bool isConcurrent, CBaseEntity* pListener, CBaseEntity* pPlayer = nullptr ) override;

	// Sets the enemy's last known position and angles
	virtual void SetEnemyInfo( const Vector& enemyLKP, const Vector& enemyLKA ) override;
	// Gets the enemy information
	virtual void GetEnemyInfo( Vector& enemyLKP, Vector& enemyLKA ) override;
	// Sets the last time the player was sighted
	virtual void SetLastPlayerSightTime( Double time ) override;

	// Clears AI conditions
	virtual void ClearConditions( Uint64 conditionBits ) override;
	// Sets AI conditions
	virtual void SetConditions( Uint64 conditionBits ) override;
	// Checks conditions
	virtual bool CheckConditions( Uint64 conditionBits ) const override;

	// Set memory bits
	virtual void SetMemory( Uint64 memoryBits ) override;

	// Adds a new enemy
	virtual void PushEnemy( CBaseEntity* pEnemy, const Vector& lastPosition, const Vector& lastAngles ) override;
	// Sets the current enemy
	virtual void SetEnemy( CBaseEntity* pEnemy ) override;

	// Forgets the player
	virtual void ForgetPlayer( CBaseEntity* pPlayer ) override;

	// Changes the schedule to a new schedule
	virtual void ChangeSchedule( const CAISchedule* pNewSchedule ) override;

	// Reports the current AI state
	virtual void ReportAIState( void ) override;

	// Sets a trigger condition
	virtual void SetAITriggerCondition( Int32 conditionIndex, Int32 condition, const Char* pstrTarget ) override;

	// Tells if weapons can be dropped
	virtual bool CanDropWeapons( void ) const override;
	// Sets whether npc can drop weapons
	virtual void SetCanDropWeapons( bool canDrop ) override;

	// Returns the squad leader
	virtual CBaseEntity* GetSquadLeader( void ) override { return this; };

	// Calls to gib the NPC
	virtual void CallGibNPC( void ) override;
	// Callback function for spawning gibs
	virtual void OnGibSpawnCallback( CBaseEntity* pGib ) override;

	// Return damage multiplier for hitgroup
	virtual Float GetHitgroupDmgMultiplier( Int32 hitgroup ) override;

	// Sets the NPC to be pulled by a trigger_pullnpc
	virtual void SetNPCPuller( CBaseEntity* pPuller, const Vector& pullPosition ) override;
	
public:
	// Plays a flex script
	void PlayFlexScript( const Char* pstrSentenceName );

	// Stops a sentence playback
	void StopSentence( void );

	// Takes damage when dead
	bool TakeDamageDead( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags );
	// Decapitates the NPC
	void DecapitateNPC( bool spawnGib, Int32 bodyGroup, Int32 bodyNumber );
	// Tells if the NPC should be gibbed
	bool ShouldGibNPC( gibbing_t gibFlag ) const;


protected:
	// Calls for the NPC think code
	void EXPORTFN CallNPCThink( void );
	// Called when NPC corpse is falling
	void EXPORTFN CorpseFallThink( void );
	// Calls for the NPC think code
	void EXPORTFN CallNPCDeadThink( void );

protected:
	// Performs NPC think functions
	void NPCThink( void );
	// Performs NPC think functions
	void NPCDeadThink( void );

	// Updates head controllers
	void UpdateHeadControllers( void );
	// Updates idle animation
	void UpdateIdleAnimation( void );
	// Stops an animation
	void StopAnimation( void );

protected:
	// Initializes a dead NPC
	void InitDeadNPC( void );
	// Starts the NPC
	void StartNPC( void );
	// Resets the NPC
	void ResetNPC( void );
	// Turns the NPC into the dead state
	void BecomeDead( bool startedDead );

	// Calls for the NPC initialization code
	void EXPORTFN NPCInitThink( void );
	// EXPORT function for NPC use call
	void EXPORTFN NPCUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value );

	// Tells if a task is running
	bool IsTaskRunning( void ) const;
	// Tells if a task is complete
	bool IsTaskComplete( void ) const;
	// Returns the current task
	const ai_task_t* GetTask();
	// Begins a new task
	void TaskBegin( void );

	// Tells if movement is completed
	bool IsMovementComplete( void ) const;
	// Performs movement
	void PerformMovement( Double animInterval );
	// Stops the movement
	void StopMovement( void );

	// Drops an item/weapon
	CBaseEntity* DropItem( const Char* pstrClassName, const Vector& itemPosition, const Vector& itemAngles );

	// Checks clear damage list for any dmg bit that needs to be cleared
	void ProcessClearDamageList( void );
	// Adds a new damage bit that needs delayed clearing
	void AddClearDamage( Int32 dmgbit, Float delay );

protected:
	// Updates expressions
	void UpdateExpressions( void );

	// Returns the current flex AI state
	enum flexaistates_t GetFlexAIState( void );
	// Returns the NPC's flex type
	static enum flextypes_t GetFlexNPCType( void );

protected:
	// Hears any sounds
	void HearSounds( void );
	// Updates the best sound info
	void UpdateBestSound( void );

	// Returns the next available enemy
	bool GetNextEnemy( void );
	// Checks AI triggers
	void CheckAITriggers( void );
	// Checks a single AI trigger
	bool CheckAITrigger( Int32 triggerCondition );

	// Returns the firing direction
	Vector GetShootVector( const Vector& shootOrigin );
	// Returns the ideal enemy body target
	Vector GetIdealEnemyBodyTarget( const Vector& shootOrigin );
	// Tells if enemy is shootable
	bool IsEnemyShootable( CBaseEntity& enemy, bool ignoreGlass, bool ignoreBreakables, stance_t stance = STANCE_ACTUAL );
	// Checks if an enemy's body target is shootable
	bool IsEnemyBodyTargetShootable( CBaseEntity& enemy, bool ignoreGlass, bool ignoreBreakables, const Vector& gunPosition, const Vector& enemyBodyTarget );
	// Checks firing vectors with stance info
	bool CheckFiringWithStance( CBaseEntity& enemy, const Vector& firingCone, bool& shouldStand );
	// Check glass penetration
	bool CheckMaterialPenetration( CBaseEntity* pHitEntity, const Vector& gunPosition, const Vector& hitPosition, const Vector& hitNormal, Vector& outPosition );

	// Fires a weapon
	void FireWeapon( Uint32 numShots, const Char* soundPattern, Uint32 numSounds, Uint32* ptrAmmoLoaded );

	// Pops an enemy from the stack
	bool PopEnemy( void );
	// Returns the most ideal enemy to attack
	CBaseEntity* GetBestVisibleEnemy( void );

	// Sets a sequence by name
	void SetSequenceByName( const Char* pstrName );
	// Manages a footstep event
	void FootStep( void );

	// Tells if the NPC is facing the ideal yaw
	bool IsFacingIdealYaw( void );
	// Sets the ideal yaw from a vector
	void SetIdealYaw( const Vector& targetVector, bool isPositionVector = true );
	// Returns the difference between current and ideal yaw
	Float GetYawDifference( void );
	// Changes the yaw to the ideal yaw based on time
	void ChangeYaw( Double timeInterval );
	// Turns a position vector to a yaw
	Float VectorToYaw( const Vector& direction ) const;
	// Turns a position vector to a pitch
	Float VectorToPitch( const Vector& direction ) const;
	// Sets the turning activity
	void SetTurnActivity( void );

	// Returns the ideal flinch activity
	activity_t GetFlinchActivity( void );

protected:
	// Clears the route info
	void ClearRoute( void );
	// Sets up for a new route
	void NewRoute( void );
	// Tells if route is clear
	bool IsRouteClear( void ) const;
	// Refreshes the route to the destination
	bool RefreshRoute( void );
	// Updates the route
	bool UpdateRoute( CBaseEntity* pTargetEntity, const Vector& destination );
	// Shows the current route
	void ShowRoute( bool isDetour, Uint32 maxPath, const Vector& destination );
	// Shows the current route
	void ShowRoute( const Vector* pPoints, Uint32 numPoints );
	// Shows an error path
	void ShowErrorPath( const Vector& startPosition, const Vector& endPosition );
	// Builds a route fto a destination
	bool BuildRoute( const Vector& destination, Uint64 moveFlags, CBaseEntity* pTargetEntity );
	// Checks if a route is traversable to the destination from the start
	bool CheckRoute( const Vector& startPosition, const Vector& endPosition, CBaseEntity* pTargetEntity = nullptr );
	// Builds a node route to a destination vector and target entity
	bool BuildNodeRoute( const Vector& destination, CBaseEntity* pTargetEntity = nullptr );
	// Checks if a node route is available from start to end
	bool CheckNodeRoute( const Vector& startPosition, const Vector& endPosition, CBaseEntity* pTargetEntity = nullptr );
	// Builds a node detour route around an obstable marked by some nodes
	bool BuildNodeDetourRoute( const Vector& destination, CBaseEntity* pBlocker, CBaseEntity* pTargetEntity = nullptr );
	// Builds a route where a position vector is visible
	bool BuildNearestVisibleRoute( const Vector& destination, const Vector& viewOffset, Float minDistance, Float maxDistance );
	// Builds a nearest route to a position vector
	bool BuildNearestRoute( const Vector& destination, Float minDistance, Float maxDistance );
	// Finds a node that's not visible from our current position
	bool FindUnseenNode( void );
	// Finds a random search spot
	bool FindRandomSearchSpot( void );
	// Advances the index in the route array
	advance_result_t AdvanceRoute( Float distance );
	// Marks movement as completed
	void MovementComplete( void );
	// Tells the route's type
	static movegoal_t ClassifyRoute( Uint64 moveFlags );
	// Tells if a position is navigable
	bool IsPositionNavigable( const Vector& position );

	// Attempts to triangulate to a destination position
	bool AttemptTriangulation( const Vector& startPosition, const Vector& endPosition, Float distance, CBaseEntity* pTargetEntity, Vector* pApexPosition, bool isTest = false );
	// Attemps to circle around an obstacle
	bool AttemptCircleAround( bool isRetry, const Vector& destination, CBaseEntity* pBlocker, CBaseEntity* pTargetEntity );
	// Attempts to shift the route point destination to make it traversable
	bool AttemptShiftDestination( CBaseEntity* pTargetEntity );
	// Tells if the route should be simplified
	static bool ShouldSimplifyRoute( const Vector& currentPosition, const Vector& testPosition, Int32 routeTypeBits );
	// Simplifies the current route
	void SimplifyRoute( CBaseEntity* pTargetEntity );
	// Inserts a point into the route at a given position
	void InsertRoutePoint( const Vector& position, Int32 nodeIndex, Uint64 moveFlags );
	// Inserts a point at the end of the route
	void InsertRoutePoint( Int32 insertIndex, const Vector& position, Int32 nodeIndex, Uint64 moveFlags );

	// Does a trace to see how far an NPC can go in a given direction
	bool WalkMoveTrace( const Vector& origin, const Vector& moveDirection, Vector& outPosition, Float moveDistance, Float& distanceMoved, bool noNPCs = false );

	// Nudges the player if he's in the way during scripted_sequences
	void NudgePlayer( CBaseEntity* pPlayer ) const;
	// Handles a blockage when performing movement
	bool HandleBlockage( CBaseEntity* pBlocker, CBaseEntity* pTargetEntity, Float moveDistance );
	// Checks the result of a move to a given direction and distance
	bool CheckMoveResult( localmove_t moveResult, Float moveDistance, const Vector& vectorToTarget );
	// Check if you can advance the route index
	bool CheckAdvanceRoute( Float distanceToPoint, CBaseEntity* pTargetEntity );
	// Executes movement for thE NPC
	void ExecuteMovement( CBaseEntity* pTargetEntity, const Vector& direction, Double animInterval, Float checkDistance );

	// Tries to find a lateral shooting position
	bool GetLateralShootingPosition( const Vector& enemyPosition );
	// Tries to find a shooting position closer to the enemy
	bool GetClosestShootingPosition( const Vector& enemyPosition );

	// Finds a hint node
	Int32 FindHintNode( void );
	// Validates a hint node type
	static bool ValidateHintType( Int32 hintType ); 

	// Finds a clear node
	bool FindClearNode( CBaseEntity* pBlockedEntity );
	// Tells if an entity is facing a position
	static bool IsFacing( CBaseEntity* pEntity, const Vector& checkPosition );

protected:
	// Tells if the current schedule
	bool IsScheduleValid( void );
	// Returns the current schedule's flags
	Uint64 GetScheduleFlags( void ) const;
	// Tells if the npc has an active schedule
	bool HasActiveSchedule( void ) const;
	// Tells if schedule is finished
	bool IsScheduleDone( void );
	// Begins the next scheduled task in the schedule
	void BeginNextScheduledTask( void );
	// Maintains the current schedule
	void MaintainSchedule( void );

	// Checks which attacks are possible
	void CheckAttacks( CBaseEntity* pTargetEntity, Float distance );
	// Tells if we can do ranged attacks
	bool CanRangeAttack( void ) const;

	// Return skill cvar value
	Float GetSkillCVarValue( Int32 skillcvar ) const;

protected:
	// Returns the firing coverage for a targer position
	Float GetFiringCoverage( const Vector& shootOrigin, const Vector& targetPosition, const Vector& firingCone );
	// Returns visibility bits for an NPC/player
	Uint64 GetNPCVisibilityBits( CBaseEntity* pEntity, bool checkGlass = false );
	// Returns an entity we can kick
	CBaseEntity* GetKickEntity( Float checkDistance );
	// Calculates coverage for a position
	Float CalculateCoverage( const Vector& lookOrigin, const Vector& lookOffset, const Vector& enemyEyePosition );

	// Return the most ideal firing cone
	virtual Uint32 GetFogAttenuatedFiringCone( Uint32 coneIndex );

protected:
	// Moves to a given location and tells if it's possible
	bool MoveToLocation( activity_t moveActivity, Float waitTime, const Vector& goalPosition );
	// Moves to a given node's position
	bool MoveToNode( activity_t moveActivity, Float waitTime, const Vector& goalPosition );
	// Moves to the current target
	bool MoveToTarget( activity_t moveActivity, Float waitTime );

	// Returns the length of the route we are taking
	Float GetRouteLength( void );

	// Opens a door
	Double OpenDoor( CBaseEntity* pDoorEntity );

	// Tells if we're in danger
	bool IsInDanger( void );
	// Sets us up for a danger check
	void StartDangerCheck( const Vector& dangerPosition, CBaseEntity* pDangerEntity, bool coverPosition );
	// Examines current dangers
	void ExamineDangers( void );
	// Finds cover with the best distance
	bool FindCoverWithBestDistance( const Vector& threatPosition, Float minDistance, float maxDistance, Float optimalDistance, Vector& outPosition );
	// Finds a dodge cover from a threat
	bool FindDodgeCover( const Vector& threatPosition, Float minDistance, float maxDistance, Vector& outPosition );
	// Finds a cover position from a threat
	bool FindCover( const Vector& threatPosition, const Vector& viewOffset, Float minDistance, Float maxDistance, CBaseEntity* pThreatEntity );
	// Finds a lateral cover position
	bool FindLateralCover( const Vector& threatPosition, const Vector& viewOffset );

protected:
	// Sets ideal head angles
	void SetIdealHeadAngles( Float pitch, Float yaw );
	// Sets the ideal yaw
	bool SetIdealHeadYaw( Float yaw );

	// Sets the ideal expression based on AI state
	void SetIdealExpression( void );

protected:
	// Clears the memory of specified bits
	void ClearMemory( Uint64 memoryBits );
	// Tells if we have the specified memory bits
	bool HasMemory( Uint64 memoryBits ) const;

	// Sets capability bits
	void SetCapabilities( Uint64 capabilityBits );
	// Tells if the NPC has the specified capabilities
	bool HasCapabilities( Uint64 capabilityBits ) const;

protected:
	// Initializes the NPC
	virtual void InitNPC( void );
	// Sets eye position
	virtual void SetEyePosition( void );

	// Sets the NPC state
	virtual void SetNPCState( npcstate_t state );

	// Returns the ideal death activity
	virtual activity_t GetDeathActivity( void );

	// Performs sight-related functions
	virtual void Look( void );

	// Initializes squad related data
	virtual void InitSquad( void ) { };
	// Tells if the NPC can do crouching
	virtual bool CanCrouch( void ) { return false; }
	// Checks the current enemy's state
	virtual bool CheckEnemy( void );

	// Sets the ideal yaw speed
	virtual void SetYawSpeed( void ) = 0;
	// Returns the sound mask for the NPC
	virtual Uint64 GetSoundMask( void ) = 0;
	// Returns the hearing sensitivity for this npc
	virtual Float GetHearingSensitivity( void );
	// Returns the firing cone
	virtual const Uint32 GetFiringCone( bool attenuateByFog ) { return 0; }
	// Used to override NPC AI behavior
	const CAISchedule* GetScheduleMain( void );
	// Returns the ideal schedule
	virtual const CAISchedule* GetSchedule( void );
	// Returns a schedule by it's index
	virtual const CAISchedule* GetScheduleByIndex( Int32 scheduleIndex );
	// Returns the schedule for idle stand
	virtual const CAISchedule* GetIdleStandSchedule( Int32 scheduleIndex );

	// Collects data from senses
	virtual void RunSenses( void );

	// Updates the awareness factor
	virtual void UpdateAwareness( Uint64 sightBits );
	// Returns the time time to be aware of a player who's leaning
	virtual Float GetLeanAwarenessTime( void );

	// Gibs the NPC
	virtual void GibNPC( void );

	// Runs AI code
	virtual void RunAI( void );

	// Plays idle sounds
	virtual void EmitIdleSound( void ) { };
	// Plays pain sounds
	virtual void EmitPainSound( void ) { };
	// Plays death sounds
	virtual void EmitDeathSound( void ) { };
	// Plays alert sounds
	virtual void EmitAlertSound( void ) { };
	// Returns the voice pitch
	virtual Uint32 GetVoicePitch( void );

	// Sets the ideal activity
	virtual void SetIdealActivity( Int32 activity );

	// Checks if we can do range attack 1
	virtual bool CheckRangeAttack1( Float dp, Float distance ) { return false; }
	// Checks if we can do range attack 2
	virtual bool CheckRangeAttack2( Float dp, Float distance ) { return false; }
	// Checks if we can do melee attack 1
	virtual bool CheckMeleeAttack1( Float dp, Float distance ) { return false; }
	// Checks if we can do melee attack 2
	virtual bool CheckMeleeAttack2( Float dp, Float distance ) { return false; }
	// Checks if we can do special attack 1
	virtual bool CheckSpecialAttack1( Float dp, Float distance ) { return false; }
	// Checks if we can do melee attack 2
	virtual bool CheckSpecialAttack2( Float dp, Float distance ) { return false; }
	// Tells if we can check the attacks
	virtual bool CanCheckAttacks( void ) const;
	// Checks if we are likely to shoot a friendly NPC
	virtual bool CheckFriendlyFire( void );

	// Performs pre-schedule think functions
	virtual void PreScheduleThink( void );
	// Called when a schedule is changed
	virtual void OnScheduleChange( void );

	// Returns the ideal cover distance
	virtual Float GetCoverDistance( void );
	// Returns the gun position
	virtual Vector GetGunPosition( stance_t stance = STANCE_ACTUAL ) = 0;
	// Allows squad members to validate a cover position
	virtual bool ValidateCover( const Vector& coverPosition );

	// Tells if the NPC can be active while idle
	virtual bool CanActiveIdle( void ) { return false; }
	// Tells if the NPC can be instantly decapitated
	virtual bool CanBeInstantlyDecapitated( void ) { return true; }

	// Return the stopped activity
	virtual activity_t GetStoppedActivity( void );

	// Returns the conditions to ignore
	virtual Uint64 GetIgnoreConditions( void );
	// Get any conditions to be kept when changing schedules
	virtual Uint64 GetScheduleChangeKeptConditions( void ) { return AI_COND_NONE; }

	// Checks the ammo capacity
	virtual void CheckAmmo( void ) { }

	// Sets the activity and related states
	virtual void SetActivity( Int32 activity );

	// Starts a task
	virtual void StartTask( const ai_task_t* pTask );
	// Runs a task
	virtual void RunTask( const ai_task_t* pTask );
	// Sets task as failed
	virtual void SetTaskFailed( bool allowRetry = true );
	// Sets task as completed
	virtual void SetTaskCompleted( void );

	// Processes a sound heard
	virtual bool ProcessHeardSound( ai_sound_t& sound, Uint64 soundMask );
	// Tells if we should see an NPC
	virtual bool ShouldSeeNPC( Uint64 sightBits, CBaseEntity* pEntity );
	// Tells if NPC should check navigability
	virtual bool ShouldCheckEnemyNavigability( void ) { return false; }

	// Updates visibility and shooting distances
	virtual void UpdateDistances( void );

	// Plays a footstep sound
	virtual void PlayFootStepSound( const Char* pstrMaterialName, Float volume ) { };

	// Tells if the NPC should be gibbed
	virtual bool ShouldDamageGibNPC( Float damageAmount, Float prevHealth, Int32 dmgFlags, bool wasDecapitated );
	// Spawns particles when NPC is gibbed
	virtual void SpawnGibbedParticles( void );

	// Picks a proper reload schedule
	virtual const CAISchedule* GetReloadSchedule( void );

	// Returns the reaction time
	virtual Float GetReactionTime( void );

	// Return bullet type used by NPC
	virtual bullet_types_t GetBulletType( void ) { return BULLET_NONE; }

	// Return the NPC's minimum firing distance
	virtual Float GetMinimumRangeAttackDistance( void ) { return 0; }

protected:
	// Last time the NPC thought
	Double						m_npcLastThinkTime;
	// Think interval time
	Double						m_thinkIntervalTime;

	// Head controller related for yaw
	Float						m_idealHeadYaw;
	Float						m_headYaw;

	// Head controller related for pitch
	Float						m_idealHeadPitch;
	Float						m_headPitch;

	// TRUE if we shouldn't drop any weapons
	bool						m_dontDropWeapons;

	// Current activity
	Int32						m_currentActivity;
	// Ideal activity
	Int32						m_idealActivity;
	// Last time we changed the activity
	Double						m_lastActivityTime;

	// Current task status
	Int32						m_taskStatus;
	// Current schedule
	const CAISchedule*			m_pSchedule;
	// Current schedule index
	Int32						m_scheduleTaskIndex;
	// Failure schedule index
	Int32						m_failureScheduleIndex;
	// Next schedule to play after current schedule has ended for any reason
	Int32						m_nextScheduleIndex;

	// NPC AI state
	Int32						m_npcState;
	// Ideal NPC AI state
	Int32						m_idealNPCState;
	// Schedule index
	Int32						m_currentScheduleIndex;

	// Maximum looking distance
	Float						m_lookDistance;
	// Distance for shooting
	Float						m_firingDistance;

protected:
	// Array of AI condition flags
	Uint64						m_aiConditionBits;
	// Memory contents	
	Uint64						m_memoryBits;
	// Capability bits
	Uint64						m_capabilityBits;
	// Capability bits
	Uint64						m_disabledCapabilityBits;
	// Damage bits taken
	Uint64						m_damageBits;
	// Last damage amount we took
	Float						m_lastDamageAmount;
	// Last attack vector
	Vector						m_lastAttackVector;
	// For damage types that need to be cleared after a while
	CLinkedList<cleardamage_t>	m_damageClearList;

	// Current enemy
	CEntityHandle				m_enemy;

	// Enemy last known position
	Vector						m_enemyLastKnownPosition;
	// Enemy last known angles
	Vector						m_enemyLastKnownAngles;
	// Enemy body target
	bodytarget_t				m_enemyBodyTarget;

	// Entity we're trying to follow
	CEntityHandle				m_targetEntity;

	// Array of backed-up enemy infos
	enemy_info_t				m_backedUpEnemies[MAX_BACKED_UP_ENEMIES];

	// Field of view of the npc
	Float						m_fieldOfView;

	// Wait finish time
	Double						m_waitFinishedTime;
	// Move wait finish time
	Double						m_moveWaitFinishTime;

	// Last hitgroup that took damage
	Int32						m_lastHitGroup;

	// Route to follow
	route_point_t				m_routePointsArray[MAX_ROUTE_POINTS];
	// Movement goal
	Int32						m_movementGoal;
	// Current index into the route array
	Int32						m_routePointIndex;
	// Wait time until we begin moving again
	Double						m_moveWaitTime;
	// First node index
	Int32						m_firstNodeIndex;

	// Last origin we cut corners at
	Vector						m_lastCornerCutOrigin;
	// TRUE if we can cut corners
	bool						m_canCutCorners;
	// Shortcut path index
	Int32						m_shortcutPathIndex;

	// Movement goal position
	Vector						m_movementGoalPosition;
	// Movement activity
	Int32						m_movementActivity;
	// Movement goal entity
	CEntityHandle				m_movementGoalEntity;
	// Blocker entity
	CEntityHandle				m_blockerEntity;
	// Distance travelled
	Float						m_distanceTravelled;
	// Path corner destination entity
	CEntityHandle				m_goalEntity;

	// List of audible sounds
	CLinkedList<ai_sound_t>	m_soundsList;
	// Sound types we're sensitive to
	Int32						m_soundTypes;

	// Last position we'll return to
	Vector						m_lastPosition;
	// Hint node type we're moving to
	Int32						m_hintNodeIndex;

	// NPC's maximum health
	Float						m_maximumHealth;
	// Amount of ammy loaded
	Uint32						m_ammoLoaded;
	// Clip size
	Uint32						m_clipSize;

	// Blood color type
	Int32						m_bloodColor;

	// Talk time
	Double						m_talkTime;

	// Current best sound
	ai_sound_t*					m_pBestSound;

	// Last time we saw a leaning player
	Double						m_lastLeanSightTime;
	// Leaning awareness
	Float						m_leanAwareness;

	// TRUE if values were parsed
	bool						m_valuesParsed;

	// Force skill cvar setting
	Int32						m_forceSkillCvar;

	// NPC we're blocking
	CEntityHandle				m_blockedNPC;
	// Last clear node
	Int32						m_lastClearNode;
	// Blocked NPC's goal
	Vector						m_blockedNPCDestination;

	// Last player sight time
	Double						m_lastPlayerSightTime;

	// Death mode
	Int32						m_deathMode;
	// Damage bits at death
	Int32						m_deathDamageBits;
	// Delay before NPC explodes
	Double						m_deathExplodeTime;

	// Last distance from danger source
	Float						m_lastDangerDistance;
	// Current danger exposure
	Float						m_dangerExposure;
	// TRUE if we're seeking cover
	bool						m_isSeekingCover;
	// Danger source origin
	Vector						m_dangerOrigin;
	// Danger entity
	CEntityHandle				m_dangerEntity;

	// Active flex state
	Int32						m_activeFlexState;
	// Flex script duration
	Double						m_flexScriptDuration;
	// Script flags
	Int32						m_flexScriptFlags;

	// Script state
	Int32						m_scriptState;
	// Script entity
	CEntityHandle				m_scriptEntity;

	// Infectous dangerous enemy
	CEntityHandle				m_dangerousEnemy;
	// Last navigability check position
	Vector						m_lastNavigabilityCheckPosition;

	// Linked list of enemies we can see
	CLinkedList<CEntityHandle>	m_sightedHostileNPCsList;
	// Linked list of friendlies we can see
	CLinkedList<CEntityHandle>	m_sightedFriendlyNPCsList;

	// Cinematic entity
	CScriptedSequence*			m_pScriptedSequence;

	// Puller entity(do not restore these)
	CEntityHandle				m_npcPullerEntity;
	// Puller position
	Vector						m_npcPullerPosition;

	// True if yaw should be updated
	bool						m_updateYaw;

	// Best sound we're investigating
	// This needs to be separate from
	// the one in the AI, as the best
	// sound can change, but we need this
	// to remain the same as we're checking it out
	ai_sound_t					m_currentCheckSound;
	// TRUE if best sound was set
	bool						m_checkSoundWasSet;

	// Last place we visited while looking for enemies
	Vector						m_lastEnemySeekPosition;

protected:
	// Trigger target 1
	string_t					m_triggerTarget1;
	// Trigger condition 1
	Int32						m_triggerCondition1;
	// Trigger target 2
	string_t					m_triggerTarget2;
	// Trigger condition 2
	Int32						m_triggerCondition2;

protected:
	// Last cover search node
	static Int32 g_lastCoverSearchNodeIndex;
	// Last active idle search node
	static Int32 g_lastActiveIdleSearchNodeIndex;
};
#endif //AI_BASENPC_H