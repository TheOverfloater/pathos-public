/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_FBO_H
#define R_FBO_H

struct en_texalloc_t;

struct fbobind_t
{
	fbobind_t():
		ptexture1(nullptr),
		ptexture2(nullptr),
		pdepth(nullptr),
		fboid(0),
		rboid1(0),
		rboid2(0)
	{}

	en_texalloc_t* ptexture1;
	en_texalloc_t* ptexture2;
	en_texalloc_t* pdepth;

	GLuint fboid;
	GLuint rboid1;
	GLuint rboid2;
};

#endif //R_FBO_H