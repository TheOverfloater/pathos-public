/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef PM_SHARED_H
#define PM_SHARED_H

#include "entity_state.h"
#include "trace.h"
#include "movevars.h"

struct pm_info_t
{
	pm_info_t():
		clientindex(0),
		waterjumptime(0)
		{}

	// Player state
	entity_state_t playerstate;
	// Client index
	Uint32 clientindex;

	// Movevars
	movevars_t movevars;
	// Last view angles
	Vector oldangles;

	// Waterjump time
	Float waterjumptime;

	// Player mins/maxs for each hull
	Vector player_mins[MAX_MAP_HULLS];
	Vector player_maxs[MAX_MAP_HULLS];
};
#endif //PM_SHARED_H