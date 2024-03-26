/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SYSTEM_H
#define SYSTEM_H

class CCVar;

// Size of buffer for message prints
static const Uint32 PRINT_MSG_BUFFER_SIZE = 16384;

// Gameinfo file name
static const Char GAMEINFO_FILENAME[] = "gameinfo.cfg";

// Timescale cvar
extern CCVar* g_pCvarTimeScale;

/*
=================================
-Class: CConfig
-Description:

=================================
*/
class CStartup
{
public:
	CStartup();
	~CStartup();

private:
	HMODULE m_hSDL2;
};

struct dll_export_t
{
	dll_export_t():
		functionptr(nullptr)
		{}

	CString functionname;
	void* functionptr;
};

extern CCVar* g_pCvarDeveloper;
extern CCVar* g_pCvarKeepOldSaves;

extern void Sys_Shutdown( void );
extern bool Sys_Init( CArray<CString>* argsArray );
extern bool Sys_ParseLaunchParams( const CArray<CString>* argsArray );
extern Int32 Sys_Main( CArray<CString>* argsArray );

extern void Sys_SetExit( void );
extern bool Sys_ShouldExit( void );

extern bool Sys_InitFloatTime( void );
extern Double Sys_FloatTime( void );

extern bool Sys_CheckGameDir( const CArray<CString>* argsArray );
extern bool Sys_LoadDefaultFont( const Char* pstr );
extern bool Sys_LoadGameInfo( CArray<CString>* argsArray );
extern bool Sys_IsGameControlActive( void );
extern void Sys_SetPaused( bool paused, bool print );

extern void Sys_InitCommands( void );
extern void Sys_InitCVars( void );

extern void Sys_ErrorPopup ( const Char *fmt, ... );
extern void Sys_Exit( void );

extern void Sys_WindowFocusLost( void );
extern void Sys_WindowFocusRegained( void );

extern Int32 Sys_CheckLaunchArgs( const Char* pstrArg );
extern const Char* Sys_LaunchArgv( Uint32 index );
extern Uint32 Sys_LaunchArgc( void );

extern void Con_Printf( const Char *fmt, ... );
extern void Con_DPrintf( const Char *fmt, ... );
extern void Con_VPrintf( const Char *fmt, ... );
extern void Con_EPrintf( const Char *fmt, ... );

extern bool Sys_GetDLLExports( const Char* pstrDLLName, void* pDLLHandle, CArray<dll_export_t>& destArray );

extern CCVar* g_pCVarPort;
#endif