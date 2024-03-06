/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MLIGHT_H
#define MLIGHT_H

// Max lights a studiomodel can have
static const Uint32 MAX_ENT_MLIGHTS = 6;

struct mlight_t
{
	mlight_t():
		entindex(0),
		radius(0),
		noblend(false)
		{}

	entindex_t entindex;

	Vector	origin;
	Float	radius;
	Vector	color;

	Vector mins;
	Vector maxs;

	bool noblend;
};

struct mlightinfo_t
{
	mlightinfo_t():
		lighttime(0),
		strength(0),
		prev_strength(0),
		occluded(false)
		{}

	mlight_t light;
	Double lighttime;
	Float strength;
	Float prev_strength;
	bool occluded;
};

#endif
