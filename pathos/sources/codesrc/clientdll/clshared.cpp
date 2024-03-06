/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "vector.h"

// Autoaim adjust speed
const Float AUTOAIM_ADJ_SPEED = 12;

//=============================================
//
//=============================================
void CL_UpdateAutoAim( Double frametime, const Vector& idealAutoAim, Vector& currentAutoAim )
{
	if(frametime > 1.0)
		frametime = 1.0;

	Float diffX = SDL_fabs(idealAutoAim[0] - currentAutoAim[0]);
	Float diffY = SDL_fabs(idealAutoAim[1] - currentAutoAim[1]);
	if(!diffX && !diffY)
		return;

	Float mod[2] = {0,0};
	if(diffX == diffY)
	{
		mod[0] = 1.0;
		mod[1] = 1.0;
	}
	else if(diffX > diffY)
	{
		mod[0] = 1.0;
		mod[1] = diffX / diffY;
	}
	else
	{
		mod[0] = diffY / diffX;
		mod[1] = 1.0;
	}

	for(Uint32 i = 0; i < 2; i++)
	{
		Float diff = idealAutoAim[i] - currentAutoAim[i];
		Float sgn = diff > 0 ? 1 : -1;
		diff = SDL_fabs(diff);

		Float add = AUTOAIM_ADJ_SPEED * frametime * mod[i];
		if(add > diff)
			currentAutoAim[i] = idealAutoAim[i];
		else
			currentAutoAim[i] += add * sgn;
	}
}