/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef NPCSECURITY_H
#define NPCSECURITY_H

#include "ai_wandernpc.h"

//=============================================
//
//=============================================
class CNPCSecurity : public CWanderNPC
{
public:
	// Glock clip size
	static const Uint32 NPC_GLOCK_CLIP_SIZE;
	// Desert eagle clip size
	static const Uint32 NPC_DEAGLE_CLIP_SIZE;
	// TRG42 clip size
	static const Uint32 NPC_TRG42_CLIP_SIZE;
	// View offset for npc
	static const Vector NPC_VIEW_OFFSET;
	// Attachment for weapon
	static const Uint32 NPC_WEAPON_ATTACHMENT_INDEX;
	// Yaw speed for npc
	static const Uint32 NPC_YAW_SPEED;
	// Gun position offset
	static const Vector NPC_GUN_POSITION_OFFSET;

	// Weapon flags
	enum npc_weaponflags_t
	{
		NPC_WEAPON_GLOCK	= (1<<0),
		NPC_WEAPON_DEAGLE	= (1<<1),
		NPC_WEAPON_TRG42	= (1<<2)
	};

	// Animation events
	enum npc_animevents_t
	{
		NPC_AE_DRAW			= 2,
		NPC_AE_SHOOT		= 4,
		NPC_AE_RELOAD
	};

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
	// Sentence prefix for npc
	static const Char NPC_SENTENCE_PREFIX[];

	// Bodygroup name for heads
	static const Char NPC_BODYGROUP_HEADS_NAME[];
	// Submodel name for head 1
	static const Char NPC_SUBMODEL_HEAD1_NAME[];
	// Submodel name for head 2
	static const Char NPC_SUBMODEL_HEAD2_NAME[];

	// Bodygroup name for guns
	static const Char NPC_BODYGROUP_WEAPONS_NAME[];
	// Submodel name for holstered glock
	static const Char NPC_SUBMODEL_WEAPON_GLOCK_HOLSTERED_NAME[];
	// Submodel name for holstered desert eagle
	static const Char NPC_SUBMODEL_WEAPON_DEAGLE_HOLSTERED_NAME[];
	// Submodel name for holstered glock
	static const Char NPC_SUBMODEL_WEAPON_GLOCK_UNHOLSTERED_NAME[];
	// Submodel name for holstered desert eagle
	static const Char NPC_SUBMODEL_WEAPON_DEAGLE_UNHOLSTERED_NAME[];
	// Submodel name for TRG 42
	static const Char NPC_SUBMODEL_WEAPON_TRG42_NAME[];
	// Submodel name for blank weapon
	static const Char NPC_SUBMODEL_WEAPON_BLANK_NAME[];

public:
	explicit CNPCSecurity( edict_t* pedict );
	virtual ~CNPCSecurity( void );

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Performs precache functions
	virtual void Precache( void ) override;
	// Manages a keyvalue
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	// Returns entity property flags
	virtual Int32 GetEntityFlags( void ) override;
	// Sets extra model info after setting the model
	virtual void PostModelSet( void ) override;

	// Returns the classification
	virtual Int32 GetClassification( void ) const override;
	// Handles animation events
	virtual void HandleAnimationEvent( const mstudioevent_t* pevent ) override;
	// Returns a sequence for an activity type
	virtual Int32 FindActivity( Int32 activity ) override;

	// Makes the entity take on damage
	virtual bool TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags ) override;
	// Manages getting killed
	virtual void Killed( CBaseEntity* pAttacker, gibbing_t gibbing, deathmode_t deathMode ) override;

	// Tells if the NPC is npc_security
	virtual bool IsNPCSecurity( void ) override { return true; }

	// Declares save-restore fields
	virtual void DeclareSaveFields( void ) override;

	// Sets head value of an NPC
	virtual void SetHead( Int32 head ) override { m_headSetting = head; }

public:
	// Initializes talknpc data
	virtual void InitTalkingNPC( void ) override;

	// Sets the ideal yaw speed
	virtual void SetYawSpeed( void ) override;
	// Returns the sound mask for the NPC
	virtual Uint64 GetSoundMask( void ) override;

	// Runs a task
	virtual void RunTask( const ai_task_t* pTask ) override;

	// Returns the ideal schedule
	virtual const CAISchedule* GetSchedule( void ) override;
	// Returns a schedule by it's index
	virtual const CAISchedule* GetScheduleByIndex( Int32 scheduleIndex ) override;

	// Checks if we can do range attack 1
	virtual bool CheckRangeAttack1( Float dp, Float distance ) override;

	// Checks the ammo capacity
	virtual void CheckAmmo( void ) override;

	// Returns the gun position
	virtual Vector GetGunPosition( stance_t stance = STANCE_ACTUAL ) override;

	// Plays pain sounds
	virtual void EmitPainSound( void ) override;
	// Plays death sounds
	virtual void EmitDeathSound( void ) override;
	// Plays alert sounds
	virtual void EmitAlertSound( void ) override;

	// Returns the firing cone used
	virtual const Uint32 GetFiringCone( bool attenuateByFog = false ) override;
	// Return bullet type used by NPC
	virtual bullet_types_t GetBulletType( void ) override;
	// Tells if NPC should favor ranged attacks versus melee
	virtual bool FavorRangedAttacks( void ) const override { return true; }

private:
	// TRUE if gun is drawn
	bool m_isGunDrawn;
	// TRUE if hostile to player
	bool m_isHostile;

	// Head setting
	Int32 m_headSetting;

	// Next time we can moan in pain
	Double m_nextPainTime;

private:
	// Head bodygroup index
	Int32 m_headBodyGroupIndex;
	// Head 1 submodel index
	Int32 m_head1SubmodelIndex;
	// Head 2 submodel index
	Int32 m_head2SubmodelIndex;

	// Weapons bodygroup index
	Int32 m_weaponsBodyGroupIndex;
	// Glock holstered submodel index
	Int32 m_weaponGlockHolsteredSubmodelIndex;
	// Desert Eagle holstered submodel index
	Int32 m_weaponDEagleHolsteredSubmodelIndex;
	// Glock unholstered submodel index
	Int32 m_weaponGlockUnholsteredSubmodelIndex;
	// Desert Eagle unholstered submodel index
	Int32 m_weaponDEagleUnholsteredSubmodelIndex;
	// TRG42 submodel index
	Int32 m_weaponTRG42SubmodelIndex;
	// blank submodel index
	Int32 m_weaponBlankSubmodelIndex;
};
#endif //NPCSECURITY_H