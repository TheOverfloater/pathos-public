/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2020
All Rights Reserved.
===============================================
*/

#ifndef PBSPV3FILE_H
#define PBSPV3FILE_H

#include "miptex.h"
#include "contents.h"
#include "miptex.h"

//
// BSP limits
//

static constexpr Uint32 PBSPV3_MAX_MAP_HULLS			= 4;
static constexpr Uint32 PBSPV3_MAX_MAP_MODELS			= 4096;
static constexpr Uint32 PBSPV3_MAX_MAP_BRUSHES			= 262144;
static constexpr Uint32 PBSPV3_MAX_MAP_ENTITIES			= 65535;
static constexpr Uint32 PBSPV3_MAX_MAP_ENTSTRING		= 2097152;
static constexpr Uint32 PBSPV3_MAX_MAP_PLANES			= 262144;
static constexpr Uint32 PBSPV3_MAX_MAP_NODES			= 262144;
static constexpr Uint32 PBSPV3_MAX_MAP_CLIPNODES		= 262144;
static constexpr Uint32 PBSPV3_MAX_MAP_LEAFS			= 262144;
static constexpr Uint32 PBSPV3_MAX_MAP_VERTS			= 262144;
static constexpr Uint32 PBSPV3_MAX_MAP_FACES			= 262144;
static constexpr Uint32 PBSPV3_MAX_MAP_MARKSURFACES		= 262144;
static constexpr Uint32 PBSPV3_MAX_MAP_TEXINFO			= 262144;
static constexpr Uint32 PBSPV3_MAX_MAP_EDGES			= 524288;
static constexpr Uint32 PBSPV3_MAX_MAP_SURFEDGES		= 1048576;
static constexpr Uint32 PBSPV3_MAX_MAP_TEXTURES			= 16384;
static constexpr Uint32 PBSPV3_MAX_MAP_LIGHTING			= 16777216;
static constexpr Uint32 PBSPV3_MAX_MAP_VISIBILITY		= 16777216;

static constexpr Uint32 PBSPV3_MAX_LIGHTMAPS			= 4;
static constexpr Uint32 PBSPV3_LM_SAMPLE_SIZE			= 16;
static constexpr Uint32 PBSPV3_NUM_AMBIENTS				= 4;
static constexpr Uint32 PBSPV3_VERSION					= 3;
static constexpr Uint32 PBSPV3_MAX_SURFACE_EXTENTS		= 1024;

//
// BSP lumps
//
enum pbspv3_lumps_t
{
	PBSPV3_LUMP_ENTITIES = 0,
	PBSPV3_LUMP_PLANES,
	PBSPV3_LUMP_TEXTURES,
	PBSPV3_LUMP_VERTEXES,
	PBSPV3_LUMP_VISIBILITY,
	PBSPV3_LUMP_NODES,
	PBSPV3_LUMP_TEXINFO,
	PBSPV3_LUMP_FACES,
	PBSPV3_LUMP_LIGHTING_DEFAULT,
	PBSPV3_LUMP_LIGHTING_AMBIENT,
	PBSPV3_LUMP_LIGHTING_DIFFUSE,
	PBSPV3_LUMP_LIGHTING_VECTORS,
	PBSPV3_LUMP_CLIPNODES,
	PBSPV3_LUMP_LEAFS,
	PBSPV3_LUMP_MARKSURFACES,
	PBSPV3_LUMP_EDGES,
	PBSPV3_LUMP_SURFEDGES,
	PBSPV3_LUMP_MODELS,
	PBSPV3_LUMP_VERTEX_LIGHTING,
	PBSPV3_LUMP_VERTEX_LIGHTING_AMBIENT,
	PBSPV3_LUMP_VERTEX_LIGHTING_DIFFUSE,
	PBSPV3_LUMP_VERTEX_LIGHTING_VECTORS,
	PBSPV3_NB_LUMPS
};

//
// Flags for BSP file
//
enum pbspv3_flags_t
{
	PBSPV3_FL_NONE					= 0,
	PBSPV3_FL_HAS_SMOOTHING_GROUPS	= (1<<0)
};

//
// Header for Pathos BSP V3
//

struct dpbspv3lump_t
{
	dpbspv3lump_t():
		offset(0),
		size(0)
	{}

	Int32 offset;
	Int32 size;
};

struct dpbspv3header_t
{
	dpbspv3header_t():
		id(0),
		version(0),
		flags(0)
	{
		memset(lumps, 0, sizeof(lumps));
	}

	Int32 id;
	Int32 version;
	Int64 flags;

	dpbspv3lump_t lumps[PBSPV3_NB_LUMPS];
};

//
// BSP file structures
//
struct dpbspv3model_t
{
	dpbspv3model_t():
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

	Int32 headnode[PBSPV3_MAX_MAP_HULLS];
	Int32 visleafs;

	Int32 firstface;
	Int32 numfaces;
};

struct dpbspv3vertex_t
{
	dpbspv3vertex_t()
	{
		memset(origin, 0, sizeof(origin));
	}

	Float origin[3];
};

struct dpbspv3plane_t
{
	dpbspv3plane_t():
		dist(0),
		type(0)
	{
		memset(normal, 0, sizeof(normal));
	}

	Float normal[3];
	Float dist;

	Int32 type;
};

struct dpbspv3node_t
{
	dpbspv3node_t():
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

struct dpbspv3clipnode_t
{
	dpbspv3clipnode_t():
		planenum(0)
	{
		memset(children, 0, sizeof(children));
	}

	Int32 planenum;
	Int32 children[2];
};

struct dpbspv3texinfo_t
{
	dpbspv3texinfo_t():
		miptex(0),
		flags(0)
	{
		memset(vecs, 0, sizeof(vecs));
	}

	Float vecs[2][4];
	Int32 miptex;
	Int32 flags;
};

struct dpbspv3edge_t
{
	Uint32 vertexes[2];
};

struct dpbspv3face_t
{
	dpbspv3face_t():
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

	byte lmstyles[PBSPV3_MAX_LIGHTMAPS];
	Int32 lightoffset;
};

struct dpbspv3leaf_t
{
	dpbspv3leaf_t():
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

	byte ambient_level[PBSPV3_NUM_AMBIENTS];
};

struct dpbspv3lmapdata_t
{
	dpbspv3lmapdata_t():
		compression(0),
		compressionlevel(0),
		dataoffset(0),
		datasize(0),
		noncompressedsize(0)
	{}

	Int32 compression;
	Int32 compressionlevel;
	Int32 dataoffset;
	Int32 datasize;
	Int32 noncompressedsize;
};
#endif //PBSPV3FILE_H