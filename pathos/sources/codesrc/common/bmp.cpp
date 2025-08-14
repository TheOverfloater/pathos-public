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

//=============================================
// @brief
//
//=============================================
bool BMP_Load8Bit(const char* pstrFilename, const byte* pfile, byte*& ppalette, byte*& pdata, Uint32& width, Uint32& height, Uint32& size, texture_compression_t& compression, pfnPrintf_t pfnPrintFn)
{
    const bmp_header_t* ptrBmpHeader = reinterpret_cast<const bmp_header_t*>(pfile);
    if (ptrBmpHeader->magic != BMP_MAGIC_NUMBER)
    {
        pfnPrintFn("%s Magic Header check failed.\n", pstrFilename);
        delete[] pdata;
        return false;
    }

    if(ptrBmpHeader->bits_per_pixel != 8)
    {
        pfnPrintFn("%s - File '%d' has an unsupported bits per pixel format %u, only 8-bit supported.\n", pstrFilename, ptrBmpHeader->bits_per_pixel);
        return false;
    }

    Uint32 nbColorsUsed;
    Uint64 paletteBytesCount;
    if(ptrBmpHeader->colors_used == 0)
    {
        nbColorsUsed = 256;
        paletteBytesCount = (1UL << ptrBmpHeader->bits_per_pixel) * sizeof(color32_t);
    }
    else
    {
        paletteBytesCount = ptrBmpHeader->colors_used * sizeof(color32_t);
        nbColorsUsed = ptrBmpHeader->colors_used;
    }

    // Copy over palette
    const byte* psrc = pfile + sizeof(bmp_header_t);
    const byte* psrcpalette = psrc;

    ppalette = new byte[256*3];
    memset(ppalette, 0, sizeof(byte)*256*3);

    for(Uint32 i = 0; i < nbColorsUsed; i++)
    {
        ppalette[i*3] = psrcpalette[i*4];
        ppalette[i*3+1] = psrcpalette[i*4+1];
        ppalette[i*3+2] = psrcpalette[i*4+2];
    }

    // Now load the actual pixel bytes
    const byte* psrcpixels = psrc + paletteBytesCount;

    width = ptrBmpHeader->width;
    height = ptrBmpHeader->height;

    Uint32 fileOffset = psrcpixels - pfile;
    Uint32 pixelCount = ptrBmpHeader->file_size - fileOffset;

    // Data is actually stored rounded up to multiples of 4
    Uint32 trueWidth = (width % 4) != 0 ? ((width / 4) * 4 + 4) : width;

    pdata = new byte[pixelCount];
    memcpy(pdata, psrcpixels, sizeof(byte)*pixelCount);

    return true;
}