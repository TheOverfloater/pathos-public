/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifdef USE_VLD
#include <vld.h>
#endif

#include "includes.h"

#include "vector.h"
#include "clientdll.h"
#include "cldll_interface.h"
#include "view.h"

#include "input.h"
#include "draw.h"
#include "trace.h"
#include "playermove.h"
#include "cl_entity.h"
#include "file_interface.h"
#include "stepsound.h"
#include "snd_shared.h"
#include "r_interface.h"

#include "efxapi.h"

#include "entitymanager.h"
#include "messages.h"
#include "viewmodel.h"
#include "cliententities.h"
#include "saytext.h"
#include "gameuimanager.h"
#include "huddraw.h"
#include "hud.h"
#include "ladder.h"
#include "motorbike.h"
#include "viewcontroller.h"
#include "nodedebug.h"
#include "shake.h"
#include "screentext.h"
#include "gameuielements.h"

// Declaration of gamedll enginefuncs struct
cldll_engfuncs_t cl_engfuncs;
// Trace functions from engine
trace_interface_t cl_tracefuncs;
// Declaration of file functions struct
file_interface_t cl_filefuncs;
// Render interface
r_interface_t cl_renderfuncs;
// Effects API
cl_efxapi_t cl_efxapi;

// Playermove object
CPlayerMovement gMovement;

// Step sound object on client
CStepSound g_stepSound;

// TRUE if we're executing a level change when ClientGameReset is called
bool g_isLevelChangeReset = false;

//
// Client DLL functions
//
static cldll_funcs_t CLIENT_DLL_FUNCTIONS =
{
	ClientDLLInit,				//pfnClientDLLInit
	ClientDLLShutdown,			//pfnClientDLLShutdown
	ClientConnected,			//pfnClientConnected
	ClientDisconnected,			//pfnClientDisconnected
	ClientFrame,				//pfnClientFrame
	ClientGameInit,				//pfnGameInit
	ClientGameReset,			//pfnGameReset
	ClientGLInit,				//pfnGLInit
	ClientGLClear,				//pfnGLClear
	V_CalcRefDef,				//pfnCalcRefDef
	CL_SetupView,				//pfnSetupView
	CL_DrawNormal,				//pfnDrawNormal
	CL_DrawTransparent,			//pfnDrawTransparent
	CL_DrawHUD,					//pfnDrawHUD
	ClientPreCmdThink,			//pfnClientPreCmdThink
	CL_CreateMove,				//pfnInMove
	CL_AddMouseMove,			//pfnMouseMove
	CL_RunPlayerMovement,		//pfnRunPlayermove
	CL_AddEntities,				//pfnAddEntities
	CL_DecalExternalEntities,	//pfnDecalExternalEntities
	CL_GetClientEntityList,		//pfnGetClientEntityList
	CL_ParseEntityList,			//pfnParseEntityList
	CL_FreeEntityData,			//pfnFreeEntityData
	CL_VBMEvent,				//pfnVBMEvent
	V_GetViewModel,				//pfnGetViewModel
	CL_AddSubtitle,				//pfnAddSubtitle
	CL_RemoveSubtitle,			//pfnRemoveSubtitle
	V_GetViewInfo,				//pfnGetViewInfo
	CL_IsInputOverridden,		//pfnIsInputOverridden
	CL_IsMouseOverridden,		//pfnIsMouseOverridden
	CL_IsEscapeKeyOverridden,	//pfnIsEscapeKeyOverridden
	CL_KeyEvent,				//pfnKeyEvent
	CL_MouseButtonEvent,		//pfnMouseButtonEvent
	CL_MouseWheelEvent,			//pfnMouseWheelEvent
	CL_WindowFocusLost,			//pfnWindowFocusLost
	CL_WindowFocusRegained,		//pfnWindowFocusRegained
	CL_DrawViewObjects,			//pfnDrawViewObjects
	CL_DrawViewObjectsForVSM,	//pfnDrawViewObjectsForVSM
	CL_DrawLadders,				//pfnDrawLadders
	CL_DrawLaddersForVSM,		//pfnDrawLaddersForVSM
	CL_AdjustEntityTimers,		//pfnAdjustEntityTimers
	CL_ShouldDrawPausedLogo,	//pfnShouldDrawPausedLogo
	ClientLevelChange			//pfnClientLevelChange
};

//=============================================
// @brief
//
//=============================================
bool ClientDLLInit( void )
{
	// Init input
	CL_InitInput();

	// Initialize entity manager
	gEntityManager.Init();

	// Initialize view model
	if(!gViewModel.Init())
		return false;

	// Initialize view
	if(!gDefaultView.Init())
		return false;

	// Initialize messages
	if(!gMessages.Init())
		return false;

	// Initialize saytext
	if(!gSayText.Init())
		return false;

	// Initialize game ui
	if(!gGameUIManager.Init())
		return false;

	// Initialize HUD renderer
	if(!gHUDDraw.Init())
		return false;

	// Initialize HUD
	gHUD.Init();

	// Initialize ladder
	if(!gLadder.Init())
		return false;

	// Initialize motorbike
	if(!gMotorBike.Init())
		return false;

	// Initialize view controller
	if(!gViewController.Init())
		return false;

	// Initialize node debuging
	if(!gNodeDebug.Init())
		return false;

	// Init screenshake
	if(!gShake.Init())
		return false;

	// Init screen text
	if(!gScreenText.Init())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void ClientDLLShutdown( void )
{
	// Shutdown input class
	CL_ShutdownInput();

	// Shutdown entity manager
	gEntityManager.Shutdown();

	// Shutdown messages
	gMessages.Shutdown();

	// Shut down saytext
	gSayText.Shutdown();

	// Shut down UI manager
	gGameUIManager.Shutdown();

	// Shut down hud draw
	gHUDDraw.Shutdown();

	// Shut down hud
	gHUD.Shutdown();

	// Initialize ladder
	gLadder.Shutdown();

	// Initialize motorbike
	gMotorBike.Shutdown();

	// Initialize view controller
	gViewController.Shutdown();

	// Shut down node debug
	gNodeDebug.Shutdown();

	// Shut down screen text
	gScreenText.Shutdown();
}

//=============================================
// @brief
//
//=============================================
void ClientPreCmdThink( void )
{
	// Perform think functions for HUD
	gHUD.Think();
}

//=============================================
// @brief
//
//=============================================
void ClientFrame( void )
{
	// Perform think functions on view model
	gViewModel.Think();
	// Perform think functions for UI
	gGameUIManager.Think();
	// Think for ladder
	gLadder.Think();
	// Think for motorbike
	gMotorBike.Think();
}

//=============================================
// @brief
//
//=============================================
void ClientConnected( void )
{
}

//=============================================
// @brief
//
//=============================================
void ClientDisconnected( void )
{
}

//=============================================
// @brief
//
//=============================================
void ClientInitStepSound( void )
{
	// Load the file
	Uint32 filesize = 0;
	const byte* pfile = cl_filefuncs.pfnLoadFile(FOOTSTEP_SCRIPT_FILE, &filesize);
	if(!pfile)
	{
		cl_engfuncs.pfnCon_Printf("%s - Could not load '%s'.\n", FOOTSTEP_SCRIPT_FILE);
		return;
	}

	if(!g_stepSound.Init(reinterpret_cast<const Char*>(pfile), filesize))
		cl_engfuncs.pfnCon_EPrintf("%s - Failed to initialize '%s': %s.\n", __FUNCTION__, FOOTSTEP_SCRIPT_FILE, g_stepSound.GetInfoString().c_str());

	if(!g_stepSound.IsInfoStringEmpty())
		cl_engfuncs.pfnCon_Printf(g_stepSound.GetInfoString().c_str());

	// Free the file
	cl_filefuncs.pfnFreeFile(pfile);
}

//=============================================
// @brief
//
//=============================================
bool ClientGameInit( void )
{
	// Initialize view
	if(!gDefaultView.InitGame())
		return false;

	// Initialize step sounds
	ClientInitStepSound();

	// Initialize entities on client
	gEntityManager.Setup();

	// Load titles
	if(!gMessages.InitGame())
		return false;

	// Initialize saytext
	if(!gSayText.InitGame())
		return false;

	// Initialize game ui
	if(!gGameUIManager.InitGame())
		return false;

	// Initialize HUD
	if(!gHUD.InitGame())
		return false;

	// Initialize ladder
	if(!gLadder.InitGame())
		return false;

	// Initialize motorbike
	if(!gMotorBike.InitGame())
		return false;

	// Initialize view controller
	if(!gViewController.InitGame())
		return false;

	// Init node debug
	if(!gNodeDebug.InitGame())
		return false;

	// Init viewmodel
	if(!gViewModel.InitGame())
		return false;

	if(!gScreenText.InitGame())
		return false;

	// Reset this
	g_isLevelChangeReset = false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void ClientGameReset( void )
{
	// Clear entity manager
	gEntityManager.Clear();
	// Clear view
	gDefaultView.ClearGame();
	// Clear messages
	gMessages.ClearGame();
	// Clear saytext
	gSayText.ClearGame();
	// Clear game ui
	gGameUIManager.ClearGame();
	// Clear HUD
	gHUD.ClearGame();
	// Clear ladder
	gLadder.ClearGame();
	// Clear motorbike
	gMotorBike.ClearGame();
	// Clear view controller
	gViewController.ClearGame();
	// Manage node debuger
	gNodeDebug.ClearGame();
	// Clear view model
	gViewModel.ClearGame();
	// Clear playermove
	gMovement.Reset();
	// Clear screen text
	gScreenText.ClearGame();
	// Clear shakes
	gShake.ClearGame();

	if(!g_isLevelChangeReset)
	{
		// Clear buttons only if we're not doing a level change
		CL_ResetPressedInputs();
	}
}

//=============================================
// @brief
//
//=============================================
bool ClientGLInit( void )
{
	if(!gScreenText.InitGL())
		return false;

	if(!gSayText.InitGL())
		return false;

	if(!gMessages.InitGL())
		return false;

	if(!gHUDDraw.InitGL())
		return false;

	if(!gHUD.InitGL())
		return false;

	if(!gGameUIManager.InitGL())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void ClientGLClear( void )
{
	// Clear GL states
	gScreenText.ClearGL();
	// Clear GL states
	gSayText.ClearGL();
	// Clear GL states
	gMessages.ClearGL();
	// Clear UI manager
	gGameUIManager.ClearGL();
	// Clear HUD Draw
	gHUDDraw.ClearGL();
	// Clear HUD
	gHUD.ClearGL();
}

//=============================================
// @brief
//
//=============================================
void CL_RunPlayerMovement( const usercmd_t& cmd, pm_info_t* pminfo, bool playSounds )
{
	bool isMultiplayer = (pminfo->movevars.maxclients > 1) ? true : false;

	// Run movement logic for the client
	gMovement.RunMovement(cmd, pminfo, playSounds, isMultiplayer);
}

//=============================================
// @brief
//
//=============================================
const entity_state_t* CL_GetEntityState( entindex_t entindex )
{
	cl_entity_t* pentity = cl_engfuncs.pfnGetEntityByIndex(entindex);
	if(!pentity)
		return nullptr;

	return &pentity->curstate;
}

//=============================================
// @brief
//
//=============================================
void CL_AddToTouched( entindex_t hitent, trace_t& trace, const Vector& velocity )
{
	// Do nothing on client
}

//=============================================
// @brief
//
//=============================================
void CL_PM_PlaySound( entindex_t entindex, Int32 channel, const Char* psample, Float volume, Float attenuation, Int32 pitch, Int32 flags )
{
	cl_engfuncs.pfnPlayEntitySound(entindex, channel, psample, volume, attenuation, pitch, flags, 0);
}

//=============================================
// @brief
//
//=============================================
void CL_PM_PlayStepSound( entindex_t entindex, const Char* pstrMaterialName, bool stepleft, Float volume, const Vector& origin )
{
	// Retreive the step sound to be played
	CStepSound::foot_t foot = stepleft ? CStepSound::FOOT_LEFT : CStepSound::FOOT_RIGHT;
	const CArray<CString>* pSoundsArray = g_stepSound.GetFootSoundList(foot, pstrMaterialName);
	if(!pSoundsArray || pSoundsArray->empty())
		return;

	Uint32 irand = Common::RandomLong(0, pSoundsArray->size()-1);
	const CString& sound = (*pSoundsArray)[irand];

	cl_engfuncs.pfnPlayEntitySound(entindex, SND_CHAN_BODY, sound.c_str(), volume, ATTN_NORM, PITCH_NORM, SND_FL_NONE, 0);
}

//=============================================
// @brief
//
//=============================================
void CL_DecalExternalEntities( const Vector& vpos, const Vector& vnorm, decalgroupentry_t *texptr, Int32 flags )
{
	gEntityManager.DecalExternalEntities(vpos, vnorm, texptr, flags);
}

//=============================================
// @brief
//
//=============================================
void CL_WindowFocusLost( void )
{
	// So the buttons don't get stuck
	CL_ResetPressedInputs();
}

//=============================================
// @brief
//
//=============================================
void CL_WindowFocusRegained( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CL_AddSubtitle( const Char* pstrSubtitleName, Float duration )
{
	return gHUD.AddSubtitle(pstrSubtitleName, duration);
}

//=============================================
// @brief
//
//=============================================
void CL_RemoveSubtitle( const Char* pstrSubtitleName )
{
	gHUD.RemoveSubtitle(pstrSubtitleName);
}

//=============================================
// @brief
//
//=============================================
bool CL_IsInputOverridden( void )
{
	if(gSayText.IsInInputMode())
		return true;

	if(gGameUIManager.HasActiveWindows())
		return true;

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CL_IsMouseOverridden( void )
{
	if(gSayText.IsInInputMode())
		return true;

	if(gGameUIManager.HasActiveWindows())
		return true;

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CL_IsEscapeKeyOverridden( void )
{
	if(gSayText.IsInInputMode())
		return true;

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CL_ShouldDrawPausedLogo( void )
{
	if(gGameUIManager.HasActiveWindows())
	{
		const CGameUIWindow* pWindow = gGameUIManager.GetActiveWindow();
		if(pWindow && pWindow->getWindowType() == GAMEUI_TEXTWINDOW)
			return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CL_KeyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(gSayText.IsInInputMode())
	{
		gSayText.KeyEvent(button, mod, keyDown);
		return;
	}

	if(gGameUIManager.HasActiveWindows())
	{
		gGameUIManager.KeyEvent(button, mod, keyDown);
		return;
	}
}

//=============================================
// @brief
//
//=============================================
void CL_MouseButtonEvent( Int32 button, bool keyDown )
{
	if(gGameUIManager.HasActiveWindows())
	{
		gGameUIManager.MouseButtonEvent(button, keyDown);
		return;
	}
}

//=============================================
// @brief
//
//=============================================
void CL_MouseWheelEvent( Int32 button, bool keyDown, Int32 scroll )
{
	if(gGameUIManager.HasActiveWindows())
	{
		gGameUIManager.MouseWheelEvent(button, keyDown, scroll);
		return;
	}
}

//=============================================
// @brief
//
//=============================================
void ClientLevelChange( void )
{
	g_isLevelChangeReset = true;
}

//=============================================
// @brief
//
//=============================================
extern "C" bool DLLEXPORT ClientDLL_Init( Uint32 version, cldll_funcs_t& dllFuncs, const trace_interface_t& traceFuncs, const file_interface_t& fileFuncs, const cldll_engfuncs_t& engFuncs, const cl_efxapi_t& efxAPI, const r_interface_t& renderFuncs )
{
	if(version != CLDLL_INTERFACE_VERSION)
		return false;

	// Set our engine funcs
	cl_engfuncs = engFuncs;
	// Set our trace funcs
	cl_tracefuncs = traceFuncs;
	// Set file funcs
	cl_filefuncs = fileFuncs;
	// Set render interface
	cl_renderfuncs = renderFuncs;
	// Set EFX API
	cl_efxapi = efxAPI;

	// Initialize client dll interface
	dllFuncs = CLIENT_DLL_FUNCTIONS;

	// Set playermove here, because we reference engine functions
	pm_interface_t pmInterface = 
	{
		engFuncs.pfnCon_Printf,						//pfnCon_Printf
		engFuncs.pfnCon_DPrintf,					//pfnCon_DPrintf
		engFuncs.pfnCon_VPrintf,					//pfnCon_VPrintf
		engFuncs.pfnCon_EPrintf,					//pfnCon_EPrintf
		engFuncs.pfnGetClientTime,					//pfnGetTime
		engFuncs.pfnGetModelType,					//pfnGetModelType
		engFuncs.pfnGetModelBounds,					//pfnGetModelBounds
		engFuncs.pfnGetModel,						//pfnGetModel
		engFuncs.pfnGetNumEntities,					//pfnGetNumEntities
		CL_GetEntityState,							//pfnGetEntityState
		engFuncs.pfnGetMapTextureMaterialScript,	//pfnGetMapTextureMaterialScript
		CL_PM_PlaySound,							//pfnPlaySound
		CL_PM_PlayStepSound,						//pfnPlayStepSound
		CL_AddToTouched,							//pfnAddToTouched
	};

	// Init player movement
	gMovement.Init(traceFuncs, pmInterface, false);

#ifdef _DEBUG
	cl_engfuncs.pfnCon_Printf("Client DLL loaded - [color r255]DEBUG[/color] build.\n");
#else
	cl_engfuncs.pfnCon_Printf("Client DLL loaded.\n");
#endif
	cl_engfuncs.pfnCon_Printf("Client DLL build date: %s.\n", __DATE__);

	return true;
}