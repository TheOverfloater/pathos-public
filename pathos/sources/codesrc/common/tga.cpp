/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "texturemanager.h"
#include "tga.h"
#include "file_interface.h"
#include "r_common.h"

//=============================================
// @brief Loads a TGA file and returns it's data
//
// @param pfile Pointer to raw file data
// @param pdata Destination pointer for image data
// @param width Reference to texture width variable
// @param height Reference to texture height variable
// @param bpp Reference to texture bit depth variable
// @param size To hold the output data size
// @param compression Reference to texture compression variable
// @param pborder Pointer to variable holding the border color
// @return TRUE if successfully loaded, FALSE otherwise
//=============================================
bool TGA_Load( const Char* pstrFilename, const byte* pfile, byte*& pdata, Uint32& width, Uint32& height, Uint32& bpp, Uint32& size, texture_compression_t& compression, pfnPrintf_t pfnPrintFn )
{
	// Set basic information
	const tga_header_t *ptrTgaHeader = reinterpret_cast<const tga_header_t *>(pfile);
	if(ptrTgaHeader->datatypecode != TGA_DATATYPE_RGB
		&& ptrTgaHeader->datatypecode != TGA_DATATYPE_RLE_RGB 
		|| ptrTgaHeader->bitsperpixel != 24 
		&& ptrTgaHeader->bitsperpixel != 32)
	{
		pfnPrintFn("%s is using a non-supported format. Only 24 bit and 32 bit true color formats are supported.\n", pstrFilename);
		return false;
	}

	Int16 tgaWidth = Common::ByteToInt16(ptrTgaHeader->width);
	Int16 tgaHeight = Common::ByteToInt16(ptrTgaHeader->height);
	Int16 tgaBpp = ptrTgaHeader->bitsperpixel/8;

	if(!Common::IsPowerOfTwo(tgaWidth) || !Common::IsPowerOfTwo(tgaHeight))
	{
		pfnPrintFn("%s is not a power of two texture.\n", pstrFilename);
		return false;
	}

	// Determine sizes
	Int32 nbPixels = tgaWidth*tgaHeight;
	Int32 inputSize = nbPixels*tgaBpp;
	Int32 outputSize = nbPixels*4;

	// Allocate the conversion data
	byte* pout = new byte[outputSize];

	// Load based on type
	const byte *pcur = pfile + sizeof(tga_header_t);
	if(ptrTgaHeader->datatypecode == TGA_DATATYPE_RGB)
	{
		// Set compression
		compression = TX_COMPRESSION_NONE;

		// Uncompressed TGA
		for(Int32 i = 0, j = 0; i < inputSize; i += tgaBpp, j += 4)
		{
			pout[j] = pcur[i+2];
			pout[j+1] = pcur[i+1];
			pout[j+2] = pcur[i];

			if(tgaBpp == 3)
				pout[j+3] = 255;
			else
				pout[j+3] = pcur[i+3];
		}
	}
	else if(ptrTgaHeader->datatypecode == TGA_DATATYPE_RLE_RGB)
	{
		// Set compression
		compression = TX_COMPRESSION_RLE;

		// RLE Compression
		Int32 i = 0;
		while(i < outputSize)
		{
			if((*pcur) & 0x80)
			{
				byte length = *pcur-127;
				pcur++;

				for(Int32 j = 0; j < length; j++, i += 4)
				{
					pout[i] = pcur[2];
					pout[i+1] = pcur[1];
					pout[i+2] = pcur[0];

					if(tgaBpp == 3)
						pout[i+3] = 255;
					else
						pout[i+3] = pcur[3];
				}
					
				pcur += tgaBpp;
			}
			else
			{
				byte length = *pcur+1;
				pcur++;

				for(Int32 j = 0; j < length; j++, i += 4, pcur += tgaBpp)
				{
					pout[i] = pcur[2];
					pout[i+1] = pcur[1];
					pout[i+2] = pcur[0];

					if(tgaBpp == 3)
						pout[i+3] = 255;
					else
						pout[i+3] = pcur[3];
				}
			}
		}
	}
	else
	{
		pfnPrintFn("TGA %s uses an unsupported datatype.\n", pstrFilename);
		delete[] pout;
		return false;
	}

	// Flip vertically and/or horizontally if needed
	if(!(ptrTgaHeader->imagedescriptor & 5) || (ptrTgaHeader->imagedescriptor & 4))
		R_FlipTexture(tgaWidth, tgaHeight, 4, (ptrTgaHeader->imagedescriptor & 4) ? true : false, (ptrTgaHeader->imagedescriptor & 5) ? false : true, pout);

	// Set the output data
	pdata = pout;
	width = tgaWidth;
	height = tgaHeight;
	bpp = tgaBpp;
	size = outputSize;

	return true;
}

//=============================================
// @brief Loads a TGA file and returns it's data
//
// @param pdata Image data to write
// @param bpp Bits per pixel
// @param width Width of the image
// @param height Height of the image
// @param pbuffer Destination buffer
//=============================================
void TGA_WriteData( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, byte* pbuffer )
{
	for(Uint32 i = 0; i < height; i++)
	{
		byte* pdst = pbuffer + sizeof(tga_header_t) + i*width*bpp;
		const byte* psrc = pdata + i*width*bpp;
	
		if(bpp == 4)
		{
			for(Uint32 j = 0; j < width*bpp; j += bpp)
			{
				pdst[j] = psrc[j+2];
				pdst[j+1] = psrc[j+1];
				pdst[j+2] = psrc[j];
				pdst[j+3] = psrc[j+3];
			}
		}
		else
		{
			for(Uint32 j = 0; j < width*bpp; j += bpp)
			{
				pdst[j] = psrc[j+2];
				pdst[j+1] = psrc[j+1];
				pdst[j+2] = psrc[j];
			}
		}
	}
}

//=============================================
// @brief Builds a TGA file and returns it's data
//
// @param pdata Image data to write
// @param bpp Bits per pixel
// @param width Width of the image
// @param height Height of the image
// @param poutbuffer Destination buffer pointer
// @param outsize Destination size variable
//=============================================
void TGA_BuildFile( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, byte*& poutbuffer, Uint32& outsize )
{
	outsize = width*height*bpp + sizeof(tga_header_t);
	poutbuffer = new byte[outsize];
	memset(poutbuffer, 0, sizeof(byte)*outsize);

	tga_header_t* pheader = reinterpret_cast<tga_header_t*>(poutbuffer);
	pheader->datatypecode = TGA_DATATYPE_RGB;
	pheader->bitsperpixel = bpp*8;
	pheader->width[0] = (width & 0xFF);
	pheader->width[1] = ((width >> 8) & 0xFF);
	pheader->height[0] = (height & 0xFF);
	pheader->height[1] = ((height >> 8) & 0xFF);
	pheader->imagedescriptor |= 8;

	// Store data to the buffer
	TGA_WriteData(pdata, bpp, width, height, poutbuffer);
}

//=============================================
// @brief Loads a TGA file and returns it's data
//
// @param pdata Image data to write
// @param bpp Bits per pixel
// @param width Width of the image
// @param height Height of the image
// @param pstrFilename destination file path
//=============================================
bool TGA_Write( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, const Char* pstrFilename, const file_interface_t& fileFuncs, pfnPrintf_t pfnPrintFn )
{
	// Buffer containing TGA file data
	byte* pbuffer = nullptr;
	// Size of data to be written
	Uint32 datasize = 0;

	// Build the file data
	TGA_BuildFile(pdata, bpp, width, height, pbuffer, datasize);

	// Write it to output
	bool result = fileFuncs.pfnWriteFile(pbuffer, sizeof(byte)*datasize, pstrFilename, false);

	// Delete the buffer
	delete[] pbuffer;

	if(!result)
		pfnPrintFn("%s - Failed to write %s.\n", __FUNCTION__, pstrFilename);

	return result;
}