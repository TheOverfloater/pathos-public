/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CLOGFILE_H
#define CLOGFILE_H

#include "file_interface.h"

/*
=======================
CLogFile

=======================
*/
class CLogFile
{
public:
	typedef void (*pfnConPrintf_t)( const Char *fmt, ... );

public:
	// Buffer size
	static const Uint32 LOG_BUFFER_SIZE;
	// Size of buffer for message prints
	static const Uint32 PRINT_MSG_BUFFER_SIZE;

public:
	explicit CLogFile( const Char* pstrPath, pfnConPrintf_t pfnConPrintf, file_interface_t& fileInterface, bool deletePrevious = false, bool timeStamps = true );
	~CLogFile();

public:
	bool Init( void );
	bool Close( void );

	bool Write( const Char* pstrString );
	bool Printf( const Char *fmt, ... );

private:
	bool DumpBuffer( void );
	bool WriteInternal( const Char* pstrString );

private:
	// Path to log file
	CString m_sLogPath;
	// Number of lines written
	Uint32 m_nbLinesWritten;
	// TRUE if timestamps should be used
	bool m_useTimeStamps;
	// TRUE if previous log should be deleted
	bool m_deletePreviousLog;

	// Log buffer
	Char* m_pBuffer;
	// Log buffer load
	Uint32 m_logBufferLoad;
	// Protective semaphore
	bool m_writeSemaphore;

	// Pointer to Con_Printf function
	pfnConPrintf_t m_pfnConPrintf;
	// File interface fns
	file_interface_t m_fileInterface;
};
#endif //CLOGFILE_H