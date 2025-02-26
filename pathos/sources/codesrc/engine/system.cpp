/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

/*
===============================================
Description:
Functions that deal with base engine
state variables and functionality.
===============================================
*/

#include "includes.h"
#include "system.h"
#include "window.h"
#include "vid.h"
#include "enginestate.h"
#include "config.h"
#include "file.h"

#include "logfile.h"
#include "commands.h"
#include "input.h"
#include "texturemanager.h"

#include "r_vbo.h"
#include "r_glsl.h"
#include "r_text.h"

#include "uimanager.h"
#include "uielements.h"
#include "r_menu.h"
#include "r_menuparticles.h"
#include "console.h"

#include "edict.h"
#include "cl_entity.h"
#include "cl_main.h"
#include "sv_main.h"
#include "cl_predict.h"

#include "brushmodel.h"
#include "modelcache.h"
#include "com_math.h"
#include "networking.h"
#include "cl_snd.h"
#include "sv_msg.h"
#include "r_main.h"
#include "dllexports.h"
#include "filewriterthread.h"

#if defined WIN32 && _64BUILD
#include <detours.h>
#endif

extern file_interface_t ENGINE_FILE_FUNCTIONS;

// Ugly hack to manage SDL2 load before main is called
CStartup gStartup;

#ifdef _64BUILD
// OpenAL library path
static const Char SDL2_LIBRARY_PATH[] = "x64/SDL2.dll";
#else
// OpenAL library path
static const Char SDL2_LIBRARY_PATH[] = "x86/SDL2.dll";
#endif

// Definition of engine state structure
engine_state_t ens;

// Mutex for console prints
HANDLE g_hPrintMutex = nullptr;

// Target array for exports
CArray<dll_export_t>* g_pExportsTargetArray = nullptr;

// Developer cvar
CCVar* g_pCvarDeveloper = nullptr;
// Timescale cvar
CCVar* g_pCvarTimeScale = nullptr;
// Max FPS cvar
CCVar* g_pCvarFPSMax = nullptr;
// Whether to keep old save files
CCVar* g_pCvarKeepOldSaves = nullptr;

//=============================================
// @brief Determines if the application should exit
//
// @return TRUE if we should exit, FALSE otherwise
//=============================================
bool Sys_ShouldExit( void )
{
	return ens.exit;
}

//=============================================
// @brief Initializes the basic systems
// 
// @param pparams Launch parameters
// @return TRUE if successful, FALSE on error
//=============================================
bool Sys_Init( CArray<CString>* argsArray )
{
	// Create console print mutex
	g_hPrintMutex = CreateMutex(nullptr, FALSE, "PathosConsolePrintMutex");
	if(nullptr != g_hPrintMutex)
		GetLastError();

	// Start SDL
	if(SDL_Init( SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_EVENTS |
       SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_SENSOR | SDL_INIT_VIDEO))
	{
		Sys_ErrorPopup("SDL_Init returned an error: %s", SDL_GetError());
		return false;
	}

	// Save launch args to args list
	for (Uint32 i = 0; i < argsArray->size(); i++)
		ens.launchargs.push_back((*argsArray)[i]);

	// Find out what mod we are running before doing
	// anything else
	if(!Sys_CheckGameDir(argsArray))
		return false;

	// Create file writer thread
	FWT_Init();

	// Init time tracking
	if(!Sys_InitFloatTime())
		return false;

	// Set ens.time to a nonzero value so some time-based functions work immediately
	// after starting the engine(Sys_InitFloatTime takes care of this)
	ens.time = ens.curtime;

	// Developer cvar
	g_pCvarDeveloper = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "developer", "0", "Controls developer information printing.\n");
	// Timescale cvar
	g_pCvarTimeScale = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, "host_timescale", "1.0", "Can be used to manipulate the time scale.\n");
	// Max FPS cvar
	g_pCvarFPSMax = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, "fps_max", "100", "Max framerate.\n");

	// MUST BE FIRST
	// Initialize configuration
	gConfig.Init();

	// See if the default font exists
	if(!Sys_LoadDefaultFont(nullptr))
		return false;

	// Initialize UI
	gUIManager.Init();

	// Initialize input class
	gInput.Init();
	// Initialize commands
	gCommands.Init();
	// Load font library
	gText.Init();
	// Initialize the console
	gConsole.Init();
	// Intialize model cache
	gModelCache.Init();

	// Register commands
	Sys_InitCommands();
	Sys_InitCVars();

	VID_InitCommands();

	// Load in the gameinfo file
	if(!Sys_LoadGameInfo(argsArray))
		return false;

	// Parse the launch parameters
	if(!Sys_ParseLaunchParams(argsArray))
		return false;

	// Delete the array
	delete argsArray;

	// Initialize the server
	if(!SV_Init())
		return false;

	// Initialize the client
	if(!CL_Init())
		return false;

	// Whether to keep old save files - This needs to be registered just before the config file gets executed
	g_pCvarKeepOldSaves = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY|FL_CV_SAVE, "sv_keepoldaves", "0", "Controls whether old quick/auto saves are kept.\n");

	// Load the config before VID_Init
	CString strExecCmd;
	strExecCmd << "exec " << CConfig::USER_CONFIG_FILENAME;

	gCommands.ExecuteCommand(strExecCmd.c_str(), false);

	// Clear overwritten cvars
	if(!ens.overwrittencvars.empty())
		ens.overwrittencvars.clear();

	// Initialize rendering
	if(!VID_Init())
	{
		Sys_ErrorPopup("Failed to initialize window.\n");
		return false;
	}

	// We're finished loading, so disable loading screen draw
	// and show the menu
	VID_EndLoading();

	// Tell menu to blend the bootup screen
	// and play the menu music
	gMenu.InitialStartup();

	// For logging
	CString strPrint;
	strPrint << "Engine initialized";

#ifdef _DEBUG
	strPrint << " - [color r255]DEBUG[/color] build";
#endif

#ifdef _64BUILD
	strPrint << " - x64 platform";
#else
	strPrint << " - x86 platform";
#endif

	strPrint << "\n";

	// Close the line
	Con_Printf(strPrint.c_str());
	Con_Printf("Engine build date: %s.\n", __DATE__);

	// Mark initialized
	ens.isinitialized = true;

	if(!ens.scheduledmap.empty())
	{
		CString cmd;
		cmd << "map " << ens.scheduledmap;
		gCommands.AddCommand(cmd.c_str());
		ens.scheduledmap.clear();
	}

	return true;
}

//=============================================
// @brief Performs shutdown operations
//
//=============================================
void Sys_Shutdown( void )
{
	// Disconnect client if connected
	if(cls.cl_state != CLIENT_INACTIVE)
		CL_Disconnect();

	// Shut down video
	VID_Shutdown();

	// Clear temp files
	Sys_DeleteTempFiles(RS_APP_LEVEL);

	// Manage log file if set
	if(ens.pgllogfile)
	{
		// Mark exit
		ens.pgllogfile->Write("GL log file closed.");
		if(!ens.pgllogfile->Close())
			Con_EPrintf("Error closing OpenGL log file.\n");

		delete ens.pgllogfile;
		ens.pgllogfile = nullptr;
	}

	// Delete all texture entries and texmanager object
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	pTextureManager->Shutdown();
	CTextureManager::DeleteInstance();

	// Free text library
	gText.Shutdown();

	// Shut down client and server
	CL_Shutdown();
	SV_Shutdown();

	if(ens.plogfile)
	{
		// Mark exit
		ens.plogfile->Write("Pathos Engine exiting.\n");
		if(!ens.plogfile->Close())
			Con_EPrintf("Error closing engine log file.\n");

		delete ens.plogfile;
		ens.plogfile = nullptr;
	}

	if(ens.pfileiologfile)
	{
		// Mark exit
		ens.pfileiologfile->Write("File I/O file exiting.\n");
		if(!ens.pfileiologfile->Close())
			Con_EPrintf("Error closing file I/O log log file.\n");

		delete ens.pfileiologfile;
		ens.pfileiologfile = nullptr;
	}

	if(g_hPrintMutex)
		CloseHandle(g_hPrintMutex);

	// Close file writer thread
	FWT_Shutdown();

	// Quit SDL
	SDL_Quit();
}

//=============================================
// @brief Cleans up and exits the application
// 
//=============================================
void Sys_Exit()
{
	Sys_Shutdown();
	exit(-1);
}

//====================================
// @brief Handles an error poup
// 
// @param exit Tells if we should exit immediately
// @param fmt Formatted string template
// @param ... Additional parameters
//====================================
void Sys_ErrorPopup ( const Char *fmt, ... )
{ 
	va_list	vArgPtr;
	static Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	SDL_Window* pWindow = gWindow.GetWindow();

	SDL_ShowWindow(gWindow.GetWindow());
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "FATAL ERROR", cMsg, pWindow ? pWindow : nullptr);

	// Log it to console also
	Con_EPrintf(cMsg);
}

//=============================================
// @brief Initializes time tracking
// 
// @return TRUE if successful, FALSE otherwise
//=============================================
bool Sys_InitFloatTime( void )
{
	LARGE_INTEGER performanceFrequency;
	if(!QueryPerformanceFrequency(&performanceFrequency))
	{
		Sys_ErrorPopup("Unable to get hardware timer.\n");
		return false;
	}

	Uint32 lowPart = static_cast<Uint32>(performanceFrequency.LowPart);
	Uint32 highPart = static_cast<Uint32>(performanceFrequency.HighPart);

	// Default to this value
	ens.lowshift = 0;

	while(highPart || (lowPart > 2000000.0))
	{
		ens.lowshift++;

		lowPart >>= 1;
		lowPart |= (highPart & 1) << 31;

		highPart >>= 1;
	}

	ens.perffreq = 1.0f / static_cast<Double>(lowPart);

	// So values are filled
	Sys_FloatTime();
	
	// Set current time to a non-zero value
	// so that some time tracking init functions
	// work
	ens.curtime = 0.1;
	ens.prevtime = ens.time;

	return true;
}

//=============================================
// @brief Returns the current time in seconds
// 
// @return Time in seconds
//=============================================
Double Sys_FloatTime( void )
{
	static Uint32 oldtime = 0;
	static Int32 sametimecount = 0;
	static bool isfirstcall = true;

	// I ended up recoding Quake's time stuff after realizing my
	// own implementation caused massive stuttering
	LARGE_INTEGER performanceFrequency;
	QueryPerformanceCounter(&performanceFrequency);

	Uint32 tmp = (static_cast<Uint32>(performanceFrequency.LowPart) >> ens.lowshift)
		| (static_cast<Uint32>(performanceFrequency.HighPart) << (32 - ens.lowshift));

	if(isfirstcall)
	{
		oldtime = tmp;
		isfirstcall = false;
	}
	else
	{
		if(tmp <= oldtime && (oldtime - tmp) < 0x10000000)
		{
			// So we don't get stuck(?)
			oldtime = tmp;
		}
		else
		{
			Uint32 t2 = tmp - oldtime;
			Double time = static_cast<Double>(t2*ens.perffreq);
			oldtime = tmp;
			ens.curtime += time;

			if(ens.curtime == ens.prevtime)
			{
				sametimecount++;
				if(sametimecount > 100000)
				{
					ens.curtime += 1.0;
					sametimecount = 0;
				}
			}
			else
			{
				// Clear counter
				sametimecount = 0;
			}

			ens.prevtime = ens.curtime;
		}
	}

	return ens.curtime;
}

//=============================================
// @brief Returns the FPS limit
//
// @return FPS limit as a float
//=============================================
Uint32 Sys_GetFPSLimit( void )
{
	Uint32 cvarLimit = static_cast<Uint32>(g_pCvarFPSMax->GetValue());
	return cvarLimit;
}

//=============================================
// @brief Parses launch parameters for the mod name
// and/or checks whether there's a valid gameinfog.cfg file
// present
//
// @param argsArray Array of arguments
//=============================================
bool Sys_CheckGameDir( const CArray<CString>* argsArray )
{
	// Parse all the other arguments
	for(Uint32 i = 1; i < argsArray->size(); i++)
	{
		CString strArg = (*argsArray)[i];
		CString argName(strArg);

		if(!qstrcmp(strArg, "-game"))
		{
			if(i == (argsArray->size()-1))
			{
				Con_Printf("Warning: Missing argument for %s.\n", strArg.c_str());
				continue;
			}

			ens.gamedir = (*argsArray)[++i];
			break;
		}
	}

	// Ensure the file exists, don't use FL_FileExists because it defaults
	// to the common dir, so use SDL_RW functions instead
	CString filepath;
	filepath << ens.gamedir << PATH_SLASH_CHAR << GAMEINFO_FILENAME;

	SDL_RWops* pf = SDL_RWFromFile(filepath.c_str(), "rb");
	if(!pf)
	{
		Sys_ErrorPopup("Could not locate game config file '%s'.\n", filepath.c_str());
		return false;
	}

	SDL_RWclose(pf);
	return true;
}

//=============================================
// @brief Parses launch parameters
//
// @param argsArray Array of arguments
//=============================================
bool Sys_ParseLaunchParams( const CArray<CString>* argsArray )
{
	Int32 screenHeight = 0;
	Int32 screenWidth = 0;

	// See if we're asked to clear previous logs
	bool clearPreviousLogs = false;
	Uint32 i = 0;
	for(; i < argsArray->size(); i++)
	{
		if(!qstrcmp((*argsArray)[i].c_str(), "-clearlogs"))
			break;
	}

	if(i != argsArray->size())
		clearPreviousLogs = true;

	// Seek -log first and foremost
	i = 0;
	for(; i < argsArray->size(); i++)
	{
		if(!qstrcmp((*argsArray)[i].c_str(), "-log"))
			break;
	}

	// Create the log file if needed
	if(i != argsArray->size())
	{
		Con_Printf("Warning: [color r255]Logging enabled, performance may be impacted by disk write operations.\n");

		ens.plogfile = new CLogFile("engine.log", Con_Printf, ENGINE_FILE_FUNCTIONS, clearPreviousLogs);
		if(!ens.plogfile->Init())
		{
			Sys_ErrorPopup("Failed to open log file for writing.\n");
			return false;
		}

		// Print the current date and build info
		CString strLog;
		strLog << "Pathos Engine";
#ifdef _DEBUG
		strLog << "(DEBUG)";
#endif
		strLog << " - Build date: " << __TIME__ << " " << __DATE__ << NEWLINE;
		ens.plogfile->Write(strLog.c_str());
	}

	// Seek -fileiolog
	i = 0;
	for(; i < argsArray->size(); i++)
	{
		if(!qstrcmp((*argsArray)[i].c_str(), "-fileiolog"))
			break;
	}

	// Create the log file if needed
	if(i != argsArray->size())
	{
		Con_Printf("Warning: [color r255]File I/O logging enabled, performance may be impacted by disk write operations.\n");

		ens.pfileiologfile = new CLogFile("fileio.log", Con_Printf, ENGINE_FILE_FUNCTIONS, clearPreviousLogs);
		if(!ens.pfileiologfile->Init())
		{
			Sys_ErrorPopup("Failed to open file I/O log file for writing.\n");
			return false;
		}

		// Print the current date and build info
		CString strLog;
		strLog << "File I/O log opened." << NEWLINE;;
#ifdef _DEBUG
		strLog << "(DEBUG)";
#endif
		ens.pfileiologfile->Write(strLog.c_str());
	}

	// Seek -gllog
	i = 0;
	for(; i < argsArray->size(); i++)
	{
		if(!qstrcmp((*argsArray)[i].c_str(), "-gllog"))
			break;
	}

	// Create the log file if needed
	if(i != argsArray->size())
	{
		Con_Printf("Warning: [color r255]OpenGL logging set, performance may be impacted by disk write operations.\n");

		ens.pgllogfile = new CLogFile("opengl.log", Con_Printf, ENGINE_FILE_FUNCTIONS, clearPreviousLogs);
		if(!ens.pgllogfile->Init())
		{
			Sys_ErrorPopup("Failed to open log file for writing.\n");
			return false;
		}

		// Print the current date and build info
		CString strLog;
		strLog << "Pathos Engine - Build date: " << __TIME__ << " " << __DATE__ << NEWLINE;
		ens.pgllogfile->Write(strLog.c_str());
	}

	// Parse all the other arguments
	for(i = 1; i < argsArray->size(); i++)
	{
		CString strArg = (*argsArray)[i];
		CString argName(strArg);

		if(!qstrcmp(strArg.c_str(), "-log") 
			|| !qstrcmp(strArg.c_str(), "-gllog") 
			|| !qstrcmp(strArg.c_str(), "-fileiolog")
			|| !qstrcmp(strArg.c_str(), "-clearlogs") )
			continue;

		if(strArg[0] == '-')
		{
			if(!qstrcmp(strArg, "-w") || !qstrcmp(strArg, "-width"))
			{
				if(i == (argsArray->size()-1))
				{
					Con_Printf("Warning: Missing argument for %s.\n", strArg.c_str());
					continue;
				}

				CString argNameInner = strArg;
				strArg = (*argsArray)[++i];
				if(!Common::IsNumber(strArg))
				{
					Con_Printf("Warning: Argument '%s' for %s needs to be a valid number.\n", strArg.c_str(), argNameInner.c_str());
					continue;
				}

				screenWidth = SDL_atoi(strArg.c_str());
			}
			else if(!qstrcmp(strArg, "-h") || !qstrcmp(strArg, "-height"))
			{		
				if(i == (argsArray->size()-1))
				{
					Con_Printf("Warning: Missing argument for %s.\n", strArg.c_str());
					continue;
				}

				CString argNameInner = strArg;
				strArg = (*argsArray)[++i];
				if(!Common::IsNumber(strArg))
				{
					Con_Printf("Warning: Argument '%s' for %s needs to be a valid number.\n", strArg.c_str(), argNameInner.c_str());
					continue;
				}

				screenHeight = SDL_atoi(strArg.c_str());
			}
			else if(!qstrcmp(strArg, "-max_edicts"))
			{
				if(i == (argsArray->size()-1))
				{
					Con_Printf("Warning: Missing argument for %s.\n", strArg.c_str());
					continue;
				}

				CString argNameInner = strArg;
				strArg = (*argsArray)[++i];
				if(!Common::IsNumber(strArg))
				{
					Con_Printf("Warning: Argument '%s' for %s needs to be a valid number.\n", strArg.c_str(), argNameInner.c_str());
					continue;
				}

				ens.arg_max_edicts = SDL_atoi(strArg.c_str());
				if(ens.arg_max_edicts > MAX_SERVER_ENTITIES)
				{
					Con_Printf("Invalid value %d for '-max_edicts', maximum is %d.\n", ens.arg_max_edicts, static_cast<Int32>(MAX_SERVER_ENTITIES));
					ens.arg_max_edicts = MAX_SERVER_ENTITIES;
				}
			}
			else if(!qstrcmp(strArg, "-window") || !qstrcmp(strArg, "-startwindowed") || !qstrcmp(strArg, "-windowed"))
			{
				ens.requestWMode = WM_WINDOWED;
			}
			else if(!qstrcmp(strArg, "-fullscreen"))
			{
				ens.requestWMode = WM_FULLSCREEN;
			}
			else if(!qstrcmp(strArg, "-dev"))
			{
				if(i == (argsArray->size()-1))
				{
					Con_Printf("Warning: Missing argument for %s.\n", strArg.c_str());
					continue;
				}

				CString argNameInner = strArg;
				strArg = (*argsArray)[++i];

				Int32 dMode = DEV_MODE_OFF;
				if(strArg == "0")
					dMode = DEV_MODE_OFF;
				else if(strArg == "1")
					dMode = DEV_MODE_ON;
				else if(strArg == "2")
					dMode = DEV_MODE_VERBOSE;
				else
					Con_Printf("Warning: Wrong dev level(%s) set for %s.\n", strArg.c_str(), argNameInner.c_str());

				gConsole.CVarSetFloatValue(g_pCvarDeveloper->GetName(), dMode);
			}
			else if(!qstrcmp(strArg, "-visbufsize"))
			{
				if(i == (argsArray->size()-1))
				{
					Con_Printf("Warning: Missing argument for %s.\n", strArg.c_str());
					continue;
				}

				CString argNameInner = strArg;
				strArg = (*argsArray)[++i];

				ens.visbuffersize = SDL_atoi(strArg.c_str());
				if(ens.visbuffersize < DEFAULT_VISBUFFER_SIZE)
				{
					Con_Printf("Warning: Invalid buffer size %d set for '%s'.\n", argNameInner.c_str());
					ens.visbuffersize = DEFAULT_VISBUFFER_SIZE;
				}
			}
			else if(!qstrcmp(strArg, "-game"))
			{
				// Not handled in this function
				++i;
			}
			else if(!qstrcmp(strArg, "-console"))
			{
				ens.spawnconsole = true;
			}
			else
				Con_DPrintf("Argument %s not handled in %s.\n", argName.c_str(), __FUNCTION__);
		}
		else if(strArg[0] == '+')
		{
			if(!qstrcmp(strArg.c_str()+1, "map"))
			{
				if(i == (argsArray->size()-1))
				{
					Con_Printf("Warning: Missing argument for %s.\n", strArg.c_str());
					continue;
				}

				ens.scheduledmap = (*argsArray)[i+1];
				i += 2;
			}
			else
			{
				// Extract the full command
				CString fullCmd;
				fullCmd << strArg.c_str()+1;

				// Add the cmd to the overwritten list
				ens.overwrittencvars.push_back(fullCmd);

				// Add everything until the next argument
				for(i++; i < argsArray->size(); i++)
				{
					const CString& cmdArg = (*argsArray)[i];
					if(cmdArg[0] == '-' || cmdArg[0] == '+')
					{
						i--;
						break;
					}

					fullCmd << " " << cmdArg;
				}

				gCommands.AddCommand(fullCmd.c_str());
			}
		}
		else
			Con_DPrintf("Argument %s not handled in %s.\n", argName.c_str(), __FUNCTION__);
	}
	
	// Manage the resolution arguments
	if(screenWidth != 0 && screenHeight == 0 
		|| screenWidth == 0 && screenHeight != 0)
	{
		Con_Printf("Warning: Both width and height need to be set, disregarding arguments.\n");
	}
	else
	{
		ens.requestedScrWidth = screenWidth;
		ens.requestedScrHeight = screenHeight;
	}

	return true;
}

//=============================================
// @brief Manages think functions for the interface
// 
//=============================================
void Sys_PreThink( void )
{
	// Perform menu think functions
	gMenu.Think();

	// Perform menu particle think functions
	gMenuParticles.Think();

	// Perform UI think functions
	gUIManager.Think();
}

//=============================================
// @brief Manages post-think functions for the interface
// 
//=============================================
void Sys_PostThink( void )
{
	// Perform UI think functions
	gUIManager.PostThink();

	// Check if we need to write the config file
	gConfig.PostThink();

	// Update console debug infos
	gConsole.Think();
}

//=============================================
// @brief Sets pause state
// 
//=============================================
void Sys_SetPaused( bool paused, bool print )
{
	if(svs.maxclients > 1 && svs.serverstate == SV_INACTIVE)
	{
		Con_Printf("Only the host may pause the game.\n");
		return;
	}

	if(ens.gamestate == GAME_INACTIVE)
	{
		Con_Printf("Can't pause, game is not running.\n");
		return;
	}

	// Set state
	svs.paused = paused;

	// Send msgs to clients
	for(Uint32 i = 0; i < svs.maxclients; i++)
	{
		if(!svs.clients[i].connected)
			continue;

		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_setpause, svs.clients[i].pedict);
		svs.netinfo.pnet->WriteByte(svs.paused);
		svs.netinfo.pnet->SVC_MessageEnd();

		if(print)
		{
			if(svs.paused)
				SV_ClientPrintf(svs.clients[i].pedict, "%s paused the game.\n", SV_GetString(svs.clients[i].pedict->fields.netname));
			else
				SV_ClientPrintf(svs.clients[i].pedict, "%s unpaused the game.\n", SV_GetString(svs.clients[i].pedict->fields.netname));
		}
	}
}

//=============================================
// @brief Manages think functions for the interface
// 
//=============================================
#ifdef _64BUILD
static BOOL CALLBACK ExportCallback( PVOID pContext, ULONG nOrdinal, LPCSTR szSymbol, PVOID pbTarget )
{
	dll_export_t newExport;
	newExport.functionname = szSymbol;
	g_pExportsTargetArray->push_back(newExport);

	return TRUE;
}
#else
static void ExportCallback( Char* pstrSymbolName )
{
	dll_export_t newExport;
	newExport.functionname = pstrSymbolName;
	g_pExportsTargetArray->push_back(newExport);
}
#endif

//=============================================
// @brief Manages think functions for the interface
// 
//=============================================
bool Sys_GetDLLExports( const Char* pstrDLLName, void* pDLLHandle, CArray<dll_export_t>& destArray )
{
#ifdef _64BUILD
	HMODULE hDLL = GetModuleHandleA(pstrDLLName);
	if(!hDLL)
	{
		Con_Printf("The specified library '%s' does not exist.\n", pstrDLLName);
		return false;
	}
	
	// Set the destination array
	g_pExportsTargetArray = &destArray;

	BOOL validFlag = FALSE;
	if(!DetourEnumerateExports(hDLL, &validFlag, ExportCallback))
	{
		Con_EPrintf("Couldn't get exports for '%s'.\n", pstrDLLName);
		return false;
	}
#else
	// Set the destination array
	g_pExportsTargetArray = &destArray;

	if(!EnumExportedFunctions(pstrDLLName, ExportCallback))
	{
		Con_EPrintf("Couldn't get exports for '%s'.\n", pstrDLLName);
		return false;
	}
#endif
	// Set the function pointers
	for(Uint32 i = 0; i < destArray.size(); i++)
	{
		destArray[i].functionptr = SDL_LoadFunction(pDLLHandle, destArray[i].functionname.c_str());
		if(destArray[i].functionptr == nullptr)
		{
			Con_EPrintf("Failed to find function '%s' in '%s'.\n", destArray[i].functionname.c_str(), pstrDLLName);
			return false;
		}
	}

	return true;
}

//=============================================
// @brief Manages network polling
// 
//=============================================
void Sys_Poll( void )
{
	CNetworking* pNet = CNetworking::GetInstance();
	if(!pNet)
		return;

	pNet->Poll();
}

//=============================================
// @brief Manages the main game logic loop
// 
// @param frametime Time elapsed since last frame
//=============================================
void Sys_Frame( Double frametime )
{
	Double _frametime = frametime;
	if(_frametime < 0 || _frametime > 1)
		_frametime = 0;

	// Apply timescale
	_frametime *= g_pCvarTimeScale->GetValue();

	if(ens.skipframe || ens.gamestate == GAME_LOADING)
	{
		// Cap off loading time
		_frametime = 0;

		if(ens.skipframe)
			ens.skipframe = false;
	}

	// Set the times
	ens.time += _frametime;
	ens.frametime = _frametime;

	// send key events here
	gInput.HandleInput();

	// Think after getting inputs
	Sys_PreThink();

	// Execute the command buffer
	gCommands.ExecuteCommands();

	// Perform post-think functions
	Sys_PostThink();

	// Run networking if needed
	Sys_Poll();

	// Perform any game functions
	if(ens.gamestate == GAME_RUNNING)
	{
		// Send move commands
		CL_SendCmd();
	}

	// Run client prediction for local client
	if(cls.cl_state != CLIENT_INACTIVE
		&& !svs.haltserver)
	{
		CL_RunPrediction();
	}

	if(ens.gamestate != GAME_INACTIVE)
	{
		// Server functions
		SV_Frame();
	}

	// Client functions
	CL_Frame();

	// Perform rendering functions
	VID_Draw();

	// Check for rendering errors
	if(rns.fatalerror)
	{
		Con_EPrintf("Fatal error encountered during rendering.\n");
		return;
	}

	// Update sound engine
	CL_UpdateSound();
}

//=============================================
// @brief Polls for events sent by SDL
//
//=============================================
void Sys_PollEvents( void )
{
	SDL_PumpEvents();

	SDL_Event mainEvent;
	while(SDL_PollEvent(&mainEvent) != 0)
	{
		if(mainEvent.type == SDL_WINDOWEVENT)
		{
			switch(mainEvent.window.event)
			{
			case SDL_WINDOWEVENT_MINIMIZED:
			case SDL_WINDOWEVENT_HIDDEN:
			case SDL_WINDOWEVENT_FOCUS_LOST:
				Sys_WindowFocusLost();
				break;

			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_RESTORED:
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				Sys_WindowFocusRegained();
				break;

			case SDL_WINDOWEVENT_CLOSE:
				ens.exit = true;
				break;
			}
		}
		else if(gWindow.IsActive())
		{
			// Handle any other events with the input class
			gInput.HandleSDLEvent(mainEvent);
		}
	}
}

//=============================================
// @brief Manages the main application loop
//
// @param argsArray Array of launch args
// @return Status code
//=============================================
Int32 Sys_Main( CArray<CString>* argsArray )
{
	// Avoid launching multiple instances
	HANDLE hMutex = CreateMutex(nullptr, FALSE, "PathosEngineInstanceMutex");

	if(nullptr != hMutex)
		GetLastError();

	DWORD mutexResult = WaitForSingleObject(hMutex, 0);
	if(mutexResult != WAIT_OBJECT_0 && mutexResult != WAIT_ABANDONED)
	{
		Sys_ErrorPopup("Only one instance of this game can be running at a time.");
		Con_EPrintf("Error during system initialization.\n");
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return -1;
	}

	if(!Sys_Init( argsArray ))
	{
		Con_EPrintf("Error during system initialization.\n");
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return -1;
	}

	// Get current time
	Double oldTime = Sys_FloatTime();

	// Run the main loop
	while(!Sys_ShouldExit())
	{
		// Poll for any events
		Sys_PollEvents();

		// Get current times
		Double curTime = Sys_FloatTime();
		Double frametime = curTime - oldTime;

		// Limit minimum FPS to 0.5 frames per second
		Double fpsMax = Sys_GetFPSLimit();
		if(fpsMax < 0.5)
			fpsMax = 0.5;

		// Do not let us go above the framerate limit
		if((1.0f / (fpsMax+0.5)) > frametime)
			continue;

		// Process this frame
		Sys_Frame(frametime);
		oldTime = curTime;
	}

	Sys_Shutdown();

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);

	return 0;
}

//=============================================
// @brief Called when the window loses focus
//
//=============================================
void Sys_WindowFocusLost( void )
{
	gWindow.SetActive(false);
}

//=============================================
// @brief Called when the window regains
//
//=============================================
void Sys_WindowFocusRegained( void )
{
	gWindow.SetActive(true);
}

//=============================================
// Class: CStartup
// Function: CStartup
//=============================================
CStartup::CStartup( void ):
	m_hSDL2(nullptr)
{
	m_hSDL2 = LoadLibrary(SDL2_LIBRARY_PATH);
	if(!m_hSDL2)
	{
		CString str;
		str << "Failed to load " << SDL2_LIBRARY_PATH;
		MessageBox(nullptr, str.c_str(), "Error", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		exit(-1);
	}
}

//=============================================
// Class: CStartup
// Function: CStartup
//=============================================
CStartup::~CStartup( void )
{
	if(m_hSDL2)
		FreeLibrary(m_hSDL2);
}
