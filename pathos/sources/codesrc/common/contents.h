/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CONTENTS_H
#define CONTENTS_H

//
// Leaf contents values
//

enum contents_t
{
	CONTENTS_NONE		= 0,
	CONTENTS_EMPTY		= -1,
	CONTENTS_SOLID		= -2,
	CONTENTS_WATER		= -3,
	CONTENTS_SLIME		= -4,
	CONTENTS_LAVA		= -5,
	CONTENTS_SKY		= -6,
	CONTENTS_ORIGIN		= -7, // Removed at compile
	CONTENTS_CLIP		= -8, // Changed to CONTENTS_SOLID
	CONTENTS_LADDER		= -16,
	CONTENTS_CONVEYOR	= -17,
	CONTENT_FRICTIONMOD	= -19,

	CONTENTS_MIN
};
#endif //CONTENTS_H