/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef STUDIO_H
#define STUDIO_H

#include "datatypes.h"
#include "vbmformat.h"

// Notes:
// Part of this implementation is based on the implementation in the Half-Life SDK,
// meaning that the code was referenced(aka clean room design) when writing this file.
// The studiomodel format is Valve's original work, and I take no ownership of it.
// No copyright infringement intended.

/************************
	Constants

************************/

// This is used to identify a studiomdl file
static constexpr Int32 IDSTUDIOHEADER	= (('T'<<24)+('S'<<16)+('D'<<8)+'I');
// Version used by Half-Life 1, JACK, VHE for studiomodels
static constexpr Uint32 STUDIO_VERSION	= 10;

// Max vertexes supported by VHE
static constexpr Uint32 MAXSTUDIOVERTS_REF		= 2048;
// Max nb of bones due to byte being used to store bone indexes
static constexpr Uint32 MAXSTUDIOSRCBONES		= 256;
// Max bones in a model
static constexpr Uint32 MAXSTUDIOBONES			= 128;

// Used by studiohdr->flags
enum studio_flags_t
{
	STUDIO_MF_ROCKET			= (1<<0), // Leave a trail
	STUDIO_MF_GRENADE			= (1<<1), // Leave a trail
	STUDIO_MF_GIB				= (1<<2), // Leave a blood trail
	STUDIO_MF_ROTATE			= (1<<3), // Rotate, for bonus items
	STUDIO_MF_TRACER			= (1<<4), // Green spit trail
	STUDIO_MF_ZOMGIB			= (1<<5), // Small blood trail
	STUDIO_MF_TRACER2			= (1<<6), // Create orange spit trail, and rotate
	STUDIO_MF_TRACER3			= (1<<7), // Purple trail for vore projectile
	STUDIO_MF_DYNAMIC_LIGHT		= (1<<8), // Get lighting from floor or ceiling
	STUDIO_MF_TRACE_HITBOX		= (1<<9), // Use hitboxes for collision testing
	STUDIO_MF_UNUSED1			= (1<<10), // Unused
	STUDIO_MF_SKYLIGHT			= (1<<12), // Take lighting from skybox only
	STUDIO_MF_CENTERLIGHT		= (1<<13), // Use center of model for lighting
	STUDIO_MF_HAS_FLEXES		= (1<<14), // This model supports flexes
};

// Used by animations
enum studio_motionflags_t
{
	STUDIO_MT_NONE				= -1,
	STUDIO_X					= (1<<0),
	STUDIO_Y					= (1<<1),
	STUDIO_Z					= (1<<2),
	STUDIO_XR					= (1<<3),
	STUDIO_YR					= (1<<4),
	STUDIO_ZR					= (1<<5),
	STUDIO_LX					= (1<<6),
	STUDIO_LY					= (1<<7),
	STUDIO_LZ					= (1<<8),
	STUDIO_AX					= (1<<9),
	STUDIO_AY					= (1<<10),
	STUDIO_AZ					= (1<<11),
	STUDIO_AXR					= (1<<12),
	STUDIO_AYR					= (1<<13),
	STUDIO_AZR					= (1<<14),
	STUDIO_TYPES				= 0x7FFF,
	STUDIO_RLOOP				= (1<<15)
};

// Flags for animations
enum studio_seqflags_t
{
	STUDIO_LOOPING				= (1<<0)
};

// Flags for bones
enum studio_boneflags_t
{
	STUDIO_HAS_NORMALS			= (1<<0),
	STUDIO_HAS_VERTICES			= (1<<1),
	STUDIO_HAS_BBOX				= (1<<2),
	STUDIO_HAS_CHROME			= (1<<3),
	STUDIO_DONT_BLEND			= (1<<4),
	STUDIO_VBMCONTROLLER		= (1<<5)
};

// Flags for textures
enum studio_texflags_t
{
	STUDIO_NF_FLATSHADE			= (1<<0),
	STUDIO_NF_CHROME			= (1<<1),
	STUDIO_NF_ADDITIVE			= (1<<5),
	STUDIO_NF_ALPHATEST			= (1<<6)
};

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

/************************
	Definitions

************************/
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

	// Bone name
	Char name[32];
	// Parent bone index
	Int32 parent;
	// Flags for special animation exceptions
	Int32 flags;
	// Bone controller index, -1 means null
	Int32 bonecontroller[6];
	// Default DoF values
	Float value[6];
	// Scale for delta DoF values
	Float scale[6];
};

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

	// Bone affected, -1 is bone 0
	Int32 bone;
	// Controller type
	Int32 type;
	// Start value
	Float start;
	// End value
	Float end;
	// Byte index value at neutral
	Int32 rest;
	// Controller index(0-3 are user set controllers, 4 is mouth)
	Int32 index;
};

struct mstudiobbox_t
{
	mstudiobbox_t():
		bone(0),
		group(0)
		{}

	// Bone index bbox is tied to
	Int32 bone;
	// Hitgroup
	Int32 group;
	// BBox mins
	Vector bbmin;
	// BBox maxs
	Vector bbmax;		
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

	// Label of the sequence group
	Char label[32];
	// Name of the file(always none)
	Char name[64];
	// Cache index pointer
	cache_user_t cache;
	// Hack variable for group 0
	Int32 data;
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

	// Event's frame
	Int32 frame;
	// Event id
	Int32 event;
	// Event type
	Int32 type;
	// String for optional options
	Char options[64];
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

	// Name of the sequence
	Char label[32];

	// Framerate
	Float fps;
	// Flag bits for sequence
	Int32 flags;

	// Activity id
	Int32 activity;
	// Weight of activity
	Int32 actweight;

	// Number of events 
	Int32 numevents;
	// Offset to event data
	Int32 eventindex;

	// Number of frames in sequence
	Int32 numframes;

	// Number of pivots
	Int32 numpivots;
	// Offset to pivot data
	Int32 pivotindex;

	// Motion type
	Int32 motiontype;
	// Bone doing motion
	Int32 motionbone;
	// Linear movement
	Vector linearmovement;
	// Seems to be unused
	Int32 automoveposindex;
	// Seems to be unused
	Int32 automoveangleindex;

	// Bounding box of sequence
	Vector bbmin;
	Vector bbmax;		

	// Amount of blends in sequence
	Int32 numblends;
	// Animation data offset
	Int32 animindex;

	// Blend type
	Int32 blendtype[2];
	// Blend start
	Float blendstart[2];
	// Blend end
	Float blendend[2];
	// Blend parent
	Int32 blendparent;

	// Sequence group index
	Int32 seqgroup;

	// Entry node index
	Int32 entrynode;
	// Exit node index
	Int32 exitnode;
	// Node flags
	Int32 nodeflags;
	
	// Seems to be unused
	Int32 nextseq;
};

struct mstudiopivot_t
{
	mstudiopivot_t():
		start(0),
		end(0)
		{}

	// Pivot origin
	Vector org;
	// Pivot start
	Int32 start;
	// Pivot end
	Int32 end;
};

struct mstudioattachment_t
{
	mstudioattachment_t():
		type(0),
		bone(0)
	{
		memset(name, 0, sizeof(name));
	}

	// Name of attachment
	Char name[32];
	// Attachment type
	Int32 type;
	// Bone attachment is tied to
	Int32 bone;
	// Origin in bone space
	Vector org;
	// Seems to be unused
	Vector vectors[3];
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

	// Offset values
	Uint16 offset[6];
};

// This is ugly and should be refactored
union mstudioanimvalue_t
{
	struct 
	{
		byte	valid;
		byte	total;
	} num;

	Int16 value;
};

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

	// Name of texture
	Char name[64];
	// Texture flags
	Int32 flags;
	// Width of texture
	Int32 width;
	// Height of texture
	Int32 height;
	// Index into model data
	Int32 index;
};

// Wouldn't "mstudiosubmodel_t" a better name?
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

	// Name of the submodel
	Char name[64];

	// Unused?
	Int32 type;

	// Unused?
	Float boundingradius;

	// Number of meshes
	Int32 nummesh;
	// Mesh data offset
	Int32 meshindex;

	// Number of vertexes
	Int32 numverts;
	// Vertex bone indexes offset
	Int32 vertinfoindex;
	// Vertex coordinates offset
	Int32 vertindex;
	// Number of normals
	Int32 numnorms;
	// Normal bone info indexes offset
	Int32 norminfoindex;
	// Normal vectors offset
	Int32 normindex;

	// Unused?
	Int32 numgroups;
	// Unused?
	Int32 groupindex;
};

// Same but "mstudiobodygroup_t"
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

	// Name of bodygroup
	Char name[64];
	// Number of submodels in bodygroup
	Int32 nummodels;
	// Base index
	Int32 base;
	// Submodel data offset
	Int32 modelindex;
};

struct mstudiomesh_t
{
	mstudiomesh_t():
		numtris(0),
		triindex(0),
		skinref(0),
		numnorms(0),
		normindex(0)
		{}

	// Number of triangles in mesh
	Int32 numtris;
	// Offset for triangle data
	Int32 triindex;
	// Skin reference index
	Int32 skinref;
	// Number of normals in mesh
	Int32 numnorms;
	// Normal index offset
	Int32 normindex;
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

	// Holds the ID of the file type
	Int32 id;
	// Version of the model file
	Int32 version;

	// Model file name
	Char name[64];
	// Total size of model
	Int32 length;

	// Eye offset from base
	Vector eyeposition;
	// Bounding box mins
	Vector min;
	// Bounding box maxs
	Vector max;

	// Used by clipping hull?
	Vector bbmin;
	// Used by clipping hull?
	Vector bbmax;		

	// Model flags
	Int32 flags;

	// Number of bones in model
	Int32 numbones;
	// Offset to bone data
	Int32 boneindex;

	// Number of bone controllers
	Int32 numbonecontrollers;	
	// Offset to bone controller data
	Int32 bonecontrollerindex;

	// Number of hitboxes in model
	Int32 numhitboxes;
	// Offset to hitbox data
	Int32 hitboxindex;			
	
	// Number of sequences in model
	Int32 numseq;
	// Offset to sequence data
	Int32 seqindex;

	// Number of sequence groups(deprecated in Pathos)
	Int32 numseqgroups;
	// Offset to sequence group data(deprecated in Pathos)
	Int32 seqgroupindex;

	// Number of textures in model
	Int32 numtextures;
	// Offset to texture data
	Int32 textureindex;
	// Offset at which texture bitmap data starts
	Int32 texturedataindex;

	// Number of skins in replacement matrix
	Int32 numskinref;
	// Number of skin replacements
	Int32 numskinfamilies;
	// Offset to skin matrix data
	Int32 skinindex;

	// Number of bodyparts
	Int32 numbodyparts;		
	// Offset to bodypart data
	Int32 bodypartindex;

	// Number of attachments in model
	Int32 numattachments;
	// Offset to attachment data
	Int32 attachmentindex;

	// Unused
	Int32 soundtable;
	// Unused
	Int32 soundindex;
	// Unused
	Int32 soundgroups;
	// Unused
	Int32 soundgroupindex;

	// Transition info number
	Int32 numtransitions;
	// Transition index
	Int32 transitionindex;
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

	// Pointer to studiomdl data
	studiohdr_t *pstudiohdr;
	// Pointer to vbm data
	vbmheader_t *pvbmhdr;
};

typedef CArray<pmatrix3x4_t> BoneTransformArray_t;
#endif //STUDIO_H