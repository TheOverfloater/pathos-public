/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2020
All Rights Reserved.
===============================================
*/

#ifndef PBSPV2FILE_H
#define PBSPV2FILE_H

#include "miptex.h"
#include "contents.h"
#include "miptex.h"

//
// BSP limits
//

static const Uint32 PBSPV2_MAX_MAP_HULLS			= 4;
static const Uint32 PBSPV2_MAX_MAP_MODELS			= 4096;
static const Uint32 PBSPV2_MAX_MAP_BRUSHES			= 262144;
static const Uint32 PBSPV2_MAX_MAP_ENTITIES			= 65535;
static const Uint32 PBSPV2_MAX_MAP_ENTSTRING		= 2097152;
static const Uint32 PBSPV2_MAX_MAP_PLANES			= 262144;
static const Uint32 PBSPV2_MAX_MAP_NODES			= 262144;
static const Uint32 PBSPV2_MAX_MAP_CLIPNODES		= 262144;
static const Uint32 PBSPV2_MAX_MAP_LEAFS			= 262144;
static const Uint32 PBSPV2_MAX_MAP_VERTS			= 262144;
static const Uint32 PBSPV2_MAX_MAP_FACES			= 262144;
static const Uint32 PBSPV2_MAX_MAP_MARKSURFACES		= 262144;
static const Uint32 PBSPV2_MAX_MAP_TEXINFO			= 262144;
static const Uint32 PBSPV2_MAX_MAP_EDGES			= 524288;
static const Uint32 PBSPV2_MAX_MAP_SURFEDGES		= 1048576;
static const Uint32 PBSPV2_MAX_MAP_TEXTURES			= 16384;
static const Uint32 PBSPV2_MAX_MAP_LIGHTING			= 16777216;
static const Uint32 PBSPV2_MAX_MAP_VISIBILITY		= 16777216;

static const Uint32 PBSPV2_MAX_LIGHTMAPS			= 4;
static const Uint32 PBSPV2_LM_SAMPLE_SIZE			= 16;
static const Uint32 PBSPV2_NUM_AMBIENTS				= 4;
static const Uint32 PBSPV2_VERSION					= 2;

//
// BSP lumps
//
enum pbspv2_lumps_t
{
	PBSPV2_LUMP_ENTITIES = 0,
	PBSPV2_LUMP_PLANES,
	PBSPV2_LUMP_TEXTURES,
	PBSPV2_LUMP_VERTEXES,
	PBSPV2_LUMP_VISIBILITY,
	PBSPV2_LUMP_NODES,
	PBSPV2_LUMP_TEXINFO,
	PBSPV2_LUMP_FACES,
	PBSPV2_LUMP_LIGHTING_DEFAULT,
	PBSPV2_LUMP_LIGHTING_AMBIENT,
	PBSPV2_LUMP_LIGHTING_DIFFUSE,
	PBSPV2_LUMP_LIGHTING_VECTORS,
	PBSPV2_LUMP_CLIPNODES,
	PBSPV2_LUMP_LEAFS,
	PBSPV2_LUMP_MARKSURFACES,
	PBSPV2_LUMP_EDGES,
	PBSPV2_LUMP_SURFEDGES,
	PBSPV2_LUMP_MODELS,
	PBSPV2_NB_LUMPS
};

//
// Flags for BSP file
//
enum pbspv2_flags_t
{
	PBSPV2_FL_NONE					= 0,
	PBSPV2_FL_HAS_SMOOTHING_GROUPS	= (1<<0)
};

//
// Header for Pathos BSP V1
//

struct dpbspv2lump_t
{
	dpbspv2lump_t():
		offset(0),
		size(0)
	{}

	Int32 offset;
	Int32 size;
};

struct dpbspv2header_t
{
	dpbspv2header_t():
		id(0),
		version(0),
		flags(0)
	{
		memset(lumps, 0, sizeof(lumps));
	}

	Int32 id;
	Int32 version;
	Int64 flags;

	dpbspv2lump_t lumps[PBSPV2_NB_LUMPS];
};

//
// BSP file structures
//
struct dpbspv2model_t
{
	dpbspv2model_t():
		visleafs(0),
		firstface(0),
		numfaces(0)
	{
		memset(mins, 0, sizeof(mins));
		memset(maxs, 0, sizeof(maxs));
		memset(origin, 0, sizeof(origin));
		memset(headnode, 0, sizeof(headnode));
	}

	Float mins[3];
	Float maxs[3];
	Float origin[3];

	Int32 headnode[PBSPV2_MAX_MAP_HULLS];
	Int32 visleafs;

	Int32 firstface;
	Int32 numfaces;
};

struct dpbspv2vertex_t
{
	dpbspv2vertex_t()
	{
		memset(origin, 0, sizeof(origin));
	}

	Float origin[3];
};

struct dpbspv2plane_t
{
	dpbspv2plane_t():
		dist(0),
		type(0)
	{
		memset(normal, 0, sizeof(normal));
	}

	Float normal[3];
	Float dist;

	Int32 type;
};

struct dpbspv2node_t
{
	dpbspv2node_t():
		planenum(0),
		firstface(0),
		numfaces(0)
	{
		memset(children, 0, sizeof(children));
		memset(mins, 0, sizeof(mins));
		memset(maxs, 0, sizeof(maxs));
	}

	Int32 planenum;
	Int32 children[2];
	Int16 mins[3];
	Int16 maxs[3];

	Uint32 firstface;
	Uint32 numfaces;
};

struct dpbspv2clipnode_t
{
	dpbspv2clipnode_t():
		planenum(0)
	{
		memset(children, 0, sizeof(children));
	}

	Int32 planenum;
	Int32 children[2];
};

struct dpbspv2texinfo_t
{
	dpbspv2texinfo_t():
		miptex(0),
		flags(0)
	{
		memset(vecs, 0, sizeof(vecs));
	}

	Float vecs[2][4];
	Int32 miptex;
	Int32 flags;
};

struct dpbspv2edge_t
{
	Uint32 vertexes[2];
};

struct dpbspv2face_t
{
	dpbspv2face_t():
		planenum(0),
		side(0),
		firstedge(0),
		numedges(0),
		texinfo(0),
		samplescale(0),
		smoothgroupbits(0),
		lightoffset(0)
	{
		memset(lmstyles, 0, sizeof(lmstyles));
	}

	Uint32 planenum;
	Int32 side;

	Int32 firstedge;
	Int32 numedges;
	Int32 texinfo;
	Float samplescale;
	Int32 smoothgroupbits;

	byte lmstyles[PBSPV2_MAX_LIGHTMAPS];
	Int32 lightoffset;
};

struct dpbspv2leaf_t
{
	dpbspv2leaf_t():
		contents(0),
		visoffset(0),
		firstmarksurface(0),
		nummarksurfaces(0)
	{
		memset(mins, 0, sizeof(mins));
		memset(maxs, 0, sizeof(maxs));
		memset(ambient_level, 0, sizeof(ambient_level));
	}

	Int32 contents;
	Int32 visoffset;

	Int16 mins[3];
	Int16 maxs[3];

	Uint32 firstmarksurface;
	Uint32 nummarksurfaces;

	byte ambient_level[PBSPV2_NUM_AMBIENTS];
};
#endif //PBSPV2FILE_H