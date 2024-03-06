//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxTga.cpp
// implementation: all
// last modified:  Apr 15 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxTga.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

mxImage *
mxTgaRead (const char *filename)
{
	FILE *file;
	file = fopen (filename, "rb");
	if (!file)
		return 0;

	byte identFieldLength;
	byte colorMapType;
	byte imageTypeCode;
	fread (&identFieldLength, sizeof (byte), 1, file);
	fread (&colorMapType, sizeof (byte), 1, file);
	fread (&imageTypeCode, sizeof (byte), 1, file);

	fseek (file, 12, SEEK_SET);

	word width, height;
	byte pixelSize;
	fread (&width, sizeof (word), 1, file);
	fread (&height, sizeof (word), 1, file);
	fread (&pixelSize, sizeof (byte), 1, file);

	// only 24-bit RGB uncompressed
	if (colorMapType != 0 ||
		imageTypeCode != 2 && imageTypeCode != 10 ||
		pixelSize != 24)
	{
		fclose (file);
		return 0;
	}

	fseek (file, 18 + identFieldLength, SEEK_SET);

	int imageSize = width*height*(pixelSize/8);
	byte *data = (byte *)malloc(imageSize);

	if(imageTypeCode == 2)
	{
		for (int y = 0; y < height; y++)
		{
			byte *scanline = (byte *) &data[y * width * 3];
			for (int x = 0; x < width; x++)
			{
				scanline[x * 3 + 2] = (byte) fgetc (file);
				scanline[x * 3 + 1] = (byte) fgetc (file);
				scanline[x * 3 + 0] = (byte) fgetc (file);
			}
		}
	}
	else
	{
		// RLE Compression
		int i = 0;
		while(i < imageSize)
		{
			byte pdata = (byte)fgetc(file);
			if(pdata & 0x80)
			{
				byte length = pdata-127;

				byte color[3];
				for(int j = 0; j < 3; j++)
					color[j] = (byte) fgetc (file);

				for(int j = 0; j < length; j++, i+= 3)
				{
					data[i+2] = color[0];
					data[i+1] = color[1];
					data[i] = color[2];
				}
			}
			else
			{
				byte length = pdata+1;

				for(int j = 0; j < length; j++, i+= 3)
				{
					data[i+2] = (byte) fgetc (file);
					data[i+1] = (byte) fgetc (file);
					data[i] = (byte) fgetc (file);
				}
			}
		}
	}
	fclose (file);

	mxImage *image = new mxImage ();
	if (!image->create (width, height, 24))
	{
		free(data);
		delete image;
		return 0;
	}

	// Flip vertically
	byte *pdest = (byte *) image->data;
	for(int i = 0; i < height; i++)
	{
		byte *dst = pdest + i*width*3;
		byte *src = data + (height-i-1)*width*3;
		memcpy(dst, src, sizeof(byte)*width*3);
	}

	free(data);

	return image;
}



bool
mxTgaWrite (const char *filename, mxImage *image)
{
	if (!image)
		return false;

	if (image->bpp != 24)
		return false;

	FILE *file = fopen (filename, "wb");
	if (!file)
		return false;

	//
	// write header
	//
	fputc (0, file); // identFieldLength
	fputc (0, file); // colorMapType == 0, no color map
	fputc (2, file); // imageTypeCode == 2, uncompressed RGB

	word w = 0;
	fwrite (&w, sizeof (word), 1, file); // colorMapOrigin
	fwrite (&w, sizeof (word), 1, file); // colorMapLength
	fputc (0, file); // colorMapEntrySize

	fwrite (&w, sizeof (word), 1, file); // imageOriginX
	fwrite (&w, sizeof (word), 1, file); // imageOriginY

	w = (word) image->width;
	fwrite (&w, sizeof (word), 1, file); // imageWidth

	w = (word) image->height;
	fwrite (&w, sizeof (word), 1, file); // imageHeight

	fputc (24, file); // imagePixelSize
	fputc (0, file); // imageDescriptorByte

	// write no ident field

	// write no color map

	// write imagedata

	byte *data = (byte *) image->data;
	for (int y = 0; y < image->height; y++)
	{
		byte *scanline = (byte *) &data[(image->height - y - 1) * image->width * 3];
		for (int x = 0; x < image->width; x++)
		{
			fputc ((byte) scanline[x * 3 + 2], file);
			fputc ((byte) scanline[x * 3 + 1], file);
			fputc ((byte) scanline[x * 3 + 0], file);
		}
	}

	fclose (file);

	return true;
}