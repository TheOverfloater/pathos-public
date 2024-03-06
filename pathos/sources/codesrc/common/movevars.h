/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MOVEVARS_H
#define MOVEVARS_H

struct movevars_t
{
	movevars_t():
		gravity(0),
		stopspeed(0),
		maxspeed(0),
		accelerate(0),
		airaccelerate(0),
		wateraccelerate(0),
		friction(0),
		edgefriction(0),
		waterfriction(0),
		entgravity(0),
		bounce(0),
		stepsize(0),
		maxvelocity(0),
		waterdist(0),
		maxclients(0),
		holdtoduck(false)
	{}

	Float gravity;
	Float stopspeed;
	Float maxspeed;
	
	Float accelerate;
	Float airaccelerate;
	Float wateraccelerate;

	Float friction;
	Float edgefriction;
	Float waterfriction;

	Float entgravity;
	Float bounce;
	Float stepsize;
	Float maxvelocity;
	Float waterdist;

	Uint32 maxclients;
	bool holdtoduck;
};
#endif