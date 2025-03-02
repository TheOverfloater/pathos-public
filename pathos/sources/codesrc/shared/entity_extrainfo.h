/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENTITY_EXTRAINFO_H
#define ENTITY_EXTRAINFO_H

#include "studio.h"
#include "mlight.h"
#include "flex_shared.h"
#include "animinfo.h"
#include "brushmodel_shared.h"

struct entity_animinfo_t;

// Flags used by lighting
#define MDL_LIGHT_FIRST			1
#define MDL_LIGHT_NOBLEND		2

enum entity_type_t
{
	ENTITY_TYPE_BASIC = 0,
	ENTITY_TYPE_VBM
};

struct entity_lightinfo_t
{
	entity_lightinfo_t():
		flags(0),
		lighttime(0),
		numsavedmlights(0),
		reset(false)
	{
		for(Uint32 i = 0; i < (MAX_SURFACE_STYLES-1); i++)
		{
			prev_lightstyles[i] = 0;
			target_lightstyles[i] = 0;
			lightstyles[i] = 0;
		}
	}

	Int32 flags;

	// Main light value info
	Vector ambient_color;
	Vector direct_color;
	Vector lightdirection;

	// Current lightstyle ambient colors
	Vector lightstylecolors_ambient[MAX_SURFACE_STYLES-1];
	// Current lightstyle diffuse colors
	Vector lightstylecolors_diffuse[MAX_SURFACE_STYLES-1];
	// Styles of the surface we're on
	byte lightstyles[MAX_SURFACE_STYLES-1];

	// for lerping values
	Vector prev_ambient;
	Vector target_ambient;
	Vector prev_diffuse;
	Vector target_diffuse;
	Vector prev_lightdir;
	Vector target_lightdir;

	// lerping for lightstyles
	Vector prev_stylecolors_ambient[MAX_SURFACE_STYLES-1];
	Vector target_stylecolors_ambient[MAX_SURFACE_STYLES-1];

	Vector prev_stylecolors_diffuse[MAX_SURFACE_STYLES-1];
	Vector target_stylecolors_diffuse[MAX_SURFACE_STYLES-1];

	byte prev_lightstyles[MAX_SURFACE_STYLES-1];
	byte target_lightstyles[MAX_SURFACE_STYLES-1];

	// time we changed values
	Double lighttime;

	// array of latest model light infos
	mlightinfo_t savedmlights[MAX_ENT_MLIGHTS];
	Uint32 numsavedmlights;

	Vector lastlightorigin;

	// Used by nightstage
	bool reset;
};

struct entity_extrainfo_t
{
	entity_extrainfo_t():
		entindex(0),
		type(ENTITY_TYPE_BASIC),
		paniminfo(nullptr),
		plightinfo(nullptr),
		pflexstate(nullptr),
		pvbmdecalheader(nullptr),
		pwaterdata(nullptr),
		pmonitordata(nullptr),
		pmirrordata(nullptr),
		pshadowmap(nullptr),
		pportaldata(nullptr),
		ppvsdata(nullptr),
		pentity(nullptr)
		{}
	~entity_extrainfo_t()
	{
		if(paniminfo)
			delete paniminfo;
		if(plightinfo)
			delete plightinfo;
		if(pflexstate)
			delete pflexstate;
		if(ppvsdata)
			delete[] ppvsdata;
	}

	// Entity's index
	entindex_t entindex;
	// Entity's type
	entity_type_t type;

	// Entity's abs maxs
	Vector absmax;
	// Entity's abs mins
	Vector absmin;

	// animation info(studiomodels)
	entity_animinfo_t* paniminfo;
	// lighting info(studiomodels)
	entity_lightinfo_t* plightinfo;
	// flex information(studiomodels)
	flexstate_t* pflexstate;

	// list of decals linked to a studio entity
	struct vbmdecal_t* pvbmdecalheader;
	// water entity data for this entity
	struct cl_water_t* pwaterdata;
	// tied monitor data
	struct cl_monitor_t* pmonitordata;
	// tied mirror data
	struct cl_mirror_t* pmirrordata;
	// dynamic light shadow map tied to this entity
	struct shadowmap_t* pshadowmap;
	// tied portal data
	struct cl_portal_t* pportaldata;

	// PVS data for cameras
	byte* ppvsdata;

	// Leafnums for entity
	CArray<Uint32> leafnums;
	
	// cl_entity_t we're tied to
	struct cl_entity_t* pentity;
};

#endif //ENTITY_EXTRAINFO_H