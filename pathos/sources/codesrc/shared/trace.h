/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRACE_H
#define TRACE_H

#include "plane.h"
#include "constants.h"
#include "entity_state.h"

struct hull_t;

enum trace_group_operands_t
{
	TR_GROUPOP_NONE = 0,
	TR_GROUPOP_AND,
	TR_GROUPOP_NAND,
};

enum trace_result_flags_t
{
	FL_TR_NONE			= 0,
	FL_TR_ALLSOLID		= (1<<0),
	FL_TR_STARTSOLID	= (1<<1),
	FL_TR_INOPEN		= (1<<2),
	FL_TR_INWATER		= (1<<3)
};

enum trace_flags_t
{
	FL_TRACE_NORMAL						= 0,
	FL_TRACE_WORLD_ONLY					= (1<<0),
	FL_TRACE_NO_MODELS					= (1<<1),
	FL_TRACE_NO_TRANS					= (1<<2),
	FL_TRACE_NPC_CLIP					= (1<<3),
	FL_TRACE_NO_NPCS					= (1<<4),
	FL_TRACE_EXTRASIZE					= (1<<5),
	FL_TRACE_HITBOXES					= (1<<6),
	FL_TRACE_HIT_CORPSES				= (1<<7),
	FL_TRACE_NO_TRANS_WORLDBRUSH		= (1<<8),
};

struct trace_t
{
	trace_t():
		flags(FL_TR_NONE),
		fraction(0),
		hitentity(NO_ENTITY_INDEX),
		hitgroup(0),
		hitbox(NO_POSITION)
		{};

	bool startSolid( void ) const
	{
		return (flags & FL_TR_STARTSOLID) ? true : false;
	}
	bool allSolid( void ) const
	{
		return (flags & FL_TR_ALLSOLID) ? true : false;
	}
	bool inOpen( void ) const
	{
		return (flags & FL_TR_INOPEN) ? true : false;
	}
	bool inWater( void ) const
	{
		return (flags & FL_INWATER) ? true : false;
	}
	bool noHit( void ) const
	{
		return (fraction == 1.0) ? true : false;
	}

	Int32 flags;

	Double fraction;
	Vector endpos;
	plane_t plane;
	Vector deltavelocity;

	entindex_t hitentity;
	Int32 hitgroup;
	Int32 hitbox;
};

struct trace_interface_t
{
	Int32			(*pfnTestPlayerPosition)( hull_types_t hulltype, Int32 flags, const class Vector& position );
	Int32			(*pfnPointContents)( const Vector& position, Int32* truecontents );
	Int32			(*pfnTruePointContents)( const Vector& position );
	Int32			(*pfnHullPointContents)( const hull_t* phull, Int32 num, const Vector& position );
	void			(*pfnPlayerTrace)( const Vector& start, const Vector& end, Int32 traceflags, hull_types_t hulltype, Int32 ignore_ent, trace_t& trace );
	void			(*pfnTraceLine)( const Vector& start, const Vector& end, Int32 traceflags, hull_types_t hulltype, Int32 ignore_ent, trace_t& trace );
	const hull_t*	(*pfnHullForBSP)( Int32 entity, hull_types_t hulltype, Vector* poffset );
	Float			(*pfnTraceModel)( Int32 entity, const Vector& start, const Vector& end, hull_types_t hulltype, Int32 flags, trace_t& trace );
	const Char*		(*pfnTraceTexture)( Int32 groundentity, const Vector& start, const Vector& end );
};
#endif //TRACE_H