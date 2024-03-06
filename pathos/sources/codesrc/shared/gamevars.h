/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMEVARS_H
#define GAMEVARS_H

#include "trace.h"
#include "constants.h"

struct gamevars_t
{
	gamevars_t():
		time(0),
		frametime(0),
		gametime(0),
		paused(false),
		force_retouch(false),
		numentities(0),
		maxclients(0),
		predict_player(0)
		{}

	Double time;
	Double frametime;
	Double gametime;

	bool paused;
	bool force_retouch;
	CString levelname;

	trace_t globaltrace;

	Int32 numentities;
	Int32 maxclients;

	Int32 predict_player;
};
#endif //GAMEVARS_H