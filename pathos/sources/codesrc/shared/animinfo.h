/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ANIMINFO_H
#define ANIMINFO_H

#include "entity_state.h"

struct entity_animinfo_t
{
	entity_animinfo_t():
		numbones(0),
		lastframe(0),
		lastsequence(0),
		prevframe_frame(0),
		prevframe_sequence(0),
		lastmouth(0),
		scale(0),
		lastsequencetime(0)
	{
		memset(bones, 0, sizeof(bones));
		memset(weightbones, 0, sizeof(weightbones));
		memset(rotation, 0, sizeof(rotation));
		memset(lastcontroller, 0, sizeof(lastcontroller));
		memset(lastblending, 0, sizeof(lastblending));
	}

	Float	bones[MAXSTUDIOBONES][3][4];
	Float	weightbones[MAXSTUDIOBONES][3][4];
	Int32	numbones;

	Float	lastframe;
	Int32	lastsequence;

	// Used for events
	Float	prevframe_frame;
	Int32	prevframe_sequence;

	Float	rotation[3][4];
	Vector	lastangles;
	Vector	lastorigin;

	byte	lastmouth;
	byte	lastcontroller[MAX_CONTROLLERS];
	byte	lastblending[MAX_BLENDING];

	Float	scale;

	// Used by interpolation
	Double	lastsequencetime;
};
#endif //ANIMINFO_H