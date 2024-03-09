/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CLDLL_INTERFACE_H
#define CLDLL_INTERFACE_H

#include "cmd_shared.h"
#include "cvar.h"

struct edict_t;
struct usercmd_t;
struct cl_entity_t;
struct ref_params_t;
struct movevars_t;
struct en_material_t;
struct cache_model_t;
struct brushmodel_t;
struct mleaf_t;
struct entity_extrainfo_t;
struct mnode_t;
struct decalgroupentry_t;
struct ui_schemeinfo_t;
struct ui_windowdescription_t;
struct cl_dlight_t;
struct font_set_t;

enum cmodel_type_t;
enum snd_effects_t;

class Vector;
class CDecalList;
class CUIWindow;

// client dll interface version
#define CLDLL_INTERFACE_VERSION 1

struct cldll_funcs_t
{
	bool (*pfnClientDLLInit)( void );
	void (*pfnClientDLLShutdown)( void );

	void (*pfnClientConnected)( void );
	void (*pfnClientDisconnected)( void );
	void (*pfnClientFrame)( void );

	bool (*pfnGameInit)( void );
	void (*pfnGameReset)( void );

	bool (*pfnGLInit)( void );
	void (*pfnGLClear)( void );

	void (*pfnCalcRefDef)( ref_params_t& params );
	void (*pfnSetupView)( const ref_params_t& params );
	bool (*pfnDrawNormal)( void );
	bool (*pfnDrawTransparent)( void );
	bool (*pfnDrawHUD)( bool hudOnly );

	void (*pfnClientPreCmdThink)( void );
	void (*pfnInMove)( usercmd_t& cmd );
	void (*pfnMouseMove)( usercmd_t& cmd );

	void (*pfnRunPlayermove)( const usercmd_t& cmd, struct pm_info_t* pminfo, bool playSounds );

	void (*pfnAddEntities)( void );
	void (*pfnDecalExternalEntities)( const Vector& vpos, const Vector& vnorm, decalgroupentry_t *texptr, Int32 flags );

	void (*pfnGetClientEntityList)( const struct entitydata_t*& pEntitiesPtr, Uint32& numEntities );
	void (*pfnParseEntityList)( void );
	void (*pfnFreeEntityData)( void );
	void (*pfnVBMEvent)( const struct mstudioevent_t* pvbmevent, struct cl_entity_t* pentity );

	cl_entity_t* (*pfnGetViewModel)( void );

	bool (*pfnAddSubtitle)( const Char* pstrSubtitleName, Float duration );
	void (*pfnRemoveSubtitle)( const Char* pstrSubtitleName );

	void (*pfnGetViewInfo)( Vector& vOrigin, Vector& vAngles );

	bool (*pfnIsInputOverridden)( void );
	bool (*pfnIsMouseOverridden)( void );
	bool (*pfnIsEscapeKeyOverridden)( void );
	void (*pfnKeyEvent)( Int32 button, Int16 mod, bool keyDown );
	void (*pfnMouseButtonEvent)( Int32 button, bool keyDown );
	void (*pfnMouseWheelEvent)( Int32 button, bool keyDown, Int32 scroll );

	void (*pfnWindowFocusLost)( void );
	void (*pfnWindowFocusRegained)( void );

	bool (*pfnDrawViewObjects)( void );
	bool (*pfnDrawViewObjectsForVSM)( cl_dlight_t* dl );
		 
	bool (*pfnDrawLadders)( void );
	bool (*pfnDrawLaddersForVSM)( cl_dlight_t* dl );

	void (*pfnAdjustEntityTimers)( struct entity_state_t* pstate, Double jointime );

	bool (*pfnShouldDrawPausedLogo)( void );
	void (*pfnClientLevelChange)( void );
};

struct cldll_engfuncs_t
{
	Double					(*pfnGetClientTime)( void );
	Double					(*pfnGetEngineTime)( void );
	Double					(*pfnGetFrameTime)( void );
	cl_entity_t*			(*pfnGetLocalPlayer)( void );
	cl_entity_t*			(*pfnGetEntityByIndex)( Int32 index );

	const Vector&			(*pfnGetViewAngles)( void );
	void					(*pfnSetViewAngles)( const Vector& angles );

	void					(*pfnCon_Printf)( const Char *fmt, ... );
	void					(*pfnCon_DPrintf)( const Char *fmt, ... );
	void					(*pfnCon_VPrintf)( const Char *fmt, ... );
	void					(*pfnCon_EPrintf)( const Char *fmt, ... );
	void					(*pfnErrorPopup)( const Char *fmt, ... );

	void					(*pfnCreateCommand)( const Char* name, cmdfunc_t pfn, const Char* description );

	Uint32					(*pfnCmd_Argc)( void );
	const Char*				(*pfnCmd_Argv)( Uint32 index );

	void					(*pfnGetMousePosition)( Int32& x, Int32& y );
	void					(*pfnGetMouseDelta)( Int32& deltaX, Int32& deltaY );
	const movevars_t*		(*pfnGetMoveVars)( void );

	cmodel_type_t			(*pfnGetModelType)( const cache_model_t& model );
	void					(*pfnGetModelBounds)( const cache_model_t& model, Vector& mins, Vector& maxs );
	const cache_model_t*	(*pfnGetModel)( Int32 modelindex );
	const cache_model_t*	(*pfnGetModelByName)( const Char* pstrModelName );
	Uint32					(*pfnGetNbModels)( void );

	Int32					(*pfnGetNumEntities)( void );
	Uint32					(*pfnGetMaxClients)( void );

	void					(*pfnPlayEntitySound)( entindex_t entindex, Int32 channel, const CString& sample, Float volume, Float attenuation, Int32 pitch, Int32 flags, Float timeoffset );
	void					(*pfnPlayAmbientSound)( entindex_t entindex, const Vector& origin, Int32 channel, const CString& sample, Float volume, Float attenuation, Int32 pitch, Int32 flags, Float timeoffset );
	Int32					(*pfnPrecacheSound)( const Char* pstrSample, rs_level_t level );
	void					(*pfnApplySoundEffect)( entindex_t entindex, const Char *sample,  Int32 channel, snd_effects_t effect, Float duration, Float targetvalue );
	const Char*				(*pfnGetSoundFileForServerIndex)( Int32 serverindex );

	void					(*pfnPrecacheDecal)( const Char* pstrDecalName );
	void					(*pfnPrecacheDecalGroup)( const Char* pstrDecalName );

	void					(*pfnPlayMusic)( const CString& sample, Int32 channel, Float timeOffset, Float fadeInTime, Int32 flags );
	void					(*pfnStopMusic)( Int32 channel );

	const en_material_t*	(*pfnGetMaterialScript)( const Char* pstrTextureName );
	const en_material_t*	(*pfnGetMapTextureMaterialScript)( const Char* pstrTextureName );

	void					(*pfnAddEntity)( cl_entity_t* pentity );

	const byte*				(*pfnLeafPVS)( const mleaf_t& leaf );
	const mleaf_t*			(*pfnPointInLeaf)( const Vector& position );

	const cache_model_t*	(*pfnLoadModel)( const Char* pstrModelName );

	entity_extrainfo_t*		(*pfnGetEntityExtraData)( cl_entity_t* pentity );
	void					(*pfnFindTouchedLeafs)( const brushmodel_t* pworld, CArray<Uint32>& leafnumsarray, const Vector& mins, const Vector& maxs, mnode_t* pnode );
	bool					(*pfnRecursiveLightPoint)( const brushmodel_t* pworld, struct mnode_t *pnode, const Vector &start, const Vector &end, Vector &color );

	void					(*pfnPrecacheFlexScript)( enum flextypes_t npctype, const Char* pstrscript );
	void					(*pfnSetFlexScript)( entindex_t entindex, const Char* pstrscript );

	Vector					(*pfnGetAttachment)( entindex_t entindex, Uint32 attachment );
	Vector					(*pfnGetBonePosition)( entindex_t entindex, const Char* pstrbonename );
	void					(*pfnUpdateAttachments)( cl_entity_t* pentity );

	void					(*pfnServerCommand)( const Char* pstrCommand );
	void					(*pfnClientCommand)( const Char* pstrCommand );

	void					(*pfnShowMouse)( void );
	void					(*pfnHideMouse)( void );
	void					(*pfnSetShouldHideMouse)( bool shouldhide );
	void					(*pfnUpdateMousePositions)( void );
	void					(*pfnResetMouse)( void );

	Int32					(*pfnRegisterClientUserMessage)( const Char* pstrMsgName, Int32 msgsize );
	void					(*pfnClientUserMessageBegin)( Int32 msgid );
	void					(*pfnClientUserMessageEnd)( void );

	void					(*pfnMsgWriteByte)( byte value );
	void					(*pfnMsgWriteChar)( Char value );
	void					(*pfnMsgWriteInt16)( Int16 value );
	void					(*pfnMsgWriteUint16)( Uint16 value );
	void					(*pfnMsgWriteInt32)( Int32 value );
	void					(*pfnMsgWriteUint32)( Uint32 value );
	void					(*pfnMsgWriteInt64)( Int64 value );
	void					(*pfnMsgWriteUint64)( Uint64 value );
	void					(*pfnMsgWriteSmallFloat)( Float value );
	void					(*pfnMsgWriteFloat)( Float value );
	void					(*pfnMsgWriteDouble)( Double value );
	void					(*pfnMsgWriteBuffer)( const byte* pdata, Uint32 size );
	void					(*pfnMsgWriteString)( const Char* pdata );
	void					(*pfnMsgWriteEntindex)( entindex_t entindex );

	CCVar*					(*pfnCreateCVar)( cvar_type_t type, Int32 flags, const Char* pstrName, const Char* pstrValue, const Char* pstrDescription );
	CCVar*					(*pfnCreateCVarCallback)( cvar_type_t type, Int32 flags, const Char* pstrName, const Char* pstrValue, const Char* pstrDescription, pfnCVarCallback_t pfnCallback );
	CCVar*					(*pfnGetCVarPointer)( const Char* pstrName );
	void					(*pfnSetCVarFloat)( const Char* pstrName, Float value );
	void					(*pfnSetCVarString)( const Char* pstrName, const Char* pstrValue );
	Float					(*pfnGetCvarFloatValue)( const Char* pstrCvarName );
	const Char*				(*pfnGetCvarStringValue)( const Char* pstrCvarName );

	const font_set_t*		(*pfnGetSchemaFontSet)( const Char* schemaFileName );
	const font_set_t*		(*pfnGetResolutionSchemaFontSet)( const Char* schemaFileName, Uint32 resolution );

	void					(*pfnSetPaused)( bool isPaused, bool pauseOveride );
};

#endif //CLDLL_INTERFACE_H