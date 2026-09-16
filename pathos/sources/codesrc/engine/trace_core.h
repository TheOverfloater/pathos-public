/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRACE_SHARED_H
#define TRACE_SHARED_H

struct mcdheader_t;

enum collision_method_t
{
	CM_NO_VALID_METHOD = 0, // Not a valid entity
	CM_BRUSH_COLLISIONS,	// Collide against Quake 2-style brush data
	CM_CLIPNODE_COLLISIONS,	// Collide against clipnodes
	CM_VBM_HITBOX_HULLS,	// Collide against studio hitboxes
	CM_MCD_COLLISIONS,		// Collide against MCD data
	CM_BBOX_COLLISIONS,		// Collide against bbox
};

extern Vector HULL_MINS[];
extern Vector HULL_MAXS[];

extern void TR_InitBoxHull( void );
extern hull_types_t TR_GetHullType( const Vector& mins, const Vector& maxs, hull_types_t hulltype );
extern const hull_t* TR_HullForBox( const Vector& mins, const Vector& maxs );
extern const hull_t* TR_HullForBSP( const entity_state_t& entity, hull_types_t hulltype, Vector& offset, const Vector& player_mins );

extern void TR_MoveBounds( const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, Vector& boxmins, Vector& boxmaxs );
extern void TR_MoveBoundsPoint( const Vector& start, const Vector&end, Vector& boxmins, Vector& boxmaxs );

extern void TR_PlayerTraceSingleEntity( const entity_state_t& entity, entity_vbmhulldata_t* pvbmhulldata, const Vector& start, const Vector& end, hull_types_t hulltype, Int32 traceflags, const Vector& player_mins, const Vector& player_maxs, trace_t& outtrace );
extern bool TR_RecursiveHullCheck_ClipNode( const hull_t* phull, Int32 clipnodeidx, Double p1f, Double p2f, const Vector& p1, const Vector& p2, trace_t& trace );
extern Int32 TR_HullPointContents_ClipNode( const hull_t* phull, Int32 clipnodeidx, const Vector& position );
extern const Char* TR_TraceTexture( const entity_state_t& entity, const Vector& start, const Vector& end );
extern bool TR_TracelineBBoxCheck( const entity_state_t& entity, const cache_model_t* pcachemodel, const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs );

extern Int32 TR_PointContents_Brush( const brushmodel_t* pbrushmodel, const Vector& position );
extern Int32 TR_HullPointContents_Brush( const brushmodel_t* pbrushmodel, const Vector& position, const Vector& mins, const Vector& maxs, Int32 brushtypebits );
extern void TR_BrushBoxTrace( const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, const mnode_t* pheadnode, const brushmodel_t* pbrushmodel, Int32 brushtypebits, trace_t& outtrace );
extern void TR_TraceAgainstEntity( const entity_state_t& entity, const cache_model_t* pmodel, entity_vbmhulldata_t* pvbmhulldata, const Vector& start, const Vector& end, hull_types_t hulltype, Int32 flags, const Vector& mins, const Vector& maxs, trace_t& trace );

extern collision_method_t TR_GetIdealCollisionMethod( const entity_state_t& entity, const cache_model_t* pcachemodel, Int32 flags, hull_types_t hulltype );
#endif //TRACE_SHARED_H