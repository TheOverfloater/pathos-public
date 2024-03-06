//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           Util.cpp
// last modified:  August 28th 2021, Andrew Lucas
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.2
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//

#include <mx/mx.h>

#include "mdlviewer.h"
#include "file.h"
#include "viewerstate.h"

//=============================================
// @brief Loads in data from a file
//
// @param pstr Pointer to string
//=============================================
const byte* FL_LoadFile( const Char* pstrpath, Uint32* psize )
{
	SDL_RWops* pf = SDL_RWFromFile(pstrpath, "rb");
	if(!pf)
	{
		SDL_ClearError();
		return nullptr;
	}

	SDL_RWseek(pf, 0, RW_SEEK_END);
	Int32 size = (Int32)SDL_RWtell(pf);
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

	return pbuffer;
}

//=============================================
// @brief Loads in data from a file using the game directory path
//
// @param pstr Pointer to string
//=============================================
const byte* FL_LoadFile_GameDirectory( const Char* pstrpath, Uint32* psize )
{
	CString path;
	if(qstrstr(pstrpath, ":") == nullptr)
		path << vs.moddir_path << PATH_SLASH_CHAR << pstrpath;
	else
		path = pstrpath;

	return FL_LoadFile(path.c_str(), psize);
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
// @brief Writes data to a specified file
//
// @param pstr Pointer to string
//=============================================
bool FL_WriteFile( const byte* pdata, Uint32 size, const Char* pstrpath, bool append )
{
	SDL_RWops* pf = SDL_RWFromFile(pstrpath, append ? "ab" : "wb");
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
bool FL_WriteFile_GameDirectory( const byte* pdata, Uint32 size, const Char* pstrpath, bool append )
{
	CString path;
	if(qstrstr(pstrpath, ":") == nullptr)
		path << vs.moddir_path << PATH_SLASH_CHAR << pstrpath;
	else
		path = pstrpath;

	return FL_WriteFile(pdata, size, path.c_str(), append);
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
	WIN32_FIND_DATAA findData;
	HANDLE hFile = FindFirstFileA(pstrFile, &findData);
	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	SYSTEMTIME sysTime;
	if(!FileTimeToSystemTime(&findData.ftLastWriteTime, &sysTime))
	{
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
bool FL_GetFileDate_GameDirectory( const Char* pstrFile, file_dateinfo_t& dateinfo )
{
	CString filepath;
	if(qstrstr(pstrFile, ":") == nullptr)
		filepath << vs.moddir_path << PATH_SLASH_CHAR << pstrFile;
	else
		filepath = pstrFile;

	return FL_GetFileDate(filepath.c_str(), dateinfo);
}

//=============================================
// @brief Used when rendering dynamic lights
//
// @param pitch Pitch value
// @return true or false
//=============================================
bool FL_CreateDirectory( const Char* pstrpath )
{
	DWORD type = GetFileAttributesA(pstrpath);
	if(type == INVALID_FILE_ATTRIBUTES)
		return CreateDirectoryA(pstrpath, nullptr) ? true : false;

	// Check if it's a directory
	if(type & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	return false;
}

//=============================================
// @brief Used when rendering dynamic lights
//
// @param pitch Pitch value
// @return true or false
//=============================================
bool FL_CreateDirectory_GameDirectory( const Char* pstrpath )
{
	CString filepath;
	if(qstrstr(pstrpath, ":") == nullptr)
		filepath << vs.moddir_path << PATH_SLASH_CHAR << pstrpath;
	else
		filepath = pstrpath;

	return FL_CreateDirectory(filepath.c_str());
}

//=============================================
// @brief Used when rendering dynamic lights
//
// @param pitch Pitch value
// @return true or false
//=============================================
bool FL_DeleteFile( const Char* pstrpath )
{
	return true;
}

//=============================================
// @brief Used when rendering dynamic lights
//
// @param pitch Pitch value
// @return true or false
//=============================================
bool FL_DeleteFile_GameDirectory( const Char* pstrpath )
{
	return true;
}

//=============================================
// @brief Checks if a file exists at the given path
//
//=============================================
bool FL_FileExists( const Char* pstrpath )
{
	SDL_RWops* pf = SDL_RWFromFile(pstrpath, "rb");
	if(!pf)
		return false;

	SDL_RWclose(pf);
	return true;
}

//=============================================
// @brief Checks if a file exists at the given path
//
//=============================================
bool FL_FileExists_GameDirectory( const Char* pstrpath )
{
	CString filepath;
	if(qstrstr(pstrpath, ":") == nullptr)
		filepath << vs.moddir_path << PATH_SLASH_CHAR << pstrpath;
	else
		filepath = pstrpath;

	return FL_FileExists(filepath.c_str());
}