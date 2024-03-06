/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_BASIC_VERTEX_H
#define R_BASIC_VERTEX_H

struct basic_vertex_t
{
	basic_vertex_t()
	{
		memset(origin, 0, sizeof(origin));
		memset(color, 0, sizeof(color));
		memset(texcoords, 0, sizeof(texcoords));
		memset(padding, 0, sizeof(padding));
	}

	vec4_t origin;
	vec4_t color;
	Float texcoords[2];

	byte padding[24];
};
#endif //R_BASIC_VERTEX_H