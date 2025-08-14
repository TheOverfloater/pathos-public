/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FILEFUNCS_H
#define FILEFUNCS_H

#include "file_interface.h"

extern const byte* FL_LoadFile( const Char* pstrpath, Uint32* psize );
extern void FL_FreeFile( const void* pfile );
extern bool FL_WriteFile( const byte* pdata, Uint32 size, const Char* pstrpath, bool append = false );
extern bool FL_WriteLogFile( const byte* pdata, Uint32 size, const Char* pstrpath, bool append = false );
extern Int32 FL_CompareFileDates( const file_dateinfo_t& d1, const file_dateinfo_t& d2 );
extern bool FL_GetFileDate( const Char* pstrFile, file_dateinfo_t& dateinfo );
extern bool FL_CreateDirectory( const Char* pstrpath );
extern bool FL_DeleteFile( const Char* pstrpath );
extern bool FL_FileExists( const Char* pstrpath );
extern file_interface_t g_fileInterface;
#endif //FILEFUNCS_H
