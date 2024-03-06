/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SV_PHYSICS_H
#define SV_PHYSICS_H

class CCVar;

struct saved_move_t
{
	Vector saved_origin;
	edict_t* psave_edict;
};

struct svphysics_t
{
	svphysics_t():
		numsavedmovingents(0),
		touchlinksemaphore(false)
		{}

	// Array of saved entity states
	CArray<saved_move_t> savedmovingentities;
	// Number of saved entities
	Uint32 numsavedmovingents;

	// Touch link semaphore for safety
	bool touchlinksemaphore;
};

extern CCVar* g_psv_maxvelocity;
extern CCVar* g_psv_gravity;
extern CCVar* g_psv_bounce;
extern CCVar* g_psv_stepsize;
extern CCVar* g_psv_friction;
extern CCVar* g_psv_stopspeed;

extern svphysics_t g_serverPhysics;

extern void SV_Physics_Init( void );
extern void SV_Physics( void );
extern void SV_Impact( edict_t* pentity1, edict_t* pentity2, const trace_t& trace );
extern bool SV_CheckBottom( edict_t* pedict );
#endif //SV_PHYSICS_H