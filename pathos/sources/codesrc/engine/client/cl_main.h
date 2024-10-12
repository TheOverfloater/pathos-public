/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CL_MAIN_H
#define CL_MAIN_H

#include "sv_shared.h"
#include "cldll_interface.h"
#include "cl_entity.h"
#include "usercmd.h"
#include "movevars.h"
#include "constants.h"
#include "pm_shared.h"
#include "filechunk.h"
#include "entity_extrainfo.h"
#include "usermsg.h"
#include "msgreader.h"

struct cl_entity_t;

class Vector;
class CCVar;
class CNetworking;
class CWADTextureResource;

struct trace_interface_t;
struct pm_info_t;
struct file_interface_t;
struct r_interface_t;
struct entity_extrainfo_t;
struct cl_efxapi_t;

extern CCVar* g_pCvarDefaultFOV;
extern CCVar* g_pCvarReferenceFOV;
extern CCVar* g_pCvarName;
extern CCVar* g_pCvarPredictiton;

// Usercmd history allocation size
static constexpr Uint32 USERCMD_HISTORY_ALLOC_SIZE = 1024;
// Extrainfo allocation size
static constexpr Uint32 EXTRAINFO_ALLOC_SIZE = 128;
// Max entity lights
static constexpr Uint32 MAX_ENTITY_LIGHTS = 256;
// Particle blocker alloc size
static constexpr Uint32 PARTICLEBLOCKER_ALLOC_SIZE = 16;

// Server dll path
#ifdef _64BUILD
static const Char CLIENT_DLL_PATH[] = "dlls/client_x64.dll";
#else
static const Char CLIENT_DLL_PATH[] = "dlls/client_x86.dll";
#endif

// Datatype for ClientDLLInit function in the game dll
typedef bool (*pfnClientDLLInit_t)( Uint32 version, cldll_funcs_t& dllFuncs, const trace_interface_t& traceFuncs, const file_interface_t& fileFuncs, const cldll_engfuncs_t& engFuncs, const cl_efxapi_t& efxAPI, const r_interface_t& renderFuncs );

enum clconnstate_t
{
	CLIENT_INACTIVE = 0,	// Not connected to anything
	CLIENT_CONNECTED,		// Connected to a localhost, loading
	CLIENT_ACTIVE			// Can render, interact, simulate physics
};

struct clientinfo_t
{
	clientinfo_t():
		entindex(0)
	{
	}

	// Client's view angles
	Vector viewangles;
	// Client's entindex
	entindex_t entindex;
};

struct cl_resource_t
{
	cl_resource_t():
		type(RS_TYPE_UNDEFINED),
		fileid(0),
		svindex(-1),
		missing(false)
		{}

	// File path
	CString filepath;
	// Type of resource
	rs_type_t type;
	// Unique file ID
	Int32 fileid;
	// Server-side index for sounds
	Int32 svindex;
	// TRUE if missing
	bool missing;
};

struct cl_download_t
{
	cl_download_t():
		presource(nullptr)
		{
		}

	// Pointer to resource object
	cl_resource_t* presource;

	// Linked list of received chunks
	CLinkedList<filechunk_t*> filechunks;
	// Bits for chunk completion
	CBitSet chunkbits;
};

// Datatype for uermsg read function on client(doesn't need edict ptr)
typedef bool (*pfnCLUserMsg_t)( const Char* pstrName, const byte* pdata, Uint32 msgsize );

struct cl_usermsgfunction_t
{
	cl_usermsgfunction_t():
		id(0),
		pfnReadMsg(nullptr)
		{}

	// Message name
	CString name;
	// Server-side ID
	Uint32 id;

	// pointer to function
	pfnCLUserMsg_t pfnReadMsg;
};

struct cl_net_t
{
	cl_net_t():
		connecting(false),
		numretries(0),
		nextretrytime(0),
		nummissingresources(0),
		numdownloadedresources(0),
		pnet(nullptr)
		{}

	// TRUE if we're attempting to connect
	bool connecting;
	// Number of times we tried to connect
	Int32 numretries;
	// Next time until we retry
	Float nextretrytime;

	// Holds any active downloads
	cl_download_t download;

	// User message functions array
	CArray<cl_usermsgfunction_t> usermsgfunctions;

	// List of resources
	CLinkedList<cl_resource_t> resourcestlist;
	// Number of missing resources
	Uint32 nummissingresources;
	// Number of downloaded resources
	Uint32 numdownloadedresources;

	// Decal cache array
	CArray<decalcache_t> decalcache;

	// usermsg array
	CArray<usermsg_t> usermsgs;
	// current usermsg data
	usermsgdata_t msgdata;

	// Networking class pointer
	CNetworking* pnet;

	// Message reader
	CMSGReader reader;
};

struct entitylight_t
{
	entitylight_t():
		die(0),
		key(0),
		attachment(-1)
		{}

	Double die;
	Int32 key;
	Int32 attachment;

	mlight_t mlight;
};

struct clientstate_t
{
	clientstate_t():
		clientindex(0),
		cl_state(CLIENT_INACTIVE),
		hasclientdata(false),
		hasentitydata(false),
		hasallresources(false),
		hasserverdata(false),
		hasplayerentitydata(false),
		cl_pingtime(0),
		cl_time(0),
		cl_clsvtime(0),
		frametime(0),
		parsecount(0),
		framecount(0),
		numentities(0),
		numextrainfos(0),
		maxclients(0),
		curusercmdidx(0),
		usercmdhistorynum(0),
		pdllhandle(nullptr),
		paused(false)
	{
		memset(&dllfuncs, 0, sizeof(dllfuncs));
	}

	// client index on server
	Uint32 clientindex;
	// Client's state
	clconnstate_t cl_state;
	// true if we got any client data
	bool hasclientdata;
	// true if we recieved packet entities
	bool hasentitydata;
	// true if we got all resources in
	bool hasallresources;
	// true if we got the server data
	bool hasserverdata;
	// TRUe if we got player entity data
	bool hasplayerentitydata;

	// Time we pinged the server
	Double cl_pingtime;
	// Current time
	Double cl_time;
	// Server client time
	Double cl_clsvtime;
	// frametime
	Double frametime;
	// Parse count for client
	Uint64 parsecount;
	// Frame count for client processing
	Uint64 framecount;

	// entities array
	CArray<cl_entity_t> entities;
	// Max number of entities on client
	Int32 numentities;

	// Particle blocker entities
	CArray<entindex_t> particleblockers;
	// Number of particle blockers
	Int32 numparticleblockers;

	// extrainfos for entities
	CArray<entity_extrainfo_t*> entityextrainfos;
	// Number of extrainfos in use
	Uint32 numextrainfos;

	// Client related data
	clientinfo_t clinfo;

	// Networking related data
	cl_net_t netinfo;

	// Max clients on server
	Uint32 maxclients;

	// Last sent usercmd index
	Uint64 curusercmdidx;
	// Last usercmd sent to server
	usercmd_t cmd;
	// Usercmd history
	CArray<usercmd_t> usercmdhistory;
	// Number of kept usercmds
	Uint32 usercmdhistorynum;

	// Client DLL handle
	void* pdllhandle;
	// Game dll functions
	cldll_funcs_t dllfuncs;

	// Skybox texture name
	CString skyname;
	// Sky vector and color
	Vector skyvec;
	Vector skycolor;

	// playermove info
	pm_info_t pminfo;

	// association of map textures/material scripts
	CArray<maptexturematerial_t> mapmaterialfiles;

	// Entity lights
	entitylight_t entitylights[MAX_ENTITY_LIGHTS];

	// true if paused
	bool paused;
};

extern clientstate_t cls;
extern CCVar* g_pCvarDefaultFOV;
extern CCVar* g_pCvarName;

extern bool CL_Init( void );
extern void CL_Shutdown( void );
extern void CL_Frame( void );
extern void CL_UpdateSound( void );
extern void CL_SendCmd( void );
extern bool CL_InitGame( void );

extern bool CL_EstablishConnection( const Char* pstrhost, bool reconnect = false );
extern void CL_Disconnect( bool clearserver = true, bool clearloadingplaque = true );
extern void CL_InitCommands( void );

extern bool CL_IsGameActive( void );
extern bool CL_CanPlayGameSounds( void );
extern bool CL_IsHostClient( void );
extern bool CL_CheckGameReady( void );

extern void CL_CleanUserCmdHistory( Uint64 lastsvusercmdindex );
extern void CL_SendHeartbeat( bool prompt );

extern entity_extrainfo_t* CL_GetEntityExtraData( cl_entity_t* pentity );
extern void CL_InitExtraInfos( void );
extern void CL_ClearExtraInfos( void );
extern void CL_ResetLighting( void );
extern bool CL_AddTempEntity( cl_entity_t *entity );
extern void CL_ClearEntities( void );

extern void CL_ServerCommand( const Char* pstrCommand );
extern void CL_ClientCommand( const Char* pstrCommand );

extern void CL_PrecacheFlexScript( enum flextypes_t npctype, const Char* pstrscript );
extern void CL_SetFlexScript( entindex_t entindex, const Char* pstrscript );
extern void CL_LinkMapTextureMaterials( CArray<CString>& wadList );
extern void CL_UpdateAttachments( cl_entity_t* pentity );

extern Uint32 CL_GetMaxClients( void );
extern void CL_UpdateEntityLights( void );
extern void CL_UpdateParentedEntities( void );
extern void CL_NotifyLevelChange( void );

extern void CL_SetPaused( bool isPaused, bool pauseOveride );
#endif