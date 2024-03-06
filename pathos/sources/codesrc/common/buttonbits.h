/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef BUTTONBITS_H
#define BUTTONBITS_H

// Contains button bits used by the input code
enum buttonbits_t
{
	IN_ATTACK				= (1<<0),
	IN_JUMP					= (1<<1),
	IN_DUCK					= (1<<2),
	IN_FORWARD				= (1<<3),
	IN_BACK					= (1<<4),
	IN_CANCEL				= (1<<5),
	IN_LEAN					= (1<<6),
	IN_MOVELEFT				= (1<<7),
	IN_MOVERIGHT			= (1<<8),
	IN_ATTACK2				= (1<<9),
	IN_WALKMODE				= (1<<10),
	IN_RELOAD				= (1<<11),
	IN_SPRINT				= (1<<12),
	IN_HEAL					= (1<<13),
	IN_USE					= (1<<14),
	IN_UP					= (1<<15),
	IN_DOWN					= (1<<16),
	IN_SPECIAL				= (1<<17)
};
#endif