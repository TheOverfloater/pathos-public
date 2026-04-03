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
extern bool TGA_Write( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, const Char* pstrFilename, const file_interface_t& fileFuncs, pfnPrintf_t pfnPrintFn, Uint32* ptrCompressionRatio = nullptr, Uint32* ptrBytesWritten = nullptr, bool flipVertical = false );
extern void TGA_BuildFile( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, CBuffer& fileBuffer, Uint32* ptrCompressionRatio, bool flipVertical );
extern void TGA_BuildImageData( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, CBuffer& fileBuffer, tga_header_t*& pheader, Uint32& compressionPercentage, bool flipVertical );
#endif // TGA_H