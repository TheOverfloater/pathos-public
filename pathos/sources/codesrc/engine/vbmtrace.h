/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef STUDIOTRACE_H
#define STUDIOTRACE_H

#include "entity_state.h"
#include "constants.h"
#include "studio.h"

struct mclipnode_t;
struct cache_model_t;
struct plane_t;
struct hull_t;
struct trace_t;
struct studiohdr_t;
struct mstudioseqdesc_t;

// Number of planes for a hull
static const Uint32 NUM_STUDIO_HULL_PLANES = 6;
// Number of planes for a hull
static const Uint32 NUM_STUDIO_HULL_CLIPNODES = 6;

struct vbmhitboxhull_t
{
	vbmhitboxhull_t():
		hitgroup(0),
		hitboxindex(0),
		boneindex(0)
		{
		}

	// Hull information
	hull_t hull;

	// Array of planes
	plane_t planes[NUM_STUDIO_HULL_PLANES];

	// Hitgroup id
	Uint32 hitgroup;
	// Index into studiomodel hitboxes
	Uint32 hitboxindex;
	// Index into studiomodel bones
	Uint32 boneindex;

	// bbox mins
	Vector mins;
	// bbox maxs
	Vector maxs;
};

struct vbmhull_t
{
	vbmhull_t():
		hullset(false)
		{}

	// Array of hulls for hitboxes
	CArray<vbmhitboxhull_t> hullsarray;
	// TRUE if this hull was set
	bool hullset;
	// Size used to construct the hull
	Vector size;
};

struct entity_vbmhulldata_t
{
	entity_vbmhulldata_t():
		entindex(0),
		frame(0),
		sequence(0),
		pcachemodel(nullptr)
		{
			memset(controller, 0, sizeof(controller));
			memset(blending, 0, sizeof(blending));
			memset(bonetransform, 0, sizeof(bonetransform));
		}
		
	// Entity index
	entindex_t entindex;

	// Animation information
	Float frame;
	Int32 sequence;

	// Orientation info
	Vector angles;
	Vector origin;

	// Last controller states
	Float controller[MAX_CONTROLLERS];
	Float blending[MAX_BLENDING];

	// Pointer to cached model
	const cache_model_t* pcachemodel;

	// Array of clipnodes
	mclipnode_t clipnodes[NUM_STUDIO_HULL_CLIPNODES];
	// Array of hulls
	vbmhull_t hulls[MAX_MAP_HULLS];

	// Last bone transform used
	Float bonetransform[MAXSTUDIOBONES][3][4];
};

extern void TR_VBMSetHullInfo( entity_vbmhulldata_t*& pdataptr, const cache_model_t* pmodel, const Vector& hullmins, const Vector& hullmaxs, const entity_state_t& state, Float time, hull_types_t hulltype );
extern void TR_VBMSetupBones( entity_vbmhulldata_t* phulldata, const studiohdr_t* pstudiohdr, Float time, Float frame, const mstudioseqdesc_t* pseqdesc, const cache_model_t* pmodel, const entity_state_t& state );
extern bool TR_VBMCheckHullInfo( entity_vbmhulldata_t* pdata, const cache_model_t* pmodel, Float frame, const entity_state_t& state );
extern const CArray<vbmhitboxhull_t>* TR_VBMGetHulls( entity_vbmhulldata_t* pvbmhulldata, const Vector& hullmins, const Vector& hullmaxs, hull_types_t hulltype, Int32 flags, Vector* poffset );
extern void TR_VBMHullCheck( const CArray<vbmhitboxhull_t>* phulls, const Vector& start, const Vector& end, trace_t& tr );
extern void TR_VBMSetStateInfo( entity_vbmhulldata_t* pvbmhulldata, const cache_model_t* pmodel, Float frame, const entity_state_t& state );
#endif //STUDIOTRACE_H