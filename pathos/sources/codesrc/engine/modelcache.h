/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MODELCACHE_H
#define MODELCACHE_H

#include "cache_model.h"

struct maptexturematerial_t;

extern cmodel_type_t Cache_GetModelType( const cache_model_t& model );
extern const cache_model_t* Cache_GetModel( Int32 modelindex );
extern void Cache_GetModelBounds( const cache_model_t& model, Vector& mins, Vector& maxs );
extern Uint64 Cache_GetModelFrameCount( const cache_model_t& model );
extern Uint32 Cache_GetNbModels( void );
extern const cache_model_t* Cache_GetModelByName( const Char* pstrModelName );

/*
=======================
CModelCache

=======================
*/
class CModelCache
{
public:
	CModelCache( void );
	~CModelCache( void );

public:
	// Initializes the class
	void Init( void );

	// Clears the cache
	void ClearCache( void );
	// Clears GL-related states
	void ClearGL( void );

	// Loads a model into the cache
	cache_model_t* LoadModel( const Char* pstrFilename );

	// Finds a model in the cache by it's name
	cache_model_t* FindModelByName( const Char* pstrFilename );
	// Returns a model by it's index
	cache_model_t* GetModelByIndex( Uint32 index );
	// Returns the model's type
	cmodel_type_t GetType( Uint32 index );
	// Returns the number of models
	Uint32 GetNbCachedModels( void ) const { return m_modelCacheArray.size(); }
	// Returns all resources used by a model
	void GatherModelResources( const Char* pstrFilename, CArray<maptexturematerial_t>& mapTextureLinks, CArray<CString>& outMaterialsArray, CArray<CString>& outTexturesArray );

private:
	// Loads a studiomodel and the VBM file
	cache_model_t* LoadVBMModel( const Char* pstrFilename, const byte* pfile, Uint32 filesize );
	// Loads a BSP model from file
	cache_model_t* LoadBSPModel( const Char* pstrFilename, const byte* pfile );
	// Loads a sprite model from file
	cache_model_t* LoadSpriteModel( const Char* pstrFilename, const byte* pfile, Uint32 filesize );
	// Sets up the other BSP models
	void SetupBSPSubmodels( struct brushmodel_t& model, const Char* loadName );

	// Gathers resources of a material script
	static void GatherMaterialResources( struct en_material_t* pmaterialscript, CArray<CString>& outMaterialsArray, CArray<CString>& outTexturesArray );

public:
	// Missing models hash
	CHashList m_missingModelsHashList;
	// Cached models array
	CArray<cache_model_t*> m_modelCacheArray;
};
extern CModelCache gModelCache;
#endif