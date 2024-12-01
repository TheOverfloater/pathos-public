#include "includes.h"
#include "bmp.h"
#include "bmpformat.h"
#include "file_interface.h"
#include "r_common.h"

bool BMP_Load(const char* pstrFilename, const byte* pfile, byte*& pdata, Uint32& width, Uint32& height, Uint32& bpp, Uint32& size, texture_compression_t& compression, pfnPrintf_t pfnPrintFn) {
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
    bpp = ptrBmpHeader->bits_per_pixel;
    size = width * height * (bpp / 8);
    pdata = new byte[size];
    memcpy(pdata, pfile + ptrBmpHeader->data_offset, size);

    if (bpp == 24) {
        for (Uint32 i = 0; i < size; i += 3) {
            byte temp = pdata[i];
            pdata[i] = pdata[i + 2];
            pdata[i + 2] = temp;
        }
    }
    else if (bpp == 32) {
        for (Uint32 i = 0; i < size; i += 4) {
            byte temp = pdata[i];
            pdata[i] = pdata[i + 2];
            pdata[i + 2] = temp;
        }
    }
    else
    {
        pfnPrintFn("%s unsupported bits per pixel: %u\n", pstrFilename, bpp);
        delete[] pdata;
        return false;
    }

    return true;
}