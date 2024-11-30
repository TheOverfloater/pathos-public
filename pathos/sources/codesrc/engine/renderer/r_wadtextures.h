/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#ifndef WADTEXTURES_H
#define WADTEXTURES_H

#include <map>

struct dmiptex_t;
struct en_texture_t;
struct en_material_t;
struct dmiptexlump_t;

// Path to legacy detail texture associations
const Char DETAIL_TEXTURE_ASSOCIATION_FILE_PATH[] = "scripts/legacy/detailtextures.txt";
// Detail texture folder name
const Char DETAIL_TEXTURE_FOLDER_NAME[] = "detail";

// Material types script
const Char MATERIAL_TYPES_FILE_PATH[] = "scripts/legacy/materialtypes.txt";

struct detailtexture_t
{
	detailtexture_t():
		width(0),
		height(0)
		{
			memset(filename, 0, sizeof(filename));
		}

	Char filename[MAX_PARSE_LENGTH];

	Uint32 width;
	Uint32 height;
};

struct detail_association_t
{
	detail_association_t():
		detailtextureidx(0)
		{
			memset(maptexturename, 0, sizeof(maptexturename));
		}

	Char maptexturename[MAX_PARSE_LENGTH];
	Int32 detailtextureidx;
};

/*
====================
CWADTextureResource

====================
*/
class CWADTextureResource
{
private:
	typedef std::map<CString, Int32> NameIndexMap_t;

private:
	struct material_association_t
	{
		material_association_t():
			materialtypecode('\0')
			{
				memset(maptexturename, 0, sizeof(maptexturename));
			}

		Char maptexturename[MAX_PARSE_LENGTH];
		Char materialtypecode;
	};
	struct material_type_t
	{
		material_type_t():
			materialcode('\0')
			{
				memset(materialtype, 0, sizeof(materialtype));
			}
		Char materialtype[MAX_PARSE_LENGTH];
		Char materialcode;
	};
	struct wad3file_t
	{
		wad3file_t():
			pwadfile(nullptr),
			hasmissing(false)
			{
				memset(wadfilename, 0, sizeof(wadfilename));
			}

		Char wadfilename[MAX_PARSE_LENGTH];
		const byte* pwadfile;
		NameIndexMap_t texturenameindexmap;

		bool hasmissing;
	};

private:
	// Path to legacy texture material type associations
	static const Char TEXTURE_MATERIAL_ASSOCIATION_FILE_PATH[];

public:
	CWADTextureResource( void );
	~CWADTextureResource( void );

public:
	// Initializes the class, loads BSP and all WADs
	bool Init( const Char* pstrBSPName, const CArray<CString>& wadFilesList, bool generateMissingWAD, bool generateMissingBSP );
	// Returns a WAD texture's pointer
	en_texture_t* GetWADTexture( en_material_t* pmaterial, const Char* pstrContainerName, const Char* pstrTextureName );

private:
	// Loads material type associations
	bool LoadMaterialTypeCodeAssociations( void );
	// Loads material associations
	bool LoadMaterialTextureAssociations( void );

	// Checks if the material script exists
	static bool IsMaterialScriptPresent( const Char* pstrTextureName, const Char* pstrContainerName );
	// Creates a material script for a WAD texture
	void CreateMaterialScript( const dmiptex_t* ptexture, const Char* pstrContainerName );

	// Return BSP texture data lump
	const dmiptexlump_t* GetBSPTextureData( void );

private:
	// BSP file basename
	CString				m_BSPFileName;
	// BSP file pointer
	byte*				m_pBSPFile;
	// BSP texture name index map
	NameIndexMap_t		m_bspTextureNameIndexMap;
	// Array of WAD3 file ptrs
	CArray<wad3file_t>	m_pWADFilesArray;

	// Array of detail textures loaded
	CArray<detailtexture_t*> m_detailTexturesArray;
	// Array of map texture->detail texture associations
	CArray<detail_association_t*> m_detailTextureAssociationArray;
	// Array of map texture->detail texture associations
	CArray<material_association_t*> m_materialAssociationArray;
	// Material code->type associations
	CArray<material_type_t*> m_materialCodeTypesAssociations;
};

extern bool WAD_LoadDetailTextureAssociations( CArray<detailtexture_t*>& detailtextures, CArray<detail_association_t*>& associations );
extern CString WAD_GetWADFolderPath( const Char* pstrContainerName, const Char* pstrBasePath = nullptr );
extern CString WAD_GetWADTexturePath( const Char* pstrFolderName, const Char* pstrWorldTextureName );
#endif //WADTEXTURES_H