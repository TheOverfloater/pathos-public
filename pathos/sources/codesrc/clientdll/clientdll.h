/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CLIENTDLL_H
#define CLIENTDLL_H
#define DLLEXPORT __declspec( dllexport )

#include "includes.h"
#include "cldll_interface.h"
#include "trace.h"
#include "file_interface.h"
#include "r_interface.h"
#include "efxapi.h"

// Declaration of gamedll enginefuncs struct
extern cldll_engfuncs_t cl_engfuncs;
// Trace functions from engine
extern trace_interface_t cl_tracefuncs;
// Declaration of file functions struct
extern file_interface_t cl_filefuncs;
// Render interface
extern r_interface_t cl_renderfuncs;
// Effects API
extern cl_efxapi_t cl_efxapi;
// Stepsound structure
extern class CStepSound g_stepSound;

// TRUE if we're executing a level change when ClientGameReset is called
extern bool g_isLevelChangeReset;

extern bool ClientDLLInit( void );
extern void ClientDLLShutdown( void );

extern void ClientFrame( void );

extern void ClientConnected( void );
extern void ClientDisconnected( void );

extern void ClientInitUIInterface( void );
extern void ClientInitStepSound( void );

extern bool ClientGameInit( void );
extern void ClientGameReset( void );

extern bool ClientGLInit( void );
extern void ClientGLClear( void );

extern void ClientPreCmdThink( void );
extern void ClientLevelChange( void );

extern void CL_WindowFocusLost( void );
extern void CL_WindowFocusRegained( void );

extern void CL_RunPlayerMovement( const usercmd_t& cmd, pm_info_t* pminfo, bool playSounds );
extern void CL_DecalExternalEntities( const Vector& vpos, const Vector& vnorm, decalgroupentry_t *texptr, Int32 flags );
extern bool CL_AddSubtitle( const Char* pstrSubtitleName, Float duration );
extern void CL_RemoveSubtitle( const Char* pstrSubtitleName );
extern bool CL_IsInputOverridden( void );
extern bool CL_IsMouseOverridden( void );
extern bool CL_IsEscapeKeyOverridden( void );
extern void CL_KeyEvent( Int32 button, Int16 mod, bool keyDown );
extern void CL_MouseButtonEvent( Int32 button, bool keyDown );
extern void CL_MouseWheelEvent( Int32 button, bool keyDown, Int32 scroll );
extern bool CL_ShouldDrawPausedLogo( void );

#endif //CLIENTDLL_H
