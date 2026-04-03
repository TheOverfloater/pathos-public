/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.

===============================================
*/

#ifndef MAIN_H
#define MAIN_H
extern bool CreateDirectory( const Char* dirPath );

extern bool OpenFile( const Char* pstrFilepath, Uint32& fileSize, const byte*& dataPtr );
extern void FreeFile( const byte* dataPtr );
#endif