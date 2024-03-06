/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef PLAYER_H
#define PLAYER_H

#include "baseentity.h"
#include "stepsound.h"
#include "weapons_shared.h"
#include "ehandle.h"
#include "gameui_shared.h"

class CTriggerSubwayController;
class CTriggerKeypad;
class CTriggerLogin;
class CPlayerWeapon;
class CEnvLadder;
class CItemMotorBike;
class CTriggerCameraModel;

enum subwayline_t;

extern void ClientPreThink( edict_t* pclient );
extern void ClientPostThink( edict_t* pclient );

extern bool ClientCommand( edict_t* pclient );
extern bool ClientConnect( edict_t* pclient, const Char* pname, const Char* paddress, CString& rejectReason );
extern void ClientDisconnected( edict_t* pclient );

extern void CmdStart( const usercmd_t& cmd, edict_t* pclient );
extern void CmdEnd( edict_t* pclient );

extern void PM_PlayStepSound( entindex_t entindex, const Char* pstrMaterialName, bool stepleft, Float volume, const Vector& origin );
extern void PM_PlaySound( entindex_t entindex, Int32 channel, const Char* psample, Float volume, Float attenuation, Int32 pitch, Int32 flags );

// Maximum saved keypad id/keypad code mappings
static const Uint32 MAX_SAVED_PASSCODES = 128;

/*
=======================
CPlayerEntity

=======================
*/
class CPlayerEntity : public CBaseEntity
{
public:
	enum timebaseddmgtypes_t
	{
		TBD_RADIATION = 0,
		TBD_DROWN_RECOVER,
		TBD_ACID,
		TBD_BURN,
		TBD_FREEZE,

		NUM_TIMEBASED_DMG
	};
	struct cheatinfo_t
	{
		cheatinfo_t(const Char* pstrCheatCode, const Char* pstrDescription):
			cheatcode(pstrCheatCode),
			description(pstrDescription)
			{
			}
		cheatinfo_t(Int32 impulsecode, const Char* pstrDescription):
			description(pstrDescription)
			{
				cheatcode << "impulse " << impulsecode;
			}

		CString cheatcode;
		CString description;
	};
	struct npc_awarenessinfo_t
	{
		npc_awarenessinfo_t():
			awareness(0),
			lasttime(0),
			timeoutdelay(0)
			{}

		CEntityHandle pnpc;
		Float awareness;
		Double lasttime;
		Float timeoutdelay;
	};

	struct music_data_t
	{
		music_data_t():
			filename(NO_STRING_VALUE),
			duration(0),
			begintime(0),
			flags(0),
			channel(0)
		{}

		// Current OGG file being played
		string_t filename;
		// Duration of OGG file
		Float duration;
		// Time when we began playing music
		Double begintime;
		// Music playback flags
		Int32 flags;
		// Music channel
		Int32 channel;
	};

public:
	// Player punch treshold for falling
	static const Float PLAYER_FALL_VELOCITY_PUNCH_MIN;
	// Player punch treshold for falling
	static const Float PLAYER_FALL_VELOCITY_SAFE_LIMIT;
	// Player punch treshold for falling
	static const Float PLAYER_FALL_VELOCITY_FATAL;
	// Player punch treshold for falling
	static const Float DAMAGE_FALL_FOR_VELOCITY;
	// Player falling velocity limit for producing audible sounds to NPCs
	static const Float PLAYER_FALL_SOUND_LIMIT;
	// Player heal time
	static const Double PLAYER_HEAL_TIME;
	// Player armor damage absorption
	static const Float PLAYER_ARMOR_DMG_ABSORB;
	// Player armor damage drain
	static const Float PLAYER_ARMOR_DMG_DRAIN;
	// Time based damage delay
	static const Float DROWNRECOVER_HEAL_DELAY;
	// Time based damage delay
	static const Float DROWNRECOVER_MAX_HEAL;
	// Time based damage delay
	static const Float TIMEBASED_DMG_DELAY;
	// Time based dmg type-dmg bit associations
	static const Int32 TIMEBASED_DMG_BITS[NUM_TIMEBASED_DMG];
	// Time based dmg type-dmg durations
	static const Float TIMEBASED_DMG_DURATIONS[NUM_TIMEBASED_DMG];
	// Time based dmg type-damage values
	static const Float TIMEBASED_DMG_AMOUNTS[NUM_TIMEBASED_DMG];
	// Color of screen fade when hurt
	static const color24_t PAIN_SCREENFADE_COLOR;
	// Player use radius
	static const Float PLAYER_USE_RADIUS;
	// Flashlight toggle sound
	static const Char FLASHLIGHT_TOGGLE_SOUND[];
	// Flashlight drain time
	static const Float FLASHLIGHT_DRAIN_TIME;
	// Flashlight drain time
	static const Float FLASHLIGHT_CHARGE_TIME;
	// Flashlight impulse value
	static const Int32 FLASHLIGHT_IMPULSE_VALUE;
	// Player give weapons cheat code
	static const Int32 PLAYER_CHEATCODE_ALLWEAPONS;
	// Texture name tracing impulse code
	static const Int32 PLAYER_CHEATCODE_TRACE_TEXTURE;
	// Report AI state impulse code
	static const Int32 PLAYER_CHEATCODE_REPORT_AI_STATE;
	// Dump all globals cheat code
	static const Int32 PLAYER_CHEATCODE_DUMPGLOBALS;
	// Dump all codes cheat code
	static const Int32 PLAYER_CHEATCODE_DUMP_CODES;
	// Get nearest node's index
	static const Int32 PLAYER_CHEATCODE_GET_NEAREST_NODE_INDEX;
	// Show small node mins/maxs for nearest node
	static const Int32 PLAYER_CHEATCODE_SHOW_SMALL_HULL_NEAREST_NODE_MINS_MAXS;
	// Show fly hull node mins/maxs for nearest node
	static const Int32 PLAYER_CHEATCODE_SHOW_FLY_HULL_NEAREST_NODE_MINS_MAXS;
	// Show large hull node mins/maxs for nearest node
	static const Int32 PLAYER_CHEATCODE_SHOW_LARGE_HULL_NEAREST_NODE_MINS_MAXS;
	// Show human hull node mins/maxs for nearest node
	static const Int32 PLAYER_CHEATCODE_SHOW_HUMAN_HULL_NEAREST_NODE_MINS_MAXS;
	// Show small node mins/maxs
	static const Int32 PLAYER_CHEATCODE_SHOW_SMALL_HULL_NODE_MINS_MAXS;
	// Show fly hull node mins/maxs
	static const Int32 PLAYER_CHEATCODE_SHOW_FLY_HULL_NODE_MINS_MAXS;
	// Show large hull node mins/maxs
	static const Int32 PLAYER_CHEATCODE_SHOW_LARGE_HULL_NODE_MINS_MAXS;
	// Show human hull node mins/maxs
	static const Int32 PLAYER_CHEATCODE_SHOW_HUMAN_HULL_NODE_MINS_MAXS;
	// Show small node paths
	static const Int32 PLAYER_CHEATCODE_SHOW_SMALL_HULL_NODE_PATHS;
	// Show fly hull node paths
	static const Int32 PLAYER_CHEATCODE_SHOW_FLY_HULL_NODE_PATHS;
	// Show large hull node paths
	static const Int32 PLAYER_CHEATCODE_SHOW_LARGE_HULL_NODE_PATHS;
	// Show human hull node paths
	static const Int32 PLAYER_CHEATCODE_SHOW_HUMAN_HULL_NODE_PATHS;
	// Show all node paths
	static const Int32 PLAYER_CHEATCODE_SHOW_ALL_NODE_PATHS;
	// Show nearest node connections
	static const Int32 PLAYER_CHEATCODE_SHOW_NEAREST_NODE_CONNECTIONS;
	// Dump all codes cheat code
	static const Int32 PLAYER_CHEATCODE_REMOVE_ENTITY;
	// Grant all subway stops
	static const Int32 PLAYER_CHEATCODE_GRANT_ALL_SUBWAY_STOPS;
	// Name entity in front
	static const Int32 PLAYER_CHEATCODE_NAME_ENTITY_IN_FRONT;
	// Cheat codes and their descriptions
	static const cheatinfo_t PLAYER_CHEAT_DESCRIPTIONS[];
	// Player drown delay
	static const Float PLAYER_DROWN_DELAY_TIME;
	// Player drown damage ramp-up
	static const Float PLAYER_DROWN_RAMP_AMOUNT;
	// Player drown damage ramp-up limit
	static const Float PLAYER_DROWN_RAMP_LIMIT;
	// Player flashlight drain speed
	static const Float PLAYER_FLASHLIGHT_DRAIN_TIME;
	// Player shoulder flashlight drain speed
	static const Float PLAYER_FLASHLIGHT_DRAIN_TIME_SHOULDERLIGHT;
	// Player flashlight drain speed
	static const Float PLAYER_FLASHLIGHT_CHARGE_TIME;
	// Max kevlar a player can carry
	static const Float MAX_PLAYER_KEVLAR;
	// Player awareness field of view
	static const Float PLAYER_NPC_AWARENESS_FIELD_OF_VIEW;
	// Max player sound radius
	static const Float PLAYER_SOUND_MAX_RADIUS;
	// Sound radius decay speed
	static const Float PLAYER_SND_RADIUS_DECAY_SPEED;
	// Weapon sound radius decay speed
	static const Float PLAYER_WEAPON_SND_RADIUS_DECAY_SPEED;
	// Weapon flash decay speed
	static const Float PLAYER_WEAPON_FLASH_DECAY_SPEED;

public:
	explicit CPlayerEntity( edict_t* pedict );
	virtual ~CPlayerEntity();

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Precaches resources
	virtual void Precache( void ) override;
	// Called after save-restore
	virtual bool Restore( void ) override;

	// Declares save fields
	virtual void DeclareSaveFields( void ) override;
	// Tells if the entity is a player
	virtual bool IsPlayer( void ) const override { return true; }

	// Adds to the player's health
	virtual bool TakeHealth( Float amount, Int32 damageFlags ) override;
	// Makes the entity take on damage
	virtual bool TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags ) override;
	// Handles damage calculation for a hitscan
	virtual void TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags ) override;
	// Manages dying
	virtual void Killed( CBaseEntity* pAttacker, gibbing_t gibbing, deathmode_t deathMode = DEATH_NORMAL ) override;
	// Returns blood color setting
	virtual bloodcolor_t GetBloodColor( void ) override;

	// Returns the entity's targeting origin
	virtual Vector GetBodyTarget( const Vector& targetingPosition ) override;

	// Tells if the entity is alive
	virtual bool IsAlive( void ) const override;
	// Tells if the player is on a motorbike
	virtual bool IsOnMotorBike( void ) const override;
	// Returns the classification
	virtual Int32 GetClassification( void ) const override;
	// Returns relation with other entity
	virtual Int32 GetRelationship( CBaseEntity* pOther ) override;

	// Returns the player's entity index
	virtual Int32 GetClientIndex( void ) const override { return m_pEdict->clientindex; }

	// Returns the view position
	virtual Vector GetEyePosition( bool addlean = false ) const override;
	// Returns a navigable position
	virtual Vector GetNavigablePosition( void ) const override;
	// Returns the view origin for VIS checks
	virtual Vector GetVISEyePosition( void ) const override;
	// Returns the view origin for VIS checks
	virtual bool GetAdditionalVISPosition( Vector& outVector ) const override;
	// Tells if an entity is in the view cone
	virtual bool IsInView( CBaseEntity* pOther ) const override;
	// Tells if an entity is in the view cone
	virtual bool IsInView( const Vector& position ) const override;

	// Sets the falling velocity
	virtual void SetFallingVelocity( Float velocity ) override;

	// Gives ammo to the player
	virtual Int32 GiveAmmo( Int32 amount, const Char* pstrammoname, Int32 max, bool display, CBaseEntity* pentity ) override;
	// Manages textmessage and textmessage_team functions
	virtual void HostSay( const Char* pstrText, bool teamonly ) override;
	// Drops active weapon
	virtual void DropCurrentWeapon( void ) override;
	// Gives an item by clasname
	virtual void GiveItemByName( const Char* pstrClassname, Uint32 amount, bool removeunneeded ) override;
	// Selects the last weapon used
	virtual void SelectPreviousWeapon( void ) override;

	// Called when using a ladder
	virtual void EnterLadder( CEnvLadder *pLadder ) override;
	// Sets the room type
	virtual void SetRoomType( Int32 roomtype ) override;
	// Sets nightstage state
	virtual void SetSpecialFog( bool specialfogenabled ) override;

	// Get day stage
	virtual daystage_t GetDayStage( void ) const override;
	// Sets nightstage state
	virtual void SetDayStage( daystage_t daystage ) override;

	// Sets dialouge duration for player
	virtual void SetDialougeDuration( Float duration ) override;

	// Adds a medkit to the player
	virtual bool AddMedkit( const Char* pstrClassname, bool noNotice ) override;
	// Adds a medkit to the player
	virtual bool AddKevlar( const Char* pstrClassname, bool noNotice ) override;
	// Adds a shoulder light to the player
	virtual bool AddShoulderLight( const Char* pstrClassname, bool noNotice ) override;

	// When player gets on the bike
	virtual void EnterBike( CItemMotorBike *pEntity ) override;

	// Adds a new passcode
	virtual void AddPasscode( const Char* pstrid, const Char* pstrpasscode ) override;

	// Tells if forced holstering is set
	virtual bool IsForceHolsterSet( void ) const override { return m_forceHolster; }
	// Sets force holster state
	virtual void SetForceHolster( bool forceholster ) override;
	// Sets forced slow movement
	virtual void SetForceSlowMove( bool forceslowmove, bool nosprinting ) override;
	// Sets dream state
	virtual void SetIsInDream( bool isindream ) override;

	// Spawns a text window
	virtual void SpawnTextWindow( const Char* pstrfilepath, const Char* pstrpasscode, const Char* pstrid ) override;
	// Spawns a keypad window
	virtual void SpawnKeypadWindow( const Char* pstrid, const Char* pstrkeypadcode, CTriggerKeypad* pkeypad, bool staytillnext ) override;
	// Spawns a subway window
	virtual void SpawnSubwayWindow( subwayline_t type, CTriggerSubwayController* pcontroller, bool isdummy ) override;
	// Spawns a login window
	virtual void SpawnLoginWindow( const Char* pstruser, const Char* pstrpasscode, const Char* pstrid, CTriggerLogin* plogin, bool staytillnext ) override;
	// Sets subway flag
	virtual void SetSubwayFlag( Int32 flag ) override;

	// Selects a named weapon
	virtual void SelectWeapon( const Char* pstrWeaponName ) override;
	// Selects a weapon by weapon Id
	virtual void SelectWeapon( weaponid_t weaponId ) override;
	// Returns the active weapon
	virtual CPlayerWeapon* GetActiveWeapon( void ) override;
	// Returns the weapon list pointer
	virtual CPlayerWeapon* GetWeaponList( void ) override;
	// Returns ammo count for a type
	virtual Int32 GetAmmoCount( Int32 ammotype ) const override;
	// Sets ammo count for a type
	virtual void SetAmmoCount( Int32 ammotype, Int32 ammocount ) override;
	// Adds item to player
	virtual bool AddPlayerWeapon( CPlayerWeapon* pWeapon ) override;

	// Sets the save-game title
	virtual void SetSaveGameTitle( const Char* pstrtitle ) override;
	// Returns the save-game title
	virtual const Char* GetSaveGameTitle( void ) override;

	// Sets the view entity
	virtual void SetViewEntity( CBaseEntity* pEntity ) override;
	// Sets control enabled state
	virtual void SetControlEnable( bool enable ) override;
	// Set player paralysis bitflag
	virtual void SetPlayerParalyzed( bool enable ) override;
	// Tells whether player is paralyzed
	virtual bool GetIsPlayerParalyzed( void ) override;
	// Set camera entity
	virtual void SetCameraEntity( CTriggerCameraModel* pCamera ) override;

	// Adds full ammo for dual-wielded weapons
	virtual bool AddFullAmmoDual( CPlayerWeapon* pWeapon ) const override;
	// Tells if the player can have an ammo type
	virtual bool CanHaveAmmo( const Char* pstrammotype, Int32 maxammo ) const override;
	// Tells if the player can have a weapon
	virtual bool CanHaveWeapon( CPlayerWeapon* pWeapon ) const override;

	// TRUE if a black hole can pull this entity
	virtual bool CanBlackHolePull( void ) const override;

public:
	// Gets the entity's illumination
	virtual Int32 GetIllumination( void ) override;
	// Plays a music track for a player
	virtual void PlayMusic( const Char* pstrFilename, Int32 channel, Float fadeInTime, Int32 flags ) override;
	// Stops any playing music tracks
	virtual void StopMusic( const Char* pstrFilename, Int32 channel, Float fadeTime ) override;
	// Tells if talking npc can answer
	virtual bool CanAnswer( void ) override { return true; };
	// Tells if entity should set bounds on restore
	virtual bool ShouldSetBoundsOnRestore( void ) override { return false; }
	// Unducks the player
	virtual void UnDuckPlayer( void ) override;

public:
	// Uses a medkit
	void UseMedkit( void );
	// Performs think functions while heal is in progress
	void HealThink( void );

	// Player use think function
	void PlayerUseThink( void );
	// Called when player presses "use"
	void PlayerUse( void );
	// Called when jumping
	void Jump( void );
	// Manages movement while in water
	void WaterMove( void );
	// Updates flashlight
	void UpdateFlashlight( void );

	// Tells if player is on ladder
	bool IsOnLadder( void ) const;

public:
	// Plays death sounds
	void PlayDeathSound( void );
	// Plays a pain sound
	void PlayPainSound( void );

public:
	// Performs think functions before usercmd is run
	void PreCmdThink( void );
	// Performs think functions after usercmd is run
	void PostCmdThink( void );

	// Plays a footstep sound
	void PlayStepSound( const Char* pstrMaterialName, bool stepleft, Float volume, const Vector& origin );
	// Plays a player sound
	void PlaySound( Int32 channel, const Char* psample, Float volume, Float attenuation, Int32 pitch, Int32 flags );

	// Manages a game UI event
	void ManageGameUIEvent( class CMSGReader& reader );

	// Updates client data
	void UpdateClientData( void );

public:
	// Returns the passcode for an id
	const Char* GetPasscodeForId( const Char* pstrid );
	// Dumps all codes saved by player
	void DumpAllCodes( void );

	// Destroys any active windows
	void DestroyGameUIWindows( void );

public:
	// Removes ammo for a given ammo type
	void RemoveAmmo( Int32 ammotype, Uint32 numremove );
	// Updates ammo counts on client
	void UpdateClientAmmoCounts( void );

	// Removes a player weapon
	bool RemovePlayerWeapon( CPlayerWeapon* pWeapon );

	// Returns the next best weapon
	CPlayerWeapon* GetNextBestWeapon( CPlayerWeapon* pWeapon );
	// Tells if the weapon should holster
	bool ShouldHolster( void ) const;
	// Removes a weapon
	void RemoveWeapon( CPlayerWeapon* pWeapon );
	// Removes all weapons
	void RemoveAllWeapons( void );
	// Tells if player can pick up a weapon
	bool CanPickupWeapon( Int32 slot, enum weaponid_t weaponid );
	// Tells if player should auto-switch to this weapon
	bool ShouldSwitchToWeapon( CPlayerWeapon* pWeapon );
	// Switches to the specified weapon
	bool SwitchToWeapon( CPlayerWeapon* pWeapon );
	// Clears current weapon
	void ClearCurrentWeapon( void );

	// Returns the next weapon to be deployed
	CPlayerWeapon* GetNextWeapon( void );
	// Returns active weapon on client
	CPlayerWeapon* GetClientActiveWeapon( void );

	// Tells if the player has any weapons
	bool HasAnyWeapons( void ) const;
	// Sets a weapon bit
	void SetWeaponBit( Int64 weaponid );
	// Removes a weapon bit
	void RemoveWeaponBit( Int64 weaponid );
	// Checks if the player has the named weapon
	bool HasPlayerWeapon( const Char* pstrWeaponClassName ) const;
	// Returns the drop position for weapons
	Vector GetWeaponDropPosition( void );

	// Sets weapon volume
	void SetWeaponVolume( Uint32 volume );
	// Sets weapon flash brightness
	void SetWeaponFlashBrightness( Uint32 brightness );

	// Tells if the a weapon update needs to be forced
	bool ForceWeaponUpdate( void ) const;
	// Sets forced weapon update flag
	void SetForceWeaponUpdate( bool update );

	// Returns cone index on client
	Uint32 GetClientConeIndex( void ) const;
	// Sets cone index on client
	void SetClientConeIndex( Uint32 coneIndex );

	// Updates player sounds
	void UpdatePlayerSound( void );

public:
	// Applies view punch force on axis
	void ApplyAxisPunch( Int32 axis, Float value );
	// Sets direct view punch on axis
	void ApplyDirectAxisPunch( Int32 axis, Float value );
	// Sets view model
	void SetViewModel( const Char* pstrviewmodel );
	// Sets the client view model
	void SetClientViewModel( const Char* pstrviewmodel );

public:
	// Sets field of view
	void SetFOV( Int32 value );
	// Returns field of view value
	Int32 GetFOV( void ) const;
	// Returns field of iew on client
	Int32 GetClientFOV( void ) const;

	// Calculates leaning
	void LeanThink( void );
	// Returns lean offset
	Vector GetLeanOffset( Int32 buttons ) const;

	// Returns lean angle
	Vector GetLeanAngle( void ) const;
	// Returns lean offset
	Vector GetLeanOffset( void ) const;
	// Returns gun position
	Vector GetGunPosition( void ) const;
	// Returns gun position
	Vector GetGunAngles( bool addPunch = true ) const;

public:
	// Tells if flashlight is on
	bool IsFlashlightOn( bool onlyDimLight = false, bool onlyShoulderLight = false ) const;
	// Turns the flashlight off
	bool TurnFlashlightOff( bool onlyDimLight = false, bool onlyShoulderLight = false );
	// Turns the flashlight on
	bool TurnFlashlightOn( bool isShoulderLight = false );

public:
	// Called when preparing to demount a ladder
	void InitLeaveLadder( enum ladder_verify_codes_t exitcode );
	// Clears ladder related variables
	void ClearLadder( void );
	// Performs think functions while on ladder
	void LadderThink( void );

public:
	// Set NPC awareness
	virtual void SetNPCAwareness( Float awareness, CBaseEntity* pNPC, Float timeoutDelay ) override;
	// Think function for NPC awareness
	void NPCAwarenessThink( void );

public:
	// Dumps cheats and their descriptions
	static void DumpCheatCodes( void );

private:
	// Initializes step sounds
	void InitStepSounds( void );
	// Manages flashlight
	void ManageFlashlight( void );

	// Autoaim think function
	void AutoAimThink( void );
	// Tells if we should use autoaim
	bool ShouldUseAutoAim( void ) const;

private:
	// Think function to perform while dead
	void EXPORTFN DeadThink( void );

	// Updates time based damages
	void UpdateTimeBasedDamages( void );
	// Manages sprinting
	void SprintThink( void );
	// Manages pre-cmd weapon thinking
	void WeaponPreFrameThink( void );
	// Manages post-cmd weapon thinking
	void WeaponPostFrameThink( void );

	// Manages impulse commands
	void ManageImpulseCommands( void );
	// Manages cheat impulse commands
	void ManageCheatImpulseCommands( Int32 impulse );

public:
	// Manages when touching an entity with the motorbike
	void BikeTouch( CBaseEntity *pOther );
	// Makes player exit the bike
	void LeaveBike( void );

private:
	// Finds a drop spot near the bike
	bool FindBikeDropSpot( Vector& angles, Vector& origin );
	// Performs think functions while on bike
	void BikeThink( void );

public:
	// Begins playback of a tape track
	virtual void PlaybackTapeTrack( const Char* pstrTrackFilename, Float duration, const Char* pstrPlaybackTitle, const Vector& titleColor, Float titleAlpha ) override;
	// Stops playback of a tape track
	virtual void StopTapeTrack( const Char* pstrTrackFilename ) override;
	// Performs think functions for tape track playback
	void TapePlaybackThink( void );

public:
	// Begins playback of a diary
	virtual void BeginDiaryPlayback( const Char* pstrFilename, Float duration, CBaseEntity* pDiaryEntity ) override;
	// Performs think functions while playing diary track
	void DiaryPlaybackThink( void );

public:
	// Adds a new mission objective
	virtual void AddMissionObjective( const Char* pstrObjectiveIdentifier, bool notify ) override;
	// Removes a new mission objective
	virtual void RemoveMissionObjective( const Char* pstrObjectiveIdentifier, bool notify ) override;

	// Spawns an objectives window
	virtual void SpawnObjectivesWindow( void ) override;

public:
	// Sets countdown timer
	virtual void SetCountdownTimer( Float duration, const Char* pstrTitle ) override;
	// Clears countdown timer
	virtual void ClearCountdownTimer( void ) override;

public:
	// Sets a global delayed trigger
	virtual void SetGlobalDelayedTrigger( Float delay, const Char* pstrTargetName ) override;
	// Clears a global delayed trigger
	virtual void ClearGlobalDelayedTrigger( void ) override;
	// Think function for the global trigger
	void DelayedGlobalTriggerThink( void );

public:
	// Tells if a cheat command is active
	static bool IsUsingCheatCommand( void );

private:
	// Step sound class
	CStepSound m_pstepSound;

private:
	// Camera entity controlling player
	CBaseEntity*				m_pCameraEntity;

	// Day stage setting
	Int32						m_dayStage;
	// Nightstage state on client
	Int32						m_clientDayStageState;
	// TRUE if specialfog is set
	bool						m_specialFogEnabled;
	// Specialfog state on client
	bool						m_clientSpecialFogEnabled;

	// Last light origin used
	Vector						m_lastLightOrigin;
	// Last illumination value
	Int32						m_lastIllumination;

	// Current trigger_camera entity
	CBaseEntity*				m_pTriggerCameraEntity;

private:
	// Login window entity
	CTriggerLogin*				m_pLoginEntity;
	// Keypad entity
	CTriggerKeypad*				m_pKeypadEntity;
	// Subway controller entity
	CTriggerSubwayController*	m_pSubwayController;
	// Subway flags
	Int32						m_subwayFlags;

	// Tells if we have an active UI window
	bool						m_hasActiveUIWindows;

	// Keypad id-> keypad code mappings
	string_t					m_savedPasscodes[MAX_SAVED_PASSCODES];
	// Number of saved codes
	Uint32						m_numSavedPasscodes;

private:
	// True if force holster is enabled
	bool						m_forceHolster;
	// True if we're in a dream sequence
	bool						m_isInDreamSequence;
	// True if we're forced to move slowly
	bool						m_forceSlowMove;
	// True if we walked slowly before slow move
	bool						m_walkedSlowBeforeSlowMove;
	// True if we walked slowly before weapon asked to slow down
	bool						m_walkedSlowBeforeWeaponSlowdown;
	// Previous weapon slowdown state
	bool						m_previousWeaponSlowdownState;

	// Save game title in use
	string_t					m_saveGameTitle;
	// Current room type
	Int32						m_roomType;
	// Room type on client
	Int32						m_clientRoomType;

	// FOV setting
	Int32						m_fov;
	// FOV on client
	Int32						m_clientFOV;

	// Cone on client
	Int32						m_clientConeIndex;

	// Next time until we can perform actions
	Double						m_nextActionTime;

	// Buttons pressed last frame
	Int32						m_prevFrameButtons;
	// Buttons pressed right now
	Int32						m_buttonsPressed;
	// Buttons released since last
	Int32						m_buttonsReleased;

	// Health value on client
	Int32						m_clientHealth;
	// Time at which to reload
	Double						m_reloadTime;
	// Time at which we died
	Double						m_deathTime;
	// Stamina on client
	Float						m_clientStamina;
	// Kevlar on client
	Float						m_clientKevlar;

	// Current falling velocity
	Float						m_fallingVelocity;
	// TRUE if hud should be visible
	bool						m_isHUDVisible;
	// TRUE if hud is visible on client
	bool						m_clientHUDVisible;

	// Movement noise
	Float						m_movementNoise;
	// Client movement noise
	Float						m_clientMovementNoise;

private:
	// Used by ladder code
	CEnvLadder*					m_pLadderEntity;
	// Ladder state
	Int32						m_ladderState;
	// Ladder update time
	Double						m_ladderUpdateTime;

	// Ladder destination origin
	Vector						m_ladderDestOrigin;
	// Ladder destination angles
	Vector						m_ladderDestAngles;

	// Ladder move time
	Double						m_nextLadderMoveTime;
	// Ladder move direction
	Int32						m_ladderMoveDirection;
	// Client move direction
	Int32						m_clientLadderMoveDirection;

public:
	// Next time when bike states update
	Double						m_bikeUpdateTime;
	// Current bike state
	Int32						m_bikeState;
	Int32						m_clientBikeState;

	// Bike acceleration
	Float						m_bikeAcceleration;
	// Bike velocity
	Vector						m_bikeVelocity;

	// Bike drop-off origin
	Vector						m_dropOrigin;
	// Bike drop-off angles
	Vector						m_dropAngles;

private:
	// Bike entity pointer
	CItemMotorBike*				m_pBikeEntity;

private:
	// Previous player flags
	Int64						m_prevFlags;
	// Previous buttons
	Int32						m_prevButtons;

	// Ideal lean offset
	Vector						m_idealLeanOffset;
	// Previous lean offset
	Vector						m_prevLeanOffset;
	// Current lean offset
	Vector						m_curLeanOffset;

	// Ideal lean angles
	Vector						m_idealLeanAngles;
	// Previous lean angles
	Vector						m_prevLeanAngles;
	// Current lean angles
	Vector						m_curLeanAngles;

	// Leaning time
	Double						m_leanTime;
	// Leaning state
	Int32						m_leanState;

private:
	// Active weapon
	CPlayerWeapon*				m_pActiveWeapon;
	// Active weapon on client
	CPlayerWeapon*				m_pClientActiveWeapon;
	// Active weapon
	CPlayerWeapon*				m_pNextWeapon;
	// Previous weapon
	CPlayerWeapon*				m_pPreviousWeapon;
	// Last weapon used
	CPlayerWeapon*				m_pLastWeapon;
	// True if we have a new weapon
	bool						m_hasNewWeapon;

	// Weapon flash value
	Int32						m_weaponFlashBrightness;
	// Weapon sound radius
	Float						m_weaponSoundRadius;
	// Step sound radius
	Float						m_stepSoundRadius;

	// Viewmodel on client
	string_t					m_clientViewModel;

	// Linked list of weapons attached to player
	CPlayerWeapon*				m_pWeaponsList;

	// Next weapon prompt time
	Double						m_weaponFullPromptTime;

	// Number of medkits we have
	Int32						m_numMedkits;
	// Number of medkits on client
	Int32						m_numClientMedkits;
	// Heal progress
	Float						m_healProgress;
	// Client heal progress
	Float						m_clientHealProgress;

	// Force weapon update flag
	bool						m_forceWeaponUpdate;
	// TRUE if weapon is holstered
	bool						m_isWeaponHolstered;

	// Ammo counts for ammo types
	Int32						m_ammoCounts[MAX_AMMO_TYPES];
	// Ammo counts on client
	Int32						m_clientAmmoCounts[MAX_AMMO_TYPES];

	// Flashlight battery
	Float						m_flashlightBattery;
	// Flashlight battery on client
	Float						m_clientFlashlightBattery;

	// TRUE if we have the shoulder light
	bool						m_hasShoulderLight;

private:					
	// Damage types sustained
	Int32						m_damageTypes;
	// Last hit group impacted
	Int32						m_lastImpactedHitGroup;
	// Last damage amount
	Int32						m_lastDmgAmount;
	// Last damage time
	Double						m_lastDamageTime;
	// Next pain sound time
	Double						m_nextPainSoundTime;
	// Drown damage amount
	Float						m_drownDamageAmount;
	// Drown damage healed
	Float						m_drownDamageHealed;
	// TRUE if underwater sound is playing
	bool						m_isUnderwaterSoundPlaying;
	// Time we went underwater
	Double						m_underwaterTime;
	// Last underwater dmg time
	Double						m_lastWaterDamageTime;
	// Last water damage dealt
	Float						m_lastWaterDamage;
	// Next swim sound time
	Double						m_nextSwimSoundTime;
	// Last water level
	Int32						m_prevWaterLevel;

	// Damage values for time based damages
	Double						m_timeBasedDmgTime[NUM_TIMEBASED_DMG];
	// Last time we were hurt by time based dmg
	Double						m_lastTimeBasedDmgTime[NUM_TIMEBASED_DMG];

	// Dialouge playback timer
	Double						m_dialougePlaybackTime;

private:
	// Music playback info
	music_data_t				m_musicPlaybackInfoArray[NB_MUSIC_CHANNELS];

private:
	// Current tape track's name
	string_t					m_tapeTrackFile;
	// Tape track playback begin time
	Double						m_tapeTrackPlayBeginTime;
	// Tape track duration in seconds
	Float						m_tapeTrackDuration;
	// Playback title
	string_t					m_tapePlaybackTitle;
	// Tape playback title color
	Vector						m_tapeTitleColor;
	// Tape playback alpha
	Float						m_tapePlaybackAlpha;

private:
	// Current diary track's name
	string_t					m_currentDiaryTrackName;
	// Diary track playback begin time
	Double						m_diaryTrackPlayBeginTime;
	// Diary track duration in seconds
	Float						m_diaryTrackDuration;
	// Diary entity pointer
	CEntityHandle				m_diaryEntity;

private:
	// Linked list of aware NPCs and their awareness
	CLinkedList<npc_awarenessinfo_t> m_npcAwarenessList;

	// Current highest awareness level
	Float						m_highestAwarenessLevel;
	// Current highest awareness level on client
	Float						m_clientHighestAwarenessLevel;

private:
	// Array of active objectives
	string_t					m_objectivesArray[GAMEUI_MAX_OBJECTIVES];
	// Flags indicating which objective is new/changed
	Int16						m_objectivesNewFlags;
	// Last time an objective was added
	Double						m_lastObjectiveAddTime;

private:
	// Current player usable object selected
	CEntityHandle				m_currentPlayerUsableObject;
	// Current usable object mins
	Vector						m_currentUsableObjectMins;
	// Current usable object maxs
	Vector						m_currentUsableObjectMaxs;
	// Current usable object type
	Int32						m_currentUsableObjectType;

private:
	// Countdown timer ending time
	Double						m_countdownTimerEndTime;
	// Countdown timer title
	string_t					m_countdownTimerTitle;

private:
	// Delayed global trigger time
	Double						m_delayedGlobalTriggerTime;
	// Delayed global trigger target entity
	string_t					m_delayedGlobalTriggerTarget;

private:
	// Stores the current autoaim vector
	Vector						m_autoAimVector;
	// Last sent autoaim vector
	Vector						m_lastAutoAimVector;
	// Tells if autoaim is on-target
	bool						m_isOnTarget;

private:
	// TRUE if using cheat commands
	static bool					m_cheatCommandUsed;
};
#endif //PLAYER_H