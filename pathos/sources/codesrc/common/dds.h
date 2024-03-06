/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef DDS_H
#define DDS_H

#include "constants.h"

#define DDS_MAGIC					0x20534444

#define DDSD_CAPS                   0x00000001
#define DDSD_PIXELFORMAT            0x00001000
#define DDPF_FOURCC                 0x00000004

#define DDS_DATA_OFFSET				128

#define D3DFMT_DXT1					'1TXD'    //  DXT1 compression texture format 
#define D3DFMT_DXT5					'5TXD'    //  DXT5 compression texture format 

struct dds_header_t 
{
	byte bMagic[4];
	byte bSize[4];
	byte bFlags[4];
	byte bHeight[4];
	byte bWidth[4];
	byte bPitchOrLinearSize[4];

	byte bPad1[52];

	byte bPFSize[4];
	byte bPFFlags[4];
	byte bPFFourCC[4];
};

bool DDS_Load( const Char* pstrFilename, const byte* pfile, byte*& pdata, Uint32& width, Uint32& height, Uint32& bpp, Uint32& size, texture_compression_t& compression, pfnPrintf_t pfnPrintFn );
#endif // DDS_H