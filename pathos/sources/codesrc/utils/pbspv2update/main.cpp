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
#include "pbspv1file.h"
#include "pbspv2file.h"
#include "aldformat.h"
#include "cbuffer.h"
#include "com_math.h"

// Header for ALD version 3
#define ALD3_HEADER_ENCODED		(('3'<<24)+('D'<<16)+('L'<<8)+'A')

struct ald3lump_t
{
	ald3lump_t():
		type(0),
		lumpoffset(0),
		lumpsize(0)
	{}

	// Type of ALD lump
	Int32 type;
	// The offsets to the lump data
	Int32 lumpoffset;
	// The size of lighting data
	Int32 lumpsize;
};

struct ald3header_t
{
	ald3header_t():
		header(0),
		flags(0),
		lumpoffset(0),
		numlumps(0)
		{}

	Int32 header;
	Int32 flags;

	Int32 lumpoffset;
	Int32 numlumps;
};

// Pointer to ALD file data
ald3header_t* g_pALDFileData = nullptr;

struct ald_convdata_t
{
	ald_convdata_t():
		type(0),
		plightdata_original(nullptr)
	{
		for(Uint32 i = 0; i < NB_SURF_LIGHTMAP_LAYERS; i++)
			plightdata_layers[i] = nullptr;
	}

	~ald_convdata_t()
	{
		if(plightdata_original)
			delete[] plightdata_original;

		for(Uint32 i = 0; i < NB_SURF_LIGHTMAP_LAYERS; i++)
		{
			if(plightdata_layers[i])
				delete[] plightdata_layers[i];
		}
	}

	Int32 type;
	byte* plightdata_original;
	byte* plightdata_layers[NB_SURF_LIGHTMAP_LAYERS];
};

// Conversion data for the ALD file
CLinkedList<ald_convdata_t*> g_pALDConversionData;
// Flags of the original ALD file
Int32 g_pALDHeaderFlags = 0;

// Original BSP file lighting data
byte* g_pOriginalBSPLighting = nullptr;
// Original lighting data size
Uint32 g_originalBSPLightDataSize = 0;
// Final lighting data size
Uint32 g_finalBSPLightDataSize = 0;

// Original BSP texinfos
mtexinfo_t* g_pOriginalBSPTexInfos = nullptr;
// Original BSP surfedges
Int32* g_pOriginalBSPSurfEdges = nullptr;
// Original BSP edges
medge_t* g_pOriginalBSPEdges = nullptr;
// Original BSP vertexes
mvertex_t* g_pOriginalBSPVertexes = nullptr;

// Original BSP textures
mtexture_t* g_pOriginalBSPTextures = nullptr;
// Number of original BSP textures
Uint32 g_numOriginalBSPTextures = 0;

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
void ALD3_Clear( void )
{
	if(!g_pALDConversionData.empty())
	{
		g_pALDConversionData.begin();
		while(!g_pALDConversionData.end())
		{
			delete g_pALDConversionData.get();
			g_pALDConversionData.next();
		}

		g_pALDConversionData.clear();
	}

	g_pALDHeaderFlags = 0;
}

//===============================================
// 
//
//===============================================
void ALD3_LoadALDFile( const Char* pstrFilePath )
{
	const byte* pdata = LoadFile(pstrFilePath, nullptr);
	if(!pdata)
		return;

	ALD3_Clear();

	printf("Processing file '%s'.\n", pstrFilePath);

	Int32 headerId = Common::ByteToInt32(pdata);
	if(headerId != ALD3_HEADER_ENCODED)
	{
		printf("%s - '%s' is not an ALD3 file.\n", __FUNCTION__, pstrFilePath);
		delete[] pdata;
		return;
	}

	const ald3header_t* pald3header = reinterpret_cast<const ald3header_t*>(pdata);
	const ald3lump_t* plumps = reinterpret_cast<const ald3lump_t*>(reinterpret_cast<const byte*>(pald3header) + pald3header->lumpoffset);

	for(Uint32 i = 0; i < pald3header->numlumps; i++)
	{
		if(plumps[i].lumpsize != g_originalBSPLightDataSize)
		{
			printf("%s - '%s' has a mismatch in light data size(%d) with BSP light data size(%d) on lump %d.\n", __FUNCTION__, pstrFilePath, g_originalBSPLightDataSize, plumps[i].lumpsize, i);
			delete[] pdata;
			return;
		}
	}

	// Copy flags info
	g_pALDHeaderFlags = pald3header->flags;

	// Set up temporary buffers for lumps
	for(Uint32 i = 0; i < pald3header->numlumps; i++)
	{
		const byte* plumpdata = reinterpret_cast<const byte*>(pald3header) + plumps[i].lumpoffset;

		ald_convdata_t* pconvdata = new ald_convdata_t;
		pconvdata->type = plumps[i].type;
		pconvdata->plightdata_original = new byte[plumps[i].lumpsize];
		memcpy(pconvdata->plightdata_original, plumpdata, plumps[i].lumpsize);

		for(Uint32 j = 0; j < NB_SURF_LIGHTMAP_LAYERS; j++)
		{
			pconvdata->plightdata_layers[j] = new byte[plumps[i].lumpsize];
			memset(pconvdata->plightdata_layers[j], 0, sizeof(byte)*plumps[i].lumpsize);
		}

		g_pALDConversionData.radd(pconvdata);
	}

	delete[] pdata;
}

//===============================================
// 
//
//===============================================
void ALD3_Convert_Finish( const Char* pstrOriginalName, const Char* pstrOutputPath, bool hasBumpData )
{
	if(g_pALDConversionData.empty())
		return;

	Int32 initialSize = sizeof(aldheader_t) + sizeof(aldlump_t)*g_pALDConversionData.size();
	CBuffer fileBuffer(nullptr, initialSize);

	void*& pdataptr = fileBuffer.getbufferdata();
	aldheader_t* pheader = reinterpret_cast<aldheader_t*>(pdataptr);
	Uint32 fileofs = sizeof(aldheader_t);

	pheader->header = ALD_HEADER_ENCODED;
	pheader->version = ALD_HEADER_VERSION;
	pheader->flags = g_pALDHeaderFlags;
	pheader->lightdatasize = g_finalBSPLightDataSize;

	pheader->lumpoffset = fileofs;
	pheader->numlumps = g_pALDConversionData.size();
	fileofs += sizeof(aldlump_t)*pheader->numlumps;

	// Set lump info
	aldlump_t* plumps = reinterpret_cast<aldlump_t*>(reinterpret_cast<byte*>(pheader) + pheader->lumpoffset);
	fileBuffer.addpointer(reinterpret_cast<void**>(&plumps));
	
	// Set individual lump data
	Uint32 lumpIndex = 0;
	g_pALDConversionData.begin();
	while(!g_pALDConversionData.end())
	{
		ald_convdata_t* pconvdata = g_pALDConversionData.get();
		plumps[lumpIndex].type = pconvdata->type;

		if(hasBumpData)
		{
			for(Uint32 i = 0; i < NB_SURF_LIGHTMAP_LAYERS; i++)
			{
				fileBuffer.append(pconvdata->plightdata_layers[i], g_finalBSPLightDataSize);
				plumps[lumpIndex].layeroffsets[i] = fileofs;
				fileofs += g_finalBSPLightDataSize;
			}
		}
		else
		{
			fileBuffer.append(pconvdata->plightdata_layers[SURF_LIGHTMAP_DEFAULT], g_finalBSPLightDataSize);
			plumps[lumpIndex].layeroffsets[SURF_LIGHTMAP_DEFAULT] = fileofs;
			fileofs += g_finalBSPLightDataSize;
		}

		g_pALDConversionData.next();
		lumpIndex++;
	}

	// Write the data to the destination file
	CString filebasename;
	Common::Basename(pstrOriginalName, filebasename);

	CString fullOutputPath;
	fullOutputPath << pstrOutputPath << PATH_SLASH_CHAR << filebasename << ".ald";

	const byte* pdata = reinterpret_cast<const byte*>(fileBuffer.getbufferdata());
	if(!WriteFile(pdata, fileBuffer.getsize(), fullOutputPath.c_str(), false))
		printf("%s - Failed to write '%s'.\n", __FUNCTION__, fullOutputPath.c_str());

	// Clear these
	ALD3_Clear();

	printf("Completed processing file '%s'.\n", fullOutputPath.c_str());
}

//===============================================
// 
//
//===============================================
void ALD3_Surface_Convert( msurface_t* psurface, Uint32 originaloffset, Uint32 newoffset, Int32* layerIndexes, bool bumpData )
{
	if(g_pALDConversionData.empty())
		return;

	// Calculate extents
	Uint32 xsize = (psurface->extents[0]>>4)+1;
	Uint32 ysize = (psurface->extents[1]>>4)+1;
	Uint32 size = xsize*ysize;

	g_pALDConversionData.begin();
	while(!g_pALDConversionData.end())
	{
		ald_convdata_t* pconvdata = g_pALDConversionData.get();

		// Manage reorganization of lighting data into what the engine needs
		if(bumpData)
		{
			for(Uint32 j = 0; j < NB_SURF_LIGHTMAP_LAYERS; j++)
			{
				// Set final data offset and copy the data
				byte* pdestination = pconvdata->plightdata_layers[j] + newoffset;
				color24_t* psrclightdata = reinterpret_cast<color24_t*>(pconvdata->plightdata_original + originaloffset) + size * layerIndexes[j];
				memcpy(pdestination, psrclightdata, sizeof(color24_t)*size);
			}
		}
		else
		{
			// We only have the base layer
			byte* pdestination = pconvdata->plightdata_layers[SURF_LIGHTMAP_DEFAULT] + newoffset;
			color24_t* psrclightdata = reinterpret_cast<color24_t*>(pconvdata->plightdata_original + originaloffset);
			memcpy(pdestination, psrclightdata, sizeof(color24_t)*size);			
		}

		g_pALDConversionData.next();
	}
}

//=============================================
// @brief
//
//=============================================
bool BSP_CalcSurfaceExtents( msurface_t* psurf, Uint32 sampleSize )
{
	Vector vmins = NULL_MINS;
	Vector vmaxs = NULL_MAXS;

	Float mins[2] = { NULL_MINS[0], NULL_MINS[1] };
	Float maxs[2] = { NULL_MAXS[0], NULL_MAXS[1] };

	mtexinfo_t* ptexinfo = psurf->ptexinfo;
	for(Uint32 i = 0; i < psurf->numedges; i++)
	{
		// Get the vertex
		mvertex_t* pvertex = nullptr;
		Int32 edgeindex = g_pOriginalBSPSurfEdges[psurf->firstedge+i];
		if(edgeindex >= 0)
			pvertex = &g_pOriginalBSPVertexes[g_pOriginalBSPEdges[edgeindex].vertexes[0]];
		else
			pvertex = &g_pOriginalBSPVertexes[g_pOriginalBSPEdges[-edgeindex].vertexes[1]];

		// Set mins/maxs coords
		for(Uint32 j = 0; j < 3; j++)
		{
			if(pvertex->origin[j] < vmins[j])
				vmins[j] = pvertex->origin[j];

			if(pvertex->origin[j] > vmaxs[j])
				vmaxs[j] = pvertex->origin[j];
		}

		// Calculate surface extents
		for(Uint32 j = 0; j < 2; j++)
		{
			Float val = pvertex->origin[0] * static_cast<Double>(ptexinfo->vecs[j][0]) 
					+ pvertex->origin[1] * static_cast<Double>(ptexinfo->vecs[j][1])
					+ pvertex->origin[2] * static_cast<Double>(ptexinfo->vecs[j][2])
					+ static_cast<Double>(ptexinfo->vecs[j][3]);

			if(val < mins[j])
				mins[j] = val;

			if(val > maxs[j])
				maxs[j] = val;
		}
	}

	// Set mins/maxs
	psurf->mins = vmins;
	psurf->maxs = vmaxs;

	for(Uint32 i = 0; i < 2; i++)
	{
		Int32 boundsmin = static_cast<Int32>(SDL_floor(mins[i]/sampleSize));
		Int32 boundsmax = static_cast<Int32>(SDL_ceil(maxs[i]/sampleSize));

		psurf->texturemins[i] = boundsmin*sampleSize;
		psurf->extents[i] = (boundsmax - boundsmin) * sampleSize;

		if(!(ptexinfo->flags & TEXFLAG_SPECIAL) && psurf->extents[i] > MAX_SURFACE_EXTENTS)
		{
			printf("%s: Bad surface extents.\n", __FUNCTION__);
			return false;
		}
	}

	return true;
}

//=============================================
//
//=============================================
Int32 BSP_StyleIndex ( const msurface_t *psurface, Uint32 style )
{
	for (Uint32 j = 0 ; j < MAX_SURFACE_STYLES && psurface->styles[j] != 255 ; j++)
	{
		if (psurface->styles[j] == style)
			return j;
	}

	return NO_POSITION;
}

//===============================================
// 
//
//===============================================
bool BSP_ConvertFaces( const byte* psrcfile, const dpbspv1lump_t& srclump, CBuffer& dbspbuffer, dpbspv2header_t*& dstheader, Uint32& fileoffset, bool& hasBumpData )
{
	// Check if sizes are correct
	if(srclump.size % sizeof(dpbspv1face_t))
	{
		printf("%s - Inconsistent lump size in.\n", __FUNCTION__);
		return false;
	}

	// Load the data in
	Uint32 count = srclump.size/sizeof(dpbspv1face_t);
	const dpbspv1face_t* pinfaces = reinterpret_cast<const dpbspv1face_t*>(psrcfile + srclump.offset);
	msurface_t* poutsurfaces = new msurface_t[count];

	// If we need to re-organize lighting data
	byte* plightdata[NB_SURF_LIGHTMAP_LAYERS] = { nullptr };
	Uint32 lightdatasize = 0;

	// Allocate and initialize output data
	dpbspv2face_t* poutfaces = new dpbspv2face_t[count];

	for(Uint32 i = 0; i < count; i++)
	{
		Uint32 j = 0;
		for(; j < MAX_SURFACE_STYLES; j++)
		{
			if(pinfaces[i].lmstyles[j] == PBSPV1_LM_AMBIENT_STYLE
				|| pinfaces[i].lmstyles[j] == PBSPV1_LM_DIFFUSE_STYLE
				|| pinfaces[i].lmstyles[j] == PBSPV1_LM_LIGHTVECS_STYLE)
			{
				break;
			}
		}

		if(j != MAX_SURFACE_STYLES)
		{
			hasBumpData = true;
			break;
		}
	}

	// If we found bump data, allocate the lightmaps
	if(hasBumpData)
	{
		for(Uint32 j = 0; j < NB_SURF_LIGHTMAP_LAYERS; j++)
		{
			plightdata[j] = new byte[g_originalBSPLightDataSize];
			memset(plightdata[j], 0, sizeof(g_originalBSPLightDataSize));
		}
	}

	// Check if we have any bump map data
	for(Uint32 i = 0; i < count; i++)
	{
		msurface_t* pout = &poutsurfaces[i];

		pout->firstedge = pinfaces[i].firstedge;
		pout->numedges = Common::ByteToInt32(reinterpret_cast<const byte *>(&pinfaces[i].numedges));
		pout->flags = 0;

		Uint32 planeindex = Common::ByteToUint32(reinterpret_cast<const byte*>(&pinfaces[i].planenum));
		Int32 side = Common::ByteToInt32(reinterpret_cast<const byte*>(&pinfaces[i].side));
		if(side)
			pout->flags |= SURF_PLANEBACK;
		
		Int32 texinfoindex = Common::ByteToInt32(reinterpret_cast<const byte*>(&pinfaces[i].texinfo));
		pout->ptexinfo = &g_pOriginalBSPTexInfos[texinfoindex];

		if(!BSP_CalcSurfaceExtents(pout, PBSPV1_LM_SAMPLE_SIZE))
		{
			delete [] poutsurfaces;
			delete [] poutfaces;
			return false;
		}

		for(Uint32 j = 0; j < MAX_SURFACE_STYLES; j++)
			pout->styles[j] = pinfaces[i].lmstyles[j];
		
		if(pinfaces[i].lightoffset != -1)
		{
			Int32 ambientIndex = BSP_StyleIndex(pout, PBSPV1_LM_AMBIENT_STYLE);
			Int32 diffuseIndex = BSP_StyleIndex(pout, PBSPV1_LM_DIFFUSE_STYLE);
			Int32 vectorsIndex = BSP_StyleIndex(pout, PBSPV1_LM_LIGHTVECS_STYLE);
			
			// Set up indexes
			Int32 layerIndexes[NB_SURF_LIGHTMAP_LAYERS] = { 
				0,				// SURF_LIGHTMAP_DEFAULT
				vectorsIndex,	 // SURF_LIGHTMAP_VECTORS
				ambientIndex,	 // SURF_LIGHTMAP_AMBIENT
				diffuseIndex,	 // SURF_LIGHTMAP_DIFFUSE
			};

			// Manage reorganization of lighting data into what the engine needs
			if(hasBumpData && ambientIndex != NO_POSITION && diffuseIndex != NO_POSITION && vectorsIndex != NO_POSITION)
			{
				// Calculate extents
				Uint32 xsize = (pout->extents[0]>>4)+1;
				Uint32 ysize = (pout->extents[1]>>4)+1;
				Uint32 size = xsize*ysize;

				for(Uint32 j = 0; j < NB_SURF_LIGHTMAP_LAYERS; j++)
				{
					// Set final data offset and copy the data
					byte* pdestination = plightdata[j] + lightdatasize;
					color24_t* psrclightdata = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(g_pOriginalBSPLighting) + pinfaces[i].lightoffset);
					
					psrclightdata += size*layerIndexes[j];
					memcpy(pdestination, psrclightdata, sizeof(color24_t)*size);
				}

				// Set light data offset and increase size
				pout->lightoffset = lightdatasize;
				lightdatasize += size*sizeof(color24_t);

				ALD3_Surface_Convert(pout, pinfaces[i].lightoffset, pout->lightoffset, layerIndexes, true);
			}
			else
			{
				// We only have the base layer
				pout->psamples[SURF_LIGHTMAP_DEFAULT] = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(g_pOriginalBSPLighting) + pinfaces[i].lightoffset);
				pout->lightoffset = pinfaces[i].lightoffset;

				ALD3_Surface_Convert(pout, pinfaces[i].lightoffset, pout->lightoffset, layerIndexes, false);
			}

			// If any of these were set, clear the styles entirely past position 0
			if(ambientIndex != NO_POSITION || diffuseIndex != NO_POSITION || vectorsIndex != NO_POSITION)
			{
				for(Uint32 j = 1; j < MAX_SURFACE_STYLES; j++)
					pout->styles[j] = 255;
			}
		}
		else
		{
			// No light data at all
			pout->lightoffset = -1;
		}

		// Flag sky surfaces
		if(!qstrncmp(pout->ptexinfo->ptexture->name.c_str(), "sky", 3))
			pout->flags |= SURF_DRAWSKY;

		// Set final data in BSPV2
		dpbspv2face_t* poutface = &poutfaces[i];
		poutface->firstedge = pinfaces[i].firstedge;
		poutface->lightoffset = pout->lightoffset;
		poutface->numedges = pinfaces[i].numedges;
		poutface->planenum = pinfaces[i].planenum;
		poutface->texinfo = pinfaces[i].texinfo;
		poutface->side = pinfaces[i].side;

		poutface->samplescale = 1; // Default for now
		poutface->smoothgroupbits = 0; // No support for smoothing groups

		Uint32 j = 0;
		for(; j < PBSPV1_MAX_LIGHTMAPS; j++)
			poutface->lmstyles[j] = pout->styles[j];

		for(; j < PBSPV2_MAX_LIGHTMAPS; j++)
			poutface->lmstyles[j] = 255;
	}

	// Save face data to output BSP
	dpbspv2lump_t& faceslump = dstheader->lumps[PBSPV2_LUMP_FACES];
	faceslump.offset = fileoffset;
	Uint32 facedatasize = sizeof(dpbspv2face_t)*count;
	faceslump.size = facedatasize;
	fileoffset += facedatasize;

	dbspbuffer.append(poutfaces, facedatasize);

	// Re-organize light data if needed
	if(hasBumpData)
	{
		// Set up indexes
		Int32 lumpIndexes[NB_SURF_LIGHTMAP_LAYERS] = { 
			PBSPV2_LUMP_LIGHTING_DEFAULT,	// SURF_LIGHTMAP_DEFAULT
			PBSPV2_LUMP_LIGHTING_VECTORS,	// SURF_LIGHTMAP_VECTORS
			PBSPV2_LUMP_LIGHTING_AMBIENT,	// SURF_LIGHTMAP_AMBIENT
			PBSPV2_LUMP_LIGHTING_DIFFUSE,	// SURF_LIGHTMAP_DIFFUSE
		};

		// Write the final data to the respective lumps
		for(Uint32 i = 0; i < NB_SURF_LIGHTMAP_LAYERS; i++)
		{
			// Set offset into lump
			dpbspv2lump_t& destlump = dstheader->lumps[lumpIndexes[i]];
			destlump.offset = fileoffset;
			destlump.size = lightdatasize;
			fileoffset += lightdatasize;

			// Append to the buffer
			dbspbuffer.append(plightdata[i], sizeof(byte)*lightdatasize);
			delete[] plightdata[i];
		}
	}

	g_finalBSPLightDataSize = lightdatasize;

	delete [] poutsurfaces;
	delete [] poutfaces;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool BSP_LoadEdges( const byte* pfile, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv1edge_t))
	{
		printf("%s - Inconsistent lump size.\n", __FUNCTION__);
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv1edge_t);
	const dpbspv1edge_t* pinedges = reinterpret_cast<const dpbspv1edge_t*>(pfile + lump.offset);
	medge_t* poutedges = new medge_t[count];

	g_pOriginalBSPEdges = poutedges;

	for(Uint32 i = 0; i < count; i++)
	{
		poutedges[i].vertexes[0] = Common::ByteToUint32(reinterpret_cast<const byte*>(&pinedges[i].vertexes[0]));
		poutedges[i].vertexes[1] = Common::ByteToUint32(reinterpret_cast<const byte*>(&pinedges[i].vertexes[1]));
	}

	return true;
}


//=============================================
// @brief
//
//=============================================
bool BSP_LoadSurfEdges( const byte* pfile, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(Int32))
	{
		printf("%s - Inconsistent lump size.\n", __FUNCTION__);
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(Int32);
	const Int32* pinedges = reinterpret_cast<const Int32*>(pfile + lump.offset);
	Int32* poutedges = new Int32[count];

	g_pOriginalBSPSurfEdges = poutedges;

	memcpy(poutedges, pinedges, sizeof(Int32)*count);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSP_LoadTexinfos( const byte* pfile, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv1texinfo_t))
	{
		printf("%s - Inconsistent lump size.\n", __FUNCTION__);
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv1texinfo_t);
	const dpbspv1texinfo_t* pintexinfos = reinterpret_cast<const dpbspv1texinfo_t*>(pfile + lump.offset);
	mtexinfo_t* pouttexinfos = new mtexinfo_t[count];
	
	g_pOriginalBSPTexInfos = pouttexinfos;

	for(Uint32 i = 0; i < count; i++)
	{
		// Copy the alignment info
		memcpy(pouttexinfos[i].vecs, pintexinfos[i].vecs, sizeof(Float)*8);

		pouttexinfos[i].flags = pintexinfos[i].flags;
		Int32 textureindex = pintexinfos[i].miptex;

		if(textureindex >= static_cast<Int32>(g_numOriginalBSPTextures))
		{
			printf("Invalid texture index '%d'.\n", textureindex);
			continue;
		}

		pouttexinfos[i].ptexture = &g_pOriginalBSPTextures[textureindex];
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool BSP_LoadLighting( const byte* pfile, const dpbspv1lump_t& lump )
{
	if(!lump.size)
		return true;

	// Check if sizes are correct
	if(lump.size % sizeof(color24_t))
	{
		printf("%s - Inconsistent lump size.\n", __FUNCTION__);
		return false;
	}

	g_pOriginalBSPLighting = reinterpret_cast<byte *>(new byte[lump.size]);
	g_originalBSPLightDataSize = lump.size;

	const byte *psrc = (pfile + lump.offset);
	memcpy(g_pOriginalBSPLighting, psrc, sizeof(byte)*lump.size);

	return true;
}

//=============================================
// @brief
//
//=============================================
bool BSP_LoadTextures( const byte* pfile, const dpbspv1lump_t& lump )
{
	if(!lump.size)
	{
		printf("%s - No textures present.\n", __FUNCTION__);
		return true;
	}

	// Get texture counts
	const dmiptexlump_t* pmiptexlump = reinterpret_cast<const dmiptexlump_t*>(pfile + lump.offset);

	g_numOriginalBSPTextures = pmiptexlump->nummiptex;
	g_pOriginalBSPTextures = new mtexture_t[g_numOriginalBSPTextures];

	for(Uint32 i = 0; i < g_numOriginalBSPTextures; i++)
	{
		// Get pointer to miptex data
		const dmiptex_t* pmiptex = reinterpret_cast<const dmiptex_t*>(reinterpret_cast<const byte*>(pmiptexlump) + pmiptexlump->dataoffsets[i]);

		// We only get the name and width/height here
		mtexture_t* ptexture = &g_pOriginalBSPTextures[i];
		ptexture->name = pmiptex->name;
		ptexture->width = pmiptex->width;
		ptexture->height = pmiptex->height;
	}
	
	return true;
}

//=============================================
// @brief
//
//=============================================
bool BSP_LoadVertexes( const byte* pfile, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv1vertex_t))
	{
		printf("%s - Inconsistent lump size.\n", __FUNCTION__);
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv1vertex_t);
	const dpbspv1vertex_t* pinverts = reinterpret_cast<const dpbspv1vertex_t*>(pfile + lump.offset);
	mvertex_t* poutverts = new mvertex_t[count];

	g_pOriginalBSPVertexes = poutverts;

	for(Uint32 i = 0; i < count; i++)
		Math::VectorCopy(pinverts[i].origin, poutverts[i].origin);

	return true;
}

//===============================================
// 
//
//===============================================
void BSP_FreeConversionData( void )
{
	if(g_pOriginalBSPLighting)
	{
		delete[] g_pOriginalBSPLighting;
		g_pOriginalBSPLighting = nullptr;
	}

	if(g_pOriginalBSPSurfEdges)
	{
		delete[] g_pOriginalBSPSurfEdges;
		g_pOriginalBSPSurfEdges = nullptr;
	}

	if(g_pOriginalBSPEdges)
	{
		delete[] g_pOriginalBSPEdges;
		g_pOriginalBSPEdges = nullptr;
	}

	if(g_pOriginalBSPVertexes)
	{
		delete[] g_pOriginalBSPVertexes;
		g_pOriginalBSPVertexes = nullptr;
	}

	if(g_pOriginalBSPTextures)
	{
		delete[] g_pOriginalBSPTextures;
		g_pOriginalBSPTextures = nullptr;
	}

	if(g_pOriginalBSPTexInfos)
	{
		delete[] g_pOriginalBSPTexInfos;
		g_pOriginalBSPTexInfos = nullptr;
	}

	g_originalBSPLightDataSize = 0;
	g_numOriginalBSPTextures = 0;
	g_finalBSPLightDataSize = 0;

	// Free any data related to ALDs
	if(!g_pALDConversionData.empty())
		g_pALDConversionData.clear();

	g_pALDHeaderFlags = 0;
}

//===============================================
// 
//
//===============================================
bool BSP_ConvertBSPFile( const Char* pstrFilePath, const Char* pstrOutputPath )
{
	const byte* pdata = LoadFile(pstrFilePath, nullptr);
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
	if(headerVersion != PBSPV1_VERSION)
	{
		printf("%s - '%s' is not a version 1 Pathos BSP file(it is a version %d instead).\n", __FUNCTION__, pstrFilePath, headerVersion);
		delete[] pdata;
		return false;
	}

	// Create output buffer
	Uint32 initialSize = sizeof(dpbspv2header_t) + sizeof(dpbspv2lump_t)*PBSPV2_NB_LUMPS;
	Uint32 fileOffset = initialSize;
	CBuffer bspFileBuffer(nullptr, initialSize);

	const dpbspv1header_t* psrcheader = reinterpret_cast<const dpbspv1header_t*>(pdata);
	if(!psrcheader->lumps[PBSPV1_LUMP_LIGHTING].size)
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

	for(Uint32 i = 0; i < PBSPV1_NB_LUMPS; i++)
	{
		Int32 dstLumpIndex;
		switch(i)
		{
		case PBSPV1_LUMP_ENTITIES:
			dstLumpIndex = PBSPV2_LUMP_ENTITIES;
			break;
		case PBSPV1_LUMP_PLANES:
			dstLumpIndex = PBSPV2_LUMP_PLANES;
			break;
		case PBSPV1_LUMP_TEXTURES:
			dstLumpIndex = PBSPV2_LUMP_TEXTURES;
			break;
		case PBSPV1_LUMP_VERTEXES:
			dstLumpIndex = PBSPV2_LUMP_VERTEXES;
			break;
		case PBSPV1_LUMP_VISIBILITY:
			dstLumpIndex = PBSPV2_LUMP_VISIBILITY;
			break;
		case PBSPV1_LUMP_NODES:
			dstLumpIndex = PBSPV2_LUMP_NODES;
			break;
		case PBSPV1_LUMP_TEXINFO:
			dstLumpIndex = PBSPV2_LUMP_TEXINFO;
			break;
		case PBSPV1_LUMP_CLIPNODES:
			dstLumpIndex = PBSPV2_LUMP_CLIPNODES;
			break;
		case PBSPV1_LUMP_LEAFS:
			dstLumpIndex = PBSPV2_LUMP_LEAFS;
			break;
		case PBSPV1_LUMP_MARKSURFACES:
			dstLumpIndex = PBSPV2_LUMP_MARKSURFACES;
			break;
		case PBSPV1_LUMP_EDGES:
			dstLumpIndex = PBSPV2_LUMP_EDGES;
			break;
		case PBSPV1_LUMP_SURFEDGES:
			dstLumpIndex = PBSPV2_LUMP_SURFEDGES;
			break;
		case PBSPV1_LUMP_MODELS:
			dstLumpIndex = PBSPV2_LUMP_MODELS;
			break;
		default:
		case PBSPV1_LUMP_FACES:
		case PBSPV1_LUMP_LIGHTING:
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

	// Load vertexes
	if(!BSP_LoadVertexes(pdata, psrcheader->lumps[PBSPV1_LUMP_VERTEXES]))
	{
		printf("%s - Failed to load lump PBSPV1_LUMP_VERTEXES.\n", __FUNCTION__);
		BSP_FreeConversionData();
		delete[] pdata;
		return false;
	}

	// Load edges
	if(!BSP_LoadEdges(pdata, psrcheader->lumps[PBSPV1_LUMP_EDGES]))
	{
		printf("%s - Failed to load lump PBSPV1_LUMP_EDGES.\n", __FUNCTION__);
		BSP_FreeConversionData();
		delete[] pdata;
		return false;
	}

	// Load surfedges
	if(!BSP_LoadSurfEdges(pdata, psrcheader->lumps[PBSPV1_LUMP_SURFEDGES]))
	{
		printf("%s - Failed to load lump PBSPV1_LUMP_SURFEDGES.\n", __FUNCTION__);
		BSP_FreeConversionData();
		delete[] pdata;
		return false;
	}

	// Load textures
	if(!BSP_LoadTextures(pdata, psrcheader->lumps[PBSPV1_LUMP_TEXTURES]))
	{
		printf("%s - Failed to load lump PBSPV1_LUMP_TEXTURES.\n", __FUNCTION__);
		BSP_FreeConversionData();
		delete[] pdata;
		return false;
	}

	// Load texinfos
	if(!PBSP_LoadTexinfos(pdata, psrcheader->lumps[PBSPV1_LUMP_TEXINFO]))
	{
		printf("%s - Failed to load lump PBSPV1_LUMP_TEXINFO.\n", __FUNCTION__);
		BSP_FreeConversionData();
		delete[] pdata;
		return false;
	}

	// Load lighting data from the BSP file
	if(!BSP_LoadLighting(pdata, psrcheader->lumps[PBSPV1_LUMP_LIGHTING]))
	{
		printf("%s - Failed to load lump PBSPV1_LUMP_LIGHTING.\n", __FUNCTION__);
		BSP_FreeConversionData();
		delete[] pdata;
		return false;
	}

	// Check if we have an ALD file
	CString aldFilepath(pstrFilePath);
	aldFilepath.erase(aldFilepath.find(0, ".bsp"), 4);
	aldFilepath << ".ald";

	if(FileExists(aldFilepath.c_str()))
	{
		// ALD exists, so load it
		ALD3_LoadALDFile(aldFilepath.c_str());
	}

	// Now load and process the faces
	bool hasBumpData = false;
	if(!BSP_ConvertFaces(pdata, psrcheader->lumps[PBSPV1_LUMP_FACES], bspFileBuffer, pdstheader, fileOffset, hasBumpData))
	{
		printf("%s - Failed to convert lump PBSPV1_LUMP_FACES.\n", __FUNCTION__);
		BSP_FreeConversionData();
		delete[] pdata;
		return false;
	}

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

	if(result)
		printf("Completed processing file '%s'.\n", pstrFilePath);

	ALD3_Convert_Finish(pstrFilePath, pstrOutputPath, hasBumpData);

	// Free any data we used
	BSP_FreeConversionData();

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
		printf("Pathos BSP conversion tool, converts Pathos Version 1 BSP to Pathos BSP Version 2.\n");
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
