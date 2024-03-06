/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef REF_PARAMS_H
#define REF_PARAMS_H

#include "entity_state.h"

class Vector;
struct usercmd_t;
struct movevars_t;

enum renderflags_t
{
	RENDERER_FL_NONE	= 0,
	RENDERER_FL_DONT_DRAW_MODELS	= (1<<0),
	RENDERER_FL_DONT_DRAW_PARTICLES	= (1<<1)
};

struct ref_params_t
{
	ref_params_t():
		frametime(0),
		time(0),
		paused(false),
		onground(NO_ENTITY_INDEX),
		waterlevel(0),
		idealpitch(0),
		viewsize(0),
		screenwidth(0),
		screenheight(0),
		pcmd(nullptr),
		pmovevars(nullptr),
		nodraw(false),
		smoothsteps(false),
		renderflags(RENDERER_FL_NONE)
		{};

	// view origin
	Vector v_origin;
	// view angles
	Vector v_angles;
	// view height
	Vector v_height;

	// Client frametime
	Double frametime;
	// Client time
	Double time;

	// Tells if game is paused
	bool paused;
	// ground entity 
	Int32 onground;
	// Water level
	Int32 waterlevel;

	// player's velocity
	Vector pl_velocity;
	// player origin
	Vector pl_origin;
	// player's view offset
	Vector pl_viewoffset;
	// player's view angles
	Vector pl_viewangles;
	// player's punchangles
	Vector pl_punchangle;
	// player's ideal pitch
	Float idealpitch;

	// View size in degrees
	Float viewsize;

	// screen width
	Float screenwidth;
	// screen height
	Float screenheight;

	// Last usercmd predicted
	const usercmd_t* pcmd;
	// Movevars used
	const movevars_t* pmovevars;

	// if true, nothing is drawn by the engine
	bool nodraw;
	// if true, step-smoothing is enabled
	bool smoothsteps;

	// rendererflags
	Int32 renderflags;
};

#endif //REF_PARAMS_H