/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef NPCMAKER_H
#define NPCMAKER_H

#include "delayentity.h"

//=============================================
//
//=============================================
class CNPCMaker : public CDelayEntity
{
public:
	// Proximity check boundary size
	static const Vector BOUNDARY_CHECK_SIZE;
	// Max number of classnames for weapons
	static const Uint32 MAX_CLASSNAMES = 32;

public:
	enum
	{
		FL_START_ON	= (1<<0),
		FL_CYCLIC	= (1<<2),
		FL_NPC_CLIP	= (1<<3),
		FL_GAG		= (1<<4),
		FL_IDLE		= (1<<5)
	};
public:
	explicit CNPCMaker( edict_t* pedict );
	virtual ~CNPCMaker( void );

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Performs precache functions
	virtual void Precache( void ) override;
	// Manages a keyvalue
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	// Calls for classes and their children
	virtual void DeclareSaveFields( void ) override;
	// Death notice from child entities
	virtual void ChildDeathNotice( CBaseEntity* pChild ) override;
	// Initializes the entity after map has done loading
	virtual void InitEntity( void ) override;
	// Returns entity's flags
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }

public:
	// Called when npcmaker is toggled
	void EXPORTFN ToggleUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value );
	// Called when npcmaker is cyclic
	void EXPORTFN CyclicUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value );
	// Think function for spawning npcs
	void EXPORTFN MakerThink( void );
	// Called to fire(duh)(I need better comments)
	void EXPORTFN FireThink( void );

public:
	// Creates the NPC
	void CreateNPC( bool isCyclic );

private:
	// Classname of NPC to be spawned
	string_t m_npcClassName;
	// Number of NPCs this can create
	Int32 m_numNPCsToCreate;
	// Max live npcs
	Int32 m_maxLiveChildren;

	// Number of live children
	Uint32 m_numLiveChildren;
	// Ground coords under this entity
	Vector m_groundPosition;

	// Target to trigger on child's death
	string_t m_deathTriggerTarget;

	// Classname specified for weapon
	string_t m_weaponSettingClassnames[MAX_CLASSNAMES];
	// Weapon value on classname
	Int32 m_classnameWeaponSettings[MAX_CLASSNAMES];
	// Number of weapon classnames
	Uint32 m_numClassnameWeaponSettings;

	// Classname specified for head
	string_t m_headSettingClassnames[MAX_CLASSNAMES];
	// Head value on classname
	Int32 m_classnameHeadSettings[MAX_CLASSNAMES];
	// Number of head classnames
	Uint32 m_numClassnameHeadSettings;

	// TRUE if active
	bool m_isActive;
	// TRUE if children should fade
	bool m_fadeChildren;
};
#endif //NPCMAKER_H