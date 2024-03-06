/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef USERCMD_H
#define USERCMD_H

class Vector;

struct usercmd_t
{
	usercmd_t():
		cmdidx(0),
		lerp_msec(0),
		msec(0),
		forwardmove(0),
		sidemove(0),
		upmove(0),
		weaponselect(0),
		impulse(0),
		buttons(0)
	{
	}

	Uint64 cmdidx;		// Index of usercmd

	Uint32 lerp_msec;	// interpolation time on client
	byte msec;			// duration in ms of command
	Vector viewangles;	// view angles at time of cmd send

	Float forwardmove;	// forward movement velocity
	Float sidemove;		// side movement velocity
	Float upmove;		// upwards movement velocity
	byte weaponselect;	// weapon selection
	byte impulse;		// impulse command value
	Uint32 buttons;		// buttons pressed
};

#endif //USERCMD_H