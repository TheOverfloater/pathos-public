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
static constexpr Int32 SAVE_FILE_STRING_MAX_LENGTH = 256;
// Maximum transitioning entities
static constexpr Uint32 MAX_TRANSITIONING_ENTITIES = 512;

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
	EFIELD_FLOAT,				// A float vlaue
	EFIELD_DOUBLE,				// Double precision float
	EFIELD_STRING,				// string_t object
	EFIELD_ENTINDEX,			// Server side entindex
	EFIELD_ENTPOINTER,			// Entity private data pointer
	EFIELD_EDICT,				// Edict pointer
	EFIELD_ENTSTATE,			// Entvars pointer
	EFIELD_EHANDLE,				// Entity handle
	EFIELD_VECTOR,				// Standard vector object
	EFIELD_COORD,				// Coordinate vector(fixed up at level transition)
	EFIELD_INT16,				// Signed 16-bit integer value
	EFIELD_UINT16,				// Unsigned 16-bit integer value
	EFIELD_INT32,				// Signed 32-bit integer value
	EFIELD_UINT32,				// Unsigned 32-bit integer
	EFIELD_INT64,				// Signed 64-bit integer value
	EFIELD_UINT64,				// Unsigned 64-bit integer value
	EFIELD_FUNCPTR,				// Function pointer for private data
	EFIELD_BOOLEAN,				// Boolean value
	EFIELD_BYTE,				// A single byte
	EFIELD_CHAR,				// A single character
	EFIELD_TIME,				// Time value(fixed up at level transition)
	EFIELD_MODELNAME,			// Model name(needs precache)
	EFIELD_SOUNDNAME,			// Sound name(needs precache)
	EFIELD_CARRAY_FLOAT,		// CArray of floats
	EFIELD_CARRAY_DOUBLE,		// CArray of double precision float
	EFIELD_CARRAY_STRING,		// CArray of string_t objects
	EFIELD_CARRAY_ENTINDEX,		// CArray of server side entindexes
	EFIELD_CARRAY_ENTPOINTER,	// CArray of entity private data pointers
	EFIELD_CARRAY_EDICT,		// CArray of edict pointers
	EFIELD_CARRAY_ENTSTATE,		// CArray of entvars pointers
	EFIELD_CARRAY_EHANDLE,		// CArray of entity handles
	EFIELD_CARRAY_VECTOR,		// CArray of vector objects
	EFIELD_CARRAY_COORD,		// CArray of coordinate vectors(fixed up at level transition)
	EFIELD_CARRAY_INT16,		// CArray of signed 16-bit integer value
	EFIELD_CARRAY_UINT16,		// CArray of unsigned 16-bit integer value
	EFIELD_CARRAY_INT32,		// CArray of signed 32-bit integer value
	EFIELD_CARRAY_UINT32,		// CArray of unsigned 32-bit integer
	EFIELD_CARRAY_INT64,		// CArray of signed 64-bit integer value
	EFIELD_CARRAY_UINT64,		// CArray of unsigned 64-bit integer value
	EFIELD_CARRAY_BOOLEAN,		// CArray of boolean values
	EFIELD_CARRAY_TIME,			// CArray of time values(fixed up at level transition)
	EFIELD_CBITSET,

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