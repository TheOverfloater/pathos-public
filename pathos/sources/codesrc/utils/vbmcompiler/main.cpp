/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

//
// main.cpp
//

#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )

#include <SDL.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include <stdarg.h>

#include "includes.h"
#include "common.h"
#include "studio.h"
#include "tgaformat.h"
#include "compiler_types.h"
#include "main.h"
#include "options.h"
#include "qcparser.h"
#include "logfile.h"
#include "filefuncs.h"
#include "mcdcompiler.h"

// Size of buffer for message prints
static const Uint32 PRINT_MSG_BUFFER_SIZE = 16384;

// Major version number
static const Uint32 MAJOR_VERSION = 0;
// Minor version number
static const Uint32 MINOR_VERSION = 9;

// Pointer to VBM compiler instance
CVBMCompiler* g_pVBMCompiler = nullptr;
// Pointer to StudioMDL compiler instance
CStudioModelCompiler* g_pStudioModelCompiler = nullptr;
// Pointer to QC parser instance
CQCParser* g_pQCParser = nullptr;
// Log file ptr
CLogFile* g_pLogFile = nullptr;

//===============================================
// @brief Prints out the usage information for the application
//
//===============================================
void ApplicationUsage( void )
{
	Msg("Pathos VBMCompiler:\n");
	Msg("Usage: vbmcompiler <options> <qc file>.\n");
	Msg(" - Options:\n");
	Msg("\t'-r' - Tag reversed triangles.\n");
	Msg("\t'-n' - Tag corrupted normals.\n");
	Msg("\t'-f' - Flip normals of all triangles.\n");
	Msg("\t'-a' - Specify normal blend angle.\n");
	Msg("\t'-h' - Dump hitbox information.\n");
	Msg("\t'-s' - Strip HL1 MDL geometry data.\n");
	Msg("\t'-q' - Strip HL1 MDL texture data.\n");
	Msg("\t'-l' - Remove 2048 vertex limit on HL1 MDL files.\n");
	Msg("\t'-d' - Do not create VBM data for model.\n");
	Msg("\t'-w' - Wait for key input before terminating instance.\n");
	Msg("\t'-t' - Set texture to rename(original name, new name).\n");
	Msg("\t'-e' - Set maximum texture resolution.\n");
	Msg("\t'-p' - Set texture padding size.\n");
	Msg("\t'-m' - Don't enforce collision meshes having to be closed.\n");
}

//===============================================
// @brief Prints a generic message to the console and the log
//
// @param fmt Formatted string
// @param ... Parameters for string formatting
//===============================================
void Msg( const Char *fmt, ... )
{
	// compile the string result
	va_list	vArgPtr;
	static Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	printf(cMsg);

	if(g_pLogFile)
		g_pLogFile->Printf(cMsg);
}

//===============================================
// @brief Prints a generic message to the console and the log
//
// @param fmt Formatted string
// @param ... Parameters for string formatting
//===============================================
void WarningMsg( const Char *fmt, ... )
{
	// compile the string result
	va_list	vArgPtr;
	static Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	printf("Warning: %s", cMsg);

	if(g_pLogFile)
		g_pLogFile->Printf(cMsg);
}

//===============================================
// @brief Prints a generic message to the console and the log
//
// @param fmt Formatted string
// @param ... Parameters for string formatting
//===============================================
void ErrorMsg( const Char *fmt, ... )
{
	// compile the string result
	va_list	vArgPtr;
	static Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	printf("Error: %s", cMsg);

	if(g_pLogFile)
		g_pLogFile->Printf(cMsg);
}

//===============================================
// @brief Loads a file for reading, going through all the $cd paths and the QC folder
// to check for the file
//
// @param pstrFilepath Path to file to load
// @param fileSize reference to variable that will hold our file's size
// @param dataPtr Pointer that'll hold our file data
// @return TRUE if file loaded successfully, FALSE otherwise
//===============================================
bool OpenFile( const Char* pstrFilepath, Uint32& fileSize, const byte*& dataPtr )
{
	if(pstrFilepath[1] == ':')
	{
		if(g_pLogFile)
			g_pLogFile->Printf("Loading file '%s'.\n", pstrFilepath);

		const byte* pFile = g_fileInterface.pfnLoadFile(pstrFilepath, &fileSize);
		if(pFile)
		{
			dataPtr = pFile;
			return true;
		}
	}

	for(Uint32 i = 0; i < g_options.dirpaths.size(); i++)
	{
		CString fullPath;
		fullPath << g_options.dirpaths[i] << pstrFilepath;

		fullPath = Common::CleanupPath(fullPath.c_str());

		if(g_pLogFile)
			g_pLogFile->Printf("Loading file '%s'.\n", fullPath.c_str());

		const byte* pFile = g_fileInterface.pfnLoadFile(fullPath.c_str(), &fileSize);
		if(pFile)
		{
			dataPtr = pFile;
			return true;
		}
	}

	if(g_pLogFile)
		g_pLogFile->Printf("Failed to load file '%s'.\n", pstrFilepath);

	dataPtr = nullptr;
	fileSize = 0;

	return false;
}

//===============================================
// @brief Releases a file that we loaded from memory
//
// @param dataPtr Pointer to file data to release
//===============================================
void FreeFile( const byte* dataPtr )
{
	if(!dataPtr)
		return;
	
	g_fileInterface.pfnFreeFile(dataPtr);
	dataPtr = nullptr;
}

//===============================================
// @brief Performs functions for closing the applocation
//
// @param forceKeyInput Force whether key input is required to exit
//===============================================
void OnExitApplication( bool forceKeyInput )
{
	if(g_pLogFile)
	{
		if(!g_pLogFile->Close())
			printf("Failed to close log file.\n");

		delete g_pLogFile;
		g_pLogFile = nullptr;
	}

	if(g_pVBMCompiler)
	{
		delete g_pVBMCompiler;
		g_pVBMCompiler = nullptr;
	}

	if(g_pStudioModelCompiler)
	{
		delete g_pStudioModelCompiler;
		g_pStudioModelCompiler = nullptr;
	}

	if(g_pQCParser)
	{
		delete g_pQCParser;
		g_pQCParser = nullptr;
	}

	if(forceKeyInput || g_options.isFlagSet(CMP_FL_WAIT_FOR_KEY))
	{
		printf("Press any key to exit...\n");
		getchar();
	}
}

//===============================================
// _tmain Main function
//
// @param argc Number of arguments
// @param argv Array of command arguments
// @return Exit code
//===============================================
int _tmain(Int32 argc, _TCHAR* argv[])
{
	// Check for usage
	if(argc < 2)
	{
		ApplicationUsage();
		OnExitApplication();
		return -1;
	}

	// Init options
	if(!g_options.initialize())
	{
		OnExitApplication();
		return -1;
	}

	// We need the target path first and foremost
	CString qcFilepath;
	for(Uint32 i = 1; i < argc; i++)
	{
		CString arg = argv[i];
		if(arg[0] == '-')
			continue;

		if(!qcFilepath.empty())
		{
			// Do not allow multiple QC files(for now)
			printf("Unhandled launch argument '%s' specified.\n", arg.c_str());
		}
		else
		{
			// First parameter not starting with '-' is
			// treated as being the QC file path
			qcFilepath = arg;
		}
	}

	// Ensure QC was specified
	if(qcFilepath.empty())
	{
		ErrorMsg("No .QC file was specified in launch arguments.\n");
		ApplicationUsage();
		OnExitApplication();
		return -1;
	}

	// Open log file
	CString folderPath;
	Common::GetDirectoryPath(qcFilepath.c_str(), folderPath);

	// Add as base path
	g_options.addDirPath(folderPath.c_str());

	CString basename;
	Common::Basename(qcFilepath.c_str(), basename);

	CString logFilePath;
	logFilePath << folderPath << PATH_SLASH_CHAR << basename << ".log";

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

	Msg("VBMCompiler version %d.%d.\n", MAJOR_VERSION, MINOR_VERSION);
	Msg("Build date: %s.\n", __DATE__);
	Msg("QC file path: %s.\n", qcFilepath.c_str());
	Msg("Reading launch arguments:\n");

	// Now parse each argument
	for(Uint32 i = 1; i < argc; i++)
	{
		CString arg = argv[i];
		if(arg[0] != '-')
			continue;

		CString argument(arg);
		if(!qstrcmp(argument, "-r"))
		{
			g_options.setFlag(CMP_FL_TAG_REVERSED_TRIANGLES);
			Msg(" - Reversed triangle tagging enabled.\n");
		}
		else if(!qstrcmp(argument, "-n"))
		{
			g_options.setFlag(CMP_FL_TAG_BAD_NORMALS);
			Msg(" - Bad normal tagging enabled.\n");
		}
		else if(!qstrcmp(argument, "-f"))
		{
			g_options.setFlag(CMP_FL_REVERSE_TRIANGLES);
			Msg(" - Triangle normals will be flipped.\n");
		}
		else if(!qstrcmp(argument, "-a"))
		{
			if((i + 1) >= argc)
			{
				ErrorMsg("Expected value after parameter '%s'.\n", argument.c_str());
				OnExitApplication();
				return -1;
			}

			i++;
			CString argparam = argv[i];
			if(argparam[0] == '-' || !Common::IsNumber(argparam.c_str()))
			{
				ErrorMsg("Expected numerical value for parameter '%s', got '%s' instead.\n", argument.c_str(), argparam.c_str());
				OnExitApplication();
				return -1;
			}

			Float value = SDL_atof(argparam.c_str());
			value = SDL_cos(value * (M_PI / 180.0));
			g_options.normal_merge_treshold = value;

			Msg(" - Normal blend angle set at '%f'\n", value);
		}
		else if(!qstrcmp(argument, "-p"))
		{
			if((i + 1) >= argc)
			{
				ErrorMsg("Expected value after parameter '%s'.\n", argument.c_str());
				OnExitApplication();
				return -1;
			}

			i++;
			CString argparam = argv[i];
			if(argparam[0] == '-' || !Common::IsNumber(argparam.c_str()))
			{
				ErrorMsg("Expected numerical value for parameter '%s', got '%s' instead.\n", argument.c_str(), argparam.c_str());
				OnExitApplication();
				return -1;
			}

			g_options.border_padding = SDL_atoi(argparam.c_str());
			if(g_options.border_padding < 0)
			{
				ErrorMsg("Invalid value %d for parameter '%s'.\n", g_options.border_padding, argument.c_str());
				OnExitApplication();
			}

			Msg(" - Texture pad size set at %d.\n", g_options.max_texture_resolution);
		}
		else if(!qstrcmp(argument, "-h"))
		{
			g_options.setFlag(CMP_FL_DUMP_HITBOX_DATA);
			Msg(" - Hitbox data dumping enabled.\n");
		}
		else if(!qstrcmp(argument, "-b"))
		{
			g_options.setFlag(CMP_FL_NO_STUDIOMDL_BONE_LIMIT);
			Msg(" - Max GoldSrc bone limit overridden.\n");
			WarningMsg(" - Note: The hard limit of bones remains at %d due to 'byte' datatype being limited to 0-255 max.\n", MAX_TOTAL_BONES);
		}
		else if(!qstrcmp(argument, "-e"))
		{
			if((i + 1) >= argc)
			{
				ErrorMsg("Expected value after parameter '%s'.\n", argument.c_str());
				OnExitApplication();
				return -1;
			}

			i++;
			CString argparam = argv[i];
			if(argparam[0] == '-' || !Common::IsNumber(argparam.c_str()))
			{
				ErrorMsg("Expected numerical value for parameter '%s', got '%s' instead.\n", argument.c_str(), argparam.c_str());
				OnExitApplication();
				return -1;
			}

			g_options.max_texture_resolution = SDL_atoi(argparam.c_str());
			if(g_options.max_texture_resolution < MIN_TEXTURE_RESOLUTION)
			{
				WarningMsg("Invalid value %d specified for '%s', minimum is %d.\n", argparam.c_str(), argument.c_str(), MIN_TEXTURE_RESOLUTION);
				g_options.max_texture_resolution = MIN_TEXTURE_RESOLUTION;
			}

			Msg(" - Maximum texture resolution set at %d.\n", g_options.max_texture_resolution);
		}
		else if(!qstrcmp(argument, "-s"))
		{
			g_options.setFlag(CMP_FL_STRIP_STUDIO_TRI_DATA|CMP_FL_STRIP_MDL_TEXTURES);
			Msg(" - Half-Life 1 MDL geometry and texture data will be stripped.\n");
		}
		else if(!qstrcmp(argument, "-q"))
		{
			g_options.setFlag(CMP_FL_STRIP_MDL_TEXTURES);
			Msg(" - Half-Life 1 MDL texture data will be stripped.\n");
		}
		else if(!qstrcmp(argument, "-l"))
		{
			g_options.setFlag(CMP_FL_NO_STUDIOMDL_VERT_LIMIT);
			Msg(" - Half-Life 1 MDL vertex/normal limit overridden.\n");
		}
		else if(!qstrcmp(argument, "-d"))
		{
			g_options.setFlag(CMP_FL_DISABLE_VBM_GENERATION);
			Msg(" - Pathos VBM model file will not be generated.\n");
		}
		else if(!qstrcmp(argument, "-w"))
		{
			g_options.setFlag(CMP_FL_WAIT_FOR_KEY);
			Msg(" - Application will only exit on key input.\n");
		}
		else if(!qstrcmp(argument, "-t"))
		{
			if((i + 2) >= argc)
			{
				ErrorMsg("Expected two strings after '%s'.\n", argument.c_str());
				OnExitApplication();
				return -1;
			}

			i++;
			CString original = argv[i];
			if(original[0] == '-')
			{
				ErrorMsg("Expected original texture name after '%s', got '%s' instead.\n", argument.c_str(), original.c_str());
				OnExitApplication();
				return false;
			}

			i++;
			CString replacement = argv[i];
			if(replacement[0] == '-')
			{
				ErrorMsg("Expected replacement texture name after '%s', got '%s' instead.\n", argument.c_str(), original.c_str());
				OnExitApplication();
				return false;
			}

			g_options.addRenamedTexture(original.c_str(), replacement.c_str());
		}
		else if(!qstrcmp(argument, "-m"))
		{
			g_options.setFlag(CMP_FL_MCD_NO_NEIGHBOR_CHECK);
			Msg(" - Closed collision mesh enforcement disabled.\n");
		}
		else
		{
			WarningMsg("Unknown parameter '%s' specified.\n", argument.c_str());
		}
	}

	// Initialize studio compiler class
	g_pStudioModelCompiler = new CStudioModelCompiler();
	if(!g_pStudioModelCompiler->Init(argv[0]))
	{
		ErrorMsg("Failure encountered when initializing StudioModel Compiler.\n");
		OnExitApplication();
		return -1;
	}

	// Construct QC file path(this ensures adding '.qc' is always valid
	CString qcFilePath;
	qcFilePath << folderPath << PATH_SLASH_CHAR << basename << ".qc";

	// Create VBM compiler unless specified not to create one
	if(!g_options.isFlagSet(CMP_FL_DISABLE_VBM_GENERATION))
		g_pVBMCompiler = new CVBMCompiler((*g_pStudioModelCompiler));

	// Load in the QC script
	g_pQCParser = new CQCParser((*g_pStudioModelCompiler), g_pVBMCompiler, argv[0]);
	if(!g_pQCParser->ProcessFile(qcFilePath.c_str()))
	{
		ErrorMsg("Error while parsing QC file '%s'.\n", qcFilePath.c_str());
		OnExitApplication();
		return -1;
	}

	if(g_options.outputname.empty())
	{
		ErrorMsg("No output name specified.\n", qcFilePath.c_str());
		OnExitApplication();
		return -1;
	}

	// Now compile the studiomdl
	if(!g_pStudioModelCompiler->ProcessInputData())
	{
		ErrorMsg("Error encountered while processing inputs for MDL.'\n");
		OnExitApplication();
		return -1;
	}

	// Create the MDL output
	if(!g_pStudioModelCompiler->WriteMDLFile())
	{
		ErrorMsg("Error encountered while compiling MDL file.'\n");
		OnExitApplication();
		return -1;
	}

	// Create VBM output if not disabled
	if(g_pVBMCompiler && !g_pVBMCompiler->CreateVBMFile())
	{
		ErrorMsg("Error encountered while compiling VBM file.'\n");
		OnExitApplication();
		return -1;
	}

	// See if we have any collision data to compile
	if(g_pStudioModelCompiler->HasCollisionMeshes())
	{
		// Pointer to MCD compiler instance
		CMCDCompiler mcdCompiler((*g_pStudioModelCompiler));
		if(!mcdCompiler.CreateMCDFile())
		{
			ErrorMsg("Error encountered while compiling VBM file.'\n");
			OnExitApplication();
			return -1;
		}
	}

	// Clear this AFTER creating the VBM file
	g_pStudioModelCompiler->Clear();

	// Close log file
	if(g_pLogFile)
	{
		g_pLogFile->Printf("Closing log");
		if(!g_pLogFile->Close())
			printf("Failed to close log file.\n");

		delete g_pLogFile;
		g_pLogFile = nullptr;
	}

	// Exit application
	OnExitApplication();
	return 0;
}
