/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef BASEENTITY_H
#define BASEENTITY_H

#include "constants.h"
#include "activity.h"
#include "util.h"

enum usemode_t
{
	USE_OFF = 0,
	USE_ON,
	USE_SET,
	USE_TOGGLE
};

class CEntityHandle;
class CBaseEntity;
class CFuncDoor;
class CScriptedSequence;
class CAISchedule;
class CEnvLadder;
class CItemMotorBike;
class CTriggerKeypad;
class CTriggerSubwayController;
class CTriggerLogin;
class CPlayerWeapon;
class CTriggerCameraModel;

enum subwayline_t;
enum activity_t;
enum deathstate_t;
enum weaponid_t;

struct weaponinfo_t;

enum entity_flags_t
{
	FL_ENTITY_TRANSITION		= (1<<0),
	FL_ENTITY_MUST_CALL_SPAWN	= (1<<1),
	FL_ENTITY_DONT_SAVE			= (1<<2),
	FL_ENTITY_PLAYER_USABLE		= (1<<3),
	FL_ENTITY_ONOFF_USE			= (1<<4),
	FL_ENTITY_CONTINOUS_USE		= (1<<5),
	FL_ENTITY_MASTER			= (1<<6),
	FL_ENTITY_DIRECTIONAL_USE	= (1<<7)
};

#ifdef _DEBUG
typedef void (CBaseEntity::*THINKFNPTR)( void );
typedef void (CBaseEntity::*INTERACTFNPTR)( CBaseEntity* pOther );
typedef void (CBaseEntity::*USEFNPTR)( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value );

#define SetThink( function ) _SetThink(static_cast <void (CBaseEntity::*)( void )>(function), #function)
#define SetTouch( function ) _SetTouch(static_cast <void (CBaseEntity::*)( CBaseEntity* pOther )>(function), #function)
#define SetBlocked( function ) _SetBlocked(static_cast <void (CBaseEntity::*)( CBaseEntity* pOther )>(function), #function)
#define SetUse( function ) _SetUse(static_cast <void (CBaseEntity::*)( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )>(function), #function)
#else
#define SetThink( function ) m_pfnThinkFunction = static_cast <void (CBaseEntity::*)( void )>(function)
#define SetTouch( function ) m_pfnTouchFunction = static_cast <void (CBaseEntity::*)( CBaseEntity* pOther )>(function)
#define SetBlocked( function ) m_pfnBlockedFunction = static_cast <void (CBaseEntity::*)( CBaseEntity* pOther )>(function)
#define SetUse( function ) m_pfnUseFunction = static_cast <void (CBaseEntity::*)( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )>(function)
#endif

#define STUBWARNING Util::EntityConPrintf(m_pEdict, "Stub function '%s' called.\n", __FUNCTION__);

/*
=======================
CBaseEntity

=======================
*/
class CBaseEntity
{
public:
	friend class CEntityHandle;
	friend class CWorldSpawn;

public:
	explicit CBaseEntity( edict_t* pedict );
	virtual ~CBaseEntity( void );

public:
	// Spawns the entity
	virtual bool Spawn( void );
	// Performs precache functions
	virtual void Precache( void ) { };
	// Called after save-restoring an entity
	virtual bool Restore( void );
	// Called after transition save-restore for global entities
	virtual void AdjustEntityPositions( const Vector& posAdj );
	// Manages a keyvalue
	virtual bool KeyValue( const keyvalue_t& kv );
	// Called when the entity is freed
	virtual void FreeEntity( void );
	// Called when entity is overriding a local entity
	virtual void OnOverrideEntity( void ) { };
	// Calld when aiment is freed
	virtual void OnAimentFreed( void ) { };

	// Sets object's collision box
	virtual void SetObjectCollisionBox( void );
	// Makes the entity dormant
	virtual void MakeEntityDormant( void );
	// Tells if entity is dormant
	virtual bool IsDormant( void );
	// Sets solidity on bike blockers
	virtual void SetBikeBlock( bool enable ) { };
	// Tells if entity is a bike blocked
	virtual bool IsBikeBlocker( void ) { return false; }
	// Tells if entity is worldspawn ent
	virtual bool IsWorldSpawn( void ) { return false; }

	// Sets whether the object should make an impact sound
	virtual void SetEnableImpactSound( bool enable ) { };

	// Calls think function
	virtual void CallThink( void );
	// Calls touch function
	virtual void CallTouch( CBaseEntity* pOther );
	// Calls blocked function
	virtual void CallBlocked( CBaseEntity* pOther );
	// Calls use function
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value );
	// Initializes parenting
	virtual void InitParenting( void );
	// Sets the entity to be removed
	virtual void FlagForRemoval( void );

	// Tells if entity should toggle it's state
	virtual bool ShouldToggle( usemode_t useMode, bool currentState );

	// Takes an amount of health
	virtual bool TakeHealth( Float amount, Int32 damageFlags );
	// Makes the entity take on damage
	virtual bool TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags );
	// Handles damage calculation for a hitscan
	virtual void TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags );
	// Manages spawning of decals
	virtual void SpawnBloodDecals( Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags );
	// Manages dying
	virtual void Killed( CBaseEntity* pAttacker, gibbing_t gibbing, deathmode_t deathMode = DEATH_NORMAL );
	// Returns blood color setting
	virtual bloodcolor_t GetBloodColor( void ) { return BLOOD_NONE; }

	// Called by NPC that got killed by this entity
	virtual void OnNPCKilled( CBaseEntity* pKilledNPC ) { }

	// Tells if the entity is alive
	virtual bool IsAlive( void ) const;
	// Tells if the entity is locked by a master
	virtual bool IsLockedByMaster( void ) { return false; }
	// Tells usable type of object
	virtual usableobject_type_t GetUsableObjectType( void ) { return USABLE_OBJECT_DEFAULT; }

	// Returns the brushmodel's center
	virtual Vector GetBrushModelCenter( void ) const;
	// Returns the entity's center
	virtual Vector GetCenter( void );
	// Returns the entity's targeting origin
	virtual Vector GetBodyTarget( const Vector& targetingPosition );
	// Returns the view position
	virtual Vector GetEyePosition( bool addlean = false ) const { return m_pState->origin + m_pState->view_offset; };
	// Returns a navigable position
	virtual Vector GetNavigablePosition( void ) const { return m_pState->origin; }
	// Returns the view origin for VIS checks
	virtual Vector GetVISEyePosition( void ) const { return GetEyePosition(); }
	// Tells if entity can be auto-aimed
	virtual bool IsAutoAimable( CBaseEntity* pAimerEntity ) { return false; }
	// Returns the view origin for VIS checks
	virtual bool GetAdditionalVISPosition( Vector& outVector ) const { return false; }

	// Returns the entity's target entity
	virtual CBaseEntity* GetTargetEntity( void ) { return nullptr; }

	// Gets the damage decal
	virtual const Char* GetDamageDecal( Int32 damageFlags );

	// Returns entity property flags
	virtual Int32 GetEntityFlags( void ) { return FL_ENTITY_TRANSITION; }
	// Returns the player's entity index
	virtual Int32 GetClientIndex( void ) const { return NO_CLIENT_INDEX; }
	// Returns the func_monitor entity's camera entity
	virtual const CBaseEntity* GetCameraEntity( void ) const { return nullptr; }
	// Clears think functions
	virtual void ClearThinkFunctions( void );
	// Initializes the entity after map has done loading
	virtual void InitEntity( void ) {}
	// Sends init messages for a client on game start/client connect
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) {};
	// Culls by visibility distance if set
	virtual bool CullByVisibilityDistance( const edict_t* pclient ) const;
	// Gets called when entity gets added to a packet list
	virtual void PreAddPacket( void ) { };

	// Tells if the entity has been triggered(used by masters)
	virtual bool IsTriggered( const CBaseEntity* pentity ) const { return true; }
	// Sets a trigger_multiple to wait
	virtual void SetTriggerWait( void ) { };

	// Tells if this is a brush model
	virtual bool IsBrushModel( void ) const;
	// Tells if the entity is moving
	virtual bool IsMoving( void ) const { return m_pState->velocity.IsZero() ? false : true; }

	// Tells if the entity is breakable
	virtual bool IsFuncBreakableEntity( void ) const { return false; }
	// Tells if this entity is a monitor entity
	virtual bool IsFuncMonitorEntity( void ) const { return false; };
	// Tells if this entity is a info_monitorcamera
	virtual bool IsInfoMonitorCameraEntity( void ) const { return false; }
	// Tells if the entity is a pushable entity
	virtual bool IsFuncPushableEntity( void ) const { return false; }
	// Tells if the entity is a door
	virtual bool IsFuncDoorEntity( void ) const { return false; }
	// Tells if the entity is a rotating door
	virtual bool IsFuncDoorRotatingEntity( void ) const { return false; }
	// Tells if the entity is a path_track entity
	virtual bool IsPathTrackEntity( void ) const { return false; }
	// Tells if the entity is a path_track entity
	virtual bool IsPathCornerEntity( void ) const { return false; }
	// Tells if the entity is a func_tracktrain entity
	virtual bool IsFuncTrackTrainEntity( void ) const { return false; }
	// Tells if the entity is a func_train entity or one of it's derivatives
	virtual bool IsFuncTrainEntity( void ) { return false; }
	// Tells if this entity is a trigger_multiple entity
	virtual bool IsTriggerMultipleEntity( void ) const { return false; }
	// Tells if the entity is a func_monsterclip entity
	virtual bool IsFuncNPCClipEntity( void ) const { return false; }
	// Tells if this entity is a func_illusionary
	virtual bool IsFuncIllusionaryEntity( void ) const { return false; }
	// Tells if the entity is a func_wall_toggle entity
	virtual bool IsFuncWallToggleEntity( void ) const { return false; }
	// Tells if the entity is a scripted_sequence entity
	virtual bool IsScriptedSequenceEntity( void ) const { return false; }
	// TRUE if a black hole can pull this entity
	virtual bool CanBlackHolePull( void ) const { return false; }
	// Tells if this entity is a trigger_auto entity
	virtual bool IsTriggerAutoEntity( void ) const { return false; }
	// Tells if entity is a func_button
	virtual bool IsFuncButtonEntity( void ) const { return false; }
	// Tells if this entity is a trigger_changetarget entity
	virtual bool IsTriggerChangeTargetEntity( void ) const { return false; }

	// Triggers targets of this entity
	virtual void UseTargets( CBaseEntity* pActivator, usemode_t useMode, Float value, string_t target = NO_STRING_VALUE );

	// Advances states on diaries
	virtual void AdvanceState( void ) { }
	// Sets monitor for an info_monitorcamera entity
	virtual void AddCameraMonitorEntity( const edict_t* pedict ) { }
	// Gets the entity's illumination
	virtual Int32 GetIllumination( void ) { return 0; }
	// Sets parent door for func_door entities
	virtual void SetParentDoor( CFuncDoor* pParent ) { }
	// Nudges this ground entity
	virtual void GroundEntityNudge( bool noExceptions = false ) { }
	// Called when a related entity is removed
	virtual void ChildEntityRemoved( CBaseEntity* pEntity ) { }
	// Sets a door to be forced to close
	virtual void SetForcedClose( void ) { }
	// Reroutes a train to another path_corner
	virtual void Reroute( CBaseEntity* pTarget, Float speed ) { };
	// Moves a train to a specified path_corner entity
	virtual void MoveTrainToPathCorner( CBaseEntity* pPathCorner, CBaseEntity* pTargetingPathCorner ) { };
	// Returns the number of monitors tied to an info_monitorcamera entity
	virtual Uint32 GetNbCameraMonitors( void ) const { return 0; }
	// Returns a monitor by it's index
	virtual const edict_t* GetMonitorByIndex( Uint32 index ) const { return nullptr; }
	// Checks entity against info_monitorcamera's bounding box
	virtual bool CheckCameraBBox( const edict_t* pedict ) const { return false; }
	// Tells the trigger_multiple to wait
	virtual void TriggerWait( CBaseEntity* pActivator ) { };
	// Tells the wait time for toggle entities
	virtual Float GetWaitTime( void ) { return -1; }
	// Tells the func_button to become unusable for n seconds
	virtual void SetPairedButtonDelay( Float delayTime ) { };

	// Tells if the entity is an envpos_portal entity
	virtual bool IsEnvPosPortalEntity( void ) const { return false; }
	// Adds a func_portal_surface entity
	virtual void AddPortalSurfaceEntity( const edict_t* pedict ) { }
	// Checks the bbox of a portal entity
	virtual bool CheckPortalBBox( const edict_t* pedict ) const { return false; }
	// Tells if the entity is a func_portal_surface entity
	virtual bool IsFuncPortalSurfaceEntity( void ) const { return false; }
	// Returns the envpos_portal entity tied to a portal surface
	virtual const CBaseEntity* GetEnvPosPortalEntity( void ) const { return nullptr; }
	// Tells if the entity is a envpos_portal_world entity
	virtual bool IsEnvPosPortalWorldEntity( void ) const { return false; }

	// Returns toggle state of an entity
	virtual togglestate_t GetToggleState( void ) const { return TS_AT_TOP; }
	virtual void SetToggleState( togglestate_t state, bool reverse ) { };

	// Returns the number of portal surfaces tied to an envpos_portal entity
	virtual Uint32 GetNbPortalSurfaces( void ) const { return 0; }
	// Returns a particular func_portal_surface entity by index
	virtual const edict_t* GetPortalSurfaceByIndex( Uint32 index ) const { return nullptr; }

	// Returns the entity's toggle state
	virtual togglestate_t GetToggleState( void ) { return TS_NONE; }
	// Death notice from child entities
	virtual void ChildDeathNotice( CBaseEntity* pChild ) { };

	// Calls CallUse in USE_TOGGLE mode
	void EXPORTFN CallUseToggleThink( void );
	// Removes the entity
	void EXPORTFN RemoveThink( void );
	// Begins fading the entity
	void EXPORTFN FadeBeginThink( void );
	// Fades the entity out
	void EXPORTFN FadeThink( void );

	// Sets PVS data for entities that meed it
	virtual void SetPVSData( void ) { };
	// Returns PVS data from entities that need it
	virtual const byte* GetPVSData( void ) const { return nullptr; }

	// Returns the delay
	virtual Float GetDelay( void ) { return 0; }
	// Returns the next target entity
	virtual CBaseEntity* GetNextTarget( void );

	// Copies edict data from one entity to the current one
	virtual void CopyEdictData( CBaseEntity* pSrcEntity );
	// Tells if keyvalue can be overridden by base entity class
	virtual bool ShouldOverrideKeyValue( const Char* pstrKeyValue );

	// Sets extra model info after setting the model
	virtual void PostModelSet( void ) { };
	// Tells if entity should set bounds on restore
	virtual bool ShouldSetBoundsOnRestore( void ) { return true; }

	// Stops playback of a diary entry
	virtual void StopDiaryPlayback( void ) { STUBWARNING; }

	// Returns the use reticle mins/maxs
	virtual void GetUseReticleMinsMaxs( Vector& outMins, Vector& outMaxs, CBaseEntity* pPlayer );

public:
	//
	// Animating entity functions
	//

	// Sets a sequence index as the current sequence
	virtual bool SetSequence( Int32 sequenceIndex ) { return false; };
	// Sets a sequence by name
	virtual bool SetSequence( const Char* pstrSequenceName ) { return false; };
	// Sets a sequence index as the current sequence
	virtual Int32 GetSequence( void ) { STUBWARNING; return NO_SEQUENCE_VALUE; };
	// Tells if a sequence is looped
	virtual bool IsSequenceLooped( const Char* pstrSequenceName ) { return false; };
	// Resets animation states
	virtual void ResetSequenceInfo( void ) { };
	// Sets sequence blending
	virtual Float SetBlending( Int32 blenderIndex, Float value ) { STUBWARNING; return 0; };
	// Returns the number of sequence blenders
	virtual Uint32 GetNbBlenders( void ) { return 0; };
	// TRUE if entity is an animating entity
	virtual bool IsAnimatingEntity( void ) { return false; }

	// Precaches sounds for a sequence
	virtual void PrecacheSequenceSounds( Int32 sequenceIndex ) { };
	// Precaches sounds for a sequence
	virtual void PrecacheSequenceSounds( Int32 modelIndex, Int32 sequenceIndex ) { };
	// Precaches sounds for a sequence
	virtual void PrecacheSequenceSounds( const Char* pstrSequenceName ) { };

	// Returns the bodygroup index by name
	virtual Int32 GetBodyGroupIndexByName( const Char* pstrBodygroupName ) { STUBWARNING; return NO_POSITION; }
	// Returns the submodel index in a bodygroup by name
	virtual Int32 GetSubmodelIndexByName( Int32 bodygroupindex, const Char* pstrSubmodelName ) { return NO_SEQUENCE_VALUE; }
	// Sets bodygroup setting
	virtual void SetBodyGroup( Int32 groupindex, Int32 value ) { STUBWARNING; }
	// Finds a sequence by name
	virtual Int32 FindSequence( const char* pstrsequencename ) { return NO_SEQUENCE_VALUE; }
	// Returns sequence duration
	virtual Float GetSequenceTime( Int32 sequenceIndex ) { STUBWARNING; return 0; }

public:
	//
	// Shared by both player and npc code
	//

	// Returns the classification
	virtual Int32 GetClassification( void ) const { return CLASS_NONE; }
	// Returns relation with other entity
	virtual Int32 GetRelationship( CBaseEntity* pOther ) { return R_NONE; }
	// Tells if an entity is in the view cone
	virtual bool IsInView( CBaseEntity* pOther ) const { return false; }
	// Tells if an entity is in the view cone
	virtual bool IsInView( const Vector& position ) const { return false; }

	// Returns the flags for releasing the entity on the client
	virtual Int32 GetFreeMessageFlags( void ) { return FREE_MSG_FL_NONE; }

public:
	//
	// NPC specific functions
	//

	// Decapitates an NPC
	virtual void Decapitate( bool spawnHeadGib ) { };
	// Returns the current enemy
	virtual CBaseEntity* GetEnemy( void ) const { STUBWARNING; return nullptr; }
	// Sets the script state for an npc
	virtual void SetScriptState( scriptstate_t state ) { STUBWARNING; };
	// Returns the script state for NPCs
	virtual scriptstate_t GetScriptState( void ) const { STUBWARNING; return AI_SCRIPT_STATE_NONE; }
	// Returns a pointer to the scripted_sequence entity posessing this entity
	virtual CScriptedSequence* GetScriptedSequence( void ) { STUBWARNING; return nullptr; }
	// Tells if the entity is an NPC
	virtual bool IsNPC( void ) const { return false; }
	// Tells if the NPC is too dangerous to just attack
	virtual bool IsNPCDangerous( void ) const { return false; }
	// Tells if the entity is aware of the entity in the param
	virtual bool IsNPCAwareOf( CBaseEntity* pNPC ) const { return false; }
	// Tells if the NPC has the param ent as an enemy
	virtual bool IsEnemyOfNPC( CBaseEntity* pNPC ) const { return false; }
	// Tells if the entity is an enemy of this entity
	virtual bool IsEnemyOf( CBaseEntity* pNPC ) const { return false; }
	// Tells if the NPC is aware of another one as an enemy
	virtual bool IsAwareOf( CBaseEntity* pNPC ) const { return false; }
	// Sets the NPC's state as blocking another
	virtual void SetPathBlocked( CBaseEntity* pBlockedEntity, const Vector& destination ) { STUBWARNING; };
	// Returns the ideal NPC state
	virtual npcstate_t GetIdealNPCState( void ) { STUBWARNING; return NPC_STATE_NONE; }
	// Sets the ideal NPC state
	virtual void SetIdealNPCState( npcstate_t state ) { STUBWARNING; }
	// Returns the AI state of the NPC
	virtual npcstate_t GetNPCState( void ) { STUBWARNING; return NPC_STATE_NONE; }
	// Sets the last activity time
	virtual void SetLastActivityTime( Double time ) { STUBWARNING; };
	// Returns the ideal activity
	virtual Int32 GetIdealActivity( void ) { STUBWARNING; return ACT_RESET; };
	// Clears AI conditions
	virtual void ClearConditions( Uint64 conditionBits ) { STUBWARNING; };
	// Sets AI conditions
	virtual void SetConditions( Uint64 conditionBits ) { STUBWARNING; };
	// Checks conditions
	virtual bool CheckConditions( Uint64 conditionBits ) const { STUBWARNING; return false; };
	// Adds a new enemy
	virtual void PushEnemy( CBaseEntity* pEnemy, const Vector& lastPosition, const Vector& lastAngles ) { STUBWARNING; };
	// Sets the current enemy
	virtual void SetEnemy( CBaseEntity* pEnemy ) { STUBWARNING; };
	// Sets the last time the player was sighted
	virtual void SetLastPlayerSightTime( Double time ) { STUBWARNING; };
	// Forgets the player
	virtual void ForgetPlayer( CBaseEntity* pPlayer ) { STUBWARNING; };

	// Set memory bits
	virtual void SetMemory( Uint64 memoryBits ) { STUBWARNING; };
	// Sets current activity for npcs
	virtual void SetCurrentActivity( enum activity_t activity ) { STUBWARNING; };
	// Exits the current scripted_sequence
	virtual void ExitScriptedSequence( void ) { STUBWARNING; };
	// Cleans up the current scripted_sequence
	virtual void CleanupScriptedSequence( void ) { STUBWARNING; };
	// Tells if an entity can play a scripted sequence
	virtual bool CanPlaySequence( bool disregardState, script_interrupt_level_t interruptLevel ) { STUBWARNING; return false; };
	// Tells if the entity can play a scripted sentence
	virtual bool CanPlaySentence( bool disregardState ) { STUBWARNING; return false; }

	// Performs pfnWalkMove on the entity
	bool WalkMove( Float yaw, Float dist, enum walkmove_t movemode );
	// Checks a localmove between two points
	virtual localmove_t CheckLocalMove( const Vector startPosition, const Vector& endPosition, const CBaseEntity* pTargetEntity, Float* pDistance = nullptr, bool isInitial = false ) { STUBWARNING; return LOCAL_MOVE_INVALID_NO_TRIANGULATION; }

	// Sets the goal entity for npcs
	virtual void SetGoalEntity( CBaseEntity* pEntity ) { STUBWARNING; };
	// Sets the scripted sequence entity for npcs
	virtual void SetScriptEntity( CScriptedSequence* pScriptEntity ) { STUBWARNING; };
	// Sets the target entity for npcs
	virtual void SetTargetEntity( CBaseEntity* pEntity ) { STUBWARNING; };
	// Changes the schedule to a new schedule
	virtual void ChangeSchedule( const CAISchedule* pNewSchedule ) { STUBWARNING; };
	// Clears current schedule
	virtual void ClearSchedule( void ) { STUBWARNING; };

	// Plays a scripted_sentence
	virtual void PlayScriptedSentence( const Char* pstrSentenceName, Float duration, Float volume, Float attenuation, Float timeOffset, bool subtitleOnlyInRadius, bool isConcurrent, CBaseEntity* pListener, CBaseEntity* pPlayer = nullptr ) { STUBWARNING; };

	// Reports the current AI state
	virtual void ReportAIState( void ) { STUBWARNING; };

	// Sets a trigger condition
	virtual void SetAITriggerCondition( Int32 conditionIndex, Int32 condition, const Char* pstrTarget ) { STUBWARNING; };

	// Tells if weapons can be dropped
	virtual bool CanDropWeapons( void ) const { STUBWARNING; return false; };
	// Sets whether npc can drop weapons
	virtual void SetCanDropWeapons( bool canDrop ) { STUBWARNING; };

	// Returns the squad leader
	virtual CBaseEntity* GetSquadLeader( void ) { STUBWARNING; return this; };

	// Tells if the NPC is npc_security
	virtual bool IsNPCSecurity( void ) { return false; }

	// Sets head value of an NPC
	virtual void SetHead( Int32 head ) { };

	// Calls to gib the NPC
	virtual void CallGibNPC( void ) { };
	// Callback function for spawning gibs
	virtual void OnGibSpawnCallback( CBaseEntity* pGib ) { };

	// Return damage multiplier for hitgroup
	virtual Float GetHitgroupDmgMultiplier( Int32 hitgroup ) { return 1.0; };

	// Sets the NPC to be pulled by a trigger_pullnpc
	virtual void SetNPCPuller( CBaseEntity* pPuller, const Vector& pullPosition ) { };
	// Removes a pulled NPC from the list
	virtual void RemovePulledNPC( CBaseEntity* pNPC ) { };

	// Tells if NPC should favor ranged attacks versus melee
	virtual bool FavorRangedAttacks( void ) const { return false; }

	// Tells if NPC is an infectous NPC
	virtual bool IsInfectiousNPC( void ) const { return false; }

public:
	//
	// Squad npc specific functions
	//

	// Returns the squad slots
	virtual Uint64 GetSquadSlots( void ) { STUBWARNING; return 0; };
	// Returns the squad slots
	virtual void SetSquadSlots( Uint64 squadSlots ) { STUBWARNING; };
	// Removes a squad member
	virtual void RemoveNPCFromSquad( CBaseEntity* pRemoveNPC ) { STUBWARNING; };
	// Sets the squad leader
	virtual void SetSquadLeader( CBaseEntity* pSquadLeader ) { STUBWARNING; };
	// Removes a member of the squad
	virtual void RemoveSquadMember( CBaseEntity* pRemoveNPC ) { STUBWARNING; };
	// Sets the enemy's last known position and angles
	virtual void SetEnemyInfo( const Vector& enemyLKP, const Vector& enemyLKA ) { STUBWARNING; };
	// Gets the enemy information
	virtual void GetEnemyInfo( Vector& enemyLKP, Vector& enemyLKA ) { STUBWARNING; };
	// Sets the last enemy sight time
	virtual void SetLastEnemySightTime( Double time ) { STUBWARNING; };
	// Sets the last enemy sight time
	virtual Double GetLastEnemySightTime( void ) { STUBWARNING; return 0; };
	// Sets whether the enemy has eluded us
	virtual void SetEnemyEluded( bool enemyEluded ) { STUBWARNING; };
	// Tells whether the enemy has eluded us
	virtual bool HasEnemyEluded( void ) { STUBWARNING; return false; }
	// Returns the squad leader
	virtual CBaseEntity* GetSquadMember( Uint32 index ) { STUBWARNING; return nullptr; };
	// Tells if the NPC is of squadNPC type
	virtual bool IsSquadNPC( void ) const { return false; }
	// Tells if this npc is the leader
	virtual bool IsSquadLeader( void ) const { STUBWARNING; return false; }
	// Adds an NPC to the squad
	virtual bool AddToSquad( CBaseEntity* pAddNPC ) { STUBWARNING; return false; };

public:
	//
	// Talknpc specific functions
	//

	// Tells if the NPC is a medic
	virtual bool IsMedic( void ) const { return false; }
	// Tells if this NPC can heal another
	virtual bool MedicCanHeal( CBaseEntity* pHealTarget ) { STUBWARNING; return false; }
	// Tells if the entity is a talking NPC
	virtual bool IsTalkNPC( void ) const { return false; }
	// Stops following the target
	virtual void StopFollowing( bool clearSchedule ) { STUBWARNING; };
	// Tells if talking npc can answer
	virtual bool CanAnswer( void ) { STUBWARNING; return true; };
	// Sets to answer a question
	virtual void SetAnswerQuestion( CBaseEntity* pSpeaker, Double talkTime ) { STUBWARNING; };
	// Sets the follow target for this NPC
	virtual void SetFollowTarget( CBaseEntity* pTarget ) { STUBWARNING; };
	// Tells if NPC is following a player
	virtual bool IsFollowing( void ) const { return false; }
	// Manages a healer call
	virtual void NPCHealerCall( CBaseEntity* pCaller ) { STUBWARNING; };

public:
	//
	// Patrol npc specific functions
	//

	// Validates whether an intended patrol destination should be visited
	virtual bool ValidatePatrolDestination( const Vector& destPosition ) { STUBWARNING; return true; };
	// Pushes a patrol destination to the history stack
	virtual void PushPatrolDestination( const Vector& destPosition ) { STUBWARNING; };
	// Prompts a patrol error
	virtual void PatrolErrorPrompt( CBaseEntity* pEntity ) { STUBWARNING; };
	// Returns the last node index a squad member chose to visit
	virtual void SetLastPatrolNodeIndex( Int32 nodeIndex ) { STUBWARNING; };
	// Returns the last node index a squad member chose to visit
	virtual Int32 GetLastPatrolNodeIndex( void ) { STUBWARNING; return NO_POSITION; };
	// Clears patrol history stack
	virtual void ClearPatrolHistory( void ) { STUBWARNING; };

public:
	//
	// Wander npc specific functions
	//

	// Tells if this is a wandering NPC
	virtual bool IsWanderNPC( void ) { return false; }
	// Returns the wander node used
	virtual Int32 GetWanderNode( void ) { STUBWARNING; return NO_POSITION; };
	// Sets wander state
	virtual void SetWanderState( bool state ) { STUBWARNING; }
	// Returns the wander state
	virtual bool GetWanderState( void ) { STUBWARNING; return false; }

public:
	// 
	// Random NPC functions
	//
	
	// Tells how many grenades this NPC has
	virtual Uint32 GetNumGrenades( void ) { STUBWARNING; return 0; }
	// Sets grenade count on NPC
	virtual void SetNumGrenades( Uint32 numGrenades ) { STUBWARNING; }

public:
	// 
	// Other
	//

	// Sets whether the NPC should always remain alert
	virtual void SetAlwaysAlert( bool alwaysAlert ) { };
	// Tells the parent that the child NPC has finished being born
	virtual void ChildFinishedBirth( void ) { STUBWARNING; };

public:
	//
	// Player specific functions
	//

	// Tells if the entity is a weapon
	virtual bool IsWeapon( void ) const { return false; }
	// Returns the weapon information
	virtual bool GetWeaponInfo( weaponinfo_t* pWeapon ) { STUBWARNING; return false; };
	// Gives ammo to the player
	virtual Int32 GiveAmmo( Int32 amount, const Char* pstrammoname, Int32 max, bool display, CBaseEntity* pentity ) { STUBWARNING; return -1; };
	// Manages textmessage and textmessage_team functions
	virtual void HostSay( const Char* pstrText, bool teamonly ) { STUBWARNING; };
	// Drops active weapon
	virtual void DropCurrentWeapon( void ) { STUBWARNING; };
	// Gives an item by clasname
	virtual void GiveItemByName( const Char* pstrClassname, Uint32 amount, bool removeunneeded ) { STUBWARNING; };
	// Selects the last weapon used
	virtual void SelectPreviousWeapon( void ) { STUBWARNING; };
	// Tells if the entity is a player
	virtual bool IsPlayer( void ) const { return false; }
	// Tells if the player is on a motorbike
	virtual bool IsOnMotorBike( void ) const { STUBWARNING; return false; }
	// Called when using a ladder
	virtual void EnterLadder( CEnvLadder *pLadder ) { STUBWARNING; };
	// Sets the room type
	virtual void SetRoomType( Int32 roomtype ) { STUBWARNING; };
	// Sets special fog state
	virtual void SetSpecialFog( bool specialfogenabled ) { STUBWARNING; };
	// Get day stage
	virtual daystage_t GetDayStage( void ) const { STUBWARNING; return DAYSTAGE_NORMAL; };
	// Sets day state
	virtual void SetDayStage( daystage_t daystage ) { STUBWARNING; };
	// Sets dialouge duration for player
	virtual void SetDialougeDuration( Float duration ) { STUBWARNING; }

	// Set NPC awareness
	virtual void SetNPCAwareness( Float awareness, CBaseEntity* pNPC, Float timeoutDelay ) { STUBWARNING; };

	// Adds a medkit to the player
	virtual bool AddMedkit( const Char* pstrClassname, bool noNotice ) { STUBWARNING; return false; };
	// Adds a medkit to the player
	virtual bool AddKevlar( const Char* pstrClassname, bool noNotice ) { STUBWARNING; return false; };
	// Adds a shoulder light to the player
	virtual bool AddShoulderLight( const Char* pstrClassname, bool noNotice ) { STUBWARNING; return false; };

	// When player gets on the bike
	virtual void EnterBike( CItemMotorBike *pEntity ) { STUBWARNING; };

	// Adds a new passcode
	virtual void AddPasscode( const Char* pstrid, const Char* pstrpasscode ) { STUBWARNING; };

	// Tells if forced holstering is set
	virtual bool IsForceHolsterSet( void ) const { STUBWARNING; return false; }
	// Sets force holster state
	virtual void SetForceHolster( bool forceholster ) { STUBWARNING; };
	// Sets forced slow movement
	virtual void SetForceSlowMove( bool forceslowmove, bool nosprinting ) { STUBWARNING; };
	// Sets dream state
	virtual void SetIsInDream( bool isindream ) { STUBWARNING; };

	// Spawns a text window
	virtual void SpawnTextWindow( const Char* pstrfilepath, const Char* pstrpasscode, const Char* pstrid ) { STUBWARNING; };
	// Spawns a keypad window
	virtual void SpawnKeypadWindow( const Char* pstrid, const Char* pstrkeypadcode, CTriggerKeypad* pkeypad, bool staytillnext ) { STUBWARNING; };
	// Spawns a subway window
	virtual void SpawnSubwayWindow( subwayline_t type, CTriggerSubwayController* pcontroller, bool isdummy ) { STUBWARNING; };
	// Spawns a login window
	virtual void SpawnLoginWindow( const Char* pstruser, const Char* pstrpasscode, const Char* pstrid, CTriggerLogin* plogin, bool staytillnext ) { STUBWARNING; };
	// Sets subway flag
	virtual void SetSubwayFlag( Int32 flag ) { STUBWARNING; };

	// Selects a named weapon
	virtual void SelectWeapon( const Char* pstrWeaponName ) { STUBWARNING; };
	// Selects a weapon by weapon Id
	virtual void SelectWeapon( weaponid_t weaponId ) { STUBWARNING; };
	// Returns the active weapon
	virtual CPlayerWeapon* GetActiveWeapon( void ) { STUBWARNING; return nullptr; };
	// Returns the weapon list pointer
	virtual CPlayerWeapon* GetWeaponList( void ) { STUBWARNING; return nullptr; };
	// Returns ammo count for a type
	virtual Int32 GetAmmoCount( Int32 ammotype ) const { STUBWARNING; return 0; };
	// Sets ammo count for a type
	virtual void SetAmmoCount( Int32 ammotype, Int32 ammocount ) { STUBWARNING; };
	// Adds item to player
	virtual bool AddPlayerWeapon( CPlayerWeapon* pWeapon ) { STUBWARNING; return false; };
	// Sets the save-game title
	virtual void SetSaveGameTitle( const Char* pstrtitle ) { STUBWARNING; };
	// Returns the save-game title
	virtual const Char* GetSaveGameTitle( void ) { STUBWARNING; return ""; };
	// Tells if the player can have a weapon
	virtual bool CanHaveWeapon( CPlayerWeapon* pWeapon ) const { STUBWARNING; return false; };
	// Callback for firing a round
	virtual void OnWeaponShotImpact( const Vector& traceStart, const trace_t& tr, bool isPenetrationShot ) { };
	// Callback for getting skill cvar multiplier for weapon
	virtual Float GetWeaponDmgMultiplier( CBaseEntity* pOther ) { return 1.0; }

	// Sets the view entity
	virtual void SetViewEntity( CBaseEntity* pEntity ) { STUBWARNING; };
	// Sets control enabled state
	virtual void SetControlEnable( bool enable ) { STUBWARNING; };
	// Set player paralysis bitflag
	virtual void SetPlayerParalyzed( bool enable ) { STUBWARNING; };
	// Tells whether player is paralyzed
	virtual bool GetIsPlayerParalyzed( void ) { return false; };
	// Set camera entity
	virtual void SetCameraEntity( CTriggerCameraModel* pCamera ) { STUBWARNING; };

	// Adds full ammo for dual-wielded weapons
	virtual bool AddFullAmmoDual( CPlayerWeapon* pWeapon ) const { STUBWARNING; return false; };
	// Tells if the player can have an ammo type
	virtual bool CanHaveAmmo( const Char* pstrammotype, Int32 maxammo ) const { STUBWARNING; return false; };
	// Tries to add accessories from pWeapon
	virtual void AddAccessories( CPlayerWeapon* pWeapon ) { };

	// Plays a music track for a player
	virtual void PlayMusic( const Char* pstrFilename, Int32 channel, Float fadeInTime, Int32 flags ) { STUBWARNING; };
	// Stops any playing music tracks
	virtual void StopMusic( const Char* pstrFilename, Int32 channel, Float fadeTime ) { STUBWARNING; };

	// Unducks the player
	virtual void UnDuckPlayer( void ) { STUBWARNING; };

	// Begins playback of a tape track
	virtual void PlaybackTapeTrack( const Char* pstrTrackFilename, Float duration, const Char* pstrPlaybackTitle, const Vector& titleColor, Float titleAlpha ) { STUBWARNING; };
	// Stops playback of a tape track
	virtual void StopTapeTrack( const Char* pstrTrackFilename ) { STUBWARNING; };

	// Begins playback of a diary
	virtual void BeginDiaryPlayback( const Char* pstrFilename, Float duration, CBaseEntity* pDiaryEntity ) { STUBWARNING; };

	// Adds a new mission objective
	virtual void AddMissionObjective( const Char* pstrObjectiveIdentifier, bool notify ) { STUBWARNING; };
	// Removes a new mission objective
	virtual void RemoveMissionObjective( const Char* pstrObjectiveIdentifier, bool notify ) { STUBWARNING; };
	// Spawns an objectives window
	virtual void SpawnObjectivesWindow( void ) { STUBWARNING; };

	// Sets countdown timer
	virtual void SetCountdownTimer( Float duration, const Char* pstrTitle ) { STUBWARNING; };
	// Clears countdown timer
	virtual void ClearCountdownTimer( void ) { STUBWARNING; };

	// Sets a global delayed trigger
	virtual void SetGlobalDelayedTrigger( Float delay, const Char* pstrTargetName ) { STUBWARNING; };
	// Clears a global delayed trigger
	virtual void ClearGlobalDelayedTrigger( void ) { STUBWARNING; };

public:
	//
	// Custer Phantom NPC functions
	//

	// Returns the warp spot info of this NPC
	virtual bool GetWarpSpotInfo( Vector& warpSpot, Int32& nodeIndex ) { STUBWARNING; return false; }
	// Delays spawning if waiting for it
	virtual void DelayCusterSpawning( void ) { STUBWARNING; };

public:
	//
	// Edict specific data accessors and setters
	//

	// Returns the entity index
	inline entindex_t GetEntityIndex( void ) const;
	// Returns the edict pointer
	inline const edict_t* GetEdict( void ) const;

	// Returns the entity's origin
	inline const Vector& GetOrigin( void ) const;
	// Sets the entity's origin
	inline void SetOrigin( const Vector& origin );

	// Returns the entity's angles
	inline const Vector& GetAngles( void ) const;
	// Sets the entity's angles
	inline void SetAngles( const Vector& angles );

	// Returns the entity's velocity
	inline const Vector& GetVelocity( void ) const;
	// Sets the entity's velocity
	inline void SetVelocity( const Vector& velocity );

	// Returns the angular velocity
	inline const Vector& GetAngularVelocity( void ) const;
	// Sets the angular velocity
	inline void SetAngularVelocity( const Vector& velocity );

	// Returns the entity's base velocity
	inline const Vector& GetBaseVelocity( void ) const;
	// Sets the entity's base velocity
	inline void SetBaseVelocity( const Vector& basevelocity );

	// Sets the entity's flag
	inline void SetFlags( Uint64 flagbits );
	// Sets the entity's flag
	inline void RemoveFlags( Uint64 flagbits );
	// Returns the entity's flags
	inline Int64 GetFlags( void ) const;

	// Tells if the entity has a targetname
	inline bool HasTargetName( void ) const;
	// Returns the targetname of the entity
	inline const Char* GetTargetName( void ) const;
	// Sets the entity's targetname
	inline void SetTargetName( const Char* pstrtargetname );
	// Sets the entity's targetname
	inline void SetTargetName( const string_t targetname );

	// Tells if the entity has a targetname
	inline bool HasModelName( void ) const;
	// Returns the model name of the entity
	inline const Char* GetModelName( void ) const;
	// Sets the entity's model name
	inline void SetModelName( const Char* pstrmodelname );
	// Sets the entity's model name
	inline void SetModelName( const string_t modelname );

	// Get entity's modelindex
	inline Int32 GetModelIndex( void ) const;

	// Tells if the entity is a visible entity
	inline bool IsVisible( void ) const;

	// Tells if the entity has a targetname
	inline bool HasTarget( void ) const;
	// Returns the target of the entity
	inline const Char* GetTarget( void ) const;
	// Sets the entity's target
	inline void SetTarget( const Char* pstrtarget );
	// Sets the entity's target
	inline void SetTarget( const string_t target );

	// Returns the entity's solidity
	inline solid_t GetSolidity( void ) const;
	// Sets the entity's solidity
	inline void SetSolidity( solid_t solid );

	// Returns absolute world mins
	inline const Vector& GetAbsMins( void ) const;
	// Returns absolute world maxs
	inline const Vector& GetAbsMaxs( void ) const;

	// Sets mins/maxs
	inline void SetMinsMaxs( const Vector& mins, const Vector& maxs );
	// Returns relative mins
	inline const Vector& GetMins( void ) const;
	// Returns relative maxs
	inline const Vector& GetMaxs( void ) const;

	// Returns thw classname of this entity
	inline const Char* GetClassName( void ) const;
	// Returns takedamage setting
	inline takedamage_t GetTakeDamage( void ) const;

	// Returns button bits(for player)
	inline Int32 GetButtonBits( void ) const;
	// Sets button bits(for player)
	inline void SetButtonBits( Int32 bitflags );
	// Clears specific button bits(for player)
	inline void ClearButtonBits( Int32 bitflags );

	// Returns the entity's view offset
	inline Vector GetViewOffset( bool fromNavigable = false ) const;
	// Sets the entity's view offset
	inline void SetViewOffset( const Vector& offset );

	// Returns view angles(for player)
	inline const Vector& GetViewAngles( void ) const;
	// Sets view angles(for player)
	inline void SetViewAngles( const Vector& angles );

	// Sets aiment
	inline void SetAiment( CBaseEntity* pEntity );
	// Returns the aiment
	inline CBaseEntity* GetAiment( void ) const;

	// Sets owner
	inline void SetOwner( CBaseEntity* pEntity );
	// Returns owner
	inline CBaseEntity* GetOwner( void ) const;

	// Sets ground entity
	inline void SetGroundEntity( CBaseEntity* pEntity );
	// Returns ground entity
	inline CBaseEntity* GetGroundEntity( void ) const;

	// Returns the entity's waterlevel
	inline waterlevel_t GetWaterLevel( void ) const;
	// Sets the entity's waterlevel
	inline void SetWaterLevel( waterlevel_t level );

	// Returns the size of the object
	inline const Vector& GetSize( void ) const;
	// Sets the size of the object
	inline void SetSize( const Vector& size );

	// Returns the punch angles
	inline const Vector& GetPunchAngle( void ) const;
	// Sets punch angles
	inline void SetPunchAngle( const Vector& value );

	// Returns the punch angles
	inline const Vector& GetPunchAmount( void ) const;
	// Sets punch amount
	inline void SetPunchAmount( const Vector& value );

	// Returns the spawnflags
	inline Int64 GetSpawnFlags( void ) const;
	// Tells if a spawnflag is set
	inline bool HasSpawnFlag( Int64 bit ) const;
	// Sets a spawnflag
	inline void SetSpawnFlag( Int64 flag );
	// Sets a spawnflag
	inline void RemoveSpawnFlag( Int64 flag );

	// Return the render color
	inline const Vector& GetRenderColor( void ) const;
	// Sets the render color
	inline void SetRenderColor( const Vector& color );
	// Sets the render color
	inline void SetRenderColor( Int32 r, Int32 g, Int32 b );

	// Returns the rendermode of the entity
	inline rendermode_t GetRenderMode( void ) const;
	// Sets the rendermode of the entity
	inline void SetRenderMode( rendermode_t rendermode );

	// Gets the render amount
	inline const Float GetRenderAmount( void ) const;
	// Sets the render amount
	inline void SetRenderAmount( Float amount );

	// Gets the scale
	inline const Float GetScale( void ) const;
	// Sets the scale
	inline void SetScale( Float scale );

	// Gets the renderfx
	inline const Int32 GetRenderFx( void ) const;
	// Sets the renderfx
	inline void SetRenderFx( Int32 renderfx );

	// Gets the render type
	inline const rendertype_t GetRenderType( void ) const;

	// Gets the scale
	inline const Float GetFramerate( void ) const;
	// Sets the scale
	inline void SetFramerate( Float framerate );

	// Gets the frame
	inline const Float GetFrame( void ) const;
	// Sets the frame
	inline void SetFrame( Float frame );

	// Gets the animation time
	inline const Double GetAnimationTime( void ) const;
	// Sets the animation time
	inline void SetAnimationTime( Double animtime );

	// Returns the spawnflags
	inline Int64 GetEffectFlags( void ) const;
	// Tells if a spawnflag is set
	inline bool HasEffectFlag( Int64 bit ) const;
	// Sets a spawnflag
	inline void SetEffectFlag( Int64 flag );
	// Sets a spawnflag
	inline void RemoveEffectFlag( Int64 flag );

	// Returns the speed of the entity
	inline Float GetSpeed( void ) const;
	// Sets the speed of the entity
	inline void SetSpeed( Float speed );

	// Returns the parent
	inline CBaseEntity* GetParent( void ) const;
	// Sets the parent
	inline void SetParent( CBaseEntity* pEntity );
	// Tells if entity is parented
	inline bool IsParented( void ) const;

	// Returns the skin value
	inline Int32 GetSkin( void ) const;
	// Sets the skin value
	inline void SetSkin( Int32 value );

	// Returns the body value
	inline Int64 GetBody( void ) const;
	// Sets the skin value
	inline void SetBody( Int64 value );

	// Returns the health value
	inline Float GetHealth( void ) const;
	// Sets the health value
	inline void SetHealth( Float value );

	// Returns the max health value
	inline Float GetMaxHealth( void ) const;
	// Sets the max health value
	inline void SetMaxHealth( Float value );

	// Returns the body value
	inline Float GetArmorValue( void ) const;
	// Sets the skin value
	inline void SetArmorValue( Float value );

	// Tells if the entity has a netname
	inline bool HasNetname( void ) const;
	// Returns the netname of the entity
	inline const Char* GetNetname( void ) const;
	// Sets the netname of the entity
	inline void SetNetname( const Char* pstrValue );

	// Tells if the entity has a message set
	inline bool HasMessage( void ) const;
	// Returns the netname of the entity
	inline const Char* GetMessage( void ) const;
	// Sets the netname of the entity
	inline void SetMessage( const Char* pstrValue );

	// Tells if the entity has a globalname set
	inline bool HasGlobalName( void ) const;
	// Returns the globalname of the entity
	inline const Char* GetGlobalName( void ) const;
	// Sets the globalname of the entity
	inline void SetGlobalName( const Char* pstrValue );

	// Returns the move type
	inline movetype_t GetMoveType( void ) const;
	// Sets the move type
	inline void SetMoveType( movetype_t movetype );

	// Returns the next think time
	inline Double GetNextThinkTime( void ) const;
	// Sets the next thinking time
	inline void SetNextThinkTime( Double thinktime );
	// Sets next think time
	inline void SetNextThink( Double delay );
	// Returns the local time variable's value
	inline Double GetLocalTime( void ) const;

	// Gets the ideal yaw
	inline const Float GetIdealYaw( void ) const;
	// Sets the ideal yaw
	inline void SetIdealYaw( Float idealyaw );

	// Returns the entity's gravity setting
	inline Float GetGravity( void ) const;
	// Sets the entity's gravity
	inline void SetGravity( Float gravity );

	// Returns the fall velocity
	inline Float GetFallingVelocity( void ) const;
	// Sets the falling velocity
	virtual void SetFallingVelocity( Float velocity );

	// Returns the friction value
	inline Float GetFriction( void ) const;
	// Sets the friction value
	inline void SetFriction( Float friction );

	// Returns the plane z cap value
	inline Float GetPlaneZCap( void ) const;
	// Sets the plane z cap value
	inline void SetPlaneZCap( Float planeZCap );

	// Returns the deadstate
	inline deathstate_t GetDeadState( void ) const;
	// Sets the dead state
	inline void SetDeadState( deathstate_t state );

	// Returns the weapons value
	inline Int64 GetWeapons( void ) const;
	// Sets the weapons value
	inline void SetWeapons( Int64 weapons );

	// Drops the entity to the floor
	inline bool DropToFloor( void );

	// Sets the next think time(used by trains only)
	virtual void SetNextThink( Double thinkTime, bool alwaysThink ) { };

	// Sets the model
	inline bool SetModel( const Char* pstrModelName, bool setbounds = true );
	// Sets the model
	inline bool SetModel( string_t modelNameString, bool setbounds = true );

	// Set current think time
	void UpdateLastThinkTime( void );

public:
	// Saves all data for the entity
	void SaveEntityClassData( bool istransitionsave );
	// Reads entity class data
	bool ReadEntityClassData( const Char* fieldname, const byte* pdata, Uint32 datasize, Uint32 blockindex, bool istransferglobalentity );

public:
	// Retreive class data for an edict
	static CBaseEntity* GetClass( edict_t* pedict );
	// Retreive class data for an edict
	static const CBaseEntity* GetClass( const edict_t* pedict );
	// Calls for classes and their children
	virtual void DeclareSaveFields( void );

#ifdef _DEBUG
public:
	// Verifies that the function is properly exported
	static void CheckFunction( void* pfnptr, const Char* pstrFunctionName );
	// Sets the think function in Debug mode
	THINKFNPTR _SetThink( THINKFNPTR pfnptr, const Char* pstrFunctionName );
	// Sets the touch function in Debug mode 
	INTERACTFNPTR _SetTouch( INTERACTFNPTR pfnptr, const Char* pstrFunctionName );
	// Sets the blocked function in Debug mode
	INTERACTFNPTR _SetBlocked( INTERACTFNPTR pfnptr, const Char* pstrFunctionName );
	// Sets the blocked function in Debug mode
	USEFNPTR _SetUse( USEFNPTR pfnptr, const Char* pstrFunctionName );
#endif

protected:
	// Think function pointer
	void (CBaseEntity::*m_pfnThinkFunction)( void );
	// Touch function pointer
	void (CBaseEntity::*m_pfnTouchFunction)( CBaseEntity* pOther );
	// Blocked function pointer
	void (CBaseEntity::*m_pfnBlockedFunction)( CBaseEntity* pOther );
	// Use function pointer
	void (CBaseEntity::*m_pfnUseFunction)( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value );

protected:
	// Only managed by base class
	void DeclareSaveField( entity_data_desc_t desc );

public:
	// Creates a new entity
	static CBaseEntity* CreateEntity( const Char* pstrClassName, const Vector& origin, const Vector& angles, CBaseEntity* powner );
	// Creates a new entity
	static CBaseEntity* CreateEntity( const Char* pstrClassName, CBaseEntity* powner );

protected:
	// Save data info for particular entity
	CLinkedList<entity_data_desc_t> m_saveFieldsList;
	// Entity state ptr
	entity_state_t* m_pState;
	// Entity fields ptr
	edict_fields_t* m_pFields;
	// Edict pointer
	edict_t* m_pEdict;
	// Previous think time
	Double m_lastThinkTime;

	// Visibility cull distance
	Int32 m_visibilityCullDistance;
};
#include "baseentity_inline.hpp"
#endif //BASEENTITY_H