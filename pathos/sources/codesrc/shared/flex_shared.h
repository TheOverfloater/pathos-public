/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FLEX_SHARED_H
#define FLEX_SHARED_H

#include "vbmformat.h"

enum flexmsg_t
{
	MSG_FLEX_PRECACHE = 0,
	MSG_FLEX_SCRIPT
};

enum flextypes_t
{
	FLEX_NPC_TYPE_NONE = -1,
	FLEX_NPC_TYPE_HUMAN,
	FLEX_NPC_TYPE_UNUSED,

	NUM_FLEX_NPC_TYPES
};

enum flexaistates_t
{
	FLEX_AISTATE_NONE = -1,
	FLEX_AISTATE_IDLE,
	FLEX_AISTATE_AFRAID,
	FLEX_AISTATE_DEAD,
	FLEX_AISTATE_COMBAT,
	FLEX_AISTATE_SCRIPT,

	NUM_FLEX_AISTATES
};

enum fixedflexes_t
{
	FLEX_MOUTH_OPEN = 0,
	FLEX_BLINK,
	FLEX_EYES_SQUINT,
	FLEX_EYES_WIDE,

	NB_FIXED_FLEXES
};

struct flexbind_t
{
	flexbind_t():
		time(0),
		strength(0)
	{}

	Float time;
	Float strength;
};

struct flexcontroller_t
{
	flexcontroller_t():
		index(0)
		{}

	Int32 index;
	CString name;

	CArray<flexbind_t> binds;
};

struct flexscript_t
{
	flexscript_t():
		duration(0),
		flags(0)
		{}

	CString filename;

	CArray<flexcontroller_t> controllers;
	Float duration;
	Int32 flags;
};

struct flexstate_t
{
	flexstate_t():
		time(0),
		nextblink(0),
		pscript(nullptr)
	{
		memset(prev_values, 0, sizeof(prev_values));
		memset(values, 0, sizeof(values));

		for(Uint32 i = 0; i < MAX_VBM_FLEXES; i++)
		{
			indexmap[i] = -1;
			r_indexmap[i] = -1;
		}
	}

	Float time;
	Float nextblink;

	Float prev_values[MAX_VBM_FLEXES];
	Float values[MAX_VBM_FLEXES];

	Int32 indexmap[MAX_VBM_FLEXES];
	Int32 r_indexmap[MAX_VBM_FLEXES];

	const flexscript_t* pscript;
};
#endif