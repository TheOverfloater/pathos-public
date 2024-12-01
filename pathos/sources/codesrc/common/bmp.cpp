/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

// Code by valina354
// Modified by Overfloater

#include "includes.h"
#include "bmp.h"
#include "bmpformat.h"
#include "file_interface.h"
#include "r_common.h"

//=============================================
// @brief
//
//=============================================
bool BMP_Load( const char* pstrFilename, const byte* pfile, byte*& pdata, Uint32& width, Uint32& height, Uint32& bpp, Uint32& size, texture_compression_t& compression, pfnPrintf_t pfnPrintFn ) 
{
    const bmp_header_t* ptrBmpHeader = reinterpret_cast<const bmp_header_t*>(pfile);
    if (ptrBmpHeader->magic != BMP_MAGIC_NUMBER)
    {
        pfnPrintFn("%s Magic Header check failed.\n", pstrFilename);
        delete[] pdata;
        return false;
    }

    width = ptrBmpHeader->width;
    height = ptrBmpHeader->height;

    if (!Common::IsPowerOfTwo(width) || !Common::IsPowerOfTwo(height))
    {
        pfnPrintFn("%s is not a power of two texture.\n", pstrFilename);
        delete[] pdata;
        return false;
    }

    Uint32 pxsize = width * height;
    bpp = ptrBmpHeader->bits_per_pixel;
    pdata = new byte[pxsize * 4]; // Always RGBA

    if (bpp == 24) 
    {
        for (Uint32 i = 0; i < pxsize; i++) 
        {
            const byte* psrc = pfile + ptrBmpHeader->data_offset + i * 3;
            byte *pdest = pdata + i * 4;

            pdest[0] = psrc[2];
            pdest[1] = psrc[1];
            pdest[2] = psrc[0];
            pdest[3] = 255;
        }
    }
    else if (bpp == 32) 
    {
        for (Uint32 i = 0; i < size; i++) 
        {
            const byte* psrc = pfile + ptrBmpHeader->data_offset + i * 4;
            byte *pdest = pdata + i * 4;

            pdest[0] = psrc[2];
            pdest[1] = psrc[1];
            pdest[2] = psrc[0];
            pdest[3] = psrc[3];
        }
    }
    else
    {
        pfnPrintFn("%s - File '%d' has an unsupported bits per pixel format %u\n", pstrFilename, bpp);
        delete[] pdata;
        return false;
    }

    // Texture needs to be flipped vertically
    R_FlipTexture(width, height, 4, false, true, pdata);

    return true;
}