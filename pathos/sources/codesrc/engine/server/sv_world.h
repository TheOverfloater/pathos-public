/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SV_WORLD_H
#define SV_WORLD_H

#include "trace.h"
#include "constants.h"

struct moveclip_t
{
	moveclip_t():
		pmins1(nullptr),
		pmaxs1(nullptr),
		pstart(nullptr),
		pend(nullptr),
		flags(FL_TRACE_NORMAL),
		pignore_edict(nullptr)
		{};

	Vector boxmins;
	Vector boxmaxs;

	const Vector* pmins1;
	const Vector* pmaxs1;

	Vector mins2;
	Vector maxs2;

	const Vector* pstart;
	const Vector* pend;

	trace_t trace;

	Int32 flags;
	edict_t* pignore_edict;
};

extern void SV_LinkEdict( edict_t* pentity, bool touchtriggers );
extern void SV_TouchLinks( edict_t* pentity, areanode_t* pnode );
extern void SV_UnlinkEdict( edict_t* pentity );
extern areanode_t* SV_CreateAreaNode( Int32 depth, const Vector& mins, const Vector& maxs );

extern Int32 SV_PointContents( const Vector& position, Int32* truecontents = nullptr );
extern Int32 SV_TruePointContents( const Vector& position );

extern edict_t* SV_TestEntityPosition( edict_t* pentity, hull_types_t hulltype );
extern Int32 SV_TestPlayerPosition( hull_types_t hulltype, Int32 flags, const Vector& position );

extern void SV_Move( trace_t& trace, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, Int32 flags, edict_t* pignore_edict, hull_types_t hulltype = HULL_AUTO );
extern void SV_MoveNoEntities( trace_t& trace, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, Int32 flags, edict_t* pignore_edict, hull_types_t hulltype = HULL_AUTO );
extern void SV_InitBoxHull( void );

extern const hull_t* SV_HullForBSP( const edict_t* pentity, const Vector& mins, const Vector& maxs, Vector* poffset, hull_types_t hulltype = HULL_AUTO );
extern const hull_t* SV_HullForBSP( Int32 entity, hull_types_t hulltype, Vector* poffset );
extern const hull_t* SV_HullForEntity( const edict_t* pentity, const Vector& mins, const Vector& maxs, Vector* poffset, hull_types_t hulltype = HULL_AUTO );
extern void SV_SingleClipMoveToEntity( edict_t* pentity, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, trace_t& trace, Int32 flags, hull_types_t hulltype = HULL_AUTO );

extern Float SV_TraceModel( Int32 entindex, const Vector& start, const Vector& end, hull_types_t hulltype, Int32 flags, trace_t& trace );
extern void SV_PlayerTrace( const Vector& start, const Vector& end, Int32 traceflags,hull_types_t hulltype, Int32 ignore_ent, trace_t& trace );
extern void SV_TraceLine( const Vector& start, const Vector& end, Int32 traceflags, hull_types_t hulltype, Int32 ignore_ent, trace_t& trace );
#endif