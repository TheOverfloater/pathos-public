/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef EDICT_H
#define EDICT_H

#include "entity_state.h"
#include "entitydata.h"

struct edict_t;
struct entity_vbmhulldata_t;
struct entity_animinfo_t;

typedef CLinkedList<edict_t*> EdictChainType_t;

struct edict_fields_t
{
	edict_fields_t():
		classname(NO_STRING_VALUE),
		globalname(NO_STRING_VALUE),
		modelname(NO_STRING_VALUE),
		target(NO_STRING_VALUE),
		targetname(NO_STRING_VALUE),
		netname(NO_STRING_VALUE),
		message(NO_STRING_VALUE),
		parent(NO_STRING_VALUE),
		viewmodel(NO_STRING_VALUE),
		noise(NO_STRING_VALUE),
		noise1(NO_STRING_VALUE),
		noise2(NO_STRING_VALUE),
		noise3(NO_STRING_VALUE)
	{}

	string_t classname;
	string_t globalname;
	string_t modelname;
	string_t target;
	string_t targetname;
	string_t netname;
	string_t message;
	string_t parent;
	string_t viewmodel;
	string_t noise;
	string_t noise1;
	string_t noise2;
	string_t noise3;
};

struct edict_t
{
	edict_t():
		entindex(0),
		clientindex(NO_CLIENT_INDEX),
		free(true),
		freetime(0),
		identifier(0),
		pareachain(nullptr),
		parealink(nullptr),
		pprivatedata(nullptr),
		paniminfo(nullptr),
		pvbmhulldata(nullptr)
	{
	}

	// entindex
	entindex_t entindex;
	// client index(if client)
	Int32 clientindex;
	// if the edict was freed up
	bool free;
	// Time entity was freed
	Double freetime;
	// unique identifier for this edict
	Uint32 identifier;

	// area node we're linked to
	EdictChainType_t* pareachain;
	// linked to a division node or leaf
	EdictChainType_t::link_t* parealink;

	// Leafs this entity is touching
	CArray<Uint32> leafnums;

	// Class data
	void *pprivatedata;
	//  Entity state
	entity_state_t state;
	// Entity fields
	edict_fields_t fields;

	// animation state information
	entity_animinfo_t* paniminfo;
	// vbm hull data if any
	entity_vbmhulldata_t* pvbmhulldata;
};
#endif