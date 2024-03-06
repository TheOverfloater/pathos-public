/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CL_PMOVE_H
#define CL_PMOVE_H

#include "constants.h"

struct trace_t;
struct hull_t;

extern Int32 CL_TestPlayerPosition( hull_types_t hulltype, Int32 flags, const class Vector& position );
extern Int32 CL_PointContents( const Vector& position, Int32* truecontents );
extern Int32 CL_PointContents( cl_entity_t* pentity, const Vector& position );
extern Int32 CL_TruePointContents( const Vector& position );
extern void CL_PlayerTrace( const Vector& start, const Vector& end, Int32 traceflags, hull_types_t hulltype, Int32 ignore_ent, trace_t& trace );
extern void CL_TraceLine( const Vector& start, const Vector& end, Int32 traceflags, hull_types_t hulltype, Int32 ignore_ent, trace_t& trace );
extern const hull_t* CL_HullForBSP( Int32 entity, hull_types_t hulltype, Vector* poffset );
extern Float CL_TraceModel( Int32 entity, const Vector& start, const Vector& end, hull_types_t hulltype, Int32 flags, trace_t& trace );
extern const Char* CL_TraceTexture( Int32 groundentity, const Vector& start, const Vector& end );
#endif //CL_PMOVE_H