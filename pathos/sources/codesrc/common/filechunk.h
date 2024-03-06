/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FILECHUNK_H
#define FILECHUNK_H

struct filechunk_t
{
	filechunk_t():
		fileid(0),
		chunkindex(0),
		dataoffset(0),
		datasize(0)
		{}

	Int32 fileid;
	Uint32 chunkindex;

	Uint32 dataoffset;
	Uint32 datasize;
};
#endif