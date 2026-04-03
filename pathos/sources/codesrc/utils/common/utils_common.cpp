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
#include "utils_common.h"
#include "logfile.h"

// Log file ptr
CLogFile* g_pLogFile = nullptr;

// Size of buffer for message prints
static const Uint32 PRINT_MSG_BUFFER_SIZE = 16384;

//===============================================
// @brief Prints a generic message to the console and the log
//
// @param fmt Formatted string
// @param ... Parameters for string formatting
//===============================================
void Msg( const Char *fmt, ... )
{
	// compile the string result
	va_list	vArgPtr;
	static Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	printf(cMsg);

	if(g_pLogFile)
		g_pLogFile->Printf(cMsg);
}

//===============================================
// @brief Prints a generic message to the console and the log
//
// @param fmt Formatted string
// @param ... Parameters for string formatting
//===============================================
void WarningMsg( const Char *fmt, ... )
{
	// compile the string result
	va_list	vArgPtr;
	static Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	printf("Warning: %s", cMsg);

	if(g_pLogFile)
		g_pLogFile->Printf(cMsg);
}

//===============================================
// @brief Prints a generic message to the console and the log
//
// @param fmt Formatted string
// @param ... Parameters for string formatting
//===============================================
void ErrorMsg( const Char *fmt, ... )
{
	// compile the string result
	va_list	vArgPtr;
	static Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	printf("Error: %s", cMsg);

	if(g_pLogFile)
		g_pLogFile->Printf(cMsg);
}

//===============================================
// 
//
//===============================================
bool DirectoryExists( const Char* dirPath )
{
	DWORD ftyp = GetFileAttributesA(dirPath);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}