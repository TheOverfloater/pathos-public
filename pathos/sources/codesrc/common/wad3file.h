/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef WAD3FILE_H
#define WAD3FILE_H

#define MIPLEVELS 4

// WAD file format extension
static const Char WAD_FILE_EXTENSION[] = ".wad";

struct wad3info_t
{
	wad3info_t():
		numlumps(0),
		infotableofs(0)
	{
		memset(identification, 0, sizeof(identification));
	}

    Char		identification[4];                     // should be WAD2/WAD3
    Int32		numlumps;
    Int32		infotableofs;
};

struct wad3lumpinfo_t
{
	wad3lumpinfo_t():
		filepos(0),
		disksize(0),
		size(0),
		type(0),
		compression(0),
		pad1(0),
		pad2(0)
	{
		memset(name, 0, sizeof(name));
	}

    Int32	filepos;
    Int32	disksize;
    Int32	size;
    Char	type;
    Char	compression;
    Char	pad1;
	Char	pad2;
    Char	name[16];
};

#endif //WAD3FILE_H