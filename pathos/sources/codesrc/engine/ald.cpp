/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "modelcache.h"
#include "cache_model.h"
#include "aldformat.h"
#include "file.h"
#include "common.h"
#include "brushmodel.h"
#include "system.h"
#include "r_common.h"
#include "enginestate.h"
#include "brushmodel.h"

//=============================================
// @brief
//
//=============================================
bool ALD_Load( daystage_t stage, byte** pdestarrays )
{
	// Get worldmodel
	cache_model_t* pworldcache = gModelCache.GetModelByIndex(WORLD_MODEL_INDEX);
	if(!pworldcache)
	{
		Con_Printf("%s - Couldn't get world model.\n", __FUNCTION__);
		return nullptr;
	}

	// Try to load the original file first
	CString filename = pworldcache->name;
	Uint32 begin = filename.find(0, ".bsp");
	if(begin != -1)
		filename.erase(begin, 4);
	filename << ".ald";

	// Make sure file dates are correct
	file_dateinfo_t bspFileDate;
	if(!FL_GetFileDate(pworldcache->name.c_str(), bspFileDate))
		return false;

	// Make sure file dates are correct
	file_dateinfo_t aldFileDate;
	if(!FL_GetFileDate(filename.c_str(), aldFileDate))
		return false;

	if(FL_CompareFileDates(bspFileDate, aldFileDate) < 0)
	{
		Con_EPrintf("%s - ALD file '%s' is older than the BSP file '%s'.\n", __FUNCTION__, filename.c_str(), pworldcache->name.c_str());
		return false;
	}

	Uint32 iSize = 0;
	const byte *pFile = FL_LoadFile(filename.c_str(), &iSize);
	if(!pFile)
		return false;

	const aldheader_t* ploadheader = reinterpret_cast<const aldheader_t*>(pFile);
	if(ploadheader->header != ALD_HEADER_ENCODED)
	{
		Con_EPrintf("%s - Invalid ALD file '%s'.\n", __FUNCTION__, filename.c_str());
		FL_FreeFile(pFile);
		return false;
	}

	if(ploadheader->version != ALD_HEADER_VERSION)
	{
		Con_EPrintf("%s - Invalid ALD file '%s', wrong version.\n", __FUNCTION__, filename.c_str());
		FL_FreeFile(pFile);
		return false;
	}

	// Get worldmodel ptr and check data consistency
	brushmodel_t* pworldmodel = pworldcache->getBrushmodel();

	if(static_cast<Uint32>(ploadheader->lightdatasize) != pworldmodel->lightdatasize)
	{
		Con_EPrintf("%s - ALD inconsistent with BSP light data size(%d vs %d).\n", __FUNCTION__, ploadheader->lightdatasize, pworldmodel->lightdatasize);
		FL_FreeFile(pFile);
		return false;
	}

	// Load lump infos
	const aldlump_t* plumps = reinterpret_cast<const aldlump_t*>(reinterpret_cast<const byte*>(ploadheader) + ploadheader->lumpoffset);

	// check if we have anything worth of use
	const aldlump_t* plump = nullptr;
	for(Int32 i = 0; i < ploadheader->numlumps; i++)
	{
		if((plumps[i].type == ALD_LUMP_NIGHTDATA_NOBUMP || plumps[i].type == ALD_LUMP_NIGHTDATA_BUMP) && stage == DAYSTAGE_NIGHTSTAGE 
			|| (plumps[i].type == ALD_LUMP_DAYLIGHT_RETURN_DATA_NOBUMP || plumps[i].type == ALD_LUMP_DAYLIGHT_RETURN_DATA_BUMP) && stage == DAYSTAGE_DAYLIGHT_RETURN
			|| plumps[i].type == ALD_LUMP_EXTERNAL_BUMP)
		{
			plump = &plumps[i];
			break;
		}
	}

	// Skip out on this if it's not relevant
	if(!plump)
	{
		FL_FreeFile(pFile);
		return false;
	}

	Uint32 worldLightmapLayerCount = 0;
	for(Uint32 i = 0; i < NB_SURF_LIGHTMAP_LAYERS; i++)
	{
		if(!pworldmodel->plightdata[i])
			break;

		worldLightmapLayerCount++;
	}

	Uint32 aldLightmapLayerCount = 0;
	for(Uint32 i = 0; i < NB_SURF_LIGHTMAP_LAYERS; i++)
	{
		if(!plump->layeroffsets[i])
			break;

		aldLightmapLayerCount++;
	}

	if(worldLightmapLayerCount != aldLightmapLayerCount)
	{
		Con_EPrintf("%s - ALD lump of type %d lightmap layer count is inconsistent with BSP lightmap layer count(%d vs %d).\n", __FUNCTION__, plump->type, worldLightmapLayerCount, aldLightmapLayerCount);
		FL_FreeFile(pFile);
		return false;
	}

	for(Uint32 i = 0; i < worldLightmapLayerCount; i++)
	{
		// Load in the lump and copy the data
		byte* paldlightdata = new byte[ploadheader->lightdatasize];
		const byte* pdatasrc = (pFile+plump->layeroffsets[i]);
		memcpy(paldlightdata, pdatasrc, sizeof(byte)*ploadheader->lightdatasize);

		// Set ptr for final
		pdestarrays[i] = paldlightdata;
	}

	// Release the file
	FL_FreeFile(pFile);
	return true;
}

//=============================================
// @brief
//
//=============================================
void ALD_ExportLightmaps( void )
{
	cache_model_t* pworldcache = gModelCache.GetModelByIndex(WORLD_MODEL_INDEX);
	if(!pworldcache)
	{
		Con_Printf("%s - Couldn't get world model.\n", __FUNCTION__);
		return;
	}

	// Try to load the original file first
	CString filepath = pworldcache->name;
	Uint32 begin = filepath.find(0, ".bsp");
	if(begin != -1)
		filepath.erase(begin, 4);
	filepath << ".ald";

	// Create buffer
	Int32 initialSize = sizeof(aldheader_t)+sizeof(aldlump_t);
	CBuffer fileBuffer(nullptr, initialSize);

	// Begin writing to the file
	Uint32 curlumpidx = 0;

	// Retreive data ptr
	void*& pdataptr = fileBuffer.getbufferdata();
	aldheader_t* pheader = reinterpret_cast<aldheader_t*>(pdataptr);
	Uint32 fileofs = sizeof(aldheader_t);

	// Get worldmodel ptr
	brushmodel_t* pworldmodel = pworldcache->getBrushmodel();

	// Set header
	pheader->header = ALD_HEADER_ENCODED;
	pheader->version = ALD_HEADER_VERSION;
	pheader->lumpoffset = fileofs;
	pheader->lightdatasize = pworldmodel->lightdatasize;

	// Set lump info
	aldlump_t* plumps = reinterpret_cast<aldlump_t*>(reinterpret_cast<byte*>(pheader) + pheader->lumpoffset);
	pheader->numlumps = 1;

	fileofs += sizeof(aldlump_t)*pheader->numlumps;

	Uint32 i = 0;
	for(; i < NB_SURF_LIGHTMAP_LAYERS; i++)
	{
		if(!pworldmodel->plightdata[i])
			break;

		// Set the offset and increment
		plumps[curlumpidx].layeroffsets[i] = fileofs;
		fileofs += pworldmodel->lightdatasize;

		// Append to the file buffer itself
		fileBuffer.append(pworldmodel->plightdata[i], pworldmodel->lightdatasize);
	}

	// Set type based on how many layers we processed total
	if(i == NB_SURF_LIGHTMAP_LAYERS)
		plumps[curlumpidx].type = ALD_LUMP_NIGHTDATA_BUMP;
	else
		plumps[curlumpidx].type = ALD_LUMP_NIGHTDATA_NOBUMP;

	// Re-set data ptr
	const byte* pdata = reinterpret_cast<const byte*>(fileBuffer.getbufferdata());
	if(!FL_WriteFile(pdata, fileBuffer.getsize(), filepath.c_str()))
		Con_Printf("%s - Failed to write '%s'.\n", __FUNCTION__, filepath.c_str());
}