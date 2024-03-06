/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#ifndef STUDIO_H
#define STUDIO_H

#include "datatypes.h"
#include "vbmformat.h"

// Notes:
// Part of this implementation is based on the implementation in the Half-Life SDK
// The studiomodel format is Valve's original work, and I take no ownership of it
// No copyright infringement intended

#define STUDIO_VERSION	10
#define IDSTUDIOHEADER	(('T'<<24)+('S'<<16)+('D'<<8)+'I')

#define MAXSTUDIOVERTS_REF	2048
#define MAXSTUDIOTRIANGLES	16384
#define MAXSTUDIOVERTS		8196
#define MAXSTUDIOSEQUENCES	256	
#define MAXSTUDIOSKINS		100
#define MAXSTUDIOSRCBONES	256
#define MAXSTUDIOBONES		128
#define MAXSTUDIOMODELS		32
#define MAXSTUDIOBODYPARTS	32
#define MAXSTUDIOGROUPS		32
#define MAXSTUDIOANIMATIONS	2048
#define MAXSTUDIOMESHES		32
#define MAXSTUDIOEVENTS		1024
#define MAXSTUDIOPIVOTS		256
#define MAXSTUDIOCONTROLLERS 8

struct studiohdr_t;
struct mstudiobone_t;
struct mstudiobonecontroller_t;
struct mstudiobbox_t;
struct mstudioseqgroup_t;
struct mstudioseqdesc_t;
struct mstudiopivot_t;
struct mstudioattachment_t;
struct mstudioanim_t;
union mstudioanimvalue_t;
struct mstudiobodyparts_t;
struct mstudiotexture_t;
struct mstudiomodel_t;
struct mstudiomesh_t;


// bones
struct mstudiobone_t
{
	mstudiobone_t():
		parent(0),
		flags(0)
	{
		memset(name, 0, sizeof(name));
		memset(bonecontroller, 0, sizeof(bonecontroller));
		memset(value, 0, sizeof(value));
		memset(scale, 0, sizeof(scale));
	}

	Char				name[32];	// bone name for symbolic links
	Int32		 		parent;		// parent bone
	Int32				flags;		// ??
	Int32				bonecontroller[6];	// bone controller index, -1 == none
	Float				value[6];	// default DoF values
	Float				scale[6];   // scale for delta DoF values
};


// bone controllers
struct mstudiobonecontroller_t
{
	mstudiobonecontroller_t():
		bone(0),
		type(0),
		start(0),
		end(0),
		rest(0),
		index(0)
		{}

	Int32				bone;	// -1 == 0
	Int32				type;	// X, Y, Z, XR, YR, ZR, M
	Float				start;
	Float				end;
	Int32				rest;	// byte index value at rest
	Int32				index;	// 0-3 user set controller, 4 mouth
};

// intersection boxes
struct mstudiobbox_t
{
	mstudiobbox_t():
		bone(0),
		group(0)
		{}

	Int32				bone;
	Int32				group;			// intersection group
	Vector				bbmin;		// bounding box
	Vector				bbmax;		
};

struct cache_user_t
{
	cache_user_t():
		data(0)
		{}

	// This fucks up on 64-bit, must be 32-bit
	Int32 data;
};

struct mstudioseqgroup_t
{
	mstudioseqgroup_t():
		data(0)
	{
		memset(label, 0, sizeof(label));
		memset(name, 0, sizeof(name));
	}
	Char				label[32];	// textual name
	Char				name[64];	// file name
	cache_user_t		cache;		// cache index pointer
	Int32				data;		// hack for group 0
};

struct mstudioevent_t
{
	mstudioevent_t():
		frame(0),
		event(0),
		type(0)
	{
		memset(options, 0, sizeof(options));
	}

	Int32 				frame;
	Int32				event;
	Int32				type;
	Char				options[64];
};

struct mstudioseqdesc_t
{
	mstudioseqdesc_t():
		fps(0),
		flags(0),
		activity(0),
		actweight(0),
		numevents(0),
		eventindex(0),
		numframes(0),
		numpivots(0),
		pivotindex(0),
		motiontype(0),	
		motionbone(0),
		automoveposindex(0),
		automoveangleindex(0),	
		numblends(0),
		animindex(0),
		blendparent(0),
		seqgroup(0),
		entrynode(0),
		exitnode(0),
		nodeflags(0),
		nextseq(0)
	{
		memset(label, 0, sizeof(label));
		memset(blendtype, 0, sizeof(blendtype));
		memset(blendstart, 0, sizeof(blendstart));
		memset(blendend, 0, sizeof(blendend));
	}

	const mstudioevent_t* getEvent( const studiohdr_t* phdr, Int32 index ) const
	{
		assert(index >= 0 && index < numevents);
		return reinterpret_cast<const mstudioevent_t*>(reinterpret_cast<const byte*>(phdr) + eventindex) + index;
	}

	Char				label[32];

	Float				fps;
	Int32				flags;

	Int32				activity;
	Int32				actweight;

	Int32				numevents;
	Int32				eventindex;

	Int32				numframes;

	Int32				numpivots;
	Int32				pivotindex;

	Int32				motiontype;	
	Int32				motionbone;
	Vector				linearmovement;
	Int32				automoveposindex;
	Int32				automoveangleindex;

	Vector				bbmin;
	Vector				bbmax;		

	Int32				numblends;
	Int32				animindex;

	Int32				blendtype[2];
	Float				blendstart[2];
	Float				blendend[2];
	Int32				blendparent;

	Int32				seqgroup;

	Int32				entrynode;
	Int32				exitnode;
	Int32				nodeflags;
	
	Int32				nextseq;
};

// pivots
struct mstudiopivot_t
{
	mstudiopivot_t():
		start(0),
		end(0)
		{}

	Vector				org;
	Int32				start;
	Int32				end;
};

// attachment
struct mstudioattachment_t
{
	mstudioattachment_t():
		type(0),
		bone(0)
	{
		memset(name, 0, sizeof(name));
	}

	Char				name[32];
	Int32				type;
	Int32				bone;
	Vector				org;	// attachment point
	Vector				vectors[3];
};

struct mstudioanim_t
{
	mstudioanim_t()
	{
		memset(offset, 0, sizeof(offset));
	}

	const mstudioanimvalue_t* getAnimationValue( Int32 index ) const
	{
		assert(index >= 0 && index < 6);
		return reinterpret_cast<const mstudioanimvalue_t*>(reinterpret_cast<const byte*>(this) + offset[index]);
	}

	Uint16				offset[6];
};

// animation frames
union mstudioanimvalue_t
{
	struct {
		byte	valid;
		byte	total;
	} num;
	Int16		value;
};

// skin info
struct mstudiotexture_t
{
	mstudiotexture_t():
		flags(0),
		width(0),
		height(0),
		index(0)
	{
		memset(name, 0, sizeof(name));
	}
	Char					name[64];
	Int32					flags;
	Int32					width;
	Int32					height;
	Int32					index;
};

// studio models
struct mstudiomodel_t
{
	mstudiomodel_t():
		type(0),
		boundingradius(0),
		nummesh(0),
		meshindex(0),
		numverts(0),
		vertinfoindex(0),
		vertindex(0),
		numnorms(0),
		norminfoindex(0),
		normindex(0),
		numgroups(0),
		groupindex(0)
	{
		memset(name, 0, sizeof(name));
	}
	const mstudiomodel_t* getMesh( studiohdr_t* phdr, Int32 index ) const
	{
		assert(index >= 0 && index < nummesh);
		return reinterpret_cast<const mstudiomodel_t*>(reinterpret_cast<const byte*>(phdr) + meshindex) + index;
	}

	Char				name[64];

	Int32				type;

	Float				boundingradius;

	Int32				nummesh;
	Int32				meshindex;

	Int32				numverts;
	Int32				vertinfoindex;
	Int32				vertindex;
	Int32				numnorms;
	Int32				norminfoindex;
	Int32				normindex;

	Int32				numgroups;
	Int32				groupindex;
};

// body part index
struct mstudiobodyparts_t
{
	mstudiobodyparts_t():
		nummodels(0),
		base(0),
		modelindex(0)
	{
		memset(name, 0, sizeof(name));
	}

	const mstudiomodel_t* getSubmodel( studiohdr_t* phdr, Int32 index ) const
	{
		assert(index >= 0 && index < nummodels);
		return reinterpret_cast<const mstudiomodel_t*>(reinterpret_cast<const byte*>(phdr) + modelindex) + index;
	}

	Char				name[64];
	Int32				nummodels;
	Int32				base;
	Int32				modelindex;
};


// meshes
struct mstudiomesh_t
{
	mstudiomesh_t():
		numtris(0),
		triindex(0),
		skinref(0),
		numnorms(0),
		normindex(0)
		{}

	Int32					numtris;
	Int32					triindex;
	Int32					skinref;
	Int32					numnorms;
	Int32					normindex;
};

struct studiohdr_t
{
	studiohdr_t():
		id(0),
		version(0),
		length(0),
		flags(0),
		numbones(0),
		boneindex(0),
		numbonecontrollers(0),
		bonecontrollerindex(0),
		numhitboxes(0),
		hitboxindex(0),			
		numseq(0),
		seqindex(0),
		numseqgroups(0),
		seqgroupindex(0),
		numtextures(0),
		textureindex(0),
		texturedataindex(0),
		numskinref(0),
		numskinfamilies(0),
		skinindex(0),
		numbodyparts(0),		
		bodypartindex(0),
		numattachments(0),
		attachmentindex(0),
		soundtable(0),
		soundindex(0),
		soundgroups(0),
		soundgroupindex(0),
		numtransitions(0),
		transitionindex(0)
	{
		memset(name, 0, sizeof(name));
	}

	const mstudiobone_t* getBone( Int32 index ) const
	{
		assert(index >= 0 && index < numbones);
		return reinterpret_cast<const mstudiobone_t*>(reinterpret_cast<const byte*>(this) + boneindex) + index;
	}

	const mstudiobonecontroller_t* getBoneController( Int32 index ) const
	{
		assert(index >= 0 && index < numbonecontrollers);
		return reinterpret_cast<const mstudiobonecontroller_t*>(reinterpret_cast<const byte*>(this) + bonecontrollerindex) + index;
	}

	const mstudiobbox_t* getHitBox( Int32 index ) const
	{
		assert(index >= 0 && index < numhitboxes);
		return reinterpret_cast<const mstudiobbox_t*>(reinterpret_cast<const byte*>(this) + hitboxindex) + index;
	}

	const mstudioseqdesc_t* getSequence( Int32 index ) const
	{
		assert(index >= 0 && index < numseq);
		return reinterpret_cast<const mstudioseqdesc_t*>(reinterpret_cast<const byte*>(this) + seqindex) + index;
	}

	const mstudiotexture_t* getTexture( Int32 index ) const
	{
		assert(index >= 0 && index < numtextures);
		return reinterpret_cast<const mstudiotexture_t*>(reinterpret_cast<const byte*>(this) + textureindex) + index;
	}

	const Int16* getSkinFamily( Int32 index ) const
	{
		assert(index >= 0 && index < numskinfamilies);
		return reinterpret_cast<const Int16*>(reinterpret_cast<const byte*>(this) + skinindex) + index*numskinref;
	}

	const mstudiobodyparts_t* getBodyPart( Int32 index ) const
	{
		assert(index >= 0 && index < numbodyparts);
		return reinterpret_cast<const mstudiobodyparts_t*>(reinterpret_cast<const byte*>(this) + bodypartindex) + index;
	}

	const mstudioattachment_t* getAttachment( Int32 index ) const
	{
		assert(index >= 0 && index < numattachments);
		return reinterpret_cast<const mstudioattachment_t*>(reinterpret_cast<const byte*>(this) + attachmentindex) + index;
	}

	const mstudioseqgroup_t* getSequenceGroup( Int32 groupindex ) const
	{
		assert(groupindex >= 0 && groupindex < numseqgroups);
		return reinterpret_cast<const mstudioseqgroup_t*>(reinterpret_cast<const byte*>(this) + seqgroupindex) + groupindex;
	}

	Int32				id;
	Int32				version;

	Char				name[64];
	Int32				length;

	Vector				eyeposition;	// ideal eye position
	Vector				min;			// ideal movement hull size
	Vector				max;			

	Vector				bbmin;			// clipping bounding box
	Vector				bbmax;		

	Int32				flags;

	Int32				numbones;			// bones
	Int32				boneindex;

	Int32				numbonecontrollers;		// bone controllers
	Int32				bonecontrollerindex;

	Int32				numhitboxes;			// complex bounding boxes
	Int32				hitboxindex;			
	
	Int32				numseq;				// animation sequences
	Int32				seqindex;

	Int32				numseqgroups;		// demand loaded sequences
	Int32				seqgroupindex;

	Int32				numtextures;		// raw textures
	Int32				textureindex;
	Int32				texturedataindex;

	Int32				numskinref;			// replaceable textures
	Int32				numskinfamilies;
	Int32				skinindex;

	Int32				numbodyparts;		
	Int32				bodypartindex;

	Int32				numattachments;		// queryable attachable points
	Int32				attachmentindex;

	Int32				soundtable;
	Int32				soundindex;
	Int32				soundgroups;
	Int32				soundgroupindex;

	Int32				numtransitions;		// animation node to animation node transition graph
	Int32				transitionindex;
};

struct vbmcache_t
{
	vbmcache_t():
		pstudiohdr(nullptr),
		pvbmhdr(nullptr)
		{}
	~vbmcache_t()
	{
		if(pstudiohdr)
			delete pstudiohdr;

		if(pvbmhdr)
			delete pvbmhdr;
	}

	studiohdr_t *pstudiohdr;
	vbmheader_t *pvbmhdr;
};

#define STUDIO_DEMO_OFFSET		4096

// Used in $flags
#define STUDIO_MF_ROCKET		1 // leave a trail 
#define STUDIO_MF_GRENADE		2 // leave a trail 
#define STUDIO_MF_GIB			4 // leave a trail 
#define STUDIO_MF_ROTATE		8 // rotate (bonus items) 
#define STUDIO_MF_TRACER		16 // green split trail 
#define STUDIO_MF_ZOMGIB		32 // small blood trail 
#define STUDIO_MF_TRACER2		64 // orange split trail + rotate 
#define STUDIO_MF_TRACER3		128 // purple trail 
#define STUDIO_MF_DYNAMIC_LIGHT	256 // dynamically get lighting from floor or ceil (flying monsters) 
#define STUDIO_MF_TRACE_HITBOX	512 // always use hitbox trace instead of bbox
#define STUDIO_MF_SMALLFOV		1024 // Use a small FOV(view models)
#define STUDIO_MF_DEMOLOCK		2048 // Scramble model data
#define STUDIO_MF_SKYLIGHT		4096 // Take light from skybox
#define STUDIO_MF_CENTERLIGHT	8192 // Take light from absolute center
#define STUDIO_MF_HAS_FLEXES	16384 // This model supports flexes

// motion flags
#define STUDIO_X				0x0001
#define STUDIO_Y				0x0002	
#define STUDIO_Z				0x0004
#define STUDIO_XR				0x0008
#define STUDIO_YR				0x0010
#define STUDIO_ZR				0x0020
#define STUDIO_LX				0x0040
#define STUDIO_LY				0x0080
#define STUDIO_LZ				0x0100
#define STUDIO_AX				0x0200
#define STUDIO_AY				0x0400
#define STUDIO_AZ				0x0800
#define STUDIO_AXR				0x1000
#define STUDIO_AYR				0x2000
#define STUDIO_AZR				0x4000
#define STUDIO_TYPES			0x7FFF
#define STUDIO_RLOOP			0x8000	// controller that wraps shortest distance

// sequence flags
#define STUDIO_LOOPING			0x0001

// bone flags
#define STUDIO_HAS_NORMALS		0x0001
#define STUDIO_HAS_VERTICES		0x0002
#define STUDIO_HAS_BBOX			0x0004
#define STUDIO_HAS_CHROME		0x0008	// if any of the textures have chrome on them
#define STUDIO_DONT_BLEND		0x0010	// Don't perform animation blending
#define STUDIO_VBMCONTROLLER	0x0012	// This bone is affected by a VBM-defined controller

#define RAD_TO_STUDIO			(32768.0/M_PI)
#define STUDIO_TO_RAD			(M_PI/32768.0)

#define STUDIO_NF_CHROME		0x0002

#endif //STUDIO_H