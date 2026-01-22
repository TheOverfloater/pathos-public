/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FILE_H
#define FILE_H

#include <map>

/*
=================================
CFileCache

=================================
*/
class CFileCache
{
private:
	struct file_t
	{
		file_t():
			pfile(nullptr),
			filesize(0),
			refcount(0)
		{
		}
		~file_t()
		{
			if(pfile)
				delete[] pfile;
		}

		const byte* pfile;
		Uint32 filesize;
		Uint32 refcount;
	};

	typedef std::map<CString, file_t*> FileCacheMap_t;
	typedef std::map<const byte*, FileCacheMap_t::iterator> FilePtrCacheIteratorMap_t;

public:
	CFileCache( void );
	~CFileCache( void );

public:
	// Loads a file into memory
	const byte* LoadFile( const Char* pstrpath, Uint32* psize = nullptr );
	// Reduces refcount on a file, and removes it is refcount is zero
	bool FreeFile( const void* pfile );
	// Dumps open files list
	void Dump( void );

private:
	// Map containing files loaded currently
	FileCacheMap_t m_fileCacheMap;
	// Map of file pointer->cache map iterator mappings
	FilePtrCacheIteratorMap_t m_filePtrCacheIteratorMap;
};
extern CFileCache gFileCache;

extern const byte* FL_LoadFile( const Char* pstrpath, Uint32* psize = nullptr );
extern const byte* FL_LoadFileFromRoot( const Char* pstrpath, Uint32* psize = nullptr );
extern bool FL_WriteFile( const byte* pdata, Uint32 size, const Char* pstrpath, bool append = false );
extern bool FL_WriteLogFile( const byte* pdata, Uint32 size, const Char* pstrpath, bool append = false );
extern bool FL_WriteFileRoot( const byte* pdata, Uint32 size, const Char* pstrpath, bool append = false );
extern void FL_FreeFile( const void* pfile );
extern bool FL_FileExists( const Char* pstrpath );
extern bool FL_DeleteFile( const Char* pstrpath );
extern bool FL_DeleteFileRoot( const Char* pstrpath );
extern bool FL_CreateDirectory( const Char* pstrpath );
extern bool FL_GetFileDate( const Char* pstrFile, file_dateinfo_t& dateinfo );
extern Int32 FL_CompareFileDates( const file_dateinfo_t& d1, const file_dateinfo_t& d2 );
extern const Char* FL_GetGameDirectory( void );
extern struct file_interface_t& FL_GetInterface( void );
#endif //FILE_H