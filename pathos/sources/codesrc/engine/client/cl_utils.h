/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CL_UTILS_H
#define CL_UTILS_H

#include "cmd_shared.h"
#include "movevars.h"
#include "cvar.h"

struct en_material_t;
struct cache_model_t;
struct ui_schemeinfo_t;
struct ui_windowdescription_t;
struct cl_entity_t;
struct font_set_t;

enum snd_effects_t;

class CUIWindow;

extern cl_entity_t* CL_GetEntityByIndex( Int32 index );
extern cl_entity_t* CL_GetLocalPlayer( void );
extern Double CL_GetClientTime( void );
extern const Vector& CL_GetViewAngles( void );
extern void CL_SetViewAngles( const Vector& angles );
extern void CL_GetMouseDelta( Int32& deltaX, Int32 deltaY );
extern void CL_GetMouseDelta( Int32& deltaX, Int32& deltaY );
extern const movevars_t* CL_GetMoveVars( void );
extern Int32 CL_GetNumEntities( void );
extern void CL_PlayMusic( const CString& sample, Int32 channel, Float timeOffset, Float fadeInTime, Int32 flags );
extern void CL_StopMusic( Int32 channel );
extern void	CL_PlayEntitySound( entindex_t entindex, Int32 channel, const CString& sample, Float volume, Float attenuation, Int32 pitch, Int32 flags, Float timeoffset );
extern void	CL_PlayAmbientSound( entindex_t entindex, const Vector& vecOrigin, Int32 channel, const CString& sample, Float volume, Float attenuation, Int32 pitch, Int32 flags, Float timeoffset );
extern void CL_ApplySoundEffect( entindex_t entindex, const Char *sample,  Int32 channel, snd_effects_t effect, Float duration, Float targetvalue );
extern Int32 CL_PrecacheSound( const Char* pstrSample, rs_level_t level );
extern const Char*	CL_GetSoundFileForServerIndex( Int32 serverindex );
extern Vector CL_GetAttachment( entindex_t entindex, Uint32 attachment );
extern Vector CL_GetBonePosition( entindex_t entindex, const Char* pstrbonename );
extern void CL_GetMousePosition( Int32& x, Int32& y );
extern void CL_ShowMouse( void );
extern void CL_HideMouse( void );
extern void CL_UpdateMousePositions( void );
extern void CL_ResetMouse( void );
extern ui_schemeinfo_t* CL_UILoadSchemaFile( const Char* pstrFilename );
extern const en_material_t* CL_GetMapTextureMaterial( const Char* pstrtexturename );
extern Double CL_GetFrameTime( void );
extern void CL_SetShouldHideMouse( bool shouldhide );
extern const byte* CL_LeafPVS( const struct mleaf_t& leaf );
extern void	CL_PrecacheDecal( const Char* pstrDecalName );
extern void	CL_PrecacheDecalGroup( const Char* pstrDecalName );
extern const font_set_t* CL_GetSchemaFontSet( const Char* schemaFileName );
extern const font_set_t* CL_GetResolutionSchemaFontSet( const Char* schemaFileName, Uint32 resolution );
#endif //CL_UTILS_H