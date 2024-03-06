/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2020
All Rights Reserved.
===============================================
*/

#ifndef MIPTEX_H
#define MIPTEX_H

#define MAX_TEXNAME_LENGTH		16
#define MAX_MIPTEX				4
#define MAX_TEXTURE_ANIMS		16

struct dmiptexlump_t
{
	dmiptexlump_t():
		nummiptex(0)
	{
		memset(dataoffsets, 0, sizeof(dataoffsets));
	}

	Int32 nummiptex;
	Int32 dataoffsets[MAX_MIPTEX];
};

struct dmiptex_t
{
	dmiptex_t():
		width(0),
		height(0)
	{
		memset(name, 0, sizeof(name));
		memset(offsets, 0, sizeof(offsets));
	}

	Char name[MAX_TEXNAME_LENGTH];
	Uint32 width;
	Uint32 height;

	Uint32 offsets[MAX_MIPTEX];
};
#endif //MIPTEX_H