/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CACHE_MODEL_H
#define CACHE_MODEL_H

#include "brushmodel_shared.h"
#include "studio.h"
#include "sprformat.h"

// Model types
enum cmodel_type_t
{
	MOD_NONE = 0,
	MOD_BRUSH,
	MOD_SPRITE,
	MOD_VBM
};

// Model cache structure
struct cache_model_t
{
	cache_model_t():
		flags(0),
		type(MOD_NONE),
		cacheindex(0),
		isloaded(false),
		radius(0),
		pcachedata(nullptr)
	{}

	//=============================================
	// @brief Returns a pointer to the burshmodel data
	//
	// @return Pointer to brushmodel data
	//=============================================
	brushmodel_t* getBrushmodel( void )
	{
		assert(type == MOD_BRUSH);
		return reinterpret_cast<brushmodel_t*>(pcachedata);
	}

	//=============================================
	// @brief Returns a const pointer to the burshmodel data
	//
	// @return Const pointer to brushmodel data
	//=============================================
	const brushmodel_t* getBrushmodel( void ) const 
	{
		assert(type == MOD_BRUSH);
		return reinterpret_cast<const brushmodel_t*>(pcachedata);
	}

	//=============================================
	// @brief Returns a pointer to the VBM data
	//
	// @return Pointer to the VBM data
	//=============================================
	vbmcache_t* getVBMCache( void )
	{
		assert(type == MOD_VBM);
		return reinterpret_cast<vbmcache_t*>(pcachedata);
	}

	//=============================================
	// @brief Returns a const pointer to the VBM data
	//
	// @return Const pointer to the VBM data
	//=============================================
	const vbmcache_t* getVBMCache( void ) const 
	{
		assert(type == MOD_VBM);
		return reinterpret_cast<const vbmcache_t*>(pcachedata);
	}

	//=============================================
	// @brief Returns a pointer to the sprite data
	//
	// @return Pointer to the sprite data
	//=============================================
	msprite_t* getSprite( void )
	{
		assert(type == MOD_SPRITE);
		return reinterpret_cast<msprite_t*>(pcachedata);
	}

	//=============================================
	// @brief Returns a const pointer to the sprite data
	//
	// @return Const pointer to the sprite data
	//=============================================
	const msprite_t* getSprite( void ) const 
	{
		assert(type == MOD_SPRITE);
		return reinterpret_cast<const msprite_t*>(pcachedata);
	}

	// Model flags
	Int32 flags;

	// File path of the model
	CString name;
	// Model type
	cmodel_type_t type;
	// Index into cache
	Uint32 cacheindex;
	// TRUE if model was loaded into GL
	bool isloaded;

	// Mins of the model
	Vector mins;
	// Maxs of the model
	Vector maxs;
	// Radius of the model
	Float radius;

	// Pointer to cache data
	void *pcachedata;
};
#endif