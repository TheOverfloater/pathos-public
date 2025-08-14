/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MAIN_H
#define MAIN_H

// Log file ptr
class CLogFile;

extern CLogFile* g_pLogFile;

extern void Msg( const Char *fmt, ... );
extern void WarningMsg( const Char *fmt, ... );
extern void ErrorMsg( const Char *fmt, ... );
extern bool OpenFile( const Char* pstrFilepath, Uint32& fileSize, const byte*& dataPtr );
extern void FreeFile( const byte* dataPtr );
extern void OnExitApplication( bool forceKeyInput = false );
#endif //MAIN_H