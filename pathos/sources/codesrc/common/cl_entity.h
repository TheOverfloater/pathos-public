/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CL_ENTITY_H
#define CL_ENTITY_H

#include "entity_state.h"

// Maximum attachments on an entity
static const Int32 MAX_ATTACHMENTS = 32;
// Maximum previous animation states
static const Int32 MAX_PREV_ANIMSTATES = 4;

struct cache_model_t;
struct entity_extrainfo_t;
struct entity_vbmhulldata_t;

struct mouth_t
{
	mouth_t():
		mouthopen(0)
		{}

	byte mouthopen;
};

struct latchedstates_t
{
	latchedstates_t():
		animtime(0),
		sequencetime(0),
		sequence(0),
		frame(0)
		{
			memset(controllers, 0, sizeof(controllers));
			memset(blending, 0, sizeof(blending));
			memset(prevseqblending, 0, sizeof(prevseqblending));
		}

	Double animtime;
	Double sequencetime;

	Vector origin;
	Vector angles;

	Int32 sequence;
	Float frame;

	Float controllers[MAX_CONTROLLERS];
	Float blending[MAX_BLENDING];
	Float prevseqblending[MAX_BLENDING];
};

struct glowstate_t
{
	glowstate_t():
		currentalpha(0),
		lastrendertime(0)
		{
		}

	Float currentalpha;
	Double lastrendertime;
	CArray<Float> lastfrac;
};

struct cl_entity_t
{
	cl_entity_t():
		entindex(0),
		identifier(0),
		player(false),
		pmodel(nullptr),
		visframe(0),
		pextradata(nullptr),
		pvbmhulldata(nullptr),
		eventframe(0)
	{
	}

	const Vector& getAttachment( Int32 index ) const
	{
		if(index < 0 || index >= MAX_ATTACHMENTS)
			return curstate.origin;
		else
			return attachments[index];
	}

public:
	// Index into cl_entities
	entindex_t entindex;
	// Identifier of entity edict
	Uint32 identifier;
	// TRUE if it's a player
	bool player;

	// State from previous frame
	entity_state_t prevstate;
	// Predicted entity state
	entity_state_t curstate;

	// Previous animation states for blending
	latchedstates_t latched;
	// Render info for env_glows
	glowstate_t glowstate;

	// mouth state
	mouth_t mouth;

	// Attachment points
	Vector attachments[MAX_ATTACHMENTS];

	// Model to render
	const cache_model_t* pmodel;
	// Last visframe this was drawn on
	Int32 visframe;

	// Pointer to entity extradata
	entity_extrainfo_t* pextradata;
	// Pointer to vbm hull data
	entity_vbmhulldata_t* pvbmhulldata;

	// Last frame on which an event was played
	Float eventframe;
};

#endif //CL_ENTITY_H