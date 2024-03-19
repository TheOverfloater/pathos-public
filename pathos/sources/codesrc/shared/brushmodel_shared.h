/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef BRUSHMODEL_SHARED_H
#define BRUSHMODEL_SHARED_H

#include "bspv30file.h"
#include "plane.h"
#include "constants.h"

// No lightmaps for this surface
#define	TEXFLAG_SPECIAL		1

#define NO_INFO_INDEX		-1

enum bsp_lightmaps_t
{
	LM_AMBIENT_STYLE	= 61,
	LM_DIFFUSE_STYLE	= 62,
	LM_LIGHTVECS_STYLE	= 63,
};

//
// BSP file structures
//

struct lump_t
{
	lump_t():
		offset(0),
		size(0)
	{}

	Int32 offset;
	Int32 size;
};

struct dheader_t
{
	dheader_t():
		version(0)
	{
		memset(lumps, 0, sizeof(lumps));
	}

	Int32 version;
	lump_t lumps[NB_LUMPS];
};

//
// BSP structures in memory
//

enum surf_flags_t
{
	SURF_PLANEBACK			= 2,
	SURF_DRAWSKY			= 4,
	SURF_DRAWSPRITE			= 8,
	SURF_DRAWTURB			= 16,
	SURF_SHADERWATER		= 32
};

struct mvertex_t
{
	// Position in world
	Vector origin;
};

struct medge_t
{
	medge_t():
		cachededgeoffset(0)
	{
		memset(vertexes, 0, sizeof(vertexes));
	}

	// Vertex indexes
	Uint32 vertexes[2];
	Uint32 cachededgeoffset;
};

struct mtexture_t
{
	mtexture_t():
		width(0),
		height(0),
		ptexturechain(nullptr),
		anim_total(0),
		anim_min(0),
		anim_max(0),
		panim_next(nullptr),
		palt_anims(nullptr),
		infoindex(NO_INFO_INDEX)
	{}

	// Texture name
	CString name;

	// Internal width/height
	Uint16 width;
	Uint16 height;

	// Texchain used for rendering
	struct msurface_t *ptexturechain;

	// Texture animations
	Int32 anim_total;
	Int32 anim_min;
	Int32 anim_max;

	mtexture_t* panim_next;
	mtexture_t* palt_anims;
	
	// render info index
	Int32 infoindex;
};

struct mtexinfo_t
{
	mtexinfo_t():
		ptexture(nullptr),
		flags(0)
	{
		memset(vecs, 0, sizeof(vecs));
	}

	// unit vectors in world space
	Float vecs[2][4];
	// Pointer to material
	mtexture_t* ptexture;

	// texture flags
	Int64 flags;
};

struct mnode_t
{
	mnode_t():
		contents(0),
		visframe(0),
		pparent(nullptr),
		pplane(nullptr),
		firstsurface(0),
		numsurfaces(0)
	{
		memset(pchildren, 0, sizeof(pchildren));
	}

	// Node contents
	Int32 contents;
	// Visible if current
	Int32 visframe;

	// Bounding box
	Vector mins;
	Vector maxs;

	// Parent node
	mnode_t* pparent;

	// Node specifics start here
	plane_t* pplane;
	// Child nodes
	mnode_t* pchildren[2];

	// First surface
	Uint32 firstsurface;
	// Number of surfaces
	Uint32 numsurfaces;
};

struct mleaf_t
{
	mleaf_t():
		contents(0),
		visframe(0),
		pparent(nullptr),
		pcompressedvis(nullptr),
		pcompressedpas(nullptr),
		pfirstmarksurface(nullptr),
		nummarksurfaces(0)
	{
	}

	// Node contents
	Int32 contents;
	// Visible if current
	Int32 visframe;

	// Bounding box
	Vector mins;
	Vector maxs;

	// Parent node
	mnode_t* pparent;

	// Leaf specifics start here
	// Compressed VIS data pointer
	byte* pcompressedvis;
	// PAS data pointer
	byte* pcompressedpas;

	// Marksurfaces pointer
	msurface_t** pfirstmarksurface;
	// Number of marksurfaces
	Uint32 nummarksurfaces;
};

struct msurface_t
{
	msurface_t():
		visframe(0),
		pplane(nullptr),
		flags(0),
		firstedge(0),
		numedges(0),
		light_s(0),
		light_t(0),
		ptexinfo(nullptr),
		psamples(nullptr),
		lightoffset(0),
		ptexturechain(nullptr),
		infoindex(-1)
	{
		memset(texturemins, 0, sizeof(texturemins));
		memset(extents, 0, sizeof(extents));
	}

	// Visframe this was drawn on
	Int32 visframe;

	// Plane tied to this node
	plane_t* pplane;
	// Flags for the surface
	Int64 flags;
	
	// Starting edge index
	Uint32 firstedge;
	// Number of edges
	Uint32 numedges;

	// Texture min/max values
	Int32 texturemins[2];
	// Surface extents
	Int32 extents[2];

	// Lightmap S coord
	Uint32 light_s;
	// Lightmap T coord
	Uint32 light_t;

	// texinfo
	mtexinfo_t* ptexinfo;

	// For the bounding box
	Vector mins;
	Vector maxs;

	// lightmap styles
	// made this an array as part
	// of getting rid of fixed size
	// arrays. will depend on bsp
	// loader logic from now on
	CArray<byte> styles;
	// Pointer to lightmap samples
	color24_t* psamples;
	// original offset value into samples
	Int32 lightoffset;

	// texture chains
	msurface_t* ptexturechain;

	// info index for rendering
	Int32 infoindex;
};

struct mclipnode_t
{
	mclipnode_t():
		planenum(0)
	{
		memset(children, 0, sizeof(children));
	}

	Int32 planenum;
	Int32 children[2];
};

struct mmodel_t
{
	mmodel_t():
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

	Int32 headnode[MAX_MAP_HULLS];
	Int32 visleafs;

	Int32 firstface;
	Int32 numfaces;
};

struct hull_t
{
	hull_t():
		pclipnodes(nullptr),
		pplanes(nullptr),
		firstclipnode(0),
		lastclipnode(0)
	{}

	mclipnode_t* pclipnodes;
	plane_t* pplanes;

	Int32 firstclipnode;
	Int32 lastclipnode;

	Vector clipmins;
	Vector clipmaxs;
};

struct brushmodel_t
{
	brushmodel_t():
		version(0),
		freedata(false),
		radius(0),
		firstmodelsurface(0),
		nummodelsurfaces(0),
		psubmodels(nullptr),
		numsubmodels(0),
		pplanes(nullptr),
		numplanes(0),
		pleafs(nullptr),
		numleafs(0),
		pvertexes(nullptr),
		numvertexes(0),
		pedges(nullptr),
		numedges(0),
		pnodes(nullptr),
		numnodes(0),
		ptexinfos(nullptr),
		numtexinfos(0),
		psurfaces(nullptr),
		numsurfaces(0),
		psurfedges(nullptr),
		numsurfedges(0),
		pclipnodes(nullptr),
		numclipnodes(0),
		pmarksurfaces(nullptr),
		nummarksurfaces(0),
		ptextures(nullptr),
		numtextures(0),
		pvisdata(nullptr),
		visdatasize(0),
		ppasdata(nullptr),
		pasdatasize(0),
		plightdata(nullptr),
		lightdatasize(0),
		pentdata(nullptr),
		entdatasize(0)
	{
	}

	~brushmodel_t()
	{
		if(freedata)
		{
			if(psubmodels) 
				delete[] psubmodels;
			if(pplanes) 
				delete[] pplanes;
			if(pleafs) 
				delete[] pleafs;
			if(pvertexes) 
				delete[] pvertexes;
			if(pedges) 
				delete[] pedges;
			if(pnodes) 
				delete[] pnodes;
			if(ptexinfos) 
				delete[] ptexinfos;
			if(psurfaces) 
				delete[] psurfaces;
			if(psurfedges) 
				delete[] psurfedges;
			if(pclipnodes) 
				delete[] pclipnodes;
			if(pmarksurfaces) 
				delete[] pmarksurfaces;
			if(ptextures) 
				delete[] ptextures;
			if(pvisdata) 
				delete[] pvisdata;
			if(ppasdata) 
				delete[] ppasdata;
			if(plightdata)
				delete[] plightdata;
			if(pentdata) 
				delete[] pentdata;
			if(hulls[0].pclipnodes)
				delete[] hulls[0].pclipnodes;
		}
	}

	// Model name with path
	CString name;
	// BSP version
	Int32 version;
	// Tells if we should free our data
	bool freedata;

	// for bounding boxes
	Vector mins;
	Vector maxs;
	Float radius;

	// surface offset for bmodels
	Int32 firstmodelsurface;
	// number of surfaces for bmodels
	Uint32 nummodelsurfaces;

	// submodels(for world only)
	mmodel_t* psubmodels;
	Uint32 numsubmodels;

	// planes
	plane_t* pplanes;
	Uint32 numplanes;

	// leafs
	mleaf_t* pleafs;
	Uint32 numleafs;

	// vertexes
	mvertex_t* pvertexes;
	Uint32 numvertexes;

	// edges
	medge_t* pedges;
	Uint32 numedges;

	// nodes
	mnode_t* pnodes;
	Uint32 numnodes;

	// texinfos
	mtexinfo_t* ptexinfos;
	Uint32 numtexinfos;

	// surfaces
	msurface_t* psurfaces;
	Uint32 numsurfaces;

	// surfedges
	Int32* psurfedges;
	Uint32 numsurfedges;

	// clipnodes
	mclipnode_t* pclipnodes;
	Int32 numclipnodes;

	// marksurfaces
	msurface_t** pmarksurfaces;
	Uint32 nummarksurfaces;

	// clipping hulls
	hull_t hulls[MAX_MAP_HULLS];

	// Textures
	mtexture_t* ptextures;
	Uint32 numtextures;

	// VIS data
	byte* pvisdata;
	Uint32 visdatasize;

	// PAS data
	byte *ppasdata;
	Uint32 pasdatasize;

	// light data
	color24_t* plightdata;
	Uint32 lightdatasize;
	
	// entities
	Char* pentdata;
	Uint32 entdatasize;
};
#endif //BRUSHMODEL_SHARED_H