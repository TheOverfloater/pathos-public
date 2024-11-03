/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SV_MAIN_H
#define SV_MAIN_H

#include <string>
#include <unordered_map>
#include <map>

#include "gdll_interface.h"
#include "sv_shared.h"
#include "usercmd.h"
#include "gamevars.h"
#include "edict.h"
#include "pm_shared.h"
#include "brushmodel.h"
#include "system.h"
#include "filechunk.h"
#include "networking.h"
#include "hashlist.h"
#include "usermsg.h"
#include "msgreader.h"
#include "save_shared.h"

// Limits on area nodes
static constexpr Int32 MAX_AREA_DEPTH = 4;
static constexpr Int32 MAX_AREA_NODES = 32;

// Max touchents for players
static constexpr Int32 MAX_TOUCHENTS = 64;

// Usercmd buffer allocation size
static constexpr Uint32 USERCMD_ALLOC_SIZE = 64;

// Server dll path
#ifdef _64BUILD
static const Char SERVER_DLL_PATH[] = "dlls/game_x64.dll";
static const Char SERVER_DLL_NAME[] = "game_x64.dll";
#else
static const Char SERVER_DLL_PATH[] = "dlls/game_x86.dll";
static const Char SERVER_DLL_NAME[] = "game_x86.dll";
#endif

class Vector;
class CCVar;
class CNetworking;

struct trace_t;

// Datatype for linking edicts to their private data
typedef void (*pfnPrivateData_t)( edict_t* pedict );
// Datatype for decal cache key
typedef std::pair<CString, decalcache_type_t> DecalCacheMapKey_t;
// Datatype for cache name->index mappings
typedef std::map<DecalCacheMapKey_t, Uint32> DecalCacheNameIndexMap_t;
// Datatype for cache name->index mappings
typedef std::unordered_map<CString, Uint32> CacheNameIndexMap_t;

enum sv_state_t
{
	SV_INACTIVE = 0,
	SV_ACTIVE
};

enum sv_clstate_t
{
	SVCL_OK = 0,
	SVCL_REJECTED,
	SVCL_CLS_BAD,
	SVCL_RESOURCES_ERROR,
	SVCL_SPAWN_FAILED,
	SVCL_DISCONNECTED,
	SVCL_NET_ERROR,
	SVCL_LOST_CONNECTION,
	SVCL_INCONSISTENT_FILE,
	SVCL_NOT_CONSISTENT
};

struct sv_packetentity_t
{
	entity_state_t state;
	entity_state_t cl_state;
};

struct sv_entitypacket_t
{
	sv_entitypacket_t():
		numentities(0),
		cl_packetindex(0)
	{
		for(Uint32 i = 0; i < MAX_VISIBLE_ENTITIES; i++)
			entities[i] = entity_state_t();

		for(Uint32 i = 0; i < cl_entitystates.size(); i++)
			cl_entitystates[i] = entity_state_t();
	}

	CArray<entity_state_t> cl_entitystates;
	CArray<bool> cl_wasinpacket;

	entity_state_t entities[MAX_VISIBLE_ENTITIES];
	Uint32 numentities;

	// client index in packet
	Uint32 cl_packetindex;
};

struct areanode_t
{
	areanode_t():
		index(0),
		axis(0),
		dist(0)
		{
			pchildren[0] = nullptr;
			pchildren[1] = nullptr;
		}

	Int32 index;
	Int32 axis;
	Float dist;

	struct areanode_t *pchildren[2];

	EdictChainType_t trigger_edicts;
	EdictChainType_t solid_edicts;
};

struct sv_upload_t
{
	sv_upload_t():
		fileid(0)
		{}

	// File path
	CString filepath;
	// File id on client
	Uint32 fileid;

	// File chunks
	CLinkedList<filechunk_t*> chunkslist;
};

struct saveddecal_t;

//
// Most of this structure is filled up by a trigger_changelevel
// before the levelchange command is called. it's used to perform the level change
//
struct sv_levelchangeinfo_t
{
	sv_levelchangeinfo_t():
		numtransitionentities(0)
		{
			memset(transitionentitylist, 0, sizeof(transitionentitylist));
		}

	// Level we're changing to
	CString nextlevelname;
	// Level we're changing from
	CString prevlevelname;

	// Landmark name
	CString landmarkname;
	// Landmark position
	Vector landmarkposition;

	// Previous map's save-state filename
	// set after level is saved
	CString prevlevelsavefilename;

	// List of entities to transfer
	Int32 transitionentitylist[MAX_TRANSITIONING_ENTITIES];
	// Number of entities in list
	Uint32 numtransitionentities;

	// List of transitioning decals
	CLinkedList<saveddecal_t> transitiondecallist;
};

struct sv_client_t
{
	sv_client_t():
		index(0),
		pedict(nullptr),
		jointime(0),
		numusercmd(0),
		lastusercmdidx(0),
		active(false),
		spawned(false),
		connected(false),
		initialized(false),
		originset(false)
		{
		};

	// Client's index
	Uint32 index;
	// Pointer to player entity
	edict_t* pedict;
	// Time player joined
	Double jointime;

	// Packet of entities
	sv_entitypacket_t packet;

	// Array of usercmds
	CArray<usercmd_t> usercmdarray;
	// Number of usercmds
	Uint32 numusercmd;

	// Last acknowledged usercmd
	Uint64 lastusercmdidx;

	// Tells if client is active
	bool active;
	// Tells if client has spawned
	bool spawned;
	// Tells if client is connected
	bool connected;
	// Tells if client has been initialized
	bool initialized;
	// Tells if we should set the origin from a spawn point
	bool originset;

	// Current upload info
	sv_upload_t upload;

	// Consistency list of files that need to be checked
	CLinkedList<CString> consistencylist;

	// Playermove information
	pm_info_t pminfo;

	// last sky vector sent
	Vector currentskyvector;
	// last sky color sent
	Vector currentskycolor;

	// Cached up messages
	CLinkedList<usermsgdata_t*> cachedmsglist;
};

struct sv_level_connection_t
{
	// map name this connects to
	CString othermapname;
	// name of landmark on this connection
	CString landmarkname;
};

struct sv_levelinfo_t
{
	// Name of level
	CString mapname;
	// save file linked to connection
	CString mapsavename;

	// List of connections on this level
	CLinkedList<sv_level_connection_t> connectionslist;
};

struct sv_sound_t
{
	sv_sound_t():
		sv_index(NO_PRECACHE),
		duration(0)
		{}

	// File path
	CString filepath;
	// Server-side index
	Int32 sv_index;

	// sound file length
	Float duration;
};

struct sv_model_t
{
	sv_model_t():
		cache_index(NO_PRECACHE)
		{};

	// Name of the model
	CString modelname;
	// Index in cache
	Int32 cache_index;
};

struct sv_particlecache_t
{
	sv_particlecache_t():
		type(PART_SCRIPT_SYSTEM)
		{}

	CString scriptpath;
	part_script_type_t type;
};

// Datatype for uermsg read function on server(needs edict ptr)
typedef bool (*pfnSVUserMsg_t)( edict_t* pclient, const Char* pstrName, const byte* pdata, Uint32 msgsize );

struct sv_usermsgfunction_t
{
	sv_usermsgfunction_t():
		id(0),
		pfnReadMsg(nullptr)
		{}

	// Message name
	CString name;
	// Server-side ID
	Uint32 id;

	// pointer to function
	pfnSVUserMsg_t pfnReadMsg;
};

struct sv_netinfo_t
{
	sv_netinfo_t():
		pnet(nullptr),
		pcurrentmsg(nullptr)
		{}

	// Pointer to networking object
	CNetworking* pnet;
	// Message reader
	CMSGReader reader;

	// Files that must be consistent
	CArray<CString> enforcedfiles;

	// User message functions array
	CArray<sv_usermsgfunction_t> usermsgfunctions;

	// usermsg array
	CArray<usermsg_t> usermsgs;
	// current usermsg data
	usermsgdata_t* pcurrentmsg;
};

struct stringbuffer_t
{
	stringbuffer_t():
		numstrings(0)
		{}

	// Buffer holding the strings
	CArray<const CString*> buffer;
	// Number of strings in buffer
	Uint32 numstrings;
	// Name->position map
	std::unordered_map<CString, Uint32> stringposmap;
};

struct saveddecal_t
{
	saveddecal_t():
		pedict(nullptr),
		identifier(0),
		flags(0)
		{}

	Vector origin;
	Vector normal;

	CString decaltexture;

	edict_t* pedict;
	Uint32 identifier;

	Int32 flags;
};

struct serverstate_t
{
	serverstate_t():
		time(0),
		frametime(0),
		maxclients(0),
		phostclient(nullptr),
		plastspawnentity(nullptr),
		pmoveplayerindex(0),
		numpmovetraces(0),
		pdllhandle(nullptr),
		numareanodes(0),
		pviewentity(nullptr),
		serverstate(SV_INACTIVE),
		haltserver(false),
		paused(false),
		pauseovveride(false),
		saverestore(false),
		pvisbuffer(nullptr),
		ppasbuffer(nullptr),
		pvischeckbuffer(nullptr),
		perrorsprite(nullptr),
		perrormodel(nullptr),
		loadbegintime(0),
		lastpvschecktime(0),
		lastpvscheckclient(-1)
	{
		for(Uint32 i = 0; i < MAX_MAP_HULLS; i++)
		{
			for(Uint32 j = 0; j < 3; j++)
			{
				player_mins[i][j] = 0;
				player_maxs[i][j] = 0;
			}
		}

		memset(&dllfuncs, 0, sizeof(dllfuncs));
	}
	~serverstate_t()
	{
		if(pvisbuffer)
			delete[] pvisbuffer;

		if(ppasbuffer)
			delete[] ppasbuffer;

		if(pvischeckbuffer)
			delete[] pvischeckbuffer;
	}

	// Level name
	CString mapname;
	// Previous level name
	CString prevmapname;

	// Time since game was spawned
	Double time;
	// Server frametime
	Double frametime;

	// max clients
	Uint32 maxclients;
	// client info array
	sv_client_t clients[MAX_CLIENTS];
	// host client ptr
	sv_client_t* phostclient;
	// last spawn entity
	edict_t* plastspawnentity;

	// player hulls
	Vector player_mins[MAX_MAP_HULLS];
	Vector player_maxs[MAX_MAP_HULLS];

	// player idx currently managed by pmove
	Uint32 pmoveplayerindex;
	// entities touched by player in pmove
	trace_t pmovetraces[MAX_TOUCHENTS];
	// number of entities touched by player in pmove
	Uint32 numpmovetraces;

	// Game DLL handle
	void* pdllhandle;
	// Game DLL export functions array
	CArray<dll_export_t> exports;
	// Game dll functions
	gdll_funcs_t dllfuncs;

	// Array of area nodes
	CArray<areanode_t> areanodes;
	// Number of area nodes used
	Uint32 numareanodes;

	// view entity
	edict_t* pviewentity;

	// Game info structure
	gamevars_t gamevars;

	// Networking related info
	sv_netinfo_t netinfo;

	// server state
	sv_state_t serverstate;

	// server sound cache
	CArray<sv_sound_t> sndcache;
	// Map linking sound names to cache indexes
	CacheNameIndexMap_t sndcachemap;

	// server model cache
	CArray<sv_model_t> modelcache;
	// Map linking model names to cache indexes
	CacheNameIndexMap_t modelcachemap;

	// generic resources list
	CArray<CString> genericsourcesarray;
	// Map linking model names to cache indexes
	CacheNameIndexMap_t genericcachemap;

	// particle scripts list
	CArray<sv_particlecache_t> particlescache;
	// Map linking model names to cache indexes
	CacheNameIndexMap_t particlecachemap;

	// decals precache list
	CArray<decalcache_t> decalcache;
	// Map linking model names to cache indexes
	DecalCacheNameIndexMap_t decalcachemap;

	// association of map textures/material scripts
	CArray<maptexturematerial_t> mapmaterialfiles;
	// Mapping of material names to positions
	CacheNameIndexMap_t mapmaterialfilesnamemap;

	// All level connections known
	CLinkedList<sv_levelinfo_t> levelinfos;

	// String buffer
	stringbuffer_t strbuffer;

	// true if it's the first server frame after loading
	bool haltserver;
	// true if paused
	bool paused;
	// true if player can unpause with pause button
	bool pauseovveride;
	// true if loading from save-restore
	bool saverestore;

	// VIS buffer
	byte* pvisbuffer;
	// PAS buffer
	byte* ppasbuffer;
	// PVS checkbuffer
	byte* pvischeckbuffer;

	// Error sprite
	cache_model_t* perrorsprite;
	// Error model
	cache_model_t* perrormodel;

	// Level change information
	sv_levelchangeinfo_t levelchangeinfo;

	// List of decals to be save/restored
	CLinkedList<saveddecal_t> saveddecalslist;

	// Load begin time
	Double loadbegintime;

	// Last pvs check time
	Double lastpvschecktime;
	// Last PVS check client
	entindex_t lastpvscheckclient;
};

extern serverstate_t svs;

extern CCVar* g_psv_skyname;
extern CCVar* g_psv_skycolor_r;
extern CCVar* g_psv_skycolor_g;
extern CCVar* g_psv_skycolor_b;
extern CCVar* g_psv_skyvec_x;
extern CCVar* g_psv_skyvec_y;
extern CCVar* g_psv_skyvec_z;
extern CCVar* g_psv_maxplayers;
extern CCVar* g_psv_maxspeed;
extern CCVar* g_psv_accelerate;
extern CCVar* g_psv_airaccelerate;
extern CCVar* g_psv_wateraccelerate;
extern CCVar* g_psv_edgefriction;
extern CCVar* g_psv_waterfriction;
extern CCVar* g_psv_waterdist;
extern CCVar* g_psv_skill;
extern CCVar* g_psv_chunksize;
extern CCVar* g_psv_allowdownload;
extern CCVar* g_psv_netdebug;
extern CCVar* g_psv_holdtoduck;

extern bool SV_Init( void );
extern void SV_Shutdown( void );
extern bool SV_SpawnGame( const Char* pstrLevelName, const Char* pstrSaveFile = nullptr, const Char* pstrTransitionSave = nullptr, bool clearLoadingScreen = true );
extern void SV_ClearGame( bool clearloadingscreen = true, bool clearconnections = true );
extern void SV_Frame( void );
extern void SV_Physics( void );
extern bool SV_InitGame( void );

extern void SV_GetPlayerHulls( void );
extern void SV_DropClient( sv_client_t& cl, const Char* pstrReason );
extern void SV_ClearClient( sv_client_t& client );
extern void SV_PrepareClient( Uint32 client_index );
extern void SV_ClientDisconnected( sv_client_t& cl );
extern void SV_EstablishedClientConnection( Uint32 client_index );
extern bool SV_VerifyClient( sv_client_t& cl, const Char* pstrPlayerName );
extern bool SV_SpawnClient( sv_client_t& cl );
extern sv_client_t* SV_GetHostClient( void );
extern edict_t* SV_FindSpawnSpot( void );

extern Int32 SV_PrecacheModel( const Char* pstrFilepath );
extern Int32 SV_PrecacheSound( const Char* pstrFilepath );

extern byte* SV_SetPAS( const Vector& origin );
extern byte* SV_SetPVS( const Vector& origin );

extern void SV_SetGroupMask( Int32 mask, Int32 op );
extern void* SV_FunctionFromName( const Char* pstrName );
extern const Char* SV_NameForFunction( const void* functionPtr );
extern void SV_AddEnforcedConsistencyFile( const Char* pstrFilename );

extern bool SV_IsHostClient( entindex_t entindex );
extern bool SV_IsWorldSpawn( entindex_t entindex );

extern const Char* SV_TraceTexture( Int32 groundentity, const Vector& start, const Vector& end );
extern const en_material_t* SV_GetMapTextureMaterial( const Char* pstrtexturename );

extern const Char* SV_GetString( string_t stringindex );
extern Uint32 SV_AllocString( const Char* pString );

extern void SV_ServerCommand( const Char* pstrCmd );
extern void SV_ClientCommand( edict_t* pclient, const Char* pstrCmd );

extern bool SV_GetBonePositionByName( edict_t* pedict, const Char* pstrbonename, Vector& position );
extern bool SV_GetBonePositionByIndex( edict_t* pedict, Uint32 boneindex, Vector& position );
extern bool SV_GetAttachment( edict_t* pedict, Uint32 index, Vector& position );

extern void SV_PrecacheGeneric( const Char* pstrresourcename );

extern Float SV_GetSoundDuration( const Char* pstrfilename, Uint32 pitch );
extern Float SV_GetWAVFileDuration( const Char* pstrfilename );
extern Float SV_GetOGGFileDuration( const Char* pstrfilename );
extern Uint64 SV_GetModelFrameCount( Int32 modelindex );

extern void SV_AddLevelConnection( const Char* pstrLevelName, const Char* pstrOtherLevelName, const Char* pstrLandmarkName, const Char* pstrMapSaveFileName );
extern void SV_BeginLevelChange( const Char* pstrOtherLevelName, const Char* pstrLandmarkName, const Vector& landmarkPosition );
extern void SV_GetTransitionList( const Int32** pEntityList, Uint32& numEntities );
extern void SV_PerformLevelChange( const Char* pstrlevelname, const Char* pstrlandmarkname );
extern void SV_SetLevelSavefile( const Char* pstrLevelName, const Char* pstrSaveFileName );
extern void SV_ClearLevelChange( void );
extern void SV_ClearConnections( void );

extern void SV_RestoreSavedDecals( void );
extern void SV_AddSavedDecal( const Vector& origin, const Vector& normal, entindex_t entityindex, const Char* pstrDecalTexture, Int32 decalflags );

extern edict_t* SV_FindClientInPVS( const edict_t* pedict );

extern void SV_MaxPlayersCvarCallBack( CCVar* pCVar );
extern void SV_EndGame( const Char* pstrEndGameCode );
#endif