/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "enginestate.h"

#include "cl_entity.h"
#include "cl_main.h"
#include "cl_msg.h"
#include "cl_predict.h"
#include "cl_pmove.h"
#include "cl_resource.h"
#include "efxapi.h"
#include "cl_efx.h"

#include "sv_main.h"
#include "networking.h"
#include "system.h"
#include "usercmd.h"
#include "console.h"
#include "input.h"
#include "commands.h"
#include "cl_utils.h"
#include "r_interface.h"
#include "r_main.h"
#include "r_menu.h"
#include "modelcache.h"
#include "networking.h"
#include "vid.h"
#include "trace_shared.h"
#include "cl_snd.h"
#include "file_interface.h"
#include "file.h"
#include "entity_extrainfo.h"
#include "brushmodel.h"
#include "r_vbm.h"
#include "r_cables.h"
#include "flexmanager.h"
#include "cl_tempentities.h"
#include "vbmtrace.h"
#include "r_wadtextures.h"
#include "enginefuncs.h"

// client state structure
clientstate_t cls;

// holds the default field of view value
CCVar* g_pCvarDefaultFOV;
CCVar* g_pCvarReferenceFOV;
CCVar* g_pCvarName;
CCVar* g_pCvarPredictiton;

// Time before we try reconnecting again
static const Float RECONNECT_DELAY_TIME = 5.0f;
// Max attempts at reconnecting
static const Uint32 MAX_CONNECTION_RETRIES = 4;
// Amount of time we wait without getting messages, before sending a heartbeat to the server
static const Float HEARTBEAT_DELAY_TIME = 2.5f;

//
// Client dll engine functions
//
static cldll_engfuncs_t CLIENTDLL_ENGINE_FUNCTION_TABLE = 
{
	CL_GetClientTime,				//pfnGetClientTime
	Engine_GetEngineTime,			//pfnGetEngineTime
	CL_GetFrameTime,				//pfnGetFrameTime
	CL_GetLocalPlayer,				//pfnGetLocalPlayer
	CL_GetEntityByIndex,			//pfnGetEntityByIndex
	CL_GetViewAngles,				//pfnGetViewAngles
	CL_SetViewAngles,				//pfnSetViewAngles
	Con_Printf,						//pfnCon_Printf
	Con_DPrintf,					//pfnCon_DPrintf
	Con_VPrintf,					//pfnCon_VPrintf
	Con_EPrintf,					//pfnCon_EPrintf
	Sys_ErrorPopup,					//pfnErrorPopup
	Engine_CreateCommand,			//pfnCreateCommand
	Engine_Cmd_Argc,				//pfnCmd_Argc
	Engine_Cmd_Argv,				//pfnCmd_Argv
	CL_GetMousePosition,			//pfnGetMousePosition
	CL_GetMouseDelta,				//pfnGetMouseDelta
	CL_GetMoveVars,					//pfnGetMoveVars
	Cache_GetModelType,				//pfnGetModelType
	Cache_GetModelBounds,			//pfnGetModelBounds
	Cache_GetModel,					//pfnGetModel
	Cache_GetModelByName,			//pfnGetModelByName
	Cache_GetNbModels,				//pfnGetNbModels
	CL_GetNumEntities,				//pfnGetNumEntities
	CL_GetMaxClients,				//pfnGetMaxClients
	CL_PlayEntitySound,				//pfnPlayEntitySound
	CL_PlayAmbientSound,			//pfnPlayAmbientSound
	CL_PrecacheSound,				//pfnPrecacheSound
	CL_ApplySoundEffect,			//pfnApplySoundEffect
	CL_GetSoundFileForServerIndex,	//pfnGetSoundFileForServerIndex
	CL_PrecacheDecal,				//pfnPrecacheDecal
	CL_PrecacheDecalGroup,			//pfnPrecacheDecalGroup
	CL_PlayMusic,					//pfnPlayMusic
	CL_StopMusic,					//pfnStopMusic
	Engine_GetMaterialScript,		//pfnGetMaterialScript
	CL_GetMapTextureMaterial,		//pfnGetMapTextureMaterialScript
	R_AddEntity,					//pfnAddEntity
	CL_LeafPVS,						//pfnLeafPVS
	Engine_PointInLeaf,				//pfnPointInLeaf
	Engine_LoadModel,				//pfnLoadModel
	CL_GetEntityExtraData,			//pfnGetEntityExtraData
	Mod_FindTouchedLeafs,			//pfnFindTouchedLeafs
	Mod_RecursiveLightPoint,		//pfnRecursiveLightPoint
	CL_PrecacheFlexScript,			//pfnPrecacheFlexScript
	CL_SetFlexScript,				//pfnSetFlexScript
	CL_GetAttachment,				//pfnGetAttachment
	CL_GetBonePosition,				//pfnGetBonePosition
	CL_UpdateAttachments,			//pfnUpdateAttachments
	CL_ServerCommand,				//pfnServerCommand
	CL_ClientCommand,				//pfnClientCommand
	CL_ShowMouse,					//pfnShowMouse
	CL_HideMouse,					//pfnHideMouse
	CL_SetShouldHideMouse,			//pfnSetShouldHideMouse
	CL_UpdateMousePositions,		//pfnUpdateMousePositions
	CL_ResetMouse,					//pfnResetMouse
	CL_RegisterClientUserMessage,	//pfnRegisterClientUserMessage
	CL_ClientUserMessageBegin,		//pfnClientUserMessageBegin
	CL_ClientUserMessageEnd,		//pfnClientUserMessageEnd
	CL_Msg_WriteByte,				//pfnMsgWriteByte
	CL_Msg_WriteChar,				//pfnMsgWriteChar
	CL_Msg_WriteInt16,				//pfnMsgWriteInt16
	CL_Msg_WriteUint16,				//pfnMsgWriteUint16
	CL_Msg_WriteInt32,				//pfnMsgWriteInt32
	CL_Msg_WriteUint32,				//pfnMsgWriteUint32
	CL_Msg_WriteInt64,				//pfnMsgWriteInt64
	CL_Msg_WriteUint64,				//pfnMsgWriteUint64
	CL_Msg_WriteSmallFloat,			//pfnMsgWriteSmallFloat
	CL_Msg_WriteFloat,				//pfnMsgWriteFloat
	CL_Msg_WriteDouble,				//pfnMsgWriteDouble
	CL_Msg_WriteBuffer,				//pfnMsgWriteBuffer
	CL_Msg_WriteString,				//pfnMsgWriteString
	CL_Msg_WriteEntindex,			//pfnMsgWriteEntindex
	Engine_CreateCVar,				//pfnCreateCVar
	Engine_CreateCVarCallback,		//pfnCreateCVarCallback
	Engine_GetCVarPointer,			//pfnGetCVarPointer
	Engine_SetCVarFloat,			//pfnSetCVarFloat
	Engine_SetCVarString,			//pfnSetCVarString
	Engine_GetCvarFloatValue,		//pfnGetCvarFloatValue
	Engine_GetCvarStringValue,		//pfnGetCvarStringValue
	CL_GetSchemaFontSet,			//pfnGetSchemaFontSet
	CL_GetResolutionSchemaFontSet,	//pfnGetResolutionSchemaFontSet
	CL_SetPaused,					//pfnSetPaused
};

//
// Client DLL traceline functions
//
trace_interface_t CLIENTDLL_TRACE_FUNCTIONS =
{
	CL_TestPlayerPosition,		//pfnTestPlayerPosition
	CL_PointContents,			//pfnPointContents
	CL_TruePointContents,		//pfnTruePointContents
	TR_HullPointContents,		//pfnHullPointContents
	CL_PlayerTrace,				//pfnPlayerTrace
	CL_TraceLine,				//pfnTraceLine
	CL_HullForBSP,				//pfnHullForBSP
	CL_TraceModel,				//pfnTraceModel
	CL_TraceTexture				//pfnTraceTexture
};

//=============================================
//
//=============================================
bool CL_Init( void )
{
	// Try to load the client dll
	CString fulldllpath;
	fulldllpath << ens.gamedir << PATH_SLASH_CHAR << CLIENT_DLL_PATH;

	cls.pdllhandle = SDL_LoadObject(fulldllpath.c_str());
	if(!cls.pdllhandle)
	{
		Sys_ErrorPopup("Failed to load '%s'.\n", fulldllpath.c_str());
		return false;
	}

	// Init the gamedll interface
	pfnClientDLLInit_t pfnCLDLLInit = reinterpret_cast<pfnClientDLLInit_t>(SDL_LoadFunction(cls.pdllhandle, "ClientDLL_Init"));
	if(!pfnCLDLLInit)
	{
		Sys_ErrorPopup("Failed to hook 'ClientDLL_Init' in client dll.\n");
		return false;
	}

	// Trace interface
	trace_interface_t traceFuncs;
	traceFuncs.pfnTestPlayerPosition = CL_TestPlayerPosition;
	traceFuncs.pfnHullForBSP = CL_HullForBSP;
	traceFuncs.pfnHullPointContents = TR_HullPointContents;
	traceFuncs.pfnPointContents = CL_PointContents;
	traceFuncs.pfnTraceModel = CL_TraceModel;
	traceFuncs.pfnPlayerTrace = CL_PlayerTrace;
	traceFuncs.pfnTraceLine = CL_TraceLine;
	traceFuncs.pfnTraceTexture = CL_TraceTexture;
	traceFuncs.pfnTruePointContents = CL_TruePointContents;

	// Prepare render interface
	r_interface_t renderFuncs;
	R_InitRenderInterface(renderFuncs);

	cl_efxapi_t efxAPI;
	CL_InitEffectsInterface(efxAPI);

	// Init the game dll-engine interface
	if(!pfnCLDLLInit(CLDLL_INTERFACE_VERSION, cls.dllfuncs, CLIENTDLL_TRACE_FUNCTIONS, FL_GetInterface(), CLIENTDLL_ENGINE_FUNCTION_TABLE, efxAPI, renderFuncs))
	{
		Sys_ErrorPopup("Failed to init client dll.\n");
		return false;
	}

	// Initiate entities array
	Uint32 maxEntities;
	if(ens.arg_max_edicts != 0)
		maxEntities = ens.arg_max_edicts;
	else
		maxEntities = DEFAULT_MAX_EDICTS;

	// Set the array size
	cls.entities.resize(maxEntities);

	// Allocate usercmd history for prediction
	cls.usercmdhistory.resize(USERCMD_HISTORY_ALLOC_SIZE);
	cls.curusercmdidx = 0;

	// Init cvars
	CString defaultfovvalue;
	defaultfovvalue << (Int32)DEFAULT_FOV_VALUE;

	CString referencefovvalue;
	referencefovvalue << (Int32)REFERENCE_FOV_VALUE;

	g_pCvarDefaultFOV = Engine_CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, DEFAULT_FOV_CVAR_NAME, defaultfovvalue.c_str(), "Default field of view value.");
	g_pCvarReferenceFOV = Engine_CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, REFERENCE_FOV_CVAR_NAME, referencefovvalue.c_str(), "Reference field of view value.");
	g_pCvarName = Engine_CreateCVar(CVAR_STRING, (FL_CV_CLIENT|FL_CV_SAVE), "name", "Chris Cornell", "Player's name.");
	g_pCvarPredictiton = Engine_CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "cl_predict", "1", "Toggles player prediction.");

	// Init commands
	CL_InitCommands();

	// Init tempentities
	if(!gTempEntities.Init())
		return false;

	// Initialize renderer-related stuff
	if(!R_Init())
		return false;

	// Initialize sound engine
	if(!gSoundEngine.Init())
		return false;

	// Call game dll to initialize
	if(!cls.dllfuncs.pfnClientDLLInit())
	{
		Sys_ErrorPopup("Failed to init client dll.\n");
		return false;
	}

	return true;
}

//=============================================
//
//=============================================
void CL_Shutdown( void )
{
	// Disconnect client if connected
	if(cls.cl_state != CLIENT_INACTIVE)
		CL_Disconnect();

	// Shutdown renderer
	R_Shutdown();

	// Clear tempents
	gTempEntities.Shutdown();

	// Call shutdown on game dll
	cls.dllfuncs.pfnClientDLLShutdown();

	// Unload the game dll
	if(cls.pdllhandle != nullptr)
	{
		SDL_UnloadObject(cls.pdllhandle);
		cls.pdllhandle = nullptr;
	}
}

//=============================================
//
//=============================================
void CL_ResetGame( void )
{
	// Reset the renderer
	R_ResetGame();

	// Clear tempents
	gTempEntities.ClearGame();

	// Clear extrainfos
	CL_ClearExtraInfos();

	// Clears entities
	CL_ClearEntities();

	// Reset this
	cls.skyname.clear();

	// Call game reset function
	cls.dllfuncs.pfnGameReset();

	// Clear links
	svs.mapmaterialfiles.clear();

	// Make sure this is deleted
	if(ens.pwadresource)
	{
		delete ens.pwadresource;
		ens.pwadresource = nullptr;
	}

	// Clear model lights
	for(Uint32 i = 0; i < MAX_ENTITY_LIGHTS; i++)
		cls.entitylights[i] = entitylight_t();

	// Make sure this is reset
	gMenu.SetShouldHideMouse(true);
	// Make sure continue button is reset
	gMenu.UpdateContineButton();

	if(cls.netinfo.pnet && CL_IsHostClient())
		cls.netinfo.pnet = nullptr;

	if(ens.pfileiologfile)
	{
		CString msg;
		msg << "Game reset.\n";
		ens.pfileiologfile->Write(msg.c_str());
	}
}

//=============================================
//
//=============================================
void CL_CleanUserCmdHistory( Uint64 lastsvusercmdindex )
{
	Uint64 i = 0;
	for(; i < cls.usercmdhistorynum; i++)
	{
		if(cls.usercmdhistory[i].cmdidx >= lastsvusercmdindex)
			break;
	}

	if(i == cls.usercmdhistorynum)
		return;

	// Remove any outdated usercmds
	Uint64 numremove = i + 1;
	for(Uint64 j = 0; j < numremove; j++)
		memcpy(&cls.usercmdhistory[j], &cls.usercmdhistory[i+j+1], sizeof(usercmd_t));

	cls.usercmdhistorynum -= numremove;
}

//=============================================
//
//=============================================
void CL_AddUserCmd( const usercmd_t& cmd )
{
	// Allocate more usercmd history if we need to
	if(cls.usercmdhistorynum >= cls.usercmdhistory.size())
		cls.usercmdhistory.resize(cls.usercmdhistory.size() + USERCMD_HISTORY_ALLOC_SIZE);

	// Add this usercmd to the history for prediction
	usercmd_t& destcmd = cls.usercmdhistory[cls.usercmdhistorynum];
	cls.usercmdhistorynum++;

	// Copy values
	destcmd.cmdidx = cmd.cmdidx;
	destcmd.buttons = cmd.buttons;
	destcmd.forwardmove = cmd.forwardmove;
	destcmd.lerp_msec = cmd.lerp_msec;
	destcmd.msec = cmd.msec;
	destcmd.sidemove = cmd.sidemove;
	destcmd.upmove = cmd.upmove;
	destcmd.viewangles = cmd.viewangles;
}

//=============================================
//
//=============================================
void CL_SendCmd( void )
{
	// Call this before sending cmds
	cls.dllfuncs.pfnClientPreCmdThink();

	// No command sends on inactive clients
	if(cls.cl_state != CLIENT_ACTIVE)
		return;
	
	// Do not predict if we've lost connection
	if(cls.netinfo.pnet->GetClientState(0) != NETCL_CONNECTED)
		return;

	// Clear
	cls.cmd = usercmd_t();

	// Do not run commands if control is not in the game
	if(cls.paused)
	{
		// Don't take mouse movement away from the menu
		if(!gMenu.IsActive() && !cls.dllfuncs.pfnIsMouseOverridden())
			gInput.UpdateMousePositions(false);
		
		return;
	}

	// Add in keyboard movement
	cls.dllfuncs.pfnInMove(cls.cmd);

	if(Sys_IsGameControlActive())
	{
		// Add in mouse movement
		cls.dllfuncs.pfnMouseMove(cls.cmd);
	}

	cls.cmd.lerp_msec = ens.frametime*SECONDS_TO_MILLISECONDS;
	cls.cmd.msec = ens.frametime*SECONDS_TO_MILLISECONDS;

	// Write the usercmd to the server
	if(cls.netinfo.pnet->GetClientState(0) != NETCL_CONNECTED)
		return;

	// Set usercmd index
	cls.cmd.cmdidx = cls.curusercmdidx;
	cls.curusercmdidx++;

	cls.netinfo.pnet->CLS_MessageBegin(cls_usercmd);
	{
		cls.netinfo.pnet->WriteUint64(cls.cmd.cmdidx);
		cls.netinfo.pnet->WriteUint32(cls.cmd.lerp_msec);
		cls.netinfo.pnet->WriteByte(cls.cmd.msec);
	
		for(Uint32 i = 0; i < 3; i++)
			cls.netinfo.pnet->WriteFloat(cls.cmd.viewangles[i]);

		cls.netinfo.pnet->WriteFloat(cls.cmd.forwardmove);
		cls.netinfo.pnet->WriteFloat(cls.cmd.sidemove);
		cls.netinfo.pnet->WriteFloat(cls.cmd.upmove);
		cls.netinfo.pnet->WriteByte(cls.cmd.weaponselect);
		cls.netinfo.pnet->WriteByte(cls.cmd.impulse);
		cls.netinfo.pnet->WriteUint32(cls.cmd.buttons);
	}
	cls.netinfo.pnet->CLS_MessageEnd();

	// Add it to the history
	CL_AddUserCmd(cls.cmd);
}

//=============================================
//
//=============================================
void CL_Frame( void )
{
	// Check for reconnection attempts
	if(cls.netinfo.connecting)
	{
		if(cls.netinfo.nextretrytime <= ens.time)
		{
			if(!CL_EstablishConnection(nullptr, true))
				return;
		}
	}

	// Don't run client functions if not active
	if(cls.cl_state == CLIENT_INACTIVE)
		return;
	
	if(CL_IsHostClient())
	{
		// Sync server and client time for host client
		cls.cl_time = svs.time;
	}
	else
	{
		// non-host clients need to keep track of
		// client time on their own, and server
		// timers are compensated for
		if(!cls.paused && cls.cl_clsvtime)
			cls.cl_time += ens.frametime;
	}

	// Set frametime
	cls.frametime = ens.frametime;

	// See if we had any network errors
	netcl_state_t clstate = cls.netinfo.pnet->GetClientState(0);
	if(clstate != NETCL_CONNECTED)
	{
		switch(clstate)
		{
		case NETCL_NET_ERROR:
		case NETCL_DISCONNECTED:
			cls.netinfo.pnet->SVC_ClearMessages();
			return;

		case NETCL_LOST_CONNECTION:
			if(!cls.hasclientdata || !cls.hasentitydata || !cls.hasallresources || !cls.hasserverdata || !cls.hasplayerentitydata)
			{
				// Disconnect if we haven't fully connected and we lost the connection
				CL_Disconnect();
			}

			// Update framecount
			cls.framecount++;

			// Otherwise wait for a reconnection
			return;
		}
	}

	// Read messages from the server
	if(!CL_ReadMessages())
	{
		CL_Disconnect();
		return;
	}

	if(CL_IsGameActive())
	{
		// Update entity lights
		CL_UpdateEntityLights();

		// Update parented entities
		CL_UpdateParentedEntities();
	}

	if(CL_IsGameActive() && clstate == NETCL_CONNECTED)
	{
		if(cls.netinfo.pnet->GetLastMessageTime(0) > HEARTBEAT_DELAY_TIME)
		{
			// Check on server state
			CL_SendHeartbeat(false);
		}
	}

	// Call think functions on client
	cls.dllfuncs.pfnClientFrame();

	// Update framecount
	cls.framecount++;
}

//=============================================
//
//=============================================
void CL_UpdateSound( void )
{
	// Use the view info set up by the renderer part
	ref_params_t *pparams = &rns.view.params;

	// Set current time
	pparams->time = cls.cl_time;

	// Run sound engine functions
	gSoundEngine.Update(pparams);
}

//=============================================
//
//=============================================
bool CL_InitGame( void )
{
	// Needs to be set before all else
	cls.cl_state = CLIENT_ACTIVE;

	// Set worldmodel
	cache_model_t* pworld = gModelCache.GetModelByIndex(WORLD_MODEL_INDEX);
	if(!pworld)
	{
		Con_EPrintf("Worldmodel not loaded.\n");
		return true;
	}

	// Set states for game
	ens.pworld = pworld->getBrushmodel();

	// Set the world entity
	if(!cls.numentities)
		cls.numentities = 1;

	cl_entity_t* pworldentity = CL_GetEntityByIndex(WORLDSPAWN_ENTITY_INDEX);
	pworldentity->pmodel = pworld;
	pworldentity->curstate.modelindex = 1;
	pworldentity->entindex = 0;

	// Create initial extrainfos
	CL_InitExtraInfos();

	// Call client function before initializing the renderer
	if(!cls.dllfuncs.pfnGameInit())
		return false;

	CArray<CString> wadFilesList;
	if(!Common::GetWADList(ens.pworld->pentdata, wadFilesList))
	{
		Con_EPrintf("%s - Failed to get WAD list for '%s'.\n", __FUNCTION__, ens.pworld->name.c_str());
		wadFilesList.clear();
	}

	// Manage WAD resource object - This will have already been set
	// by server for localhost, so in that case ens.pwadresource will
	// be already set
	if(!ens.pwadresource)
	{
		// Update loading screen
		VID_DrawLoadingScreen("Loading WAD files...");

		// Legacy texture managing object
		ens.pwadresource = new CWADTextureResource();
		if (!ens.pwadresource->Init(
			ens.pworld->name.c_str(),
			wadFilesList,
			(g_pCvarWadTextureChecks->GetValue() >= 1) ? true : false,
			(g_pCvarBspTextureChecks->GetValue() >= 1) ? true : false))
		{
			// This is done for non-localhosts
			CL_LinkMapTextureMaterials(wadFilesList);
		}
		else
		{
			// Log failure
			Con_EPrintf("%s - Failed to initialize WAD resources.\n", __FUNCTION__);
		}
	}
	else
	{
		// Copy mappings from server for localhost
		cls.mapmaterialfiles = svs.mapmaterialfiles;
	}

	// Call renderer to initialize
	if(!R_InitGame())
	{
		CL_Disconnect();
		return false;
	}

	// Set the sound engine
	gSoundEngine.InitGame();

	// If we're the local player, call server entity init function
	if(CL_IsHostClient())
	{
		if(!SV_InitGame())
			return false;
	}

	// Delete wad resource after launch
	if(ens.pwadresource)
	{
		delete ens.pwadresource;
		ens.pwadresource = nullptr;
	}

	// Force a re-read of client messages
	CL_ReadMessages();

	// Only mark this after loading everything
	ens.gamestate = GAME_RUNNING;

	// If menu is visible, hide it
	if(gMenu.IsActive())
		gMenu.HideMenu();

	// Tell engine to skip rendering this frame, so
	// server-side things we've affected in SV_InitGame will send
	// proper data
	rns.numskipframes = g_pCvarSkipFrames->GetValue();

	if(ens.pfileiologfile)
	{
		CString msg;
		msg << "Game initialized on " << ens.pworld->name << ".\n";
		ens.pfileiologfile->Write(msg.c_str());
	}

	return true;
}

//=============================================
//
//=============================================
bool CL_EstablishConnection( const Char* pstrhost, bool reconnect )
{
	// Watch out for null ptr
	if(!reconnect && (!pstrhost || !qstrlen(pstrhost)))
	{
		Con_Printf("No host specified.\n");
		return false;
	}

	// Disconnect from any current servers
	if(!reconnect && cls.cl_state != CLIENT_INACTIVE)
		CL_Disconnect();

	// Try to connect again if we failed before
	if(reconnect)
	{
		Con_Printf("Retrying connection.\n", pstrhost);

		// Try again
		if(!cls.netinfo.pnet->AttemptConnection())
		{
			cls.netinfo.nextretrytime = ens.time + RECONNECT_DELAY_TIME;
			cls.netinfo.numretries++;

			// Check the limit
			if(cls.netinfo.numretries == MAX_CONNECTION_RETRIES)
			{
				CNetworking::DeleteInstance();
				cls.netinfo.pnet = nullptr;

				cls.netinfo.numretries = 0;
				cls.netinfo.connecting = false;
				cls.netinfo.nextretrytime = 0;

				Con_Printf("Failed to connect.\n");
			}

			return false;
		}
		
		// If all went well, begin the loading screen
		VID_BeginLoading(true);
	}
	else
	{
		// Set us as attempting to connect
		cls.netinfo.connecting = true;
		cls.netinfo.numretries = 0;

		if(qstrcmp(pstrhost, "localhost"))
		{
			// Allocate a client network structure
			cls.netinfo.pnet = CNetworking::CreateInstance(true);

			// Try connecting
			Con_Printf("Connecting to %s.\n", pstrhost);
			if(!cls.netinfo.pnet->Init(pstrhost))
			{
				cls.netinfo.nextretrytime = ens.time + RECONNECT_DELAY_TIME;
				return false;
			}

			VID_BeginLoading(true);
		}
		else
		{
			// Server already created an instance
			cls.netinfo.pnet = CNetworking::GetInstance();

			// Resources are already loaded by this point for localhost
			cls.hasallresources = true;
		}
	}

	// Set connected state for client
	cls.cl_state = CLIENT_CONNECTED;

	// Send player info to server
	CL_SendPlayerInfo();

	// Reset these
	cls.netinfo.connecting = false;
	cls.netinfo.nextretrytime = 0;
	cls.netinfo.numretries = 0;
	
	// Call cldll function
	cls.dllfuncs.pfnClientConnected();

	return true;
}

//=============================================
//
//=============================================
void CL_Disconnect( bool clearserver, bool clearloadingplaque )
{
	// Only run this if we're active
	if(cls.cl_state == CLIENT_INACTIVE)
	{
		Con_Printf("Not connected.\n");
		return;
	}

	// Reset the variables
	cls.cl_state = CLIENT_INACTIVE;
	cls.clientindex = 0;
	cls.cl_time = 0;
	cls.cl_clsvtime = 0;
	cls.framecount = 0;

	cls.parsecount = 0;
	cls.curusercmdidx = 0;
	cls.usercmdhistorynum = 0;
	cls.maxclients = 0;

	// Reset these flags
	cls.hasallresources = false;
	cls.hasclientdata = false;
	cls.hasentitydata = false;
	cls.hasserverdata = false;
	cls.hasplayerentitydata = false;
	cls.paused = false;

	// Call function on client
	cls.dllfuncs.pfnClientDisconnected();

	// Manage disconnection with remote host
	if(!CL_IsHostClient())
	{
		// Tell server we're out
		if(cls.netinfo.pnet->GetClientState(0) == NETCL_CONNECTED)
		{
			cls.netinfo.pnet->CLS_MessageBegin(cls_disconnect);
				cls.netinfo.pnet->WriteByte(false); // error flag
			cls.netinfo.pnet->CLS_MessageEnd();
		}

		// Print the info string locally
		const Char* pstrInfo = cls.netinfo.pnet->GetInfoString(0);
		if(pstrInfo[0] != '\0')
			Con_Printf("Disconnected from server - %s.\n", pstrInfo);
		else
			Con_Printf("Disconnected from server.\n");

		// Disconnect from server
		cls.netinfo.pnet->Disconnect(0);
		CNetworking::DeleteInstance();
		cls.netinfo.pnet = nullptr;

		// Set this
		ens.gamestate = GAME_INACTIVE;
	}
	else
	{
		Con_Printf("Disconnected.\n");

		if(clearserver)
			SV_ClearGame();
	}

	// Clear downloads
	CL_ClearResources();

	// Reset the game
	CL_ResetGame();

	if(clearloadingplaque)
	{
		// Remove loading plaque
		VID_EndLoading();
	}

	// Reset game sounds
	gSoundEngine.ResetGame();

	// Reset this
	ens.gamestate = GAME_INACTIVE;
}

//=============================================
//
//=============================================
bool CL_CheckGameReady ( void )
{
	if(!cls.hasclientdata || !cls.hasentitydata || !cls.hasallresources || !cls.hasserverdata)
		return true;

	// Keep track of network errors
	if(cls.netinfo.pnet->GetClientState(0) != NETCL_CONNECTED)
		return true;

	if(!cls.hasplayerentitydata)
	{
		// Tell the server we're ready to spawn
		cls.netinfo.pnet->CLS_MessageBegin(cls_clientready);
		cls.netinfo.pnet->CLS_MessageEnd();
		return true;
	}

	if(!CL_InitGame())
	{
		Con_Printf("Failure at game initialization.\n");
		return false;
	}

	// Disable load plaque
	VID_EndLoading();

	// Reset mouse position
	gInput.ResetMouse();

	return true;
}

//=============================================
//
//=============================================
bool CL_IsGameActive( void )
{
	return (ens.gamestate == GAME_RUNNING) ? true : false;
}

//=============================================
//
//=============================================
bool CL_CanPlayGameSounds( void )
{
	return (ens.gamestate == GAME_RUNNING && !ens.isloading) ? true : false;
}

//=============================================
//
//=============================================
bool CL_IsHostClient( void )
{
	return (svs.serverstate == SV_ACTIVE) ? true : false;
}

//=============================================
//
//=============================================
void CL_SendHeartbeat( bool prompt )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("Not connected.\n");
		return;
	}

	// set ping time
	cls.cl_pingtime = cls.cl_time;

	cls.netinfo.pnet->CLS_MessageBegin(cls_heartbeat);
		cls.netinfo.pnet->WriteByte(prompt);
	cls.netinfo.pnet->CLS_MessageEnd();
}

//====================================
//
//====================================
void CL_SetEntityTypeData( cl_entity_t* pentity, entity_extrainfo_t* pinfo, entity_type_t type )
{
	if(type == ENTITY_TYPE_VBM)
	{
		if(!pinfo->paniminfo)
			pinfo->paniminfo = new entity_animinfo_t();

		if(!pinfo->pflexstate)
		{
			pinfo->pflexstate = new flexstate_t();
			const vbmcache_t* pcache = pentity->pmodel->getVBMCache();

			if(pcache->pvbmhdr && pcache->pstudiohdr && pcache->pstudiohdr->flags & STUDIO_MF_HAS_FLEXES)
			{
				CFlexManager* pFlexManager = gVBMRenderer.GetFlexManager();
				if(pFlexManager)
					pFlexManager->SetFlexMappings(pcache->pvbmhdr, pinfo->pflexstate);
			}
		}

		if(!pinfo->plightinfo)
		{
			pinfo->plightinfo = new entity_lightinfo_t();
			pinfo->plightinfo->flags |= MDL_LIGHT_FIRST;
		}
	}
	else
	{
		if(pinfo->paniminfo)
		{
			delete pinfo->paniminfo;
			pinfo->paniminfo = nullptr;
		}

		if(pinfo->pflexstate)
		{
			delete pinfo->pflexstate;
			pinfo->pflexstate = nullptr;
		}

		if(pinfo->plightinfo)
		{
			delete pinfo->plightinfo;
			pinfo->plightinfo = nullptr;
		}
	}
}

//=============================================
//
//=============================================
entity_extrainfo_t* CL_GetEntityExtraData( cl_entity_t* pentity )
{
	if(!pentity)
		return nullptr;

	if(pentity->entindex < 0)
		return nullptr;

	// Get the entity type on the fly, in case it changes
	entity_type_t type;
	if(pentity->pmodel && pentity->pmodel->type == MOD_VBM)
		type = ENTITY_TYPE_VBM;
	else
		type = ENTITY_TYPE_BASIC;

	if(pentity->pextradata)
	{
		// Make sure the typedata is set up properly
		entity_extrainfo_t* pextrainfo = pentity->pextradata;
		CL_SetEntityTypeData(pentity, pextrainfo, type);

		return pextrainfo;
	}
	
	// First try to find an entry with this entindex
	entity_extrainfo_t* pnewinfo = nullptr;
	for(Uint32 i = 0; i < cls.numextrainfos; i++)
	{
		if(cls.entityextrainfos[i]->entindex == pentity->entindex)
		{
			pnewinfo = cls.entityextrainfos[i];
			break;
		}
	}

	if(!pnewinfo)
	{
		// Expand array if needed
		if(cls.numextrainfos == cls.entityextrainfos.size())
		{
			Uint32 origsize = cls.entityextrainfos.size();
			cls.entityextrainfos.resize(cls.entityextrainfos.size()+EXTRAINFO_ALLOC_SIZE);

			for(Uint32 i = 0; i < EXTRAINFO_ALLOC_SIZE; i++)
				cls.entityextrainfos[origsize+i] = new entity_extrainfo_t;
		}

		// Allocate a new entry
		pnewinfo = cls.entityextrainfos[cls.numextrainfos];
		cls.numextrainfos++;
	}

	pnewinfo->entindex = pentity->entindex;
	pnewinfo->type = type;
	pnewinfo->pentity = pentity;
	pentity->pextradata = pnewinfo;

	// Make sure the typedata is set up properly
	CL_SetEntityTypeData(pentity, pnewinfo, type);

	return pnewinfo;
}

//=============================================
//
//=============================================
void CL_InitExtraInfos( void )
{
	cls.entityextrainfos.resize(EXTRAINFO_ALLOC_SIZE);

	for(Uint32 i = 0; i < EXTRAINFO_ALLOC_SIZE; i++)
		cls.entityextrainfos[i] = new entity_extrainfo_t;
}

//=============================================
//
//=============================================
void CL_ClearExtraInfos( void )
{
	if(cls.entityextrainfos.empty())
		return;

	for(Uint32 i = 0; i < cls.entityextrainfos.size(); i++)
		delete cls.entityextrainfos[i];

	cls.entityextrainfos.clear();
	cls.numextrainfos = 0;
}

//=============================================
//
//=============================================
void CL_ClearEntities( void )
{
	for(Int32 i = 0; i < cls.numentities; i++)
	{
		if(cls.entities[i].pvbmhulldata)
		{
			delete cls.entities[i].pvbmhulldata;
			cls.entities[i].pvbmhulldata = nullptr;
		}

		if(cls.entities[i].pextradata)
			cls.entities[i].pextradata = nullptr;

		cls.entities[i] = cl_entity_t();
	}

	cls.numentities = 0;
}

//=============================================
//
//=============================================
void CL_ResetLighting( void )
{
	for(Uint32 i = 0; i < cls.numextrainfos; i++)
	{
		if(!cls.entityextrainfos[i]->plightinfo)
			continue;

		cls.entityextrainfos[i]->plightinfo->reset = true;
	}
}

//=============================================
//
//=============================================
void CL_PrecacheFlexScript( enum flextypes_t npctype, const Char* pstrscript )
{
	CFlexManager* pFlexManager = gVBMRenderer.GetFlexManager();
	if(!pFlexManager)
	{
		Con_EPrintf("%s - Flex manager not initialized!.\n", __FUNCTION__);
		return;
	}

	if(!pFlexManager->LoadAssociationScript(npctype, pstrscript))
		Con_Printf("%s - Failed to load script: %s.\n", __FUNCTION__, pFlexManager->GetError());
}

//=============================================
//
//=============================================
void CL_SetFlexScript( entindex_t entindex, const Char* pstrscript )
{
	CFlexManager* pFlexManager = gVBMRenderer.GetFlexManager();
	if(!pFlexManager)
	{
		Con_EPrintf("%s - Flex manager not initialized!.\n", __FUNCTION__);
		return;
	}

	cl_entity_t* pentity = CL_GetEntityByIndex((Uint32)entindex);
	if(!pentity || !pentity->pmodel || pentity->pmodel->type != MOD_VBM)
	{
		Con_DPrintf("%s - Could not get entity for script '%s'.\n", __FUNCTION__, pstrscript);
		return;
	}

	entity_extrainfo_t* pextrainfo = CL_GetEntityExtraData(pentity);
	if(!pextrainfo || !pextrainfo->pflexstate)
	{
		Con_DPrintf("%s - Could not get entity extradata for script '%s'.\n", __FUNCTION__, pstrscript);
		return;
	}
	
	// Set the script
	const vbmcache_t* pcache = pentity->pmodel->getVBMCache();
	pFlexManager->SetScript(pcache->pvbmhdr, pextrainfo->pflexstate, cls.cl_time, pstrscript);
}

//=============================================
//
//=============================================
void CL_ClientCommand( const Char* pstrCommand )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	// Add to command buffer, newline is automatically added
	gCommands.AddCommand(pstrCommand);
}

//=============================================
//
//=============================================
void CL_ServerCommand( const Char* pstrCommand )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	// Add "cmd" so it's forwarded to server immediately
	CString cmd;
	cmd << "cmd " << pstrCommand;

	gCommands.AddCommand(cmd.c_str());
}

//=============================================
//
//=============================================
void CL_LinkMapTextureMaterials( CArray<CString>& wadList )
{
	if(!cls.mapmaterialfiles.empty())
		cls.mapmaterialfiles.clear();

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

		if(!filepath.empty())
		{
			// Add to the association list
			maptexturematerial_t newmat;
			newmat.maptexturename = ens.pworld->ptextures[i].name;
			newmat.materialfilepath = filepath;
			cls.mapmaterialfiles.push_back(newmat);
		}
		else
			Con_Printf("%s - Failed to find material file for world texture '%s'.\n", __FUNCTION__, ens.pworld->ptextures[i].name.c_str());
	}
}

//=============================================
//
//=============================================
void CL_UpdateAttachments( cl_entity_t* pentity )
{
	gVBMRenderer.UpdateAttachments(pentity);
}

//=============================================
//
//=============================================
Uint32 CL_GetMaxClients( void )
{
	return cls.maxclients;
}

//=============================================
//
//=============================================
void CL_UpdateParentedEntities( void )
{
	cl_entity_t* pplayer = CL_GetLocalPlayer();
	if(!pplayer)
		return;

	for(Int32 i = 0; i < cls.numentities; i++)
	{
		cl_entity_t* pentity = CL_GetEntityByIndex(i);
		if(!pentity)
			continue;

		if(pentity->curstate.msg_num != pplayer->curstate.msg_num)
			continue;

		if(!pentity->pmodel)
			continue;

		if(pentity->curstate.effects & EF_NODRAW)
			continue;

		if(!(pentity->curstate.flags & FL_PARENTED)
			|| pentity->curstate.parent == NO_ENTITY_INDEX)
			continue;

		cl_entity_t* pparent = CL_GetEntityByIndex(pentity->curstate.parent);
		if(!pparent)
			continue;

		if(pparent->curstate.msg_num != pplayer->curstate.msg_num)
			continue;

		// Set origin always
		pentity->curstate.origin = pparent->curstate.origin;
		Math::VectorAdd(pentity->curstate.origin, pentity->curstate.parentoffset, pentity->curstate.origin);
		
		if(pentity->curstate.effects & EF_TRACKANGLES)
			pentity->curstate.angles = pparent->curstate.angles;
	}
}

//=============================================
//
//=============================================
void CL_UpdateEntityLights( void )
{
	for(Uint32 i = 0; i < MAX_ENTITY_LIGHTS; i++)
	{
		entitylight_t& light = cls.entitylights[i];
		if(light.die <= cls.cl_time)
			continue;

		if(!light.key)
			continue;

		cl_entity_t* pentity = CL_GetEntityByIndex(light.key);
		if(!pentity || !pentity->pmodel)
			continue;

		if(light.attachment != -1)
		{
			// Get attachment
			gVBMRenderer.UpdateAttachments(pentity);
			light.mlight.origin = pentity->getAttachment(light.attachment);
		}
		else
		{
			// Just set origin
			light.mlight.origin = pentity->curstate.origin;
		}
	}
}

//=============================================
//
//=============================================
void CL_SetPaused( bool isPaused, bool pauseOveride )
{
	cls.netinfo.pnet->CLS_MessageBegin(cls_pausegame);
	{
		cls.netinfo.pnet->WriteByte(isPaused);
		if(isPaused)
			cls.netinfo.pnet->WriteByte(pauseOveride);
	}
	cls.netinfo.pnet->CLS_MessageEnd();
}

//=============================================
//
//=============================================
void CL_NotifyLevelChange( void )
{
	cls.dllfuncs.pfnClientLevelChange();
}