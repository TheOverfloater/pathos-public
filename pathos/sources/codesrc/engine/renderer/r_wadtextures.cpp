/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "r_wadtextures.h"
#include "wad3file.h"
#include "file.h"
#include "system.h"
#include "entitydata.h"
#include "texturemanager.h"
#include "bspv30file.h"
#include "brushmodel.h"
#include "miptex.h"
#include "pbspv1file.h"
#include "pbspv2file.h"
#include "bspv30.h"
#include "bsp_shared.h"
#include "enginestate.h"

// Path to legacy texture material type associations
const Char CWADTextureResource::TEXTURE_MATERIAL_ASSOCIATION_FILE_PATH[] = "scripts/legacy/materials.txt";

//=============================================
// @brief Constructor
//
//=============================================
CWADTextureResource::CWADTextureResource( void ):
	m_pBSPFile(nullptr)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CWADTextureResource::~CWADTextureResource( void )
{
	if(!m_detailTexturesArray.empty())
	{
		for(Uint32 i = 0; i < m_detailTexturesArray.size(); i++)
			delete m_detailTexturesArray[i];

		m_detailTexturesArray.clear();
	}

	if(!m_detailTextureAssociationArray.empty())
	{
		for(Uint32 i = 0; i < m_detailTextureAssociationArray.size(); i++)
			delete m_detailTextureAssociationArray[i];

		m_detailTextureAssociationArray.clear();
	}

	if(!m_materialAssociationArray.empty())
	{
		for(Uint32 i = 0; i < m_materialAssociationArray.size(); i++)
			delete m_materialAssociationArray[i];

		m_materialAssociationArray.clear();
	}

	if(!m_materialCodeTypesAssociations.empty())
	{
		for(Uint32 i = 0; i < m_materialCodeTypesAssociations.size(); i++)
			delete m_materialCodeTypesAssociations[i];

		m_materialCodeTypesAssociations.clear();
	}

	if(!m_pWADFilesArray.empty())
	{
		for(Uint32 i = 0; i < m_pWADFilesArray.size(); i++)
		{
			FL_FreeFile(m_pWADFilesArray[i].pwadfile);
		}

		m_pWADFilesArray.clear();
	}

	if(m_pBSPFile)
	{
		delete m_pBSPFile;
		m_pBSPFile = nullptr;
	}
}

//=============================================
// @brief Destructor
//
//=============================================
bool CWADTextureResource::Init( const Char* pstrBSPName, const CArray<CString>& wadFilesList, bool generateMissingWAD, bool generateMissingBSP )
{
	Uint32 filesize = 0;
	const byte* pfile = FL_LoadFile(pstrBSPName, &filesize);
	if(!pfile)
	{
		Con_Printf("%s - Failed to load '%s'.\n", __FUNCTION__, pstrBSPName);
		return false;
	}

	// TRUE if we need to load detail textures
	// and material associations from legacy stuff
	bool generateBSPPMFFiles = false;
	bool generateWADPMFFiles = false;

	// Create a copy of the BSP data
	m_pBSPFile = new byte[filesize];
	memcpy(m_pBSPFile, pfile, sizeof(byte)*filesize);
	FL_FreeFile(pfile);

	// Save BSP file basename
	Common::Basename(pstrBSPName, m_BSPFileName);
	m_BSPFileName << ".bsp";

	// Now check WAD files if any
	for(Uint32 i = 0; i < wadFilesList.size(); i++)
	{
		// Get basename
		static Char basename[MAX_PARSE_LENGTH];
		Common::Basename(wadFilesList[i].c_str(), basename);

		// Build wad path
		CString wadpath;
		wadpath << WORLD_TEXTURES_BASE_PATH << basename << PATH_SLASH_CHAR << wadFilesList[i];

		// Load the WAD file in
		filesize = 0;
		pfile = FL_LoadFile(wadpath.c_str(), &filesize);
		if(!pfile)
		{
			Con_Printf("%s - Wad file '%s' is missing.\n", __FUNCTION__, wadpath.c_str());
			continue;
		}

		const wad3info_t* pwadinfo = reinterpret_cast<const wad3info_t*>(pfile);
		if(qstrncmp(pwadinfo->identification, "WAD3", 4))
		{
			Con_Printf("%s - WAD file '%s' is not of valid type.\n", __FUNCTION__, wadpath.c_str());
			continue;
		}

		wad3file_t wad;
		wad.pwadfile = pfile;
		qstrcpy(wad.wadfilename, wadFilesList[i].c_str());

		m_pWADFilesArray.push_back(wad);
	}

	// Only generate new ones if set
	if(!generateMissingWAD && !generateMissingBSP)
		return true;

	const dmiptexlump_t* ptexturelump = nullptr;
	if (generateMissingBSP)
	{
		// Now that all WAD files are loaded, check if the world has data for it
		Int32 fileHeaderId = Common::ByteToInt32(m_pBSPFile);
		if(fileHeaderId == PBSP_HEADER)
		{
			// Now get the version
			Int32 fileHeaderVersion = Common::ByteToInt32(m_pBSPFile + sizeof(Int32));
			switch(fileHeaderVersion)
			{
			case PBSPV1_VERSION:
				{
					const dpbspv1header_t* pbspheader = reinterpret_cast<const dpbspv1header_t*>(m_pBSPFile);
					ptexturelump = reinterpret_cast<const dmiptexlump_t*>(m_pBSPFile + pbspheader->lumps[PBSPV1_LUMP_TEXTURES].offset);
				}
				break;
			case PBSPV2_VERSION:
				{
					const dpbspv2header_t* pbspheader = reinterpret_cast<const dpbspv2header_t*>(m_pBSPFile);
					ptexturelump = reinterpret_cast<const dmiptexlump_t*>(m_pBSPFile + pbspheader->lumps[PBSPV2_LUMP_TEXTURES].offset);
				}
				break;
			default:
				Con_EPrintf("%s - PBSP file '%s' has an unknown version number '%d'.\n", __FUNCTION__, ens.pworld->name.c_str(), fileHeaderVersion);
				return false;
				break;
			}
		}
		else
		{
			// Get the version from the file and determine if it's usable
			Int32 bspVersion = Common::ByteToInt32(m_pBSPFile);
			switch(bspVersion)
			{
			case BSPV30_VERSION:
				{
					const dv30header_t* pheaderv30bsp = reinterpret_cast<const dv30header_t*>(m_pBSPFile);
					ptexturelump = reinterpret_cast<const dmiptexlump_t*>(m_pBSPFile + pheaderv30bsp->lumps[V30_LUMP_TEXTURES].offset);
				}
				break;
			default:
				Con_EPrintf("%s - BSP file '%s' has wrong version number '%d', which should be '%d'.\n", __FUNCTION__, ens.pworld->name.c_str(), bspVersion, BSPV30_VERSION);
				return false;
				break;
			}
		}

		for (Int32 i = 0; i < ptexturelump->nummiptex; i++)
		{
			if (ptexturelump->dataoffsets[i] == -1)
				continue;

			const dmiptex_t* pmiptex = reinterpret_cast<const dmiptex_t*>(reinterpret_cast<const byte*>(ptexturelump) + ptexturelump->dataoffsets[i]);
			if (!pmiptex->offsets[0])
				continue;

			if (!IsMaterialScriptPresent(pmiptex->name, pstrBSPName))
			{
				generateBSPPMFFiles = true;
				break;
			}
		}
	}

	// Check WADs too
	if (generateMissingWAD)
	{
		for (Uint32 i = 0; i < m_pWADFilesArray.size(); i++)
		{
			const wad3info_t* pwadinfo = reinterpret_cast<const wad3info_t*>(m_pWADFilesArray[i].pwadfile);
			const wad3lumpinfo_t* plumps = reinterpret_cast<const wad3lumpinfo_t*>(reinterpret_cast<const byte*>(pwadinfo) + pwadinfo->infotableofs);

			for (Int32 j = 0; j < pwadinfo->numlumps; j++)
			{
				const wad3lumpinfo_t* plump = &plumps[j];
				if (plump->type != 0 && !(plump->type & 0x43))
					continue;

				CString wadbasename;
				Common::Basename(m_pWADFilesArray[i].wadfilename, wadbasename);
				wadbasename << WAD_FILE_EXTENSION;

				if (!IsMaterialScriptPresent(plump->name, wadbasename.c_str()))
				{
					m_pWADFilesArray[i].hasmissing = true;
					generateWADPMFFiles = true;
					break;
				}
			}
		}
	}

	// Generate PMF files if needed
	if(generateBSPPMFFiles || generateWADPMFFiles)
	{
		// Load detail textures
		if(!WAD_LoadDetailTextureAssociations(m_detailTexturesArray, m_detailTextureAssociationArray))
			return false;

		// Load material codes
		if(!LoadMaterialTypeCodeAssociations())
			return false;

		// Load material associations
		if(!LoadMaterialTextureAssociations())
			return false;

		// Find all missing entries and create PMF files for them for BSP
		if (generateBSPPMFFiles && generateMissingBSP)
		{
			for (Int32 i = 0; i < ptexturelump->nummiptex; i++)
			{
				if (ptexturelump->dataoffsets[i] == -1)
					continue;

				const dmiptex_t* pmiptex = reinterpret_cast<const dmiptex_t*>(reinterpret_cast<const byte*>(ptexturelump) + ptexturelump->dataoffsets[i]);
				if (!pmiptex->offsets[0])
					continue;

				if (!IsMaterialScriptPresent(pmiptex->name, m_BSPFileName.c_str()))
					CreateMaterialScript(pmiptex, m_BSPFileName.c_str());
			}
		}
		

		// Find also in the WAD files
		if(generateWADPMFFiles && generateMissingWAD && !m_pWADFilesArray.empty())
		{
			for(Uint32 i = 0; i < m_pWADFilesArray.size(); i++)
			{
				if(!m_pWADFilesArray[i].hasmissing)
					continue;

				const wad3info_t* pwadinfo = reinterpret_cast<const wad3info_t*>(m_pWADFilesArray[i].pwadfile);
				const wad3lumpinfo_t* plumps = reinterpret_cast<const wad3lumpinfo_t*>(reinterpret_cast<const byte*>(pwadinfo) + pwadinfo->infotableofs);

				for(Int32 j = 0; j < pwadinfo->numlumps; j++)
				{
					const wad3lumpinfo_t* plump = &plumps[j];
					if(plump->type != 0 && !(plump->type & 0x43))
						continue;

					static Char wadbasename[MAX_PARSE_LENGTH];
					Common::Basename(m_pWADFilesArray[i].wadfilename, wadbasename);
					strcat(wadbasename, WAD_FILE_EXTENSION);

					const dmiptex_t *pmiptex = reinterpret_cast<const dmiptex_t*>(m_pWADFilesArray[i].pwadfile + plump->filepos);
					if(!IsMaterialScriptPresent(plump->name, wadbasename))
						CreateMaterialScript(pmiptex, wadbasename);
				}
			}
		}
	}

	return true;
}

//=============================================
// @brief Destructor
//
//=============================================
bool CWADTextureResource::LoadMaterialTypeCodeAssociations( void )
{
	// Load texture material type associations
	Uint32 filesize = 0;
	const byte* pfile = FL_LoadFile(MATERIAL_TYPES_FILE_PATH, &filesize);
	if(!pfile)
	{
		Con_Printf("%s - Failed to load '%s'.\n", __FUNCTION__, MATERIAL_TYPES_FILE_PATH);
		return false;
	}

	// Reset line count
	Uint32 linecount = 0;
	static Char line[MAX_LINE_LENGTH];

	// Read the contents
	const Char* pstr = reinterpret_cast<const Char*>(pfile);
	while(pstr)
	{
		linecount++;

		// Parse the line
		pstr = Common::ReadLine(pstr, line);
		if(!qstrlen(line))
			continue;

		// Read in the map texture name token
		static Char materialcode[MAX_PARSE_LENGTH];
		const Char* plinestr = Common::Parse(line, materialcode);
		if(!plinestr)
		{
			Con_Printf("%s - Unexpected end of line %d in '%s'.\n", __FUNCTION__, linecount, MATERIAL_TYPES_FILE_PATH);
			continue;
		}

		// Read in the detail texture name token
		static Char materialname[MAX_PARSE_LENGTH];
		plinestr = Common::Parse(plinestr, materialname);
		if(!qstrlen(materialname))
		{
			Con_Printf("%s - Unexpected end of line %d in '%s'.\n", __FUNCTION__, linecount, MATERIAL_TYPES_FILE_PATH);
			continue;
		}

		// Add the association
		material_type_t* pmattype = new material_type_t;
		pmattype->materialcode = materialcode[0];
		qstrcpy(pmattype->materialtype, materialname);

		m_materialCodeTypesAssociations.push_back(pmattype);
	}
	FL_FreeFile(pfile);

	return true;
}

//=============================================
// @brief Destructor
//
//=============================================
bool CWADTextureResource::LoadMaterialTextureAssociations( void )
{
	// Load texture material type associations
	Uint32 filesize = 0;
	const byte* pfile = FL_LoadFile(TEXTURE_MATERIAL_ASSOCIATION_FILE_PATH, &filesize);
	if(!pfile)
	{
		Con_Printf("%s - Failed to load '%s'.\n", __FUNCTION__, TEXTURE_MATERIAL_ASSOCIATION_FILE_PATH);
		return false;
	}

	// Reset line count
	static Char line[MAX_LINE_LENGTH];

	// Count the number of lines
	Uint32 linecount = Common::GetFileLineCount(reinterpret_cast<const Char*>(pfile));

	// For faster load
	m_materialAssociationArray.reserve(linecount);
	linecount = 0;

	// Read the contents
	const Char* pstr = reinterpret_cast<const Char*>(pfile);
	while(pstr)
	{
		linecount++;

		// Parse the line
		pstr = Common::ReadLine(pstr, line);
		if(!qstrlen(line))
			continue;

		// Read in the map texture name token
		static Char materialtype[MAX_PARSE_LENGTH];
		const Char* plinestr = Common::Parse(line, materialtype);
		if(!plinestr)
		{
			Con_Printf("%s - Unexpected end of line %d in '%s'.\n", __FUNCTION__, linecount, TEXTURE_MATERIAL_ASSOCIATION_FILE_PATH);
			continue;
		}

		// Read in the detail texture name token
		static Char maptexturename[MAX_PARSE_LENGTH];
		plinestr = Common::Parse(plinestr, maptexturename);
		if(!qstrlen(maptexturename))
		{
			Con_Printf("%s - Unexpected end of line %d in '%s'.\n", __FUNCTION__, linecount, TEXTURE_MATERIAL_ASSOCIATION_FILE_PATH);
			continue;
		}

		// Add the association
		material_association_t* pnewassoc = new material_association_t;
		qstrcpy(pnewassoc->maptexturename, maptexturename);
		Common::ConvertStringToLowerCase(pnewassoc->maptexturename);
		pnewassoc->materialtypecode = materialtype[0];

		m_materialAssociationArray.push_back(pnewassoc);
	}
	FL_FreeFile(pfile);

	// Resize the array
	m_materialAssociationArray.resize(m_materialAssociationArray.size());

	return true;
}

//=============================================
// @brief Destructor
//
//=============================================
bool CWADTextureResource::IsMaterialScriptPresent( const Char* pstrTextureName, const Char* pstrContainerName )
{
	static Char basename[MAX_PARSE_LENGTH];
	Common::Basename(pstrContainerName, basename);

	// Build file path
	static Char filepath[MAX_PARSE_LENGTH];
	sprintf(filepath, "%s%s/%s%s", WORLD_TEXTURES_BASE_PATH, basename, pstrTextureName, PMF_FORMAT_EXTENSION);

	return FL_FileExists(filepath);
}

//=============================================
// @brief Destructor
//
//=============================================
void CWADTextureResource::CreateMaterialScript( const dmiptex_t* ptexture, const Char* pstrContainerName )
{
	// Convert name to lowercase
	CString texturename(ptexture->name);
	texturename.tolower();

	CString data;
	data << "$texture" << NEWLINE;
	data << "{" << NEWLINE;

	// Check if we have detail textures for this
	detail_association_t* pdetailassoc = nullptr;
	for(Uint32 i = 0; i < m_detailTextureAssociationArray.size(); i++)
	{
		if(!qstrcmp(m_detailTextureAssociationArray[i]->maptexturename, texturename))
		{
			pdetailassoc = m_detailTextureAssociationArray[i];
			break;
		}
	}

	// Set internal size
	data << "\t$int_width " << static_cast<Int32>(ptexture->width) << NEWLINE;
	data << "\t$int_height " << static_cast<Int32>(ptexture->height) << NEWLINE;

	if(pdetailassoc)
	{
		detailtexture_t* pdetail = m_detailTexturesArray[pdetailassoc->detailtextureidx];
		Float detailxscale = (static_cast<Float>(ptexture->width)/256.0)*(128.0/(static_cast<float>(pdetail->width))*12.0);
		Float detailyscale = (static_cast<Float>(ptexture->height)/256.0)*(128.0/(static_cast<float>(pdetail->height))*12.0);

		data << "\t$dt_scalex " << detailxscale << NEWLINE;
		data << "\t$dt_scaley " << detailyscale << NEWLINE;
	}

	if(ptexture->name[0] == '{')
		data << "\t$alphatest" << NEWLINE;

	// Add entry for container
	data << "\t$container " << pstrContainerName << NEWLINE;

	// Remove special chars for material lookup
	CString texturematname(texturename);
	if(texturename[0] == '-' || texturename[0] == '+')
		texturematname.erase(0, 2);
	else if(texturename[0] == '{' || texturename[0] == '!' || texturename[0] == '~' || texturename[0] == ' ')
		texturematname.erase(0, 1);

	// Find material type
	CString materialtype = "concrete";
	for(Uint32 i = 0; i < m_materialAssociationArray.size(); i++)
	{
		if(!qstrcmp(m_materialAssociationArray[i]->maptexturename, texturematname))
		{
			material_association_t& matassoc = *m_materialAssociationArray[i];

			// Find the material type based on code
			for(Uint32 j = 0; j < m_materialCodeTypesAssociations.size(); j++)
			{
				if(m_materialCodeTypesAssociations[j]->materialcode == matassoc.materialtypecode)
				{
					materialtype = m_materialCodeTypesAssociations[j]->materialtype;
					break;
				}
			}

			// Handle error
			if(materialtype.empty())
			{
				Con_Printf("%s - Unknown material type code '%c' specified for texture '%s'.\n", __FUNCTION__, matassoc.materialtypecode, ptexture->name);
				materialtype = "concrete";
			}

			break;
		}
	}

	// Set material type and diffuse texture name
	data << "\t$material " << materialtype << NEWLINE;
	data << "\t$texture diffuse " << texturename << NEWLINE;
	
	// Add detail texture if present
	if(pdetailassoc)
	{
		detailtexture_t* pdetail = m_detailTexturesArray[pdetailassoc->detailtextureidx];
		data << "\t$texture detail " << pdetail->filename << NEWLINE;
	}

	// Set end bracket
	data << "}" << NEWLINE;

	// Build folder path
	CString folderpath = WAD_GetWADFolderPath(pstrContainerName, WORLD_TEXTURES_BASE_PATH);

	// Make sure folder exists
	if(!FL_CreateDirectory(folderpath.c_str()))
	{
		Con_Printf("%s - Failed to create directory '%s'.\n", __FUNCTION__, folderpath.c_str());
		return;
	}

	// Create the PMF file
	CString filepath = WAD_GetWADTexturePath(folderpath.c_str(), ptexture->name);

	Con_VPrintf("Generating material script '%s'.\n", filepath.c_str());

	const byte* pwritedata = reinterpret_cast<const byte*>(data.c_str());
	if(!FL_WriteFile(pwritedata, data.length(), filepath.c_str()))
		Con_Printf("%s - Failed to write '%s'.\n", __FUNCTION__, filepath.c_str());
}

//=============================================
// @brief Destructor
//
//=============================================
en_texture_t* CWADTextureResource::GetWADTexture( en_material_t* pmaterial, const Char* pstrContainerName, const Char* pstrTextureName )
{
	CString bsptexturename(pstrTextureName);
	bsptexturename.tolower();

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Find the texture in the given container
	if(!qstrcmp(m_BSPFileName, pstrContainerName))
	{
		// Now that all WAD files are loaded, check if the world has data for it
		const dmiptexlump_t* ptexturelump = nullptr;
		// Now that all WAD files are loaded, check if the world has data for it
		Int32 fileHeaderId = Common::ByteToInt32(m_pBSPFile);
		if(fileHeaderId == PBSP_HEADER)
		{
			// Now get the version
			Int32 fileHeaderVersion = Common::ByteToInt32(m_pBSPFile + sizeof(Int32));
			switch(fileHeaderVersion)
			{
			case PBSPV1_VERSION:
				{
					const dpbspv1header_t* pbspheader = reinterpret_cast<const dpbspv1header_t*>(m_pBSPFile);
					ptexturelump = reinterpret_cast<const dmiptexlump_t*>(m_pBSPFile + pbspheader->lumps[PBSPV1_LUMP_TEXTURES].offset);
				}
				break;
			case PBSPV2_VERSION:
				{
					const dpbspv2header_t* pbspheader = reinterpret_cast<const dpbspv2header_t*>(m_pBSPFile);
					ptexturelump = reinterpret_cast<const dmiptexlump_t*>(m_pBSPFile + pbspheader->lumps[PBSPV2_LUMP_TEXTURES].offset);
				}
				break;
			default:
				Con_EPrintf("%s - PBSP file '%s' has an unknown version number '%d'.\n", __FUNCTION__, ens.pworld->name.c_str(), fileHeaderVersion);
				return nullptr;
				break;
			}
		}
		else
		{
			// Get the version from the file and determine if it's usable
			Int32 bspVersion = Common::ByteToInt32(m_pBSPFile);
			switch(bspVersion)
			{
			case BSPV30_VERSION:
				{
					const dv30header_t* pheaderv30bsp = reinterpret_cast<const dv30header_t*>(m_pBSPFile);
					ptexturelump = reinterpret_cast<const dmiptexlump_t*>(m_pBSPFile + pheaderv30bsp->lumps[V30_LUMP_TEXTURES].offset);
				}
				break;
			default:
				Con_EPrintf("%s - BSP file '%s' has wrong version number '%d', which should be '%d'.\n", __FUNCTION__, ens.pworld->name.c_str(), bspVersion, BSPV30_VERSION);
				return nullptr;
				break;
			}
		}

		for(Int32 i = 0; i < ptexturelump->nummiptex; i++)
		{
			if(ptexturelump->dataoffsets[i] == -1)
				continue;

			const dmiptex_t* pmiptex = reinterpret_cast<const dmiptex_t*>(reinterpret_cast<const byte*>(ptexturelump) + ptexturelump->dataoffsets[i]);
			if(!pmiptex->offsets[0])
				continue;

			CString texturename(pmiptex->name);
			texturename.tolower();

			if(!qstrcmp(bsptexturename, texturename))
			{
				Uint32 texturesize = pmiptex->height*pmiptex->width;
				Uint32 paletteoffs = texturesize + (texturesize/4) + (texturesize/16) + (texturesize/64);
				const byte *pcolorindexes = reinterpret_cast<const byte*>(pmiptex) + pmiptex->offsets[0];
				const color24_t *ppalette = reinterpret_cast<const color24_t*>(pcolorindexes + paletteoffs + 2);

				en_texture_t* ptexture = pTextureManager->LoadPallettedTexture(pstrTextureName, RS_GAME_LEVEL, pcolorindexes, ppalette, pmiptex->width, pmiptex->height, pmaterial->flags);
				if(ptexture)
					return ptexture;
			}
		}

		// Just set the dummy one
		return pTextureManager->GetDummyTexture();
	}

	for(Uint32 i = 0; i < m_pWADFilesArray.size(); i++)
	{
		if(qstrcmp(pstrContainerName, m_pWADFilesArray[i].wadfilename))
			continue;

		const wad3info_t* pwadinfo = reinterpret_cast<const wad3info_t*>(m_pWADFilesArray[i].pwadfile);
		const wad3lumpinfo_t* plumps = reinterpret_cast<const wad3lumpinfo_t*>(reinterpret_cast<const byte*>(pwadinfo) + pwadinfo->infotableofs);

		for(Int32 j = 0; j < pwadinfo->numlumps; j++)
		{
			const wad3lumpinfo_t* plump = &plumps[j];
			if(plump->type != 0 && !(plump->type & 0x43))
				continue;

			const dmiptex_t *pmiptex = reinterpret_cast<const dmiptex_t*>(m_pWADFilesArray[i].pwadfile + plump->filepos);
			if(!pmiptex->offsets[0])
				continue;

			CString texturename(pmiptex->name);
			texturename.tolower();

			if(!qstrcmp(texturename, bsptexturename))
			{
				Uint32 texturesize = pmiptex->height*pmiptex->width;
				Uint32 paletteoffs = texturesize + (texturesize/4) + (texturesize/16) + (texturesize/64);
				const byte *pcolorindexes = reinterpret_cast<const byte*>(pmiptex) + pmiptex->offsets[0];
				const color24_t *ppalette = reinterpret_cast<const color24_t*>(pcolorindexes + paletteoffs + 2);

				en_texture_t* ptexture = pTextureManager->LoadPallettedTexture(pstrTextureName, RS_GAME_LEVEL, pcolorindexes, ppalette, pmiptex->width, pmiptex->height, pmaterial->flags);
				if(ptexture)
					return ptexture;
			}
		}
	}

	// Just set the dummy one
	return pTextureManager->GetDummyTexture();
}

//=============================================
// @brief
//
//=============================================
bool WAD_LoadDetailTextureAssociations( CArray<detailtexture_t*>& detailtextures, CArray<detail_association_t*>& associations )
{
	// Load detail texture associations
	Uint32 filesize = 0;
	const byte* pfile = FL_LoadFile(DETAIL_TEXTURE_ASSOCIATION_FILE_PATH, &filesize);
	if(!pfile)
	{
		Con_Printf("%s - Failed to load '%s'.\n", __FUNCTION__, DETAIL_TEXTURE_ASSOCIATION_FILE_PATH);
		return false;
	}

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Count the number of lines
	static Char line[MAX_LINE_LENGTH];
	Uint32 linecount = Common::GetFileLineCount(reinterpret_cast<const Char*>(pfile));

	associations.reserve(linecount);
	detailtextures.reserve(linecount);
	linecount = 0;

	// Read the contents
	const Char* pstr = reinterpret_cast<const Char*>(pfile);
	while(pstr)
	{
		linecount++;

		// Parse the line
		pstr = Common::ReadLine(pstr, line);
		if(!qstrlen(line))
			continue;

		// Read in the map texture name token
		static Char maptexturename[MAX_PARSE_LENGTH];
		const Char* plinestr = Common::Parse(line, maptexturename);
		if(!plinestr)
		{
			Con_Printf("%s - Unexpected end of line %d in '%s'.\n", __FUNCTION__, linecount, DETAIL_TEXTURE_ASSOCIATION_FILE_PATH);
			continue;
		}

		// Read in the detail texture name token
		static Char detailtexturename[MAX_PARSE_LENGTH];
		plinestr = Common::Parse(plinestr, detailtexturename);
		if(!qstrlen(maptexturename))
		{
			Con_Printf("%s - Unexpected end of line %d in '%s'.\n", __FUNCTION__, linecount, DETAIL_TEXTURE_ASSOCIATION_FILE_PATH);
			continue;
		}

		// Convert both to lowercase
		Common::ConvertStringToLowerCase(maptexturename);
		Common::ConvertStringToLowerCase(detailtexturename);

		// Load the detail texture
		CString filepath;
		filepath << DETAIL_TEXTURE_FOLDER_NAME << PATH_SLASH_CHAR << detailtexturename << ".dds";
		en_texture_t* ptexture = pTextureManager->LoadTexture(filepath.c_str(), RS_GAME_LEVEL);
		if(!ptexture)
		{
			Con_Printf("%s - Detail texture '%s', specified in '%s' does not exist.\n", __FUNCTION__, filepath.c_str(), DETAIL_TEXTURE_ASSOCIATION_FILE_PATH);
			continue;
		}

		Int32 detailtexindex = -1;
		for(Uint32 i = 0; i < detailtextures.size(); i++)
		{
			if(!qstrcmp(detailtextures[i]->filename, filepath))
				detailtexindex = i;
		}

		if(detailtexindex == -1)
		{
			detailtexindex = detailtextures.size();

			// Create detail texture entry
			detailtexture_t* pdttexture = new detailtexture_t;
			qstrcpy(pdttexture->filename, filepath.c_str());
			pdttexture->width = ptexture->width;
			pdttexture->height = ptexture->height;
			detailtextures.push_back(pdttexture);
		}

		// Create the association
		detail_association_t* pnewassoc = new detail_association_t;
		qstrcpy(pnewassoc->maptexturename, maptexturename);
		Common::ConvertStringToLowerCase(pnewassoc->maptexturename);
		pnewassoc->detailtextureidx = detailtexindex;
		associations.push_back(pnewassoc);
	}
	FL_FreeFile(pfile);

	// Resize arrays
	associations.resize(associations.size());
	detailtextures.resize(detailtextures.size());

	return true;	
}

//=============================================
// @brief
//
//=============================================
CString WAD_GetWADFolderPath( const Char* pstrContainerName, const Char* pstrBasePath )
{
	// Build folder path
	CString basename;
	Common::Basename(pstrContainerName, basename);

	CString folderpath;
	if(pstrBasePath)
		folderpath << pstrBasePath;
	
	folderpath << basename << PATH_SLASH_CHAR;
	return folderpath;
}

//=============================================
// @brief
//
//=============================================
CString WAD_GetWADTexturePath( const Char* pstrFolderName, const Char* pstrWorldTextureName )
{
	// Create the PMF file
	CString filepath;
	filepath << pstrFolderName << pstrWorldTextureName << PMF_FORMAT_EXTENSION;

	return filepath;
}