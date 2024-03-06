/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef VBMFORMAT_H
#define VBMFORMAT_H

/************************
	Definitions

************************/
#define MAX_VBM_FLEXES				64
#define	MAX_VBM_FLEXVERTS			8192
#define MAX_VBM_VERTEXES			262144
#define MAX_SHADER_BONES			32
#define MAX_VBM_BONEWEIGHTS			4
#define MAX_VBM_LODS				16
#define MAX_SUBMODEL_NAME_LENGTH	32

#define VBM_FLEXTEXTURE_SIZE		128

#define VBM_HEADER					(('1'<<24)+('M'<<16)+('B'<<8)+'V')

struct vbmvertex_t;
struct vbmmesh_t;
struct vbmlod_t;
struct vbmsubmodel_t;
struct vbmbodypart_t;
struct vbmheader_t;
struct vbmflexcontroller_t;
struct vbmboneinfo_t;
struct vbmflexinfo_t;
struct vbmcontroller_t;
struct vbmtexture_t;
struct vbmflexvertinfo_t;
struct vbmflexvertex_t;

/************************
	Flags for VBM file

************************/
#define VBM_HAS_FLEXES	1

/************************
	Flags for VBM file

************************/
#define FL_VBM_TEXTURE_BLEND 1

/************************
	Enums for VBM format

************************/
enum vbmcontroller_type_t
{
	CONTROLLER_HEAD_XY = 0,
	CONTROLLER_R_EYE_XY,
	CONTROLLER_L_EYE_XY,
};

enum vbmlod_type_t
{
	VBM_LOD_NONE = -1,
	VBM_LOD_DISTANCE,
	VBM_LOD_SHADOW
};

enum vbmflexinterp_t
{
	VBM_FLEX_SINE = 0,
	VBM_FLEX_LINEAR,
};

/************************
	File format structures

************************/
struct vbmvertex_t
{
	vbmvertex_t():
		flexvertindex(0)
	{
		memset(texcoord, 0, sizeof(texcoord));
		memset(boneindexes, 0, sizeof(boneindexes));
		memset(boneweights, 0, sizeof(boneweights));
		memset(tangent, 0, sizeof(tangent));
	}

	Vector origin;
	Vector normal;
	vec4_t tangent;
	Float texcoord[2];
	Int16 flexvertindex;
	byte boneindexes[MAX_VBM_BONEWEIGHTS];
	byte boneweights[MAX_VBM_BONEWEIGHTS];
};

struct vbmmesh_t
{
	vbmmesh_t():
		boneoffset(0),
		numbones(0),
		skinref(0),
		start_index(0),
		num_indexes(0)
		{}

	const byte* getBones( const vbmheader_t* phdr ) const
	{
		assert(numbones > 0);
		return reinterpret_cast<const byte*>(reinterpret_cast<const byte*>(phdr) + boneoffset);
	}

	Int32 boneoffset;
	Int32 numbones;

	Int32 skinref;
	Int32 start_index;
	Int32 num_indexes;
};

struct vbmlod_t
{
	vbmlod_t():
		type(VBM_LOD_NONE),
		submodeloffset(0),
		distance(0)
		{}

	const vbmsubmodel_t* getSubmodel( const vbmheader_t* phdr ) const
	{
		return reinterpret_cast<const vbmsubmodel_t*>(reinterpret_cast<const byte*>(phdr) + submodeloffset);
	}

	vbmsubmodel_t* getSubmodel( vbmheader_t* phdr )
	{
		return reinterpret_cast<vbmsubmodel_t*>(reinterpret_cast<byte*>(phdr) + submodeloffset);
	}

	vbmlod_type_t type;
	Int32 submodeloffset;
	
	Float distance;
};

struct vbmsubmodel_t
{
	vbmsubmodel_t():
		meshoffset(0),
		nummeshes(0),
		flexinfoindex(0),
		lodoffset(0),
		numlods(0)
	{
		memset(name, 0, sizeof(name));
	}

	const vbmmesh_t* getMesh( const vbmheader_t* phdr, Int32 index ) const
	{
		assert(index >= 0 && index < nummeshes);
		return reinterpret_cast<const vbmmesh_t*>(reinterpret_cast<const byte*>(phdr) + meshoffset) + index;
	}

	vbmmesh_t* getMesh( vbmheader_t* phdr, Int32 index )
	{
		assert(index >= 0 && index < nummeshes);
		return reinterpret_cast<vbmmesh_t*>(reinterpret_cast<byte*>(phdr) + meshoffset) + index;
	}

	const vbmlod_t* getLOD( const vbmheader_t* phdr, Int32 index ) const
	{
		assert(index >= 0 && index < numlods);
		return reinterpret_cast<const vbmlod_t*>(reinterpret_cast<const byte*>(phdr) + lodoffset) + index;
	}

	vbmlod_t* getLOD( vbmheader_t* phdr, Int32 index )
	{
		assert(index >= 0 && index < numlods);
		return reinterpret_cast<vbmlod_t*>(reinterpret_cast<byte*>(phdr) + lodoffset) + index;
	}

	Char name[32];

	Int32 meshoffset;
	Int32 nummeshes;

	Int32 flexinfoindex;

	Int32 lodoffset;
	Int32 numlods;
};

struct vbmbodypart_t
{
	vbmbodypart_t():
		base(0),
		numsubmodels(0),
		submodeloffset(0)
	{
		memset(name, 0, sizeof(name));
	}

	const vbmsubmodel_t* getSubmodel( const vbmheader_t* phdr, Int32 index ) const
	{
		assert(index >= 0 && index < numsubmodels);
		return reinterpret_cast<const vbmsubmodel_t*>(reinterpret_cast<const byte*>(phdr) + submodeloffset) + index;
	}

	vbmsubmodel_t* getSubmodel( vbmheader_t* phdr, Int32 index )
	{
		assert(index >= 0 && index < numsubmodels);
		return reinterpret_cast<vbmsubmodel_t*>(reinterpret_cast<byte*>(phdr) + submodeloffset) + index;
	}

	Char name[32];
	Int32 base;

	Int32 numsubmodels;
	Int32 submodeloffset;
};

struct vbmflexcontroller_t
{
	vbmflexcontroller_t():
		interpmode(VBM_FLEX_SINE),
		minvalue(0),
		maxvalue(0)
	{
		memset(name, 0, sizeof(name));
	}

	Char name[32];

	vbmflexinterp_t interpmode;

	Float minvalue;
	Float maxvalue;
};

struct vbmboneinfo_t
{
	vbmboneinfo_t():
		flags(0),
		index(0),
		parentindex(0)
	{
		memset(name, 0, sizeof(name));
		memset(scale, 0, sizeof(scale));
		memset(bindtransform, 0, sizeof(bindtransform));
	}

	Char name[64];

	Int32 flags;
	Int32 index;
	Int32 parentindex;

	Vector position;
	Vector angles;

	Float scale[6];

	Float bindtransform[3][4];
};

struct vbmflexinfo_t
{
	vbmflexinfo_t():
		type(0),
		flexvertoffset(0),
		numflexvert(0),
		flexvertinfooffset(0),
		numflexvertinfo(0),
		flexcontrolleridxoffset(0),
		numflexes(0),
		first_vertex(0),
		num_vertexes(0)
		{}

	const vbmflexvertex_t* getFlexVertexes( const vbmheader_t* phdr ) const
	{
		return reinterpret_cast<const vbmflexvertex_t*>(reinterpret_cast<const byte*>(phdr) + flexvertoffset);
	}

	const vbmflexvertinfo_t* getFlexVertexInfos( const vbmheader_t* phdr ) const
	{
		return reinterpret_cast<const vbmflexvertinfo_t*>(reinterpret_cast<const byte*>(phdr) + flexvertinfooffset);
	}

	const byte* getFlexControllerIndexes( const vbmheader_t* phdr ) const
	{
		return reinterpret_cast<const byte*>(reinterpret_cast<const byte*>(phdr) + flexcontrolleridxoffset);
	}

	Int32 type;

	Int32 flexvertoffset;
	Int32 numflexvert;

	Int32 flexvertinfooffset;
	Int32 numflexvertinfo;

	Int32 flexcontrolleridxoffset;
	Int32 numflexes;

	Int32 first_vertex;
	Int32 num_vertexes;
};

struct vbmcontroller_t
{
	vbmcontroller_t():
		type(0),
		bone(0),
		range_xmin(0),
		range_xmax(0),
		range_ymin(0),
		range_ymax(0)
		{}

	Int32 type;
	Int32 bone;

	Vector left;
	Vector forward;

	Float range_xmin;
	Float range_xmax;

	Float range_ymin;
	Float range_ymax;
};

struct vbmtexture_t
{
	vbmtexture_t():
		flags(0),
		width(0),
		height(0),
		index(0),
		value(0)
	{
		memset(name, 0, sizeof(name));
	}

	Char name[64];
	Int32 flags;

	Int32 width;
	Int32 height;
	
	Int32 index;

	Float value;
};

struct vbmflexvertinfo_t
{
	vbmflexvertinfo_t()
	{
		memset(vertinfoindexes, 0, sizeof(vertinfoindexes));
	}

	Int32 vertinfoindexes[MAX_VBM_FLEXES];
};

struct vbmflexvertex_t
{
	vbmflexvertex_t():
		offset(0)
		{}

	Int32 offset;
	Vector originoffset;
	Vector normaloffset;
};

struct vbmheader_t
{
	vbmheader_t():
		id(0),
		flags(0),
		vertexoffset(0),
		numverts(0),
		indexoffset(0),
		numindexes(0),
		bodypartoffset(0),
		numbodyparts(0),
		textureoffset(0),
		numtextures(0),
		numflexinfo(0),
		flexinfooffset(0),
		numflexcontrollers(0),
		flexcontrolleroffset(0),
		controlleroffset(0),
		numcontrollers(0),
		boneinfooffset(0),
		numboneinfo(0),
		skinoffset(0),
		numskinref(0),
		numskinfamilies(0),
		size(0),
		ibooffset(0),
		vbooffset(0)
	{
		memset(name, 0, sizeof(name));
	}

	const vbmvertex_t* getVertexes( void ) const
	{
		assert(vertexoffset > 0);
		return reinterpret_cast<const vbmvertex_t*>(reinterpret_cast<const byte*>(this) + vertexoffset);
	}

	const Uint32* getIndexes( void ) const
	{
		assert(indexoffset > 0);
		return reinterpret_cast<const Uint32*>(reinterpret_cast<const byte*>(this) + indexoffset);
	}

	const vbmbodypart_t* getBodyPart( Int32 index ) const
	{
		assert(index >= 0 && index < numbodyparts);
		return reinterpret_cast<const vbmbodypart_t*>(reinterpret_cast<const byte*>(this) + bodypartoffset) + index;
	}

	vbmbodypart_t* getBodyPart( Int32 index )
	{
		assert(index >= 0 && index < numbodyparts);
		return reinterpret_cast<vbmbodypart_t*>(reinterpret_cast<byte*>(this) + bodypartoffset) + index;
	}

	const vbmtexture_t* getTexture( Int32 index ) const
	{
		assert(index >= 0 && index < numtextures);
		return reinterpret_cast<const vbmtexture_t*>(reinterpret_cast<const byte*>(this) + textureoffset) + index;
	}

	vbmtexture_t* getTexture( Int32 index )
	{
		assert(index >= 0 && index < numtextures);
		return reinterpret_cast<vbmtexture_t*>(reinterpret_cast<byte*>(this) + textureoffset) + index;
	}

	const vbmflexinfo_t* getFlexInfo( Int32 index ) const
	{
		assert(index >= 0 && index < numflexinfo);
		return reinterpret_cast<const vbmflexinfo_t*>(reinterpret_cast<const byte*>(this) + flexinfooffset) + index;
	}

	const vbmflexcontroller_t* getFlexController( Int32 index ) const
	{
		assert(index >= 0 && index < numflexcontrollers);
		return reinterpret_cast<const vbmflexcontroller_t*>(reinterpret_cast<const byte*>(this) + flexcontrolleroffset) + index;
	}

	const vbmflexcontroller_t* getFlexControllers( void ) const
	{
		assert(flexcontrolleroffset > 0);
		return reinterpret_cast<const vbmflexcontroller_t*>(reinterpret_cast<const byte*>(this) + flexcontrolleroffset);
	}

	const vbmcontroller_t* getController( Int32 index ) const
	{
		assert(index >= 0 && index < numcontrollers);
		return reinterpret_cast<const vbmcontroller_t*>(reinterpret_cast<const byte*>(this) + controlleroffset) + index;
	}

	const vbmboneinfo_t* getBoneInfo( Int32 index ) const
	{
		assert(index >= 0 && index < numboneinfo);
		return reinterpret_cast<const vbmboneinfo_t*>(reinterpret_cast<const byte*>(this) + boneinfooffset) + index;
	}

	const Int16* getSkinFamily( Int32 index ) const
	{
		assert(index >= 0 && index < numskinfamilies);
		return reinterpret_cast<const Int16*>(reinterpret_cast<const byte*>(this) + skinoffset) + index*numskinref;
	}

	const Int16* getSkinFamilies( void ) const
	{
		assert(skinoffset > 0);
		return reinterpret_cast<const Int16*>(reinterpret_cast<const byte*>(this) + skinoffset);
	}

	Int32 id;
	Char name[128];

	Int32 flags;

	Int32 vertexoffset;
	Int32 numverts;

	Int32 indexoffset;
	Int32 numindexes;

	Int32 bodypartoffset;
	Int32 numbodyparts;

	Int32 textureoffset;
	Int32 numtextures;

	Int32 numflexinfo;
	Int32 flexinfooffset;

	Int32 numflexcontrollers;
	Int32 flexcontrolleroffset;

	Int32 controlleroffset;
	Int32 numcontrollers;

	Int32 boneinfooffset;
	Int32 numboneinfo;

	Int32 skinoffset;
	Int32 numskinref;
	Int32 numskinfamilies;

	Int32 size;
	Int32 ibooffset;
	Int32 vbooffset;
};
#endif //VBMFORMAT_H