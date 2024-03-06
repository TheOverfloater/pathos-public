/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "ogg_common.h"

//=============================================
// @brief
//
//=============================================
size_t AR_readOgg( void* dst, size_t size1, size_t size2, void* fh )
{
    snd_oggcache_t* of = reinterpret_cast<snd_oggcache_t*>(fh);
    size_t len = size1 * size2;
	
	if ( of->pcurptr + len > of->pfileptr + of->filesize )
        len = of->pfileptr + of->filesize - of->pcurptr;

    memcpy( dst, of->pcurptr, len );
    of->pcurptr += len;
    return len;
}

//=============================================
// @brief
//
//=============================================
Int32 AR_seekOgg( void *fh, ogg_int64_t to, Int32 type )
{
    snd_oggcache_t* of = reinterpret_cast<snd_oggcache_t*>(fh);

    switch( type ) {
        case SEEK_CUR:
			of->pcurptr += to;
            break;
        case SEEK_END:
			of->pcurptr = of->pfileptr + of->filesize - to;
            break;
        case SEEK_SET:
            of->pcurptr = of->pfileptr + to;
            break;
        default:
            return -1;
    }

	if ( of->pcurptr < of->pfileptr ) 
	{
        of->pcurptr = of->pfileptr;
        return -1;
    }

	if ( of->pcurptr > of->pfileptr + of->filesize ) 
	{
        of->pcurptr = of->pfileptr + of->filesize;
        return -1;
    }

    return 0;
}

//=============================================
// @brief
//
//=============================================
Int32 AR_closeOgg( void* fh )
{
    return 0;
}

//=============================================
// @brief
//
//=============================================
long AR_tellOgg( void *fh )
{
    snd_oggcache_t* of = reinterpret_cast<snd_oggcache_t*>(fh);
	return (of->pcurptr - of->pfileptr);
}