/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#ifndef TGA_H
#define TGA_H

#include "texturemanager.h"
#include "tgaformat.h"
#include "constants.h"

#define TGA_FLIP_HORIZONTAL	8
#define TGA_FLIP_VERTICAL	16

struct file_interface_t;

extern bool TGA_Load( const Char* pstrFilename, const byte* pfile, byte*& pdata, Uint32& width, Uint32& height, Uint32& bpp, Uint32& size, texture_compression_t& compression, pfnPrintf_t pfnPrintFn );
extern bool TGA_Write( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, const Char* pstrFilename, const file_interface_t& fileFuncs, pfnPrintf_t pfnPrintFn );
extern void TGA_BuildFile( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, byte*& poutbuffer, Uint32& outsize );
extern void TGA_WriteData( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, byte* pbuffer );
#endif // TGA_H