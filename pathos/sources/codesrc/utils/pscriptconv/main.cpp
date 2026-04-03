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
#include "main.h"
#include "constants.h"
#include "utils_common.h"
#include "utils_filefuncs.h"
#include "pscriptconverter.h"

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
		Msg("Press any key to exit...\n");
		getchar();
		return -1;
	}

	// Create dir if missing
	if(!DirectoryExists(argv[2]))
		g_fileInterface.pfnCreateDirectory(argv[2]);

	Uint32 numExported = 0;
	if(qstrstr(argv[1], ".txt"))
	{
		CString filepath(argv[1]);

		// Convert script to new format
		if(gScriptConverter.ProcessScript(filepath.c_str(), argv[2]))
			numExported++;
	}
	else
	{
		CString searchpath;
		searchpath << argv[1] << PATH_SLASH_CHAR << "*.txt";

		// Parse directory for files
		HANDLE dir;
		WIN32_FIND_DATA file_data;
		if ((dir = FindFirstFile(searchpath.c_str(), &file_data)) == INVALID_HANDLE_VALUE)
		{
			ErrorMsg("Directory %s not found.\n", argv[1]);
			return -1;
		}

		while (true) 
		{
			CString filepath;
			filepath << argv[1] << PATH_SLASH_CHAR << file_data.cFileName;

			CString texturefilepath(filepath);
			if (qstrcmp(file_data.cFileName, ".") != 0 && qstrcmp(file_data.cFileName, "..") != 0 && qstrstr(file_data.cFileName, ".txt"))
			{
				// Convert script to new format
				if(gScriptConverter.ProcessScript(filepath.c_str(), argv[2]))
					numExported++;
			}

			if(!FindNextFile(dir, &file_data))
				break;
		}
	}

	if(numExported == 0)
	{
		ErrorMsg("Error: Particle scripts file(s) not found.\n");
		Msg("Press any key to exit...\n");
		getchar();
		return 1;
	}
	else
	{
		Msg("%d files converted.\n", numExported);
		Msg("Press any key to exit...\n");
		getchar();
		return 0;
	}

	return 0;
}
