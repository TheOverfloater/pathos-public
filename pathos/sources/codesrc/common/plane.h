/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef PLANE_H
#define PLANE_H

#include "datatypes.h"
#include "common.h"
#include "vector.h"

enum planetype_t
{
	PLANE_UNDEFINED = -1,
	PLANE_X = 0,
	PLANE_Y,
	PLANE_Z,
	PLANE_AX,
	PLANE_AY,
	PLANE_AZ
};

struct plane_t
{
	plane_t():
		dist(0),
		type(PLANE_UNDEFINED),
		signbits(0)
	{}

	// Plane normal
	Vector normal;
	// Distance element
	Float dist;

	// Plane type
	Int32 type;
	byte signbits;
};

// Assigns sign bits for the plane
extern inline Int32 SignbitsForPlane( const plane_t& plane );

#include "plane_inline.hpp"
#endif //PLANE_H