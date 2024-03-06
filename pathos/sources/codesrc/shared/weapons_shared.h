/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef WEAPONS_SHARED_H
#define WEAPONS_SHARED_H

#include "constants.h"

// Default recoil degradation
static const Float DEFAULT_RECOIL_DEGRADE	= 0.5f;
// Default recoil limit
static const Float DEFAULT_RECOIL_LIMIT		= 3.5f;
// Value marking the weapon having no clip
static const Int32 WEAPON_NO_CLIP = -1;

// Weapon cones
static const Vector VECTOR_CONE_1DEGREES	= Vector( 0.00873, 0.00873, 0.00873 );
static const Vector VECTOR_CONE_2DEGREES	= Vector( 0.01745, 0.01745, 0.01745 );
static const Vector VECTOR_CONE_3DEGREES	= Vector( 0.02618, 0.02618, 0.02618 );
static const Vector VECTOR_CONE_4DEGREES	= Vector( 0.03490, 0.03490, 0.03490 );
static const Vector VECTOR_CONE_5DEGREES	= Vector( 0.04362, 0.04362, 0.04362 );
static const Vector VECTOR_CONE_6DEGREES	= Vector( 0.05234, 0.05234, 0.05234 );
static const Vector VECTOR_CONE_7DEGREES	= Vector( 0.06105, 0.06105, 0.06105 );
static const Vector VECTOR_CONE_8DEGREES	= Vector( 0.06976, 0.06976, 0.06976 );
static const Vector VECTOR_CONE_9DEGREES	= Vector( 0.07846, 0.07846, 0.07846 );
static const Vector VECTOR_CONE_10DEGREES	= Vector( 0.08716, 0.08716, 0.08716 );
static const Vector VECTOR_CONE_15DEGREES	= Vector( 0.13053, 0.13053, 0.13053 );
static const Vector VECTOR_CONE_20DEGREES	= Vector( 0.17365, 0.17365, 0.17365 );

#define DEFINE_BULLET_TYPE( bullet_type ) { bullet_type, #bullet_type }

// Ammo pickup sound file
static const Char AMMO_PICKUP_SOUND[] = "items/pickup_ammo.wav";
// Weapon pickup sound file
static const Char WEAPON_PICKUP_SOUND[] = "items/pickup_weapon.wav";
// Model file for bullet casings
static const Char BULLET_CASINGS_MODEL[] = "models/shellcasings.mdl";

// 9mm ammotype name
static const Char AMMOTYPE_9MM_NAME[] = "9mm";
// .45 S&W ammotype name
static const Char AMMOTYPE_45SW_NAME[] = ".45SW";
// .22 LR ammotype name
static const Char AMMOTYPE_22LR_NAME[] = ".22LR";
// .50 AE ammotype name
static const Char AMMOTYPE_50AE_NAME[] = ".50AE";
// NATO 5.56 ammotype name for Sig552
static const Char AMMOTYPE_556_SIG552_NAME[] = "5.56_Sig552";
// Buckshot ammotype name for M249
static const Char AMMOTYPE_BUCKSHOT_NAME[] = "Buckshot";
// NATO 5.56 ammotype name for M249
static const Char AMMOTYPE_556_M249_NAME[] = "5.56_M249";
// 7.62 ammotype name
static const Char AMMOTYPE_762_NAME[] = "7.62";
// 40x46mm Grenade ammotype name
static const Char AMMOTYPE_40X46MM_NAME[] = "40x46mm";
// Grenade ammotype name
static const Char AMMOTYPE_MK2GRENADE_NAME[] = "Mk2Grenade";
// Grenade ammotype name
static const Char AMMOTYPE_URANIUM_NAME[] = "U238 AP";

// Maximum 9mm ammo
static const Uint32 MAX_9MM_AMMO = 85;
// Maximum .45 SW ammo
static const Uint32 MAX_45SW_AMMO = 96;
// Maximum .22 LR ammo
static const Uint32 MAX_22LR_AMMO = 128;
// Maximum .50 AE ammo
static const Uint32 MAX_50AE_AMMO = 21;
// Maximum Sig552 5.56 ammo
static const Uint32 MAX_556_SG552_AMMO = 150;
// Maximum buckshot ammo
static const Uint32 MAX_BUCKSHOT_AMMO = 54;
// Maximum M249 5.56 ammo
static const Uint32 MAX_556_M249_AMMO = 200;
// Maximum 7.62 ammo
static const Uint32 MAX_762_AMMO = 15;
// Maximum 40x46 mm grenade ammo
static const Uint32 MAX_40X46_AMMO = 6;
// Maximum Mk2 Grenades
static const Uint32 MAX_MK2_GRENADES = 5;
// Maximum Uranium ammo
static const Uint32 MAX_URANIUM_AMMO = 126;

// Maximum amount of weapons
static const Uint32 MAX_WEAPONS = 32;
// Maximum amount of weapons
static const Uint32 MAX_AMMO_TYPES = 32;

// Max slot positions
static const Uint32 MAX_SLOT_POSITIONS		= 5;
// Max weapon slots
static const Uint32 MAX_WEAPON_SLOTS		= 5;

// Gun loud volume
static const Uint32 GUN_VOLUME_LOUD = 4096;
// Gun normal volume
static const Uint32 GUN_VOLUME_NORMAL = 2048;
// Gun low volume
static const Uint32 GUN_VOLUME_LOW = 1024;

// Gun bright flash
static const Uint32 GUN_FLASH_BRIGHT = 512;
// Gun normal flash
static const Uint32 GUN_FLASH_NORMAL = 256;
// Gun dim flash
static const Uint32 GUN_FLASH_LOW = 128;

// Maximum distance traveled by bullet
static const Float BULLET_MAX_DISTANCE = 8192;

// No ammo add result
static const Int32 NO_AMMO_INDEX = -1;

#define DEFINE_WEAPON_MAPPING( weapon ) { weapon, #weapon }

// Max weapons in a given slot
static const Uint32 WEAPON_SLOT_LIMITS[MAX_SLOT_POSITIONS] =
{
	1, // Melee slot
	3, // Pistol slot
	2, // Primary weapon slot
	2, // Large weapon slot
	1 // Explosive weapon slot
};

// Weapon flags
enum weaponflags_t
{
	FL_WEAPON_SELECT_ON_EMPTY		= (1<<0),
	FL_WEAPON_NO_AUTO_RELOAD		= (1<<1),
	FL_WEAPON_NO_AUTO_SWITCH_EMPTY	= (1<<2),
	FL_WEAPON_LIMIT_IN_WORLD		= (1<<3),
	FL_WEAPON_EXHAUSTIBLE			= (1<<4),
	FL_WEAPON_AUTO_DRAW				= (1<<5),
	FL_WEAPON_NO_FIRERATE_LIMIT		= (1<<7),
	FL_WEAPON_NO_AUTO_AIM			= (1<<8)
};

// Weapon IDs
enum weaponid_t
{
	WEAPON_NONE	= 0,
	WEAPON_GLOCK,
	WEAPON_HANDGRENADE,
	WEAPON_KNIFE,

	// Must be last
	NUM_WEAPONS
};

struct weapon_mapping_t
{
	Int32 id;
	Char* name;
};

// Names of weapon entities
static const Char* WEAPON_ENTITY_NAMES[NUM_WEAPONS] = 
{
	"",
	"weapon_glock",
	"weapon_handgrenade",
	"weapon_knife"
};

// Max ammo counts for weapons
static const Uint32 MAX_AMMO_COUNTS[NUM_WEAPONS] = 
{
	0,
	MAX_9MM_AMMO,
	MAX_MK2_GRENADES,
	0
};

enum bullet_types_t
{
	BULLET_NONE = 0,
	BULLET_PLAYER_GLOCK,
	BULLET_PLAYER_KNIFE,

	BULLET_NPC_9MM,
	BULLET_NPC_BUCKSHOT,
	BULLET_NPC_SIG552,
	BULLET_NPC_M249,
	BULLET_NPC_TRG42,

	NB_BULLET_TYPES
};

struct bullet_typemapping_t
{
	Int32 type;
	Char* name;
};

extern bullet_typemapping_t BULLET_TYPE_MAP[];

//
// w_objects bodygroups
//
enum wmodel_groups_t
{
	WMODEL_BODY_BASE = 0,
	WMODEL_KEVLAR = 0,
	WMODEL_MEDKIT,
	WMODEL_SECURITY,
	WMODEL_GLOCK,
	WMODEL_GLOCK_CLIP,
	WMODEL_GRENADE,
	WMODEL_GRENADE_PRIMED,
	WMODEL_KNIFE,
	WMODEL_SHOULDERLIGHT,
	WMODEL_SEPARATE_GLOCK_SILENCER,
	WMODEL_SEPARATE_GLOCK_FLASHLIGHT
};

// Autoaim degrees
enum autoaimdegrees_t
{
	AUTOAIM_2DEGREES = 0,
	AUTOAIM_5DEGREES,
	AUTOAIM_8DEGREES,
	AUTOAIM_10DEGREES,

	NB_AUTOAIM_DEGREES
};

// Autoaim degree values
static const Float AUTOAIM_DEGREES_VALUES [] =
{
	0.03489,
	0.08715,
	0.13917,
	0.17364
};

// Returns cone size for weapon based on params
extern Vector Weapon_GetConeSize( Int32 coneindex, Vector leanoffset = ZERO_VECTOR, Vector velocity = ZERO_VECTOR, Vector punchangles = ZERO_VECTOR );

#endif //WEAPONS_SHARED_H