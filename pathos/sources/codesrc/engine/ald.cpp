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
#include "miniz.h"
#include "ald.h"
#include "pbspv2file.h"

//=============================================
// @brief
//
//=============================================
const byte* ALD_LoadALDFile( const Char* pstrFilepath, cache_model_t* pworldcache )
{
	// Make sure file dates are correct
	file_dateinfo_t bspFileDate;
	if(!FL_GetFileDate(pworldcache->name.c_str(), bspFileDate))
		return nullptr;

	// Make sure file dates are correct
	file_dateinfo_t aldFileDate;
	if(!FL_GetFileDate(pstrFilepath, aldFileDate))
		return nullptr;

	if(FL_CompareFileDates(bspFileDate, aldFileDate) < 0)
	{
		Con_EPrintf("%s - ALD file '%s' is older than the BSP file '%s'.\n", __FUNCTION__, pstrFilepath, pworldcache->name.c_str());
		return nullptr;
	}

	Uint32 iSize = 0;
	const byte *pFile = FL_LoadFile(pstrFilepath, &iSize);
	if(!pFile)
		return nullptr;

	const aldheader_t* ploadheader = reinterpret_cast<const aldheader_t*>(pFile);
	if(ploadheader->header != ALD_HEADER_ENCODED)
	{
		Con_EPrintf("%s - Invalid ALD file '%s'.\n", __FUNCTION__, pstrFilepath);
		FL_FreeFile(pFile);
		return nullptr;
	}

	if(ploadheader->version != ALD_HEADER_VERSION)
	{
		Con_EPrintf("%s - Invalid ALD file '%s', wrong version.\n", __FUNCTION__, pstrFilepath);
		FL_FreeFile(pFile);
		return nullptr;
	}

	// Get worldmodel ptr and check data consistency
	brushmodel_t* pworldmodel = pworldcache->getBrushmodel();
	if(static_cast<Uint32>(ploadheader->lightdatasize) != pworldmodel->lightdatasize)
	{
		Con_EPrintf("%s - ALD inconsistent with BSP light data size(%d vs %d).\n", __FUNCTION__, ploadheader->lightdatasize, pworldmodel->lightdatasize);
		FL_FreeFile(pFile);
		return nullptr;
	}

	return pFile;
}

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
		return false;
	}

	// Build the file name
	CString filename = ALD_GetFilePath(stage, pworldcache);
	if(filename.empty())
	{
		Con_Printf("%s - Failure while trying to set up ALD file path.\n", __FUNCTION__);
		return false;
	}

	const byte* pFile = ALD_LoadALDFile(filename.c_str(), pworldcache);
	if(!pFile)
		return nullptr;

	// Load lump infos
	const aldheader_t* ploadheader = reinterpret_cast<const aldheader_t*>(pFile);
	const aldlump_t* plumps = reinterpret_cast<const aldlump_t*>(reinterpret_cast<const byte*>(ploadheader) + ploadheader->lumpoffset);

	// Get worldmodel ptr and check data consistency
	brushmodel_t* pworldmodel = pworldcache->getBrushmodel();

	// Set type based on how many layers we processed total
	aldlumptype_t soughttype;
	switch(stage)
	{
	case DAYSTAGE_DAYLIGHT_RETURN:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				soughttype = ALD_LUMP_DAYLIGHT_RETURN_DATA_BUMP;
			else
				soughttype = ALD_LUMP_DAYLIGHT_RETURN_DATA_NOBUMP;
		}
		break;
	case DAYSTAGE_NIGHTSTAGE:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				soughttype = ALD_LUMP_NIGHTDATA_BUMP;
			else
				soughttype = ALD_LUMP_NIGHTDATA_NOBUMP;
		}
		break;
	case DAYSTAGE_NORMAL:
	default:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				soughttype = ALD_LUMP_EXTERNAL_BUMP;
			else
				soughttype = ALD_LUMP_EXTERNAL_NOBUMP;
		}
		break;
	}

	// check if we have anything worth of use
	const aldlump_t* plump = nullptr;
	for(Int32 i = 0; i < ploadheader->numlumps; i++)
	{
		if(plumps[i].type == soughttype)
		{
			plump = &plumps[i];
			break;
		}
	}

	// Skip out on this if it's not relevant
	if(!plump)
	{
		Con_DPrintf("%s - ALD file '%s' has no relevant lightmap data.\n", __FUNCTION__, filename.c_str());
		FL_FreeFile(pFile);
		return false;
	}

	Uint32 aldLightmapLayerCount = 0;
	for(Uint32 i = 0; i < NB_SURF_LIGHTMAP_LAYERS; i++)
	{
		if(!plump->layeroffsets[i])
			break;

		aldLightmapLayerCount++;
	}

	if(pworldmodel->lightmaplayercount != aldLightmapLayerCount)
	{
		Con_EPrintf("%s - ALD lump of type %d lightmap layer count is inconsistent with BSP lightmap layer count(%d vs %d).\n", __FUNCTION__, plump->type, pworldmodel->lightmaplayercount, aldLightmapLayerCount);
		FL_FreeFile(pFile);
		return false;
	}

	for(Uint32 i = 0; i < pworldmodel->lightmaplayercount; i++)
	{
		// Load in the lump and copy the data
		mz_ulong destinationsize = ploadheader->lightdatasize;
		byte* paldlightdata = new byte[destinationsize];

		// Get ptr to layer
		const aldlayer_t* player = reinterpret_cast<const aldlayer_t*>(reinterpret_cast<const byte*>(ploadheader) + plump->layeroffsets[i]);
		const byte* pdatasrc = (pFile + player->dataoffset);

		switch(player->compression)
		{
		case ALD_COMPRESSION_NONE:
			memcpy(paldlightdata, pdatasrc, sizeof(byte)*ploadheader->lightdatasize);
			break;
		case ALD_COMPRESSION_MINIZ:
			{
				Int32 status = uncompress(paldlightdata, &destinationsize, pdatasrc, player->datasize);
				if(status != MZ_OK)
				{
					Con_EPrintf("%s - Miniz uncompress2 failed on lump of type %d layer %d with error code %d.\n", __FUNCTION__, plump->type, i, status);
					delete[] paldlightdata;
					FL_FreeFile(pFile);
					return false;
				}

				if(ploadheader->lightdatasize != static_cast<Int32>(destinationsize))
				{
					Con_EPrintf("%s - Miniz uncompress produced inconsistent output size (expected %d, got %d instead).\n", __FUNCTION__, ploadheader->lightdatasize, destinationsize);
					delete[] paldlightdata;
					FL_FreeFile(pFile);
					return false;
				}
			}
			break;
		}

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
void ALD_ExportLightmaps( aldcompression_t compressionType, daystage_t daystage )
{
	cache_model_t* pworldcache = gModelCache.GetModelByIndex(WORLD_MODEL_INDEX);
	if(!pworldcache)
	{
		Con_Printf("%s - Couldn't get world model.\n", __FUNCTION__);
		return;
	}

	// Get worldmodel ptr
	brushmodel_t* pworldmodel = pworldcache->getBrushmodel();

	// Set type based on how many layers we processed total
	Int32 lumptype;
	switch(daystage)
	{
	case DAYSTAGE_DAYLIGHT_RETURN:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				lumptype = ALD_LUMP_DAYLIGHT_RETURN_DATA_BUMP;
			else
				lumptype = ALD_LUMP_DAYLIGHT_RETURN_DATA_NOBUMP;
		}
		break;
	case DAYSTAGE_NIGHTSTAGE:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				lumptype = ALD_LUMP_NIGHTDATA_BUMP;
			else
				lumptype = ALD_LUMP_NIGHTDATA_NOBUMP;
		}
		break;
	case DAYSTAGE_NORMAL:
	case DAYSTAGE_NORMAL_RESTORE:
	default:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				lumptype = ALD_LUMP_EXTERNAL_BUMP;
			else
				lumptype = ALD_LUMP_EXTERNAL_NOBUMP;
		}
		break;
	}

	// Build the file name
	CString filename = ALD_GetFilePath(daystage, pworldcache);
	if(filename.empty())
	{
		Con_Printf("%s - Failure while trying to set up ALD file path.\n", __FUNCTION__);
		return;
	}

	// Create buffer
	Int32 initialSize = sizeof(aldheader_t);
	CBuffer fileBuffer(nullptr, initialSize);

	// Begin writing to the file
	Uint32 curlumpidx = 0;

	// Retreive data ptr
	void*& pdataptr = fileBuffer.getbufferdata();
	aldheader_t* pheader = reinterpret_cast<aldheader_t*>(pdataptr);
	fileBuffer.addpointer(reinterpret_cast<void**>(&pheader));

	// Set header
	pheader->header = ALD_HEADER_ENCODED;
	pheader->version = ALD_HEADER_VERSION;
	pheader->lightdatasize = pworldmodel->lightdatasize;

	// Check for original and include it's data
	const byte* poriginal = FL_LoadFile(filename.c_str());
	if(poriginal)
	{
		const aldheader_t* poldhdr = reinterpret_cast<const aldheader_t*>(poriginal);

		// Count how many usable lumps there are in the original
		Uint32 oldlumpaddcount = 0;
		// Only add if the light data size matches
		if(poldhdr->lightdatasize == static_cast<Int32>(pworldmodel->lightdatasize))
		{
			for(Uint32 i = 0; i < poldhdr->numlumps; i++)
			{
				const aldlump_t* poldlump = reinterpret_cast<const aldlump_t*>(reinterpret_cast<const byte*>(poldhdr) + poldhdr->lumpoffset) + i;
				if(poldlump->type == lumptype)
					continue;

				oldlumpaddcount++;
			}
		}

		// Only bother if we have any valid ones to port over
		if(oldlumpaddcount)
		{
			// Set pump offset
			pheader->lumpoffset = fileBuffer.getsize();
			fileBuffer.append(nullptr, sizeof(aldlump_t)*(oldlumpaddcount+1));

			for(Uint32 i = 0; i < poldhdr->numlumps; i++)
			{
				const aldlump_t* poldlump = reinterpret_cast<const aldlump_t*>(reinterpret_cast<const byte*>(poldhdr) + poldhdr->lumpoffset) + i;
				if(poldlump->type == lumptype)
					continue;

				aldlump_t* poutlump = reinterpret_cast<aldlump_t*>(reinterpret_cast<byte*>(pheader) + pheader->lumpoffset) + curlumpidx;
				fileBuffer.addpointer(reinterpret_cast<void**>(&poutlump));
				poutlump->type = poldlump->type;

				for(Uint32 j = 0; j < NB_SURF_LIGHTMAP_LAYERS; j++)
				{
					if(!poldlump->layeroffsets[j])
						break;

					// Set layer offset and allocate
					poutlump->layeroffsets[j] = fileBuffer.getsize();
					fileBuffer.append(nullptr, sizeof(aldlayer_t));

					// Set layer data
					const aldlayer_t* poldlayer = reinterpret_cast<const aldlayer_t*>(reinterpret_cast<const byte*>(poriginal) + poldlump->layeroffsets[j]);
					aldlayer_t* poutlayer = reinterpret_cast<aldlayer_t*>(reinterpret_cast<byte*>(pheader) + poutlump->layeroffsets[j]);
					poutlayer->compression = poldlayer->compression;
					poutlayer->compressionlevel = poldlayer->compressionlevel;
					poutlayer->datasize = poldlayer->datasize;
					poutlayer->dataoffset = fileBuffer.getsize();

					// Append the raw data
					const byte* psrcdata = reinterpret_cast<const byte*>(poriginal) + poldlayer->dataoffset;
					fileBuffer.append(psrcdata, sizeof(byte)*poldlayer->datasize);
				}

				fileBuffer.removepointer((const void**)poutlump);
				curlumpidx++;
			}
		}

		FL_FreeFile(poriginal);
	}

	// Append the the lump if we did not add any new ones
	if(curlumpidx == 0)
	{
		pheader->lumpoffset = fileBuffer.getsize();
		fileBuffer.append(nullptr, sizeof(aldlump_t));
	}

	// Set lump info
	aldlump_t* pnewlump = reinterpret_cast<aldlump_t*>(reinterpret_cast<byte*>(pheader) + pheader->lumpoffset) + curlumpidx;
	fileBuffer.addpointer(reinterpret_cast<void**>(&pnewlump));
	pheader->numlumps = curlumpidx+1;

	// Set type of new lump
	pnewlump->type = lumptype;

	// Set data in new lump
	for(Uint32 i = 0; i < NB_SURF_LIGHTMAP_LAYERS; i++)
	{
		// Optimize DAYSTAGE_NORMAL_RESTORE by using the compressed data from the BSP
		if(daystage == DAYSTAGE_NORMAL_RESTORE && pworldmodel->plightdata_original[i])
		{
			// Set the offset and increment
			pnewlump->layeroffsets[i] = fileBuffer.getsize();
			fileBuffer.append(nullptr, sizeof(aldlayer_t));

			aldlayer_t* player = reinterpret_cast<aldlayer_t*>(reinterpret_cast<byte*>(pheader) + pnewlump->layeroffsets[i]);
			player->dataoffset = fileBuffer.getsize();
			player->datasize = pworldmodel->original_lightdatasizes[i];
			player->compression = pworldmodel->original_compressiontype[i];
			player->compressionlevel = pworldmodel->original_compressionlevel[i];

			switch(pworldmodel->original_compressiontype[i])
			{
			case BSP_LMAP_COMPRESSION_MINIZ:
				player->compression = ALD_COMPRESSION_MINIZ;
				break;
			case BSP_LMAP_COMPRESSION_NONE:
			default:
				player->compression = ALD_COMPRESSION_NONE;
				break;
			}

			// append the original compressed data
			fileBuffer.append(pworldmodel->plightdata_original[i], player->datasize);
		}
		else
		{
			if(!pworldmodel->plightdata[i])
				break;

			// Set the offset and increment
			pnewlump->layeroffsets[i] = fileBuffer.getsize();
			fileBuffer.append(nullptr, sizeof(aldlayer_t));

			aldlayer_t* player = reinterpret_cast<aldlayer_t*>(reinterpret_cast<byte*>(pheader) + pnewlump->layeroffsets[i]);
			fileBuffer.addpointer(reinterpret_cast<void**>(&player));

			player->compression = compressionType;
			player->dataoffset = fileBuffer.getsize();
		
			switch(compressionType)
			{
			case ALD_COMPRESSION_NONE:
				{
					fileBuffer.append(pworldmodel->plightdata[i], pworldmodel->lightdatasize);
					player->datasize = pworldmodel->lightdatasize;
					player->compressionlevel = 0;
				}
				break;
			case ALD_COMPRESSION_MINIZ:
				{
					Uint32 destsize = compressBound(pworldmodel->lightdatasize);
					byte* pdestination = new byte[destsize];
					memset(pdestination, 0, sizeof(byte)*destsize);

					mz_ulong resultsize = destsize;
					const byte* pdatasrc = reinterpret_cast<const byte*>(pworldmodel->plightdata[i]);
					Int32 result = compress2(pdestination, &resultsize, pdatasrc, pworldmodel->lightdatasize, MZ_DEFAULT_COMPRESSION);
					if(result != MZ_OK)
					{
						printf("%s - Failed to compress layer %d, compress returned %d.\n", __FUNCTION__, i, result);
						delete[] pdestination;
						return;
					}

					fileBuffer.append(pdestination, resultsize);
					delete[] pdestination;

					player->datasize = resultsize;
					player->compressionlevel = MZ_DEFAULT_COMPRESSION;
				}
				break;
			}

			// Remove this ptr
			fileBuffer.removepointer((const void**)&player);
		}
	}

	// Re-set data ptr
	const byte* pdata = reinterpret_cast<const byte*>(fileBuffer.getbufferdata());
	if(!FL_WriteFile(pdata, fileBuffer.getsize(), filename.c_str()))
		Con_Printf("%s - Failed to write '%s'.\n", __FUNCTION__, filename.c_str());
}

//=============================================
// @brief
//
//=============================================
void ALD_CopyAndExportLightmaps( const Char* psrcaldfilename, daystage_t srcstage, daystage_t dststage )
{
	cache_model_t* pworldcache = gModelCache.GetModelByIndex(WORLD_MODEL_INDEX);
	if(!pworldcache)
	{
		Con_Printf("%s - Couldn't get world model.\n", __FUNCTION__);
		return;
	}

	// Get worldmodel ptr
	brushmodel_t* pworldmodel = pworldcache->getBrushmodel();

	// Set type based on how many layers we processed total
	Int32 dstlumptype;
	switch(dststage)
	{
	case DAYSTAGE_DAYLIGHT_RETURN:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				dstlumptype = ALD_LUMP_DAYLIGHT_RETURN_DATA_BUMP;
			else
				dstlumptype = ALD_LUMP_DAYLIGHT_RETURN_DATA_NOBUMP;
		}
		break;
	case DAYSTAGE_NIGHTSTAGE:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				dstlumptype = ALD_LUMP_NIGHTDATA_BUMP;
			else
				dstlumptype = ALD_LUMP_NIGHTDATA_NOBUMP;
		}
		break;
	case DAYSTAGE_NORMAL:
	case DAYSTAGE_NORMAL_RESTORE:
	default:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				dstlumptype = ALD_LUMP_EXTERNAL_BUMP;
			else
				dstlumptype = ALD_LUMP_EXTERNAL_NOBUMP;
		}
		break;
	}

	// Build the file name
	CString filename = ALD_GetFilePath(dststage, pworldcache);
	if(filename.empty())
	{
		Con_Printf("%s - Failure while trying to set up ALD file path.\n", __FUNCTION__);
		return;
	}

	// Create buffer
	Int32 initialSize = sizeof(aldheader_t);
	CBuffer fileBuffer(nullptr, initialSize);

	// Begin writing to the file
	Uint32 curlumpidx = 0;

	// Retreive data ptr
	void*& pdataptr = fileBuffer.getbufferdata();
	aldheader_t* pheader = reinterpret_cast<aldheader_t*>(pdataptr);
	fileBuffer.addpointer(reinterpret_cast<void**>(&pheader));

	// Set header
	pheader->header = ALD_HEADER_ENCODED;
	pheader->version = ALD_HEADER_VERSION;
	pheader->lightdatasize = pworldmodel->lightdatasize;

	// Check for original and include it's data
	const byte* poriginal = FL_LoadFile(filename.c_str());
	if(poriginal)
	{
		const aldheader_t* poldhdr = reinterpret_cast<const aldheader_t*>(poriginal);

		// Count how many usable lumps there are in the original
		Uint32 oldlumpaddcount = 0;
		if(pheader->lightdatasize == poldhdr->lightdatasize)
		{
			for(Uint32 i = 0; i < poldhdr->numlumps; i++)
			{
				const aldlump_t* poldlump = reinterpret_cast<const aldlump_t*>(reinterpret_cast<const byte*>(poldhdr) + poldhdr->lumpoffset) + i;
				if(poldlump->type == dstlumptype)
					continue;

				oldlumpaddcount++;
			}
		}

		// Only bother if we have any valid ones to port over
		if(oldlumpaddcount)
		{
			// Set pump offset
			pheader->lumpoffset = fileBuffer.getsize();
			fileBuffer.append(nullptr, sizeof(aldlump_t)*(oldlumpaddcount+1));

			for(Uint32 i = 0; i < poldhdr->numlumps; i++)
			{
				const aldlump_t* poldlump = reinterpret_cast<const aldlump_t*>(reinterpret_cast<const byte*>(poldhdr) + poldhdr->lumpoffset) + i;
				if(poldlump->type == dstlumptype)
					continue;

				aldlump_t* poutlump = reinterpret_cast<aldlump_t*>(reinterpret_cast<byte*>(pheader) + pheader->lumpoffset) + curlumpidx;
				fileBuffer.addpointer(reinterpret_cast<void**>(&poutlump));
				poutlump->type = poldlump->type;

				for(Uint32 j = 0; j < NB_SURF_LIGHTMAP_LAYERS; j++)
				{
					if(!poldlump->layeroffsets[j])
						break;

					// Set layer offset and allocate
					poutlump->layeroffsets[j] = fileBuffer.getsize();
					fileBuffer.append(nullptr, sizeof(aldlayer_t));

					// Set layer data
					const aldlayer_t* poldlayer = reinterpret_cast<const aldlayer_t*>(reinterpret_cast<const byte*>(poriginal) + poldlump->layeroffsets[j]);
					aldlayer_t* poutlayer = reinterpret_cast<aldlayer_t*>(reinterpret_cast<byte*>(pheader) + poutlump->layeroffsets[j]);
					poutlayer->compression = poldlayer->compression;
					poutlayer->compressionlevel = poldlayer->compressionlevel;
					poutlayer->datasize = poldlayer->datasize;
					poutlayer->dataoffset = fileBuffer.getsize();

					// Append the raw data
					const byte* psrcdata = reinterpret_cast<const byte*>(poriginal) + poldlayer->dataoffset;
					fileBuffer.append(psrcdata, sizeof(byte)*poldlayer->datasize);
				}

				fileBuffer.removepointer((const void**)poutlump);
				curlumpidx++;
			}
		}

		FL_FreeFile(poriginal);
	}

	// Append the the lump if we did not add any new ones
	if(curlumpidx == 0)
	{
		pheader->lumpoffset = fileBuffer.getsize();
		fileBuffer.append(nullptr, sizeof(aldlump_t));
	}

	// Set lump info
	aldlump_t* pnewlump = reinterpret_cast<aldlump_t*>(reinterpret_cast<byte*>(pheader) + pheader->lumpoffset) + curlumpidx;
	fileBuffer.addpointer(reinterpret_cast<void**>(&pnewlump));
	pheader->numlumps = curlumpidx+1;
	pnewlump->type = dstlumptype;
	
	// Set type based on how many layers we processed total
	Int32 lumptype;
	switch(srcstage)
	{
	case DAYSTAGE_DAYLIGHT_RETURN:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				lumptype = ALD_LUMP_DAYLIGHT_RETURN_DATA_BUMP;
			else
				lumptype = ALD_LUMP_DAYLIGHT_RETURN_DATA_NOBUMP;
		}
		break;
	case DAYSTAGE_NIGHTSTAGE:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				lumptype = ALD_LUMP_NIGHTDATA_BUMP;
			else
				lumptype = ALD_LUMP_NIGHTDATA_NOBUMP;
		}
		break;
	case DAYSTAGE_NORMAL:
	case DAYSTAGE_NORMAL_RESTORE:
	default:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				lumptype = ALD_LUMP_EXTERNAL_BUMP;
			else
				lumptype = ALD_LUMP_EXTERNAL_NOBUMP;
		}
		break;
	}

	// Check for original and include it's data
	CString copyfilename = ALD_GetFilePath(srcstage, pworldcache);
	const byte* pcopyfile = FL_LoadFile(copyfilename.c_str());
	if(!pcopyfile)
	{
		Con_EPrintf("%s - Could not load backup file '%s'.\n", __FUNCTION__, copyfilename.c_str());
		return;
	}

	const aldheader_t* pcopyhdr = reinterpret_cast<const aldheader_t*>(pcopyfile);
	if(pheader->lightdatasize && pcopyhdr->lightdatasize != pheader->lightdatasize)
	{
		Con_EPrintf("%s - Inconsistent light data size between copy file '%s' and original '%s'.\n", __FUNCTION__, copyfilename.c_str(), filename.c_str());
		FL_FreeFile(pcopyfile);
		return;
	}

	// Only add if the light data size matches
	Int32 i = 0;
	for(; i < pcopyhdr->numlumps; i++)
	{
		const aldlump_t* pcopylump = reinterpret_cast<const aldlump_t*>(reinterpret_cast<const byte*>(pcopyhdr) + pcopyhdr->lumpoffset) + i;
		if(pcopylump->type == lumptype)
			break;
	}

	if(i == pcopyhdr->numlumps)
	{
		Con_EPrintf("%s - Copy file '%s' does not have the required lump data.\n", __FUNCTION__, copyfilename.c_str());
		FL_FreeFile(pcopyfile);
		return;
	}

	// Create new lump from copy file
	const aldlump_t* pcopylump = reinterpret_cast<const aldlump_t*>(reinterpret_cast<const byte*>(pcopyhdr) + pcopyhdr->lumpoffset) + i;
	fileBuffer.addpointer(reinterpret_cast<void**>(&pnewlump));

	for(Uint32 j = 0; j < NB_SURF_LIGHTMAP_LAYERS; j++)
	{
		if(!pcopylump->layeroffsets[j])
			break;

		// Set layer offset and allocate
		pnewlump->layeroffsets[j] = fileBuffer.getsize();
		fileBuffer.append(nullptr, sizeof(aldlayer_t));

		// Set layer data
		const aldlayer_t* poldlayer = reinterpret_cast<const aldlayer_t*>(reinterpret_cast<const byte*>(pcopyfile) + pcopylump->layeroffsets[j]);
		aldlayer_t* poutlayer = reinterpret_cast<aldlayer_t*>(reinterpret_cast<byte*>(pheader) + pnewlump->layeroffsets[j]);
		poutlayer->compression = poldlayer->compression;
		poutlayer->compressionlevel = poldlayer->compressionlevel;
		poutlayer->datasize = poldlayer->datasize;
		poutlayer->dataoffset = fileBuffer.getsize();

		// Append the raw data
		const byte* psrcdata = reinterpret_cast<const byte*>(pcopyfile) + poldlayer->dataoffset;
		fileBuffer.append(psrcdata, sizeof(byte)*poldlayer->datasize);
	}

	fileBuffer.removepointer((const void**)pnewlump);

	FL_FreeFile(pcopyfile);

	// Re-set data ptr
	const byte* pdata = reinterpret_cast<const byte*>(fileBuffer.getbufferdata());
	if(!FL_WriteFile(pdata, fileBuffer.getsize(), filename.c_str()))
		Con_Printf("%s - Failed to write '%s'.\n", __FUNCTION__, filename.c_str());
}

//=============================================
// @brief
//
//=============================================
bool ALD_HasStageData( daystage_t stage )
{
	// Get worldmodel
	cache_model_t* pworldcache = gModelCache.GetModelByIndex(WORLD_MODEL_INDEX);
	if(!pworldcache)
	{
		Con_Printf("%s - Couldn't get world model.\n", __FUNCTION__);
		return nullptr;
	}

	// Build the file name
	CString filename = ALD_GetFilePath(stage, pworldcache);
	if(filename.empty())
	{
		Con_Printf("%s - Failure while trying to set up ALD file path.\n", __FUNCTION__);
		return false;
	}

	const byte* pFile = ALD_LoadALDFile(filename.c_str(), pworldcache);
	if(!pFile)
		return nullptr;

	// Load lump infos
	const aldheader_t* ploadheader = reinterpret_cast<const aldheader_t*>(pFile);
	const aldlump_t* plumps = reinterpret_cast<const aldlump_t*>(reinterpret_cast<const byte*>(ploadheader) + ploadheader->lumpoffset);

	// Get worldmodel ptr and check data consistency
	brushmodel_t* pworldmodel = pworldcache->getBrushmodel();

	// Set type based on how many layers we processed total
	aldlumptype_t soughttype;
	switch(stage)
	{
	case DAYSTAGE_DAYLIGHT_RETURN:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				soughttype = ALD_LUMP_DAYLIGHT_RETURN_DATA_BUMP;
			else
				soughttype = ALD_LUMP_DAYLIGHT_RETURN_DATA_NOBUMP;
		}
		break;
	case DAYSTAGE_NIGHTSTAGE:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				soughttype = ALD_LUMP_NIGHTDATA_BUMP;
			else
				soughttype = ALD_LUMP_NIGHTDATA_NOBUMP;
		}
		break;
	case DAYSTAGE_NORMAL:
	case DAYSTAGE_NORMAL_RESTORE:
	default:
		{
			if(pworldmodel->lightmaplayercount == NB_SURF_LIGHTMAP_LAYERS)
				soughttype = ALD_LUMP_EXTERNAL_BUMP;
			else
				soughttype = ALD_LUMP_EXTERNAL_NOBUMP;
		}
		break;
	}

	// check if we have anything worth of use
	bool result = false;
	for(Int32 i = 0; i < ploadheader->numlumps; i++)
	{
		if(plumps[i].type == soughttype)
		{
			result = true;
			break;
		}
	}

	FL_FreeFile(pFile);
	return result;
}

//=============================================
// @brief
//
//=============================================
bool ALD_FileExists( void )
{
	// Get worldmodel
	cache_model_t* pworldcache = gModelCache.GetModelByIndex(WORLD_MODEL_INDEX);
	if(!pworldcache)
		return false;

	// See if a "restore" file is present
	CString filename = pworldcache->name;
	Uint32 begin = filename.find(0, ".bsp");
	if(begin != CString::CSTRING_NO_POSITION)
		filename.erase(begin, 4);

	filename << ".ald";

	return FL_FileExists(filename.c_str());
}

//=============================================
// @brief
//
//=============================================
CString ALD_GetFilePath( daystage_t daystage, cache_model_t* pworldcache )
{
	CString filepath;
	if(daystage != DAYSTAGE_NORMAL_RESTORE)
	{
		filepath = pworldcache->name;
		Uint32 begin = filepath.find(0, ".bsp");
		if(begin != CString::CSTRING_NO_POSITION)
			filepath.erase(begin, 4);

		filepath << ".ald";
	}
	else
	{
		CString folderpath;
		folderpath << "tmp" << PATH_SLASH_CHAR;

		if(!FL_CreateDirectory(folderpath.c_str()))
		{
			Con_EPrintf("%s - Could not create folder '%s'.\n", __FUNCTION__, folderpath.c_str());
			return CString("");
		}

		CString basename;
		Common::Basename(pworldcache->name.c_str(), basename);

		// Set the file path
		filepath << folderpath << basename << "_restore.ald";
	}

	return filepath;
}

//=============================================
// @brief
//
//=============================================
void ALD_DeleteRestoreFile( void )
{
	// Get worldmodel
	cache_model_t* pworldcache = gModelCache.GetModelByIndex(WORLD_MODEL_INDEX);
	if(!pworldcache)
		return;

	// See if a "restore" file is present
	CString filename = ALD_GetFilePath(DAYSTAGE_NORMAL_RESTORE, pworldcache);
	if(filename.empty())
	{
		Con_Printf("%s - Failure while trying to set up ALD file path.\n", __FUNCTION__);
		return;
	}

	if(FL_FileExists(filename.c_str()))
		FL_DeleteFile(filename.c_str());
}

//=============================================
// @brief
//
//=============================================
void ALD_InitGame( void )
{
	// Write a temporary restore file
	ALD_ExportLightmaps(ALD_COMPRESSION_MINIZ, DAYSTAGE_NORMAL_RESTORE);
}

//=============================================
// @brief
//
//=============================================
void ALD_ClearGame( void )
{
	// Make sure this is cleaned up
	ALD_DeleteRestoreFile();
}