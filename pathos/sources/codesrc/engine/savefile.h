/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SAVEFILE_H
#define SAVEFILE_H

#include "constants.h"
#include "save_shared.h"

// Savefile version
static const Int32 SAVE_FILE_VERSION = 1;
// Max save file title length
static const Int32 SAVE_FILE_HEADER_MAX_LENGTH = 64;
// Max generic string length for save files
static const Int32 SAVE_FILE_FIELD_NAME_MAX_LENGTH = 16;
// Save file header Id
static const Char SAVEFILE_HEADER_ID[] = "PSF1";
// Save header in encoded form
static const Int32 SAVE_HEADER_ENCODED = (('1'<<24)+('F'<<16)+('S'<<8)+'P');

struct save_block_t
{
	save_block_t():
		index(0),
		datasize(0),
		dataoffset(0)
		{}

	Int32 index;
	Uint64 datasize;
	Int32 dataoffset;
};

struct save_field_t
{
	save_field_t():
		fieldnameindex(0),
		blockindex(0),
		numblocks(0),
		datatype(0)
		{}

	// Index into save file field names array
	Uint32 fieldnameindex;

	// Offset into save data
	Int32 blockindex;
	// Number of blocks
	Int32 numblocks;
	
	// Datatype
	byte datatype;
};

// Data for edicts
struct save_edict_info_t
{
	save_edict_info_t():
		entityindex(0),
		isworldspawn(false),
		isplayer(false),
		isglobalentity(false),
		statedata_startindex(0),
		statedata_numfields(0),
		fieldsdata_startindex(0),
		fieldsdata_numfields(0),
		classdata_startindex(0),
		classdata_numfields(0)
		{
			memset(classname, 0, sizeof(classname));
			memset(globalname, 0, sizeof(globalname));
		}

	// Entity index at time of save
	entindex_t entityindex;
	
	// Classname of the entity
	Char classname[SAVE_FILE_STRING_MAX_LENGTH];
	// Globalname of the entity
	Char globalname[SAVE_FILE_STRING_MAX_LENGTH];

	// Entity mins, used for transition adjustments
	Vector mins;

	// TRUE if this is the worldspawn entity
	bool isworldspawn;
	// TRUE if the player entity
	bool isplayer;
	// TRUE if this is an entity with a globalname
	bool isglobalentity;

	// Offset to entity state array in save data buffer
	Int32 statedata_startindex;
	// Number of entity state saved for save data buffer
	Int32 statedata_numfields;

	// Offset to entity fields array in string buffer
	Int32 fieldsdata_startindex;
	// Number of entity fields saved for string buffer
	Int32 fieldsdata_numfields;

	// Offset to entity fields array in save data buffer
	Int32 classdata_startindex;
	// Number of entity fields saved for save data buffer
	Int32 classdata_numfields;
};

struct save_decalinfo_t
{
	save_decalinfo_t():
		entityindex(NO_ENTITY_INDEX),
		identifier(0),
		flags(0)
		{
			memset(texturename, 0, sizeof(texturename));
		}

	// decal local origin
	Vector origin;
	// decal local normal
	Vector normal;

	// Decal texture name
	Char texturename[SAVE_FILE_STRING_MAX_LENGTH];

	// Entity index into local array
	entindex_t entityindex;
	// Edict's identifier
	Uint32 identifier;

	// Decal flags
	Int32 flags;
};

// Connections from map to others
struct save_level_connection_t
{
	save_level_connection_t()
	{
		memset(othermapname, 0, sizeof(othermapname));
		memset(landmarkname, 0, sizeof(landmarkname));
	}
	
	// Name of the map at connection
	Char othermapname[SAVE_FILE_STRING_MAX_LENGTH];
	// Landmark name at connection
	Char landmarkname[SAVE_FILE_STRING_MAX_LENGTH];
};

// Connections for single level
struct save_levelinfo_t
{
	save_levelinfo_t():
		connectioninfoindex(0),
		numconnections(0)
	{
		memset(mapname, 0, sizeof(mapname));
		memset(mapsavename, 0, sizeof(mapsavename));
	}

	// Name of the level
	Char mapname[SAVE_FILE_STRING_MAX_LENGTH];
	// Save file tied to the level
	Char mapsavename[SAVE_FILE_STRING_MAX_LENGTH];

	// Offset to connections array
	Int32 connectioninfoindex;
	// Number of connections on this map
	Int32 numconnections;
};

struct save_global_t
{
	save_global_t():
		state(0)
		{
			memset(name, 0, sizeof(name));
			memset(levelname, 0, sizeof(levelname));
		}

	// Name of global
	Char name[SAVE_FILE_STRING_MAX_LENGTH];
	// Name of level where this was created
	Char levelname[SAVE_FILE_STRING_MAX_LENGTH];
	// Global state
	Int32 state;
};

struct save_header_t
{
	explicit save_header_t(savefile_type_t type):
		id(SAVE_HEADER_ENCODED),
		version(SAVE_FILE_VERSION),
		type(type),
		svtime(0),
		gametime(0),
		skill(0),
		entitydescoffset(0),
		numentities(0),
		globaldataoffset(0),
		numglobals(0),
		entityfieldsoffset(0),
		numentityfields(0),
		stringblocksoffset(0),
		numstringblocks(0),
		entitydatablocksoffset(0),
		numentitydatablocks(0),
		stringdataoffset(0),
		stringdatasize(0),
		entitydataoffset(0),
		entitydatasize(0),
		decaldataoffset(0),
		numdecals(0),
		screenshotoffset(0),
		screenshotdatasize(0),
		screenshotwidth(0),
		screenshotheight(0),
		screenshotbpp(0),
		levelinfosdataoffset(0),
		numlevelinfos(0),
		filesize(0)
		{
			memset(name, 0, sizeof(name));
			memset(saveheader, 0, sizeof(saveheader));
			memset(mapname, 0, sizeof(mapname));
			memset(skyname, 0, sizeof(skyname));
		}

	// Save file ID
	Int32 id;

	// Version of save file
	Int32 version;
	// Save type
	savefile_type_t type;

	// Save file name
	Char name[MAX_PARSE_LENGTH];

	// Header for save file
	Char saveheader[SAVE_FILE_HEADER_MAX_LENGTH];
	// Name of map on which we saved
	Char mapname[SAVE_FILE_STRING_MAX_LENGTH];

	// Time spent on level
	Double svtime;
	// Total game time
	Double gametime;
	// Skill setting
	Int32 skill;

	// Sky color
	Vector skycolor;
	// Sky vector
	Vector skyvec;
	// Skybox name
	Char skyname[SAVE_FILE_STRING_MAX_LENGTH];

	// Offset to entity data
	Int32 entitydescoffset;
	// Number of entities
	Int32 numentities;

	// Offset to global data
	Int32 globaldataoffset;
	// Number of globals
	Int32 numglobals;

	// Offset to saved entity fields
	Int32 entityfieldsoffset;
	// Number of entity fields total
	Int32 numentityfields;

	// Offset to array of offsets to string values
	Int32 stringblocksoffset;
	// Number of strings saved to file
	Int32 numstringblocks;

	// Offset to entity data blocks
	Int32 entitydatablocksoffset;
	// Number of entity data blocks
	Int32 numentitydatablocks;

	// String data offset
	Int32 stringdataoffset;
	// Size of string data
	Int32 stringdatasize;

	// Entity raw data offset
	Int32 entitydataoffset;
	// Entity data size
	Int32 entitydatasize;

	// Offset to decal infos
	Int32 decaldataoffset;
	// Number of decals
	Int32 numdecals;

	// Screenshot data offset
	Int32 screenshotoffset;
	// Screenshot size
	Int32 screenshotdatasize;
	// Screenshot width
	Uint32 screenshotwidth;
	// Screenshot height
	Uint32 screenshotheight;
	// Screenshot bpp
	Uint32 screenshotbpp;

	// Level info data offset
	Int32 levelinfosdataoffset;
	// Number of level infos
	Int32 numlevelinfos;

	// Only used by transition files
	Vector landmarkorigin;

	// File size
	Int32 filesize;
};
#endif //SAVEFILE_H