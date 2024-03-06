/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CONSTANTS_H
#define CONSTANTS_H

// Player hull sizes
static const Vector VEC_HULL_MIN = Vector(-16, -16, -36 );
static const Vector VEC_HULL_MAX = Vector( 16,  16,  36 );

// Human hull size
static const Vector VEC_HUMAN_HULL_MIN = Vector( -16, -16, 0 );
static const Vector VEC_HUMAN_HULL_MAX = Vector( 16, 16, 72 );
static const Vector VEC_HUMAN_HULL_DUCK = Vector( 16, 16, 36 );

// Ducked hull size
static const Vector VEC_DUCK_HULL_MIN = Vector(-16, -16, -18 );
static const Vector VEC_DUCK_HULL_MAX = Vector( 16,  16,  18 );

// NPC hull mins
const Vector SMALL_NPC_HULL_MIN = Vector(-12, -12, 0);
// NPC hull maxs
const Vector SMALL_NPC_HULL_MAX = Vector(12, 12, 24);

// View heights
static const Vector VEC_DUCK_VIEW = Vector( 0, 0, 12 );
static const Vector VEC_VIEW = Vector( 0, 0, 32 );

// View punch damping related
static const Float VIEW_PUNCH_DAMPING = 9.0f;
static const Float VIEW_PUNCH_SPRING_CONSTANT = 60.0f;

// Dead view height
static const Float DEAD_VIEWHEIGHT = 8.0f;

// Distance between light updates
static const Float LIGHT_UPDATE_DISTANCE = 8.0f;

// Minimum velocity value in any direction
static const Float STOP_EPSILON = 0.1f;

// Fade speed
static const Float ENTITY_FADE_SPEED = 70;

// Minimum stamina to jump/sprint
static const Float PLAYER_MIN_STAMINA = 0.1f;

// Pickup-able item hull size
static const Vector ITEM_HULL_MIN = Vector(-24, -24, 0);
static const Vector ITEM_HULL_MAX = Vector(24, 24, 16);

// Weapon glow color
static const Vector WEAPON_GLOW_COLOR = Vector(255, 100, 25);
// Black color
static const color24_t BLACK_COLOR = color24_t(0, 0, 0);

// Maximum speed value for motorbike
static const Float MOTORBIKE_MAX_SPEED = 500.0f;
// Exhausted player speed
static const Float PLAYER_EXHAUST_SPEED = 150.0f;
// Sprinting player speed
static const Float PLAYER_SPRINT_SPEED = 280.0f;
// Normal player speed
static const Float PLAYER_NORMAL_SPEED = 210.0f;
// Sneaking/ducking player speed
static const Float PLAYER_SNEAK_SPEED = 100.0f;
// Swimming player speed
static const Float PLAYER_SWIM_SPEED = 115.0f;
// Ducking multiplier for speed
static const Float DUCKING_SPEED_MULTIPLIER = 0.4;
// Noclip movement speed
static const Float PLAYER_NOCLIP_SPEED = 600;

// Maximum noise amount
static const Float PLAYER_MAX_NOISE_LEVEL = 950;

// Step sound delay when on ladder
static const Float STEPTIME_LADDER = 700.0f;
// Step sound delay when in water
static const Float STEPTIME_WATER = 600.0f;
// Normal step time
static const Float STEPTIME_NORMAL = 520.0f;
// Slow walking step time
static const Float STEPTIME_SLOW = 700.0f;
// Step time when moving slowly via flag
static const Float STEPTIME_SLOWMOVE = 720.0f;
// Step time when sprinting
static const Float STEPTIME_SPRINT = 475.0f;

// Maximum leafnums an entity can hold
static const Int32 MAX_EDICT_LEAFNUMS = 256;

// Max entities on server side(client ones start after this index)
static const Uint32 MAX_SERVER_ENTITIES = 65535;
// Reserved space mins for client-side identifiers
static const Uint32 ENTITY_IDENTIFIER_RESERVED_MIN = MAX_SERVER_ENTITIES+1;
// Reserved space mins for client-side identifiers
static const Uint32 ENTITY_IDENTIFIER_RESERVED_MAX = (MAX_SERVER_ENTITIES*2);

// Maximum path size
static const Uint32 MAX_ROUTE_POINTS = 128;

// No precache result
static const Int32 NO_PRECACHE = -1;
// No specific player msg destination
static const Int32 NO_CLIENT_INDEX = -1;
// No server precache index
static const Int32 NO_SERVER_PRECACHE = -1;
// No string value
static const string_t NO_STRING_VALUE = 0;
// No sequence index
static const Int32 NO_SEQUENCE_VALUE = -1;

// Worldspawn entity index
static const Int32 WORLDSPAWN_ENTITY_INDEX = 0;
// Host player entity index
static const Int32 HOST_CLIENT_ENTITY_INDEX = 1;
// World model index
static const Int32 WORLD_MODEL_INDEX = 1;
// No body value
static const Int64 NO_BODY_VALUE = -1;
// No body value
static const Int32 NO_SKIN_VALUE = -1;
// No position value for generic positions
static const Int32 NO_POSITION = -1;

// Path to default footstep file for player
static const Char FOOTSTEP_SCRIPT_FILE[] = "scripts/footsteps.txt";

// Decal list file path
static const Char DECAL_LIST_FILE_PATH[] = "scripts/decal_list.txt";

// Sentences file path
static const Char SENTENCES_FILE_PATH[] = "scripts/sentences.txt";

// Sound folder base path
static const Char SOUND_FOLDER_BASE_PATH[] = "sound/";

// Bone name for head
static const Char HEAD_BONE_NAME[] = "Bip01 Head";
// Bone name for root bone
static const Char ROOT_BONE_NAME[] = "Bip01";
// Bone name for eye center
static const Char EYE_CENTER_BONE_NAME[] = "Bip01 Eye Center";

// Null sound filepath
static const Char NULL_SOUND_FILENAME[] = "common/null.wav";
// Null sprite filepath
static const Char NULL_SPRITE_FILENAME[] = "sprites/null.spr";
// Error model filepath
static const Char ERROR_MODEL_FILENAME[] = "models/error.mdl";
// Error model filepath
static const Char ERROR_SPRITE_FILENAME[] = "sprites/error.spr";
// Weapon world model
static const Char W_OBJECTS_MODEL_FILENAME[] = "models/w_objects.mdl";
// Human gibs model
static const Char HUMAN_GIBS_MODEL_FILENAME[] = "models/humangibs.mdl";
// View hands model path
const Char V_SEQUENCES_MODEL_NAME[] = "models/v_sequences.mdl";
// Default skybox name
static const Char DEFAULT_SKYBOX_NAME[] = "mountain_";
// Glass material type name
static const Char GLASS_MATERIAL_TYPE_NAME[] = "glass";
// Organic material type name
static const Char ORGANIC_MATERIAL_TYPE_NAME[] = "organic";
// Metal material type name
static const Char METAL_MATERIAL_TYPE_NAME[] = "metal";
// Bullets model name
static const Char BULLETS_MODEL_FILENAME[] = "models/bullets.mdl";
// Bullets model name
static const Char SHELLCASINGS_MODEL_FILENAME[] = "models/shellcasings.mdl";

// Particle script base path
static const Char PARTICLE_SCRIPT_PATH[] = "/scripts/particles/";

// Debris sound file path
static const Char DEBRIS_SOUND_BASE_DIR[] = "debris";

// Default FOV cvar name
const Char DEFAULT_FOV_CVAR_NAME[] = "default_fov";
// Reference FOV cvar name
const Char REFERENCE_FOV_CVAR_NAME[] = "reference_fov";
// Node debug cvar name
const Char NODE_DEBUG_CVAR_NAME[] = "r_nodes_debug";

// Anisotropy cvar name
const Char ANISOTROPY_CVAR_NAME[] = "r_anisotropy";
// Gamma cvar name
const Char GAMMA_CVAR_NAME[] = "r_gamma";
// Master volume cvar name
const Char VOLUME_CVAR_NAME[] = "s_volume";
// Game volume cvar name
const Char GAME_VOLUME_CVAR_NAME[] = "s_gamevolume";
// Music volume cvar name
const Char MUSIC_VOLUME_CVAR_NAME[] = "s_musicvolume";
// Mouse sensitivity cvar
const Char MOUSE_SENSITIVITY_CVAR_NAME[] = "m_sensitivity";
// Mouse filter cvar
const Char MOUSE_FILTER_CVAR_NAME[] = "m_filter";
// Mouse filter frames cvar name
const Char MOUSE_FILTER_FRAMES_CVAR_NAME[] = "m_filter_frames";
// Mouse reverse cvar
const Char MOUSE_REVERSE_CVAR_NAME[] = "m_reverse";
// Stop music command name
const Char STOP_MUSIC_CMD_NAME[] = "stopmusic";
// Gravity cvar name
const Char GRAVITY_CVAR_NAME[] = "sv_gravity";
// View roll cvar name
const Char VIEW_ROLL_CVAR_NAME[] = "v_rollangle";
// View bob cvar name
const Char VIEW_BOB_CVAR_NAME[] = "v_viewbob";
// Autoaim cvar name
const Char AUTOAIM_CVAR_NAME[] = "sv_autoaim";

// Minimum filter frames for mouse filtering
static const Uint32 MOUSE_FILTER_MIN_FRAMES = 2;
// Maximum filter frames for mouse filtering
static const Uint32 MOUSE_FILTER_MAX_FRAMES = 8;

// Null mins value
static const Vector NULL_MINS(999999999, 999999999, 999999999);
// Null maxs value
static const Vector NULL_MAXS(-999999999, -999999999, -999999999);

// env_elight radius multiplier
static const Float ENV_ELIGHT_RADIUS_MULTIPLIER = 9.5;
// Max entities on client side(reserved entities start after this + MAX_SERVER_ENTITIES
static const Uint32 MAX_CLIENTSIDE_ENTITIES = 65535;
// Entity index offset for client-side entities
static const Uint32 CL_ENTITY_INDEX_BASE = MAX_SERVER_ENTITIES;

// Max surface extents size
static const Uint32 MAX_SURFACE_EXTENTS = 1024;
// Size of a light block(1024x1024 chop size should give 64x64, but we pad by 2)
static const Uint32 BLOCKLIGHTS_SIZE = (66*66);

// Default field of view value
static const Uint32 DEFAULT_FOV_VALUE = 70;
// Reference field of view value
static const Uint32 REFERENCE_FOV_VALUE = 90;

// No direction value
static const Int32 DIRECTION_NODIR = -1;

// Max rendered entities
static const Int32 MAX_RENDER_ENTITIES = 4096;

// Type for print functions
typedef void (*pfnPrintf_t)( const Char *fmt, ... );
// Type for error popup functions
typedef void (*pfnErrorPopup_t)( const Char *fmt, ... );

// Newline definition
static const Char NEWLINE[] = "\r\n";
// World textures base path
static const Char WORLD_TEXTURES_BASE_PATH[] = "textures/world/";

// Export function macro
#define EXPORTFN _declspec( dllexport )

// Macro for usermsg exports
#define MSGFN extern "C" bool _declspec( dllexport )

// For legacy support
enum entitysteptypes_t
{
	NPC_STEP_NORMAL = 0,
	NPC_STEP_BOOTS,
	NPC_STEP_SKULLFLOWER,
	NPC_STEP_UNUSED,
	
	NB_NPC_LEGACY_STEPTYPES
};

// Type names for legacy support
static const Char* NPC_LEGACY_STEPTYPE_NAMES[NB_NPC_LEGACY_STEPTYPES] = 
{
	"shoes",
	"default",
	"skullflower",
	"unused"
};

enum npc_movetype_t
{
	MOVE_NORMAL = 0,
	MOVE_STRAFE
};

enum walkmove_t
{
	WALKMOVE_NORMAL = 0,
	WALKMOVE_WORLDONLY,
	WALKMOVE_CHECKONLY,
	WALKMOVE_NO_NPCS
};

enum hull_types_t
{
	HULL_AUTO = -1, // Hull will be determined by engine
	HULL_POINT = 0, // Point sized hull
	HULL_HUMAN = 1, // Player/human npcs
	HULL_LARGE = 2, // Large npcs
	HULL_SMALL = 3, // Small npcs

	MAX_MAP_HULLS
};

enum node_hull_types_t
{
	NODE_SMALL_HULL = 0,
	NODE_HUMAN_HULL,
	NODE_LARGE_HULL,
	NODE_FLY_HULL,

	NUM_NODE_HULLS
};

enum node_hull_types_indexes_t
{
	NODE_FLY_HULL_INDEX = HULL_SMALL,
	NODE_SMALL_HULL_INDEX = HULL_POINT,
	NODE_HUMAN_HULL_INDEX = HULL_HUMAN,
	NODE_LARGE_HULL_INDEX = HULL_LARGE,
};

enum node_types_t
{
	AI_NODE_NONE			= 0,
	AI_NODE_LAND			= (1<<0),
	AI_NODE_AIR				= (1<<1),
	AI_NODE_WATER			= (1<<2),
	AI_NODE_AHEAD			= (1<<3),
	AI_NODE_GROUP_REALM		= (AI_NODE_LAND|AI_NODE_AIR|AI_NODE_WATER)
};

enum wandernpc_dest_type_t
{
	WANDER_NPC_DEST_WANDER = 0,
	WANDER_NPC_DEST_WINDOW,
	WANDER_NPC_DEST_SIT
};

enum gibbing_t
{
	GIB_NORMAL = 0,
	GIB_NEVER,
	GIB_ALWAYS
};

enum deathmode_t
{
	DEATH_NORMAL = 0,
	DEATH_DECAPITATED,
	DEATH_EXPLODE
};

enum deathstate_t
{
	DEADSTATE_NONE = 0,
	DEADSTATE_DYING,
	DEADSTATE_DEAD
};

enum takedamage_t
{
	TAKEDAMAGE_NO = 0,
	TAKEDAMAGE_YES,
	TAKEDAMAGE_AIM
};

enum stance_t
{
	STANCE_ACTUAL = 0,
	STANCE_STANDING,
	STANCE_CROUCHING
};

enum bloodcolor_t
{
	BLOOD_NONE = -1,
	BLOOD_RED
};

enum node_msg_type_t
{
	NODE_DEBUG_PATHS = 0,
	NODE_DEBUG_BBOX,
	NODE_DEBUG_WAYPOINT
};

enum waypoint_type_t
{
	WAYPOINT_NORMAL = 0,
	WAYPOINT_DETOUR,
	WAYPOINT_ERROR
};

enum breakmaterials_t
{
	MAT_GLASS,
	MAT_WOOD,
	MAT_METAL,
	MAT_FLESH,
	MAT_CINDERBLOCK,
	MAT_CEILINGTILE,
	MAT_COMPUTER,
	MAT_UNBREAKABLE_GLASS,
	MAT_ROCKS,
	MAT_NONE,

	NB_BREAK_MATERIALS
};

enum localmove_t
{
	LOCAL_MOVE_INVALID = 0,
	LOCAL_MOVE_INVALID_NO_TRIANGULATION,
	LOCAL_MOVE_RESULT_FAILURE,

	LOCAL_MOVE_VALID,
	LOCAL_MOVE_REACHED_TARGET
};

enum dmg_types_t
{
	DMG_GENERIC				= 0,
	DMG_BULLET				= (1<<0),
	DMG_SLASH				= (1<<1),
	DMG_BURN				= (1<<2),
	DMG_FREEZE				= (1<<3),
	DMG_FALL				= (1<<4),
	DMG_EXPLOSION			= (1<<5),
	DMG_MELEE				= (1<<6),
	DMG_ELECTRICITY			= (1<<7),
	DMG_NEVERGIB			= (1<<8),
	DMG_ALWAYSGIB			= (1<<9),
	DMG_DROWN				= (1<<10),
	DMG_DROWNRECOVER		= (1<<11),
	DMG_ACID				= (1<<12),
	DMG_AXE					= (1<<13),
	DMG_UNUSED1				= (1<<14),
	DMG_INSTANTDECAP		= (1<<15),
	DMG_CRUSH				= (1<<16),
	DMG_BULLETGIB			= (1<<17),
	DMG_RADIATION			= (1<<18),
	DMG_TIMEBASED_INFLICTED = (1<<20),
	DMG_BUCKSHOT			= (1<<21),
	DMG_BLACKHOLE			= (1<<22),
	DMG_UNUSED2				= (1<<23),
	DMG_UNUSED3			= (1<<24),
	DMG_PENETRATION			= (1<<25),
	DMG_GIB_CORPSE			= (DMG_CRUSH|DMG_FALL|DMG_EXPLOSION|DMG_MELEE),
	DMG_TIMEBASED			= (DMG_DROWN|DMG_DROWNRECOVER|DMG_ACID),
	DMG_BLOODDECAL			= (DMG_CRUSH|DMG_SLASH|DMG_EXPLOSION|DMG_MELEE|DMG_AXE|DMG_BULLET)
};

// Duration of acid damage
static const Float ACID_DMG_DURATION = 4.0f;
// Acid damage amount
static const Float ACID_DMG_AMOUNT = 1.0F;

enum skill_level_t
{
	SKILL_EASY = 0,
	SKILL_NORMAL,
	SKILL_HARD
};

// dlights flags
enum dlightflags_t
{
	FL_DLIGHT_NONE				= 0,
	FL_DLIGHT_STATICSHADOWS		= (1<<0),
	FL_DLIGHT_NOSHADOWS			= (1<<1),
	FL_DLIGHT_FOLLOW_ENTITY		= (1<<2),
	FL_DLIGHT_USE_ATTACHMENT	= (1<<3),
	FL_DLIGHT_USE_LIGHTSTYLES	= (1<<4),
	FL_DLIGHT_USE_SUBKEY		= (1<<5)
};

// decal flags
enum decalflags_t
{
	FL_DECAL_NONE				= 0,
	FL_DECAL_HAS_NORMAL			= (1<<0),
	FL_DECAL_VBM				= (1<<1),
	FL_DECAL_PERSISTENT			= (1<<2),
	FL_DECAL_NORMAL_PERMISSIVE	= (1<<3),
	FL_DECAL_SPECIFIC_TEXTURE	= (1<<4),
	FL_DECAL_ALLOWOVERLAP		= (1<<5),
	FL_DECAL_TRANSITIONED		= (1<<6),
	FL_DECAL_LEVEL_TRANSITION	= (1<<7),
	FL_DECAL_TIED_TO_ENTITY		= (1<<8),
	FL_DECAL_SERVER				= (1<<9),
	FL_DECAL_CL_ENTITY_MANAGER	= (1<<10),
	FL_DECAL_BOGUS				= (1<<11),
	FL_DECAL_DIE				= (1<<12),
	FL_DECAL_DIE_FADE			= (1<<13),
	FL_DECAL_GROW				= (1<<14)
};

// env_decal shared flags
enum envdecalflags_t
{
	FL_ENVDECAL_WAIT_TRIGGER		= (1<<0), // not used
	FL_ENVDECAL_TRANSITION			= (1<<1),
	FL_ENVDECAL_NO_VBM				= (1<<2),
	FL_ENVDECAL_PERMISSIVE			= (1<<3),
	FL_ENVDECAL_RANDOM				= (1<<4)
};

// Move these out of const
enum hitgroups_t
{
	HITGROUP_GENERIC = 0,
	HITGROUP_HEAD,
	HITGROUP_CHEST,
	HITGROUP_STOMACH,
	HITGROUP_LEFT_ARM,
	HITGROUP_RIGHT_ARM,
	HITGROUP_LEFT_LEG,
	HITGROUP_RIGHT_LEG,
	HITGROUP_HELMET = 11
};

// Move these out of const
enum relations_t
{
	R_ALLY = 0,
	R_FRIEND,
	R_NONE,
	R_DISLIKE,
	R_HATE,
	R_NEMESIS,

	// Must be last
	NUM_ENEMY_RELATIONS
};

enum classifications_t
{
	CLASS_UNDEFINED = -1,
	CLASS_NONE,
	CLASS_MACHINE,
	CLASS_PLAYER,
	CLASS_HUMAN_HOSTILE,
	CLASS_HUMAN_FRIENDLY,
	CLASS_UNUSED,
	
	NB_CLASSIFICATIONS
};

// Table of npc relations
const Int32 NPC_RELATIONS_TABLE[NB_CLASSIFICATIONS][NB_CLASSIFICATIONS] = 
{
								// CLASS_NONE	CLASS_MACHINE	CLASS_PLAYER	CLASS_HUMAN_HOSTILE		CLASS_HUMAN_FRIENDLY	CLASS_UNUSED
	/*CLASS_NONE*/				{ R_NONE,		R_NONE,			R_NONE,			R_NONE,					R_NONE,					R_NONE		},
	/*CLASS_MACHINE*/			{ R_NONE,		R_ALLY,			R_DISLIKE,		R_DISLIKE,				R_DISLIKE,				R_DISLIKE	},
	/*CLASS_PLAYER*/			{ R_NONE,		R_HATE,			R_ALLY,			R_HATE,					R_ALLY,					R_NEMESIS	},
	/*CLASS_HUMAN_HOSTILE*/		{ R_NONE,		R_DISLIKE,		R_HATE,			R_ALLY,					R_HATE,					R_NEMESIS	},
	/*CLASS_HUMAN_FRIENDLY*/	{ R_NONE,		R_HATE,			R_ALLY,			R_HATE,					R_ALLY,					R_NEMESIS	},
	/*CLASS_UINUSED*/			{ R_NONE,		R_HATE,			R_HATE,			R_HATE,					R_HATE,					R_ALLY		}
};

enum npcstate_t
{
	NPC_STATE_NONE = 0,
	NPC_STATE_IDLE,
	NPC_STATE_COMBAT,
	NPC_STATE_ALERT,
	NPC_STATE_SCRIPT,
	NPC_STATE_DEAD,
	NB_AI_STATES
};

enum togglestate_t
{
	TS_NONE = 0,
	TS_AT_TOP,
	TS_AT_BOTTOM,
	TS_GOING_UP,
	TS_GOING_DOWN
};

enum scriptstate_t
{
	AI_SCRIPT_STATE_NONE = 0,
	AI_SCRIPT_PLAYING,
	AI_SCRIPT_WAIT,
	AI_SCRIPT_STATE_CLEANUP,
	AI_SCRIPT_WALK_TO_MARK,
	AI_SCRIPT_RUN_TO_MARK
};

enum script_interrupt_level_t
{
	SCRIPT_INTERRUPT_IDLE = 0,
	SCRIPT_INTERRUPT_BY_NAME,
	SCRIPT_INTERRUPT_AI
};

enum force_skillcvar_t
{
	FORCE_SKILL_OFF = 0,
	FORCE_SKILL_EASY,
	FORCE_SKILL_NORMAL,
	FORCE_SKILL_HARD,
	FORCE_SKILL_LESS_THAN_HARD
};

// View field value of 180 degrees or so
static const Float VIEW_FIELD_FULL = -1.0;
// View field of 135 degrees
static const Float VIEW_FIELD_WIDE = -0.7;
// View field of 45 degrees
static const Float VIEW_FIELD_MEDIUM = -0.4;
// View field of 20 degrees maybe
static const Float VIEW_FIELD_NARROW = 0.7;

// Max clients in a server
static const Uint32 MAX_PLAYERS = 32;

// Blend time between sequence changes
static const Float VBM_SEQ_BLEND_TIME = 0.3f;

// Zero vector
static const Vector ZERO_VECTOR = Vector(0, 0, 0);

// Values for lean calculations
static const Float LEAN_DISTANCE_UP = 15;
static const Float LEAN_DISTANCE_SIDE = 20;
static const Double LEAN_TIME = 0.4;

enum part_msg_types_t
{
	PART_MSG_SPAWN = 0,
	PART_MSG_REMOVE
};

enum part_script_type_t
{
	PART_SCRIPT_SYSTEM = 0,
	PART_SCRIPT_CLUSTER
};

enum decalcache_type_t
{
	DECAL_CACHE_SINGLE = 0,
	DECAL_CACHE_GROUP
};

enum particle_attach_t
{
	PARTICLE_ATTACH_NONE					= 0, // No attachment behavior
	PARTICLE_ATTACH_TO_PARENT				= (1<<0), // particle system will follow parent entity
	PARTICLE_ATTACH_RELATIVE				= (1<<1), // particle origins and vectors are relative to attachment position
	PARTICLE_ATTACH_TO_ATTACHMENT			= (1<<2), // particle system will be attached to entity's attachment
	PARTICLE_ATTACH_ATTACHMENT_VECTOR		= (1<<3), // direction comes from two attachment vectors
	PARTICLE_ATTACH_TO_BONE					= (1<<4), // system base position is relative to bone
};

enum vm_msg_types_t
{
	VMODEL_SET_SEQUENCE = 0,
	VMODEL_SET_MODEL,
	VMODEL_SET_SKIN,
	VMODEL_SET_BODY,
	VMODEL_SET_OFFSETS_ENABLE
};

enum renderfx_t
{
	RenderFx_None = 0, 
	RenderFx_PulseSlow, 
	RenderFx_PulseFast, 
	RenderFx_PulseSlowWide, 
	RenderFx_PulseFastWide, 
	RenderFx_FadeSlow, 
	RenderFx_FadeFast, 
	RenderFx_SolidSlow, 
	RenderFx_SolidFast, 	   
	RenderFx_StrobeSlow, 
	RenderFx_StrobeFast, 
	RenderFx_StrobeFaster, 
	RenderFx_FlickerSlow, 
	RenderFx_FlickerFast,
	RenderFx_NoDissipation,
	RenderFx_Distort,
	RenderFx_Hologram,
	RenderFx_DeadPlayer,
	RenderFx_Explode,
	RenderFx_GlowShell,
	RenderFx_ClampMinScale,
	RenderFx_NoShadow,
	RenderFx_TraceGlow
};

enum rendertype_t
{
	RT_NORMAL = 0,
	RT_ENVELIGHT,
	RT_ENVDLIGHT,
	RT_ENVSPOTLIGHT,
	RT_WATERSHADER,
	RT_MIRROR,
	RT_ENVSKYENT,
	RT_SKYWATERENT,
	RT_MONITORCAMERA,
	RT_MONITORENTITY,
	RT_ENVPOSPORTAL,
	RT_PORTALSURFACE,
	RT_ENVPOSPORTALWORLD,
	RT_BEAM,
	RT_LENSFLARE
};

enum engine_renderfx_t
{
	RenderFx_SkyEnt = 70,
	RenderFx_SkyEntNC,
	RenderFx_GlowAura = 101,
	RenderFx_MirrorOnly = 109,
	RenderFx_SoundOrg,
	RenderFx_Rotlight,
	RenderFx_RotlightNS,
	RenderFx_MonitorOnly,
	RenderFx_Diary,
	RenderFx_AngularSprite = 115,
	RenderFx_InPortalEntity,
	RenderFx_ScaledModel,
	RenderFx_InPortalScaledModel,
	RenderFx_NoVISChecks
};

// Break Model Defines
enum te_breaktypes_t
{
	TE_BOUNCE_NONE = 0,
	TE_BOUNCE_GLASS,
	TE_BOUNCE_METAL,
	TE_BOUNCE_FLESH,
	TE_BOUNCE_WOOD,
	TE_BOUNCE_SHELL,
	TE_BOUNCE_CONCRETE,
	TE_BOUNCE_SHOTSHELL
};

//
// Black hole msg types
//
enum blackhole_msg_types_t
{
	MSG_BLACKHOLE_SPAWN = 0,
	MSG_BLACKHOLE_KILL
};

//
// Tempentity flags
//
enum te_flags_t
{
	TE_FL_NONE				= 0,
	TE_FL_SINEWAVE			= (1<<0),
	TE_FL_GRAVITY			= (1<<1),
	TE_FL_ROTATE			= (1<<2),
	TE_FL_SLOWGRAVITY		= (1<<3),
	TE_FL_SMOKETRAIL		= (1<<4),
	TE_FL_COLLIDEWORLD		= (1<<5),
	TE_FL_FLICKER			= (1<<6),
	TE_FL_FADEOUT			= (1<<7),
	TE_FL_SPRITE_ANIMATE	= (1<<8),
	TE_FL_HITSOUND			= (1<<9),
	TE_FL_SPIRAL			= (1<<10),
	TE_FL_COLLIDEALL		= (1<<11),
	TE_FL_PERSIST			= (1<<12),
	TE_FL_COLLIDEKILL		= (1<<13),
	TE_FL_SPRITE_ANIM_LOOP	= (1<<14),
	TE_FL_SPRITE_CYCLE		= (1<<15),
	TE_FL_NOGRAVITY			= (1<<16)
};

enum reserved_indexes_e
{
	LAST_ENTITY_INDEX = CL_ENTITY_INDEX_BASE+MAX_CLIENTSIDE_ENTITIES,
	BIKE_ENTITY_INDEX,
	LADDER_ENTITY_INDEX,
	VIEWMODEL_ENTITY_INDEX
};

enum tempentitytypes_t
{
	TE_UNDEFINED = 0,
	TE_BREAKMODEL,
	TE_BUBBLES,
	TE_BUBBLETRAIL,
	TE_FUNNELSPRITE,
	TE_SPHEREMODEL,
	TE_TEMPMODEL,
	TE_TEMPSPRITE,
	TE_PARTICLEEXPLOSION1,
	TE_PARTICLEEXPLOSION2,
	TE_BLOBEXPLOSION,
	TE_ROCKETEXPLOSION,
	TE_PARTICLEEFFECT,
	TE_LAVASPLASH,
	TE_TELEPORTSPLASH,
	TE_ROCKETTRAIL
};

enum trailtype_t
{
	trail_rocket = 0,
	trail_smoke,
	trail_blood,
	trail_tracer1,
	trail_slightblood,
	trail_tracer2,
	trail_voortrail,
	nb_trail_types
};

struct maptexturematerial_t
{
	CString maptexturename;
	CString materialfilepath;
};

enum walkingpaces_t
{
	WALKPACE_NORMAL,
	WALKPACE_SPRINT,
	WALKPACE_SLOW,
	WALKPACE_CROUCH,

	NB_WALKING_PACE_TYPES
};

struct decalcache_t
{
	decalcache_t():
		type(DECAL_CACHE_SINGLE)
		{}

	CString name;
	decalcache_type_t type;
};

// This needs to match with studio.h
static const Uint32 MAX_CONTROLLERS = 4;
static const Uint32 MAX_BLENDING	= 2;

// No entity index
static const entindex_t NO_ENTITY_INDEX = -1;
// No attachment index
static const Int32 NO_ATTACHMENT_INDEX = -1;

enum lightstyles_t
{
	LS_NORMAL = 0,
	LS_FLICKER_A,
	LS_SLOW_STRONG_PULSE,
	LS_CANDLE_A,
	LS_FAST_STROBE,
	LS_GENTLE_PULSE,
	LS_FLICKER_B,
	LS_CANDLE_B,
	LS_CANDLE_C,
	LS_SLOW_STROBE,
	LS_FLUORESCENT_FLICKER,
	LS_SLOW_PULSE_NOBLACK
};

enum daystage_t
{
	DAYSTAGE_NORMAL = 0,
	DAYSTAGE_NIGHTSTAGE,
	DAYSTAGE_DAYLIGHT_RETURN
};

enum entity_free_flags_t
{
	FREE_MSG_FL_NONE			= 0,
	FREE_MSG_FL_KEEP_PARTICLES	= (1<<0),
	FREE_MSG_FL_KEEP_SOUNDS		= (1<<1),
	FREE_MSG_FL_KEEP_DLIGHTS	= (1<<2)
};

enum usableobject_type_t
{
	USABLE_OBJECT_NONE = 0,
	USABLE_OBJECT_DEFAULT,
	USABLE_OBJECT_LOCKED,
	USABLE_OBJECT_UNUSABLE
};

#endif //CONSTANTS_H