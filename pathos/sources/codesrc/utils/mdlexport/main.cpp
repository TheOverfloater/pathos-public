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
#include "studio.h"
#include "constants.h"
#include "tgaformat.h"
#include "vbmconvert.h"

// Path to model textures root folder
static const Char MODEL_TEXTURES_BASE_PATH[] = "models/";
// Path to model textures root folder
static const Char TEXTURES_BASE_PATH[] = "textures/";
// Texture flag in studiomodels for alphatest
static const Int32 STUDIO_TEX_FL_ALPHATEST = 64;
// Texture flag in studiomodels for additive
static const Int32 STUDIO_TEX_FL_ADDITIVE = 32;
// Texture flag in studiomodels for chrome
static const Int32 STUDIO_TEX_FL_CHROME = 0x0002;
// Texture flag in studiomodels for alphatest
static const Int32 STUDIO_TEX_FL_ALPHABLEND = 8192;
// Texture flag in studiomodels for alphatest
static const Int32 STUDIO_TEX_FL_EYEGLINT = 16384;
// Texture flag in studiomodels for no culling
static const Int32 STUDIO_TEX_FL_NOCULL = 131072;
// Texture flag in studiomodels for no culling
static const Int32 STUDIO_TEX_FL_SCOPE = 2048;

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

//=============================================
// @brief Loads a TGA file and returns it's data
//
// @param pdata Image data to write
// @param bpp Bits per pixel
// @param width Width of the image
// @param height Height of the image
// @param pbuffer Destination buffer
//=============================================
void TGA_WriteData( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, byte* pbuffer )
{
	for(Uint32 i = 0; i < height; i++)
	{
		byte* pdst = pbuffer + sizeof(tga_header_t) + (height-i-1)*width*bpp;
		const byte* psrc = pdata + i*width*bpp;
	
		if(bpp == 4)
		{
			for(Uint32 j = 0; j < width*bpp; j += bpp)
			{
				pdst[j] = psrc[j+2];
				pdst[j+1] = psrc[j+1];
				pdst[j+2] = psrc[j];
				pdst[j+3] = psrc[j+3];
			}
		}
		else
		{
			for(Uint32 j = 0; j < width*bpp; j += bpp)
			{
				pdst[j] = psrc[j+2];
				pdst[j+1] = psrc[j+1];
				pdst[j+2] = psrc[j];
			}
		}
	}
}

//=============================================
// @brief Loads a TGA file and returns it's data
//
// @param pdata Image data to write
// @param bpp Bits per pixel
// @param width Width of the image
// @param height Height of the image
// @param pstrFilename destination file path
//=============================================
bool ExportMDLTGA( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, const Char* pstrFilename )
{
	Uint32 datasize = width*height*bpp + sizeof(tga_header_t);
	byte* pbuffer = new byte[datasize];
	memset(pbuffer, 0, sizeof(byte)*datasize);

	tga_header_t* pheader = (tga_header_t*)pbuffer;
	pheader->datatypecode = TGA_DATATYPE_RGB;
	pheader->bitsperpixel = bpp*8;
	pheader->width[0] = (width & 0xFF);
	pheader->width[1] = ((width >> 8) & 0xFF);
	pheader->height[0] = (height & 0xFF);
	pheader->height[1] = ((height >> 8) & 0xFF);
	pheader->imagedescriptor |= 8;

	// Store data to the buffer
	TGA_WriteData(pdata, bpp, width, height, pbuffer);

	FILE* pf = fopen(pstrFilename, "wb");
	if(!pf)
	{
		printf("Couldn't open '%s' for writing.\n", pstrFilename);
		return false;
	}

	Uint32 numwritten = fwrite(pbuffer, sizeof(byte), datasize, pf);
	fclose(pf);

	// Delete from memory
	delete[] pbuffer;

	// Check that write was successful
	if(numwritten != datasize)
	{
		printf("Failed to write %d bytes for '%s'.\n", datasize, pstrFilename);
		return false;
	}

	return true;
}

//===============================================
// 
//
//===============================================
bool IsNullTexture( color24_t* ppalette, const Char* pstrName, Int32 width, Int32 height )
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
// 
//
//===============================================
bool ExportMDLTexture( const Char* pstrFilepath, color24_t* ppalette, byte* ppixeldata, Int32 width, Int32 height, bool alphatest )
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
					pix1.r = 0; pix1.g = 0; pix1.b = 0; alpha1 = 0;							
				} 

				if (ppixeldata[prow1[i] + pcol2[j]] == 0xFF) 
				{
					pix2.r = 0; pix2.g = 0; pix2.b = 0; alpha2 = 0;				
				} 

				if (ppixeldata[prow2[i] + pcol1[j]] == 0xFF) 
				{
					pix3.r = 0; pix3.g = 0; pix3.b = 0; alpha3 = 0;
				} 

				if (ppixeldata[prow2[i] + pcol2[j]] == 0xFF) 
				{
					pix4.r = 0; pix4.g = 0; pix4.b = 0; alpha4 = 0;
				}
			}

			pout[0] = (pix1.r + pix2.r + pix3.r + pix4.r)>>2;
			pout[1] = (pix1.g + pix2.g + pix3.g + pix4.g)>>2;
			pout[2] = (pix1.b + pix2.b + pix3.b + pix4.b)>>2;

			if(alphatest)
				pout[3] = (alpha1 + alpha2 + alpha3 + alpha4)>>2;
		}
	}

	bool result = ExportMDLTGA(ptex, bpp, outwidth, outheight, pstrFilepath);

	delete[] ptex;
	delete[] prow1;
	delete[] prow2;
	delete[] pcol1;
	delete[] pcol2;

	return result;
}

//===============================================
// 
//
//===============================================
bool CreatePMFFile( const Char* pstrOutputPath, const Char* pstrModelBaseName, const Char* pstrTextureFilename, const Char* pstrTextureBaseName, const Char* pstrTexturePath, CArray<CString>& attribsArray )
{
	bool hasNormalMap = false;
	bool isNormalDDS = false;

	bool hasLuminanceMap = false;
	bool isLuminanceDDS = false;

	bool hasSpecularMap = false;
	bool isSpecularDDS = false;

	bool isDDS = (CString(pstrTextureFilename).find(0, "dds") != -1 || CString(pstrTextureFilename).find(0, "DDS") != -1) ? true : false;

	CString specularfilepath;
	specularfilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << "_specular.tga";
	if(FileExists(specularfilepath.c_str()))
		hasSpecularMap = true;
	else
	{
		specularfilepath.clear();
		specularfilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << "_specular.dds";

		if(FileExists(specularfilepath.c_str()))
		{
			hasSpecularMap = true;
			isSpecularDDS = true;
		}
	}

	CString normalmapfilepath;
	normalmapfilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << "_normal.tga";
	if(FileExists(normalmapfilepath.c_str()))
		hasNormalMap = true;
	else
	{
		normalmapfilepath.clear();
		normalmapfilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << "_normal.dds";

		if(FileExists(normalmapfilepath.c_str()))
		{
			hasNormalMap = true;
			isNormalDDS = true;
		}
	}

	CString luminancefilepath;
	luminancefilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << "_luminance.tga";
	if(FileExists(luminancefilepath.c_str()))
		hasLuminanceMap = true;
	else
	{
		luminancefilepath.clear();
		luminancefilepath << pstrOutputPath << PATH_SLASH_CHAR << pstrTextureBaseName << "_luminance.dds";

		if(FileExists(luminancefilepath.c_str()))
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

	printf("Generating pmf file '%s' for '%s'.\n", pmffilepath.c_str(), pstrModelBaseName);

	FILE* pf = fopen(pmffilepath.c_str(), "wb");
	if(!pf)
	{
		printf("Couldn't open '%s' for writing.\n", pmffilepath.c_str());
		return false;
	}

	Uint32 numwritten = fwrite(filecontents.c_str(), sizeof(byte), filecontents.length(), pf);
	fclose(pf);

	// Check that write was successful
	if(numwritten != filecontents.length())
	{
		printf("Failed to write %d bytes for '%s'.\n", filecontents.length(), pmffilepath.c_str());
		return false;
	}

	return true;
}

//===============================================
// 
//
//===============================================
Uint32 GeneratePMFFiles( const Char* pstrModelBaseName, const Char* pstrOutputPath, mstudiotexture_t* ptextures, Uint32 numtextures )
{
	Uint32 numwritten = 0;

	CString searchpath;
	searchpath << pstrOutputPath << PATH_SLASH_CHAR << "*";

	// Parse directory for files
	HANDLE dir;
	WIN32_FIND_DATA file_data;
	if ((dir = FindFirstFile(searchpath.c_str(), &file_data)) == INVALID_HANDLE_VALUE)
	{
		printf("Directory %s not found.\n", pstrOutputPath);
		return -1;
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
			if(filename.find(0, "_normal") != -1 || filename.find(0, "_specular") != -1 || filename.find(0, "_luminance") != -1)
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
			for(Uint32 i = 0; i < numtextures; i++)
			{
				CString texname;
				Common::Basename(ptextures[i].name, texname);
				texname.tolower();

				if(!qstrcmp(texname, texturebasename))
				{
					if(ptextures[i].flags & STUDIO_TEX_FL_ALPHATEST)
						attribsArray.push_back("$alphatest");
					if(ptextures[i].flags & STUDIO_TEX_FL_ADDITIVE)
						attribsArray.push_back("$additive");
					if(ptextures[i].flags & STUDIO_TEX_FL_CHROME)
						attribsArray.push_back("$chrome");
					if(ptextures[i].flags & STUDIO_TEX_FL_ALPHABLEND)
						attribsArray.push_back("$alphablend");
					if(ptextures[i].flags & STUDIO_TEX_FL_EYEGLINT)
						attribsArray.push_back("$eyeglint");
					if(ptextures[i].flags & STUDIO_TEX_FL_NOCULL)
						attribsArray.push_back("$nocull");
					if(ptextures[i].flags & STUDIO_TEX_FL_SCOPE)
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
// 
//
//===============================================
bool ExportMDLTextures( const Char* pstrFilepath, const Char* pstrOutputPath )
{
	SDL_RWops* pf = SDL_RWFromFile(pstrFilepath, "rb");
	if(!pf)
	{
		printf("Failed to open %s for reading: %s.\n", pstrFilepath, SDL_GetError());
		SDL_ClearError();
		return false;
	}

	SDL_RWseek(pf, 0, RW_SEEK_END);
	Int32 size = (Int32)SDL_RWtell(pf);
	SDL_RWseek(pf, 0, RW_SEEK_SET);

	byte* pbuffer = new byte[size+1];
	size_t numbytes = SDL_RWread(pf, pbuffer, 1, size);
	SDL_RWclose(pf);

	studiohdr_t* phdr = (studiohdr_t*)pbuffer;
	if(!phdr->numtextures || !phdr->texturedataindex)
	{
		printf("Model '%s' has no texture data available.\n", pstrFilepath);
		delete[] pbuffer;
		return false;
	}

	CString basename;
	Common::Basename(pstrFilepath, basename);

	CString testname(pstrFilepath);
	Uint32 tpos = testname.find(0, "T.mdl");
	if(tpos != -1)
	{
		// Check if the non-T file model exists
		CString testpath;
		testpath.assign(testname.c_str(), tpos);
		testpath << ".mdl";
		if(FileExists(testpath.c_str()))
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
		if(!CreateDirectory(dirpath.c_str()))
		{
			printf("Failed to create directory '%s'.\n", dirpath.c_str());
			delete[] pbuffer;
			return false;
		}

		// Skip any slashes
		while(pstr && *pstr == PATH_SLASH_CHAR)
			pstr++;
	}

	// Keep track of exports
	Uint32 numexported = 0;

	// Convert each texture and export
	mstudiotexture_t* ptextures = (mstudiotexture_t*)((byte*)phdr + phdr->textureindex);
	for(Uint32 i = 0; i < phdr->numtextures; i++)
	{
		mstudiotexture_t* ptexture = &ptextures[i];

		CString texturename(ptexture->name);
		Uint32 ofs = 0;
		while(true)
		{
			ofs = texturename.find(ofs, " ");
			if(ofs == -1)
				break;

			texturename.erase(ofs, 1);
			texturename.insert(ofs, "_");
		}

		CString texturebasename;
		Common::Basename(texturename.c_str(), texturebasename);

		// Retrieve pixel data ptr
		byte* ppixeldata = ((byte*)phdr + ptexture->index);
		// Get pallette data ptr
		color24_t* ppalette = (color24_t*)((byte*)phdr + ptexture->index+ptexture->width*ptexture->height);

		if(IsNullTexture(ppalette, ptexture->name, ptexture->width, ptexture->height))
		{
			printf("Skipping null texture for '%s'.\n", basename.c_str());
			continue;
		}

		// Texture entry that goes into PMF file
		CString pmftexturepath;
		pmftexturepath << MODEL_TEXTURES_BASE_PATH << basename << PATH_SLASH_CHAR << texturebasename;

		// Check if DDS already exists
		CString ddsoutputfilepath;
		ddsoutputfilepath << pstrOutputPath << PATH_SLASH_CHAR << TEXTURES_BASE_PATH << PATH_SLASH_CHAR << pmftexturepath << ".dds";
		if(FileExists(ddsoutputfilepath.c_str()))
			continue;

		// Output file path for TGA
		CString tgaoutputfilepath;
		tgaoutputfilepath << pstrOutputPath << PATH_SLASH_CHAR << TEXTURES_BASE_PATH << PATH_SLASH_CHAR << pmftexturepath << ".tga";
		if(FileExists(tgaoutputfilepath.c_str()))
			continue;

		// For short
		bool alphatest = (ptexture->flags & STUDIO_TEX_FL_ALPHATEST) ? true : false;

		printf("Exporting texture '%s' for model '%s'.\n", tgaoutputfilepath.c_str(), basename.c_str());

		// Export TGA
		if(!ExportMDLTexture(tgaoutputfilepath.c_str(), ppalette, ppixeldata, ptexture->width, ptexture->height, alphatest))
			printf("Failed to export model texture '%s'.\n", tgaoutputfilepath.c_str());

		numexported++;
	}

	printf("%d textures exported for model '%s'.\n", numexported, basename.c_str());

	// Now generate PMF files for those in the folder
	Uint32 pmfcount = GeneratePMFFiles(basename.c_str(), createpath.c_str(), ptextures, phdr->numtextures);
	printf("%d pmf files generated for model '%s'.\n", pmfcount, basename.c_str());

	return true;
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
		printf("Usage: <target file/directory>");
		printf("Press any key to exit...\n");
		getchar();
		return -1;
	}

	// Create dir if missing
	if(!DirectoryExists(argv[2]))
		CreateDirectory(argv[2]);

	Uint32 numExported = 0;
	if(qstrstr(argv[1], ".mdl"))
	{
		CString filepath(argv[1]);

		CString texturefilepath(filepath);
		if(filepath.find(0, "T.mdl") == -1)
		{
			CString filepathwithT(filepath);

			filepathwithT.erase(filepathwithT.find(0, ".mdl"), 4);
			filepathwithT << "T.mdl";

			if(FileExists(filepathwithT.c_str()))
				texturefilepath = filepathwithT;
		}

		if(ExportMDLTextures(texturefilepath.c_str(), argv[2]))
			numExported++;

		// Also convert to VBM
		VBM_ConvertModel(filepath.c_str(), texturefilepath.c_str(), argv[2]);
	}
	else
	{
		CString searchpath;
		searchpath << argv[1] << PATH_SLASH_CHAR << "*.mdl";

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

			CString texturefilepath(filepath);
			if (qstrcmp(file_data.cFileName, ".") != 0 && qstrcmp(file_data.cFileName, "..") != 0 && qstrstr(file_data.cFileName, ".mdl"))
			{
				if(filepath.find(0, "T.mdl") == -1)
				{
					CString filepathwithT(filepath);

					filepathwithT.erase(filepathwithT.find(0, ".mdl"), 4);
					filepathwithT << "T.mdl";

					if(FileExists(filepathwithT.c_str()))
						texturefilepath = filepathwithT;
				}

				// Convert this file
				if(ExportMDLTextures(texturefilepath.c_str(), argv[2]))
					numExported++;

				// Also convert to VBM
				VBM_ConvertModel(filepath.c_str(), texturefilepath.c_str(), argv[2]);
			}

			if(!FindNextFile(dir, &file_data))
				break;
		}
	}

	if(numExported == 0)
	{
		printf("Error: MDL file(s) not found.\n");
		printf("Press any key to exit...\n");
		getchar();
		return 1;
	}
	else
	{
		printf("%d files exported.\n", numExported);
		printf("Press any key to exit...\n");
		getchar();
		return 0;
	}

	return 0;
}
