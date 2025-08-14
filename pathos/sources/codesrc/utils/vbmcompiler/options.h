/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef OPTIONS_H
#define OPTIONS_H

#include <map>

#include "studio.h"
#include "constants.h"
#include "vbmformat.h"
#include "com_math.h"
#include "compiler_types.h"

// Max default texture size
static const Uint32 DEFAULT_MAX_TEXTURE_RESOLUTION = 512;
// Minimum texture size
static const Uint32 MIN_TEXTURE_RESOLUTION = 8;
// Default border pad setting
static const Uint32 DEFAULT_BORDER_PAD_VALUE = 2;

enum compiler_flags_t
{
	CMP_FL_NONE						= 0,		// No particular flags
	CMP_FL_CENTERLIGHT				= (1<<0),	// Force model to get light from center of bbox
	CMP_FL_SKYLIGHT					= (1<<1),	// Force model to take lighting from light_environment
	CMP_FL_STRIP_STUDIO_TRI_DATA	= (1<<2),	// Set compiler to strip geometry data in HL1 MDL
	CMP_FL_NO_STUDIOMDL_VERT_LIMIT	= (1<<3),	// Disable vertex and normal limits in HL1 MDL generated
	CMP_FL_DISABLE_VBM_GENERATION	= (1<<4),	// Disable VBM generation(will only generate HL1 MDL)
	CMP_FL_STRIP_MDL_TEXTURES		= (1<<5),	// Set compiler to strip actual textures in HL1 MDL
	CMP_FL_WAIT_FOR_KEY				= (1<<6),	// Make compiler wait for key input before closing
	CMP_FL_REVERSE_TRIANGLES		= (1<<7),	// Reverse orientation of triangles read
	CMP_FL_DUMP_HITBOX_DATA			= (1<<8),	// Dump hitbox data
	CMP_FL_NO_STUDIOMDL_BONE_LIMIT	= (1<<9),	// Allow going over the max bones limit
	CMP_FL_TAG_BAD_NORMALS			= (1<<10),	// Tag corrupted normals
	CMP_FL_TAG_REVERSED_TRIANGLES	= (1<<11),	// Tag reversed triangles
};

struct vbmcompiler_options_t
{
	vbmcompiler_options_t():
		error(false),
		normal_merge_treshold(0),
		compilerflags(CMP_FL_NONE),
		max_texture_resolution(DEFAULT_MAX_TEXTURE_RESOLUTION),
		border_padding(DEFAULT_BORDER_PAD_VALUE)
	{
	}

public:
	// Initialize default values, etc
	bool initialize( void );
	// Set a compiler flag
	void setFlag( Int32 flag );
	// Remove a compiler flag
	void removeFlag( Int32 flag );
	// Tell if a compiler flag is set
	bool isFlagSet( Int32 flag );

	// Adds a directory path
	void addDirPath( const Char* pstrPath );
	// Add a texture rename directive
	void addRenamedTexture( const Char* pstrOriginalName, const Char* pstrNewName );

public:
	// Name of the output file
	CString outputname;
	// Base output directory
	CString basedirectory;
	// Array of file lookup dirs
	CArray<CString> dirpaths;
	// Renamed textures from launch args
	CStringMap_t renamedtexturemap;

	// TRUE if we have an error
	bool error;

	// Normal merge treshold specified in params
	Float normal_merge_treshold;

	// Compiler flags
	Int32 compilerflags;
	// Max texture resolution setting
	Uint32 max_texture_resolution;
	// Border padding
	Uint32 border_padding;
};
extern vbmcompiler_options_t g_options;
#endif //OPTIONS_H