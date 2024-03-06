/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMEDLL_H
#define GAMEDLL_H

#define DLLEXPORT __declspec( dllexport )

struct gdll_engfuncs_t;
struct trace_interface_t;
struct file_interface_t;
struct gamevars_t;
struct usercmd_t;
struct pm_info_t;

// Declaration of gamedll enginefuncs struct
extern gdll_engfuncs_t gd_engfuncs;
// Declaration of traceline interface
extern trace_interface_t gd_tracefuncs;
// File functions
extern file_interface_t gd_filefuncs;
// Declaration of gamevars pointer
extern gamevars_t* g_pGameVars;

// VIS buffer for server
extern byte* g_pVISBuffer;
// VIS buffer size
extern Uint32 g_visBufferSize;

extern bool GameDLLInit( void );
extern void GameDLLShutdown( void );
extern bool GameInit( void );
extern void GameShutdown( void );
extern void InitializeClientData( edict_t* pclient );
extern void GetHullSizes( Int32 hullIndex, Vector& pmins, Vector& pmaxs );
extern void ServerFrame( void );
extern void RunPlayerMovement( const usercmd_t& cmd, pm_info_t* pminfo );
extern void SaveEntityStateData( edict_t* pedict, bool istransitionsave );
extern void SaveEntityFieldsData( edict_t* pedict, bool istransitionsave );
extern void SaveEntityClassData( edict_t* pedict, bool istransitionsave );
extern bool IsGlobalTransitioningEntity( edict_t* pedict );
extern bool ShouldTransitionEntity( edict_t* pedict );
extern bool ShouldSaveEntity( edict_t* pedict );
extern void GetSaveGameTitle( Char* pstrBuffer, Int32 maxlength );
extern bool InconsistentFile( const Char* pstrFilename );
extern bool AreCheatsEnabled( void );
extern void DumpCheatCodes( void );
extern void SetConnectionSaveFile( const Char* pstrLevelName, const Char* pstrLandmarkName, const Char* pstrSaveFileName );
extern void PrecacheResources( void );
extern bool CanSaveGame( enum savefile_type_t type );
extern bool CanLoadGame( void );
extern void ShowSaveGameMessage( void );
extern void ShowAutoSaveGameMessage( void );
extern void ShowSaveGameBlockedMessage( void );
extern void RestoreDecal( const Vector& origin, const Vector& normal, edict_t* pedict, const Char* pstrDecalTexture, Int32 decalflags );
extern void FormatKeyValue( const Char* pstrKeyValueName, Char* pstrValue, Uint32 maxLength );
extern void PostSpawnGame( void );
#endif //GAMEDLL_H
