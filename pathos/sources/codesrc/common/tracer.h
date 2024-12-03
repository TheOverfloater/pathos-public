/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRACER_H
#define TRACER_H

enum tracer_type_t
{
	TRACER_NORMAL = 0,
	TRACER_GRAVITY,
	TRACER_SLOW_GRAVITY
};

struct tracer_t
{
	tracer_t():
		length(0),
		die(0),
		alpha(0),
		masteralpha(0),
		width(0),
		type(TRACER_NORMAL),
		pprev(nullptr),
		pnext(nullptr)
	{}

	Vector origin;
	Vector color;
	Vector velocity;
	Float length;
	Float die;
	Float alpha;
	Float masteralpha;
	Float width;
	tracer_type_t type;

	tracer_t* pprev;
	tracer_t* pnext;
};
#endif //TRACER_H