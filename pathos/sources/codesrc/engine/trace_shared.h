/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRACE_SHARED_H
#define TRACE_SHARED_H

extern void TR_InitBoxHull( void );
extern const hull_t* TR_HullForBox( const Vector& mins, const Vector& maxs );
extern const hull_t* TR_HullForBSP( const entity_state_t& entity, hull_types_t hulltype, Vector& offset, const Vector& player_mins );

extern void TR_MoveBounds( const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, Vector& boxmins, Vector& boxmaxs );
extern void TR_MoveBoundsPoint( const Vector& start, const Vector&end, Vector& boxmins, Vector& boxmaxs );

extern void TR_PlayerTraceSingleEntity( const entity_state_t& entity, entity_vbmhulldata_t* pvbmhulldata, const Vector& start, const Vector& end, hull_types_t hulltype, Int32 traceflags, const Vector& player_mins, const Vector& player_maxs, trace_t& outtrace );
extern bool TR_RecursiveHullCheck( const hull_t* phull, Int32 clipnodeidx, Double p1f, Double p2f, const Vector& p1, const Vector& p2, trace_t& trace );
extern Int32 TR_HullPointContents( const hull_t* phull, Int32 clipnodeidx, const Vector& position );
extern const Char* TR_TraceTexture( const entity_state_t& entity, const Vector& start, const Vector& end );

#endif //TRACE_SHARED_H