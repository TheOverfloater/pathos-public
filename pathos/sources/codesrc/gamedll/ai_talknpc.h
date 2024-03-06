/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef AI_TALKNPC_H
#define AI_TALKNPC_H

#include "ai_squadnpc.h"

enum talknpc_tasks_t
{
	AI_TALKNPC_TASK_CANT_FOLLOW = LAST_SQUADNPC_TASK + 1,
	AI_TALKNPC_TASK_RESPOND,
	AI_TALKNPC_TASK_SPEAK,
	AI_TALKNPC_TASK_SAY_HELLO,
	AI_TALKNPC_TASK_HEADRESET,
	AI_TALKNPC_TASK_SAY_STOPSHOOTING,
	AI_TALKNPC_TASK_STARE,
	AI_TALKNPC_TASK_LOOK_AT_PLAYER,
	AI_TALKNPC_TASK_PLAYER_STARE,
	AI_TALKNPC_TASK_EYECONTACT,
	AI_TALKNPC_TASK_IDEALYAW,
	AI_TALKNPC_TASK_FACE_PLAYER,

	// Must be last
	LAST_TALKNPC_TASK
};

enum talknpc_schedules_t
{
	AI_TALKNPC_SCHED_CANT_FOLLOW = LAST_SQUADNPC_SCHEDULE + 1,
	AI_TALKNPC_SCHED_TARGET_FACE,
	AI_TALKNPC_SCHED_TARGET_CHASE,

	// Must be last
	LAST_TALKNPC_SCHEDULE
};

extern const CAISchedule scheduleIdleResponse;
extern const CAISchedule scheduleIdleSpeak;
extern const CAISchedule scheduleIdleSpeakWait;
extern const CAISchedule scheduleIdleHello;
extern const CAISchedule scheduleSayStopShooting;
extern const CAISchedule scheduleIdleWatchPlayer;
extern const CAISchedule scheduleIdleWatchPlayerStare;
extern const CAISchedule scheduleIdleEyeContact;
extern const CAISchedule scheduleTalkNPCIdleStand;
extern const CAISchedule scheduleFollow;
extern const CAISchedule scheduleFaceTarget;

//=============================================
//
//=============================================
class CTalkNPC : public CSquadNPC
{
public:
	enum talkbits_t
	{
		TALKNPC_SAID_DAMAGE_LIGHT	= (1<<0),
		TALKNPC_SAID_DAMAGE_MEDIUM	= (1<<1),
		TALKNPC_SAID_DAMAGE_HEAVY	= (1<<3),
		TALKNPC_SAID_HELLO_PLAYER	= (1<<4),
		TALKNPC_SAID_WOUND_LIGHT	= (1<<5),
		TALKNPC_SAID_WOUND_HEAVY	= (1<<6),
		TALKNPC_SAID_HEARD_SOUND	= (1<<7)
	};
	
	enum talkgroupnames_t
	{
		TALKNPC_GRP_ANSWER = 0,
		TALKNPC_GRP_QUESTION,
		TALKNPC_GRP_IDLE,
		TALKNPC_GRP_STARE,
		TALKNPC_GRP_USE,
		TALKNPC_GRP_UNUSE,
		TALKNPC_GRP_STOP,
		TALKNPC_GRP_NOSHOOT,
		TALKNPC_GRP_HELLO,
		TALKNPC_GRP_PHELLO,
		TALKNPC_GRP_PIDLE,
		TALKNPC_GRP_PQUESTION,
		TALKNPC_GRP_PLHURT1,
		TALKNPC_GRP_PLHURT2,
		TALKNPC_GRP_PLHURT3,
		TALKNPC_GRP_WOUND,
		TALKNPC_GRP_MORTAL,
		TALKNPC_GRP_DECLINE,
		TALKNPC_GRP_CONVERSE,
		TALKNPC_GRP_FINDPLAYER,

		// Must be last
		NB_TALKNPC_GROUPS
	};

public:
	// Ideal yaw task yaw speed
	static const Float IDEALYAW_TASK_YAWSPEED;
	// Maximum distance for staring
	static const Float STARE_MAX_DIST;
	// Minimum treshold for idealyaw difference
	static const Float IDEALYAW_DIFF_TRESHOLD;
	// Minimum talk range
	static const Float MAXIMUM_TALK_RANGE;
	// Minimum push speed
	static const Float MINIMUM_PUSH_SPEED;
	// Mortal health treshold on player
	static const Float PLAYER_MORTAL_HEALTH_TRESHOLD;
	// Medium health treshold on player
	static const Float PLAYER_MEDIUM_HEALTH_TRESHOLD;
	// Light health treshold on player
	static const Float PLAYER_LIGHT_HEALTH_TRESHOLD;
	// Maximum follower distance before FOLLO_TARGET_TOOFAR is set
	static const Float MAX_FOLLOW_DISTANCE;
	// Default follow range
	static const Float DEFAULT_FOLLOW_RANGE;
	// Sentence group name postifxes
	static const Char* SENTENCE_GROUP_POSTFIXES[NB_TALKNPC_GROUPS];

public:
	explicit CTalkNPC( edict_t* pedict );
	virtual ~CTalkNPC( void );

public:
	// Performs precache functions
	virtual void Precache( void ) override;
	// Declares save-restore fields
	virtual void DeclareSaveFields( void ) override;
	// Manages keyvalues
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	// Manages taking damage
	virtual bool TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags ) override;
	// Handles damage calculation for a hitscan
	virtual void TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags ) override;
	// Calls touch function
	virtual void CallTouch( CBaseEntity* pOther ) override;
	// Manages getting killed
	virtual void Killed( CBaseEntity* pAttacker, gibbing_t gibbing, deathmode_t deathMode ) override;
	// Gets the relationship with the other entity
	virtual Int32 GetRelationship( CBaseEntity* pOther ) override;
	// Tells if the NPC can play a scripted_sentence
	virtual bool CanPlaySentence( bool disregardState ) override;
	// Plays a sentence
	virtual void PlaySentence( const Char* pstrSentenceName, Float duration, Float volume, Float attenuation, Float timeOffset, bool subtitleOnlyInRadius, CBaseEntity* pPlayer = nullptr ) override;
	// Plays a scripted_sentence
	virtual void PlayScriptedSentence( const Char* pstrSentenceName, Float duration, Float volume, Float attenuation, Float timeOffset, bool subtitleOnlyInRadius, bool isConcurrent, CBaseEntity* pListener, CBaseEntity* pPlayer = nullptr ) override;
	// Returns the conditions to ignore
	virtual Uint64 GetIgnoreConditions( void ) override;
	// Returns a schedule by it's index
	virtual const CAISchedule* GetScheduleByIndex( Int32 scheduleIndex ) override;
	// Starts a task
	virtual void StartTask( const ai_task_t* pTask ) override;
	// Runs a task
	virtual void RunTask( const ai_task_t* pTask ) override;
	// Handles an animation event
	virtual void HandleAnimationEvent( const mstudioevent_t* pevent ) override;
	// Return the stopped activity
	virtual activity_t GetStoppedActivity( void ) override;
	// Returns the ideal activity
	virtual Int32 GetIdealActivity( void ) override;
	// Returns the view position
	virtual Vector GetEyePosition( bool addlean = false ) const override;
	// Tells if the entity is a talking NPC
	virtual bool IsTalkNPC( void ) const override { return true; }
	// Stops following the target
	virtual void StopFollowing( bool clearSchedule ) override;
	// Returns the voice pitch
	virtual Uint32 GetVoicePitch( void ) override;
	// Tells if talking npc can answer
	virtual bool CanAnswer( void ) override;
	// Sets to answer a question
	virtual void SetAnswerQuestion( CBaseEntity* pSpeaker, Double talkTime ) override;
	// Performs pre-schedule think functions
	virtual void PreScheduleThink( void ) override;
	// Sets the follow target for this NPC
	virtual void SetFollowTarget( CBaseEntity* pTarget ) override;
	// Returns the schedule for idle stand
	virtual const CAISchedule* GetIdleStandSchedule( Int32 scheduleIndex ) override;
	// Tells if NPC is following a player
	virtual bool IsFollowing( void ) const override;
	// Manages a healer call
	virtual void NPCHealerCall( CBaseEntity* pCaller ) override;

public:
	// Finds the nearest NPC
	CBaseEntity* FindNearestFriend( void );
	// Finds the player
	CBaseEntity* FindPlayer( void );
	// Returns distance to target
	Float GetTargetDistance( void );
	// Stops the NPC from talking
	void StopTalking( void );

	// Manages an idle response
	void IdleRespond( void );
	// Manages idle speech
	void IdleSpeak( void );
	// Manages idle stare
	void IdleStare( void );
	// Manages idle hello
	void IdleHello( void );
	// Turns head while idling
	void IdleHeadTurn( const Vector* pTarget );

	// Tells if it's okay to speak
	bool CanSpeak( void );
	// Finds a friendly from the list
	CBaseEntity* EnumerateFriends( CBaseEntity* pPrevious, bool trace );
	// Alerts friends
	void AlertFriends( void );
	// Tells if the NPC is talking
	bool IsTalking( void ) const;
	// Sets talking duration
	void SetTalkTime( Float duration );

	// Tells if NPC can follow the player
	bool CanFollow( void ) const;
	// Starts following a target
	void StartFollowing( CBaseEntity* pTarget );
	// Limits followers to a number
	void LimitFollowers( const CBaseEntity* pPlayer, Uint32 maxFollowers );

	// Use function for following
	void EXPORTFN FollowerUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value );

	// Sets sentence group names
	void SetSentenceGroups( const Char* pstrPrefix );

public:
	// Initializes talknpc data
	virtual void InitTalkingNPC( void );
	// Declines following a target
	virtual void DeclineFollowing( void );

public:
	// Sets the time until someone else can talk
	static void SetTalkWaitTime( Double talkWaitTime );
	// Resets talk time
	static void ResetTalkTime( void );

public:
	// Bits storing what one-time sentences we've said
	Uint64			m_saidSentencesBits;
	// Number of times we've initiated talks
	Uint32			m_nbTalksInitiated;
	// Voice pitch value
	Uint32			m_voicePitch;
	// Sentence group names specific to this NPC
	CString			m_sentenceGroupNames[NB_TALKNPC_GROUPS];
	// Next time we can be used
	Double			m_nextUseTime;
	// Follow sentence group
	string_t		m_useSentenceGroup;
	// Un-follow sentence group
	string_t		m_unFollowSentenceGroup;

	// Talk target entity
	CEntityHandle	m_talkTargetEntity;
	// Next follow target
	CEntityHandle	m_nextFollowTarget;

protected:
	// Wait time until we can talk again
	static Double g_talkWaitTime;
};
#endif //AI_TALKNPC_H