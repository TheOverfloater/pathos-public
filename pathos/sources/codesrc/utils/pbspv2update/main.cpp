/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.

===============================================
*/

//
// mdlexport.c: exports the textures of a .mdl file and creates .pmf entries
// models/<scriptname>.mdl.
//

#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )

#include <SDL.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include "includes.h"
#include "common.h"
#include "constants.h"
#include "pbspv2file.h"
#include "aldformat.h"
#include "cbuffer.h"
#include "com_math.h"
#include "miniz.h"

const Char* LAYER_NAMES[NB_SURF_LIGHTMAP_LAYERS] = {
	"SURF_LIGHTMAP_DEFAULT",
	"SURF_LIGHTMAP_VECTORS",
	"SURF_LIGHTMAP_AMBIENT",
	"SURF_LIGHTMAP_DIFFUSE",
};

const Char* LUMP_NAMES[PBSPV2_NB_LUMPS] = {
	"PBSPV2_LUMP_ENTITIES",
	"PBSPV2_LUMP_PLANES",
	"PBSPV2_LUMP_TEXTURES",
	"PBSPV2_LUMP_VERTEXES",
	"PBSPV2_LUMP_VISIBILITY",
	"PBSPV2_LUMP_NODES",
	"PBSPV2_LUMP_TEXINFO",
	"PBSPV2_LUMP_FACES",
	"PBSPV2_LUMP_LIGHTING_DEFAULT",
	"PBSPV2_LUMP_LIGHTING_AMBIENT",
	"PBSPV2_LUMP_LIGHTING_DIFFUSE",
	"PBSPV2_LUMP_LIGHTING_VECTORS",
	"PBSPV2_LUMP_CLIPNODES",
	"PBSPV2_LUMP_LEAFS",
	"PBSPV2_LUMP_MARKSURFACES",
	"PBSPV2_LUMP_EDGES",
	"PBSPV2_LUMP_SURFEDGES",
	"PBSPV2_LUMP_MODELS",
};

//===============================================
// 
//
//===============================================
bool DirectoryExists( const Char* dirPath )
{
	DWORD ftyp = GetFileAttributesA(dirPath);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

//===============================================
// 
//
//===============================================
bool FileExists( const Char* filepath )
{
	DWORD ftyp = GetFileAttributesA(filepath);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;

	return true;
}

//===============================================
// 
//
//===============================================
bool CreateDirectory( const Char* dirPath )
{
	DWORD type = GetFileAttributesA(dirPath);
	if(type == INVALID_FILE_ATTRIBUTES)
		return CreateDirectoryA(dirPath, NULL) ? true : false;

	// Check if it's a directory
	if(type & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	return false;
}

//===============================================
// 
//
//===============================================
byte* LoadFile( const Char* pstrPath, Uint32* pFileSize )
{
	SDL_RWops* pf = SDL_RWFromFile(pstrPath, "rb");
	if(!pf)
	{
		printf("%s - Failed to load file '%s'.\n", __FUNCTION__, pstrPath);
		return nullptr;
	}

	SDL_RWseek(pf, 0, RW_SEEK_END);
	Int32 size = static_cast<Int32>(SDL_RWtell(pf));
	SDL_RWseek(pf, 0, RW_SEEK_SET);

	byte* pbuffer = new byte[size+1];
	size_t numbytes = SDL_RWread(pf, pbuffer, 1, size);
	SDL_RWclose(pf);

	if(pFileSize)
		(*pFileSize) = numbytes;

	return pbuffer;
}

//===============================================
// 
//
//===============================================
bool WriteFile( const byte* pdata, Uint32 size, const Char* pstrpath, bool append )
{
	SDL_RWops* pf = SDL_RWFromFile(pstrpath, append ? "ab" : "wb");
	if(!pf)
	{
		printf("Failed to open %s for writing: %s.\n", pstrpath, SDL_GetError());
		SDL_ClearError();
		return false;
	}

	size_t numbytes = SDL_RWwrite(pf, pdata, 1, size);
	SDL_RWclose(pf);

	return (numbytes == size) ? true : false;
}

//===============================================
// 
//
//===============================================
void ALD_Convert( const Char* pstrOriginalName, const Char* pstrOutputPath )
{
	Uint32 aldfilesize = 0;
	const byte* pdata = LoadFile(pstrOriginalName, &aldfilesize);
	if(!pdata)
		return;

	printf("Processing file '%s'.\n", pstrOriginalName);

	Int32 headerId = Common::ByteToInt32(pdata);
	if(headerId != ALD_HEADER_ENCODED)
	{
		printf("%s - '%s' is not an PALD file.\n", __FUNCTION__, pstrOriginalName);
		delete[] pdata;
		return;
	}

	Int32 versionNumber = Common::ByteToInt32(pdata+sizeof(Int32));
	if(versionNumber != ALD_HEADER_VERSION)
	{
		printf("%s - '%s' is version %d.\n", __FUNCTION__, pstrOriginalName, ALD_HEADER_VERSION);
		delete[] pdata;
		return;
	}

	const aldheader_t* pinheader = reinterpret_cast<const aldheader_t*>(pdata);

	// Create output buffer
	Uint32 initialSize = sizeof(aldheader_t) + sizeof(aldlump_t)*pinheader->numlumps;
	Uint32 fileOffset = initialSize;
	CBuffer aldFileBuffer(nullptr, initialSize);

	aldheader_t* poutheader = reinterpret_cast<aldheader_t*>(aldFileBuffer.getbufferdata());
	aldFileBuffer.addpointer(reinterpret_cast<void**>(&poutheader));

	poutheader->flags = pinheader->flags;
	poutheader->header = pinheader->header;
	poutheader->lightdatasize = pinheader->lightdatasize;
	poutheader->lumpoffset = sizeof(aldheader_t);
	poutheader->numlumps = pinheader->numlumps;
	poutheader->version = pinheader->version;

	const aldlump_t* pinlumps = reinterpret_cast<const aldlump_t*>(reinterpret_cast<const byte*>(pinheader) + pinheader->lumpoffset);
	aldlump_t* poutlumps = reinterpret_cast<aldlump_t*>(reinterpret_cast<byte*>(poutheader) + poutheader->lumpoffset);
	aldFileBuffer.addpointer(reinterpret_cast<void**>(&poutlumps));

	Uint32 destsize = compressBound(poutheader->lightdatasize);
	byte* ptempdata = new byte[destsize];
	memset(ptempdata, 0, sizeof(byte)*destsize);

	for(Uint32 i = 0; i < poutheader->numlumps; i++)
	{
		poutlumps[i].type = pinlumps[i].type;

		for(Uint32 j = 0; j < NB_SURF_LIGHTMAP_LAYERS; j++)
		{
			if(pinlumps[i].layeroffsets[j] == 0)
				break;

			poutlumps[i].layeroffsets[j] = aldFileBuffer.getsize();
			aldFileBuffer.append(nullptr, sizeof(aldlayer_t));
			
			aldlayer_t* player = reinterpret_cast<aldlayer_t*>(reinterpret_cast<byte*>(poutheader) + poutlumps[i].layeroffsets[j]);
			player->compression = ALD_COMPRESSION_MINIZ;
			player->compressionlevel = MZ_UBER_COMPRESSION;
			player->dataoffset = aldFileBuffer.getsize();

			const byte* pindata = reinterpret_cast<const byte*>(pinheader) + pinlumps[i].layeroffsets[j];
			mz_ulong resultsize = destsize;
			Int32 result = compress2(ptempdata, &resultsize, pindata, poutheader->lightdatasize, player->compressionlevel);
			if(result != MZ_OK)
			{
				printf("%s - Failed to compress ALD lump %d, compress returned %d.\n", __FUNCTION__, i, result);
				delete[] ptempdata;
				delete[] pdata;
				return;
			}

			{
				Float prevsizemb = static_cast<Float>(poutheader->lightdatasize) / (1024.0 * 1024.0);
				Float finalsizemb = static_cast<Float>(resultsize) / (1024.0 * 1024.0);
				Int32 percentage = (finalsizemb / prevsizemb) * 100;
				printf("ALD: Layer %s of lump %d was reduced from %.2f mbytes to %.2f mbytes(%d%%).\n", LAYER_NAMES[j], i, prevsizemb, finalsizemb, percentage);
			}

			// Append to output file
			player->datasize = resultsize;
			aldFileBuffer.append(ptempdata, resultsize);
		}
	}

	delete[] ptempdata;
	delete[] pdata;

	{
		Float prevsizemb = static_cast<Float>(aldfilesize) / (1024.0 * 1024.0);
		Float finalsizemb = static_cast<Float>(aldFileBuffer.getsize()) / (1024.0 * 1024.0);
		Int32 percentage = (finalsizemb / prevsizemb) * 100;
		printf("BSP: File size was reduced from %.2f mbytes to %.2f mbytes(%d%%).\n", prevsizemb, finalsizemb, percentage);
	}

	// Write the data to the destination file
	CString filebasename;
	Common::Basename(pstrOriginalName, filebasename);

	CString fullOutputPath;
	fullOutputPath << pstrOutputPath << PATH_SLASH_CHAR << filebasename << ".ald";

	const byte* poutdata = reinterpret_cast<const byte*>(aldFileBuffer.getbufferdata());
	if(!WriteFile(poutdata, aldFileBuffer.getsize(), fullOutputPath.c_str(), false))
		printf("%s - Failed to write '%s'.\n", __FUNCTION__, fullOutputPath.c_str());
	else
		printf("Completed processing file '%s'.\n", fullOutputPath.c_str());
}

//=============================================
// @brief
//
//=============================================
bool BSP_LoadLighting( const byte* pfile, const dpbspv2lump_t& lump, byte*& poutdataptr, Uint32& outdatasize )
{
	if(!lump.size)
		return true;

	// Check if sizes are correct
	if(lump.size % sizeof(color24_t))
	{
		printf("%s - Inconsistent lump size.\n", __FUNCTION__);
		return false;
	}

	poutdataptr = reinterpret_cast<byte *>(new byte[lump.size]);
	outdatasize = lump.size;

	const byte *psrc = (pfile + lump.offset);
	memcpy(poutdataptr, psrc, sizeof(byte)*lump.size);

	return true;
}

//===============================================
// 
//
//===============================================
bool BSP_ConvertBSPFile( const Char* pstrFilePath, const Char* pstrOutputPath )
{
	Uint32 bspsize = 0;
	const byte* pdata = LoadFile(pstrFilePath, &bspsize);
	if(!pdata)
		return false;

	printf("Processing file '%s'.\n", pstrFilePath);

	Int32 headerId = Common::ByteToInt32(pdata);
	if(headerId != PBSP_HEADER)
	{
		printf("%s - '%s' is not a Pathos BSP file.\n", __FUNCTION__, pstrFilePath);
		delete[] pdata;
		return false;
	}

	Int32 headerVersion = Common::ByteToInt32(pdata + sizeof(Int32));
	if(headerVersion != PBSPV2_VERSION)
	{
		printf("%s - '%s' is not a version 1 Pathos BSP file(it is a version %d instead).\n", __FUNCTION__, pstrFilePath, headerVersion);
		delete[] pdata;
		return false;
	}

	// Create output buffer
	Uint32 initialSize = sizeof(dpbspv2header_t) + sizeof(dpbspv2lump_t)*PBSPV2_NB_LUMPS;
	Uint32 fileOffset = initialSize;
	CBuffer bspFileBuffer(nullptr, initialSize);

	const dpbspv2header_t* psrcheader = reinterpret_cast<const dpbspv2header_t*>(pdata);
	if(!psrcheader->lumps[PBSPV2_LUMP_LIGHTING_DEFAULT].size)
	{
		printf("%s - BSP file '%s' has no lighting data, not processed.\n", __FUNCTION__, pstrFilePath);
		delete[] pdata;
		return false;
	}

	dpbspv2header_t* pdstheader = reinterpret_cast<dpbspv2header_t*>(bspFileBuffer.getbufferdata());
	bspFileBuffer.addpointer(reinterpret_cast<void**>(&pdstheader));

	pdstheader->id = PBSP_HEADER;
	pdstheader->version = PBSPV2_VERSION;
	pdstheader->flags = PBSPV2_FL_NONE;

	CArray<Int32> conversionLumps;

	for(Uint32 i = 0; i < PBSPV2_NB_LUMPS; i++)
	{
		Int32 dstLumpIndex;
		switch(i)
		{
		case PBSPV2_LUMP_ENTITIES:
			dstLumpIndex = PBSPV2_LUMP_ENTITIES;
			break;
		case PBSPV2_LUMP_PLANES:
			dstLumpIndex = PBSPV2_LUMP_PLANES;
			break;
		case PBSPV2_LUMP_TEXTURES:
			dstLumpIndex = PBSPV2_LUMP_TEXTURES;
			break;
		case PBSPV2_LUMP_VERTEXES:
			dstLumpIndex = PBSPV2_LUMP_VERTEXES;
			break;
		case PBSPV2_LUMP_VISIBILITY:
			dstLumpIndex = PBSPV2_LUMP_VISIBILITY;
			break;
		case PBSPV2_LUMP_NODES:
			dstLumpIndex = PBSPV2_LUMP_NODES;
			break;
		case PBSPV2_LUMP_TEXINFO:
			dstLumpIndex = PBSPV2_LUMP_TEXINFO;
			break;
		case PBSPV2_LUMP_CLIPNODES:
			dstLumpIndex = PBSPV2_LUMP_CLIPNODES;
			break;
		case PBSPV2_LUMP_LEAFS:
			dstLumpIndex = PBSPV2_LUMP_LEAFS;
			break;
		case PBSPV2_LUMP_MARKSURFACES:
			dstLumpIndex = PBSPV2_LUMP_MARKSURFACES;
			break;
		case PBSPV2_LUMP_EDGES:
			dstLumpIndex = PBSPV2_LUMP_EDGES;
			break;
		case PBSPV2_LUMP_SURFEDGES:
			dstLumpIndex = PBSPV2_LUMP_SURFEDGES;
			break;
		case PBSPV2_LUMP_MODELS:
			dstLumpIndex = PBSPV2_LUMP_MODELS;
			break;
		case PBSPV2_LUMP_FACES:
			dstLumpIndex = PBSPV2_LUMP_FACES;
			break;
		default:
		case PBSPV2_LUMP_LIGHTING_DEFAULT:
		case PBSPV2_LUMP_LIGHTING_VECTORS:
		case PBSPV2_LUMP_LIGHTING_DIFFUSE:
		case PBSPV2_LUMP_LIGHTING_AMBIENT:
			if(psrcheader->lumps[i].size > 0)
				conversionLumps.push_back(i);
			dstLumpIndex = NO_POSITION;
			break;
		}

		if(dstLumpIndex != NO_POSITION)
		{
			const byte* psrclumpdata = pdata + psrcheader->lumps[i].offset;
			bspFileBuffer.append(psrclumpdata, psrcheader->lumps[i].size);

			pdstheader->lumps[dstLumpIndex].offset = fileOffset;
			pdstheader->lumps[dstLumpIndex].size = psrcheader->lumps[i].size;
			fileOffset += psrcheader->lumps[i].size;
		}
	}

	for(Uint32 i = 0; i < conversionLumps.size(); i++)
	{
		// Load lighting data from the BSP file
		Uint32 datasize = 0;
		byte* pdataptr = nullptr;

		// Load the lighting lump
		if(!BSP_LoadLighting(pdata, psrcheader->lumps[conversionLumps[i]], pdataptr, datasize))
		{
			printf("%s - Failed to load lump %d.\n", __FUNCTION__, conversionLumps[i]);
			delete[] pdata;
			return false;
		}

		Uint32 destsize = compressBound(datasize);
		byte* pdestination = new byte[destsize];
		memset(pdestination, 0, sizeof(byte)*destsize);

		mz_ulong resultsize = destsize;
		Int32 result = compress2(pdestination, &resultsize, pdataptr, datasize, MZ_UBER_COMPRESSION);
		if(result != MZ_OK)
		{
			printf("%s - Failed to compress lump %d, compress returned %d.\n", __FUNCTION__, conversionLumps[i], result);
			delete[] pdataptr;
			delete[] pdata;
			return false;
		}

		{
			Float prevsizemb = static_cast<Float>(datasize) / (1024.0 * 1024.0);
			Float finalsizemb = static_cast<Float>(resultsize) / (1024.0 * 1024.0);
			Int32 percentage = finalsizemb / prevsizemb * 100;
			printf("BSP: Lump %s was reduced from %.2f mbytes to %.2f mbytes(%d%%).\n", LUMP_NAMES[conversionLumps[i]], prevsizemb, finalsizemb, percentage);
		}

		// Delete original data
		delete[] pdataptr;

		dpbspv2lump_t* pdestlump = &pdstheader->lumps[conversionLumps[i]];
		bspFileBuffer.addpointer(reinterpret_cast<void**>(&pdestlump));

		// Add space for the compressed data header
		pdestlump->offset = bspFileBuffer.getsize();
		pdestlump->size = sizeof(dpbspv2lmapdata_t);
		bspFileBuffer.append(nullptr, sizeof(dpbspv2lmapdata_t));

		dpbspv2lmapdata_t* plmapdata = reinterpret_cast<dpbspv2lmapdata_t*>(reinterpret_cast<byte*>(pdstheader) + pdestlump->offset);
		plmapdata->compression = PBSPV2_LMAP_COMPRESSION_MINIZ;
		plmapdata->compressionlevel = MZ_UBER_COMPRESSION;
		plmapdata->dataoffset = bspFileBuffer.getsize();
		plmapdata->datasize = resultsize;
		plmapdata->noncompressedsize = datasize;

		// Append to file
		bspFileBuffer.append(pdestination, resultsize);
		bspFileBuffer.removepointer((const void**)&pdestlump);
		delete[] pdestination;
	}

	delete[] pdata;

	// Write BSP file to final destination
	CString basename;
	Common::Basename(pstrFilePath, basename);

	CString outputPath;
	outputPath << pstrOutputPath << PATH_SLASH_CHAR << basename << ".bsp";

	bool result;
	const byte* poutdata = reinterpret_cast<const byte*>(bspFileBuffer.getbufferdata());
	if(!WriteFile(poutdata, bspFileBuffer.getsize(), outputPath.c_str(), false))
	{
		printf("%s - Failed to write to path '%s'.\n", __FUNCTION__, outputPath.c_str());
		result = false;
	}
	else
		result = true;

	{
		Float prevsizemb = static_cast<Float>(bspsize) / (1024.0 * 1024.0);
		Float finalsizemb = static_cast<Float>(bspFileBuffer.getsize()) / (1024.0 * 1024.0);
		Int32 percentage = (finalsizemb / prevsizemb) * 100;
		printf("BSP: File size was reduced from %.2f mbytes to %.2f mbytes(%d%%).\n", prevsizemb, finalsizemb, percentage);
	}

	if(result)
		printf("Completed processing file '%s'.\n", pstrFilePath);
	else
		printf("Failed to process '%s'.\n", pstrFilePath);

	// Check if we have an ALD file
	CString aldFilepath(pstrFilePath);
	aldFilepath.erase(aldFilepath.find(0, ".bsp"), 4);
	aldFilepath << ".ald";

	if(FileExists(aldFilepath.c_str()))
		ALD_Convert(aldFilepath.c_str(), pstrOutputPath);

	return result;
}

//===============================================
// _tmain
//
//===============================================
int _tmain(Int32 argc, Char* argv[])
{
	// Check for usage
	if(argc != 3)
	{
		printf("Pathos BSP conversion tool, converts Pathos Version 2.0 BSP to Pathos BSP Version 2.1.\n");
		printf("Also converts any accompanying ALD files if present. Note: ALD files need for the BSP to be present.\n");
		printf("Usage: <target file/directory> <output directory>\n");
		printf("Press ENTER to exit...\n");
		getchar();
		return -1;
	}

	// Create dir if missing
	if(!DirectoryExists(argv[2]))
		CreateDirectory(argv[2]);

	Uint32 numConverted = 0;
	Uint32 numFailed = 0;
	Uint32 numTotal = 0;
	if(qstrstr(argv[1], ".bsp"))
	{
		// Path to BSP file
		CString filepath(argv[1]);

		// Perform the BSP format conversion
		if(FileExists(filepath.c_str()))
		{
			if(BSP_ConvertBSPFile(filepath.c_str(), argv[2]))
				numConverted++;
			else
				numFailed++;

			numTotal++;
		}
	}
	else
	{
		CString searchpath;
		searchpath << argv[1] << PATH_SLASH_CHAR << "*.bsp";

		// Parse directory for files
		HANDLE dir;
		WIN32_FIND_DATA file_data;
		if ((dir = FindFirstFile(searchpath.c_str(), &file_data)) == INVALID_HANDLE_VALUE)
		{
			printf("Directory %s not found.\n", argv[1]);
			return -1;
		}

		while (true) 
		{
			CString filepath;
			filepath << argv[1] << PATH_SLASH_CHAR << file_data.cFileName;

			if (qstrcmp(file_data.cFileName, ".") != 0 
				&& qstrcmp(file_data.cFileName, "..") != 0 
				&& qstrstr(file_data.cFileName, ".bsp")
				&& FileExists(filepath.c_str()))
			{
				// Perform the BSP format conversion
				if(BSP_ConvertBSPFile(filepath.c_str(), argv[2]))
					numConverted++;
				else
					numFailed++;

				numTotal++;
				printf("\n");
			}

			if(!FindNextFile(dir, &file_data))
				break;
		}
	}

	if(numConverted == 0)
	{
		printf("Error: BSP file(s) not found.\n");
		printf("Press any key to exit...\n");
		getchar();
		return 1;
	}
	else
	{
		printf("%d files converted to new BSP format, %d failed of %d total.\n", numConverted, numFailed, numTotal);
		printf("Press any key to exit...\n");
		getchar();
		return 0;
	}

	return 0;
}
