/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCBREAKABLE_H
#define FUNCBREAKABLE_H

#include "delayentity.h"
#include "weapons_shared.h"

//=============================================
//
//=============================================
class CFuncBreakable : public CDelayEntity
{
public:
	struct ammo_assocation_t
	{
		weaponid_t weaponId;
		const Char* pstrAmmoName;
	};

public:
	// Spawn object types count
	static const Uint32 SPAWN_OBJECT_TYPES_COUNT = 24;
	// Object types spawned by breakable
	static const Char* SPAWN_OBJECT_TYPES[SPAWN_OBJECT_TYPES_COUNT];
	// Wood gibs model file
	static const Char GIB_MODEL_WOOD_FILENAME[];
	// Flesh gibs model file
	static const Char GIB_MODEL_FLESH_FILENAME[];
	// Computer gibs model file
	static const Char GIB_MODEL_COMPUTER_FILENAME[];
	// Glass gibs model file
	static const Char GIB_MODEL_GLASS_FILENAME[];
	// Metal gibs model file
	static const Char GIB_MODEL_METAL_FILENAME[];
	// Cinder block gibs model file
	static const Char GIB_MODEL_CINDERBLOCKS_FILENAME[];
	// Rock gibs model file
	static const Char GIB_MODEL_ROCK_FILENAME[];
	// Ceiling gibs model file
	static const Char GIB_MODEL_CEILING_FILENAME[];

	// Wood damage sounds pattern
	static const Char WOOD_DMG_SOUNDS_PATTERN[];
	// Wood break sounds pattern
	static const Char WOOD_BREAK_SOUNDS_PATTERN[];
	// Flesh sounds pattern
	static const Char FLESH_DMG_SOUNDS_PATTERN[];
	// Glass damage sounds pattern
	static const Char GLASS_DMG_SOUNDS_PATTERN[];
	// Glass break sounds pattern
	static const Char GLASS_BREAK_SOUNDS_PATTERN[];
	// Metal damage sounds pattern
	static const Char METAL_DMG_SOUNDS_PATTERN[];
	// Metal break sounds pattern
	static const Char METAL_BREAK_SOUNDS_PATTERN[];
	// Concrete damage sounds pattern
	static const Char CONCRETE_DMG_SOUNDS_PATTERN[];
	// Concrete break sounds pattern
	static const Char CONCRETE_BREAK_SOUNDS_PATTERN[];
	// Computer break sounds pattern
	static const Char COMPUTER_DMG_SOUNDS_PATTERN[];

	// Gib model lifetime
	static const Float GIB_MODEL_LIFETIME;

	// Weapon->ammo associations
	static const ammo_assocation_t WPN_AMMO_ASSOCIATIONS[NUM_WEAPONS];

public:
	enum
	{
		FL_BREAK_ON_TRIGGER_ONLY	= (1<<0),
		FL_BREAK_ON_TOUCH			= (1<<1),
		FL_BREAK_ON_PRESSURE		= (1<<2),
		FL_BREAK_CLUB				= (1<<8),
		FL_NO_GIBS					= (1<<9),
		FL_NO_PENETRATION_DAMAGE	= (1<<10),
		FL_SMART_AMMO_SPAWN			= (1<<11)
	};

	enum breakdirection_t
	{
		BREAKDIR_RANDOM = 0,
		BREAKDIR_ATTACKDIR
	};

public:
	explicit CFuncBreakable( edict_t* pedict );
	virtual ~CFuncBreakable( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual bool IsFuncBreakableEntity( void ) const override;
	virtual Int32 GetEntityFlags( void ) override { return CDelayEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
	virtual void SetBoundingBox( void ) { }

	virtual bool TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags ) override;
	virtual void TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags ) override;

	virtual Vector GetEyePosition( bool addlean = false ) const override { return GetBrushModelCenter(); };
	virtual Vector GetNavigablePosition( void ) const override { return GetBrushModelCenter(); }

public:
	void EXPORTFN BreakTouch( CBaseEntity* pOther );
	void EXPORTFN DieThink( void );

	void PlayDamageSound( void );
	void SetPrecacheObjects( void );

	Int32 GetExplosionMagnitude( void ) const;
	void SetExplosionMagnitude( Int32 magnitude );

public:
	virtual void SetSpawnProperties( void );

protected:
	Int32 m_material;
	Int32 m_breakDirection;
	Int32 m_tempentSoundFlag;

	Int32 m_breakModelIndex;
	Float m_angle;
	Int32 m_explodeMagnitude;

	string_t m_gibModelName;
	string_t m_spawnEntityName;

	string_t m_dmgSoundsPattern;
	Uint32 m_nbDmgSounds;

	string_t m_breakSoundsPattern;
	Uint32 m_nbBreakSounds;

	Uint32 m_spawnChance;

	CEntityHandle m_attacker;
};
#endif //FUNCBREAKABLE_H