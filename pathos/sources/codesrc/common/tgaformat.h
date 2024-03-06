/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#ifndef TGAFORMAT_H
#define TGAFORMAT_H

enum tga_datatype_t
{
	TGA_DATATYPE_EMPTY = 0,
	TGA_DATATYPE_COLORMAPPED,
	TGA_DATATYPE_RGB,
	TGA_DATATYPE_GRAYSCALE,
	TGA_DATATYPE_RLE_COLORMAPPED = 9,
	TGA_DATATYPE_RLE_RGB,
	TGA_DATATYPE_RLE_GRAYSCALE
};

struct tga_header_t
{
	byte	idlength;
	byte	colourmaptype;
	byte	datatypecode;
	byte	colourmaporigin[2]; //how come you have short ints there?
	byte	colourmaplength[2];
	byte	colourmapdepth;
	byte	x_origin[2];
	byte	y_origin[2];
	byte	width[2];
	byte	height[2];
	byte	bitsperpixel;
	byte	imagedescriptor;
};
#endif //TGAFORMAT_H