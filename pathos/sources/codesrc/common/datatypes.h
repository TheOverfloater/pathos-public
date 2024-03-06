/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <assert.h>

#ifndef DATATYPES_H
#define DATATYPES_H
typedef	char				Char;
typedef	unsigned char		Uchar;
typedef	unsigned __int8		byte;
typedef	__int32				Int32;
typedef unsigned __int32	Uint32;
typedef __int16				Int16;
typedef	unsigned __int16	Uint16;
typedef __int64				Int64;
typedef unsigned __int64	Uint64;
typedef float				Float;
typedef double				Double;
typedef	Float				vec4_t[4];
typedef Uint32				string_t;
typedef Int32				entindex_t;

struct color32_t
{
	color32_t():
		r(0),
		g(0),
		b(0),
		a(255)
	{}
	color32_t( byte _r, byte _g, byte _b, byte _a )
	{
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
	byte& operator[]( Uint32 n )
	{
		assert(n <= 3);

		if(n == 0)
			return r;
		else if(n == 1)
			return g;
		else if(n == 2)
			return b;
		else
			return a;
	}

	byte r;
	byte g;
	byte b;
	byte a;
};

struct color24_t
{
	color24_t():
		r(0),
		g(0),
		b(0)
	{}
	color24_t( byte _r, byte _g, byte _b )
	{
		r = _r;
		g = _g;
		b = _b;
	}
	byte& operator[]( Uint32 n )
	{
		assert(n <= 2);

		if(n == 0)
			return r;
		else if(n == 1)
			return g;
		else
			return b;
	}

	byte r;
	byte g;
	byte b;
};
#endif