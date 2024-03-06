/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#ifndef R_TEXTURES_H
#define R_TEXTURES_H

#include "common.h"
#include "textures_shared.h"
#include "file_interface.h"
#include "constants.h"

class CGLExtF;

//====================
// CTextureManager
// 
//====================
class CTextureManager
{
public:
	struct alias_mapping_t
	{
		alias_mapping_t():
			pmaterialfile(nullptr),
			level(RS_LEVEL_UNDEFINED)
			{};

		CString filename;
		en_material_t* pmaterialfile;
		rs_level_t level;
	};

public:
	// Default value for specular scale
	static const Float DEFAULT_SPECFACTOR;
	// Default phong exponent value
	static const Float DEFAULT_PHONG_EXP;
	// Anisotropy off value
	static const Uint32 ANISOTROPY_OFF_VALUE;

private:
	// Texture allocs list type
	typedef CLinkedList<en_texalloc_t*> AllocList_t;
	// Individual textures list type
	typedef CLinkedList<en_texture_t*> TexturesList_t;
	// Materials list type
	typedef CLinkedList<en_material_t*> MaterialsList_t;

private:
	CTextureManager( const file_interface_t& fileFuncs, pfnPrintf_t printFunction, pfnPrintf_t printErrorFunction, const CGLExtF& glExtF, bool onlyMaterials );
	~CTextureManager( void );

public:
	// Performs application-level initialization
	void Init( void );
	// Releases all currently used resources
	void Shutdown( void );

	// Deletes binds for all textures on a resource level
	void DeleteTextures( rs_level_t level, bool keepentry = true );
	// Removes a single texture
	void DeleteTexture( const Char *pstrFilename );
	// Removes a single texture
	void DeleteTexture( en_texture_t* ptexture );
	// Deletes materials on a resource level
	void DeleteMaterials( rs_level_t level );
	// Deletes an allocation
	void DeleteAllocation( en_texalloc_t* palloc );
	// Reloads data for existing texture entries
	void ReloadResources( void );

	// Seeks out a texture by name on a resource level
	en_texture_t* FindTexture( const Char* pstrFilename, rs_level_t level );
	// Loads a texture
	en_texture_t* LoadTexture( const Char* pstrFilename, rs_level_t level, Int32 flags = TX_FL_NONE, const GLint* pborder = nullptr );
	// Loads a texture from memory
	en_texture_t* LoadFromMemory( const Char* pstrTextureName, rs_level_t level, Int32 flags, const byte* pdata, Uint32 width, Uint32 height, Uint32 bpp, Uint32 datasize = 0 );
	// Loads the paletted texture from memory
	en_texture_t* LoadPallettedTexture( const Char* pstrFilename, rs_level_t level, const byte *pdata, const color24_t *ppal, Uint32 width, Uint32 height, Int32 flags );
	// Loads a material script
	en_material_t* LoadMaterialScript( const Char* pstrFilename, rs_level_t level, bool prompt = true, bool isloadingfromalias = false );

	// Allocates an OpenGL texture index
	en_texalloc_t* GenTextureIndex( rs_level_t level );
	// Returns the dummy texture pointer
	en_texture_t* GetDummyTexture( void ) { return m_pDummyTexture; };
	// Returns the dummy material pointer
	en_material_t* GetDummyMaterial( void ) { return m_pDummyMaterial; };

	// Seeks out a texture by name on a resource level
	en_material_t* FindMaterialScript( const Char* pstrFilename, rs_level_t level );
	// Finds the material by it's index
	en_material_t* FindMaterialScriptByIndex( Int32 index );

	// Returns the anisotropy value at a setting index
	Int32 GetAnisotropySettingValue( Uint32 settingIndex );
	// Returns the number of anisotropy setting
	Uint32 GetNbAnisotropySettings( void ) const;
	// Updates anisotropy settings
	void UpdateAnisotropySettings( Float cvarValue );
	// Populates the anisotropy list
	void PopulateAnisotropyList( void );

	// Writes a PMF material file
	void WritePMFFile( en_material_t* pmaterial );

public:
	// Creates an instance of the texture manager
	static CTextureManager* CreateInstance( const file_interface_t& fileFuncs, pfnPrintf_t printFunction, pfnPrintf_t printErrorFunction, const CGLExtF& glExtF, bool onlyMaterials );
	// Returns the current instance of the texture manager
	static CTextureManager* GetInstance( void );
	// Deletes the current instance
	static void DeleteInstance( void );

private:
	// Frees all textures on the specified resource level
	void DeleteBinds( rs_level_t level );
	// Returns the texture type for a type string
	static mt_texture_t GetTextureType( const Char* pstrTypename );
	// Returns the format for a filename extension
	static texture_format_t GetFormat( const Char* pstrFilename );

	// Allocates a new texture object
	en_texture_t* AllocTexture( void );
	// Creates the dummy texture image
	void CreateDummyTexture( void );
	
	// Sets the material index array
	void ResetMaterialsIndexArray( void );

private:
	// Array of loaded textures
	TexturesList_t m_texturesList;
	// Array of material definitions
	MaterialsList_t m_materialsList;
	// Material index->material ptr index array
	CArray<en_material_t*> m_materialsIndexPtrArray;
	// Alias mappings list
	CLinkedList<alias_mapping_t> m_aliasMappingsList;

	// Linked list of allocations
	AllocList_t m_allocsList;

	// Dummy texture's pointer
	en_texture_t* m_pDummyTexture;
	// Dummy texture's pointer
	en_material_t* m_pDummyMaterial;

	// Valid anisotropy values
	CArray<Int32> m_anisotropySettingsArray;

	// Current anisotropy setting index
	Int32 m_currentAnisotropySetting;
	// Current anisotropy value
	Int32 m_currentAnisotropyValue;

	// File functions interface
	const file_interface_t m_fileFuncs;
	// Console print function
	pfnPrintf_t m_printFunction;
	// Console print function
	pfnPrintf_t m_printErrorFunction;
	// GL ext functions class
	const CGLExtF& m_glExtF;

private:
	// Texture manager instance
	static CTextureManager* g_pInstance;
};
#endif // R_TEXTURES_H