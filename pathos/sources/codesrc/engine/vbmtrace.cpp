/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "brushmodel.h"
#include "studio.h"
#include "vbmtrace.h"
#include "com_math.h"
#include "cache_model.h"
#include "vbm_shared.h"
#include "system.h"
#include "trace.h"
#include "trace_shared.h"

// Quaternion and vector arrays used for bone transforms
Vector	g_bonePositions1[MAXSTUDIOBONES];
vec4_t	g_boneQuaternions1[MAXSTUDIOBONES];
// Quaternion and vector arrays used for bone transforms
Vector	g_bonePositions2[MAXSTUDIOBONES];
vec4_t	g_boneQuaternions2[MAXSTUDIOBONES];
// Quaternion and vector arrays used for bone transforms
Vector	g_bonePositions3[MAXSTUDIOBONES];
vec4_t	g_boneQuaternions3[MAXSTUDIOBONES];
// Quaternion and vector arrays used for bone transforms
Vector	g_bonePositions4[MAXSTUDIOBONES];
vec4_t	g_boneQuaternions4[MAXSTUDIOBONES];
// Used for bone transform calculations
Float	g_boneMatrix[3][4];

// Internal rotation matrix
Float g_rotationMatrix[3][4];

//=============================================
//
//=============================================
void TR_VBMInitHulls( const studiohdr_t* pstudiohdr, hull_types_t hulltype, entity_vbmhulldata_t& cache )
{
	// Initialize clipnodes
	for(Uint32 i = 0; i < NUM_STUDIO_HULL_CLIPNODES; i++)
	{
		Int32 side = i&1;

		mclipnode_t& clipnode = cache.clipnodes[i];

		clipnode.planenum = i;
		clipnode.children[side] = CONTENTS_EMPTY;
		clipnode.children[side^1] = (i<5) ? (i + 1) : CONTENTS_SOLID;
	}

	// Make sure it's cleared
	if(!cache.hulls[hulltype].hullsarray.empty())
		cache.hulls[hulltype].hullsarray.clear();

	// Reset this flag
	cache.hulls[hulltype].hullset = true;

	// Allocate hulls
	cache.hulls[hulltype].hullsarray.resize(pstudiohdr->numhitboxes);

	// Create the hulls
	
	for(Int32 i = 0; i < pstudiohdr->numhitboxes; i++)
	{
		const mstudiobbox_t* phitbox = pstudiohdr->getHitBox(i);
		vbmhitboxhull_t& vhull = cache.hulls[hulltype].hullsarray[i];

		// Initialize hull
		hull_t& hull = vhull.hull;
		hull.pplanes = vhull.planes;
		hull.pclipnodes = cache.clipnodes;

		hull.firstclipnode = 0;
		hull.lastclipnode = 5;

		vhull.boneindex = phitbox->bone;
		vhull.hitgroup = phitbox->group;
		vhull.mins = phitbox->bbmin;
		vhull.maxs = phitbox->bbmax;
		vhull.hitboxindex = i;
	}
}

//=============================================
//
//=============================================
bool TR_VBMCheckHullInfo( entity_vbmhulldata_t* pdata, const cache_model_t* pmodel, Float frame, const entity_state_t& state )
{
	if(pmodel != pdata->pcachemodel)
		return false;

	if(frame != pdata->frame)
		return false;

	if(state.sequence != pdata->sequence)
		return false;

	for(Uint32 i = 0; i < MAX_CONTROLLERS; i++)
	{
		if(pdata->controller[i] != state.controllers[i])
			return false;
	}

	for(Uint32 i = 0; i < MAX_BLENDING; i++)
	{
		if(pdata->blending[i] != state.blending[i])
			return false;
	}

	if(!Math::VectorCompare(pdata->origin, state.origin) || !Math::VectorCompare(pdata->angles, state.angles))
		return false;

	return true;
}

//=============================================
//
//=============================================
void TR_VBMSetupBones( entity_vbmhulldata_t* phulldata, const studiohdr_t* pstudiohdr, Float time, Float frame, const mstudioseqdesc_t* pseqdesc, const cache_model_t* pmodel, const entity_state_t& state )
{
	Vector angles = state.angles;
	angles[PITCH] = -angles[PITCH];

	Math::AngleMatrix(angles, g_rotationMatrix);

	for(Uint32 i = 0; i < 3; i++)
		g_rotationMatrix[i][3] = state.origin[i];

	// Apply scale to models that require it
	if((state.renderfx == RenderFx_ScaledModel || state.renderfx == RenderFx_InPortalScaledModel) && state.scale != 0)
	{
		for(Uint32 i = 0; i < 3; i++)
		{
			for(Uint32 j = 0; j < 3; j++)
				g_rotationMatrix[i][j] *= state.scale;
		}
	}

	// Get animation and calc rotations
	const mstudioanim_t* panim = VBM_GetAnimation(pstudiohdr, pseqdesc);
	if(!panim)
	{
		Con_EPrintf("%s - Pathos does not support models with sequence groups. Model '%s' not managed.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	// Calculate rotations
	VBM_CalculateRotations(pstudiohdr, time, state.animtime, 0, g_bonePositions1, g_boneQuaternions1, pseqdesc, panim, frame, state.controllers, state.controllers, 0);

	// Manage blending
	if(pseqdesc->numblends > 1)
	{
		panim += pstudiohdr->numbones;
		VBM_CalculateRotations(pstudiohdr, time, state.animtime, 0, g_bonePositions2, g_boneQuaternions2, pseqdesc, panim, frame, state.controllers, state.controllers, 0);
		Float interp = state.blending[0]/255.0f;

		VBM_InterpolateBones(pstudiohdr, g_boneQuaternions1, g_bonePositions1, g_boneQuaternions2, g_bonePositions2, interp, g_boneQuaternions1, g_bonePositions1);

		if(pseqdesc->numblends == 4)
		{
			panim += pstudiohdr->numbones;
			VBM_CalculateRotations(pstudiohdr, time, state.animtime,0, g_bonePositions3, g_boneQuaternions3, pseqdesc, panim, frame, state.controllers, state.controllers, 0);

			panim += pstudiohdr->numbones;
			VBM_CalculateRotations(pstudiohdr, time, state.animtime, 0, g_bonePositions4, g_boneQuaternions4, pseqdesc, panim, frame, state.controllers, state.controllers, 0);

			interp = state.blending[0]/255.0f;
			VBM_InterpolateBones(pstudiohdr, g_boneQuaternions3, g_bonePositions3, g_boneQuaternions4, g_bonePositions4, interp, g_boneQuaternions3, g_bonePositions3);

			interp = state.blending[1]/255.0f;
			VBM_InterpolateBones(pstudiohdr, g_boneQuaternions1, g_bonePositions1, g_boneQuaternions3, g_bonePositions3, interp, g_boneQuaternions1, g_bonePositions1);
		}
	}

	// Calculate bone matrices
	for(Int32 i = 0; i < pstudiohdr->numbones; i++)
	{
		const mstudiobone_t* pbone = pstudiohdr->getBone(i);
		Math::QuaternionMatrix(g_boneQuaternions1[i], g_boneMatrix);

		for(Uint32 j = 0; j < 3; j++)
			g_boneMatrix[j][3] = g_bonePositions1[i][j];

		if(pbone->parent == -1)
			Math::ConcatTransforms(g_rotationMatrix, g_boneMatrix, phulldata->bonetransform[i]);
		else
			Math::ConcatTransforms(phulldata->bonetransform[pbone->parent], g_boneMatrix, phulldata->bonetransform[i]);
	}
}

//=============================================
//
//=============================================
void TR_VBMSetHullPlane( entity_vbmhulldata_t* phulldata, plane_t& plane, Int32 boneindex, Uint32 i, Float dist )
{
	// Set type
	plane.type = PLANE_AZ;

	// Set the normal
	plane.normal[0] = phulldata->bonetransform[boneindex][0][i];
	plane.normal[1] = phulldata->bonetransform[boneindex][1][i];
	plane.normal[2] = phulldata->bonetransform[boneindex][2][i];

	// Set distance
	plane.dist = plane.normal[0] * phulldata->bonetransform[boneindex][0][3]
		+ plane.normal[1] * phulldata->bonetransform[boneindex][1][3]
		+ plane.normal[2] * phulldata->bonetransform[boneindex][2][3]
		+ dist;
}

//=============================================
//
//=============================================
void TR_VBMSetStateInfo( entity_vbmhulldata_t* pvbmhulldata, const cache_model_t* pmodel, Float frame, const entity_state_t& state )
{
	pvbmhulldata->angles = state.angles;
	pvbmhulldata->origin = state.origin;
	pvbmhulldata->sequence = state.sequence;
	pvbmhulldata->frame = frame;
	pvbmhulldata->pcachemodel = pmodel;

	for(Uint32 i = 0; i < MAX_BLENDING; i++)
		pvbmhulldata->blending[i] = state.blending[i];

	for(Uint32 i = 0; i < MAX_CONTROLLERS; i++)
		pvbmhulldata->controller[i] = state.controllers[i];
}

//=============================================
//
//=============================================
void TR_VBMSetHullInfo( entity_vbmhulldata_t*& pdataptr, const cache_model_t* pmodel, const Vector& hullmins, const Vector& hullmaxs, const entity_state_t& state, Float time, hull_types_t hulltype )
{
	// Get cache
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;

	Int32 sequence = state.sequence;
	if(sequence < 0 || sequence >= pstudiohdr->numseq)
		sequence = 0;

	// Get sequence data
	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequence);
	Float frame = VBM_EstimateFrame(pseqdesc, state, time);

	// Check if the last data matches our current state
	bool statematches = false;
	if(pdataptr)
		statematches = TR_VBMCheckHullInfo(pdataptr, pmodel, frame, state);
	
	// Determine size
	Vector size;
	Math::VectorSubtract(hullmaxs, hullmins, size);
	Math::VectorScale(size, 0.5, size);

	Int32 hullindex;
	if(hulltype == HULL_AUTO)
	{
		if(size[0] < 8.0f)
		{
			hullindex = HULL_POINT;
		}
		else
		{
			if(size[0] <= 36.0f)
			{
				if(size[2] <= 36.0f)
					hullindex = HULL_SMALL;
				else
					hullindex = HULL_HUMAN;
			}
			else
			{
				hullindex = HULL_LARGE;
			}
		}
	}
	else
	{
		// Just use the specified hull
		hullindex = hulltype;
	}

	if(statematches && pdataptr->hulls[hullindex].hullset 
		&& Math::VectorCompare(size, pdataptr->hulls[hullindex].size))
		return;

	// Allocate new data if needed
	if(!pdataptr)
		pdataptr = new entity_vbmhulldata_t;

	// Don't bother if it has no hitboxes
	if(pstudiohdr->numhitboxes <= 0)
	{
		// Store state information
		TR_VBMSetStateInfo(pdataptr, pmodel, frame, state);

		// Mark hull as set
		pdataptr->hulls[hullindex].hullset = true;
		return;
	}

	// If the state was changed, mark all hulls as not set
	if(!statematches || !pdataptr->hulls[hullindex].hullset)
	{
		// Reset these if state doesn't match
		if(!statematches)
		{
			for(Uint32 i = 0; i < MAX_MAP_HULLS; i++)
				pdataptr->hulls[i].hullset = false;
		}

		// Initialize hulls of needed
		if(pmodel != pdataptr->pcachemodel)
		{
			// If model was changed, flush current hull setup
			for(Uint32 i = 0; i < MAX_MAP_HULLS; i++)
			{
				if(pdataptr->hulls[i].hullsarray.empty())
					continue;

				pdataptr->hulls[i].hullsarray.clear();
			}
		}

		if(!pdataptr->hulls[hullindex].hullset)
		{
			// Initialize only the relevant hull
			TR_VBMInitHulls(pstudiohdr, (hull_types_t)hullindex, *pdataptr);
		}

		if(!statematches)
		{
			// Transform bones into cache
			TR_VBMSetupBones(pdataptr, pstudiohdr, time, frame, pseqdesc, pmodel, state);
		}
	}

	// Set hull planes
	for(Int32 i = 0; i < pstudiohdr->numhitboxes; i++)
	{
		const mstudiobbox_t* pbbox = pstudiohdr->getHitBox(i);
		vbmhitboxhull_t& hull = pdataptr->hulls[hullindex].hullsarray[i];

		for(Uint32 j = 0; j < 3; j++)
		{
			// Set the first hull
			plane_t& plane1 = hull.planes[j*2];
			TR_VBMSetHullPlane(pdataptr, plane1, pbbox->bone, j, pbbox->bbmax[j]);
			plane1.dist += SDL_fabs(plane1.normal[0]*size[0]) + SDL_fabs(plane1.normal[1]*size[1]) + SDL_fabs(plane1.normal[2]*size[2]);

			// Set the second hull
			plane_t& plane2 = hull.planes[j*2+1];
			TR_VBMSetHullPlane(pdataptr, plane2, pbbox->bone, j, pbbox->bbmin[j]);
			plane2.dist -= SDL_fabs(plane2.normal[0]*size[0]) + SDL_fabs(plane2.normal[1]*size[1]) + SDL_fabs(plane2.normal[2]*size[2]);
		}
	}

	// Mark this hull as having been set
	pdataptr->hulls[hullindex].hullset = true;
	pdataptr->hulls[hullindex].size = size;

	// Set data
	if(!statematches)
		TR_VBMSetStateInfo(pdataptr, pmodel, frame, state);
}

//=============================================
//
//=============================================
const CArray<vbmhitboxhull_t>* TR_VBMGetHulls( entity_vbmhulldata_t* pvbmhulldata, const Vector& hullmins, const Vector& hullmaxs, hull_types_t hulltype, Int32 flags, Vector* poffset )
{
	// Clear offset
	if(poffset)
		poffset->Clear();

	// Determine size
	Vector size;
	Math::VectorSubtract(hullmaxs, hullmins, size);
	Math::VectorScale(size, 0.5, size);

	Int32 hullindex;
	if(hulltype == HULL_AUTO)
	{
		if(size[0] < 8.0f)
		{
			hullindex = HULL_POINT;
		}
		else
		{
			if(size[0] <= 36.0f)
			{
				if(size[2] <= 36.0f)
					hullindex = HULL_SMALL;
				else
					hullindex = HULL_HUMAN;
			}
			else
			{
				hullindex = HULL_LARGE;
			}
		}
	}
	else
	{
		// Just use the specified hull
		hullindex = hulltype;
	}

	return &pvbmhulldata->hulls[hullindex].hullsarray;
}

//=============================================
//
//=============================================
void TR_VBMHullCheck( const CArray<vbmhitboxhull_t>* phulls, const Vector& start, const Vector& end, trace_t& tr )
{
	for(Uint32 i = 0; i < phulls->size(); i++)
	{
		trace_t hbtr;
		hbtr.endpos = end;
		hbtr.fraction = 1.0f;
		hbtr.flags = FL_TR_ALLSOLID;

		const vbmhitboxhull_t& hbhull = (*phulls)[i];
		const hull_t* phull = &hbhull.hull;

		// Perform trace
		TR_RecursiveHullCheck(phull, phull->firstclipnode, 0.0f, 1.0f, start, end, hbtr);

		// Assign as best trace if it's closer, or starts in a solid
		if(i == 0 || tr.flags & (FL_TR_ALLSOLID|FL_TR_STARTSOLID) || hbtr.fraction < tr.fraction)
		{
			tr.endpos = hbtr.endpos;
			tr.flags = hbtr.flags;
			tr.plane = hbtr.plane;
			tr.fraction = hbtr.fraction;
			tr.hitgroup = hbhull.hitgroup;
			tr.hitbox = hbhull.hitboxindex;
		}
	}
}