/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef HUD_H
#define HUD_H

#include "constants.h"
#include "weapons_shared.h"
#include "matrix.h"

class CCVar;
struct en_texture_t;
struct font_set_t;
class CGameHUD;

struct subtitle_t
{
	subtitle_t():
		delay(0),
		r(0),
		g(0),
		b(0)
		{}

	CString szname;
	CString subtitle;

	Float delay;
	Float r;
	Float g;
	Float b;
};

struct active_subtitle_t
{
	active_subtitle_t():
		psubtitle(nullptr),
		holdtime(0),
		timeout(0),
		spawntime(0)
		{}

	subtitle_t* psubtitle;
	Double holdtime;
	Double timeout;
	Double spawntime;
};

struct weapon_t
{
	weapon_t():
		ammotype(0),
		maxammo(0),
		maxclip(0),
		slot(0),
		slotposition(0),
		flags(0),
		weaponid(0),
		clip(0),
		dualclipleft(0),
		dualclipright(0),
		count(0),
		cone(0)
	{
	}

	CString		name;
	Int32		ammotype;
	Int32		maxammo;
	Int32		maxclip;
	Uint32		slot;
	Int32		slotposition;
	Int32		flags;
	Int32		weaponid;
	Int32		clip;

	Int32		dualclipleft;
	Int32		dualclipright;

	Int32		count;
	Int32		cone;
};

struct radiomessage_t
{
	radiomessage_t():
		life(0),
		spawntime(0),
		entindex(NO_ENTITY_INDEX)
		{}

	CString callername;
	color32_t color;
	Float life;
	Double spawntime;
	entindex_t entindex;
};

/*
====================
CWeaponInfo

====================
*/
class CWeaponInfo
{
public:
	CWeaponInfo( void );
	~CWeaponInfo( void );

public:
	// Resets the class
	void Reset( void );

	// Adds a new weapon to the info structure
	void AddWeapon( const weapon_t& weapon );
	// Assigns a weapon to a slot
	void AssignWeapon( weapon_t* pweapon );
	// Returns a weapon for an ID
	weapon_t* GetWeapon( Uint32 id );
	// Removes a weapon from it's slot
	void RemoveWeapon( weapon_t* pweapon );
	// Removes all weapons from their slots
	void RemoveAllWeapons( void );

	// Returns a weapon at a specific slot and slot position
	weapon_t* GetWeaponFromSlot( Uint32 slot, Uint32 position );
	// Returns the first weapon in a slot
	weapon_t* GetSlotFirstWeapon( Uint32 slot );
	// Returns the next available weapon in the slot
	weapon_t* GetNextSlotWeapon( Uint32 slot, Uint32 position );

	// Tells if a weapon has any ammy
	bool WeaponHasAmmo( weapon_t* pweapon ) const;
	// Returns the ammo count for an ammo type
	Uint32 GetAmmoCount( Int32 ammoId ) const;
	// Sets ammo count for an ammo type
	void SetAmmoCount( Int32 ammoId, Uint32 count );

private:
	// Array of weapons
	weapon_t	m_weaponsArray[MAX_WEAPONS];

	// Slot assignments
	weapon_t*	m_pSlotsArray[MAX_WEAPON_SLOTS][MAX_SLOT_POSITIONS];
	// Ammo counts
	Uint32		m_ammoCounts[MAX_AMMO_TYPES];
};

/*
====================
CHUDHistory

====================
*/
class CHUDHistory
{
public:
	enum history_type_t
	{
		HISTORY_AMMO = 0,
		HISTORY_WEAPON,
		HISTORY_ITEM
	};

public:
	// Time an item spends active
	static const Float HUD_HISTORY_DRAW_TIME;
	// History spacing
	static const Uint32 HISTORY_SPACING;
	// Max history height
	static const Float MAX_HISTORY_HEIGHT;
	// Text color
	static color32_t HISTORY_TEXT_COLOR;
	// Fade time for history texts
	static const Float HISTORY_FADE_TIME;

private:
	struct hudhistory_t
	{
		hudhistory_t():
			die(0),
			hascount(false),
			count(0)
			{}

		Float die;
		CString description;

		bool hascount;
		Uint32 count;
	};

public:
	CHUDHistory( CGameHUD& mainHUD );
	~CHUDHistory( void );

public:
	// Resets the list
	void Reset( void );

	// Adds a history element
	void AddElement( Uint32 type, Uint32 id, Uint32 count = 0 );
	// Adds a history element
	void AddElement( Uint32 type, const Char* pstrname, Uint32 count = 0 );

	// Tells if we have any elements to draw
	bool HasActiveElements( void );

	// Draws the history
	bool Draw( void );

private:
	// HUD onbject using us
	CGameHUD& m_hud;
	// List of history objects
	CLinkedList<hudhistory_t> m_historyList;
};

/*
====================
CGameHUD

====================
*/
class CGameHUD
{
public:
	friend class CHUDHistory;

public:
	// Default HUD alpha value
	static const Float HUD_DEFAULT_ALPHA;
	// HUD white color
	static const Vector HUD_COLOR_WHITE;
	// HUD gray color
	static const Vector HUD_COLOR_GRAY;
	// HUD black color
	static const Vector HUD_COLOR_BLACK;
	// HUD red color
	static const Vector HUD_COLOR_RED;
	// HUD blue color
	static const Vector HUD_COLOR_BLUE;
	// HUD orange color
	static const Vector HUD_COLOR_ORANGE;

	// Tilt amount
	static const Uint32 TILT_AMOUNT;
	// Generic tab size
	static const Uint32 TAB_GENERIC_SIZE_X;
	static const Uint32 TAB_GENERIC_SIZE_Y;
	static const Uint32 TAB_MOVENOISE_SIZE_X;
	// Radio message tab width
	static const Uint32 RADIO_MSGTAB_SIZE_X;
	static const Uint32 RADIO_MSGTAB_SIZE_Y;
	static const Uint32 RADIO_MSGTAB_SPACING;
	static const Float RADIO_MSG_FADETIME;
	// Tab label size
	static const Uint32 TAB_LABEL_SIZE_X;
	// Heal tab width
	static const Uint32 TAB_HEAL_SIZE_X;
	// Healthkit tab width
	static const Uint32 TAB_HEALTHKIT_SIZE_X;
	// Tactical icon width
	static const Uint32 TACTICAL_ICON_X;
	// Tactical icon height
	static const Uint32 TACTICAL_ICON_Y;
	// Tactical tab width
	static const Uint32 TAB_TACTICAL_SIZE_X;
	// Weapon tab width
	static const Uint32 TAB_WEAPON_SIZE_X;
	// Clip label width
	static const Uint32 LABEL_CLIP_SIZE_X;
	// Percentage bar height
	static const Uint32 PERCENTAGE_BAR_HEIGHT;
	// Icon size
	static const Uint32 ICON_SIZE;
	// Ammo icon width
	static const Float AMMOICON_SIZE_X;
	// Ammo icon height
	static const Float AMMOICON_SIZE_Y;
	// Weapon rubicle width
	static const Uint32 RUBICLE_WEAPONINFO_SIZE_X;
	// List label T width
	static const Uint32 LIST_LABEL_T_X;
	// List label T height
	static const Uint32 LIST_LABEL_T_Y;
	// List label S width
	static const Uint32 LIST_LABEL_S_X;
	// List label S height
	static const Uint32 LIST_LABEL_S_Y;
	// Weapon tab X
	static const Uint32 LIST_WEAPONTAB_X;
	// Weapon tab Y
	static const Uint32 LIST_WEAPONTAB_Y;
	// List edge size
	static const Uint32 LIST_EDGE_SIZE;
	// Subtitle tab width
	static const Uint32 SUBTITLE_TAB_SIZE_X;
	// Subtitle tab height
	static const Uint32 SUBTITLE_TAB_SIZE_Y;
	// Subtitle timeout duration
	static const Float SUBTITLE_TAB_TIMEOUT;
	// Subtitle tab fade out time
	static const Float	SUBTITLE_TAB_FADETIME;
	// Subtitle tab fade in time
	static const Float	SUBTITLE_TAB_FADEINTIME;
	// Ammo bar height
	static const Uint32 AMMOBAR_SIZE_Y;
	// Ammo bar width
	static const Uint32 AMMOBAR_SIZE_X;
	// Stamina fade time
	static const Uint32 STAMINA_FADE_TIME;
	// Subtitle spacing
	static const Float SUBTITLE_GAP;
	// Subtitle inset
	static const Float SUBTITLE_INSET;

	// HUD description script path
	static const Char HUD_DESCRIPTION_SCRIPT_PATH[];
	
	// Font set for HUD
	static const Char HUD_FONT_SCHEMA_FILENAME[];
	// Font set for counters
	static const Char HUD_COUNTER_FONT_SCHEMA_FILENAME[];
	// Font set for subtitles
	static const Char HUD_SUBTITLE_FONT_SCHEMA_FILENAME[];

public:
	struct hud_iconinfo_t
	{
		hud_iconinfo_t():
			ptexture(nullptr)
			{}

		CString name;
		en_texture_t* ptexture;
	};

	struct hud_weaponinfo_t
	{
		hud_weaponinfo_t():
			weaponid(WEAPON_NONE),
			pweapon_icon(nullptr),
			pammo_icon(nullptr)
			{}

		weaponid_t weaponid;
		en_texture_t* pweapon_icon;
		en_texture_t* pammo_icon;

		CString description;
		CString description_dual;
		CString description_ammo;
	};

	struct hud_iteminfo_t
	{
		hud_iteminfo_t():
			weaponid(WEAPON_NONE)
			{}

		CString name;
		weaponid_t weaponid;
		CString description;
	};

	struct hud_infopair_t
	{
		CString key;
		CString value;
	};

public:
	CGameHUD( void );
	~CGameHUD( void );

public:
	// Initializes the class
	void Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes the class's GL stuff
	bool InitGL( void );
	// Shuts down the class's GL stuff
	void ClearGL( void );

	// Initializes game objects
	bool InitGame( void );
	// Clears game objects
	void ClearGame( void );

	// Draws the HUD
	bool Draw( void );
	// Performs think functions
	void Think( void );

	// Called from DrawNormal
	bool DrawNormal( void );

	// Tells if the HUD is active
	bool IsActive( void ) const;

public:
	// Returns the stamina amount
	Float GetStamina( void ) const { return m_stamina/100.0f; }
	// Returns the health amount
	Float GetHealth( void ) const { return m_health; }
	// Returns the flashlight battery charge
	Float GetFlashlightBattery( void ) const { return m_tacticalAmount; }
	// Returns the weapon selection
	Int32 GetWeaponSelect( void ) const { return m_weaponSelection; }
	// Sets the weapon selection
	void SetWeaponSelect( Int32 select ) { m_weaponSelection = select; }
	// Selects a weapon slot
	void SelectWeaponSlot( Uint32 slot, bool advance, Int32 direction );

	// Adds a subtitle
	bool AddSubtitle( const Char *szname, Float holdtime );
	// Removes a playing subtitle
	void RemoveSubtitle( const Char *szname );
	// Adds a radio message
	void AddRadioMessage( const Char* pstrcallername, const color32_t& color, Float lifetime, entindex_t entindex );

	// Sets the stamina amount
	void SetStamina( Float stamina );
	// Sets the health amount
	void SetHealth( Float health );
	// Sets the kevlar amount
	void SetKevlar( Float kevlar );
	// Sets the number of healthkits
	void SetHealthkitNumber( Uint32 numhealthkits );
	// Sets the healing progress amount
	void SetHealProgress( Float progress );
	// Sets current weapon information
	void SetCurrentWeapon( Int32 state, Int32 id, Uint32 clip, Uint32 clipright, Uint32 clipleft, Uint32 cone );
	// Sets data for a weapon
	void AddWeaponList( const Char* pstrname, Int32 ammotype, Int32 maxammo, Int32 maxclip, Uint32 slot, Uint32 slotposition, Uint32 weaponid, Int32 flags );
	// Sets ammo count
	void SetAmmoCount( Uint32 ammoIndex, Uint32 ammoCount );
	// Manages an ammo pickup event
	void AmmoPickup( const Char* pstrentityname, Uint32 count );
	// Manages a weapon pickup event
	void WeaponPickup( Int32 id );
	// Manages an item pickup event
	void ItemPickup( const Char* pstrentityname );
	// Sets active state for HUD
	void SetActive( bool active );
	// Sets tactical battery value and state
	void SetTacticalBattery( bool ison, Float amount );
	// Sets the movement noise
	void SetMovementNoise( Float noise );
	// Sets the movement noise
	void SetNPCAwareness( Float awareness );
	// Sets new objective flag
	void SetNewObjective( bool newObjective );
	// Sets usable object mins/maxs
	void SetUsableObjectMinsMaxs( const Vector& mins, const Vector& maxs, usableobject_type_t type );
	// Sets countdown timer
	void SetCountdownTime( Double endTime, const Char* pstrTitle );
	// Set autoaim vector
	void SetAutoaimVector( Float autoAimX, Float autoAimY, bool isOnTarget );

private:
	// Draws a bar tab
	bool DrawTab_Bar( Float x, Float y, Char *sztext, en_texture_t *picon, Float bar, Int32 width, Float alpha, Float *ox = nullptr, Float *oy = nullptr, bool reverseColor = false );
	// Draws the weapon tab
	bool DrawWeaponTab( void );
	// Draws the healthkit tab
	bool DrawHealthkitTab( Float x, Float y );
	// Draws the tactical tab
	bool DrawTacticalTab( Float x, Float y );
	// Draws the weapons list
	bool DrawWeaponList( void );
	// Draws the crosshair
	bool DrawCrosshair( void );
	// Draws radio messages
	bool DrawRadioMessages( void );
	// Draws the noise tab
	bool DrawNoiseTab( void );
	// Draws the NPC awareness tab
	bool DrawNPCAwarenessTab( void );
	// Draws usable object reticle
	bool DrawUsableObjectReticle( void );
	// Draws countdowntimer
	bool DrawCountdownTimer( void );

	// Draws subtitles
	bool DrawSubtitles( void );
	// Initializes subtitles
	bool InitSubtitles( void );

	// Loads the HUD script
	bool LoadHUDScript( void );
	// Returns value for a key
	const Char* GetValueForKey( const CArray<hud_infopair_t>& srcArray, const Char* pstrKey );

	// Returns an icon texture by name 
	en_texture_t* GetIconTextureByName( const Char* pstrName );
	// Returns an item description by name
	const Char* GetItemDescriptionByName( const Char* pstrName );
	// Returns an item description by weapon id
	const Char* GetItemDescriptionByWeaponId( weaponid_t weaponid );
	// Returns a weapon info structure based on id
	const hud_weaponinfo_t* GetWeaponInfoById( weaponid_t weaponid );
	// Returns a weapon icon texture based on id
	en_texture_t* GetWeaponIconById( weaponid_t weaponid );

	// Returns information for a weapon
	void GetWeaponInfo( Int32 id, bool dual, en_texture_t *&picon, CString& name, CString& cartridge ) const;
	// Retrieves the cone size for a weapon
	Vector GetConeSize( Int32 coneindex ) const;

	// Adds an active subtitle
	void AddActiveSubtitle( subtitle_t* psubtitle, Float holdtime, Float timeout );

	// Plays a sound
	static void PlaySound( const Char* pstrfilepath );

	// Tells if we have any weapons
	bool HasAnyWeapons( void ) const;

public:
	// Processes a slot input
	void SlotInput( Int32 iSlot );
	// Closes the weapon tab
	void UserCmd_Close( void );
	// Gets the next weapon
	void UserCmd_NextWeapon( void );
	// Gets the previous weapon
	void UserCmd_PrevWeapon( void );

private:
	en_texture_t *m_pHealthIcon;
	en_texture_t *m_pKevlarIcon;
	en_texture_t *m_pTacticalIcon;
	en_texture_t *m_pMedkitIcon;
	en_texture_t *m_pStaminaIcon;
	en_texture_t *m_pNoiseIcon;
	en_texture_t *m_pAwarenessIcon;
	en_texture_t *m_pRadioIcon;
	en_texture_t *m_pNewObjectivesIcon;

	const hud_weaponinfo_t* m_pActiveWeaponInfo;

public:
	// Active subtitle header
	CLinkedList<active_subtitle_t> m_activeSubtitlesList;

	// Array of subtitles
	CArray<subtitle_t>	m_subtitlesArray;

	// Controls rendering of subtitles
	CCVar*		m_pCvarDrawSubtitles;

public:
	weapon_t	*m_pActiveSelection;
	weapon_t	*m_pLastSelection;
	weapon_t	*m_pWeapon;

	Int32		m_weaponSelection;
	bool		m_isDrySelection;

	// Linked list of radio messages
	CLinkedList<radiomessage_t> m_radioMessagesList;

public:
	CWeaponInfo	m_weaponInfo;
	CHUDHistory* m_pHistory;

public:
	const font_set_t *m_pFontSet;
	const font_set_t *m_pSubtitleSet;
	const font_set_t *m_pCounterFont;

private:
	Float	m_stamina;
	Float	m_health;
	Float	m_kevlar;
	Double	m_staminaFadeTime;

	bool	m_isTacticalOn;
	Float	m_tacticalAmount;

	Float	m_healProgress;
	Int32	m_numMedkits;

	Float	m_movementNoise;
	Float	m_npcAwareness;

	bool	m_newObjective;

	CString	m_countdownTimerTitle;
	Double	m_countdownTimerTime;

	// Ideal autoaim vector
	Vector m_idealAutoAimVector;
	// Previous autoaim vector
	Vector m_currentAutoAimVector;
	// TRUE if locked onto a target
	bool m_isOnTarget;

private:
	// Usable object world mins
	Vector m_usableObjectMins;
	// Usable object maxs
	Vector m_usableObjectMaxs;
	// Modelview matrix
	CMatrix m_modelviewMatrix;
	// Projection matrix
	CMatrix m_projectionMatrix;
	// Usable object type
	usableobject_type_t m_usableObjectType;

	// Controls rendering of subtitles
	CCVar* m_pCvarDrawUseReticle;

private:
	// Old weapon bits
	Int32 m_prevWeaponBits;
	// Current weapon bits
	Int32 m_weaponBits;
	// Last key bits
	Int32 m_keyBits;
	// TRUE if HUD is active
	bool m_isActive;
	// Weapon select usermsg id
	Uint32 m_weaponSelectUserMSGId;

	// Screen width
	Uint32 m_screenWidth;
	// Screen Height
	Uint32 m_screenHeight;

	// Array of icons
	CArray<hud_iconinfo_t> m_iconInfoArray;
	// Array of weapons
	CArray<hud_weaponinfo_t> m_weaponInfoArray;
	// Array of items
	CArray<hud_iteminfo_t> m_itemInfoArray;

	// Last time think was called
	Double m_lastThinkTime;
};
extern CGameHUD gHUD;
#endif //HUD_H