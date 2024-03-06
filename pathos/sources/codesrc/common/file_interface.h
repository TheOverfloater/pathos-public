/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FILE_INTERFACE_H
#define FILE_INTERFACE_H

struct file_interface_t
{
	const byte*		(*pfnLoadFile)( const Char* pstrpath, Uint32* psize );
	bool			(*pfnWriteFile)( const byte* pdata, Uint32 size, const Char* pstrpath, bool append );
	bool			(*pfnWriteLogFile)( const byte* pdata, Uint32 size, const Char* pstrpath, bool append );
	void			(*pfnFreeFile)( const void* pfile );
	bool			(*pfnFileExists)( const Char* pstrpath );
	bool			(*pfnDeleteFile)( const Char* pstrpath );
	bool			(*pfnCreateDirectory)( const Char* pstrpath );
	bool			(*pfnGetFileDate)( const Char* pstrFile, file_dateinfo_t& dateinfo );
	Int32			(*pfnCompareFileDates)( const file_dateinfo_t& d1, const file_dateinfo_t& d2 );
};

#endif //FILE_INTERFACE_H