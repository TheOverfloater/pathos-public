/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef OGG_SHARED_H
#define OGG_SHARED_H

#include <vorbis/vorbisfile.h>

#include "common.h"

struct snd_oggcache_t
{
	snd_oggcache_t():
		level(RS_LEVEL_UNDEFINED),
		pcurptr(nullptr),
		pfileptr(nullptr),
		filesize(0)
		{}

	CString filepath;

	rs_level_t level;

	byte* pcurptr;
	byte* pfileptr;

	Uint32 filesize;
};

extern size_t AR_readOgg( void* dst, size_t size1, size_t size2, void* fh );
extern int AR_seekOgg( void *fh, ogg_int64_t to, int type );
extern int AR_closeOgg( void* fh );
extern long AR_tellOgg( void *fh );
#endif //OGG_SHARED_H