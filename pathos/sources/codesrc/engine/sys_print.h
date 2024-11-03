/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SYS_PRINT_H
#define SYS_PRINT_H

#include <mutex>
#include <set>

#include "md5.h"

// Size of buffer for message prints
static constexpr Uint32 PRINT_MSG_BUFFER_SIZE = 16384;

/*
====================
CSysPrintInterface

====================
*/
class CSysPrintInterface
{
public:
	// Flags for a print msg
	enum printflags_t
	{
		PRINT_FL_NONE			= 0,
		PRINT_FL_ONLYONCE		= (1<<0),	// Only once per application instance
		PRINT_FL_ONLYONCE_GAME	= (1<<1)	// Only once per game instance
	};

public:
	CSysPrintInterface( void );
	~CSysPrintInterface( void );

public:
	// Clear class on cleanup
	void Clear( void );
	// Clear game specific stuff
	void ClearGame( void );

public:
	// Standard print to console
	void Printf( const Char *pstring );
	// Print to debug
	void DPrintf( const Char *pstring );
	// Print verbose
	void VPrintf( const Char *pstring );
	// Print error
	void EPrintf( const Char *pstring );

private:
	// Processes any flags inside strings
	void CheckForFlags( CString& inputstring, Int32& outflags );
	// Tells if the message should be printed
	bool ShouldPrintMessage( const Char* pstrMessage, Int32 flags );

private:
	// Set of only once an instance prints
	std::set<CString> m_applicationInstancePrintsSet;
	// Set of only once a game prints
	std::set<CString> m_gameInstancePrintsSet;
	// MD5 hasher
	CMD5 m_md5Hasher;
};
extern CSysPrintInterface gPrintInterface;

// Standard print to console
extern void Con_Printf( const Char *fmt, ... );
// Print to debug
extern void Con_DPrintf( const Char *fmt, ... );
// Print verbose
extern void Con_VPrintf( const Char *fmt, ... );
// Print error
extern void Con_EPrintf( const Char *fmt, ... );
#endif //SYS_PRINT_H
