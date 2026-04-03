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
#include "utils_filefuncs.h"
#include "utils_common.h"
#include "mdlvbmconverter.h"
#include "mdltextureexporter.h"
#include "logfile.h"

//===============================================
// @brief Performs functions for closing the applocation
//
//===============================================
void OnExitApplication( void )
{
	if(g_pLogFile)
	{
		if(!g_pLogFile->Close())
			printf("Failed to close log file.\n");

		delete g_pLogFile;
		g_pLogFile = nullptr;
	}

	printf("Press any key to exit...\n");
	getchar();
}

//===============================================
// ConvertAndExportModel
//
//===============================================
bool ConvertAndExportModel( const Char* pstrFilePath, const Char* pstrOutputPath, Uint32& fileTotalBytes )
{
	Msg("----Starting conversion and export of '%s'.\n", pstrFilePath);

	// Load the MDL file in
	const byte* pfiledata = g_fileInterface.pfnLoadFile(pstrFilePath, nullptr);
	if(!pfiledata)
	{
		ErrorMsg("Failed to open model file '%s'.\n", pstrFilePath);
		return false;
	}

	// Now check if we have a "T" file
	CString filepathwithT(pstrFilePath);

	filepathwithT.erase(filepathwithT.find(0, ".mdl"), 4);
	filepathwithT << "T.mdl";

	CString texturefilepath;
	if(g_fileInterface.pfnFileExists(filepathwithT.c_str()))
		texturefilepath = filepathwithT;
	else
		texturefilepath = pstrFilePath;

	// Load in T file if needed
	const byte* ptexturefiledata = nullptr;
	if(!qstrcmp(texturefilepath, filepathwithT))
	{
		ptexturefiledata = g_fileInterface.pfnLoadFile(texturefilepath.c_str(), nullptr);
		if(!ptexturefiledata)
		{
			ErrorMsg("Failed to open texture model file '%s'.\n", texturefilepath.c_str());
			g_fileInterface.pfnFreeFile(pfiledata);
			return false;
		}
	}
	else
	{
		// Textures are contained in the main mdl file
		ptexturefiledata = pfiledata;
	}

	// Export textures for MDL
	if(!gMDLTextureExporter.ExportMDLTextures(ptexturefiledata, pstrFilePath, pstrOutputPath))
	{
		ErrorMsg("Failed to export textures for model '%s'.\n", pstrFilePath);

		if(ptexturefiledata != pfiledata)
			g_fileInterface.pfnFreeFile(ptexturefiledata);

		g_fileInterface.pfnFreeFile(pfiledata);
		return false;
	}

	fileTotalBytes += gMDLTextureExporter.GetNbBytesWritten();

	// Also convert to VBM
	bool result = gMDLVBMConverter.ConvertModel(pstrFilePath, pstrOutputPath, pfiledata, ptexturefiledata);
	if(!result)
	{
		ErrorMsg("Failed to convert model '%s' to VBM format.\n", pstrFilePath);
	}
	else
	{
		Msg("%.6f mbytes written total for '%s'.\n", Common::BytesToMegaBytes(fileTotalBytes), pstrFilePath);
		fileTotalBytes += gMDLVBMConverter.GetNbBytesWritten();
	}

	// Free files we loaded
	if(ptexturefiledata != pfiledata)
		g_fileInterface.pfnFreeFile(ptexturefiledata);

	g_fileInterface.pfnFreeFile(pfiledata);
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
		Msg("Usage: <target file/directory>");
		OnExitApplication();
		return -1;
	}

	// Create dir if missing
	if(!DirectoryExists(argv[2]))
		g_fileInterface.pfnCreateDirectory(argv[2]);

	Uint32 totalBytes = 0;
	Uint32 numExported = 0;
	if(qstrstr(argv[1], ".mdl"))
	{
		CString filepath(argv[1]);
		if(filepath.find(0, "T.mdl") != CString::CSTRING_NO_POSITION)
		{
			ErrorMsg("'T' model was directly specified. Use the base model file to perform conversion.\n");
			OnExitApplication();
			return -1;
		}

		CString basename;
		Common::Basename(filepath.c_str(), basename);

		CString logFilePath;
		logFilePath << argv[2] << PATH_SLASH_CHAR << basename << "_" << Common::GetDateFilename() << ".log";

		// Ensure previous log is deleted
		if(g_fileInterface.pfnFileExists(logFilePath.c_str()))
			remove(logFilePath.c_str());

		g_pLogFile = new CLogFile(logFilePath.c_str(), Msg, g_fileInterface, true, false);
		if(!g_pLogFile->Init())
		{
			WarningMsg("Failed to open log file '%s'.\n", logFilePath.c_str());
			delete g_pLogFile;
			g_pLogFile = nullptr;
		}

		Uint32 fileTotalBytes = 0;
		if(ConvertAndExportModel(filepath.c_str(), argv[2], fileTotalBytes))
		{
			totalBytes += fileTotalBytes;
			numExported++;
		}
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
			ErrorMsg("Directory %s not found.\n", argv[1]);
			OnExitApplication();
			return -1;
		}

		CString logFilePath;
		logFilePath << argv[2] << PATH_SLASH_CHAR << "bulk_conversion_log_" << Common::GetDateFilename() << ".log";

		// Ensure previous log is deleted
		if(g_fileInterface.pfnFileExists(logFilePath.c_str()))
			remove(logFilePath.c_str());

		g_pLogFile = new CLogFile(logFilePath.c_str(), Msg, g_fileInterface, true, false);
		if(!g_pLogFile->Init())
		{
			WarningMsg("Failed to open log file '%s'.\n", logFilePath.c_str());
			delete g_pLogFile;
			g_pLogFile = nullptr;
		}

		while (true) 
		{
			CString filepath;
			filepath << argv[1] << PATH_SLASH_CHAR << file_data.cFileName;

			CString texturefilepath(filepath);
			if (qstrcmp(file_data.cFileName, ".") != 0 && qstrcmp(file_data.cFileName, "..") != 0 && qstrstr(file_data.cFileName, ".mdl"))
			{
				if(filepath.find(0, "T.mdl") != CString::CSTRING_NO_POSITION)
				{
					if(!FindNextFile(dir, &file_data))
						break;

					continue;
				}

				Uint32 fileTotalBytes = 0;
				if(ConvertAndExportModel(filepath.c_str(), argv[2], fileTotalBytes))
				{
					totalBytes += fileTotalBytes;
					numExported++;
				}
			}

			if(!FindNextFile(dir, &file_data))
				break;
		}
	}

	if(numExported == 0)
	{
		ErrorMsg("Error: MDL file(s) not found.\n");
		OnExitApplication();
		return 1;
	}
	else
	{
		Msg("%d files exported.\n", numExported);
		Msg("%.6f megabytes written total.\n", Common::BytesToMegaBytes(totalBytes));
		OnExitApplication();
		return 0;
	}

	return 0;
}
