/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "edict.h"
#include "sv_main.h"
#include "sv_entities.h"
#include "system.h"
#include "sv_world.h"
#include "sv_physics.h"
#include "common.h"
#include "sv_msg.h"
#include "sv_move.h"

#include "texturemanager.h"
#include "brushmodel.h"
#include "modelcache.h"
#include "enginestate.h"
#include "edictmanager.h"
#include "gdll_interface.h"
#include "networking.h"
#include "cl_main.h"
#include "commands.h"
#include "console.h"
#include "file.h"

#include "vid.h"
#include "trace_shared.h"
#include "save_shared.h"
#include "saverestore.h"
#include "constants.h"
#include "file_interface.h"
#include "vbm_shared.h"
#include "vbmtrace.h"
#include "r_wadtextures.h"
#include "snd_shared.h"
#include "enginefuncs.h"
#include "r_main.h"
#include "ogg_common.h"
#include "sv_msg.h"

#ifdef WIN32
#include <detours.h>
#endif

// Datatype for GameDLLInit function in the game dll
typedef bool (*pfnGameDLLInit_t)( Uint32 version, gdll_funcs_t& dllFuncs, const gdll_engfuncs_t& engFuncs, const trace_interface_t& traceFuncs, const file_interface_t& fileFuncs, gamevars_t& gamevars );

// String buffer allocation size
static constexpr Uint32 STRINGBUFFER_ALLOC_SIZE = 512;

// Server state object
serverstate_t svs;

// cvars used by server
CCVar* g_psv_skyname = nullptr;
CCVar* g_psv_skycolor_r = nullptr;
CCVar* g_psv_skycolor_g = nullptr;
CCVar* g_psv_skycolor_b = nullptr;
CCVar* g_psv_skyvec_x = nullptr;
CCVar* g_psv_skyvec_y = nullptr;
CCVar* g_psv_skyvec_z = nullptr;
CCVar* g_psv_maxplayers = nullptr;
CCVar* g_psv_maxspeed = nullptr;
CCVar* g_psv_accelerate = nullptr;
CCVar* g_psv_airaccelerate = nullptr;
CCVar* g_psv_wateraccelerate = nullptr;
CCVar* g_psv_edgefriction = nullptr;
CCVar* g_psv_waterfriction = nullptr;
CCVar* g_psv_waterdist = nullptr;
CCVar* g_psv_skill = nullptr;
CCVar* g_psv_chunksize = nullptr;
CCVar* g_psv_allowdownload = nullptr;
CCVar* g_psv_netdebug = nullptr;
CCVar* g_psv_holdtoduck = nullptr;
CCVar* g_psv_autoaim = nullptr;

//
// Engine functions for game dll
//
static gdll_engfuncs_t GAMEDLL_ENGINE_FUNCTIONS =
{
	SV_PrecacheModel,					//pfnPrecacheModel
	SV_PrecacheSound,					//pfnPrecacheSound
	SV_PrecacheGeneric,					//pfnPrecacheGeneric
	SV_PrecacheParticleScript,			//pfnPrecacheParticleScript
	SV_PrecacheDecal,					//pfnPrecacheDecal
	SV_PrecacheDecalGroup,				//pfnPrecacheDecalGroup
	SV_SetModel,						//pfnSetModel
	SV_SetMinsMaxs,						//pfnSetMinsMaxs
	SV_SetSize,							//pfnSetSize
	SV_SetOrigin,						//pfnSetOrigin
	SV_SetPAS,							//pfnSetPAS
	SV_SetPVS,							//pfnSetPVS
	Con_Printf,							//pfnCon_Printf
	Con_DPrintf,						//pfnCon_DPrintf
	Con_VPrintf,						//pfnCon_VPrintf
	Con_EPrintf,						//pfnCon_EPrintf
	SV_ClientPrintf,					//pfnClientPrintf
	SV_GetNumEdicts,					//pfnGetNbEdicts
	SV_GetEdictByIndex,					//pfnGetEdictByIndex
	SV_CreateEntity,					//pfnCreateEntity
	SV_RemoveEntity,					//pfnRemoveEntity
	SV_DropToFloor,						//pfnDropToFloor
	Cache_GetModel,						//pfnGetModel
	Cache_GetModelBounds,				//pfnGetModelBounds
	Cache_GetModelType,					//pfnGetModelType
	Cache_GetNbModels,					//pfnGetNbCachedModels
	Cache_GetModelByName,				//pfnGetModelByName
	SV_AddToTouched,					//pfnAddToTouched
	Mod_PointInLeaf,					//pfnPointInLeaf
	Mod_LeafPVS,						//pfnLeafPVS
	SV_SetGroupMask,					//pfnSetGroupMask
	SV_PlayEntitySound,					//pfnPlayEntitySound
	SV_PlayAmbientSound,				//pfnPlayAmbientSound
	SV_ApplySoundEffect,				//pfnApplySoundEffect
	SV_StopEntitySounds,				//pfnStopEntitySounds
	SV_MuteAllSounds,					//pfnMuteAllSounds
	SV_SetRoomType,						//pfnSetRoomType
	SV_PlayMusic,						//pfnPlayMusic
	SV_StopMusic,						//pfnStopMusic
	SV_NameForFunction,					//pfnNameForFunction
	SV_FunctionFromName,				//pfnFunctionFromName
	Engine_GetMaterialScript,			//pfnGetMaterialScript
	SV_GetMapTextureMaterial,			//pfnGetMapTextureMaterialScript
	SV_AllocString,						//pfnAllocString
	SV_GetString,						//pfnGetString
	Save_WriteBool,						//pfnSaveWriteBool
	Save_WriteByte,						//pfnSaveWriteByte
	Save_WriteBitset,					//pfnSaveWriteBitset
	Save_WriteChar,						//pfnSaveWriteChar
	Save_WriteInt16,					//pfnSaveWriteInt16
	Save_WriteUint16,					//pfnSaveWriteUint16
	Save_WriteInt32,					//pfnSaveWriteInt32
	Save_WriteUint32,					//pfnSaveWriteUint32
	Save_WriteInt64,					//pfnSaveWriteInt64
	Save_WriteUint64,					//pfnSaveWriteUint64
	Save_WriteFloat,					//pfnSaveWriteFloat
	Save_WriteDouble,					//pfnSaveWriteDouble
	Save_WriteTime,						//pfnSaveWriteTime
	Save_WriteString,					//pfnSaveWriteString
	Save_WriteRawString,				//pfnSaveWriteRawString
	Save_WriteVector,					//pfnSaveWriteVector
	Save_WriteCoord,					//pfnSaveWriteCoord
	Save_WriteEntindex,					//pfnSaveWriteEntindex
	Save_WriteGlobalState,				//pfnSaveWriteGlobalState
	SV_RegisterUserMessage,				//pfnRegisterUserMessage
	SV_UserMessageBegin,				//pfnUserMessageBegin
	SV_UserMessageEnd,					//pfnUserMessageEnd
	SV_Msg_WriteByte,					//pfnMsgWriteByte
	SV_Msg_WriteChar,					//pfnMsgWriteChar
	SV_Msg_WriteInt16,					//pfnMsgWriteInt16
	SV_Msg_WriteUint16,					//pfnMsgWriteUint16
	SV_Msg_WriteInt32,					//pfnMsgWriteInt32
	SV_Msg_WriteUint32,					//pfnMsgWriteUint32
	SV_Msg_WriteInt64,					//pfnMsgWriteInt64
	SV_Msg_WriteUint64,					//pfnMsgWriteUint64
	SV_Msg_WriteSmallFloat,				//pfnMsgWriteSmallFloat
	SV_Msg_WriteFloat,					//pfnMsgWriteFloat
	SV_Msg_WriteDouble,					//pfnMsgWriteDouble
	SV_Msg_WriteBuffer,					//pfnMsgWriteBuffer
	SV_Msg_WriteString,					//pfnMsgWriteString
	SV_Msg_WriteEntindex,				//pfnMsgWriteEntindex
	SV_Msg_WriteBitSet,					//pfnMsgWriteBitSet
	SV_ServerCommand,					//pfnServerCommand
	SV_ClientCommand,					//pfnClientCommand
	Engine_CreateCommand,				//pfnCreateCommand
	SV_GetInvokerPlayer,				//pfnGetInvokerPlayer
	SV_GetBonePositionByName,			//pfnGetBonePositionByName
	SV_GetBonePositionByIndex,			//pfnGetBonePositionByIndex
	SV_GetAttachment,					//pfnGetAttachment
	Mod_RecursiveLightPoint,			//pfnRecursiveLightPoint
	Mod_RecursiveLightPoint_BumpData,	//pfnRecursiveLightPointBumpData
	Engine_CreateCVar,					//pfnCreateCVar
	Engine_CreateCVarCallback,			//pfnCreateCVarCallback
	Engine_GetCVarPointer,				//pfnGetCVarPointer
	Engine_SetCVarFloat,				//pfnSetCVarFloat
	Engine_SetCVarString,				//pfnSetCVarString
	Engine_GetCvarFloatValue,			//pfnGetCvarFloatValue
	Engine_GetCvarStringValue,			//pfnGetCvarStringValue
	Engine_Cmd_Argc,					//pfnCmd_Argc
	Engine_Cmd_Argv,					//pfnCmd_Argv
	SV_GetSoundDuration,				//pfnGetSoundDuration
	Engine_GetVISBufferSize,			//pfnGetVISBufferSize
	SV_GetModelFrameCount,				//pfnGetModelFrameCount
	SV_AddLevelConnection,				//pfnAddLevelConnection
	SV_BeginLevelChange,				//pfnBeginLevelChange
	SV_GetTransitionList,				//pfnGetTransitionList
	SV_AddSavedDecal,					//pfnAddSavedDecal
	SV_NPC_WalkMove,					//pfnWalkMove
	SV_NPC_MoveToOrigin,				//pfnMoveToOrigin
	SV_CheckBottom,						//pfnCheckBottom
	SV_FindClientInPVS,					//pfnFindClientInPVS
	SV_EndGame,							//pfnEndGame
	Sys_FloatTime,						//pfnFloatTime
};

//
// Game DLL traceline functions
//
trace_interface_t GAMEDLL_TRACE_FUNCTIONS =
{
	SV_TestPlayerPosition,		//pfnTestPlayerPosition
	SV_PointContents,			//pfnPointContents
	SV_TruePointContents,		//pfnTruePointContents
	TR_HullPointContents,		//pfnHullPointContents
	SV_PlayerTrace,				//pfnPlayerTrace
	SV_TraceLine,				//pfnTraceLine
	SV_HullForBSP,				//pfnHullForBSP
	SV_TraceModel,				//pfnTraceModel
	SV_TraceTexture,			//pfnTraceTexture
};

//=============================================
//
//=============================================
bool SV_Init( void )
{
	// Try to load the game dll
	CString fulldllpath;
	fulldllpath << ens.gamedir << PATH_SLASH_CHAR << SERVER_DLL_PATH;

	svs.pdllhandle = SDL_LoadObject(fulldllpath.c_str());
	if(!svs.pdllhandle)
	{
		Sys_ErrorPopup("Failed to load '%s'.\n", fulldllpath.c_str());
		return false;
	}

	// Retreive the export functions
#ifdef _64BUILD
	if(!Sys_GetDLLExports(SERVER_DLL_NAME, svs.pdllhandle, svs.exports))
#else
	if(!Sys_GetDLLExports(fulldllpath.c_str(), svs.pdllhandle, svs.exports))
#endif
	{
		Sys_ErrorPopup("Failed to get exports for '%s'.\n", SERVER_DLL_PATH);
		return false;
	}

	// Init the gamedll interface
	pfnGameDLLInit_t pfnGDLLInit = static_cast<pfnGameDLLInit_t>(SDL_LoadFunction(svs.pdllhandle, "GameDLL_Init"));
	if(!pfnGDLLInit)
	{
		Sys_ErrorPopup("Failed to hook 'GameDLL_Init' in game dll.\n");
		return false;
	}

	// Init the game dll-engine interface
	if(!pfnGDLLInit(GDLL_INTERFACE_VERSION, svs.dllfuncs, GAMEDLL_ENGINE_FUNCTIONS, GAMEDLL_TRACE_FUNCTIONS, FL_GetInterface(), svs.gamevars))
	{
		Sys_ErrorPopup("Failed to init game dll.\n");
		return false;
	}

	// Init cvars
	g_psv_skyname = gConsole.CreateCVar(CVAR_STRING, FL_CV_SV_ONLY, "sv_skyname", DEFAULT_SKYBOX_NAME, "Skybox used");
	g_psv_skycolor_r = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, "sv_skycolor_r", "0", "Sky color R component");
	g_psv_skycolor_g = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, "sv_skycolor_g", "0", "Sky color G component");
	g_psv_skycolor_b = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, "sv_skycolor_b", "0", "Sky color B component");
	g_psv_skyvec_x = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, "sv_skyvec_x", "0", "Sky vector X component");
	g_psv_skyvec_y = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, "sv_skyvec_y", "0", "Sky vector Y component");
	g_psv_skyvec_z = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, "sv_skyvec_z", "0", "Sky vector Z component");
	g_psv_maxplayers = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_maxplayers", "1", "Max players", SV_MaxPlayersCvarCallBack);
	g_psv_maxspeed = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_maxspeed", "320", "Max player speed");
	g_psv_accelerate = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_accelerate", "10", "Acceleration speed");
	g_psv_airaccelerate = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_airaccelerate", "10", "Air acceleration speed");
	g_psv_wateraccelerate = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_wateraccelerate", "10", "Water acceleration speed");
	g_psv_edgefriction = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_edgefriction", "2", "Edge friction");
	g_psv_waterfriction = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_waterfriction", "1", "Water friction");
	g_psv_waterdist = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_waterdist", "8", "Water distance to view");
	g_psv_skill = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_skill", "1", "Difficulty setting");
	g_psv_chunksize = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_max_chunksize", "524288", "Max chunk size for uploads", SV_ChunkSizeCvarCallBack);
	g_psv_allowdownload = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_allowdownload", "1", "Controls whether downloads for clients are supported");
	g_psv_netdebug =  gConsole.CreateCVar(CVAR_FLOAT, FL_CV_NONE, "sv_netdebug", "0", "Enable debug prints for net data");
	g_psv_holdtoduck =  gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_SAVE), "sv_holdtoduck", "0", "Controls whether ducking is toggled or to-hold");
	g_psv_autoaim =  gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_SAVE), AUTOAIM_CVAR_NAME, "0", "Controls whether auto-aiming is enabled");

	// Initialize physics
	SV_Physics_Init();

	// Call game dll to initialize after creating cvars
	if(!svs.dllfuncs.pfnGameDLLInit())
	{
		Sys_ErrorPopup("Game dll initialization failed.\n");
		return false;
	}

	// Get hull sizes
	SV_GetPlayerHulls();

	// Create vis buffer
	if(!svs.pvisbuffer)
	{
		svs.pvisbuffer = new byte[ens.visbuffersize];
		memset(svs.pvisbuffer, 0, sizeof(byte)*ens.visbuffersize);
	}

	// Create PAS buffer
	if(!svs.ppasbuffer)
	{
		svs.ppasbuffer = new byte[ens.visbuffersize];
		memset(svs.ppasbuffer, 0, sizeof(byte)*ens.visbuffersize);
	}

	// Create buffer for vis checks
	if(!svs.pvischeckbuffer)
	{
		svs.pvischeckbuffer = new byte[ens.visbuffersize];
		memset(svs.pvischeckbuffer, 0, sizeof(byte)*ens.visbuffersize);
	}

	return true;
}

//=============================================
//
//=============================================
void SV_Shutdown( void )
{
	// Clear game if we're active
	SV_ClearGame();

	// Call shutdown on game dll
	svs.dllfuncs.pfnGameDLLShutdown();

	// Unload the game dll
	if(svs.pdllhandle != nullptr)
	{
		SDL_UnloadObject(svs.pdllhandle);
		svs.pdllhandle = nullptr;
	}
}

//=============================================
//
//=============================================
void SV_GetPlayerHulls( void )
{
	for(Int32 i = 0; i < MAX_MAP_HULLS; i++)
		svs.dllfuncs.pfnGetHullSizes(i, svs.player_mins[i], svs.player_maxs[i]);
}

//=============================================
//
//=============================================
edict_t* SV_FindSpawnSpot( void )
{
	// SP is handled specially
	if(svs.maxclients == 1)
	{
		edict_t* pstartentity = svs.dllfuncs.pfnFindEntityByString(nullptr, "classname", "info_player_start");
		if(!pstartentity)
		{
			Con_Printf("Warning: No info_player_start entity in level.\n");
			return nullptr;
		}

		return pstartentity;
	}

	// Seek specific spawn for mp
	edict_t* pspawn = nullptr;
	CString spawnEntity = "info_player_deathmatch";

	// Make sure the next one is valid
	if(svs.plastspawnentity)
	{
		pspawn = svs.dllfuncs.pfnFindEntityByString(svs.plastspawnentity, "classname", spawnEntity.c_str());

		// Reset if we reached the end
		if(!pspawn)
			svs.plastspawnentity = nullptr;
	}

	// Try again from the beginning
	if(!pspawn)
	{
		pspawn = svs.dllfuncs.pfnFindEntityByString(svs.plastspawnentity, "classname", spawnEntity.c_str());
		if(!pspawn)
		{
			Con_Printf("Warning: No %s entity in level.\n", spawnEntity.c_str());
			return nullptr;
		}
	}

	// Remember last spawn point
	svs.plastspawnentity = pspawn;
	return pspawn;
}

//=============================================
//
//=============================================
void SV_LinkMapTextureMaterials( CArray<CString>& wadList )
{
	if(!svs.mapmaterialfiles.empty())
		svs.mapmaterialfiles.clear();

	for(Uint32 i = 0; i < ens.pworld->numtextures; i++)
	{
		// First look in the BSP folder
		CString filepath;
		CString basename;
		Common::Basename(ens.pworld->name.c_str(), basename);

		filepath << WORLD_TEXTURES_PATH_BASE << basename << PATH_SLASH_CHAR << ens.pworld->ptextures[i].name << PMF_FORMAT_EXTENSION;
		
		CString fullpath;
		fullpath << TEXTURE_BASE_DIRECTORY_PATH << filepath;

		// Find the material script. Use FL_FileExists, as textures
		// are not loaded at this point
		if(!FL_FileExists(fullpath.c_str()))
		{
			filepath.clear();

			for(Uint32 j = 0; j < wadList.size(); j++)
			{
				Common::Basename(wadList[j].c_str(), basename);

				filepath << WORLD_TEXTURES_PATH_BASE << basename << PATH_SLASH_CHAR << ens.pworld->ptextures[i].name << PMF_FORMAT_EXTENSION;
				fullpath.clear();
				fullpath << TEXTURE_BASE_DIRECTORY_PATH << filepath;

				if(FL_FileExists(fullpath.c_str()))
					break;

				filepath.clear();
			}
		}

		// Add to the association list
		if(!filepath.empty())
		{
			maptexturematerial_t newmat;
			newmat.maptexturename = ens.pworld->ptextures[i].name;
			newmat.materialfilepath = filepath;
			svs.mapmaterialfiles.push_back(newmat);
		}
		else
			Con_Printf("%s - Failed to find material file for world texture '%s'.\n", __FUNCTION__, ens.pworld->ptextures[i].name.c_str());
	}
}

//=============================================
//
//=============================================
void SV_ClearNetworking( void )
{
	if(!svs.netinfo.pnet)
		return;

	CNetworking::DeleteInstance();
	svs.netinfo.pnet = nullptr;
}

//=============================================
//
//=============================================
bool SV_RestoreTransitionSave( const save_header_t* psaveheader )
{
	// Make sure spawn functions on clients are not called
	svs.saverestore = true;

	// Load connections from save file
	gSaveRestore.LoadConnectionsFromSaveFile(psaveheader);

	// Make sure it's present
	sv_levelinfo_t* plevelinfo = nullptr;

	svs.levelinfos.begin();
	while(!svs.levelinfos.end())
	{
		sv_levelinfo_t& levelinfo = svs.levelinfos.get();
		if(!qstrcmp(levelinfo.mapname, svs.mapname))
		{
			plevelinfo = &levelinfo;
			break;
		}

		svs.levelinfos.next();
	}

	// Fail if level info was not found
	if(!plevelinfo)
	{
		Con_EPrintf("%s - Information for level '%s' not found.\n", __FUNCTION__, svs.levelchangeinfo.nextlevelname.c_str());
		svs.saverestore = false;
		return false;
	}

	// Look to see if the connection is present already
	sv_level_connection_t* pconnection = nullptr;
	plevelinfo->connectionslist.begin();
	while(!plevelinfo->connectionslist.end())
	{
		sv_level_connection_t& conn = plevelinfo->connectionslist.get();
		if(!qstrcmp(conn.othermapname, psaveheader->mapname) 
			&& !qstrcmp(conn.landmarkname, svs.levelchangeinfo.landmarkname))
		{
			pconnection = &conn;
			break;
		}

		plevelinfo->connectionslist.next();
	}

	if(!pconnection)
	{
		Con_EPrintf("%s - Couldn't find connection on destination level '%s' with landmark '%s'.\n", 
			__FUNCTION__, svs.levelchangeinfo.nextlevelname.c_str(), svs.levelchangeinfo.landmarkname.c_str());
		return false;
	}

	// Find landmark locally
	edict_t* plandmarkentity = nullptr;
	while(true)
	{
		plandmarkentity = svs.dllfuncs.pfnFindEntityByString(plandmarkentity, "targetname", pconnection->landmarkname.c_str());
		if(!plandmarkentity)
			break;

		if(!qstrcmp(SV_GetString(plandmarkentity->fields.classname), "info_landmark"))
			break;
	}

	if(!plandmarkentity)
	{
		Con_EPrintf("%s - Failed to find landmark entity '%s'.\n", __FUNCTION__, pconnection->landmarkname.c_str());
		svs.saverestore = false;
		return false;
	}

	// Load entities from the save file
	if(!gSaveRestore.LoadSaveData(psaveheader, &plandmarkentity->state.origin))
	{
		Con_EPrintf("%s - Failed to parse entities from save file '%s'.\n", __FUNCTION__, psaveheader->name);
		svs.saverestore = false;
		return false;
	}

	gSaveRestore.FreeSaveFile(reinterpret_cast<const byte*>(psaveheader));
	return true;
}

//=============================================
//
//=============================================
void SV_MaxPlayersCvarCallBack( CCVar* pCVar )
{
	Int32 maxPlayers = pCVar->GetValue();
	if(maxPlayers >= MAX_PLAYERS)
	{
		Con_Printf("Warning: %s - Only 32 max players allowed.\n", __FUNCTION__, g_psv_maxplayers->GetName());
		gConsole.CVarSetFloatValue(g_psv_maxplayers->GetName(), MAX_PLAYERS);
	}
	else if(maxPlayers < 1)
	{
		Con_Printf("Warning: %s - Invalid value %d.\n", __FUNCTION__, g_psv_maxplayers->GetName(), maxPlayers);
		gConsole.CVarSetFloatValue(g_psv_maxplayers->GetName(), 1);
	}
}

//=============================================
//
//=============================================
bool SV_SpawnGame( const Char* pstrLevelName, const Char* pstrSaveFile, const Char* pstrTransitionSave, bool clearLoadingScreen )
{
	if(!ens.isinitialized)
	{
		Con_EPrintf("%s - Attempted to spawn game before engine was initialized.\n", __FUNCTION__, pstrSaveFile);
		return false;
	}

	if(!pstrLevelName && !pstrSaveFile)
	{
		Con_EPrintf("%s - No level or save file specified.\n", __FUNCTION__, pstrSaveFile);
		return false;
	}

	// If spawning from a save file, make sure it exists before doing any major work
	if(pstrSaveFile && !FL_FileExists(pstrSaveFile))
	{
		Con_EPrintf("%s - Save file '%s' does not exist.\n", __FUNCTION__, pstrSaveFile);
		return false;
	}

	// If transition save was specified, make sure it exists before doing any major work
	if(pstrTransitionSave && !FL_FileExists(pstrTransitionSave))
	{
		Con_EPrintf("%s - Transition save file '%s' does not exist.\n", __FUNCTION__, pstrTransitionSave);
		return false;
	}

	// Set load begin time
	svs.loadbegintime = Sys_FloatTime();
	svs.saverestore = false;

	// Clear any data
	SV_ClearGame(clearLoadingScreen);

	// Set global variable
	svs.maxclients = g_psv_maxplayers->GetValue();
	if(svs.maxclients < 0 || svs.maxclients > MAX_PLAYERS)
	{
		Con_EPrintf("%s - Invalid %s setting %d.\n", __FUNCTION__, g_psv_maxplayers->GetName(), static_cast<Int32>(g_psv_maxplayers->GetValue()));
		return false;
	}

	// Draw loading screen before clearing
	if(clearLoadingScreen)
		VID_BeginLoading(true);

	svs.netinfo.pnet = CNetworking::CreateInstance((svs.maxclients > 1) ? true : false);
	if(!svs.netinfo.pnet->Init("localhost"))
	{
		Con_EPrintf("%s - Failed to initialize net.\n", __FUNCTION__);
		SV_ClearNetworking();
		VID_EndLoading();
		return false;
	}

	// Get level name from save file
	const save_header_t* psaveheader = nullptr;
	if(pstrSaveFile)
	{
		psaveheader = reinterpret_cast<const save_header_t*>(gSaveRestore.LoadSaveFile(pstrSaveFile));
		if(!psaveheader)
		{
			SV_ClearNetworking();
			VID_EndLoading();
			return false;
		}

		Common::Basename(psaveheader->mapname, svs.mapname);
	}
	else
	{
		// Set from the parameter
		Common::Basename(pstrLevelName, svs.mapname);
	}

	// Set game state as loading
	ens.gamestate = GAME_LOADING;
	// Set before establishing connection
	svs.serverstate = SV_ACTIVE;
	// Set level name in gamevars too
	svs.gamevars.levelname = svs.mapname;
	// Set current time at 1 seconds,
	// so all timers on entities will be correct
	svs.time = svs.gamevars.time = 1.0;

	// Set paused state
	Sys_SetPaused(true, false);

	// Allocate edicts
	gEdicts.AllocEdicts();

	// Try to load the file
	CString filepath;
	filepath << "maps/" << svs.mapname;
	if(!qstrstr(filepath.c_str(), ".bsp"))
		filepath << ".bsp";

	// Print message
	CString message;
	message << "Loading " << filepath;
	VID_DrawLoadingScreen(message.c_str());

	// Load the world model
	cache_model_t* pworldmodel = gModelCache.LoadModel(filepath.c_str());
	if(!pworldmodel)
	{
		Con_EPrintf("%s - Could not load %s.\n", __FUNCTION__, filepath.c_str());
		SV_ClearGame(clearLoadingScreen);
		return false;
	}

	// Set the world
	ens.pworld = pworldmodel->getBrushmodel();

	// Add all brush models to the server cache
	for(Uint32 i = 0; i < gModelCache.GetNbCachedModels(); i++)
	{
		cache_model_t* pcache = gModelCache.GetModelByIndex(i+1);

		sv_model_t newmodel;
		newmodel.modelname = pcache->name;
		newmodel.cache_index = i+1;

		svs.modelcache.push_back(newmodel);
	}
	
	// Precache error model and sprite
	Int32 index = SV_PrecacheModel(ERROR_MODEL_FILENAME);
	if(index != NO_PRECACHE)
		svs.perrormodel = gModelCache.GetModelByIndex(index);

	index = SV_PrecacheModel(ERROR_SPRITE_FILENAME);
	if(index != NO_PRECACHE)
		svs.perrorsprite = gModelCache.GetModelByIndex(index);

	// Set areanode array size
	svs.areanodes.resize(MAX_AREA_NODES);
	svs.numareanodes = 0;

	for(Uint32 i = 0; i < MAX_AREA_NODES; i++)
		svs.areanodes[i].index = i;

	// Create the area nodes
	SV_CreateAreaNode(0, ens.pworld->mins, ens.pworld->maxs);
	// Create the box hull
	TR_InitBoxHull();

	// Set the pointers of all the clients
	for(Uint32 i = 0; i < svs.maxclients; i++)
	{
		sv_client_t& cl = svs.clients[i];

		cl.index = i;
		cl.pedict = gEdicts.GetEdict(svs.clients[i].index+1);
		cl.pedict->clientindex = i;
		cl.jointime = 0;

		cl.spawned = false;
		cl.active = false;
		cl.connected = false;

		// Clear previous edict array
		if(!cl.packet.cl_entitystates.empty())
			cl.packet.cl_entitystates.clear();

		// Allocate new one
		cl.packet.cl_entitystates.resize(gEdicts.GetMaxEdicts());
		cl.packet.cl_wasinpacket.resize(gEdicts.GetMaxEdicts());

		// Mark as a client entity
		cl.pedict->state.flags |= (FL_CLIENT|FL_DORMANT);
	}

	VID_DrawLoadingScreen("Loading entity data");

	// Set host client
	svs.phostclient = &svs.clients[0];
	svs.phostclient->active = false;
	svs.phostclient->connected = true;
	svs.phostclient->spawned = false;
	svs.phostclient->pedict->state.flags &= ~FL_DORMANT;

	// Register usermsgs to localhost before entity load
	SV_ClientRegisterUserMessages(*svs.phostclient);

	// Load the transition save file if specified
	const save_header_t* ptranssaveheader = nullptr;
	if(pstrTransitionSave)
	{
		ptranssaveheader = reinterpret_cast<const save_header_t*>(gSaveRestore.LoadSaveFile(pstrTransitionSave));
		if(!ptranssaveheader)
		{
			Con_EPrintf("%s - Failed to open save file '%s'.\n", __FUNCTION__, pstrTransitionSave);
			SV_ClearGame(clearLoadingScreen);
			return false;
		}

		// Load globals from the transition save file
		gSaveRestore.LoadGlobalsFromSaveFile(ptranssaveheader);
	}

	// Load entities from the BSP or save file
	if(!pstrSaveFile)
	{
		if(!gEdicts.LoadEntities( ens.pworld->pentdata ))
		{
			Con_EPrintf("%s - Failed to parse entities from world.\n", __FUNCTION__);
			SV_ClearGame(clearLoadingScreen);
			return false;
		}
	}
	else
	{
		// Prevent client spawn functions from being called
		svs.saverestore = true;

		// Load from the save file
		if(!gSaveRestore.LoadSaveData( psaveheader ))
		{
			Con_EPrintf("%s - Failed to parse entities from save file '%s'.\n", __FUNCTION__, pstrSaveFile);
			SV_ClearGame(clearLoadingScreen);
			return false;
		}

		gSaveRestore.FreeSaveFile(reinterpret_cast<const byte*>(psaveheader));
	}

	// Set host client's join time now
	// that entities have been loaded
	svs.phostclient->jointime = svs.time;

	// Load transition save if present
	if(ptranssaveheader)
	{
		bool transitionResult = SV_RestoreTransitionSave(ptranssaveheader);
		
		// Remove the file
		CString removepath;
		removepath << ens.gamedir << PATH_SLASH_CHAR << pstrTransitionSave;

		if(remove(removepath.c_str()) != 0)
			Con_Printf("%s - Failed to delete file '%s' - %s.\n", __FUNCTION__, pstrTransitionSave, strerror(errno));

		if(!transitionResult)
		{
			Con_EPrintf("%s - Failure during transition save load.\n", __FUNCTION__);
			SV_ClearGame(clearLoadingScreen);
			return false;
		}
	}

	// Call game dll to precache generic resources
	svs.dllfuncs.pfnPrecacheResources();

	// Set up world entity
	edict_t* pworldentity = gEdicts.GetEdict(WORLDSPAWN_ENTITY_INDEX);
	if(!pworldentity || pworldentity->free)
	{
		Con_EPrintf("%s - Failed to allocate worldspawn.\n", __FUNCTION__);
		SV_ClearGame(clearLoadingScreen);
		return false;
	}

	// These must be set here
	if(!SV_SetModel(pworldentity, filepath.c_str(), true))
	{
		Con_EPrintf("%s - Failed to set worldspawn model.\n", __FUNCTION__);
		SV_ClearGame(clearLoadingScreen);
		return false;
	}

	if(svs.phostclient->pedict->pprivatedata)
		svs.phostclient->originset = true;
	else
		svs.phostclient->originset = false;

	// Tell the engine we're active
	svs.frametime = 0;
	svs.gamevars.levelname = svs.mapname;

	// Halt simulation until everything is loaded
	svs.haltserver = true;

	Con_Printf("Loaded %s.\n", svs.mapname.c_str());

	// Inform client we're connected
	if(!CL_EstablishConnection("localhost"))
	{
		Con_EPrintf("%s - Failed to connect to local server.\n", __FUNCTION__);
		SV_ClearGame(clearLoadingScreen);
		return false;
	}

	// Build enforced consistency list
	SV_AddEnforcedConsistencyFile(SERVER_DLL_PATH);
	SV_AddEnforcedConsistencyFile(CLIENT_DLL_PATH);
	SV_AddEnforcedConsistencyFile(filepath.c_str());

	VID_DrawLoadingScreen("Loading WAD files");

	// Initialize WAD files on server
	CArray<CString> mapWADList;
	Common::GetWADList(ens.pworld->pentdata, mapWADList);

	// Make sure there's not one already present
	if(ens.pwadresource)
	{
		delete ens.pwadresource;
		ens.pwadresource = nullptr;
	}

	// Allocate new object
	ens.pwadresource = new CWADTextureResource();
	if(ens.pwadresource->Init(
		ens.pworld->name.c_str(), 
		mapWADList,
		(g_pCvarWadTextureChecks->GetValue() >= 1) ? true : false,
		(g_pCvarBspTextureChecks->GetValue() >= 1) ? true : false))
	{
		// Link up map textures with material scripts
		SV_LinkMapTextureMaterials(mapWADList);
	}
	else
	{
		// Log failure
		Con_EPrintf("%s - Failed to initialize WAD resources.\n", __FUNCTION__);
	}

	// Set paused state
	Sys_SetPaused(false, false);

	// Call PostSpawnGame function on game dll
	svs.dllfuncs.pfnPostSpawnGame();

	return true;
}

//=============================================
//
//=============================================
bool SV_InitGame( void )
{
	// Set these flags
	svs.haltserver = false;
	ens.skipframe = true;

	// Tell game dll we're up
	if(!svs.dllfuncs.pfnGameInit())
	{
		Con_Printf("Game setup failed in game dll.\n");
		SV_ClearGame();
		return false;
	}

	// Spawn save-restored decals
	SV_RestoreSavedDecals();

	// Update data on client
	SV_UpdateClientData();

	// Print load time
	Double loadtime = Sys_FloatTime() - svs.loadbegintime;
	Con_DPrintf("Load time: %.2f seconds.\n", loadtime);
	return true;
}

//=============================================
//
//=============================================
void SV_ClearClient( sv_client_t& client )
{
	client.active = false;
	client.connected = false;
	client.spawned = false;
	client.initialized = false;
	client.originset = false;

	// Clear entity info
	client.packet.cl_packetindex = 0;
	client.packet.numentities = 0;
	client.pminfo = pm_info_t();

	// Clear client reference arrays
	if(!client.packet.cl_entitystates.empty())
		client.packet.cl_entitystates.clear();

	if(!client.packet.cl_wasinpacket.empty())
		client.packet.cl_wasinpacket.clear();

	for(Uint32 i = 0; i < MAX_VISIBLE_ENTITIES; i++)
		client.packet.entities[i] = entity_state_t();

	// Release entity data
	gEdicts.FreeEdict(client.pedict, EDICT_REMOVED_CLEAR_CLIENT);

	client.pedict->state.flags |= (FL_DORMANT|FL_CLIENT);
	client.pedict = nullptr;

	client.jointime = 0;
	client.lastusercmdidx = 0;
	client.numusercmd = 0;
	
	if(!client.cachedmsglist.empty())
	{
		client.cachedmsglist.begin();
		while(!client.cachedmsglist.end())
		{
			delete client.cachedmsglist.get();
			client.cachedmsglist.next();
		}

		client.cachedmsglist.clear();
	}

	// Clear consistency list
	client.consistencylist.clear();
	// Clear any uploads
	SV_ClearUpload(client);
}

//=============================================
//
//=============================================
void SV_ClearGame(  bool clearloadingscreen, bool clearconnections )
{
	// Drop any connected clients
	if(CL_IsHostClient())
	{
		for(Uint32 i = 1; i < svs.maxclients; i++)
			SV_DropClient(svs.clients[i], "Server shutting down");
	}

	// Clear the model cache
	gModelCache.ClearCache();

	// Clear local client's msg cache
	if(svs.phostclient && !svs.phostclient->cachedmsglist.empty())
	{
		svs.phostclient->cachedmsglist.begin();
		while(!svs.phostclient->cachedmsglist.end())
		{
			delete svs.phostclient->cachedmsglist.get();
			svs.phostclient->cachedmsglist.next();
		}
		svs.phostclient->cachedmsglist.clear();
	}

	// Reset stuff
	svs.mapname.clear();
	ens.gamestate = GAME_INACTIVE;
	svs.frametime = 0;
	svs.time = 0;
	svs.phostclient = nullptr;
	svs.plastspawnentity = nullptr;
	svs.lastpvschecktime = 0;
	svs.lastpvscheckclient = NO_CLIENT_INDEX;
	svs.saverestore = false;
	svs.pauseovveride = false;

	// Reset gamevars
	svs.gamevars.force_retouch = false;
	svs.gamevars.paused = false;
	svs.gamevars.frametime = 0;
	svs.gamevars.time = 0;
	svs.gamevars.levelname.clear();
	svs.gamevars.gametime = 0;
	svs.gamevars.numentities = 0;

	// Clear consistency list
	svs.netinfo.enforcedfiles.clear();

	// Clear prompts hash list
	svs.promptshashlist.clear();

	// Clear all edicts
	gEdicts.ClearEdicts();

	// Tell gamedll to clean up
	svs.dllfuncs.pfnGameShutdown();

	// Clear connections
	SV_ClearConnections();

	// Clear areanodes after edicts
	if(!svs.areanodes.empty())
		svs.areanodes.clear();

	// Clear save-restored decals
	if(!svs.saveddecalslist.empty())
		svs.saveddecalslist.clear();

	svs.numareanodes = 0;

	// Clear worldent pointer
	ens.pworld = nullptr;

	// Clear caches
	svs.modelcache.clear();
	svs.sndcache.clear();
	svs.mapmaterialfiles.clear();
	svs.particlescache.clear();

	// Clear string buffer
	svs.strbuffer.buffer.clear();
	svs.strbuffer.numstrings = 0;

	svs.perrormodel = nullptr;
	svs.perrorsprite = nullptr;

	// Disconnect localhost
	if(CL_IsHostClient())
	{
		SV_DropClient(svs.clients[0], "Server shutting down");
		
		if(cls.cl_state != CLIENT_INACTIVE)
			CL_Disconnect(false, clearloadingscreen);
	}

	// Make sure this isn't left in
	if(!svs.netinfo.pcurrentmsg)
	{
		delete svs.netinfo.pcurrentmsg;
		svs.netinfo.pcurrentmsg = nullptr;
	}

	// Delete networking instance
	CNetworking::DeleteInstance();
	svs.netinfo.pnet = nullptr;

	// Clear at very end
	svs.serverstate = SV_INACTIVE;
	
	// Reset this
	svs.paused = false;

	// End loading plaque
	if(clearloadingscreen)
		VID_EndLoading();
}

//=============================================
//
//=============================================
void SV_DropClient( sv_client_t& cl, const Char *pstrReason )
{
	if(!cl.connected)
		return;

	// Disconnect the client
	SV_ClientDisconnected(cl);

	// Allow server to manage this
	svs.dllfuncs.pfnClientDisconnected(cl.pedict);

	if(cl.index > 0)
	{
		// Check for errors on client
		if(svs.netinfo.pnet->GetClientState(cl.index) == NETCL_CONNECTED)
		{
			svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_disconnect, cl.pedict);
				svs.netinfo.pnet->WriteByte(false); // rejected/banned flag
				svs.netinfo.pnet->WriteBuffer(reinterpret_cast<const byte*>(pstrReason), qstrlen(pstrReason)+1);
			svs.netinfo.pnet->SVC_MessageEnd();
		}
	}

	// Disconnect this client
	svs.netinfo.pnet->Disconnect(cl.index);

	// Clear client
	SV_ClearClient(cl);
}

//=============================================
//
//=============================================
void SV_PlayerThink( edict_t* pedict, Double clienttimebase )
{
	if(!(pedict->state.flags & FL_KILLME))
	{
		Float thinktime = pedict->state.nextthink;
		if(thinktime <= 0.0 || clienttimebase + svs.frametime < thinktime)
			return;

		if(thinktime < clienttimebase)
			thinktime = clienttimebase;

		pedict->state.nextthink = 0;
		svs.gamevars.time = thinktime;
		svs.dllfuncs.pfnDispatchThink(pedict);

		// restore time
		svs.gamevars.time = svs.time;
	}

	if(pedict->state.flags & FL_KILLME)
		gEdicts.FreeEdict(pedict, EDICT_REMOVED_KILLED);
}

//=============================================
//
//=============================================
void SV_SetMoveVars( sv_client_t& cl )
{
	cl.pminfo.movevars.accelerate = g_psv_accelerate->GetValue();
	cl.pminfo.movevars.airaccelerate = g_psv_airaccelerate->GetValue();
	cl.pminfo.movevars.bounce = g_psv_bounce->GetValue();
	cl.pminfo.movevars.edgefriction = g_psv_edgefriction->GetValue();
	cl.pminfo.movevars.entgravity = 1.0f;
	cl.pminfo.movevars.friction = g_psv_friction->GetValue();
	cl.pminfo.movevars.gravity = g_psv_gravity->GetValue();
	cl.pminfo.movevars.maxspeed = g_psv_maxspeed->GetValue();
	cl.pminfo.movevars.maxvelocity = g_psv_maxvelocity->GetValue();
	cl.pminfo.movevars.stepsize = g_psv_stepsize->GetValue();
	cl.pminfo.movevars.stopspeed = g_psv_stopspeed->GetValue();
	cl.pminfo.movevars.wateraccelerate = g_psv_wateraccelerate->GetValue();
	cl.pminfo.movevars.waterfriction = g_psv_waterfriction->GetValue();
	cl.pminfo.movevars.waterdist = g_psv_waterdist->GetValue();
	cl.pminfo.movevars.maxclients = svs.maxclients;
	cl.pminfo.movevars.holdtoduck = (g_psv_holdtoduck->GetValue() >= 1 ? true : false);
}

//=============================================
//
//=============================================
void SV_CheckMoveVars( sv_client_t& cl )
{
	if(!cl.active && !cl.spawned && !cl.connected)
		return;

	if(cl.pminfo.movevars.accelerate != g_psv_accelerate->GetValue() ||
		cl.pminfo.movevars.airaccelerate != g_psv_airaccelerate->GetValue() ||
		cl.pminfo.movevars.bounce != g_psv_bounce->GetValue() ||
		cl.pminfo.movevars.edgefriction != g_psv_edgefriction->GetValue() ||
		cl.pminfo.movevars.entgravity != 1.0f ||
		cl.pminfo.movevars.friction != g_psv_friction->GetValue() ||
		cl.pminfo.movevars.gravity != g_psv_gravity->GetValue() ||
		cl.pminfo.movevars.maxspeed != g_psv_maxspeed->GetValue() ||
		cl.pminfo.movevars.maxvelocity != g_psv_maxvelocity->GetValue() ||
		cl.pminfo.movevars.stepsize != g_psv_stepsize->GetValue() ||
		cl.pminfo.movevars.stopspeed != g_psv_stopspeed->GetValue() ||
		cl.pminfo.movevars.wateraccelerate != g_psv_wateraccelerate->GetValue() ||
		cl.pminfo.movevars.waterfriction != g_psv_waterfriction->GetValue() ||
		cl.pminfo.movevars.waterdist != g_psv_waterdist->GetValue() ||
		cl.pminfo.movevars.maxclients != svs.maxclients ||
		cl.pminfo.movevars.holdtoduck != (g_psv_holdtoduck->GetValue() >= 1 ? true : false)
		)
	{
		SV_SetMoveVars(cl);
		SV_WriteMoveVars(cl);
	}
}

//=============================================
//
//=============================================
void SV_CheckMovingGround( edict_t *pclient, Double frametime )
{
	if(pclient->state.flags & FL_ONGROUND)
	{
		edict_t* pgroundentity = gEdicts.GetEdict(pclient->state.groundent);
		if(pgroundentity && pgroundentity->state.flags & FL_CONVEYOR)
		{
			if(pclient->state.flags & FL_BASEVELOCITY)
				Math::VectorMA(pclient->state.basevelocity, pgroundentity->state.speed, pgroundentity->state.movedir, pclient->state.basevelocity);
			else
				Math::VectorScale(pgroundentity->state.movedir, pgroundentity->state.speed, pclient->state.basevelocity);

			pclient->state.flags |= FL_BASEVELOCITY;
		}
	}

	if(!(pclient->state.flags & FL_BASEVELOCITY))
	{
		Math::VectorMA(pclient->state.velocity, frametime*0.5+1.0f, pclient->state.basevelocity, pclient->state.velocity);
		pclient->state.basevelocity.Clear();
	}

	pclient->state.flags &= ~FL_BASEVELOCITY;
}

//=============================================
//
//=============================================
void SV_RunCmd( usercmd_t& cmd, sv_client_t& cl )
{
	// Set last acknowledged usercmd
	// NOTE: messages should come in
	// order, but let's be sure
	if(cmd.cmdidx > cl.lastusercmdidx)
		cl.lastusercmdidx = cmd.cmdidx;

	// Don't proceed further if paused
	if(svs.paused)
		return;

	// Tell engine which player we're running cmds for
	svs.gamevars.predict_player = cl.index;

	// Set impulse
	cl.pedict->state.impulse = cmd.impulse;
	cl.pedict->state.buttons = cmd.buttons;

	// Manage base velocity
	SV_CheckMovingGround(cl.pedict, cmd.msec*MILLISECONDS_TO_SECONDS);

	// Run pre-command client functions
	svs.gamevars.time = svs.time;
	svs.dllfuncs.pfnClientPreThink(cl.pedict);
	SV_PlayerThink(cl.pedict, svs.time);

	// Set current player index and reset traces
	cl.pminfo.clientindex = cl.index;
	svs.numpmovetraces = 0;

	// Copy hull sizes
	for(Int32 i = 0; i < MAX_MAP_HULLS; i++)
	{
		Math::VectorCopy(svs.player_mins[i], cl.pminfo.player_mins[i]);
		Math::VectorCopy(svs.player_maxs[i], cl.pminfo.player_maxs[i]);
	}

	// Save stuff
	cl.pminfo.oldangles = cl.pedict->state.viewangles;
	cl.pminfo.playerstate = cl.pedict->state;

	// Set client's view angles
	if(!cl.pedict->state.fixangles)
		cl.pminfo.playerstate.viewangles = cmd.viewangles;

	Vector savedviewangles = cl.pminfo.playerstate.viewangles;

	// Run client movement code
	svs.dllfuncs.pfnRunPlayerMovement(cmd, &cl.pminfo);

	cl.pedict->state = cl.pminfo.playerstate;

	// Restore saved view angles
	if(cl.pedict->state.fixangles)
		cl.pedict->state.viewangles = savedviewangles;

	cl.pedict->state.angles = cl.pedict->state.viewangles;
	cl.pedict->state.oldbuttons = cmd.buttons;

	// Run impact function for any entities we touched
	if(cl.pedict->state.solid != SOLID_NOT && svs.numpmovetraces > 0)
	{
		SV_LinkEdict(cl.pedict, true);

		Vector savedvelocity = cl.pedict->state.velocity;
		for(Uint32 i = 0; i < svs.numpmovetraces; i++)
		{
			trace_t& tr = svs.pmovetraces[i];
			edict_t* ptouchent = gEdicts.GetEdict(tr.hitentity);

			cl.pedict->state.velocity = tr.deltavelocity;
			SV_Impact(ptouchent, cl.pedict, tr);
		}

		cl.pedict->state.velocity = savedvelocity;
	}

	// Run post-command client functions before linking
	svs.dllfuncs.pfnClientPostThink(cl.pedict);

	// Touch links on edicts
	SV_LinkEdict(cl.pedict, true);

	svs.dllfuncs.pfnCmdEnd(cl.pedict);
}

//=============================================
//
//=============================================
void SV_UpdateClients( void )
{
	for(Uint32 i = 0; i < svs.maxclients; i++)
	{
		sv_client_t& cl = svs.clients[i];
		if(!cl.connected)
			continue;

		sv_clstate_t result = SV_ReadClientMessages(cl);
		if(result == SVCL_LOST_CONNECTION)
		{
			// Don't do anything during timeouts
			svs.netinfo.pnet->CLS_ClearMessages(i);
			continue;
		}
		else if(result != SVCL_OK)
		{
			CString reason;
			switch(result)
			{
			case SVCL_REJECTED: 
				reason = "Rejected by server"; 
				break;
			case SVCL_CLS_BAD: 
				reason = "cls_bad"; 
				break;
			case SVCL_SPAWN_FAILED:
				reason = "Error on spawn";
				break;
			case SVCL_NOT_CONSISTENT:
				reason = "Consistency check invalid";
				break;
			case SVCL_DISCONNECTED:
				reason = "Disonnected";
				break;
			case SVCL_NET_ERROR:
				reason = "Network error";
				break;
			case SVCL_RESOURCES_ERROR:
				reason = "Failure during file transfer";
				break;
			case SVCL_INCONSISTENT_FILE:
				reason = "Inconsistency detected in an enforced file";
				break;
			}

			if(&cl == svs.phostclient)
			{
				Con_Printf("%s.\n", reason.c_str());
				SV_ClearGame();
				return;
			}
			else 
			{
				if(cl.connected)
				{
					// If the client wasn't already dropped
					SV_DropClient(cl, reason.c_str());
				}

				continue;
			}
		}

		// Clear CLS arrays
		svs.netinfo.pnet->CLS_ClearMessages(i);

		// Run command if client has spawned
		if(cl.spawned)
		{
			for(Uint32 j = 0; j < cl.numusercmd; j++)
				SV_RunCmd(cl.usercmdarray[j], cl);

			// Restore origin
			SV_SetOrigin(cl.pedict, cl.pedict->state.origin);
		}

		// reset it to zero
		cl.numusercmd = 0;
	}
}

//=============================================
//
//=============================================
void SV_PostClientUpdate( void )
{
	Int32 numedicts = gEdicts.GetNbEdicts();
	for(Int32 i = 0; i < numedicts; i++)
	{
		edict_t* pedict = gEdicts.GetEdict(i);
		if(pedict->free)
			continue;

		if(pedict->state.effects & EF_NOINTERP)
			pedict->state.effects &= ~EF_NOINTERP;

		if(pedict->state.effects & EF_MUZZLEFLASH)
			pedict->state.effects &= ~EF_MUZZLEFLASH;

		if(pedict->state.effects & EF_NOLERP)
			pedict->state.effects &= ~EF_NOLERP;

		if(pedict->state.effects & EF_SET_SEQTIME)
			pedict->state.effects &= ~EF_SET_SEQTIME;
	}
}

//=============================================
//
//=============================================
void SV_Frame( void )
{
	// Only simulate for host
	if(svs.serverstate != SV_ACTIVE)
		return;

	// Set frame time
	svs.frametime = ens.frametime;

	// set nb of entities
	svs.gamevars.numentities = gEdicts.GetNbEdicts();
	// set nb of clients
	svs.gamevars.maxclients = svs.maxclients;

	// Read client messages
	SV_UpdateClients();

	if(!svs.paused && !svs.haltserver)
	{
		// Simulate physics
		SV_Physics();

		// Increment time after doing physics
		svs.time += ens.frametime;

		// Increment game time
		svs.gamevars.gametime += ens.frametime;
	}

	// set after physics simulation again
	svs.gamevars.numentities = gEdicts.GetNbEdicts();

	// Update movevars on clients if needed
	for(Uint32 i = 0; i < svs.maxclients; i++)
		SV_CheckMoveVars(svs.clients[i]);

	// Run frame on gamedll too
	svs.dllfuncs.pfnServerFrame();

	// Send data to the clients
	SV_UpdateClientData();

	// Clears specific entity flags after client updated
	SV_PostClientUpdate();
}

//=============================================
//
//=============================================
Int32 SV_PrecacheModel( const Char* pstrFilepath )
{
	// Check if it's already in the cache
	for(Uint32 i = 0; i < svs.modelcache.size(); i++)
	{
		if(!qstrcmp(svs.modelcache[i].modelname, pstrFilepath))
			return svs.modelcache[i].cache_index;
	}

	// Do not allow precache past loading
	if(ens.gamestate != GAME_LOADING)
	{
		Con_Printf("%s - Precache for models only allowed during level load.\n", __FUNCTION__);
		return NO_PRECACHE;
	}

	cache_model_t* pmodel = gModelCache.LoadModel(pstrFilepath);
	if(!pmodel)
	{
		const byte* phashdata = reinterpret_cast<const byte*>(pstrFilepath);
		if(svs.promptshashlist.addhash(phashdata, qstrlen(pstrFilepath)))
			Con_Printf("%s - Could not precache '%s'.\n", __FUNCTION__, pstrFilepath);
		return NO_PRECACHE;
	}

	// Add to the cache
	sv_model_t newmodel;
	newmodel.modelname = pstrFilepath;
	newmodel.cache_index = pmodel->cacheindex;
	svs.modelcache.push_back(newmodel);

	return newmodel.cache_index;
}

//=============================================
//
//=============================================
byte* SV_SetPAS( const Vector& origin )
{
	// Get the leaf we're on
	const mleaf_t* pleaf = Mod_PointInLeaf(origin, (*ens.pworld));

	Mod_LeafPAS(svs.ppasbuffer, ens.visbuffersize, (*pleaf), (*ens.pworld));
	return svs.ppasbuffer;
}

//=============================================
//
//=============================================
byte* SV_SetPVS( const Vector& origin )
{
	// Get the leaf we're on
	const mleaf_t* pleaf = Mod_PointInLeaf(origin, (*ens.pworld));
	
	Mod_LeafPVS(svs.pvisbuffer, ens.visbuffersize, (*pleaf), (*ens.pworld));
	return svs.pvisbuffer;
}

//=============================================
//
//=============================================
void SV_EstablishedClientConnection( Uint32 client_index )
{
	// Establish new client entity
	sv_client_t* pclient = &svs.clients[client_index];

	// Set active flag to true
	pclient->active = true;
	pclient->connected = true;
	pclient->jointime = svs.time;
}

//=============================================
//
//=============================================
void SV_PrepareClient( Uint32 client_index )
{
	// Establish new client entity
	sv_client_t* pclient = &svs.clients[client_index];

	// Client hasn't spawned
	pclient->spawned = false;
	pclient->initialized = false;
	pclient->originset = false;
	pclient->connected = true;

	// Reset this
	pclient->lastusercmdidx = 0;
}

//=============================================
//
//=============================================
void SV_ClientDisconnected( sv_client_t& cl )
{
	assert(svs.netinfo.pnet != nullptr);

	const Char* pstrInfo = svs.netinfo.pnet->GetInfoString(cl.index);
	if(qstrlen(pstrInfo))
		SV_ClientPrintf(nullptr, "%s has left the game - %s.\n", SV_GetString(cl.pedict->fields.netname), pstrInfo);
	else
		SV_ClientPrintf(nullptr, "%s has left the game.\n", SV_GetString(cl.pedict->fields.netname));

	// Call server to handle this
	svs.dllfuncs.pfnClientDisconnected(cl.pedict);
}

//=============================================
//
//=============================================
bool SV_VerifyClient( sv_client_t& cl, const Char* pstrPlayerName )
{
	// Retrieve IP of the client that wants to connect
	CString clientIp = svs.netinfo.pnet->GetIPAddress(cl.index);

	// See if the game is willing to accept the client
	CString rejectReason;
	if(!svs.dllfuncs.pfnClientConnect(cl.pedict, pstrPlayerName, clientIp.c_str(), rejectReason))
	{
		// Tell him why he was dropped
		SV_ClientPrintf(nullptr, "%s rejected, reason: %s.\n", pstrPlayerName, rejectReason.c_str());
		SV_DropClient(cl, rejectReason.c_str());
		return false;
	}

	return true;
}

//=============================================
//
//=============================================
bool SV_SpawnClient( sv_client_t& cl )
{
	// Find spawn spot
	edict_t* pspawn = nullptr;
	if(!cl.originset)
	{
		pspawn = SV_FindSpawnSpot();
		if(!pspawn && svs.maxclients > 1)
		{
			// MP cannot allow for the lack of spawn points
			Con_EPrintf("Error: No spawn points in level, exiting.\n");
			return false;
		}
	}

	edict_t* pedict = gEdicts.GetEdict(cl.index+1);
	if(!pedict)
	{
		Con_EPrintf("Create failed on player entity.\n");
		return false;
	}

	if(!pedict->pprivatedata)
		pedict = gEdicts.CreatePlayerEntity(cl.index);

	// Set origin/angles
	if(!cl.originset)
	{
		if(pspawn)
		{
			cl.pedict->state.origin = pspawn->state.origin;
			cl.pedict->state.angles = pspawn->state.angles;
		}
		else
		{
			cl.pedict->state.origin = ZERO_VECTOR;
			cl.pedict->state.angles = ZERO_VECTOR;
		}
	}

	// Set view angles and make them be reset
	cl.pedict->state.viewangles = cl.pedict->state.angles;
	cl.pedict->state.flags &= ~FL_DORMANT;
	cl.pedict->state.flags |= FL_CLIENT;
	cl.pedict->state.fixangles = true;
	cl.pedict->clientindex = cl.index;

	// Spawn the client
	if(!svs.saverestore && !svs.dllfuncs.pfnDispatchSpawn(cl.pedict))
	{
		Con_EPrintf("Spawn failed on player entity.\n");
		return false;
	}

	if(svs.maxclients > 1)
	{
		CString message;
		message << SV_GetString(pedict->fields.netname) << " has joined the game.\n";

		Uint32 sayMsgId = SV_FindUserMessageByName("SayText");
		if(sayMsgId != 0)
		{
			CString netname;
			netname << "Server notice: ";

			SV_UserMessageBegin(MSG_ALL, sayMsgId, nullptr, nullptr);
				SV_Msg_WriteString(netname.c_str());
				SV_Msg_WriteString(message.c_str());
			SV_UserMessageEnd();
		}

		// Add it to console as well
		SV_ClientPrintf(nullptr, message.c_str());
	}

	// Spawn the client
	cl.spawned = true;
	cl.active = true;
	cl.initialized = false;
	cl.originset = true;

	// Call for entities to send init messages
	svs.dllfuncs.pfnInitializeClientData(cl.pedict);

	return true;
}

//=============================================
//
//=============================================
const Char* SV_TraceTexture( Int32 groundentity, const Vector& start, const Vector& end )
{
	if(groundentity < 0 || groundentity >= static_cast<Int32>(gEdicts.GetNbEdicts()))
	{
		Con_Printf("%s - Bogus entity index %d.\n", __FUNCTION__, groundentity);
		return nullptr;
	}

	edict_t* pedict = gEdicts.GetEdict(groundentity);
	if(!pedict->state.modelindex)
	{
		Con_Printf("%s - Called on entity %d with no model set.\n", __FUNCTION__, groundentity);
		return nullptr;
	}
	
	return TR_TraceTexture(pedict->state, start, end);
}

//=============================================
//
//=============================================
void SV_SetGroupMask( Int32 mask, Int32 op )
{
	ens.tr_groupmask = mask;
	ens.tr_groupop = op;
}

//=============================================
//
//=============================================
bool SV_IsHostClient( entindex_t entindex )
{
	return (entindex == HOST_CLIENT_ENTITY_INDEX) ? true : false;
}

//=============================================
//
//=============================================
bool SV_IsWorldSpawn( entindex_t entindex )
{
	return (entindex == WORLDSPAWN_ENTITY_INDEX) ? true : false;
}

//=============================================
//
//=============================================
const Char* SV_NameForFunction( const void* functionPtr )
{
	for(Uint32 i = 0; i < svs.exports.size(); i++)
	{
		if(svs.exports[i].functionptr == functionPtr)
			return svs.exports[i].functionname.c_str();
	}

	return nullptr;
}

//=============================================
//
//=============================================
void* SV_FunctionFromName( const Char* pstrName )
{
	for(Uint32 i = 0; i < svs.exports.size(); i++)
	{
		if(!qstrcmp(svs.exports[i].functionname, pstrName))
			return svs.exports[i].functionptr;
	}

	return nullptr;
}

//=============================================
//
//=============================================
sv_client_t* SV_GetHostClient( void )
{
	return &svs.clients[0];
}

//=============================================
//
//=============================================
const en_material_t* SV_GetMaterialScript( const Char* pstrTextureName )
{
	return CTextureManager::GetInstance()->FindMaterialScript(pstrTextureName, RS_GAME_LEVEL);
}

//=============================================
//
//=============================================
void SV_AddEnforcedConsistencyFile( const Char* pstrFilename )
{
	for(Uint32 i = 0; i < svs.netinfo.enforcedfiles.size(); i++)
	{
		if(!qstrcmp(svs.netinfo.enforcedfiles[i], pstrFilename))
			return;
	}

	svs.netinfo.enforcedfiles.push_back(pstrFilename);
}

//=============================================
//
//=============================================
Uint32 SV_AllocString( const Char* pString )
{
	if(!pString || qstrlen(pString) == 0)
		return NO_STRING_VALUE;

	// See if it's already present
	for(Uint32 i = 0; i < svs.strbuffer.numstrings; i++)
	{
		if(!qstrcmp(pString, svs.strbuffer.buffer[i]))
			return i + 1;
	}

	// Resize the buffer if needed
	if(svs.strbuffer.buffer.empty() || svs.strbuffer.buffer.size() == svs.strbuffer.numstrings)
		svs.strbuffer.buffer.resize(svs.strbuffer.buffer.size() + STRINGBUFFER_ALLOC_SIZE);
	
	// Add it to the list
	Uint32 returnindex = svs.strbuffer.numstrings;
	svs.strbuffer.buffer[svs.strbuffer.numstrings] = pString;
	svs.strbuffer.numstrings++;

	return returnindex + 1;
}

//=============================================
//
//=============================================
const Char* SV_GetString( string_t stringindex )
{
	if(stringindex <= NO_STRING_VALUE)
		return "";

	// Do not allow overindexing
	Uint32 realindex = stringindex-1;
	if(realindex >= svs.strbuffer.numstrings)
	{
		Con_Printf("%s - Index %d out of bounds.\n", __FUNCTION__, stringindex);
		return nullptr;
	}

	return svs.strbuffer.buffer[realindex].c_str();
}

//=============================================
//
//=============================================
bool SV_IsValidCommand( const Char* pstrCmd )
{
	Uint32 length = qstrlen(pstrCmd);
	if(length > 0 && (pstrCmd[length-1] == '\n' || pstrCmd[length-1] == ';'))
		return true;
	else
		return false;
}

//=============================================
//
//=============================================
void SV_ServerCommand( const Char* pstrCmd )
{
	if(!SV_IsValidCommand(pstrCmd))
	{
		Con_Printf("%s - Not a valid command: %s.\n", __FUNCTION__, pstrCmd);
		return;
	}

	gCommands.AddCommand(pstrCmd);
}

//=============================================
//
//=============================================
void SV_ClientCommand( edict_t* pclient, const Char* pstrCmd )
{
	if(!pclient)
	{
		Con_Printf("%s - Client is nullptr.\n", __FUNCTION__);
		return;
	}

	if(pclient->clientindex == NO_CLIENT_INDEX)
	{
		Con_Printf("%s - Not a client.\n", __FUNCTION__);
		return;
	}

	if(svs.serverstate != SV_ACTIVE)
	{
		Con_Printf("%s - Called on inactive server.\n", __FUNCTION__);
		return;
	}

	if(!SV_IsValidCommand(pstrCmd))
	{
		Con_Printf("%s - Not a valid command: %s.\n", __FUNCTION__, pstrCmd);
		return;
	}

	// Send it to the destination client
	cls.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_clcommand, pclient);
		cls.netinfo.pnet->WriteString(pstrCmd);
	cls.netinfo.pnet->SVC_MessageEnd();
}

//=============================================
//
//=============================================
bool SV_GetBonePositionByName( edict_t* pedict, const Char* pstrbonename, Vector& position )
{
	if(!pedict)
	{
		Con_Printf("%s - Entity was null.\n", __FUNCTION__);
		return false;
	}

	if(!pedict->state.modelindex)
	{
		Con_Printf("%s - Entity has no model.\n", __FUNCTION__);
		return false;
	}

	const cache_model_t* pmodel = gModelCache.GetModelByIndex(pedict->state.modelindex);
	if(pmodel->type != MOD_VBM)
	{
		Con_Printf("%s - Entity is not a vbm model.\n", __FUNCTION__);
		return false;
	}

	// Get cache
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;

	Int32 sequence = pedict->state.sequence;
	if(sequence >= pstudiohdr->numseq)
		sequence = 0;

	// Get sequence data
	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequence);
	if(!pseqdesc)
		return false;

	Float frame = VBM_EstimateFrame(pseqdesc, pedict->state, svs.time);

	if(!pedict->pvbmhulldata)
		pedict->pvbmhulldata = new entity_vbmhulldata_t;

	// Reuse VBM trace info structures
	if(!TR_VBMCheckHullInfo(pedict->pvbmhulldata, pmodel, svs.time, pedict->state))
	{
		// Set animation state
		TR_VBMSetStateInfo(pedict->pvbmhulldata, pmodel, frame, pedict->state);

		for(Uint32 i = 0; i < MAX_MAP_HULLS; i++)
			pedict->pvbmhulldata->hulls[i].hullset = false;

		// Update bones
		TR_VBMSetupBones(pedict->pvbmhulldata, pstudiohdr, svs.time, frame, pseqdesc, pmodel, pedict->state);
	}

	// Find the bone
	const mstudiobone_t* pbones = reinterpret_cast<const mstudiobone_t*>(reinterpret_cast<const byte*>(pstudiohdr) + pstudiohdr->boneindex);
	for(Int32 i = 0; i < pstudiohdr->numbones; i++)
	{
		if(!qstrcmp(pbones[i].name, pstrbonename))
		{
			position[0] = pedict->pvbmhulldata->bonetransform[i][0][3];
			position[1] = pedict->pvbmhulldata->bonetransform[i][1][3];
			position[2] = pedict->pvbmhulldata->bonetransform[i][2][3];
			return true;
		}
	}
	
	return false;
}

//=============================================
//
//=============================================
bool SV_GetBonePositionByIndex( edict_t* pedict, Uint32 boneindex, Vector& position )
{
	if(!pedict)
	{
		Con_Printf("%s - Entity was null.\n", __FUNCTION__);
		return false;
	}

	if(!pedict->state.modelindex)
	{
		Con_Printf("%s - Entity has no model.\n", __FUNCTION__);
		return false;
	}

	const cache_model_t* pmodel = gModelCache.GetModelByIndex(pedict->state.modelindex);
	if(pmodel->type != MOD_VBM)
	{
		Con_Printf("%s - Entity is not a vbm model.\n", __FUNCTION__);
		return false;
	}

	// Get cache
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;

	Int32 sequence = pedict->state.sequence;
	if(sequence >= pstudiohdr->numseq)
		sequence = 0;

	// Get sequence data
	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequence);
	if(!pseqdesc)
		return false;

	Float frame = VBM_EstimateFrame(pseqdesc, pedict->state, svs.time);

	if(!pedict->pvbmhulldata)
		pedict->pvbmhulldata = new entity_vbmhulldata_t;

	// Reuse VBM trace info structures
	if(!TR_VBMCheckHullInfo(pedict->pvbmhulldata, pmodel, svs.time, pedict->state))
	{
		// Set animation state
		TR_VBMSetStateInfo(pedict->pvbmhulldata, pmodel, frame, pedict->state);

		for(Uint32 i = 0; i < MAX_MAP_HULLS; i++)
			pedict->pvbmhulldata->hulls[i].hullset = false;

		// Update bones
		TR_VBMSetupBones(pedict->pvbmhulldata, pstudiohdr, svs.time, frame, pseqdesc, pmodel, pedict->state);
	}

	// Find the bone
	if(boneindex >= static_cast<Uint32>(pstudiohdr->numbones))
	{
		Con_Printf("%s - Bone index %d is out of range.\n", __FUNCTION__, boneindex);
		return false;
	}

	// Set the values
	position[0] = pedict->pvbmhulldata->bonetransform[boneindex][0][3];
	position[1] = pedict->pvbmhulldata->bonetransform[boneindex][1][3];
	position[2] = pedict->pvbmhulldata->bonetransform[boneindex][2][3];
	
	return false;
}

//=============================================
//
//=============================================
bool SV_GetAttachment( edict_t* pedict, Uint32 index, Vector& position )
{
	if(index >= MAX_ATTACHMENTS)
	{
		Con_Printf("%s - Invalid attachment index %d specified.\n", __FUNCTION__, index);
		return false;
	}

	if(!pedict)
	{
		Con_Printf("%s - Entity was null.\n", __FUNCTION__);
		return false;
	}

	if(!pedict->state.modelindex)
	{
		Con_Printf("%s - Entity has no model.\n", __FUNCTION__);
		return false;
	}

	const cache_model_t* pmodel = gModelCache.GetModelByIndex(pedict->state.modelindex);
	if(pmodel->type != MOD_VBM)
	{
		Con_Printf("%s - Entity is not a vbm model.\n", __FUNCTION__);
		return false;
	}

	// Get cache
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;

	if(static_cast<Int32>(index) >= pstudiohdr->numattachments)
	{
		Con_Printf("%s - Model '%s' has %d attachments, index %d is out of bounds.\n", __FUNCTION__, pmodel->name.c_str(), pstudiohdr->numattachments, index);
		return false;
	}

	Int32 sequence = pedict->state.sequence;
	if(sequence >= pstudiohdr->numseq)
		sequence = 0;

	// Get sequence data
	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequence);
	if(!pseqdesc)
		return false;

	Float frame = VBM_EstimateFrame(pseqdesc, pedict->state, svs.time);

	if(!pedict->pvbmhulldata)
		pedict->pvbmhulldata = new entity_vbmhulldata_t;

	// Reuse VBM trace info structures
	if(!TR_VBMCheckHullInfo(pedict->pvbmhulldata, pmodel, svs.time, pedict->state))
	{
		// Set animation state
		TR_VBMSetStateInfo(pedict->pvbmhulldata, pmodel, frame, pedict->state);

		for(Uint32 i = 0; i < MAX_MAP_HULLS; i++)
			pedict->pvbmhulldata->hulls[i].hullset = false;

		// Update bones
		TR_VBMSetupBones(pedict->pvbmhulldata, pstudiohdr, svs.time, frame, pseqdesc, pmodel, pedict->state);
	}
	
	// Transform the attachment
	const mstudioattachment_t* pattachments = reinterpret_cast<const mstudioattachment_t*>(reinterpret_cast<const byte*>(pstudiohdr) + pstudiohdr->attachmentindex);
	Math::VectorTransform(pattachments[index].org, pedict->pvbmhulldata->bonetransform[pattachments[index].bone], position);

	return true;
}

//=============================================
//
//=============================================
const en_material_t* SV_GetMapTextureMaterial( const Char* pstrtexturename )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	for(Uint32 i = 0; i < svs.mapmaterialfiles.size(); i++)
	{
		if(!qstrcmp(svs.mapmaterialfiles[i].maptexturename, pstrtexturename))
		{
			return pTextureManager->FindMaterialScript(svs.mapmaterialfiles[i].materialfilepath.c_str(), RS_GAME_LEVEL);
		}
	}

	return nullptr;
}

//=============================================
//
//=============================================
void SV_PrecacheGeneric( const Char* pstrresourcename )
{
	for(Uint32 i = 0; i < svs.genericsourcesarray.size(); i++)
	{
		if(!qstrcmp(svs.genericsourcesarray[i], pstrresourcename))
			return;
	}

	if(!FL_FileExists(pstrresourcename))
	{
		Con_EPrintf("%s - File '%s' does not exist.\n", __FUNCTION__, pstrresourcename);
		return;
	}

	svs.genericsourcesarray.push_back(pstrresourcename);
}

//=============================================
//
//=============================================
Float SV_GetSoundDuration( const Char* pstrfilename, Uint32 pitch ) 
{
	// Calculate pitch adjustment
	Float pitchmod = static_cast<Float>(pitch)/static_cast<Float>(PITCH_NORM);

	// Look it up in precache list
	for(Uint32 i = 0; i < svs.sndcache.size(); i++)
	{
		if(!qstrcmp(svs.sndcache[i].filepath, pstrfilename))
			return svs.sndcache[i].duration * pitchmod;
	}

	CString filename(pstrfilename);
	filename.tolower();

	// Otherwise load it in and check
	if(filename.find(0, ".wav") != -1)
		return SV_GetWAVFileDuration(pstrfilename) * pitchmod;
	else if(filename.find(0, ".ogg") != -1)
		return SV_GetOGGFileDuration(pstrfilename) * pitchmod;
	else
	{
		Con_EPrintf("%s - Unknown sound file '%s'.\n", __FUNCTION__, pstrfilename);
		return 0;
	}
}

//=============================================
//
//=============================================
Float SV_GetOGGFileDuration( const Char* pstrfilename ) 
{
	CString filepath;
	filepath << SOUND_FOLDER_BASE_PATH << pstrfilename;

	Uint32 fileSize = 0;
	const byte *pfile = FL_LoadFile(filepath.c_str(), &fileSize);
	if(!pfile)
	{
		Con_EPrintf("%s - Could not load %s\n", __FUNCTION__, pstrfilename);
		return 0;
	}

	byte* pdata = new byte[fileSize];
	memcpy(pdata, pfile, sizeof(byte)*fileSize);
	FL_FreeFile(pfile);

	snd_oggcache_t* pogg = new snd_oggcache_t();
	pogg->pfileptr = pdata;
	pogg->filepath = pstrfilename;
	pogg->pcurptr = pogg->pfileptr;
	pogg->filesize = fileSize;

	// Ogg vorbis callback functions
	ov_callbacks oggCallbacks;
	oggCallbacks.read_func = AR_readOgg;
	oggCallbacks.seek_func = AR_seekOgg;
	oggCallbacks.close_func = AR_closeOgg;
	oggCallbacks.tell_func = AR_tellOgg;

	OggVorbis_File stream = OggVorbis_File();
	Int32 openResult = ov_open_callbacks(pogg, &stream, nullptr, 0, oggCallbacks);
	if(openResult < 0)
	{
		Con_Printf("%s - Could not open '%s' for streaming.\n", __FUNCTION__, pstrfilename);
		delete[] pdata;
		delete pogg;
		return 0;
	}

	Double result = ov_time_total(&stream, -1);

	// Release data
	ov_clear(&stream);

	delete[] pdata;
	delete pogg;

	if(result == OV_EINVAL)
	{
		Con_Printf("%s - Failed to get length for ogg file '%s'.\n", __FUNCTION__, pstrfilename);
		return 0;
	}
	else
	{
		return result;
	}
}

//=============================================
//
//=============================================
Float SV_GetWAVFileDuration( const Char* pstrfilename ) 
{
	CString filepath;
	filepath << SOUND_FOLDER_BASE_PATH << pstrfilename;

	Uint32 filesize = 0;
	const byte* pfile = FL_LoadFile(filepath.c_str(), &filesize);
	if(!pfile)
	{
		Con_Printf("%s - Failed to open '%s'.\n", __FUNCTION__, pstrfilename);
		return 0;
	}

	if(qstrncmp(reinterpret_cast<const Char*>(pfile), "RIFF", 4))
	{
		Con_Printf("%s - '%s' is not a valid WAV file.\n", __FUNCTION__, pstrfilename);
		FL_FreeFile(pfile);
		return 0;
	}

	const byte *pbegin = pfile + 12;
	const byte *pend = pfile + filesize;

	Uint32 numchannels = 0;
	Uint32 samplerate = 0;
	Uint32 bitspersample = 0;
	Uint32 datasize = 0;

	while(1)
	{
		if(pbegin >= pend)
			break;

		DWORD ilength = Common::ByteToInt32(pbegin+4);
		Common::ScaleByte(&ilength);

		if(!strncmp(reinterpret_cast<const char*>(pbegin), "fmt ", 4))
		{
			numchannels = Common::ByteToUint16(pbegin+10);
			samplerate = Common::ByteToInt32(pbegin+12);
			bitspersample = Common::ByteToUint16(pbegin+22);
		}

		if(!strncmp(reinterpret_cast<const char*>(pbegin), "data", 4))
		{
			datasize = Common::ByteToInt32(pbegin+4);
		}

		pbegin = pbegin + 8 + ilength;
	}

	FL_FreeFile(pfile);

	// calculate basic duration
	Uint32 bytepersec = numchannels * samplerate * (bitspersample>>3);
	return (Float)datasize / (Float)bytepersec;
}

//=============================================
//
//=============================================
Uint64 SV_GetModelFrameCount( Int32 modelindex )
{
	cache_model_t* pmodel = gModelCache.GetModelByIndex(modelindex);
	if(!pmodel)
		return 0;

	return Cache_GetModelFrameCount(*pmodel);
}

//=============================================
//
//=============================================
void SV_SetLevelSavefile( const Char* pstrLevelName, const Char* pstrSaveFileName )
{
	svs.levelinfos.begin();
	while(!svs.levelinfos.end())
	{
		sv_levelinfo_t& info = svs.levelinfos.get();
		if(!qstrcmp(info.mapname, pstrLevelName))
		{
			info.mapsavename = pstrSaveFileName;
			return;
		}

		svs.levelinfos.next();
	}

	Con_EPrintf("%s - Level info for level '%s' not found.\n", __FUNCTION__, pstrLevelName);
}

//=============================================
//
//=============================================
void SV_AddLevelConnection( const Char* pstrLevelName, const Char* pstrOtherLevelName, const Char* pstrLandmarkName, const Char* pstrMapSaveFileName )
{
	sv_levelinfo_t* plevelinfo = nullptr;

	// Make sure it's not already present
	svs.levelinfos.begin();
	while(!svs.levelinfos.end())
	{
		sv_levelinfo_t& levelconns = svs.levelinfos.get();
		if(!qstrcmp(levelconns.mapname, pstrLevelName))
		{
			// Set pointer
			plevelinfo = &levelconns;

			// Look to see if the connection is present already
			levelconns.connectionslist.begin();
			while(!levelconns.connectionslist.end())
			{
				sv_level_connection_t& conn = levelconns.connectionslist.get();
				if(!qstrcmp(conn.othermapname, pstrOtherLevelName) 
					&& !qstrcmp(conn.landmarkname, pstrLandmarkName))
				{
					Con_Printf("%s - Connection from level '%s' to level '%s' with landmark '%s' already registered.\n", __FUNCTION__, svs.mapname.c_str(), pstrOtherLevelName, pstrLandmarkName);
					return;
				}
				levelconns.connectionslist.next();
			}

			break;
		}

		svs.levelinfos.next();
	}

	// If not present, create it
	if(!plevelinfo)
	{
		sv_levelinfo_t newinfo;
		newinfo.mapname = pstrLevelName;
		newinfo.mapsavename = pstrMapSaveFileName;

		plevelinfo = &svs.levelinfos.radd(newinfo)->_val;
	}

	// Add the new connection
	sv_level_connection_t newconn;
	newconn.othermapname = pstrOtherLevelName;
	newconn.landmarkname = pstrLandmarkName;
	
	plevelinfo->connectionslist.radd(newconn);
}

//=============================================
//
//=============================================
void SV_BeginLevelChange( const Char* pstrOtherLevelName, const Char* pstrLandmarkName, const Vector& landmarkPosition )
{
	// Set the changelevel info
	svs.levelchangeinfo.nextlevelname = pstrOtherLevelName;
	svs.levelchangeinfo.landmarkname = pstrLandmarkName;
	svs.levelchangeinfo.landmarkposition = landmarkPosition;

	svs.levelchangeinfo.prevlevelname = ens.pworld->name;
	svs.levelchangeinfo.prevlevelsavefilename.clear();

	// Issue a command for the level change
	CString cmd;
	cmd << "changelevel " << pstrOtherLevelName << " " << pstrLandmarkName;

	// Issue the level change via command
	gCommands.AddCommand(cmd.c_str());

	// Stop audio playback
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ALL, svc_sndengine, nullptr);
		svs.netinfo.pnet->WriteByte(MSG_SNDENGINE_STOP_MUSIC);
		svs.netinfo.pnet->WriteInt16(MUSIC_CHANNEL_ALL);
		svs.netinfo.pnet->WriteByte(FALSE);
	svs.netinfo.pnet->SVC_MessageEnd();
}

//=============================================
//
//=============================================
void SV_PerformLevelChange( const Char* pstrlevelname, const Char* pstrlandmarkname )
{
	if(svs.levelchangeinfo.nextlevelname.empty() 
		|| svs.levelchangeinfo.landmarkname.empty()
		|| svs.levelchangeinfo.prevlevelname.empty() )
	{
		// Invalid attempt
		Con_Printf("%s - Levelchange info not set.\n", __FUNCTION__);
		return;
	}

	// Make sure the specified data matches
	if(qstrcmp(svs.levelchangeinfo.nextlevelname, pstrlevelname))
	{
		Con_Printf("%s - Mismatch in parameters for next level - Command: %s, Levelchange info: %s.\n", 
			__FUNCTION__, pstrlevelname, svs.levelchangeinfo.nextlevelname.c_str());
		return;
	}

	if(qstrcmp(svs.levelchangeinfo.landmarkname, pstrlandmarkname))
	{
		Con_Printf("%s - Mismatch in parameters for landmark - Command: %s, Levelchange info: %s.\n", 
			__FUNCTION__, pstrlandmarkname, svs.levelchangeinfo.landmarkname.c_str());
		return;
	}

	// See if we have a save file on the other level, also checks if this
	// is a valid connection
	bool connectionFound = false;

	// Find info for current level
	svs.levelinfos.begin();
	while(!svs.levelinfos.end())
	{
		sv_levelinfo_t& levelinfo = svs.levelinfos.get();
		if(!qstrcmp(levelinfo.mapname, svs.mapname))
		{
			// Find connection on this level to next one
			levelinfo.connectionslist.begin();
			while(!levelinfo.connectionslist.end())
			{
				sv_level_connection_t& connection = levelinfo.connectionslist.get();

				if(!qstrcmp(connection.othermapname, svs.levelchangeinfo.nextlevelname)
					&& !qstrcmp(connection.landmarkname, svs.levelchangeinfo.landmarkname))
				{
					connectionFound = true;
					break;
				}

				levelinfo.connectionslist.next();
			}
			break;
		}
		svs.levelinfos.next();
	}

	// Fail if connection wasn't found
	if(!connectionFound)
	{
		Con_Printf("%s - No connection with level name '%s' and landmark '%s' found.\n", 
			__FUNCTION__, svs.levelchangeinfo.nextlevelname.c_str(), svs.levelchangeinfo.landmarkname.c_str());
		return;
	}

	// Find next level's info if prsent
	CString nextlevelstatesave;

	svs.levelinfos.begin();
	while(!svs.levelinfos.end())
	{
		sv_levelinfo_t& levelinfo = svs.levelinfos.get();
		if(!qstrcmp(levelinfo.mapname, svs.levelchangeinfo.nextlevelname))
		{
			nextlevelstatesave = levelinfo.mapsavename;
			break;
		}
		svs.levelinfos.next();
	}

	// Notify client we're doing a level change
	CL_NotifyLevelChange();

	// Begin the loading plaque
	VID_BeginLoading(false);

	// First make sure the landmark entity is valid
	edict_t* plandmarkedict = nullptr;
	while(true)
	{
		plandmarkedict = svs.dllfuncs.pfnFindEntityByString(plandmarkedict, "targetname", svs.levelchangeinfo.landmarkname.c_str());
		if(!plandmarkedict)
			break;

		if(!qstrcmp(SV_GetString(plandmarkedict->fields.classname), "info_landmark"))
			break;
	}

	if(!plandmarkedict)
	{
		Con_Printf("%s - Landmark entity '%s' not found.\n", __FUNCTION__, svs.levelchangeinfo.landmarkname.c_str());
		return;
	}

	if(plandmarkedict->state.origin != svs.levelchangeinfo.landmarkposition)
	{
		Con_Printf("%s - Position mismatch with landmark entity '%s'.\n", __FUNCTION__, svs.levelchangeinfo.landmarkname.c_str());
		return;
	}

	// Find transition trigger entity
	edict_t* ptransitiontrigger = nullptr;
	while(true)
	{
		ptransitiontrigger = svs.dllfuncs.pfnFindEntityByString(ptransitiontrigger, "targetname", svs.levelchangeinfo.landmarkname.c_str());
		if(!ptransitiontrigger)
			break;

		if(!qstrcmp(SV_GetString(ptransitiontrigger->fields.classname), "trigger_transition"))
			break;
	}

	// If present, copy mins/maxs
	const Vector *ptransmins = nullptr, *ptransmaxs = nullptr;
	if(ptransitiontrigger)
	{
		ptransmins = &ptransitiontrigger->state.absmin;
		ptransmaxs = &ptransitiontrigger->state.absmax;
	}

	// Get PVS for landmark edict
	byte* ppvsbuffer = new byte[ens.visbuffersize];
	memset(ppvsbuffer, 0, sizeof(ens.visbuffersize));

	const mleaf_t* pleaf = nullptr;
	for(Uint32 i = 0; i < 4; i++)
	{
		Vector testPosition = svs.levelchangeinfo.landmarkposition + Vector(0, 0, static_cast<Float>(i));
		pleaf = Mod_PointInLeaf(testPosition, (*ens.pworld));
		if(!pleaf || pleaf->contents != CONTENTS_SOLID)
			break;
	}

	if(!pleaf || pleaf->contents == CONTENTS_SOLID)
	{
		Con_Printf("%s - Failed to get valid leaf for level change. The info_landmark might be in a solid.\n", __FUNCTION__, svs.levelchangeinfo.landmarkname.c_str());
		delete[] ppvsbuffer;
		SV_ClearLevelChange();
		SV_ClearGame();
		return;
	}

	// Retrieve VIS ptr and copy it to temp buffer
	const byte* ppvs = Mod_LeafPVS(ppvsbuffer, ens.visbuffersize, (*pleaf), (*ens.pworld));

	// Allow game DLL to modify PVS used for transition
	svs.dllfuncs.pfnAdjustLandmarkPVSData(plandmarkedict, ppvsbuffer, ens.visbuffersize);

	// Collect entities to transition
	if(!svs.dllfuncs.pfnGetTransitioningEntities(ppvs,
		ptransmins,
		ptransmaxs,
		svs.levelchangeinfo.landmarkname.c_str(), 
		svs.levelchangeinfo.landmarkposition, 
		svs.levelchangeinfo.transitionentitylist,
		svs.levelchangeinfo.numtransitionentities,
		MAX_TRANSITIONING_ENTITIES))
	{
		Con_Printf("%s - Level change failed.\n", __FUNCTION__);
		delete[] ppvsbuffer;
		SV_ClearLevelChange();
		SV_ClearGame();
		return;
	}

	// Collect transitioning decals
	if(!svs.saveddecalslist.empty())
	{
		svs.saveddecalslist.begin();
		while(!svs.saveddecalslist.end())
		{
			saveddecal_t& decal = svs.saveddecalslist.get();

			// Skip any decals already marked with this
			if(decal.flags & FL_DECAL_TRANSITIONED)
			{
				svs.saveddecalslist.next();
				continue;
			}

			// Mark decal as having transitioned for BOTH
			// current and next level, as we don't want it
			// to be re-transitioned if we return to the
			// previous level and THEN again to the level
			// we are currently changing to
			decal.flags |= FL_DECAL_TRANSITIONED;

			// Check transition trigger if present
			if(ptransmins && ptransmaxs && !Math::PointInMinsMaxs(decal.origin, (*ptransmins), (*ptransmaxs)))
			{
				svs.saveddecalslist.next();
				continue;
			}

			// Now check leaf
			const mleaf_t* pdecalleaf = nullptr;
			for(Uint32 i = 0; i < 4; i++)
			{
				Vector testPosition = decal.origin + Vector(0, 0, static_cast<Float>(i));
				pdecalleaf = Mod_PointInLeaf(testPosition, (*ens.pworld));
				if(!pdecalleaf || pdecalleaf->contents != CONTENTS_SOLID)
					break;
			}

			// Bogus decal, do not bother
			if(!pdecalleaf || pdecalleaf->contents == CONTENTS_SOLID)
			{
				svs.saveddecalslist.next();
				continue;
			}

			// Check if it's in PVS of landmark
			Int32 decalleafnum = pdecalleaf-ens.pworld->pleafs-1;
			if (!(ppvs[decalleafnum >> 3] & (1 << (decalleafnum&7) )))
			{
				svs.saveddecalslist.next();
				continue;
			}

			// Save decal as a transitioning decal
			saveddecal_t transdecal = decal;
			// Mark as having transitioned from another level
			decal.flags |= FL_DECAL_LEVEL_TRANSITION;

			svs.levelchangeinfo.transitiondecallist.add(transdecal);
			svs.saveddecalslist.next();
		}
	}

	// Delete vis buffer
	delete[] ppvsbuffer;

	CString savebasename;
	savebasename << svs.mapname << "-state";

	// Create save file of current map state
	CString savefilename;
	if(!gSaveRestore.CreateSaveFile(savebasename.c_str(), SAVE_MAPSAVE, nullptr, &savefilename))
	{
		Con_Printf("%s - Failed to write level state to save file.\n", __FUNCTION__);
		SV_ClearLevelChange();
		SV_ClearGame();
		return;
	}

	// Set the previous level's save file
	SV_SetLevelSavefile(svs.mapname.c_str(), savefilename.c_str());

	// Create save file for transitioning entities
	CString transitionsave;
	if(!gSaveRestore.CreateSaveFile("transition", SAVE_TRANSITION, &svs.levelchangeinfo.landmarkposition, &transitionsave))
	{
		Con_Printf("%s - Failed to write transitioning entities to save file.\n", __FUNCTION__);
		SV_ClearLevelChange();
		SV_ClearGame();
		return;
	}

	// Shut down the server and load the next level
	SV_ClearGame(false);

	// Spawn from transition save
	if(!SV_SpawnGame(svs.levelchangeinfo.nextlevelname.c_str(), nextlevelstatesave.empty() ? nullptr : nextlevelstatesave.c_str(), transitionsave.c_str(), false))
	{
		// As SpawnGame cannot clear the loading screen, do it here in case of error
		VID_EndLoading();
	}

	// Clear levelchange info
	SV_ClearLevelChange();
}

//=============================================
//
//=============================================
void SV_GetTransitionList( const Int32** pEntityList, Uint32& numEntities )
{
	(*pEntityList) = svs.levelchangeinfo.transitionentitylist;
	numEntities = svs.levelchangeinfo.numtransitionentities;
}

//=============================================
//
//=============================================
void SV_ClearLevelChange( void )
{
	svs.levelchangeinfo.nextlevelname.clear();
	svs.levelchangeinfo.landmarkname.clear();
	svs.levelchangeinfo.landmarkposition.Clear();
	svs.levelchangeinfo.prevlevelname.clear();
	svs.levelchangeinfo.prevlevelsavefilename.clear();

	if(!svs.levelchangeinfo.transitiondecallist.empty())
		svs.levelchangeinfo.transitiondecallist.clear();

	for(Uint32 i = 0; i < svs.levelchangeinfo.numtransitionentities; i++)
		svs.levelchangeinfo.transitionentitylist[i] = -1;

	svs.levelchangeinfo.numtransitionentities = 0;
}

//=============================================
//
//=============================================
void SV_ClearConnections( void )
{
	if(svs.levelinfos.empty())
		return;

	svs.levelinfos.clear();
}

//=============================================
//
//=============================================
void SV_EndGame( const Char* pstrEndGameCode )
{
	gCommands.AddCommand("disconnect");
}

//=============================================
//
//=============================================
bool SV_IsTransitionDecalValid( saveddecal_t& decal )
{
	// Skip decals tied to moving entities, as doors
	// might be inside a wall at the moment
	if(decal.pedict 
		&& (decal.pedict->state.movetype != MOVETYPE_NONE
		|| decal.pedict->state.takedamage == TAKEDAMAGE_YES))
		return true;

	// Check leaf
	const mleaf_t* pdecalleaf = nullptr;
	for(Uint32 i = 0; i < 4; i++)
	{
		Vector testPosition = decal.origin + Vector(0, 0, static_cast<Float>(i));
		pdecalleaf = Mod_PointInLeaf(testPosition, (*ens.pworld));
		if(!pdecalleaf || pdecalleaf->contents != CONTENTS_SOLID)
			break;
	}

	// Deal is inside a solid in this level, skip
	if(!pdecalleaf || pdecalleaf->contents == CONTENTS_SOLID)
		return false;

	return true;
}

//=============================================
//
//=============================================
void SV_RestoreSavedDecals( void )
{
	if(svs.saveddecalslist.empty())
		return;

	svs.saveddecalslist.begin();
	while(!svs.saveddecalslist.end())
	{
		saveddecal_t& decal = svs.saveddecalslist.get();
		if((decal.flags & FL_DECAL_LEVEL_TRANSITION) 
			&& !SV_IsTransitionDecalValid(decal))
		{
			svs.saveddecalslist.remove(svs.saveddecalslist.get_link());
			svs.saveddecalslist.next();
			continue;
		}

		Vector decalorigin = decal.origin;
		Vector decalnormal = decal.normal;

		if(decal.pedict)
		{
			// Rotate from entity space to world coordinates
			if(!decal.pedict->state.angles.IsZero())
			{
				Math::RotateFromEntitySpace(decal.pedict->state.angles, decalorigin);
				Math::RotateFromEntitySpace(decal.pedict->state.angles, decalnormal);
			}

			// Apply entity origin offset to decal origin
			if(!decal.pedict->state.origin.IsZero())
				Math::VectorAdd(decalorigin, decal.pedict->state.origin, decalorigin);
		}

		// Call server to create the decal
		svs.dllfuncs.pfnRestoreDecal(decalorigin, decalnormal, decal.pedict, decal.decaltexture.c_str(), decal.flags);

		// Go onto next one
		svs.saveddecalslist.next();
	}
}

//=============================================
//
//=============================================
void SV_AddSavedDecal( const Vector& origin, const Vector& normal, entindex_t entityindex, const Char* pstrDecalTexture, Int32 decalflags )
{
	edict_t* pedict = nullptr;
	Vector decalorg = origin;
	Vector decalnormal = normal;

	// Do not count worldspawn as a valid entity index
	entindex_t entindex = (entityindex == WORLDSPAWN_ENTITY_INDEX) ? NO_ENTITY_INDEX : entityindex;

	// Translate coordinates to local space
	if(entindex != NO_ENTITY_INDEX)
	{
		pedict = gEdicts.GetEdict(entindex);
		if(!pedict)
		{
			Con_Printf("%s - Failed to get edict with entindex %d.\n", __FUNCTION__, entindex);
			return;
		}

		// Only store entity pointer for entities that move
		// or take damage
		if((pedict->state.movetype != MOVETYPE_NONE
			|| pedict->state.takedamage == TAKEDAMAGE_YES)
			&& !(pedict->state.flags & FL_WORLDBRUSH))
		{
			// Move origin to entity space
			Math::VectorSubtract(decalorg, pedict->state.origin, decalorg);

			// Rotate it to entity local space
			if(!pedict->state.angles.IsZero())
			{
				Math::RotateToEntitySpace(pedict->state.angles, decalorg);
				Math::RotateToEntitySpace(pedict->state.angles, decalnormal);
			}
		}
		else
		{
			// No need to store edict for this decal
			pedict = nullptr;
		}
	}

	saveddecal_t newdecal;
	newdecal.origin = decalorg;
	newdecal.normal = decalnormal;
	newdecal.pedict = pedict;
	newdecal.decaltexture = pstrDecalTexture;
	newdecal.flags = decalflags;
	if(pedict)
		newdecal.identifier = pedict->identifier;

	svs.saveddecalslist.add(newdecal);
}

//=============================================
//
//=============================================
Int32 SV_NewCheckClient( Int32 check )
{
	Int32 _check = clamp(check, 1, (Int32)svs.maxclients);

	Int32 i = 1;
	if(_check != static_cast<Int32>(svs.maxclients))
		i = _check + 1;

	edict_t* pedict = nullptr;
	while(true)
	{
		if(i == static_cast<Int32>(svs.maxclients+1))
			i = 1;

		pedict = gEdicts.GetEdict(i);
		if(i == _check)
			break;

		if(!pedict->free && pedict->pprivatedata && !(pedict->state.flags & FL_NOTARGET))
			break;

		i++;
	}

	Vector origin;
	Math::VectorAdd(pedict->state.origin, pedict->state.view_offset, origin);

	const mleaf_t* pleaf = Mod_PointInLeaf(origin, *ens.pworld);
	if(!pleaf)
		return i;

	Mod_LeafPVS(svs.pvischeckbuffer, ens.visbuffersize, (*pleaf), (*ens.pworld));
	return i;
}

//=============================================
//
//=============================================
edict_t* SV_FindClientInPVS( const edict_t* pedict )
{
	if(svs.time - svs.lastpvschecktime > 0.1)
	{
		svs.lastpvscheckclient = SV_NewCheckClient(svs.lastpvscheckclient);
		svs.lastpvschecktime = svs.time;
	}

	edict_t* pclient = gEdicts.GetEdict(svs.lastpvscheckclient);
	if(!pclient->free && pclient->pprivatedata)
	{
		Vector view;
		Math::VectorAdd(pedict->state.origin, pedict->state.view_offset, view);

		const mleaf_t* pleaf = Mod_PointInLeaf(view, (*ens.pworld));
		Int32 i = (pleaf - ens.pworld->pleafs) - 1;
		if(i >= 0 && ((1<<(i&7)) & svs.pvischeckbuffer[i>>3]))
			return pclient;
	}

	return nullptr;
}