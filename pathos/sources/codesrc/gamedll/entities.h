/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENTITIES_H
#define ENTITIES_H

#include "save_shared.h"

struct saverestore_data_t
{
	saverestore_data_t():
		transitionload(false),
		transitionsave(false),
		pentityindexarray(nullptr)
		{}

	bool transitionload;
	bool transitionsave;
	Vector landmarkOffset;

	const CArray<entindex_t>* pentityindexarray;
};

// Entity data description
struct entity_data_desc_t
{
	entity_data_desc_t():
		type(EFIELD_UNDEFINED),
		offset(0),
		size(0),
		flags(0)
		{}

	entity_data_desc_t(entfieldtype_t type, const Char* name, Uint64 offset, Uint16 size, Int16 flags):
		type(type),
		fieldname(name),
		offset(offset),
		size(size),
		flags(flags)
		{}

	// Datatype
	entfieldtype_t type;
	// Name of the variable
	CString fieldname;
	// Offset into data
	Uint64 offset;
	// Size of data
	Uint16 size;
	// flags
	Int16 flags;
};

// Data for save-restore
extern saverestore_data_t g_saveRestoreData;

// Field definitions for entity_state_t
extern entity_data_desc_t g_edictStateFields[];
// Field definitions for edict_fields_t
extern entity_data_desc_t g_edictStringFields[];

// Initializes private data for an entity
template <class T> void SetPrivateData( edict_t* pedict )
{
	pedict->pprivatedata = reinterpret_cast<void *>(new T(pedict));
}

// For linking .MAP entities with their classes
#define LINK_ENTITY_TO_CLASS(mapClassName, codeClassname) extern "C" DLLEXPORT void mapClassName( edict_t* pedict ) { SetPrivateData<codeClassname>(pedict); }

extern void SaveEntityState( entity_state_t& es, bool istransitionsave );
extern void SaveEntityFields( edict_fields_t& ef, bool istransitionsave );

// Engine functions to manage entities
extern bool DispatchSpawn( edict_t* pedict );
extern void DispatchThink( edict_t* pedict );
extern void DispatchTouch( edict_t* pedict, edict_t* pother );
extern void DispatchBlocked( edict_t* pedict, edict_t* pother );
extern void DispatchCrossedWater( edict_t* pedict, bool entering );
extern bool DispatchRestore( edict_t* pedict, bool istransferglobalentity );
extern void DispatchDeclareSaveFields( edict_t* pedict );
extern bool ShouldCollide( edict_t* pedict, edict_t* pOther );
extern void SetAbsBox( edict_t* pedict );
extern bool RunEntityPhysics( edict_t* pedict );
extern void ManageEntityGlobalState( edict_t* pedict, class CBaseEntity* pEntity );
extern void AdjustEntityPositions( edict_t* pedict, Vector prevmins );

extern bool KeyValue( edict_t* pedict, const keyvalue_t& keyvalue );
extern void FreeEntity( edict_t* pedict );
extern void OnAimentFreed( edict_t* pedict );
extern void SetupVisiblity( edict_t* pclient, byte*& ppvs, byte*& ppas );
extern bool AddPacketEntity( entity_state_t& state, entindex_t entindex, edict_t& entity, const edict_t& client, const byte* pset );
extern bool GetTransitioningEntities( const byte* pPVS, const Vector* pTransitionMins, const Vector* pTransitionMaxs, const Char* pstrLandmarkName, const Vector& landmarkPosition, Int32 *pTransitionList, Uint32& numEntities, Uint32 maxEntities );

extern void InitializeEntities( void );
extern void SendEntityInitMessages( edict_t* pPlayer );

extern void	BeginLoadSave( bool isLoadSave, bool isTransitionSave, bool isTransitionLoad, const Vector* pLandmarkOffset, const CArray<entindex_t>& entityIndexArray );
extern bool ReadEntityStateData( edict_t* pedict, const Char* fieldname, const byte* pdata, Uint32 datasize, Uint32 blockindex, bool istransferglobalentity );
extern bool ReadEntityFieldData( edict_t* pedict, const Char* fieldname, const byte* pdata, Uint32 datasize, Uint32 blockindex, bool istransferglobalentity );
extern bool ReadEntityClassData( edict_t* pedict, const Char* fieldname, const byte* pdata, Uint32 datasize, Uint32 blockindex, bool istransferglobalentity );
#ifdef _DEBUG
extern entity_data_desc_t CheckSaveField( entity_data_desc_t desc, Uint64 typesize, Int32 count, const Char* pstrObjectName, const Char* pstrVariableName );
#endif

extern edict_t* FindEntityByString( edict_t* pStartEntity, const Char* pstrFieldName, const Char* pstrValue );
extern edict_t* FindGlobalEntity( const Char* pstrClassname, const Char* pstrGlobalName );

// Global entity fields
#define EFIELD_GLOBAL		(1<<0)
#define EFIELD_SAVE_ALWAYS	(1<<1)

// Macros for entity fields
#ifdef _DEBUG
#define EFIELD(objtype, fieldname, fieldtype, size, flags)				CheckSaveField(entity_data_desc_t(fieldtype, #fieldname, offsetof(objtype, fieldname), size, flags), sizeof(reinterpret_cast<objtype *>(0)->fieldname), size, #objtype, #fieldname)
#else
#define EFIELD(objtype, fieldname, fieldtype, size, flags)				entity_data_desc_t(fieldtype, #fieldname, offsetof(objtype, fieldname), size, flags)
#endif

// Used to define entity fields
#define DEFINE_DATA_FIELD(objtype, fieldname, fieldtype)				EFIELD(objtype, fieldname, fieldtype, 1, 0)
#define DEFINE_DATA_FIELD_FLAGS(objtype, fieldname, fieldtype, flags)	EFIELD(objtype, fieldname, fieldtype, 1, flags)
#define DEFINE_DATA_FIELD_GLOBAL(objtype, fieldname, fieldtype)			EFIELD(objtype, fieldname, fieldtype, 1, EFIELD_GLOBAL)
#define DEFINE_DATA_FIELD_ARRAY(objtype, fieldname, fieldtype, count)	EFIELD(objtype, fieldname, fieldtype, count, 0)
#endif //ENTITIES_H