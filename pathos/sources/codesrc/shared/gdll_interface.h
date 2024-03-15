/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GDLL_INTERFACE_H
#define GDLL_INTERFACE_H

#include "cvar.h"
#include "cmd_shared.h"

struct edict_t;
struct usercmd_t;
struct gamevars_t;
struct trace_interface_t;
struct cache_model_t;
struct trace_t;
struct entity_state_t;
struct en_material_t;
struct mnode_t;
struct brushmodel_t;
struct mleaf_t;
struct brushmodel_t;
struct save_globalentity_t;

enum cmodel_type_t;
enum savefile_type_t;
enum entfieldtype_t;
enum msgdest_t;
enum part_script_type_t;
enum npc_movetype_t;

// GameDLL interface version
#define GDLL_INTERFACE_VERSION 1

struct gdll_funcs_t
{
	bool			(*pfnGameDLLInit)( void );
	void			(*pfnGameDLLShutdown)( void );

	bool			(*pfnGameInit)( void );
	void			(*pfnGameShutdown)( void );
	void			(*pfnServerFrame)( void );

	bool			(*pfnDispatchSpawn)( edict_t* pedict );
	bool			(*pfnDispatchRestore)( edict_t* pedict, bool isglobal );
	void			(*pfnDispatchThink)( edict_t* pedict );
	void			(*pfnDispatchTouch)( edict_t* pedict, edict_t* pOther );
	void			(*pfnDispatchBlocked)( edict_t* pedict, edict_t* pOther );
	void			(*pfnDispatchCrossedWater)( edict_t* pedict, bool entering );
	void			(*pfnDispatchDeclareSaveFields)( edict_t* pedict );
	bool			(*pfnShouldCollide)( edict_t* pedict, edict_t* pOther );
	void			(*pfnSetAbsBox)( edict_t* pedict );
	bool			(*pfnRunEntityPhysics)( edict_t* pedict );
	void			(*pfnPrecacheResources)( void );
	void			(*pfnPostSpawnGame)( void );

	bool			(*pfnKeyValue)( edict_t* pedict, const struct keyvalue_t& keyvalue );
	void			(*pfnFreeEntity)( edict_t* pedict );
	void			(*prnOnAimentFreed)( edict_t* pedict );

	void			(*pfnGetHullSizes)( Int32 hullIndex, Vector& pmins, Vector& pmaxs );
	void			(*pfnSetupVisibility)( edict_t* pclient, byte*& ppvs, byte*& ppas );
	bool			(*pfnAddPacketEntity)( entity_state_t& state, entindex_t entindex, edict_t& entity, const edict_t& client, const byte* pset );

	void			(*pfnCmdStart)( const usercmd_t& cmd, edict_t* pclient );
	void			(*pfnCmdEnd)( edict_t* pclient );

	void			(*pfnRunPlayerMovement)( const usercmd_t& cmd, struct pm_info_t* pminfo );

	edict_t*		(*pfnFindEntityByString)( edict_t* pStartEntity, const Char* pstrFieldName, const Char* pstrValue );

	bool			(*pfnClientCommand)( edict_t* pclient );
	void			(*pfnClientPreThink)( edict_t* pclient );
	void			(*pfnClientPostThink)( edict_t* pclient );

	bool			(*pfnClientConnect)( edict_t* pclient, const Char* pname, const Char* paddress, CString& rejectReason );
	void			(*pfnClientDisconnected)( edict_t* pclient );
	void			(*pfnInitializeClientData)( edict_t* pclient );

	void			(*pfnSaveEntityStateData)( edict_t* pedict, bool istransitionsave );
	void			(*pfnSaveEntityFieldsData)( edict_t* pedict, bool istransitionsave );
	void			(*pfnSaveEntityClassData)( edict_t* pedict, bool istransitionsave );

	bool			(*pfnShouldTransitionEntity)( edict_t* pedict );
	bool			(*pfnIsGlobalTransitioningEntity)( edict_t* pedict );
	bool			(*pfnShouldSaveEntity)( edict_t* pedict );

	void			(*pfnGetSaveGameTitle)( Char* pstrBuffer, Int32 maxlength );
	void			(*pfnBeginLoadSave)( bool isLoadSave, bool isTransitionSave, bool isTransitionLoad, const Vector* pLandmarkOffset, const CArray<entindex_t>& entityIndexArray );
	bool			(*pfnReadEntityStateData)( edict_t* pedict, const Char* fieldname, const byte* pdata, Uint32 datasize, Uint32 blockindex, bool istransferglobalentity );
	bool			(*pfnReadEntityFieldData)( edict_t* pedict, const Char* fieldname, const byte* pdata, Uint32 datasize, Uint32 blockindex, bool istransferglobalentity );
	bool			(*pfnReadEntityClassData)( edict_t* pedict, const Char* fieldname, const byte* pdata, Uint32 datasize, Uint32 blockindex, bool istransferglobalentity );
	edict_t*		(*pfnFindGlobalEntity)( const Char* pstrClassname, const Char* pstrGlobalName );
	void			(*pfnAdjustEntityPositions)( edict_t* pedict, Vector prevmins );

	bool			(*pfnInconsistentFile)( const Char* pstrFilename );

	Uint32			(*pfnGetNbGlobalStates)( void );
	void			(*pfnSaveGlobalStates)( void );
	void			(*pfnReadGlobalStateData)( const Char* pstrglobalname, const Char* pstrlevelname, enum globalstate_state_t state );

	bool			(*pfnAreCheatsEnabled)( void );

	bool			(*pfnGetTransitioningEntities)( const byte* pPVS, const Vector* pTransitionMins, const Vector* pTransitionMaxs, const Char* pstrLandmarkName, const Vector& landmarkPosition, Int32 *pTransitionList, Uint32& numEntities, Uint32 maxEntities );
	bool			(*pfnCanSaveGame)( enum savefile_type_t type );
	bool			(*pfnCanLoadGame)( void );

	void			(*pfnShowSaveGameMessage)( void );
	void			(*pfnShowAutoSaveGameMessage)( void );
	void			(*pfnShowSaveGameBlockedMessage)( void );

	void			(*pfnRestoreDecal)( const Vector& origin, const Vector& normal, edict_t* pedict, const Char* pstrDecalTexture, Int32 decalflags );
	void			(*pfnFormatKeyValue)( const Char* pstrKeyValueName, Char* pstrValue, Uint32 maxLength );
};

struct gdll_engfuncs_t
{
	Int32					(*pfnPrecacheModel)( const Char* pstrFilepath );
	Int32					(*pfnPrecacheSound)( const Char* pstrFilepath );
	void					(*pfnPrecacheGeneric)( const Char* pstrresourcename );
	bool					(*pfnPrecacheParticleScript)( const Char* pstrFilepath, part_script_type_t type );
	void					(*pfnPrecacheDecal)( const Char* pstrDecalName );
	void					(*pfnPrecacheDecalGroup)( const Char* pstrDecalName );

	bool					(*pfnSetModel)( edict_t* pedict, const Char* pstrFilepath, bool setbounds );
	void					(*pfnSetMinsMaxs)( edict_t* pedict, const Vector& mins, const Vector& maxs );
	void					(*pfnSetSize)( edict_t* pedict, const Vector& size );
	void					(*pfnSetOrigin)( edict_t* pedict, const Vector& origin );

	byte*					(*pfnSetPAS)( const Vector& origin );
	byte*					(*pfnSetPVS)( const Vector& origin );

	void					(*pfnCon_Printf)( const Char *fmt, ... );
	void					(*pfnCon_DPrintf)( const Char *fmt, ... );
	void					(*pfnCon_VPrintf)( const Char *fmt, ... );
	void					(*pfnCon_EPrintf)( const Char *fmt, ... );
	void					(*pfnClientPrintf)( const edict_t* pclient, const Char *fmt, ... );

	Int32					(*pfnGetNbEdicts)( void );
	edict_t*				(*pfnGetEdictByIndex)( entindex_t entindex );
	edict_t*				(*pfnCreateEntity)( const Char* pstrClassName );
	void					(*pfnRemoveEntity)( edict_t* pentity );
	bool					(*pfnDropToFloor)( edict_t* pentity );

	const cache_model_t*	(*pfnGetModel)( Int32 modelindex );
	void					(*pfnGetModelBounds)( const cache_model_t& model, Vector& mins, Vector& maxs );
	cmodel_type_t			(*pfnGetModelType)( const cache_model_t& model );
	Uint32					(*pfnGetNbCachedModels)( void );
	const cache_model_t*	(*pfnGetModelByName)( const Char* pstrModelName );

	void					(*pfnAddToTouched)( entindex_t hitent, trace_t& trace, const Vector& velocity );
	const mleaf_t*			(*pfnPointInLeaf)( const Vector& position, const brushmodel_t& model );
	const byte*				(*pfnLeafPVS)( byte* pbuffer, Uint32 bufsize, const mleaf_t& leaf, const brushmodel_t& model );

	void					(*pfnSetGroupMask)( Int32 mask, Int32 op );

	void					(*pfnPlayEntitySound)( entindex_t entindex, const Char* pstrPath, Int32 flags, Int32 channel, Float volume, Float attenuation, Int32 pitch, Float timeoffset, Int32 dest_player );
	void					(*pfnPlayAmbientSound)( entindex_t entindex, const Char* pstrPath, const Vector& origin, Int32 flags, Float volume, Float attenuation, Int32 pitch, Float timeoffset, Int32 dest_player );
	void					(*pfnApplySoundEffect)( entindex_t entindex, const Char* pstrPath, Int32 channel, enum snd_effects_t effect, Float duration, Float targetvalue, Int32 dest_player );
	void					(*pfnStopEntitySounds)( entindex_t entindex, Int32 channel, Int32 dest_player );
	void					(*pfnSetMuteAllSounds)( bool mutesounds );
	void					(*pfnSetRoomType)( Int32 roomtype );

	void					(*pfnPlayMusic)( const Char* sample, Int32 channel, Int32 flags, Float timeOffset, Float fadeInTime, Int32 dest_player );
	void					(*pfnStopMusic)( Int32 dest_player, const Char* pstrFilename, Int32 channel, Float fadeTime );

	const Char*				(*pfnNameForFunction)( const void* functionPtr );
	void*					(*pfnFunctionFromName)( const Char* pstrName );

	const en_material_t*	(*pfnGetMaterialScript)( const Char* pstrTextureName );
	const en_material_t*	(*pfnGetMapTextureMaterialScript)( const Char* pstrtexturename );

	Uint32					(*pfnAllocString)( const Char* pString );
	const Char*				(*pfnGetString)( string_t stringindex );

	void					(*pfnSaveWriteBool)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteByte)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteChar)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteInt16)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteUint16)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteInt32)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteUint32)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteInt64)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteUint64)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteFloat)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteDouble)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteTime)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteString)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteRawString)( const Char* fieldname, const byte* pdata, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteVector)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteCoord)( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteEntindex)( const Char* fieldname, const byte* const pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	void					(*pfnSaveWriteGlobalState)( Uint32 index, const Char* pstrglobalname, const Char* pstrlevelname, enum globalstate_state_t state );

	Int32					(*pfnRegisterUserMessage)( const Char* pstrMsgName, Int32 msgsize );
	void					(*pfnUserMessageBegin)( msgdest_t dest, Int32 msgid, const Vector* porigin, const edict_t* pedict );
	void					(*pfnUserMessageEnd)( void );

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

	void					(*pfnServerCommand)( const Char* pstrCmd );
	void					(*pfnClientCommand)( edict_t* pclient, const Char* pstrCmd );
	void					(*pfnCreateCommand)( const Char* name, cmdfunc_t pfn, const Char* description );

	bool					(*pfnGetBonePositionByName)( edict_t* pedict, const Char* pstrbonename, Vector& position );
	bool					(*pfnGetBonePositionByIndex)( edict_t* pedict, Uint32 boneindex, Vector& position );
	bool					(*pfnGetAttachment)( edict_t* pedict, Uint32 index, Vector& position );

	bool					(*pfnRecursiveLightPoint)( const brushmodel_t* pworld, struct mnode_t *pnode, const Vector &start, const Vector &end, Vector &color );

	CCVar*					(*pfnCreateCVar)( cvar_type_t type, Int32 flags, const Char* pstrName, const Char* pstrValue, const Char* pstrDescription );
	CCVar*					(*pfnCreateCVarCallback)( cvar_type_t type, Int32 flags, const Char* pstrName, const Char* pstrValue, const Char* pstrDescription, pfnCVarCallback_t pfnCallback );
	CCVar*					(*pfnGetCVarPointer)( const Char* pstrName );
	void					(*pfnSetCVarFloat)( const Char* pstrName, Float value );
	void					(*pfnSetCVarString)( const Char* pstrName, const Char* pstrValue );
	Float					(*pfnGetCvarFloatValue)( const Char* pstrCvarName );
	const Char*				(*pfnGetCvarStringValue)( const Char* pstrCvarName );

	Uint32					(*pfnCmd_Argc)( void );
	const Char*				(*pfnCmd_Argv)( Uint32 index );

	Float					(*pfnGetSoundDuration)( const Char* pstrfilename, Uint32 pitch );
	Uint32					(*pfnGetVISBufferSize)( void );
	Uint64					(*pfnGetModelFrameCount)( Int32 modelindex );

	void					(*pfnAddLevelConnection)( const Char* pstrLevelName, const Char* pstrOtherLevelName, const Char* pstrLandmarkName, const Char* pstrMapSaveFileName );
	void					(*pfnBeginLevelChange)( const Char* pstrlevelname, const Char* pstrlandmarkname, const Vector& landmarkposition );
	void					(*pfnGetTransitionList)( const Int32** pEntityList, Uint32& numEntities );

	void					(*pfnAddSavedDecal)( const Vector& origin, const Vector& normal, entindex_t entityindex, const Char* pstrDecalTexture, Int32 decalflags );

	bool					(*pfnWalkMove)( edict_t* pentity, Float yaw, Float dist, enum walkmove_t movemode );
	void					(*pfnMoveToOrigin)( edict_t* pedict, const Vector& goalPosition, Float moveyaw, Float dist, npc_movetype_t movetype );
	bool					(*pfnCheckBottom)( edict_t* pedict );
	edict_t*				(*pfnFindClientInPVS)( const edict_t* pedict );

	void					(*pfnEndGame)( const Char* pstrEndGameCode );
	Double					(*pfnFloatTime)( void );
};
#endif //GDLL_INTERFACE_H