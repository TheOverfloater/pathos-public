/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <SDL.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include "includes.h"
#include "options.h"
#include "main.h"

// Structure that holds data for this application
vbmcompiler_options_t g_options;

//===============================================
// @brief Initializes the default values of the options
//
// @return TRUE if succeeded, FALSE otherwise
//===============================================
bool vbmcompiler_options_t::initialize( void )
{
	// This is enabled by default
	setFlag(CMP_FL_REVERSE_TRIANGLES);

	return true;
}

//===============================================
// @brief Set a compiler flag
//
// @param flag Flag(s) to set
//===============================================
void vbmcompiler_options_t::setFlag( Int32 flag )
{
	compilerflags |= flag;
}

//===============================================
// @brief Remove a compiler flag
//
// @param flag Flag(s) to remove
//===============================================
void vbmcompiler_options_t::removeFlag( Int32 flag )
{
	compilerflags &= ~flag;
}

//===============================================
// @brief Tell if a compiler flag is set
//
// @param flag Flag(s) to check for
//===============================================
bool vbmcompiler_options_t::isFlagSet( Int32 flag )
{
	return (compilerflags & flag) ? true : false;
}

//===============================================
// @brief Adds a new directory path to the list of directories.
// First path added will be used as the base directory, and will
// be added to all other directories specified
//
// @param pstrPath Path to add to the list
//===============================================
void vbmcompiler_options_t::addDirPath( const Char* pstrPath )
{
	// don't add current dir $cd's
	if(!qstrcmp(pstrPath, "./") || !qstrcmp(pstrPath, ".\\") || !qstrcmp(".", pstrPath))
		return;

	CString path;
	if(dirpaths.empty())
	{
		// If the list is empty, then this is the base path
		path << pstrPath << PATH_SLASH_CHAR;
		path = Common::CleanupPath(path.c_str());
		basedirectory = path;
	}
	else
	{
		
		path << basedirectory << PATH_SLASH_CHAR << pstrPath;
		path = Common::CleanupPath(path.c_str());
	}

	dirpaths.push_back(path);
}

//===============================================
// @brief TAdd a texture rename directive
//
// @param pstrOriginalName Original texture name to rename
// @param pstrNewName New name to set
//===============================================
void vbmcompiler_options_t::addRenamedTexture( const Char* pstrOriginalName, const Char* pstrNewName )
{
	CStringMap_t::iterator it = renamedtexturemap.find(pstrOriginalName);
	if(it != renamedtexturemap.end())
	{
		WarningMsg("Texture '%s' already marked to be renamed.\n", pstrOriginalName);
		return;
	}

	renamedtexturemap.insert(std::pair<CString, CString>(pstrOriginalName, pstrNewName));
}