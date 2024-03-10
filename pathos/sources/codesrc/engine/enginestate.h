/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CENGINESTATE_H
#define CENGINESTATE_H

#include "logfile.h"
#include "trace.h"

class CWADTextureResource;

// Default is MAX_MAP_LEAFS/8
static const Uint32 DEFAULT_VISBUFFER_SIZE = 16384;
// Default game directory
static const Char DEFAULT_GAMEDIR[] = "pathos";
// Common game directory
static const Char COMMON_GAMEDIR[] = "common";

enum dev_mode_t
{
	DEV_MODE_OFF = 0,
	DEV_MODE_ON,
	DEV_MODE_VERBOSE
};

enum window_mode_t
{
	WM_NONE = 0,
	WM_WINDOWED,
	WM_FULLSCREEN
};

enum gamestate_t
{
	GAME_INACTIVE = 0,
	GAME_LOADING,
	GAME_RUNNING
};

struct engine_state_t
{
	engine_state_t():
		exit(false),
		skipframe(false),
		spawnconsole(false),
		isinprocesstringcommand(false),
		isloading(false),
		isinitialized(false),
		gamestate(GAME_INACTIVE),
		pworld(nullptr),
		requestWMode(WM_NONE),
		requestedScrWidth(0),
		requestedScrHeight(0),
		requestedDisplayDevice(-1),
		requestedMSAASetting(-1),
		requestedVSyncSetting(-1),
		arg_max_edicts(0),
		lowshift(0),
		perffreq(0),
		curtime(0),
		prevtime(0),
		time(0),
		frametime(0),
		gamedir(DEFAULT_GAMEDIR),
		plogfile(nullptr),
		pgllogfile(nullptr),
		pfileiologfile(nullptr),
		tr_groupmask(0),
		tr_groupop(TR_GROUPOP_NONE),
		visbuffersize(DEFAULT_VISBUFFER_SIZE),
		pwadresource(nullptr)
	{}

	// indicates the main loop can be terminated
	bool exit;
	// TRUE if the next frame should have it's frametime reset to zero
	bool skipframe;
	// TRUE if console should be spawned on menu load
	bool spawnconsole;
	// TRUE if we're processing commands from SV_ProcessStringCommand
	bool isinprocesstringcommand;
	// true if we're in the middle of a load
	bool isloading;
	// TRUE if engine is initialized
	bool isinitialized;

	// Game state
	gamestate_t gamestate;

	// world model
	struct brushmodel_t* pworld;

	// Array of launch args
	CArray<CString> launchargs;

	// Requesting windowed mode
	window_mode_t requestWMode;
	// Requested screen width
	Uint32 requestedScrWidth;
	// Requested screen height
	Uint32 requestedScrHeight;
	// Requested display device
	Int32 requestedDisplayDevice;
	// Requested vsync setting
	Int32 requestedMSAASetting;
	// Requested vsync setting
	Int32 requestedVSyncSetting;

	// max_edicts specified via launch args
	Uint32 arg_max_edicts;

	// Amount to shift lowpart by
	Uint32 lowshift;
	// Performance freqency
	Double perffreq;
	// Current time
	Double curtime;
	// Previous time
	Double prevtime;

	// Time since application start
	Double time;
	// Frametime on main thread
	Double frametime;

	// Game title
	CString gametitle;
	// Start map
	CString startmap;
	// Game directory
	CString gamedir;

	// Map scheduled to load on in command line
	CString scheduledmap;

	// Log file to log to
	CLogFile *plogfile;
	// Log file to log to
	CLogFile *pgllogfile;
	// Log file for file loads
	CLogFile *pfileiologfile;

	// trace group mask
	Int32 tr_groupmask;
	// trace group op
	Int32 tr_groupop;

	// VIS buffer size
	Uint32 visbuffersize;

	// List of cvars overwritten by launch args
	CArray<CString> overwrittencvars;

	// wad resource
	CWADTextureResource* pwadresource;
};
extern engine_state_t ens;
#endif // ENGINESTATE_H
