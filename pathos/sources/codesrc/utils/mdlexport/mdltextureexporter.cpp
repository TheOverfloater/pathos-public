/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.

===============================================
*/

#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )

#include <SDL.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include "includes.h"
#include "common.h"
#include "studio.h"
#include "constants.h"
#include "tgaformat.h"
#include "mdltextureexporter.h"
#include "com_math.h"
#include "main.h"
#include "utils_filefuncs.h"
#include "utils_common.h"
#include "tga.h"

// Object declaration
CMDLTextureExporter gMDLTextureExporter;

// Path to model textures root folder
const Char CMDLTextureExporter::MODEL_TEXTURES_BASE_PATH[] = "models/";
// Path to model textures root folder
const Char CMDLTextureExporter::TEXTURES_BASE_PATH[] = "textures/";

//===============================================
// @brief Constructor for CMDLTextureExporter class
//
//===============================================
CMDLTextureExporter::CMDLTextureExporter( void ):
	m_pTextureHeader(nullptr),
	m_pStudioTextures(nullptr),
	m_nbBytesWritten(0)
{
}

//===============================================
// @brief Destructor for CMDLVBMConverter class
//
//===============================================
CMDLTextureExporter::~CMDLTextureExporter( void )
{
}

//===============================================
// @brief Tells if a texture is a null texture generated
// by the VBM compiler for a model
//
// @param ppalette Palette for the texture
// @param pstrName Name of the texture
// @param width Width of the texture
// @param height Height of the texture
// @return TRUE if it's a null texture, FALSE otherwise
//===============================================
bool CMDLTextureExporter::IsNullTexture( const color24_t* ppalette, const Char* pstrName, Int32 width, Int32 height )
{
	if(qstrcmp(pstrName, "null"))
		return false;

	if(width != 32 || height != 32)
		return false;

	color24_t whitecolor(255, 255, 255);
	for(Uint32 i = 0; i < 256; i++)
	{
		if(memcmp(&ppalette[i], &whitecolor, sizeof(color24_t)))
			return false;
	}

	return true;
}

//===============================================
// @brief Convert a palette texture to a bitmap file 
//
// @param pstrFilepath Path to output file
// @param ppalette Palette of texture
// @param ppixeldata 8-bit pixel data for texture
// @param width Width of texture
// @param height Height of texture
// @param alphatest TRUE if the texture uses 1-bit alpha
// @return TRUE if the texture was successfully exported, FALSE otherwise
//===============================================
bool CMDLTextureExporter::ExportTextureToTGA( const Char* pstrFilepath, const color24_t* ppalette, const byte* ppixeldata, Int32 width, Int32 height, bool alphatest, Uint32* ptrCompressionRatio )
{
	color24_t pix1, pix2, pix3, pix4;
	byte alpha1, alpha2, alpha3, alpha4;

	// convert texture to power of 2
	Uint32 outwidth, outheight;
	for (outwidth = 1; outwidth < width; outwidth <<= 1);
	for (outheight = 1; outheight < height; outheight <<= 1);

	// Allocate arrays
	Int32* prow1 = new Int32[outheight];
	Int32* prow2 = new Int32[outheight];
	Int32* pcol1 = new Int32[outwidth];
	Int32* pcol2 = new Int32[outwidth];

	Uint32 bpp = (alphatest) ? 4 : 3;
	byte* ptex = new byte[(outwidth*outheight*bpp*sizeof(byte))];
	byte* pout = ptex;

	for (Uint32 i = 0; i < outwidth; i++)
	{
		pcol1[i] = (Int32) ((i + 0.25) * (width / (Float)outwidth));
		pcol2[i] = (Int32) ((i + 0.75) * (width / (Float)outwidth));
	}

	for (Uint32 i = 0; i < outheight; i++)
	{
		prow1[i] = (Int32) ((i + 0.25) * (height / (Float)outheight)) * width;
		prow2[i] = (Int32) ((i + 0.75) * (height / (Float)outheight)) * width;
	}

	for (Uint32 i = 0; i < outheight; i++)
	{
		for (Uint32 j = 0; j < outwidth; j++, pout += bpp)
		{
			pix1 = ppalette[ppixeldata[prow1[i] + pcol1[j]]];
			pix2 = ppalette[ppixeldata[prow1[i] + pcol2[j]]];
			pix3 = ppalette[ppixeldata[prow2[i] + pcol1[j]]];
			pix4 = ppalette[ppixeldata[prow2[i] + pcol2[j]]];
			alpha1 = 0xFF; alpha2 = 0xFF; alpha3 = 0xFF; alpha4 = 0xFF;

			if(alphatest)
			{
				if (ppixeldata[prow1[i] + pcol1[j]] == 0xFF) 
				{
					pix1.r = 0; 
					pix1.g = 0; 
					pix1.b = 0; 
					alpha1 = 0;							
				} 

				if (ppixeldata[prow1[i] + pcol2[j]] == 0xFF) 
				{
					pix2.r = 0; 
					pix2.g = 0; 
					pix2.b = 0; 
					alpha2 = 0;				
				} 

				if (ppixeldata[prow2[i] + pcol1[j]] == 0xFF) 
				{
					pix3.r = 0; 
					pix3.g = 0; 
					pix3.b = 0;
					alpha3 = 0;
				} 

				if (ppixeldata[prow2[i] + pcol2[j]] == 0xFF) 
				{
					pix4.r = 0; 
					pix4.g = 0; 
					pix4.b = 0; 
					alpha4 = 0;
				}
			}

			pout[0] = (pix1.r + pix2.r + pix3.r + pix4.r)>>2;
			pout[1] = (pix1.g + pix2.g + pix3.g + pix4.g)>>2;
			pout[2] = (pix1.b + pix2.b + pix3.b + pix4.b)>>2;

			if(alphatest)
				pout[3] = (alpha1 + alpha2 + alpha3 + alpha4)>>2;
		}
	}

	delete[] prow1;
	delete[] prow2;
	delete[] pcol1;
	delete[] pcol2;

	Uint32 bytesWritten = 0;
	bool result = TGA_Write(ptex, bpp, outwidth, outheight, pstrFilepath, g_fileInterface, ErrorMsg, ptrCompressionRatio, &bytesWritten, true);
	if(result)
		m_nbBytesWritten += bytesWritten;

	delete[] ptex;

	return result;
}

//===============================================
// @brief Creates a PMF file based on the attributes
//
// @param pstrOutputPath Output folder path
// @param pstrModelBaseName Basename of model
// @param pstrTextureFilename Name of texture file
// @param pstrTextureBaseName Basename of texture file
// @param pstrTexturePath Texture path for export
// @param attribsArray Array of attributes
// @return TRUE if successful, FALSE otherwise
// //===============================================
bool CMDLTextureExporter::CreatePMFFile( const Char* pstrOutputPath, const Char* pstrModelBaseName, const Char* pstrTextureFilename, const Char* pstrTextureBaseName, const Char* pstrTexturePath, CArray<CString>& attribsArray )
{
	bool hasNormalMap = false;
	bool isNormalDDS = false;

	bool hasLuminanceMap = false;
	bool isLuminanceDDS = false;

	bool hasSpecularMap = false;
	bool isSpecularDDS = false;

	bool isDDS = (CString(pstrTextureFilename).find(0, "dds") != CString::CSTRING_NO_POSITION || CString(pstrTextureFilename).find(0, "DDS") != CString::CSTRING_NO_POSITION) ? true : false;

	CString specularfilepath;
	specularfilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << "_specular.tga";
	if(g_fileInterface.pfnFileExists(specularfilepath.c_str()))
	{
		hasSpecularMap = true;
	}
	else
	{
		specularfilepath.clear();
		specularfilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << "_specular.dds";

		if(g_fileInterface.pfnFileExists(specularfilepath.c_str()))
		{
			hasSpecularMap = true;
			isSpecularDDS = true;
		}
	}

	CString normalmapfilepath;
	normalmapfilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << "_normal.tga";
	if(g_fileInterface.pfnFileExists(normalmapfilepath.c_str()))
	{
		hasNormalMap = true;
	}
	else
	{
		normalmapfilepath.clear();
		normalmapfilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << "_normal.dds";

		if(g_fileInterface.pfnFileExists(normalmapfilepath.c_str()))
		{
			hasNormalMap = true;
			isNormalDDS = true;
		}
	}

	CString luminancefilepath;
	luminancefilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << "_luminance.tga";
	if(g_fileInterface.pfnFileExists(luminancefilepath.c_str()))
	{
		hasLuminanceMap = true;
	}
	else
	{
		luminancefilepath.clear();
		luminancefilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << "_luminance.dds";

		if(g_fileInterface.pfnFileExists(luminancefilepath.c_str()))
		{
			hasLuminanceMap = true;
			isLuminanceDDS = true;
		}
	}

	CString filecontents;
	filecontents << "$texture" << NEWLINE;
	filecontents << "{" << NEWLINE;
	for(Uint32 i = 0; i < attribsArray.size(); i++)
		filecontents << "\t" << attribsArray[i] << NEWLINE;

	filecontents << "\t$texture diffuse " << MODEL_TEXTURES_BASE_PATH << pstrModelBaseName << PATH_SLASH_CHAR << pstrTextureBaseName << (isDDS ? ".dds" : ".tga") << NEWLINE;
	if(hasNormalMap)
		filecontents << "\t$texture normal " << MODEL_TEXTURES_BASE_PATH << pstrModelBaseName << PATH_SLASH_CHAR << pstrTextureBaseName << "_normal." << (isNormalDDS ? "dds" : "tga") << NEWLINE;
	if(hasSpecularMap)
		filecontents << "\t$texture specular " << MODEL_TEXTURES_BASE_PATH << pstrModelBaseName << PATH_SLASH_CHAR << pstrTextureBaseName << "_normal." << (isSpecularDDS ? "dds" : "tga") << NEWLINE;
	if(hasLuminanceMap)
		filecontents << "\t$texture luminance " << MODEL_TEXTURES_BASE_PATH << pstrModelBaseName << PATH_SLASH_CHAR << pstrTextureBaseName << "_luminance." << (isLuminanceDDS ? "dds" : "tga") << NEWLINE;
	filecontents << "}" << NEWLINE;

	CString pmffilepath;
	pmffilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << ".pmf";

	Msg("Generating pmf file '%s' for '%s'.\n", pmffilepath.c_str(), pstrModelBaseName);

	const byte* pdata = reinterpret_cast<const byte*>(filecontents.c_str());
	if(!g_fileInterface.pfnWriteFile(pdata, filecontents.length(), pmffilepath.c_str(), false))
	{
		ErrorMsg("Couldn't open '%s' for writing.\n", pmffilepath.c_str());
		return false;
	}
	else
	{
		m_nbBytesWritten += filecontents.length() * sizeof(Char);
	}

	return true;
}

//===============================================
// @brief Generate PMF files for a model
//
// @param pstrModelBaseName Basename of model
// @param pstrOutputPath Output path
// @param ptextures Pointer to texture info array
// @param numtextures Number of textures
// @return Number of generated PMF files
//===============================================
Uint32 CMDLTextureExporter::GeneratePMFFiles( const Char* pstrModelBaseName, const Char* pstrOutputPath )
{
	Uint32 numwritten = 0;

	CString searchpath;
	searchpath << pstrOutputPath << PATH_SLASH_CHAR << "*";

	// Parse directory for files
	HANDLE dir;
	WIN32_FIND_DATA file_data;
	if ((dir = FindFirstFile(searchpath.c_str(), &file_data)) == INVALID_HANDLE_VALUE)
	{
		ErrorMsg("Directory %s not found.\n", pstrOutputPath);
		return 0;
	}

	while (true) 
	{
		CString filepath;
		filepath << pstrOutputPath << PATH_SLASH_CHAR << file_data.cFileName;

		if (qstrcmp(file_data.cFileName, ".") != 0 && qstrcmp(file_data.cFileName, "..") != 0 
			&& (qstrstr(file_data.cFileName, ".tga") || qstrstr(file_data.cFileName, ".dds")
			|| qstrstr(file_data.cFileName, ".TGA") || qstrstr(file_data.cFileName, ".DDS")))
		{
			CString filename(file_data.cFileName);
			if(filename.find(0, "_normal") != CString::CSTRING_NO_POSITION 
				|| filename.find(0, "_specular") != CString::CSTRING_NO_POSITION 
				|| filename.find(0, "_luminance") != CString::CSTRING_NO_POSITION)
			{
				if(FindNextFile(dir, &file_data))
					continue;
				else
					break;
			}

			CString texturebasename;
			Common::Basename(filename.c_str(), texturebasename);
			texturebasename.tolower();

			CArray<CString> attribsArray;

			bool alphatest = false;
			for(Uint32 i = 0; i < m_pTextureHeader->numtextures; i++)
			{
				const mstudiotexture_t* ptexture = &m_pStudioTextures[i];

				CString texname;
				Common::Basename(ptexture->name, texname);
				texname.tolower();

				if(!qstrcmp(texname, texturebasename))
				{
					if(ptexture->flags & STUDIO_TEX_FL_ALPHATEST)
						attribsArray.push_back("$alphatest");
					if(ptexture->flags & STUDIO_TEX_FL_ADDITIVE)
						attribsArray.push_back("$additive");
					if(ptexture->flags & STUDIO_TEX_FL_CHROME)
						attribsArray.push_back("$chrome");
					if(ptexture->flags & STUDIO_TEX_FL_ALPHABLEND)
						attribsArray.push_back("$alphablend");
					if(ptexture->flags & STUDIO_TEX_FL_EYEGLINT)
						attribsArray.push_back("$eyeglint");
					if(ptexture->flags & STUDIO_TEX_FL_NOCULL)
						attribsArray.push_back("$nocull");
					if(ptexture->flags & STUDIO_TEX_FL_SCOPE)
						attribsArray.push_back("$scope");
					break;
				}
			}

			// Convert this file
			if(CreatePMFFile(pstrOutputPath, pstrModelBaseName, file_data.cFileName, texturebasename.c_str(), filepath.c_str(), attribsArray))
				numwritten++;
		}

		if(!FindNextFile(dir, &file_data))
			break;
	}

	return numwritten;
}

//===============================================
// @brief Export textures for a model
//
// @param pTextureMDLData Texture model ptr
// @param pstrModelFilePath MDL file path
// @param pstrOutputPath Output file path
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CMDLTextureExporter::ExportMDLTextures( const byte* pTextureMDLData, const Char* pstrModelFilePath, const Char* pstrOutputPath )
{
	// Reset this
	m_nbBytesWritten = 0;

	// Get basename from model
	CString basename;
	Common::Basename(pstrModelFilePath, basename);

	// Check if the non-T file model exists, and if it does,
	// remove the "T" part of the name
	CString testname(pstrModelFilePath);
	Uint32 tpos = testname.find(0, "T.mdl");
	if(tpos != CString::CSTRING_NO_POSITION)
	{
		CString testpath;
		testpath.assign(testname.c_str(), tpos);
		testpath << ".mdl";

		if(g_fileInterface.pfnFileExists(testpath.c_str()))
			basename.erase(basename.length()-1, 1);
	}

	CString modeldirectorypath;
	modeldirectorypath << TEXTURES_BASE_PATH << MODEL_TEXTURES_BASE_PATH << basename;
	CString createpath;
	createpath << pstrOutputPath << PATH_SLASH_CHAR << modeldirectorypath;

	// Fix slashes
	createpath = Common::FixSlashes(createpath.c_str());

	CString dirpath, token;
	const Char* pstr = createpath.c_str();
	while(pstr)
	{
		while(*pstr == '\\' || *pstr == '/')
			pstr++;

		CString breakChars;
		breakChars << PATH_SLASH_CHAR;

		pstr = Common::Parse(pstr, token, breakChars.c_str());
		if(token.empty())
			break;

		if(qstrstr(token.c_str(), "."))
			break;
		
		dirpath << token << PATH_SLASH_CHAR;
		if(!g_fileInterface.pfnCreateDirectory(dirpath.c_str()))
		{
			ErrorMsg("Failed to create directory '%s'.\n", dirpath.c_str());
			return false;
		}

		// Skip any slashes
		while(pstr && *pstr == PATH_SLASH_CHAR)
			pstr++;
	}

	// Keep track of exports
	Uint32 numexported = 0;

	// Convert each texture and export
	m_pTextureHeader = reinterpret_cast<const studiohdr_t*>(pTextureMDLData);
	m_pStudioTextures = reinterpret_cast<const mstudiotexture_t*>(reinterpret_cast<const byte*>(m_pTextureHeader) + m_pTextureHeader->textureindex);

	for(Uint32 i = 0; i < m_pTextureHeader->numtextures; i++)
	{
		const mstudiotexture_t* ptexture = &m_pStudioTextures[i];

		CString texturename(ptexture->name);
		Uint32 ofs = 0;
		while(true)
		{
			ofs = texturename.find(ofs, " ");
			if(ofs == CString::CSTRING_NO_POSITION)
				break;

			texturename.erase(ofs, 1);
			texturename.insert(ofs, "_");
		}

		CString texturebasename;
		Common::Basename(texturename.c_str(), texturebasename);

		// Retrieve pixel data ptr
		const byte* ppixeldata = reinterpret_cast<const byte*>(m_pTextureHeader) + ptexture->index;
		// Get pallette data ptr
		const color24_t* ppalette = reinterpret_cast<const color24_t*>(reinterpret_cast<const byte*>(m_pTextureHeader) + ptexture->index+ptexture->width*ptexture->height);

		if(IsNullTexture(ppalette, ptexture->name, ptexture->width, ptexture->height))
		{
			Msg("Skipping null texture for '%s'.\n", basename.c_str());
			continue;
		}

		// Texture entry that goes into PMF file
		CString pmftexturepath;
		pmftexturepath << MODEL_TEXTURES_BASE_PATH << basename << PATH_SLASH_CHAR << texturebasename;

		// Check if DDS already exists
		CString ddsoutputfilepath;
		ddsoutputfilepath << pstrOutputPath << PATH_SLASH_CHAR << TEXTURES_BASE_PATH << PATH_SLASH_CHAR << pmftexturepath << ".dds";
		if(g_fileInterface.pfnFileExists(ddsoutputfilepath.c_str()))
			continue;

		// Output file path for TGA
		CString tgaoutputfilepath;
		tgaoutputfilepath << pstrOutputPath << PATH_SLASH_CHAR << TEXTURES_BASE_PATH << PATH_SLASH_CHAR << pmftexturepath << ".tga";

		// For short
		bool alphatest = (ptexture->flags & STUDIO_TEX_FL_ALPHATEST) ? true : false;

		// Export TGA
		Uint32 compressionRatio = 0;
		if(!ExportTextureToTGA(tgaoutputfilepath.c_str(), ppalette, ppixeldata, ptexture->width, ptexture->height, alphatest, &compressionRatio))
		{
			ErrorMsg("Failed to export model texture '%s'.\n", tgaoutputfilepath.c_str());
		}
		else
		{
			if(compressionRatio < 100)
				Msg("Exported texture '%s' for model '%s'(%d percent compression using RLE).\n", tgaoutputfilepath.c_str(), basename.c_str(), compressionRatio);
			else
				Msg("Exported texture '%s' for model '%s'(uncompressed).\n", tgaoutputfilepath.c_str(), basename.c_str());

			numexported++;
		}
	}

	Msg("%d textures exported for model '%s'.\n", numexported, basename.c_str());

	// Now generate PMF files for those in the folder
	Uint32 pmfcount = GeneratePMFFiles(basename.c_str(), createpath.c_str());
	Msg("%d pmf files generated for model '%s'.\n", pmfcount, basename.c_str());

	m_pTextureHeader = nullptr;
	m_pStudioTextures = nullptr;

	return true;
}