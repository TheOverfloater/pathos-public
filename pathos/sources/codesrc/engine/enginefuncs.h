/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENGINEFUNCS_H
#define ENGINEFUNCS_H

extern const en_material_t* Engine_GetMaterialScript( const Char* pstrTextureName );
extern const byte* Engine_LeafPVS( const mleaf_t& leaf );
extern const mleaf_t* Engine_PointInLeaf( const Vector& position );
extern const cache_model_t* Engine_LoadModel( const Char* pstrModelPath );
extern const Char* Engine_GetLevelName( void );
extern Double Engine_GetEngineTime( void );
extern CCVar* Engine_CreateCVar( cvar_type_t type, Int32 flags, const Char* pstrName, const Char* pstrValue, const Char* pstrDescription );
extern CCVar* Engine_CreateCVarCallback( cvar_type_t type, Int32 flags, const Char* pstrName, const Char* pstrValue, const Char* pstrDescription, pfnCVarCallback_t pfnCallback );
extern CCVar* Engine_GetCVarPointer( const Char* pstrName );
extern void Engine_SetCVarFloat( const Char* pstrName, Float value );
extern void Engine_SetCVarString( const Char* pstrName, const Char* pstrValue );
extern void Engine_CreateCommand( const Char* name, cmdfunc_t pfn, const Char* description );
extern Uint32 Engine_Cmd_Argc( void );
extern const Char* Engine_Cmd_Argv( Uint32 index );
extern Float Engine_GetCvarFloatValue( const Char* pstrCvarName );
extern const Char* Engine_GetCvarStringValue( const Char* pstrCvarName );
extern Uint32 Engine_GetVISBufferSize( void );
#endif //ENGINEFUNCS_H