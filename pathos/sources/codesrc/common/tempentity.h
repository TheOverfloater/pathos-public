/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TEMPENTITY_H
#define TEMPENTITY_H

#include "cl_entity.h"
#include "contents.h"

struct tempentity_t
{
	tempentity_t():
		flags(0),
		die(0),
		framemax(0),
		flickertime(0),
		fadespeed(0),
		startrenderamt(0),
		bouncefactor(0),
		buoyancy(0),
		waterfriction(0),
		soundtype(0),
		prevcontents(CONTENTS_EMPTY),
		pprev(nullptr),
		pnext(nullptr)
		{}

	Int32 flags;

	Float die;
	Float framemax;
	Float flickertime;

	Vector add;

	Float fadespeed;
	Float startrenderamt;
	Float bouncefactor;
	Float buoyancy;
	Float waterfriction;
	Int32 soundtype;
	Int32 prevcontents;

	// Entity state data
	cl_entity_t entity;

	// Pointer to previous in list
	tempentity_t* pprev;
	// Pointer to next in list
	tempentity_t* pnext;
};

#endif //TEMPENTITY_H