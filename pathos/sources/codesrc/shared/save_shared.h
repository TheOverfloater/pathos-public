/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SAVE_SHARED_H
#define SAVE_SHARED_H

// Max generic string length for save files
static const Int32 SAVE_FILE_STRING_MAX_LENGTH = 256;
// Maximum transitioning entities
static const Uint32 MAX_TRANSITIONING_ENTITIES = 512;

// Save file extension
static const Char SAVE_FILE_EXTENSION[] = ".psf";
// Quicksave file name
static const Char QUICKSAVE_FILE_NAME[] = "quick";
// Autosave file name
static const Char AUTOSAVE_FILE_NAME[] = "autosave";

// Entity field types
enum entfieldtype_t
{
	EFIELD_UNDEFINED = -1,
	EFIELD_FLOAT,		// A float vlaue
	EFIELD_DOUBLE,		// Double precision float
	EFIELD_STRING,		// CString object
	EFIELD_ENTINDEX,	// Server side entindex
	EFIELD_ENTPOINTER,	// Entity private data pointer
	EFIELD_EDICT,		// Edict pointer
	EFIELD_ENTSTATE,	// Entvars pointer
	EFIELD_EHANDLE,		// Entity handle
	EFIELD_VECTOR,		// Standard vector object
	EFIELD_COORD,		// Coordinate vector(fixed up at level transition)
	EFIELD_INT16,		// Signed 16-bit integer value
	EFIELD_UINT16,		// Unsigned 16-bit integer value
	EFIELD_INT32,		// Signed 32-bit integer value
	EFIELD_UINT32,		// Unsigned 32-bit integer
	EFIELD_INT64,		// Signed 64-bit integer value
	EFIELD_UINT64,		// Unsigned 64-bit integer value
	EFIELD_FUNCPTR,		// Function pointer for private data
	EFIELD_BOOLEAN,		// Boolean value
	EFIELD_BYTE,		// A single byte
	EFIELD_CHAR,		// A single character
	EFIELD_TIME,		// Time value(fixed up at level transition)
	EFIELD_MODELNAME,	// Model name(needs precache)
	EFIELD_SOUNDNAME,	// Sound name(needs precache)

	NB_FIELDTYPES
};

enum savefile_type_t
{
	SAVE_UNDEFINED = -1,
	SAVE_REGULAR, // Regular savefile, saved from menu or on exit
	SAVE_QUICK, // Quicksave made by player
	SAVE_AUTO, // Autosave by game
	SAVE_TRANSITION, // Transition data carried between levels
	SAVE_MAPSAVE // Save data for map made during transitioning
};
#endif // SAVE_SHARED_H