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
		&& ptrTgaHeader->datatypecode != TGA_DATATYPE_COLORMAPPED 
		&& ptrTgaHeader->datatypecode != TGA_DATATYPE_RLE_COLORMAPPED
		&& ptrTgaHeader->datatypecode != TGA_DATATYPE_GRAYSCALE
		&& ptrTgaHeader->datatypecode != TGA_DATATYPE_RLE_GRAYSCALE
		|| ptrTgaHeader->bitsperpixel != 24 
		&& ptrTgaHeader->bitsperpixel != 32
		&& ptrTgaHeader->bitsperpixel != 8)
	{
		pfnPrintFn("%s is using a non-supported format. Only 24 bit and 32 bit true color formats are supported.\n", pstrFilename);
		return false;
	}

	Int16 tgaWidth = Common::ByteToInt16(ptrTgaHeader->width);
	Int16 tgaHeight = Common::ByteToInt16(ptrTgaHeader->height);
	Int16 tgaBpp = ptrTgaHeader->bitsperpixel/8; 

	// Determine sizes
	Int32 nbPixels = tgaWidth*tgaHeight;
	Int32 inputSize = nbPixels*tgaBpp;
	Int32 outputSize = nbPixels*4;

	// Allocate the conversion data
	byte* pout = new byte[outputSize];

	// Load based on type
	const byte *pcur = pfile + sizeof(tga_header_t) + ptrTgaHeader->idlength;
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
				byte length = (*pcur)-127;
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
				byte length = (*pcur)+1;
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
	else if(ptrTgaHeader->datatypecode == TGA_DATATYPE_COLORMAPPED)
	{
		Uint32 colormapDepth = ptrTgaHeader->colourmapdepth;
		if(colormapDepth != 24 && colormapDepth != 32)
		{
			pfnPrintFn("TGA %s uses an unsupported color map depth of %d.\n", pstrFilename, colormapDepth);
			delete[] pout;
			return false;
		}

		Uint32 colormapBpp = colormapDepth / 8;
		Uint32 colormapLength = Common::ByteToInt16(ptrTgaHeader->colourmaplength);
		Uint32 colormapOrigin = Common::ByteToInt16(ptrTgaHeader->colourmaporigin);
		Uint32 colorMapSize = colormapLength * colormapBpp;

		const byte* pcolormap = pcur;
		const byte* ppixeldata = pcur + colorMapSize;

		// Set compression
		compression = TX_COMPRESSION_COLORMAP;

		// Uncompressed TGA
		for(Int32 i = 0, j = 0; i < inputSize; i++, j += 4)
		{
			const byte* pcolor = pcolormap + (colormapOrigin + ppixeldata[i]) * colormapBpp;

			pout[j] = pcolor[2];
			pout[j+1] = pcolor[1];
			pout[j+2] = pcolor[0];

			if(colormapBpp == 3)
				pout[j+3] = 255;
			else
				pout[j+3] = pcolor[3];
		}
	}
	else if(ptrTgaHeader->datatypecode == TGA_DATATYPE_RLE_COLORMAPPED)
	{
		Uint32 colormapDepth = ptrTgaHeader->colourmapdepth;
		if(colormapDepth != 24 && colormapDepth != 32)
		{
			pfnPrintFn("TGA %s uses an unsupported color map depth of %d.\n", pstrFilename, colormapDepth);
			delete[] pout;
			return false;
		}

		Uint32 colormapBpp = colormapDepth / 8;
		Uint32 colormapLength = Common::ByteToInt16(ptrTgaHeader->colourmaplength);
		Uint32 colormapOrigin = Common::ByteToInt16(ptrTgaHeader->colourmaporigin);
		Uint32 colorMapSize = colormapLength * colormapBpp;

		const byte* pcolormap = pcur;
		const byte* ppixeldata = pcur + colorMapSize;

		// Set compression
		compression = TX_COMPRESSION_RLE;

		// RLE Compression
		Int32 i = 0;
		while(i < outputSize)
		{
			if((*ppixeldata) & 0x80)
			{
				byte length = (*ppixeldata)-127;
				ppixeldata++;

				const byte* pcolor = pcolormap + (colormapOrigin + (*ppixeldata)) * colormapBpp;
				for(Int32 j = 0; j < length; j++, i += 4)
				{
					pout[i] = pcolor[2];
					pout[i+1] = pcolor[1];
					pout[i+2] = pcolor[0];

					if(colormapBpp == 3)
						pout[i+3] = 255;
					else
						pout[i+3] = pcolor[3];
				}
					
				ppixeldata++;
			}
			else
			{
				byte length = (*ppixeldata)+1;
				ppixeldata++;

				for(Int32 j = 0; j < length; j++, i += 4, ppixeldata++)
				{
					const byte* pcolor = pcolormap + (colormapOrigin + (*ppixeldata)) * colormapBpp;

					pout[i] = pcolor[2];
					pout[i+1] = pcolor[1];
					pout[i+2] = pcolor[0];

					if(colormapBpp == 3)
						pout[i+3] = 255;
					else
						pout[i+3] = pcolor[3];
				}
			}
		}
	}
	else if(ptrTgaHeader->datatypecode == TGA_DATATYPE_GRAYSCALE)
	{
		if(ptrTgaHeader->bitsperpixel != 8)
		{
			pfnPrintFn("TGA %s is a greyscale image with an unsupported pixel depth of %d.\n", pstrFilename, (ptrTgaHeader->bitsperpixel*8));
			delete[] pout;
			return false;
		}

		// Set compression
		compression = TX_COMPRESSION_GREYSCALE;

		// Uncompressed TGA
		for(Int32 i = 0, j = 0; i < inputSize; i++, j += 4)
		{
			byte greyColorValue = pcur[i];
			for(Uint32 k = 0; k < 3; k++)
				pout[j+k] = greyColorValue;
			
			pout[j+3] = 255;
		}
	}
	else if(ptrTgaHeader->datatypecode == TGA_DATATYPE_RLE_GRAYSCALE)
	{
		if(ptrTgaHeader->bitsperpixel != 8)
		{
			pfnPrintFn("TGA %s is a greyscale image with an unsupported pixel depth of %d.\n", pstrFilename, (ptrTgaHeader->bitsperpixel*8));
			delete[] pout;
			return false;
		}

		// Set compression
		compression = TX_COMPRESSION_GREYSCALE_RLE;

		// RLE Compression
		Int32 i = 0;
		while(i < outputSize)
		{
			if((*pcur) & 0x80)
			{
				byte length = (*pcur)-127;
				pcur++;

				byte greyColorValue = (*pcur);
				for(Int32 j = 0; j < length; j++, i += 4)
				{
					for(Uint32 k = 0; k < 3; k++)
						pout[i+k] = greyColorValue;

					pout[i+3] = 255;
				}
					
				pcur++;
			}
			else
			{
				byte length = (*pcur)+1;
				pcur++;

				for(Int32 j = 0; j < length; j++, i += 4, pcur++)
				{
					byte greyColorValue = (*pcur);
					for(Uint32 k = 0; k < 3; k++)
						pout[i+k] = greyColorValue;

					pout[i+3] = 255;;
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
		Common::FlipTexture(tgaWidth, tgaHeight, 4, (ptrTgaHeader->imagedescriptor & 4) ? true : false, (ptrTgaHeader->imagedescriptor & 5) ? false : true, pout);

	// Set the output data
	pdata = pout;
	width = tgaWidth;
	height = tgaHeight;
	bpp = tgaBpp;
	size = outputSize;

	return true;
}

//=============================================
// @brief Stores a repeating pixel and it's repeat
// count into the data buffer
//
// @param rleBuffer Destination buffer object
// @param repeatingPixel The pixel that we're storing
// @param nbPixels Number of repeats
// @param bpp Bytes per pixel
//=============================================
inline void TGA_RLE_StoreRepeatingPixels( CBuffer& rleBuffer, color32_t repeatingPixel, Uint32 nbPixels, Uint32 bpp )
{
	// Mark as repeating(0x80/128), and store the repeat number
	byte dataMark = 0x80 + (nbPixels - 1);
	rleBuffer.append(&dataMark, sizeof(byte));

	// Store the pixel that repeats
	color32_t storeColor;
	storeColor.r = repeatingPixel.r;
	storeColor.g = repeatingPixel.g;
	storeColor.b = repeatingPixel.b;

	if(bpp == 4)
		storeColor.a = repeatingPixel.a;

	// Store into the buffer
	rleBuffer.append(&storeColor, sizeof(byte)*bpp);
}

//=============================================
// @brief Stores a block of unique, non-repeating
// pixels into the destination buffer
//
// @param rleBuffer Destination buffer object
// @param pixelsArray Array of unique pixels
// @param nbPixels Number of non-repeating pixels
// @param bpp Bytes per pixel
//=============================================
inline void TGA_RLE_StoreUniquePixels( CBuffer& rleBuffer, CArray<color32_t>& pixelsArray, Uint32 nbPixels, Uint32 bpp )
{
	// Mark number of unique pixels
	byte dataMark = (nbPixels - 1);
	rleBuffer.append(&dataMark, sizeof(byte));

	// Store each unique pixel in the buffer
	color32_t storeColor;
	for(Uint32 j = 0; j < nbPixels; j++)
	{
		storeColor.r = pixelsArray[j].r;
		storeColor.g = pixelsArray[j].g;
		storeColor.b = pixelsArray[j].b;

		if(bpp == 4)
			storeColor.a = pixelsArray[j].a;

		// Store into the buffer
		rleBuffer.append(&storeColor, sizeof(byte)*bpp);
	}
}

//=============================================
// @brief Compress RGB(A) data as RLE information
//
// @param pdata Image data to write
// @param bpp Bits per pixel
// @param width Width of the image
// @param height Height of the image
// @param rleBuffer Destination buffer
//=============================================
void TGA_RLE_CompressData( const byte* pdata, Uint32 width, Uint32 height, Uint32 bpp, CBuffer& rleBuffer )
{
	CArray<color32_t> pixelsArray(128);
	Uint32 nbPixels = 0;

	color32_t lastPixel;
	Int32 nbPixelMatches = -1; // Mark as first pixel

	// Size of data in pixel count
	Uint32 dataSizePixels = width*height;
	const byte* pColorData = reinterpret_cast<const byte*>(pdata);
	for(Uint32 i = 0; i < dataSizePixels; i++, pColorData += bpp)
	{
		// Get the current pixel
		color32_t currentPixel;
		currentPixel.r = pColorData[0];
		currentPixel.g = pColorData[1];
		currentPixel.b = pColorData[2];
		if(bpp == 4)
			currentPixel.a = pColorData[3];

		if(i > 0)
		{
			if(!memcmp(&currentPixel, &lastPixel, sizeof(byte)*bpp) && nbPixelMatches != -1)
			{
				// Store unique pixels if we have any
				if(nbPixels > 0)
				{
					// Ingore last element, as it's our repeating pixel
					if(nbPixels > 1)
					{
						Uint32 nbStorePixels = nbPixels-1;
						TGA_RLE_StoreUniquePixels(rleBuffer, pixelsArray, nbStorePixels, bpp);
						
					}

					nbPixels = 0;
					nbPixelMatches = 1;
				}

				nbPixelMatches++;

				// Check if we reached the 128 limit
				if(nbPixelMatches == 128)
				{
					TGA_RLE_StoreRepeatingPixels(rleBuffer, lastPixel, nbPixelMatches, bpp);
					nbPixelMatches = 0;
				}
			}
			else
			{
				// Store matching pixels if we have any
				if(nbPixelMatches == -1)
				{
					nbPixelMatches = 0;

					pixelsArray[nbPixels] = lastPixel;
					nbPixels++;
				}
				else if(nbPixelMatches > 0)
				{
					TGA_RLE_StoreRepeatingPixels(rleBuffer, lastPixel, nbPixelMatches, bpp);
					nbPixelMatches = 0;
				}

				// Add current pixel
				pixelsArray[nbPixels] = currentPixel;
				nbPixels++;

				// Check for limit
				if(nbPixels == 128)
				{
					TGA_RLE_StoreUniquePixels(rleBuffer, pixelsArray, nbPixels, bpp);
					nbPixels = 0;
				}
			}
		}

		// Mark last pixel we stored
		lastPixel = currentPixel;
	}

	// Store trailing part
	if(nbPixels > 0)
		TGA_RLE_StoreUniquePixels(rleBuffer, pixelsArray, nbPixels, bpp);
	else if(nbPixelMatches > 0)
		TGA_RLE_StoreRepeatingPixels(rleBuffer, lastPixel, nbPixelMatches, bpp);
}

//=============================================
// @brief Builds pixel data for exporting as TGA
//
// @param pdata Image data to write
// @param bpp Bits per pixel
// @param width Width of the image
// @param height Height of the image
// @param fileBuffer Destination buffer
//=============================================
void TGA_BuildImageData( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, CBuffer& fileBuffer, tga_header_t*& pheader, Uint32& compressionPercentage, bool flipVertical )
{
	// Get ptr to destination
	byte* pfinalbuffer = new byte[width*height*bpp];
	for(Uint32 i = 0; i < height; i++)
	{
		byte* pdst;
		if(flipVertical)
			pdst = pfinalbuffer + (height-i-1)*width*bpp;
		else
			pdst = pfinalbuffer + i*width*bpp;

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

	// Try compressing with RLE
	CBuffer rleBuffer(width*height*bpp);
	TGA_RLE_CompressData(pfinalbuffer, width, height, bpp, rleBuffer);

	// If uncompressed size is smaller than RLE compressed size, then
	// store the uncompressed image. Otherwise, store the RLE compressed
	// data to save on disk space
	Uint32 uncompressedSize = width*height*bpp;
	Uint32 compressedSize = rleBuffer.getdatasize();
	if(uncompressedSize > compressedSize)
	{
		// Mark as compressed RGB(A)
		pheader->datatypecode = TGA_DATATYPE_RLE_RGB;
		// Directly copy the buffer contents
		fileBuffer.append(rleBuffer.getbufferdata(), compressedSize);

		// Calculate compression percentage
		compressionPercentage = (static_cast<Float>(compressedSize) / static_cast<Float>(uncompressedSize)) * 100;
	}
	else
	{
		// Mark as uncompressed TGA file
		pheader->datatypecode = TGA_DATATYPE_RGB;
		compressionPercentage = 100; // No compression

		// Just append to the buffer
		fileBuffer.append(pfinalbuffer, uncompressedSize*sizeof(byte));
	}

	delete[] pfinalbuffer;
}

//=============================================
// @brief Builds a TGA file and returns it's data
//
// @param pdata Image data to write
// @param bpp Bits per pixel
// @param width Width of the image
// @param height Height of the image
// @param fileBuffer Destination buffer
//=============================================
void TGA_BuildFile( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, CBuffer& fileBuffer, Uint32* ptrCompressionRatio, bool flipVertical )
{
	tga_header_t* pheader = reinterpret_cast<tga_header_t*>(fileBuffer.getbufferdata());
	fileBuffer.append(nullptr, sizeof(tga_header_t));
	fileBuffer.addpointer(reinterpret_cast<void**>(&pheader));

	pheader->bitsperpixel = bpp*8;
	pheader->width[0] = (width & 0xFF);
	pheader->width[1] = ((width >> 8) & 0xFF);
	pheader->height[0] = (height & 0xFF);
	pheader->height[1] = ((height >> 8) & 0xFF);

	// Store data to the buffer
	Uint32 compressionRatio = 0;
	TGA_BuildImageData(pdata, bpp, width, height, fileBuffer, pheader, compressionRatio, flipVertical);
	if(ptrCompressionRatio)
		(*ptrCompressionRatio) = compressionRatio;

	fileBuffer.removepointer(reinterpret_cast<void**>(&pheader));
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
bool TGA_Write( const byte* pdata, Uint32 bpp, Uint32 width, Uint32 height, const Char* pstrFilename, const file_interface_t& fileFuncs, pfnPrintf_t pfnPrintFn, Uint32* ptrCompressionRatio, Uint32* ptrBytesWritten, bool flipVertical )
{
	// Buffer containing TGA file data
	Uint32 bufferAllocSize = width*height*bpp + sizeof(tga_header_t);
	CBuffer tgaDataBuffer(bufferAllocSize);

	// Build the file data
	TGA_BuildFile(pdata, bpp, width, height, tgaDataBuffer, ptrCompressionRatio, flipVertical);

	// Write it to output
	Uint32 finalBufferSize = tgaDataBuffer.getdatasize();
	const byte* pbufferdata = reinterpret_cast<const byte*>(tgaDataBuffer.getbufferdata());
	bool result = fileFuncs.pfnWriteFile(pbufferdata, finalBufferSize, pstrFilename, false);
	if(!result)
		pfnPrintFn("%s - Failed to write %s.\n", __FUNCTION__, pstrFilename);
	else if(ptrBytesWritten)
		(*ptrBytesWritten) = finalBufferSize;

	return result;
}