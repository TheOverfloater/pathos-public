/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ALDHEADER_H
#define ALDHEADER_H

#include "brushmodel_shared.h"

#define ALD_HEADER_ENCODED		(('D'<<24)+('L'<<16)+('A'<<8)+'P')
#define ALD_HEADER_VERSION		1

enum aldlumptype_t 
{
	ALD_LUMP_NIGHTDATA_BUMP = 1, // nighttime light data for bsp
	ALD_LUMP_EDD_NIGHTDATA_NOBUMP, // nighttime light data for edd(not used in Pathos, only kept for legacy support)
	ALD_LUMP_EXTERNAL_BUMP, // externally stored regular BSP lightdata(not used, legacy support)
	ALD_LUMP_EXTERNAL_NOBUMP, // external data with no bump info(not used, legacy support)
	ALD_LUMP_NIGHTDATA_NOBUMP, // nighttime data with no bump
	ALD_LUMP_EDD_NIGHTDATA_BUMP, // edd data with no bump info(not used in Pathos, only kept for legacy support)
	ALD_LUMP_DAYLIGHT_RETURN_DATA_NOBUMP,
	ALD_LUMP_DAYLIGHT_RETURN_DATA_BUMP
};

enum aldcompression_t
{
	ALD_COMPRESSION_NONE = 0,
	ALD_COMPRESSION_MINIZ
};

struct aldheader_t
{
	aldheader_t():
		header(0),
		version(0),
		flags(0),
		lumpoffset(0),
		numlumps(0),
		lightdatasize(0)
		{}

	Int32 header;
	Int32 version;
	Int32 flags;

	Int32 lumpoffset;
	Int32 numlumps;
	Int32 lightdatasize;
};

struct aldlump_t
{
	aldlump_t():
		type(0)
	{
		for(Uint32 i = 0; i < NB_SURF_LIGHTMAP_LAYERS; i++)
			layeroffsets[i] = 0;
	}

	// Type of ALD lump
	Int32 type;
	// The offsets to each layer
	Int32 layeroffsets[NB_SURF_LIGHTMAP_LAYERS];
};

struct aldlayer_t
{
	aldlayer_t():
		compression(0),
		compressionlevel(0),
		dataoffset(0),
		datasize(0)
	{}

	Int32 compression;
	Int32 compressionlevel;
	Int32 dataoffset;
	Int32 datasize;
};
#endif