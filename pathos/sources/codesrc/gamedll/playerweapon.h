/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef PLAYERWEAPON_H
#define PLAYERWEAPON_H

#include "animatingentity.h"
#include "weapons_shared.h"
#include "constants.h"

class CPlayerEntity;

struct weaponinfo_t
{
	weaponinfo_t():
		id(0),
		slot(0),
		position(0),
		maxammo(0),
		maxclip(0),
		flags(0),
		weight(0),
		cone(0),
		autoaimdegrees(0)
	{}

	Int32 id;
	Int32 slot;
	Int32 position;
	CString ammo;
	Int32 maxammo;
	CString name;
	Int32 maxclip;
	Int32 flags;
	Int32 weight;
	Int32 cone;
	Int32 autoaimdegrees;
};

struct ammoinfo_t
{
	ammoinfo_t():
		id(0)
		{}

	CString name;
	Uint32 id;
};

//=============================================
//
//=============================================
class CPlayerWeapon : public CAnimatingEntity
{
public:
	enum
	{
		FL_WEAPON_NO_NOTICE					= (1<<5),
		FL_WEAPON_TRIGGER_ON_PICKUP_ONLY	= (1<<6)
	};

public:
	explicit CPlayerWeapon( edict_t* pedict );
	virtual ~CPlayerWeapon( void );

public:
	// Performs spawn functions
	virtual bool Spawn( void ) override;
	// Declares save-restore fields
	virtual void DeclareSaveFields( void ) override;
	// Called after restoring the entity
	virtual bool Restore( void ) override;
	// Called after level load
	virtual void InitEntity( void ) override;

	// Sets whether the object should make an impact sound
	virtual void SetEnableImpactSound( bool enable ) override;
	// Tells if entity should set bounds on restore
	virtual bool ShouldSetBoundsOnRestore( void ) override { return false; }

public:
	// Adds weapon to player
	virtual void AddToPlayer( CPlayerEntity* pPlayer );
	// Adds a duplicate gun
	virtual bool AddDuplicate( CPlayerWeapon* poriginal );
	// Tells if the weapon should be removed from the world
	virtual bool ShouldRemove( CPlayerWeapon* pWeapon ) { return true; };
	// Sets spawn properties
	virtual void SetSpawnProperties( void ) = 0;

	// Extracts ammo from a gun
	bool ExtractAmmo( CPlayerWeapon* pWeapon );
	// Extracts ammo from the clip
	bool ExtractClipAmmo( CPlayerWeapon* pWeapon );

	// Sets the viewmodel weapon animation
	void SetWeaponAnimation( const Char* pstrsequence, Int64 body = NO_BODY_VALUE, Int32 skin = NO_SKIN_VALUE, bool blend = true );

	// Returns the firing cone
	Vector GetCone( void );

	// Adds ammo to player
	virtual bool AddAmmo( Int32 count, const Char* pstrname, Int32 maxclip, Int32 maxcarry, CBaseEntity* pWeapon );

public:
	// Performs deployment of a weapon
	bool DefaultDeploy( const Char* pstrviewmodel, const Char* pstrsequence, Int64 body = NO_BODY_VALUE, Int32 skin = NO_SKIN_VALUE );
	// Performs reload of a weapon
	bool DefaultReload( Int32 clipsize, const Char* pstrsequence, Int64 body = NO_BODY_VALUE, Int32 skin = NO_SKIN_VALUE, bool blendanimation = true );

public:
	// Destroys the weapon
	void EXPORTFN DestroyWeapon( void );
	// Called when touching the ground
	void EXPORTFN LandTouch( CBaseEntity* pOther );
	// Manages touching by a player
	void EXPORTFN DefaultTouch( CBaseEntity* pOther );

public:
	// Tells if the weapon can deploy
	virtual bool CanDeploy( void );
	// Manages deployment of the entity
	virtual bool Deploy( void ) = 0;
	// Tells if the gun has a silencer
	virtual bool HasSilencer( void ) { return false; }
	// Sets whether the gun has a silencer
	virtual void SetHasSilencer( bool hasSilencer ) { }
	// Tells if the gun has a flashlight
	virtual bool HasFlashlight( void ) { return false; }
	// Sets whether the gun has a flashlight
	virtual void SetHasFlashlight( bool hasFlashlight ) { }
	// Tells if the weapon can holster
	virtual bool CanHolster( void ) { return true; }
	// Retires the weapon
	virtual void Retire( void );
	// Holsters the weapon
	virtual void Holster( void );
	// Flags weapon to be dropped
	virtual void FlagDrop( void ) { m_dropWeapon = true; }
	// Returns weapon's next think time
	virtual Double GetWeaponNextThinkTime( void ) const { return m_nextThinkTime; }
	// Tells if the entity is a weapon
	virtual bool IsWeapon( void ) const override { return true; }
	// Tries to add accessories from pWeapon
	virtual void AddAccessories( CPlayerWeapon* pWeapon ) override { };

public:
	// Performs post-think functions
	virtual void PostThink( void );

	// Attaches weapon to a player
	void AttachToPlayer( CBaseEntity* pPlayer );
	// Sets glow on weapon entity
	void Glow( void );
	// Drops the weapon
	void DropWeapon( bool destroy = false );
	// Makes the weapon materialize
	void EnablePlayerTouch( void );
	// Cancels any reload in progress
	void CancelReload( void );

	// Sets the entity to fall to the ground
	void StartFalling( void );

public:
	// Performs primary attack
	virtual void PrimaryAttack( void ) = 0;
	// Performs secondary attack
	virtual void SecondaryAttack( void ) {}
	// Performs weapon special function
	virtual void WeaponSpecialFunction( void ) {}
	// Reloads the weapon
	virtual void Reload( void ) {}
	// Sets idle animation
	virtual void Idle( void ) = 0;

	// Finishes reload
	virtual void FinishReload( void );
	
public:
	// Tells if weapon is usable
	bool IsUsable( void );
	// Tells if weapon is reloading
	bool IsReloading( void ) const { return m_inReload; }
	// Tells if weapon is zoomed
	virtual bool IsZoomed( void ) const { return false; }
	// Sets zoom on weapon
	virtual void SetZoom( bool iszooming, CPlayerEntity* pPlayer ) {}
	// Toggles tactical flashlight
	virtual void FlashlightToggle( bool drain = false ) {}
	// Tells that player requested a flashlight turn-on
	virtual void SetFlashlightRequest( bool requested ) {}
	// Tells if attack time can be reset
	virtual bool CanResetAttackTime( void ) { return true; }
	// Tells if secondary attack press should be ignored
	virtual bool IgnoreSecondaryAttack( void ) { return false; }
	// Tells if we should force player to move slowly
	virtual bool ShouldPlayerMoveSlowly( void ) { return false; }
	// Tells if we should hide the HUD
	virtual bool ShouldHideHUD( void ) { return false; }

	// Returns the ammo index
	virtual Int32 GetAmmoIndex( void ) { return m_ammoType; }
	
	// Updates client data
	bool UpdateClientData( CPlayerEntity* pPlayer );
	// Adds ammo from dual weapon
	virtual bool AddFullAmmoDual( CPlayerWeapon* pcheckweapon ) { return false; }
	
	// Gets nb of bullets in clip
	Int32 GetClip( void ) const;
	// Gets nb of bullets in left clip
	Int32 GetLeftClip( void ) const;
	// Gets nb of bullets in right clip
	Int32 GetRightClip( void ) const;

	// Sets nb of bullets in clip
	void SetClip( Int32 clip );
	// Sets nb of bullets in right clip
	void SetRightClip( Int32 clip );
	// Sets nb of bullets in left clip
	void SetLeftClip( Int32 clip );

public:
	// Returns the weapon id
	Int32 GetId( void ) const;
	// Returns the ammo type name
	const Char* GetAmmoTypeName( void );
	// Returns the max ammo for the weapon
	Int32 GetMaxAmmo( void );
	// Returns the weapon name
	const Char* GetWeaponName( void );
	// Returns the max clip capacity
	virtual Int32 GetMaxClip( void );
	// Returns the weapon's weight
	Int32 GetWeight( void );
	// Returns the weapon flags
	Int32 GetWeaponFlags( void );
	// Returns the cone used
	virtual Uint32 GetConeIndex( void );
	// Returns the HUD position
	Int32 GetHUDPosition( void );
	// Returns the HUD slot
	Int32 GetHUDSlot( void );
	// Tells if the weapon can reload
	virtual bool CanReload( void );

	// Tells if the dual weapon has both weapons
	bool HasDual( void ) const { return m_hasDual; }
	// Sets duplicate flag
	void SetDuplicate( bool isduplicate ) { m_isDuplicate = isduplicate; }
	// Returns the default ammo
	Int32 GetDefaultAmmo( void ) const { return m_defaultAmmo; }

public:
	// Degrades the recoil
	void DegradeRecoil( void );
	// Adds to the recoil
	void AddRecoil( Float recoil );
	// Sets the view model's bodygroup
	void SetViewModelBodyGroup( Int32 group, Int32 value );

	// Returns the recoil degradation speed
	virtual Float GetRecoilDegradeFactor( void ) { return DEFAULT_RECOIL_DEGRADE; }
	// Returns the recoil max limit
	virtual Float GetRecoilLimit( void ) { return DEFAULT_RECOIL_LIMIT; }
	// Returns the autoaim degrees
	virtual Float GetAutoAimDegrees( void );

	// Returns next weapon in linked list
	CPlayerWeapon* GetNextWeapon( void );
	// Sets the next weapon ptr
	void SetNextWeapon( CPlayerWeapon* pWeapon );
	// Returns the player pointer
	CPlayerEntity* GetPlayer( void ) { return m_pPlayer; }

public:
	// Called from weapon constructor
	static void RegisterWeapon( CBaseEntity* pWeapon );
	// Called from weapon constructor
	static void RegisterAmmoType( const Char* pstrAmmoTypeName );
	// Clears info arrays
	static void ClearWeaponInfos( void );
	// Retrieves ammo index for an ammo type
	static Int32 GetAmmoTypeIndex( const Char* pstrAmmoTypeName );
	// Returns a weapon definition for an id
	static weaponinfo_t& GetWeaponInfo( weaponid_t weaponid );

protected:
	// Array of weapon infos
	static weaponinfo_t WEAPON_INFO_LIST[MAX_WEAPONS];
	// Array of ammo types
	static ammoinfo_t AMMO_INFO_LIST[MAX_AMMO_TYPES];

protected:
	// Owner player
	CPlayerEntity* m_pPlayer;
	// Next weapon
	CPlayerWeapon* m_pNext;

	// Weapon Id
	weaponid_t m_weaponId;

	// True if flagged for dropping
	bool	m_dropWeapon;
	// True if this is a duplicate being picked up
	bool	m_isDuplicate;
	// True if dual wielding is available
	bool	m_hasDual;
	// True if this is the first time we draw the weapon
	bool	m_firstDraw;

protected:
	// Next time until weapon can think
	Double	m_nextThinkTime;
	// Next time we can use any attacks
	Double	m_nextAttackTime;
	// Next time we can idle
	Double	m_nextIdleTime;
	// Reload duration
	Double	m_reloadTime;
	// Reload delay time
	Double	m_reloadDisabledTime;

	// Ammo type
	Int32	m_ammoType;
	// Clip capacity
	Int32	m_clip;
	// Clip on client side
	Int32	m_clientClip;
	// Right clip capacity
	Int32	m_rightClip;
	// Right clip on client side
	Int32	m_clientRightClip;
	// Right clip capacity
	Int32	m_leftClip;
	// Right clip on client side
	Int32	m_clientLeftClip;
	// Weapon state on client
	Int32	m_clientWeaponState;
	// Body value
	Int64	m_viewModelBody;
	// Skin value
	Int32	m_viewModelSkin;

	// True if we're retiring
	bool	m_isRetired;
	// True if we're forced to retire
	bool	m_isForcedToRetire;
	// True if we've deployed
	bool	m_isDeployed;
	// True if we're in reload
	bool	m_inReload;
	// True if in dual mode
	bool	m_inDual;
	// True if weapon should make an impact sound
	bool	m_makeImpactSound;

	// Ammo the player gets when picking up the gun
	Int32	m_defaultAmmo;
	// Recoil multiplier
	Float	m_recoilMultiplier;
	// Last attack button pressed
	Int32 m_lastAttackButtonPressed;
};

extern void Weapon_Precache( const Char* pstrClassname );
#endif //PLAYERWEAPON_H