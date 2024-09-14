/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMEUI_SHARED_H
#define GAMEUI_SHARED_H

// Base script file path for subway
static const Char GAMEUI_SCRIPT_BASE_PATH[] = "scripts/gameui/";
// Subway subfolder name
static const Char SUBWAYWINDOW_SCRIPT_SUBFOLDER_NAME[] = "subway";

// Max keypad code length
static const Uint32 MAX_PASSCODE_LENGTH = 32;

// Max objectives a player can have, due to Int32 being used for the new objective flag stuff
static const Uint64 MAX_PLAYER_OBJECTIVES = 32;

// Flags for subway
enum subwayflags_t
{
	FL_SUBWAY_NONE				= 0,
	FL_SUBWAY_GOT_BERGENST		= (1<<0),
	FL_SUBWAY_GOT_IBMANNST		= (1<<1),
	FL_SUBWAY_GOT_ECKHARTST		= (1<<2),
	FL_SUBWAY_GOT_MARSHALLST	= (1<<3),
	FL_SUBWAY_GOT_KASSARST		= (1<<4),
	FL_SUBWAY_GOT_AIELLOST		= (1<<5),
	FL_SUBWAY_DISABLED			= (1<<6),
};

enum subwayline_t
{
	SUBWAYLINE_BERGEN_ECKHART = 0,
	SUBWAYLINE_KASSAR_STILLWELL,
	SUBWAYLINE_MARSHALL_LYNE
};

// Window types to be spawned
enum gameui_windows_t
{
	GAMEUI_WINDOW_NONE = -1,
	GAMEUI_TEXTWINDOW = 0,
	GAMEUI_KEYPADWINDOW,
	GAMEUI_LOGINWINDOW,
	GAMEUI_SUBWAYWINDOW,
	GAMEUI_OBJECTIVESWINDOW,
	GAMEUI_DOCUMENTSWINDOW,
	GAMEUI_KILLWINDOWS
};

enum gameui_player_event_t
{
	GAMEUIEVENT_CLOSED_ALL_WINDOWS = 0,
	GAMEUIEVENT_CODE_MATCHES,
	GAMEUIEVENT_SUBWAY_SELECTION,
	GAMEUIEVENT_READ_OBJECTIVE
};

#endif //GAMEUI_SHARED_H