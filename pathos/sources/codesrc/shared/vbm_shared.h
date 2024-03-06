/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef VBM_SHARED_H
#define VBM_SHARED_H

// Materials scripts base path
static const Char MODEL_MATERIALS_BASE_PATH[] = "models/";

// Notes:
// Part of this implementation is referenced from the implementation in the Half-Life SDK
// The studiomodel format is Valve's original work, and I take no ownership of it
// No copyright infringement intended

enum vbm_renderflags_t
{
	VBM_RENDER = (1<<0), // Render the model
	VBM_ANIMEVENTS = (1<<1), // Trigger animevents
	VBM_SETUPBONES = (1<<2) // Only set up bones
};

struct studiohdr_t;
struct model_t;
struct mstudioseqdesc_t;
struct mstudiobone_t;
struct mstudioanim_t;
struct vbmheader_t;
struct vbmsubmodel_t;
struct entity_state_t;
struct en_material_t;
struct cache_model_t;

enum vbmlod_type_t;

extern Float VBM_EstimateInterpolant( Float time, Float animtime, Float prevanimtime );
extern void VBM_CalculateRotations( const studiohdr_t* phdr, Float time, Float animtime, Float prevanimtime, Vector* ppositions, vec4_t* pquaternions, const mstudioseqdesc_t* pseqdesc, const mstudioanim_t* panim, Float frame, const Float* pcontroller1, const Float* pcontroller2, byte mouth );
extern void VBM_CalculateBoneAdjustments( const studiohdr_t* phdr, Float dadt, Float* padj, const Float* pcontroller1, const Float* pcontroller2, byte mouth );
extern void VBM_InterpolateBones( const studiohdr_t* phdr, const vec4_t* pquaternions1, const Vector* ppositions1, const vec4_t* pquaternions2, const Vector* ppositions2, Float interpolant, vec4_t *poutquaternions, Vector* poutpositions );
extern const mstudioanim_t* VBM_GetAnimation( const studiohdr_t* phdr, const mstudioseqdesc_t* psequencedesc );
extern void VBM_CalculateBoneQuaternion( Int32 frame, Float interpolant, const mstudiobone_t* pbone, const mstudioanim_t* panimation, const Float* padj, vec4_t& quaternion );
extern void VBM_CalculateBonePosition( Int32 frame, Float interpolant, const mstudiobone_t* pbone, const mstudioanim_t* panimation, const Float* padj, Vector& outpos );
extern Float VBM_EstimateFrame( const mstudioseqdesc_t* pseqdesc, const entity_state_t& entitystate, Double time );
extern bool VBM_HasTransparentParts( vbmheader_t* pvbmheader, Uint64 body, Int32 skin );
extern void VBM_NormalizeWeights( Float* pflweights, Uint32 maxweights );
extern Int32 VBM_FindSequence( const studiohdr_t* pstudiohdr, const Char* pstrsequencename );
extern Float VBM_GetSequenceTime( const studiohdr_t* pstudiohdr, Int32 sequence, Float framerate );
extern bool VBM_PostLoadVBMCheck( vbmheader_t* pvbm, en_material_t* (*pfnFindMaterialScriptByIndex)( Int32 index ) );
extern Float VBM_SetController( const cache_model_t* pmodel, Uint32 controllerindex, Float value, Float* pcontrollers, void (*pfnCon_Printf)( const Char *fmt, ... ) );
#endif //VBM_SHARED_H