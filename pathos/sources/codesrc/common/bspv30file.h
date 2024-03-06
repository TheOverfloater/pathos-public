/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2020
All Rights Reserved.
===============================================
*/

#ifndef BSPV30FILE_H
#define BSPV30FILE_H

#include "contents.h"

//
// BSP limits
//

#define V30_MAX_MAP_HULLS			4

#define V30_MAX_MAP_MODELS			1024
#define V30_MAX_MAP_BRUSHES			65536
#define V30_MAX_MAP_ENTITIES		8192
#define V30_MAX_MAP_ENTSTRING		65536

#define V30_MAX_MAP_PLANES			32768
#define V30_MAX_MAP_NODES			32768
#define V30_MAX_MAP_CLIPNODES		32768
#define V30_MAX_MAP_LEAFS			32768
#define V30_MAX_MAP_VERTS			65536
#define V30_MAX_MAP_FACES			65536
#define V30_MAX_MAP_MARKSURFACES	65536
#define V30_MAX_MAP_TEXINFO			32768
#define V30_MAX_MAP_EDGES			262144
#define V30_MAX_MAP_SURFEDGES		524288
#define V30_MAX_MAP_TEXTURES		4096
#define V30_MAX_MAP_LIGHTING		16777216
#define V30_MAX_MAP_VISIBILITY		16777216

#define V30_MAX_LIGHTMAPS			4
#define V30_LM_BASE_SAMPLE_SIZE		16

#define V30_NUM_AMBIENTS			4

//
// BSP hulls
//
enum bspv30_hulls_t
{
	POINT_HULL,
	HUMAN_HULL,
	LARGE_HULL,
	HEAD_HULL
};

//
// BSP lumps
//
enum bspv30_lumps_t
{
	LUMP_ENTITIES = 0,
	LUMP_PLANES,
	LUMP_TEXTURES,
	LUMP_VERTEXES,
	LUMP_VISIBILITY,
	LUMP_NODES,
	LUMP_TEXINFO,
	LUMP_FACES,
	LUMP_LIGHTING,
	LUMP_CLIPNODES,
	LUMP_LEAFS,
	LUMP_MARKSURFACES,
	LUMP_EDGES,
	LUMP_SURFEDGES,
	LUMP_MODELS,
	NB_LUMPS
};

//
// BSP file structures
//
struct dv30model_t
{
	dv30model_t():
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

	Int32 headnode[V30_MAX_MAP_HULLS];
	Int32 visleafs;

	Int32 firstface;
	Int32 numfaces;
};

struct dv30vertex_t
{
	dv30vertex_t()
	{
		memset(origin, 0, sizeof(origin));
	}

	Float origin[3];
};

struct dv30plane_t
{
	dv30plane_t():
		dist(0),
		type(0)
	{
		memset(normal, 0, sizeof(normal));
	}

	Float normal[3];
	Float dist;

	Int32 type;
};

struct dv30node_t
{
	dv30node_t():
		planenum(0),
		firstface(0),
		numfaces(0)
	{
		memset(children, 0, sizeof(children));
		memset(mins, 0, sizeof(mins));
		memset(maxs, 0, sizeof(maxs));
	}

	Int32 planenum;
	Int16 children[2];
	Int16 mins[3];
	Int16 maxs[3];

	Uint16 firstface;
	Uint16 numfaces;
};

struct dv30clipnode_t
{
	dv30clipnode_t():
		planenum(0)
	{
		memset(children, 0, sizeof(children));
	}

	Int32 planenum;
	Int16 children[2];
};

struct dv30texinfo_t
{
	dv30texinfo_t():
		miptex(0),
		flags(0)
	{
		memset(vecs, 0, sizeof(vecs));
	}

	Float vecs[2][4];
	Int32 miptex;
	Int32 flags;
};

struct dv30edge_t
{
	Uint16 vertexes[2];
};

struct dv30face_t
{
	dv30face_t():
		planenum(0),
		side(0),
		firstedge(0),
		numedges(0),
		texinfo(0),
		lightoffset(0)
	{
		memset(lmstyles, 0, sizeof(lmstyles));
	}

	Uint16 planenum;
	Int16 side;

	Int32 firstedge;
	Int16 numedges;
	Int16 texinfo;

	byte lmstyles[V30_MAX_LIGHTMAPS];
	Int32 lightoffset;
};

struct dv30leaf_t
{
	dv30leaf_t():
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

	Uint16 firstmarksurface;
	Uint16 nummarksurfaces;

	byte ambient_level[V30_NUM_AMBIENTS];
};
#endif //BSPV30FILE_H