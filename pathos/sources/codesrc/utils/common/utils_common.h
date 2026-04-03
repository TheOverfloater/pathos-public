/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef UTILS_COMMON_H
#define UTILS_COMMON_H
// Log file ptr
class CLogFile;

extern CLogFile* g_pLogFile;

extern void Msg( const Char *fmt, ... );
extern void WarningMsg( const Char *fmt, ... );
extern void ErrorMsg( const Char *fmt, ... );
extern bool DirectoryExists( const Char* dirPath );
#endif //UTILS_COMMON_H