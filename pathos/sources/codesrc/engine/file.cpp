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

// Declaration of object
CFileCache gFileCache;

//=============================================
// @brief Constructor
//
//=============================================
CFileCache::CFileCache( void )
{
}

//=============================================
// @brief Destructor
//
//=============================================
CFileCache::~CFileCache( void )
{
	if(m_fileCacheMap.empty())
		return;

	FileCacheMap_t::iterator it = m_fileCacheMap.begin();
	while(it != m_fileCacheMap.end())
	{
		delete it->second;
		it++;
	}
}

//=============================================
// @brief Loads a file into memory
//
//=============================================
const byte* CFileCache::LoadFile( const Char* pstrpath, Uint32* psize )
{
	FileCacheMap_t::iterator it = m_fileCacheMap.find(pstrpath);
	if(it != m_fileCacheMap.end())
	{
		it->second->refcount++;
		return it->second->pfile;
	}
	else
	{
		SDL_RWops* pf = SDL_RWFromFile(pstrpath, "rb");
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

		file_t* pnew = new file_t;
		pnew->pfile = pbuffer;
		pnew->filesize = size;
		pnew->refcount = 1;

		if(psize)
			*psize = size;

		it = m_fileCacheMap.insert(std::pair<CString, file_t*>(pstrpath, pnew)).first;
		std::pair<FilePtrCacheIteratorMap_t::iterator, bool> result = m_filePtrCacheIteratorMap.insert(std::pair<const byte*, FileCacheMap_t::iterator>(pnew->pfile, it));
		if(!result.second)
			Con_EPrintf("%s - Iterator for '%s' already inside file pointer->cache iterator map.\n", __FUNCTION__, pstrpath);

		return pnew->pfile;
	}
}

//=============================================
// @brief Reduces refcount on a file, and removes 
// it is refcount is zero
//
//=============================================
bool CFileCache::FreeFile( const void* pfile )
{
	const byte* pfileptr = reinterpret_cast<const byte*>(pfile);
	FilePtrCacheIteratorMap_t::iterator it = m_filePtrCacheIteratorMap.find(pfileptr);
	if(it == m_filePtrCacheIteratorMap.end())
	{
		Con_Printf("%s - A file pointer was not found inside the cache.\n", __FUNCTION__);
		return false;
	}

	// Safeguard against error
	FileCacheMap_t::iterator itCache = it->second;
	if(itCache->second->refcount > 0)
		itCache->second->refcount--;

	if(itCache->second->refcount <= 0)
	{
		delete itCache->second;
		m_fileCacheMap.erase(itCache);
		m_filePtrCacheIteratorMap.erase(it);
		return true;
	}
	else
	{
		return false;
	}
}

//=============================================
// @brief Dumps open files list
//
//=============================================
void CFileCache::Dump( void )
{
	if(m_fileCacheMap.empty())
	{
		Con_Printf("File cache is empty.\n");
		return;
	}

	Con_Printf("Number of files loaded: %d.\n", m_fileCacheMap.size());

	Uint32 index = 0;
	FileCacheMap_t::iterator it = m_fileCacheMap.begin();
	while(it != m_fileCacheMap.end())
	{
		file_t* pfile = it->second;
		Float sizemb = static_cast<Float>(pfile->filesize) / 1024.0f * 1024.0f;

		Con_Printf("\t%d - File '%s', size: %.2f mb, reference count: %d.\n", index, it->first.c_str(), sizemb, pfile->refcount);
		index++;
		it++;
	}
}

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
	FL_CompareFileDates,	//pfnCompareFileDates
	FL_GetGameDirectory,	//pfnGetGameDirectory
};

//=============================================
// @brief Writes data to a specified file
//
// @param pdata Pointer to data block to be written
// @param size Size of data to be written
// @param pstrpath Path to write the file to
// @param append If true, the data will be appeneded to a file with this name if it already exists
// @return TRUE if write was successful, FALSE otherwise
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
// @brief Writes data to a specified log file
//
// @param pdata Pointer to data block to be written
// @param size Size of data to be written
// @param pstrpath Path to write the file to
// @param append If true, the data will be appeneded to a file with this name if it already exists
// @return TRUE if write was successful, FALSE otherwise
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
// @brief Writes data to a specified file starting from the root folder
//
// @param pdata Pointer to data block to be written
// @param size Size of data to be written
// @param pstrpath Path to write the file to
// @param append If true, the data will be appeneded to a file with this name if it already exists
// @return TRUE if write was successful, FALSE otherwise
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
// @param pstrpath Path to file to load
// @param psize Pointer to Uint32 to store the size of the file
// @return Pointer to file data if successfully loaded, nullptr otherwise
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

	Uint32 filesize = 0;
	const byte* pfile = gFileCache.LoadFile(filepath.c_str(), &filesize);
	if(!pfile && qstrcmp(ens.gamedir, COMMON_GAMEDIR))
	{
		// Try loading from the base dir if it's a mod
		filepath.clear();
		filepath << COMMON_GAMEDIR << PATH_SLASH_CHAR << pstrpath;

		pfile = gFileCache.LoadFile(filepath.c_str(), &filesize);
	}

	if(!pfile)
	{
		SDL_ClearError();
		return nullptr;
	}

	if(ens.pfileiologfile)
	{
		Double timeEnd = Sys_FloatTime();
		Double duration = timeEnd - timeBegin;

		CString str;
		str << "Read file in " << static_cast<Float>(duration) << " seconds, " << filesize << " bytes." << NEWLINE;
		ens.pfileiologfile->Write(str.c_str());
	}

	if(psize)
		(*psize) = filesize;

	return pfile;
}

//=============================================
// @brief Loads in data from a file starting from the root folder
//
// @param pstrpath Path to file to load
// @param psize Pointer to Uint32 to store the size of the file
// @return Pointer to file data if successfully loaded, nullptr otherwis
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
	
	Uint32 filesize = 0;
	const byte* pfile = gFileCache.LoadFile(pstrpath, &filesize);
	if(!pfile)
	{
		SDL_ClearError();
		return nullptr;
	}

	if(ens.pfileiologfile)
	{
		Double timeEnd = Sys_FloatTime();
		Double duration = timeEnd - timeBegin;

		CString str;
		str << "Read file in " << static_cast<Float>(duration) << " seconds, " << filesize << " bytes." << NEWLINE;
		ens.pfileiologfile->Write(str.c_str());
	}

	if(psize)
		(*psize) = filesize;

	return pfile;
}

//=============================================
// @brief Frees memory allocated for a file that was loaded
//
// @param pf Pointer to file data in memory
//=============================================
void FL_FreeFile( const void* pfile )
{
	gFileCache.FreeFile(pfile);
}

//=============================================
// @brief Checks if a file exists at the given path
//
// @param pstrpath File path to check
// @return TRUE if file exists, FALSE otherwise
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
// @brief Deletes a directory path starting from the mod folder
//
// @return TRUE if successful, FALSE otherwise
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
// @brief Deletes a directory path starting from the root folder
//
// @return TRUE if successful, FALSE otherwise
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
// @brief Creates a directory path recirsively
//
// @param pstrpath The directory path to create
// @return TRUE if successful, FALSE otherwise
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
	{
		CString createpath;

		// Process it per slash
		const Char* pstr = filepath.c_str();
		const Char* pstrnextbegin = pstr;
		while(*pstr)
		{
			pstr++;

			if((*pstr) == PATH_SLASH_CHAR || (*pstr) == '\\' || (*pstr) == '\0')
			{
				Uint32 length = (pstr - pstrnextbegin);
				CString foldername(pstrnextbegin, length);

				createpath << foldername << PATH_SLASH_CHAR;
				type = GetFileAttributesA(createpath.c_str());
				if(type == INVALID_FILE_ATTRIBUTES)
				{
					if(!CreateDirectoryA(createpath.c_str(), nullptr))
						return false;
				}
				else if(!(type & FILE_ATTRIBUTE_DIRECTORY))
				{
					return false;
				}

				while((*pstr) && ((*pstr) == PATH_SLASH_CHAR || (*pstr) == '\\'))
					pstr++;

				pstrnextbegin = pstr;
			}
		}

		return true;
	}
	else
	{
		// Check if it's a directory
		if(type & FILE_ATTRIBUTE_DIRECTORY)
			return true;
		else
			return false;
	}
}

//=============================================
// @brief Compares file dates
//
// @param d1 File 1 date info reference
// @param d1 File 2 date info reference
// @return Result of comparison
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
// @brief Return the modification date of a file
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
// @brief Returns the game directory name
//
//=============================================
const Char* FL_GetGameDirectory( void )
{
	return ens.gamedir.c_str();
}

//=============================================
// @brief Returns a pointer to the file interface function table
//
//=============================================
file_interface_t& FL_GetInterface( void )
{
	return ENGINE_FILE_FUNCTIONS;
}