/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MCDFORMAT_H
#define MCDFORMAT_H

#include "constants.h"
#include "plane.h"

// MCD file id
static const Int32 MCD_FORMAT_HEADER = (('D'<<24)+('C'<<16)+('M'<<8)+'P');
// MCD file version
static const Int32 MCD_FORMAT_VERSION = 1;

struct mcdheader_t;
struct mcdbvhnode_t;
struct mcdsubmodel_t;

enum mcdcollisiontype_t
{
	MCD_COLLISION_NULL = -1,
	MCD_COLLISION_TRIANGLES,		// The pure triangle data
	MCD_COLLISION_BVH,				// Bounding volume hierarchy information

	// Must be last
	NB_MCD_COLLISION_TYPES,
};

struct mcdcollisiontypemodel_t
{
	mcdcollisiontypemodel_t():
		type(MCD_COLLISION_NULL),
		dataoffset(0)
	{}

	const void* getTypeData( const mcdheader_t* phdr ) const
	{
		assert(phdr != nullptr);
		return reinterpret_cast<const byte*>(phdr) + dataoffset;
	}

	void* getTypeData( mcdheader_t* phdr ) const
	{
		assert(phdr != nullptr);
		return reinterpret_cast<byte*>(phdr) + dataoffset;
	}

	mcdcollisiontype_t type;
	Uint32 dataoffset;
};

struct mcdsubmodel_t
{
	mcdsubmodel_t():
		collisiontypesoffset(0),
		numcollisiontypes(0)
	{
		memset(name, 0, sizeof(name));
	}

	const void* getTypeData( const mcdheader_t* phdr, mcdcollisiontype_t type ) const
	{
		assert(phdr != nullptr);
		assert(type >= MCD_COLLISION_TRIANGLES && type < NB_MCD_COLLISION_TYPES);
		if(type >= numcollisiontypes)
			return nullptr;

		const mcdcollisiontypemodel_t* ptypemodel = reinterpret_cast<const mcdcollisiontypemodel_t*>(reinterpret_cast<const byte*>(phdr) + collisiontypesoffset) + (Int32)type;
		assert(ptypemodel->type == type);
		if(type != type)
			return nullptr;
		else
			return ptypemodel->getTypeData(phdr);
	}

	void* getTypeData( mcdheader_t* phdr, mcdcollisiontype_t type )
	{
		assert(phdr != nullptr);
		assert(type >= MCD_COLLISION_TRIANGLES && type < NB_MCD_COLLISION_TYPES);
		if(type >= numcollisiontypes)
			return nullptr;

		mcdcollisiontypemodel_t* ptypemodel = reinterpret_cast<mcdcollisiontypemodel_t*>(reinterpret_cast<byte*>(phdr) + collisiontypesoffset) + (Int32)type;
		assert(ptypemodel->type == type);
		if(type != type)
			return nullptr;
		else
			return ptypemodel->getTypeData(phdr);
	}

	Char name[128];

	Vector mins;
	Vector maxs;

	Uint32 collisiontypesoffset;
	Uint32 numcollisiontypes;
};

struct mcdbodypart_t
{
	mcdbodypart_t():
		base(0),
		submodeloffset(0),
		numsubmodels(0)
	{
		memset(name, 0, sizeof(name));
	}

	const mcdsubmodel_t* getSubmodel( const mcdheader_t* phdr, Int32 index ) const
	{
		assert(index >= 0 && index < numsubmodels);
		return reinterpret_cast<const mcdsubmodel_t*>(reinterpret_cast<const byte*>(phdr) + submodeloffset) + index;
	}

	mcdsubmodel_t* getSubmodel( mcdheader_t* phdr, Int32 index ) const
	{
		assert(index >= 0 && index < numsubmodels);
		return reinterpret_cast<mcdsubmodel_t*>(reinterpret_cast<byte*>(phdr) + submodeloffset) + index;
	}

	Char name[128];

	Uint32 base;
	Uint32 submodeloffset;
	Uint32 numsubmodels;
};

struct mcdbone_t
{
	mcdbone_t():
		parentindex(NO_POSITION)
	{
		memset(name, 0, sizeof(name));
	}

	Char name[64];
	Vector position;
	Vector rotation;
	Int32 parentindex;
};

struct mcdvertex_t
{
	mcdvertex_t():
		boneindex(NO_POSITION)
	{}

	Vector origin;
	Int32 boneindex;
};

struct mcdtrimeshtriangle_t
{
	mcdtrimeshtriangle_t():
		skinref(NO_POSITION),
		distance(0),
		planetype(PLANE_AZ),
		signbits(0)
	{
		for(Uint32 i = 0; i < 3; i++)
			trivertexes[i] = 0;
	}

	Int32 skinref;
	Vector normal;
	Float distance;
	Int32 planetype;
	Int32 signbits;
	Uint32 trivertexes[3];
};

struct mcdtrimeshtype_t
{
	mcdtrimeshtype_t():
		triangleoffset(0),
		numtriangles(0),
		vertexoffset(0),
		numvertexes(0)
	{}

	const mcdtrimeshtriangle_t* getTriangles( const mcdheader_t* phdr ) const
	{
		assert(phdr != nullptr);
		return reinterpret_cast<const mcdtrimeshtriangle_t*>(reinterpret_cast<const byte*>(phdr) + triangleoffset);
	}

	mcdtrimeshtriangle_t* getTriangles( mcdheader_t* phdr ) const
	{
		assert(phdr != nullptr);
		return reinterpret_cast<mcdtrimeshtriangle_t*>(reinterpret_cast<byte*>(phdr) + triangleoffset);
	}

	const mcdvertex_t* getVertexes( const mcdheader_t* phdr ) const
	{
		assert(phdr != nullptr);
		return reinterpret_cast<const mcdvertex_t*>(reinterpret_cast<const byte*>(phdr) + vertexoffset);
	}

	mcdvertex_t* getVertexes( mcdheader_t* phdr ) const
	{
		assert(phdr != nullptr);
		return reinterpret_cast<mcdvertex_t*>(reinterpret_cast<byte*>(phdr) + vertexoffset);
	}

	Uint32 triangleoffset;
	Uint32 numtriangles;

	Uint32 vertexoffset;
	Uint32 numvertexes;
};

struct mcdbvhnode_t
{
	mcdbvhnode_t():
		index(NO_POSITION),
		isleaf(false),
		triindexoffset(0),
		numtriangles(0)
	{
		for(Uint32 i = 0; i < 2; i++)
			children[i] = NO_POSITION;
	}

	const Int32* getTriangleIndexes( const mcdheader_t* phdr ) const
	{
		assert(phdr != nullptr);
		return reinterpret_cast<const Int32*>(reinterpret_cast<const byte*>(phdr) + triindexoffset);
	}

	Int32* getTriangleIndexes( mcdheader_t* phdr )
	{
		assert(phdr != nullptr);
		return reinterpret_cast<Int32*>(reinterpret_cast<byte*>(phdr) + triindexoffset);
	}

	Int32 index;
	Vector mins;
	Vector maxs;
	Int32 children[2];
	bool isleaf;

	Uint32 triindexoffset;
	Uint32 numtriangles;
};

struct mcdbvhtype_t
{
	mcdbvhtype_t():
		bvhnodeoffset(0),
		numbvhnodes(0)
	{}

	const mcdbvhnode_t* getNode( const mcdheader_t* phdr, Int32 index ) const
	{
		assert(phdr != nullptr);
		assert(index >= 0 && index < numbvhnodes);
		return reinterpret_cast<const mcdbvhnode_t*>(reinterpret_cast<const byte*>(phdr) + bvhnodeoffset) + index;
	}

	mcdbvhnode_t* getNode( mcdheader_t* phdr, Int32 index )
	{
		assert(phdr != nullptr);
		assert(index >= 0 && index < numbvhnodes);
		return reinterpret_cast<mcdbvhnode_t*>(reinterpret_cast<byte*>(phdr) + bvhnodeoffset) + index;
	}

	Uint32 bvhnodeoffset;
	Uint32 numbvhnodes;
};

struct mcdtexture_t
{
	mcdtexture_t():
		dataptr(0),
		index(NO_POSITION)
	{
		memset(name, 0, sizeof(name));
	}

	Char name[128];
	Uint64 dataptr;
	Int32 index;
};

struct mcdheader_t
{
	mcdheader_t():
		id(0),
		version(0),
		bodypartoffset(0),
		numbodyparts(0),
		textureoffset(0),
		numtextures(0),
		boneoffset(0),
		numbones(0),
		size(0)
	{
		memset(name, 0, sizeof(name));
	}

	const mcdbodypart_t* getBodyPart( Int32 index ) const
	{
		assert(index >= 0 && index < numbodyparts);
		return reinterpret_cast<const mcdbodypart_t*>(reinterpret_cast<const byte*>(this) + bodypartoffset) + index;
	}

	mcdbodypart_t* getBodyPart( Int32 index )
	{
		assert(index >= 0 && index < numbodyparts);
		return reinterpret_cast<mcdbodypart_t*>(reinterpret_cast<byte*>(this) + bodypartoffset) + index;
	}

	const mcdtexture_t* getTexture( Int32 index ) const
	{
		assert(index >= 0 && index < numtextures);
		return reinterpret_cast<const mcdtexture_t*>(reinterpret_cast<const byte*>(this) + textureoffset) + index;
	}

	mcdtexture_t* getTexture( Int32 index )
	{
		assert(index >= 0 && index < numtextures);
		return reinterpret_cast<mcdtexture_t*>(reinterpret_cast<byte*>(this) + textureoffset) + index;
	}
	
	const mcdbone_t* getBoneInfo( Int32 index ) const
	{
		assert(index >= 0 && index < numbones);
		return reinterpret_cast<const mcdbone_t*>(reinterpret_cast<const byte*>(this) + boneoffset) + index;
	}

	mcdbone_t* getBoneInfo( Int32 index )
	{
		assert(index >= 0 && index < numbones);
		return reinterpret_cast<mcdbone_t*>(reinterpret_cast<byte*>(this) + boneoffset) + index;
	}

	Int32 id;
	Int32 version;

	Char name[256];

	Uint32 bodypartoffset;
	Uint32 numbodyparts;

	Uint32 textureoffset;
	Uint32 numtextures;

	Uint32 boneoffset;
	Uint32 numbones;

	Uint32 size;
};
#endif