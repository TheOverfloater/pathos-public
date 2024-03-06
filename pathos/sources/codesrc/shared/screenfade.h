/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SCREENFADE_H
#define SCREENFADE_H

// Max env_fade layers
static const Uint32 MAX_FADE_LAYERS = 8;

enum fadeflags_t
{
	FL_FADE_IN			= (1<<0),
	FL_FADE_OUT			= (1<<1),
	FL_FADE_MODULATE	= (1<<2),
	FL_FADE_STAYOUT		= (1<<3),
	FL_FADE_CLEARGAME	= (1<<4),
	FL_FADE_PERMANENT	= (1<<5)
};

struct screenfade_t
{
	screenfade_t():
		speed(0),
		end(0),
		totalend(0),
		reset(0),
		color(0, 0, 0),
		alpha(0),
		flags(0),
		curfade(0)
		{}

	Float speed;
	Double end;
	Float totalend;
	Float reset;
	color24_t color;
	byte alpha;
	Int32 flags;
	Float curfade;
};

#endif // SCREENFADE_H
