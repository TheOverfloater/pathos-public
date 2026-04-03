/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.

===============================================
*/

#ifndef MDLTEXTUREEXPORT_H
#define MDLTEXTUREEXPORT_H

/*
=======================
CMDLTextureExporter

=======================
*/
class CMDLTextureExporter
{
private:
	// Path to model textures root folder
	static const Char MODEL_TEXTURES_BASE_PATH[];
	// Path to model textures root folder
	static const Char TEXTURES_BASE_PATH[];

private:
	enum studio_texflags_t
	{
		STUDIO_TEX_FL_CHROME = 2,			// Texture flag in studiomodels for chrome
		STUDIO_TEX_FL_ADDITIVE = 32,		// Texture flag in studiomodels for additive
		STUDIO_TEX_FL_ALPHATEST = 64,		// Texture flag in studiomodels for alphatest
		STUDIO_TEX_FL_SCOPE = 2048,			// Texture flag in studiomodels for no culling
		STUDIO_TEX_FL_ALPHABLEND = 8192,	// Texture flag in studiomodels for alpha blending
		STUDIO_TEX_FL_EYEGLINT = 16384,		// Texture flag in studiomodels for eye glint
		STUDIO_TEX_FL_NOCULL = 131072,		// Texture flag in studiomodels for no culling
	};

public:
	// Constructor
	CMDLTextureExporter( void );
	// Destructor
	~CMDLTextureExporter( void );

public:
	// Export textures for a model
	bool ExportMDLTextures( const byte* pTextureMDLData, const Char* pstrModelFilePath, const Char* pstrOutputPath );
	// Returns the number of bytes written for the last processed model
	Uint32 GetNbBytesWritten( void ) const { return m_nbBytesWritten; }

private:
	// Tells if a texture is a null texture
	bool IsNullTexture( const color24_t* ppalette, const Char* pstrName, Int32 width, Int32 height );
	// Convert a palette texture to a bitmap file
	bool ExportTextureToTGA( const Char* pstrFilepath, const color24_t* ppalette, const byte* ppixeldata, Int32 width, Int32 height, bool alphatest, Uint32* ptrCompressionRatio );
	// Creates a PMF file based on the attributes
	bool CreatePMFFile( const Char* pstrOutputPath, const Char* pstrModelBaseName, const Char* pstrTextureFilename, const Char* pstrTextureBaseName, const Char* pstrTexturePath, CArray<CString>& attribsArray );
	// Generate PMF files for a model
	Uint32 GeneratePMFFiles( const Char* pstrModelBaseName, const Char* pstrOutputPath );

private:
	// Studiomodel data header
	const studiohdr_t* m_pTextureHeader;
	// Studiomodel texture data ptr
	const mstudiotexture_t* m_pStudioTextures;
	// Number of bytes written for last processed mdl
	Uint32 m_nbBytesWritten;
};
extern CMDLTextureExporter gMDLTextureExporter;
#endif //MDLTEXTUREEXPORT_H