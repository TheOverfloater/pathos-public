/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#ifndef TEXTURES_SHARED_INLINE_HPP
#define TEXTURES_SHARED_INLINE_HPP

//====================================
// @brief Return the material script path for a map texture
//
// @param pstrTextureName Name of texture
// @return Path to map texture material script
//====================================
inline CString GetMapTexturePath( const Char* pstrFolderName, const Char* pstrTextureName )
{
	// First look in the BSP folder
	CString folderbasename;
	Common::Basename(pstrFolderName, folderbasename);

	CString texturebasename;
	Common::Basename(pstrTextureName, texturebasename);

	CString filepath;
	filepath << WORLD_TEXTURES_PATH_BASE << folderbasename << PATH_SLASH_CHAR << texturebasename << PMF_FORMAT_EXTENSION;
	return filepath;
}

//====================================
// @brief Return the material script path for a model texture
//
// @param pstrTextureName Name of model
// @param pstrTextureName Name of texture
// @return Path to model texture material script
//====================================
inline CString GetModelTexturePath( const Char* pstrModelName, const Char* pstrTextureName )
{
	CString textureName;
	Common::Basename(pstrTextureName, textureName); 
	textureName.tolower();

	CString modelName;
	Common::Basename(pstrModelName, modelName); 
	modelName.tolower();

	// Create and assign the group
	CString materialscriptpath;
	materialscriptpath << MODEL_MATERIALS_BASE_PATH << modelName << PATH_SLASH_CHAR << textureName.c_str() << PMF_FORMAT_EXTENSION;
	return materialscriptpath;
}
#endif // TEXTURES_SHARED_INLINE_HPP