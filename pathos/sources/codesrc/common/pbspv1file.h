/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2020
All Rights Reserved.
===============================================
*/

#ifndef PBSPV1FILE_H
#define PBSPV1FILE_H

#include "miptex.h"
#include "contents.h"
#include "miptex.h"

//
// BSP limits
//

static const Uint32 PBSPV1_MAX_MAP_HULLS			= 4;
static const Uint32 PBSPV1_MAX_MAP_MODELS			= 4096;
static const Uint32 PBSPV1_MAX_MAP_BRUSHES			= 262144;
static const Uint32 PBSPV1_MAX_MAP_ENTITIES			= 65535;
static const Uint32 PBSPV1_MAX_MAP_ENTSTRING		= 2097152;
static const Uint32 PBSPV1_MAX_MAP_PLANES			= 262144;
static const Uint32 PBSPV1_MAX_MAP_NODES			= 262144;
static const Uint32 PBSPV1_MAX_MAP_CLIPNODES		= 262144;
static const Uint32 PBSPV1_MAX_MAP_LEAFS			= 262144;
static const Uint32 PBSPV1_MAX_MAP_VERTS			= 262144;
static const Uint32 PBSPV1_MAX_MAP_FACES			= 262144;
static const Uint32 PBSPV1_MAX_MAP_MARKSURFACES		= 262144;
static const Uint32 PBSPV1_MAX_MAP_TEXINFO			= 262144;
static const Uint32 PBSPV1_MAX_MAP_EDGES			= 524288;
static const Uint32 PBSPV1_MAX_MAP_SURFEDGES		= 1048576;
static const Uint32 PBSPV1_MAX_MAP_TEXTURES			= 16384;
static const Uint32 PBSPV1_MAX_MAP_LIGHTING			= 16777216;
static const Uint32 PBSPV1_MAX_MAP_VISIBILITY		= 16777216;

static const Uint32 PBSPV1_MAX_LIGHTMAPS			= 4;
static const Uint32 PBSPV1_LM_SAMPLE_SIZE			= 16;
static const Uint32 PBSPV1_NUM_AMBIENTS				= 4;
static const Uint32 PBSPV1_VERSION					= 1;

//
// BSP lumps
//
enum pbspv1_lumps_t
{
	PBSPV1_LUMP_ENTITIES = 0,
	PBSPV1_LUMP_PLANES,
	PBSPV1_LUMP_TEXTURES,
	PBSPV1_LUMP_VERTEXES,
	PBSPV1_LUMP_VISIBILITY,
	PBSPV1_LUMP_NODES,
	PBSPV1_LUMP_TEXINFO,
	PBSPV1_LUMP_FACES,
	PBSPV1_LUMP_LIGHTING,
	PBSPV1_LUMP_CLIPNODES,
	PBSPV1_LUMP_LEAFS,
	PBSPV1_LUMP_MARKSURFACES,
	PBSPV1_LUMP_EDGES,
	PBSPV1_LUMP_SURFEDGES,
	PBSPV1_LUMP_MODELS,
	PBSPV1_NB_LUMPS
};

//
// The lightstyles reserved for bump map data
//
enum pbspv1_lightmaps_t
{
	PBSPV1_LM_AMBIENT_STYLE		= 61,
	PBSPV1_LM_DIFFUSE_STYLE		= 62,
	PBSPV1_LM_LIGHTVECS_STYLE	= 63,
};

//
// Header for Pathos BSP V1
//

struct dpbspv1lump_t
{
	dpbspv1lump_t():
		offset(0),
		size(0)
	{}

	Int32 offset;
	Int32 size;
};

struct dpbspv1header_t
{
	dpbspv1header_t():
		id(0),
		version(0)
	{
		memset(lumps, 0, sizeof(lumps));
	}

	Int32 id;
	Int32 version;
	dpbspv1lump_t lumps[PBSPV1_NB_LUMPS];
};

//
// BSP file structures
//
struct dpbspv1model_t
{
	dpbspv1model_t():
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

	Int32 headnode[PBSPV1_MAX_MAP_HULLS];
	Int32 visleafs;

	Int32 firstface;
	Int32 numfaces;
};

struct dpbspv1vertex_t
{
	dpbspv1vertex_t()
	{
		memset(origin, 0, sizeof(origin));
	}

	Float origin[3];
};

struct dpbspv1plane_t
{
	dpbspv1plane_t():
		dist(0),
		type(0)
	{
		memset(normal, 0, sizeof(normal));
	}

	Float normal[3];
	Float dist;

	Int32 type;
};

struct dpbspv1node_t
{
	dpbspv1node_t():
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

struct dpbspv1clipnode_t
{
	dpbspv1clipnode_t():
		planenum(0)
	{
		memset(children, 0, sizeof(children));
	}

	Int32 planenum;
	Int32 children[2];
};

struct dpbspv1texinfo_t
{
	dpbspv1texinfo_t():
		miptex(0),
		flags(0)
	{
		memset(vecs, 0, sizeof(vecs));
	}

	Float vecs[2][4];
	Int32 miptex;
	Int32 flags;
};

struct dpbspv1edge_t
{
	Uint32 vertexes[2];
};

struct dpbspv1face_t
{
	dpbspv1face_t():
		planenum(0),
		side(0),
		firstedge(0),
		numedges(0),
		texinfo(0),
		lightoffset(0)
	{
		memset(lmstyles, 0, sizeof(lmstyles));
	}

	Uint32 planenum;
	Int32 side;

	Int32 firstedge;
	Int32 numedges;
	Int32 texinfo;

	byte lmstyles[PBSPV1_MAX_LIGHTMAPS];
	Int32 lightoffset;
};

struct dpbspv1leaf_t
{
	dpbspv1leaf_t():
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

	byte ambient_level[PBSPV1_NUM_AMBIENTS];
};
#endif //PBSPV1FILE_H