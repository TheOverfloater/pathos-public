/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef NPCCLONESOLDIER_H
#define NPCCLONESOLDIER_H

#include "ai_patrolnpc.h"

enum clone_soldier_schedules_t
{
	AI_CLONE_SOLDIER_SCHED_SUPPRESSING_FIRE = LAST_PATROLNPC_SCHEDULE + 1,
	AI_CLONE_SOLDIER_SCHED_TAKE_TACTICAL_POSITION,
	AI_CLONE_SOLDIER_SCHED_IDLE_SWEEP,
	AI_CLONE_SOLDIER_SCHED_AMBUSH_ENEMY,
	AI_CLONE_SOLDIER_SCHED_HIDE_AND_WAIT
};

enum clone_soldier_tasks_t
{
	AI_CLONE_SOLDIER_TASK_FACE_TOSS_DIR = LAST_PATROLNPC_TASK + 1,
	AI_CLONE_SOLDIER_TASK_SPEAK_SENTENCE,
	AI_CLONE_SOLDIER_TASK_GET_AMBUSH_PATH,
	AI_CLONE_SOLDIER_TASK_GET_TACTICAL_POSITION,
	AI_CLONE_SOLDIER_TASK_GET_HIDING_POSITION
};

//=============================================
//
//=============================================
class CNPCCloneSoldier : public CPatrolNPC
{
public:
	// View offset for npc
	static const Vector NPC_VIEW_OFFSET;
	// Yaw speed for npc
	static const Uint32 NPC_YAW_SPEED;
	// Kick distance for NPC
	static const Float NPC_KICK_DISTANCE;
	// Kick treshold distance for NPC
	static const Float NPC_KICK_TRESHOLD_DISTANCE;
	// Attachment for weapon
	static const Uint32 NPC_WEAPON_ATTACHMENT_INDEX;
	// Gun position offset when standing
	static const Vector NPC_GUN_POSITION_STANDING_OFFSET;
	// Gun position offset when crouching
	static const Vector NPC_GUN_POSITION_CROUCHING_OFFSET;
	// Clip size for Sig552
	static const Uint32 NPC_SIG552_CLIP_SIZE;
	// Clip size for shotgun
	static const Uint32 NPC_SHOTGUN_CLIP_SIZE;
	// Clip size for M249 SAW
	static const Uint32 NPC_M249_CLIP_SIZE;
	// Clip size for TRG42
	static const Uint32 NPC_TRG42_CLIP_SIZE;
	// Grenade explode delay
	static const Float NPC_GRENADE_EXPLODE_DELAY;
	// Next grenade check delay
	static const Float NPC_GRENADE_CHECK_DELAY;
	// NPC weapon sound radius
	static const Float NPC_WEAPON_SOUND_RADIUS;
	// NPC weapon sound duration
	static const Float NPC_WEAPON_SOUND_DURATION;
	// NPC helmet health
	static const Float NPC_HELMET_HEALTH;
	// How much of the damage the helmet takes
	static const Float NPC_HELMET_DMG_TAKE;
	// How much of the damage the helmet absorbs
	static const Float NPC_HELMET_DMG_ABSORB;
	// Minimum enemy distance for support type
	static const Float NPC_MIN_ENEMY_DISTANCE;
	// Maximum tactical position distance for support type
	static const Float NPC_MAX_TACTICALPOS_DISTANCE;
	// Minimum distance at which we'll throw grenades with a visible enemy
	static const Float NPC_MIN_GRENADE_DISTANCE;
	// Max grenades on a clone soldier
	static const Uint32 NPC_NUM_GRENADES;
	// Max look distance
	static const Float NPC_MAX_LOOK_DISTANCE;
	// Max normal firing distance
	static const Float NPC_MAX_FIRING_DISTANCE;
	// Max normal firing distance for shotgunner
	static const Float NPC_MAX_SHOTGUNNER_FIRING_DISTANCE;
	// Precise firing distance
	static const Float NPC_PRECISE_FIRING_DISTANCE;
	// Minimum ambush distance
	static const Float NPC_MIN_AMBUSH_DISTANCE_DISTANCE;

	// Question types
	enum npc_question_types_t
	{
		NPC_QUESTION_NONE = 0,
		NPC_QUESTION_CHECKIN,
		NPC_QUESTION_NORMAL,
		NPC_QUESTION_IDLE
	};

	// NPC weapon bits
	enum npc_weapons_t
	{
		NPC_WEAPON_SIG552	= (1<<0),
		NPC_WEAPON_GRENADE	= (1<<1),
		NPC_WEAPON_M249		= (1<<3),
		NPC_WEAPON_SHOTGUN	= (1<<4),
		NPC_WEAPON_TRG42	= (1<<5)
	};

	// Animation events
	enum npc_animevents_t
	{
		NPC_AE_RELOAD = 2,
		NPC_AE_KICK,
		NPC_AE_BURST1,
		NPC_AE_BURST2,
		NPC_AE_BURST3,
		NPC_AE_GREN_TOSS,
		NPC_AE_CAUGHT_ENEMY,
		NPC_AE_DROP_GUN = 11,
		NPC_AE_BURST1_PRECISE,
		NPC_AE_BURST2_PRECISE,
		NPC_AE_BURST3_PRECISE
	};

	// Clone soldier types
	enum npc_types_t
	{
		NPC_TYPE_SUPPORT = 0,
		NPC_TYPE_OFFENSIVE
	};

	enum npc_sentences_t
	{
		NPC_SENT_NONE = -1,
		NPC_SENT_GREN = 0,
		NPC_SENT_ALERT,
		NPC_SENT_MONSTER,
		NPC_SENT_COVER,
		NPC_SENT_THROW,
		NPC_SENT_CHARGE,
		NPC_SENT_TAUNT,

		NUM_NPC_SENTENCES
	};

	// Array of clone soldier sentences
	static const Char* NPC_SENTENCES[NUM_NPC_SENTENCES];

	// Model name for the npc
	static const Char NPC_MODEL_NAME[];

	// Pain sound pattern
	static const Char NPC_PAIN_SOUND_PATTERN[];
	// Number of pain sounds
	static const Uint32 NPC_NB_PAIN_SOUNDS;
	// Death sound pattern
	static const Char NPC_DEATH_SOUND_PATTERN[];
	// Number of death sounds
	static const Uint32 NPC_NB_DEATH_SOUNDS;

	// Bodygroup name for heads
	static const Char NPC_BODYGROUP_HEADS_NAME[];
	// Submodel name for normal head
	static const Char NPC_SUBMODEL_HEAD_NORMAL_NAME[];
	// Submodel name for decapitated head
	static const Char NPC_SUBMODEL_HEAD_DECAPITATED_NAME[];

	// Bodygroup name for weapons
	static const Char NPC_BODYGROUP_WEAPONS_NAME[];
	// Submodel name for sig552 weapon
	static const Char NPC_SUBMODEL_WEAPON_SIG552_NAME[];
	// Submodel name for shotgun weapon
	static const Char NPC_SUBMODEL_WEAPON_SHOTGUN_NAME[];
	// Submodel name for m249 weapon
	static const Char NPC_SUBMODEL_WEAPON_M249_NAME[];
	// Submodel name for trg42 weapon
	static const Char NPC_SUBMODEL_WEAPON_TRG42_NAME[];
	// Submodel name for blank weapon
	static const Char NPC_SUBMODEL_WEAPON_BLANK_NAME[];

public:
	explicit CNPCCloneSoldier( edict_t* pedict );
	virtual ~CNPCCloneSoldier( void );

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Performs precache functions
	virtual void Precache( void ) override;
	// Sets extra model info after setting the model
	virtual void PostModelSet( void ) override;
	// Declares save-restore fields
	virtual void DeclareSaveFields( void ) override;

	// Returns the classification
	virtual Int32 GetClassification( void ) const override;
	// Handles animation events
	virtual void HandleAnimationEvent( const mstudioevent_t* pevent ) override;

	// Handles damage calculation for a hitscan
	virtual void TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags ) override;

	// Returns a sequence for an activity type
	virtual Int32 FindActivity( Int32 activity ) override;
	// Returns the ideal activity
	virtual Int32 GetIdealActivity( void ) override;

	// Set by an NPC if this NPC is blocking them
	virtual void SetPathBlocked( CBaseEntity* pBlockedEntity, const Vector& destination ) override;

	// Tells how many grenades this NPC has
	virtual Uint32 GetNumGrenades( void ) override;
	// Sets grenade count on NPC
	virtual void SetNumGrenades( Uint32 numGrenades ) override;

public:
	// Sets the ideal yaw speed
	virtual void SetYawSpeed( void ) override;
	// Returns the sound mask for the NPC
	virtual Uint64 GetSoundMask( void ) override;

	// Returns the ideal schedule
	virtual const CAISchedule* GetSchedule( void ) override;
	// Returns a schedule by it's index
	virtual const CAISchedule* GetScheduleByIndex( Int32 scheduleIndex ) override;

	// Plays pain sounds
	virtual void EmitPainSound( void ) override;
	// Plays death sounds
	virtual void EmitDeathSound( void ) override;
	// Plays idle sounds
	virtual void EmitIdleSound( void ) override;

	// Checks the ammo capacity
	virtual void CheckAmmo( void ) override;
	// Returns the gun position
	virtual Vector GetGunPosition( stance_t stance = STANCE_ACTUAL ) override;

	// Tells if we can check the attacks
	virtual bool CanCheckAttacks( void ) const override;
	// Checks if we can do range attack 1
	virtual bool CheckRangeAttack1( Float dp, Float distance ) override;
	// Checks if we can do range attack 2
	virtual bool CheckRangeAttack2( Float dp, Float distance ) override;
	// Checks if we can do melee attack 1
	virtual bool CheckMeleeAttack1( Float dp, Float distance ) override;

	// Tells if the NPC can do crouching
	virtual bool CanCrouch( void ) override { return true; }
	// Returns the firing cone used
	const Uint32 GetFiringCone( bool attenuateByFog = false ) override;

	// Starts a task
	virtual void StartTask( const ai_task_t* pTask ) override;
	// Runs a task
	virtual void RunTask( const ai_task_t* pTask ) override;

	// Gibs the NPC
	virtual void GibNPC( void ) override;
	// Decapitates an NPC
	virtual void Decapitate( bool spawnHeadGib ) override;

	// Returns the conditions to ignore
	virtual Uint64 GetIgnoreConditions( void ) override;
	// Sets the NPC state
	virtual void SetNPCState( npcstate_t state ) override;
	// Updates visibility and shooting distances
	virtual void UpdateDistances( void ) override;

	// Initializes squad related data
	virtual void InitSquad( void ) override;

	// Returns the voice pitch
	virtual Uint32 GetVoicePitch( void ) override;

	// Sets task as failed
	virtual void SetTaskFailed( bool allowRetry = true ) override;
	// Sets task as completed
	virtual void SetTaskCompleted( void ) override;

	// Performs pre-schedule think functions
	virtual void PreScheduleThink( void ) override;

	// Returns the reaction time
	virtual Float GetReactionTime( void ) override;

	// Return bullet type used by NPC
	virtual bullet_types_t GetBulletType( void ) override;

	// Tells if NPC should favor ranged attacks versus melee
	virtual bool FavorRangedAttacks( void ) const override { return true; }

public:
	// Speaks a sentence
	void SpeakSentence( void );

	// Builds a route to a tactical position
	bool BuildTacticalRoute( CBaseEntity* pTargetEntity, const Vector& threatPosition, const Vector& threatAngles, const Vector& viewOffset, Float minDistance, Float maxDistance, bool& isPartialCover );
	// Builds a route to an ambush position
	bool BuildAmbushRoute( bool isObscured, const Vector& threatPosition, const Vector& threatAngles, const Vector& viewOffset, Float minDistance, Float maxDistance );
	// Finds a nearby cover position from the enemy's view
	bool BuildNearbyHidingPosition( CBaseEntity* pTargetEntity, const Vector& threatPosition, const Vector& viewOffset, Float minDistance, Float maxDistance );

	// Returns a schedule for a combat state
	const CAISchedule* GetCombatSchedule( void );

	// Tells if it's okay to speak
	bool CanSpeak( void ) const;
	// Called when a clone soldier just spoke
	void Spoke( void );

public:
	// Resets global npc states
	static void Reset( void );

private:
	// Next time we can moan in pain
	Double m_nextPainTime;
	// Next grenade check time
	Double m_nextGrenadeCheckTime;
	// Next sweep time
	Double m_nextSweepTime;
	// Helmet health
	Float m_helmetHealth;
	// Precise firing distance
	Float m_preciseDistance;
	// Tactical firing position coverage
	Float m_tacticalCoverage;

	// Grenade toss velocity
	Vector m_grenadeTossVelocity;
	// TRUE if we can throw a grenade
	bool m_tossGrenade;
	// TRUe if standing to shoot
	bool m_isStanding;
	// TRUE if we should ignore attack chance while moving
	bool m_takeAttackChance;
	// TRUE if we're doing precise aiming
	bool m_isPreciseAiming;

	// Last sentence said
	Int32 m_sentence;
	// Voice pitch
	Uint32 m_voicePitch;
	// Attack type
	Int32 m_attackType;
	// Number of grenades
	Uint32 m_numGrenades;

private:
	// Heads bodygroup index
	Int32 m_headsBodyGroupIndex;
	// Normal head submodel index
	Int32 m_headNormalSubmodelIndex;
	// Decapitated head submodel index
	Int32 m_headDecapitatedSubmodelIndex;

	// Weapons bodygroup index
	Int32 m_weaponsBodyGroupIndex;
	// Sig552 weapon submodel index
	Int32 m_weaponSig552SubmodelIndex;
	// Shotgun weapon submodel index
	Int32 m_weaponShotgunSubmodelIndex;
	// M249 weapon submodel index
	Int32 m_weaponM249SubmodelIndex;
	// TRG42 weapon submodel index
	Int32 m_weaponTRG42SubmodelIndex;
	// Blank weapon submodel index
	Int32 m_weaponBlankSubmodelIndex;

private:
	// Question asked
	static npc_question_types_t g_questionAsked;
	// Time until we talk again
	static Double g_talkWaitTime;
};
#endif //NPCCLONESOLDIER_H