/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <SDL.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include "includes.h"
#include "file.h"
#include "system.h"
#include "enginestate.h"
#include "file_interface.h"

//
// Engine file functions
//
file_interface_t ENGINE_FILE_FUNCTIONS =
{
	FL_LoadFile,			//pfnLoadFile
	FL_WriteFile,			//pfnWriteFile
	FL_WriteLogFile,		//pfnWriteLogFile
	FL_FreeFile,			//pfnFreeFile
	FL_FileExists,			//pfnFileExists
	FL_DeleteFile,			//pfnDeleteFile
	FL_CreateDirectory,		//pfnCreateDirectory
	FL_GetFileDate,			//pfnGetFileDate
	FL_CompareFileDates		//pfnCompareFileDates
};

//=============================================
// @brief Writes data to a specified file
//
// @param pstr Pointer to string
//=============================================
bool FL_WriteFile( const byte* pdata, Uint32 size, const Char* pstrpath, bool append )
{
	CString filepath;
	filepath << ens.gamedir << PATH_SLASH_CHAR << pstrpath;

	if(ens.pfileiologfile)
	{
		CString str;
		str << "Opening file " << pstrpath << " for writing";

		if(append)
			str << " to append";

		if(ens.gamestate == GAME_RUNNING)
			str << "[GAME ACTIVE]";
		else if(ens.gamestate == GAME_LOADING)
			str << "[GAME LOADING]";
		else
			str << "[GAME INACTIVE]";

		str << "." << NEWLINE;

		ens.pfileiologfile->Write(str.c_str());
	}

	SDL_RWops* pf = SDL_RWFromFile(filepath.c_str(), append ? "ab" : "wb");
	if(!pf)
	{
		Con_EPrintf("Failed to open %s for writing: %s.\n", filepath.c_str(), SDL_GetError());
		SDL_ClearError();
		return false;
	}

	size_t numbytes = SDL_RWwrite(pf, pdata, 1, size);
	SDL_RWclose(pf);

	if(!append)
		Con_DPrintf("Wrote %d bytes for %s.\n", numbytes, pstrpath);

	return (numbytes == size) ? true : false;
}

//=============================================
// @brief Writes data to a specified file
//
// @param pstr Pointer to string
//=============================================
bool FL_WriteLogFile( const byte* pdata, Uint32 size, const Char* pstrpath, bool append )
{
	CString filepath;
	filepath << ens.gamedir << PATH_SLASH_CHAR << pstrpath;

	SDL_RWops* pf = SDL_RWFromFile(filepath.c_str(), append ? "ab" : "wb");
	if(!pf)
	{
		SDL_ClearError();
		return false;
	}

	size_t numbytes = SDL_RWwrite(pf, pdata, 1, size);
	SDL_RWclose(pf);

	return (numbytes == size) ? true : false;
}

//=============================================
// @brief Writes data to a specified file
//
// @param pstr Pointer to string
//=============================================
bool FL_WriteFileRoot( const byte* pdata, Uint32 size, const Char* pstrpath, bool append )
{
	if(ens.pfileiologfile)
	{
		CString str;
		str << "Opening file " << pstrpath << " for writing";
		if(append)
			str << " to append";

		if(ens.gamestate == GAME_RUNNING)
			str << "[GAME ACTIVE]";
		else if(ens.gamestate == GAME_LOADING)
			str << "[GAME LOADING]";
		else
			str << "[GAME INACTIVE]";

		str << "." << NEWLINE;

		ens.pfileiologfile->Write(str.c_str());
	}

	SDL_RWops* pf = SDL_RWFromFile(pstrpath, append ? "ab" : "wb");
	if(!pf)
	{
		Con_EPrintf("Failed to open %s for writing: %s.\n", pstrpath, SDL_GetError());
		SDL_ClearError();
		return false;
	}

	size_t numbytes = SDL_RWwrite(pf, pdata, 1, size);
	SDL_RWclose(pf);

	if(!append)
		Con_DPrintf("Wrote %d bytes for %s.\n", numbytes, pstrpath);
	return (numbytes == size) ? true : false;
}

//=============================================
// @brief Loads in data from a file
//
// @param pstr Pointer to string
//=============================================
const byte* FL_LoadFile( const Char* pstrpath, Uint32* psize )
{
	CString filepath;
	filepath << ens.gamedir << PATH_SLASH_CHAR << pstrpath;

	Double timeBegin = 0;
	if(ens.pfileiologfile)
	{
		timeBegin = Sys_FloatTime();

		CString str;
		str << "Opening file " << pstrpath << " for reading";

		if(ens.gamestate == GAME_RUNNING)
			str << "[GAME ACTIVE]";
		else if(ens.gamestate == GAME_LOADING)
			str << "[GAME LOADING]";
		else
			str << "[GAME INACTIVE]";

		str << "." << NEWLINE;

		ens.pfileiologfile->Write(str.c_str());
	}

	SDL_RWops* pf = SDL_RWFromFile(filepath.c_str(), "rb");
	if(!pf && qstrcmp(ens.gamedir, COMMON_GAMEDIR))
	{
		// Try loading from the base dir if it's a mod
		filepath.clear();
		filepath << COMMON_GAMEDIR << PATH_SLASH_CHAR << pstrpath;

		pf = SDL_RWFromFile(filepath.c_str(), "rb");
	}

	if(!pf)
	{
		SDL_ClearError();
		return nullptr;
	}

	SDL_RWseek(pf, 0, RW_SEEK_END);
	Int32 size = static_cast<Int32>(SDL_RWtell(pf));
	SDL_RWseek(pf, 0, RW_SEEK_SET);

	byte* pbuffer = new byte[size+1];
	size_t numbytes = SDL_RWread(pf, pbuffer, 1, size);
	SDL_RWclose(pf);

	if(numbytes != size)
	{
		delete[] pbuffer;
		pbuffer =  nullptr;
		return nullptr;
	}

	// null terminate all files
	pbuffer[size] = '\0';

	if(psize)
		*psize = size;

	if(ens.pfileiologfile)
	{
		Double timeEnd = Sys_FloatTime();
		Double duration = timeEnd - timeBegin;

		CString str;
		str << "Read file in " << static_cast<Float>(duration) << " seconds, " << size << " bytes." << NEWLINE;
		ens.pfileiologfile->Write(str.c_str());
	}

	return pbuffer;
}

//=============================================
// @brief Loads in data from a file
//
// @param pstr Pointer to string
//=============================================
const byte* FL_LoadFileFromRoot( const Char* pstrpath, Uint32* psize )
{
	Double timeBegin = 0;
	if(ens.pfileiologfile)
	{
		timeBegin = Sys_FloatTime();

		CString str;
		str << "Opening file " << pstrpath << " for reading";

		if(ens.gamestate == GAME_RUNNING)
			str << "[GAME ACTIVE]";
		else if(ens.gamestate == GAME_LOADING)
			str << "[GAME LOADING]";
		else
			str << "[GAME INACTIVE]";

		str << "." << NEWLINE;

		ens.pfileiologfile->Write(str.c_str());
	}

	SDL_RWops* pf = SDL_RWFromFile(pstrpath, "rb");
	if(!pf && qstrcmp(ens.gamedir, COMMON_GAMEDIR))
	{
		SDL_ClearError();
		return nullptr;
	}

	SDL_RWseek(pf, 0, RW_SEEK_END);
	Int32 size = static_cast<Int32>(SDL_RWtell(pf));
	SDL_RWseek(pf, 0, RW_SEEK_SET);

	byte* pbuffer = new byte[size+1];
	size_t numbytes = SDL_RWread(pf, pbuffer, 1, size);
	SDL_RWclose(pf);

	if(numbytes != size)
	{
		delete[] pbuffer;
		pbuffer =  nullptr;
		return nullptr;
	}

	// null terminate all files
	pbuffer[size] = '\0';

	if(psize)
		*psize = size;

	if(ens.pfileiologfile)
	{
		Double timeEnd = Sys_FloatTime();
		Double duration = timeEnd - timeBegin;

		CString str;
		str << "Read file in " << static_cast<Float>(duration) << " seconds, " << size << " bytes." << NEWLINE;
		ens.pfileiologfile->Write(str.c_str());
	}

	return pbuffer;
}

//=============================================
// @brief Frees memory for a file
//
// @param pf Pointer to file data in memory
//=============================================
void FL_FreeFile( const void* pfile )
{
	delete[] pfile;
}

//=============================================
// @brief Checks if a file exists at the given path
//
//=============================================
bool FL_FileExists( const Char* pstrpath )
{
	if(ens.pfileiologfile)
	{
		CString str;
		str << "Opening file " << pstrpath << " to check if it exists";

		if(ens.gamestate == GAME_RUNNING)
			str << "[GAME ACTIVE]";
		else if(ens.gamestate == GAME_LOADING)
			str << "[GAME LOADING]";
		else
			str << "[GAME INACTIVE]";

		str << "." << NEWLINE;

		ens.pfileiologfile->Write(str.c_str());
	}

	CString filepath;
	filepath << ens.gamedir << PATH_SLASH_CHAR << pstrpath;

	SDL_RWops* pf = SDL_RWFromFile(filepath.c_str(), "rb");
	if(!pf && qstrcmp(ens.gamedir, COMMON_GAMEDIR))
	{
		// Try loading from the base dir if it's a mod
		filepath.clear();
		filepath << COMMON_GAMEDIR << PATH_SLASH_CHAR << pstrpath;

		pf = SDL_RWFromFile(filepath.c_str(), "rb");
	}

	if(!pf)
		return false;

	SDL_RWclose(pf);
	return true;
}

//=============================================
// @brief Checks if a file exists at the given path
//
//=============================================
bool FL_DeleteFile( const Char* pstrpath )
{
	if(ens.pfileiologfile)
	{
		CString str;
		str << "Deleting file " << pstrpath;

		if(ens.gamestate == GAME_RUNNING)
			str << "[GAME ACTIVE]";
		else if(ens.gamestate == GAME_LOADING)
			str << "[GAME LOADING]";
		else
			str << "[GAME INACTIVE]";

		str << "." << NEWLINE;

		ens.pfileiologfile->Write(str.c_str());
	}

	if(!pstrpath || !qstrlen(pstrpath))
		return false;

	CString filepath;
	filepath << ens.gamedir << PATH_SLASH_CHAR << pstrpath;

	if(remove(filepath.c_str()))
	{
		Con_EPrintf("%s - Could not delete '%s'.\n", filepath.c_str());
		return false;
	}

	return true;
}

//=============================================
// @brief Checks if a file exists at the given path
//
//=============================================
bool FL_DeleteFileRoot( const Char* pstrpath )
{
	if(ens.pfileiologfile)
	{
		CString str;
		str << "Deleting file " << pstrpath;

		if(ens.gamestate == GAME_RUNNING)
			str << "[GAME ACTIVE]";
		else if(ens.gamestate == GAME_LOADING)
			str << "[GAME LOADING]";
		else
			str << "[GAME INACTIVE]";

		str << "." << NEWLINE;

		ens.pfileiologfile->Write(str.c_str());
	}

	if(remove(pstrpath))
	{
		Con_EPrintf("%s - Could not delete '%s'.\n", pstrpath);
		return false;
	}

	return true;
}

//=============================================
// @brief Used when rendering dynamic lights
//
// @param pitch Pitch value
// @return true or false
//=============================================
bool FL_CreateDirectory( const Char* pstrpath )
{
	if(ens.pfileiologfile)
	{
		CString str;
		str << "Creating directory " << pstrpath;

		if(ens.gamestate == GAME_RUNNING)
			str << "[GAME ACTIVE]";
		else if(ens.gamestate == GAME_LOADING)
			str << "[GAME LOADING]";
		else
			str << "[GAME INACTIVE]";

		str << "." << NEWLINE;

		ens.pfileiologfile->Write(str.c_str());
	}

	CString filepath;
	filepath << ens.gamedir << PATH_SLASH_CHAR << pstrpath;

	DWORD type = GetFileAttributesA(filepath.c_str());
	if(type == INVALID_FILE_ATTRIBUTES)
		return CreateDirectoryA(filepath.c_str(), nullptr) ? true : false;

	// Check if it's a directory
	if(type & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	return false;
}

//=============================================
// @brief Makes slashes uniform in a path string
//
// @param pstring String to format
// @return Formatted string
//=============================================
Int32 FL_CompareFileDates( const file_dateinfo_t& d1, const file_dateinfo_t& d2 )
{
	if(d1.year == d2.year
		&& d1.month == d2.month
		&& d1.day == d2.day
		&& d1.hour == d2.hour
		&& d1.minute == d2.minute
		&& d1.second == d2.second)
		return 0;

	if(d1.year < d2.year)
		return 1;

	if(d1.year == d2.year 
		&& d1.month < d2.month)
		return 1;

	if(d1.year == d2.year
		&& d1.month == d2.month
		&& d1.day < d2.day)
		return 1;

	if(d1.year == d2.year
		&& d1.month == d2.month
		&& d1.day == d2.day
		&& d1.hour < d2.hour)
		return 1;

	if(d1.year == d2.year
		&& d1.month == d2.month
		&& d1.day == d2.day
		&& d1.hour == d2.hour
		&& d1.minute < d2.minute)
		return 1;

	if(d1.year == d2.year
		&& d1.month == d2.month
		&& d1.day == d2.day
		&& d1.hour == d2.hour
		&& d1.minute == d2.minute
		&& d1.second < d2.second)
		return 1;

	return -1;
}

//=============================================
// @brief Checks visibility on a set of leaf numbers
//
//=============================================
bool FL_GetFileDate( const Char* pstrFile, file_dateinfo_t& dateinfo )
{
	CString filepath;
	filepath << ens.gamedir << PATH_SLASH_CHAR << pstrFile;

	if(ens.pfileiologfile)
	{
		CString str;
		str << "Opening file " << filepath << " to get file date";

		if(ens.gamestate == GAME_RUNNING)
			str << "[GAME ACTIVE]";
		else if(ens.gamestate == GAME_LOADING)
			str << "[GAME LOADING]";
		else
			str << "[GAME INACTIVE]";

		str << "." << NEWLINE;

		ens.pfileiologfile->Write(str.c_str());
	}

	WIN32_FIND_DATAA findData;
	HANDLE hFile = FindFirstFileA(filepath.c_str(), &findData);
	if(hFile == INVALID_HANDLE_VALUE && qstrcmp(ens.gamedir, COMMON_GAMEDIR))
	{
		// Try loading from the base dir if it's a mod
		filepath.clear();
		filepath << COMMON_GAMEDIR << PATH_SLASH_CHAR << pstrFile;

		hFile = FindFirstFileA(filepath.c_str(), &findData);
	}

	if(hFile == INVALID_HANDLE_VALUE)
	{
		Con_DPrintf("%s - Failed to open '%s' for reading.\n", __FUNCTION__, filepath.c_str());
		return false;
	}

	SYSTEMTIME sysTime;
	if(!FileTimeToSystemTime(&findData.ftLastWriteTime, &sysTime))
	{
		Con_Printf("%s - Failed to get file write date for '%s'.\n", __FUNCTION__, filepath.c_str());
		FindClose(hFile);
		return false;
	}

	dateinfo.year = sysTime.wYear;
	dateinfo.month = sysTime.wMonth;
	dateinfo.day = sysTime.wDay;
	dateinfo.hour = sysTime.wHour;
	dateinfo.minute = sysTime.wMinute;
	dateinfo.second = sysTime.wSecond;

	FindClose(hFile);
	return true;
}

//=============================================
// @brief Checks visibility on a set of leaf numbers
//
//=============================================
file_interface_t& FL_GetInterface( void )
{
	return ENGINE_FILE_FUNCTIONS;
}