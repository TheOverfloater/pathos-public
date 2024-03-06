//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           Util.h
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

#ifndef UTIL_H
#define UTIL_H

#include "includes.h"

extern const byte* FL_LoadFile( const Char* pstrpath, Uint32* psize );
extern const byte* FL_LoadFile_GameDirectory( const Char* pstrpath, Uint32* psize );
extern void FL_FreeFile( const void* pfile );
extern bool FL_WriteFile( const byte* pdata, Uint32 size, const Char* pstrpath, bool append = false );
extern bool FL_WriteFile_GameDirectory( const byte* pdata, Uint32 size, const Char* pstrpath, bool append );
extern Int32 FL_CompareFileDates( const file_dateinfo_t& d1, const file_dateinfo_t& d2 );
extern bool FL_GetFileDate( const Char* pstrFile, file_dateinfo_t& dateinfo );
extern bool FL_GetFileDate_GameDirectory( const Char* pstrFile, file_dateinfo_t& dateinfo );
extern bool FL_CreateDirectory( const Char* pstrpath );
extern bool FL_CreateDirectory_GameDirectory( const Char* pstrpath );
extern bool FL_DeleteFile( const Char* pstrpath );
extern bool FL_DeleteFile_GameDirectory( const Char* pstrpath );
extern bool FL_FileExists( const Char* pstrpath );
extern bool FL_FileExists_GameDirectory( const Char* pstrpath );
#endif //UTIL_H