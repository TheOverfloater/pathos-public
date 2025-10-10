/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef COMMON_INLINE_H
#define COMMON_INLINE_H

#include "cstring.h"

#pragma warning(disable: 4018)

//=============================================
// @brief Returns the length of a string
//
// @param pstr Pointer to string
// @return Length of the string
//=============================================
inline Uint32 qstrlen( const Char* pstr )
{
	if(!pstr || pstr[0] == '\0')
		return 0;

	Uint32 length = 0;
	while(*pstr)
	{
		length++;
		pstr++;
	}

	return length;
}

//=============================================
// @brief Copies a string to another container
//
// @param pdest Pointer to destination container
// @param psrc Pointer to source string
// @return Pointer to destination string
//=============================================
inline Char* qstrcpy( Char* pdest, const Char* psrc )
{
	assert(psrc != nullptr);
	assert(pdest != nullptr);

	const Char* ppsrc = psrc;
	Char* ppdest = pdest;
	while(*ppsrc)
	{
		*ppdest = *ppsrc;
		ppdest++; ppsrc++;
	}
	
	// null terminate
	*ppdest = '\0';

	return pdest;
}

//=============================================
// @brief Copies a number of characters from a string to another container
//
// @param pdest Pointer to destination container
// @param psrc Pointer to source string
// @param size Length/size to copy
// @return Pointer to destination string
//=============================================
inline Char* qstrncpy( Char* pdest, const Char* psrc, Uint32 size )
{
	assert(psrc != nullptr);
	assert(pdest != nullptr);

	const Char* ppsrc = psrc;
	Char* ppdest = pdest;
	while(*ppsrc && (ppsrc-psrc) < size)
	{
		*ppdest = *ppsrc;
		ppdest++; ppsrc++;
	}

	// null terminate
	*ppdest = '\0';

	return pdest;
}

//=============================================
// @brief Copies a number of characters from a string to another container,
// but with the max size of the destination container specified, to avoid
// buffer overwrites
//
// @param pdest Pointer to destination container
// @param psrc Pointer to source string
// @param m Max characters
// @return Pointer to destination string
//=============================================
inline Char* qstrcpy_s( Char* pdest, const Char *psrc, Uint32 m )
{
	Uint32 length = qstrlen(psrc);
	if(length > (m-1))
		length = (m-1);

	qstrncpy(pdest, psrc, length);
	return pdest;
}

//=============================================
// @brief Compares two strings, both of const Char pointer type
//
// @param pstr1 Pointer to first string to compare
// @param pstr2 Pointer to second string to compare
// @return Difference between point of divergence
//=============================================
inline Int32 qstrcmp( const Char* pstr1, const Char* pstr2 )
{
	assert(pstr1 != nullptr);
	assert(pstr2 != nullptr);

	const Char* ppstr1 = pstr1;
	const Char* ppstr2 = pstr2;
	while(true)
	{
		if(*ppstr1 != *ppstr2)
			return (*ppstr1 - *ppstr2);

		if(!*ppstr1 || !*ppstr2)
			break;

		ppstr1++; 
		ppstr2++;
	}
	
	return 0;
}

//=============================================
// @brief Compares two strings, both of CString type
//
// @param pstr1 Reference to CString object
// @param pstr2 Reference to CString object
// @return Difference between point of divergence
//=============================================
inline Int32 qstrcmp( const CString& str1, const CString& str2 )
{
	const Char* ppstr1 = str1.c_str();
	const Char* ppstr2 = str2.c_str();

	if(!ppstr1 || !ppstr2)
		return true;
	else
		return qstrcmp(ppstr1, ppstr2);
}

//=============================================
// @brief Compares two strings, first type of const Char ptr,
// second of CString type
//
// @param pstr1 Reference to CString object
// @param pstr2 Reference to CString object
// @return Difference between point of divergence
//=============================================
inline Int32 qstrcmp( const Char* pstr1, const CString& str2 )
{
	assert(pstr1 != nullptr);

	const Char* ppstr1 = pstr1;
	const Char* ppstr2 = str2.c_str();

	if(!ppstr1 || !ppstr2)
		return true;
	else
		return qstrcmp(ppstr1, ppstr2);
}

//=============================================
// @brief Compares two strings, first of CString type,
// second of const Char ptr
//
// @param pstr1 Reference to CString object
// @param pstr2 Reference to CString object
// @return Difference between point of divergence
//=============================================
inline Int32 qstrcmp( const CString& str1, const Char* pstr2 )
{
	assert(pstr2 != nullptr);

	const Char* ppstr1 = str1.c_str();
	const Char* ppstr2 = pstr2;

	if(!ppstr1 || !ppstr2)
		return true;
	else
		return qstrcmp(ppstr1, ppstr2);
}

//=============================================
// @brief Compares two strings in case insensitive mode
//
// @param pstr1 Pointer to first string to compare
// @param pstr2 Pointer to second string to compare
// @return Difference between point of divergence
//=============================================
inline Int32 qstrcicmp( const Char* pstr1, const Char* pstr2 )
{
	assert(pstr1 != nullptr);
	assert(pstr2 != nullptr);

	const Char* ppstr1 = pstr1;
	const Char* ppstr2 = pstr2;
	while(true)
	{
		const Char c1 = SDL_tolower(*ppstr1);
		const Char c2 = SDL_tolower(*ppstr2);

		if(c1 != c2)
			return (c1 - c2);

		if(!c1 || !c2)
			break;

		ppstr1++; 
		ppstr2++;
	}
	
	return 0;
}

//=============================================
// @brief Compares two strings in case insensitive mode
//
// @param pstr1 Reference to CString object
// @param pstr2 Reference to CString object
// @return Difference between point of divergence
//=============================================
inline Int32 qstrcicmp( const CString& str1, const CString& str2 )
{
	const Char* ppstr1 = str1.c_str();
	const Char* ppstr2 = str2.c_str();

	if(!ppstr1 || !ppstr2)
		return true;
	else
		return qstrcicmp(ppstr1, ppstr2);
}

//=============================================
// @brief Compares two strings in case insensitive mode
//
// @param pstr1 Reference to CString object
// @param pstr2 Reference to CString object
// @return Difference between point of divergence
//=============================================
inline Int32 qstrcicmp( const Char* pstr1, const CString& str2 )
{
	assert(pstr1 != nullptr);

	const Char* ppstr1 = pstr1;
	const Char* ppstr2 = str2.c_str();

	if(!ppstr1 || !ppstr2)
		return true;
	else
		return qstrcicmp(ppstr1, ppstr2);
}

//=============================================
// @brief Compares two strings in case insensitive mode
//
// @param pstr1 Reference to CString object
// @param pstr2 Reference to CString object
// @return Difference between point of divergence
//=============================================
inline Int32 qstrcicmp( const CString& str1, const Char* pstr2 )
{
	assert(pstr2 != nullptr);

	const Char* ppstr1 = str1.c_str();
	const Char* ppstr2 = pstr2;

	if(!ppstr1 || !ppstr2)
		return true;
	else
		return qstrcicmp(ppstr1, ppstr2);
}


//=============================================
// @brief Compares two strings until n number of characters
//
// @param pstr1 Pointer to first string to compare
// @param pstr2 Pointer to second string to compare
// @param n Number of characters to compare
// @return Difference between point of divergence
//=============================================
inline Int32 qstrncmp( const Char* pstr1, const Char* pstr2, Uint32 n )
{
	assert(pstr1 != nullptr);
	assert(pstr2 != nullptr);

	const Char* ppstr1 = pstr1;
	const Char* ppstr2 = pstr2;
	while(ppstr1 - pstr1 < n)
	{
		if(*ppstr1 != *ppstr2)
			return (*ppstr1 - *ppstr2);

		if(!*ppstr1 || !*ppstr2)
			break;

		ppstr1++; 
		ppstr2++;
	}
	
	return false;
}

//=============================================
// @brief Compares two strings until n number of characters
//
// @param pstr1 Reference to CString object
// @param pstr2 Reference to CString object
// @param n Number of characters to compare
// @return Difference between point of divergence
//=============================================
inline Int32 qstrncmp( const CString& str1, const CString& str2, Uint32 n )
{
	const Char* ppstr1 = str1.c_str();
	const Char* ppstr2 = str2.c_str();

	if(!ppstr1 || !ppstr2)
		return true;
	else
		return qstrncmp(ppstr1, ppstr2, n);
}

//=============================================
// @brief Compares two strings until n number of characters
//
// @param pstr1 Reference to CString object
// @param pstr2 Reference to CString object
// @param n Number of characters to compare
// @return Difference between point of divergence
//=============================================
inline Int32 qstrncmp( const Char* pstr1, const CString& str2, Uint32 n )
{
	assert(pstr1 != nullptr);

	const Char* ppstr1 = pstr1;
	const Char* ppstr2 = str2.c_str();

	if(!ppstr1 || !ppstr2)
		return true;
	else
		return qstrncmp(ppstr1, ppstr2, n);
}

//=============================================
// @brief Compares two strings until n number of characters
//
// @param pstr1 Reference to CString object
// @param pstr2 Reference to CString object
// @param n Number of characters to compare
// @return Difference between point of divergence
//=============================================
inline Int32 qstrncmp( const CString& str1, const Char* pstr2, Uint32 n )
{
	assert(pstr2 != nullptr);

	const Char* ppstr1 = str1.c_str();
	const Char* ppstr2 = pstr2;

	if(!ppstr1 || !ppstr2)
		return true;
	else
		return qstrncmp(ppstr1, ppstr2, n);
}

//=============================================
// @brief Compares two strings until n number of characters in case insensitive mode
//
// @param pstr1 Pointer to first string to compare
// @param pstr2 Pointer to second string to compare
// @param n Number of characters to compare
// @return Difference between point of divergence
//=============================================
inline Int32 qstrncimp( const Char* pstr1, const Char* pstr2, Uint32 n )
{
	assert(pstr1 != nullptr);
	assert(pstr2 != nullptr);

	const Char* ppstr1 = pstr1;
	const Char* ppstr2 = pstr2;
	while(ppstr1 - pstr1 < n)
	{
		const Char c1 = SDL_tolower(*ppstr1);
		const Char c2 = SDL_tolower(*ppstr2);

		if(c1 != c2)
			return (c1 - c2);

		if(!c1 || !c2)
			break;

		ppstr1++; 
		ppstr2++;
	}
	
	return false;
}

//=============================================
// @brief Inserts a string into another at an offset
//
// @param psrc Source string pointer
// @param pdest Destination to insert source into
// @param offset Offset into the destination string
//=============================================
inline void qstrins( const Char* psrc, Char *pdest, int offset )
{
	assert(psrc != nullptr);
	assert(pdest != nullptr);

	Uint32 inslength = qstrlen(psrc);
	Uint32 destlength = qstrlen(pdest);

	Char* pchsrc = pdest + destlength + offset;
	Char* pchdst = pchsrc + inslength;
	while(pchsrc >= pdest)
	{
		*pchdst = *pchsrc;
		pchsrc--; 
		pchdst--;
	}

	qstrncpy(pdest, psrc, inslength);
}

//=============================================
// @brief Seeks out a substring in a string
//
// @param pstr String to seek substring in
// @param psubstr Substring to seek
// @return Pointer to where the substring was found
//=============================================
inline const Char* qstrstr( const Char* pstr, const Char *psubstr )
{
	assert(pstr != nullptr);
	assert(psubstr != nullptr);

	const Char* ppstr = pstr;
	while(*ppstr)
	{
		if(*ppstr == psubstr[0])
		{
			const Char* pcpstr = ppstr;
			const Char* ppsubstr = psubstr;
			while(*pcpstr && *ppsubstr 
				&& *pcpstr == *ppsubstr)
			{
				ppsubstr++;
				pcpstr++;
			}

			if(*ppsubstr == '\0')
				return ppstr;
			else if(*pcpstr == '\0')
				return nullptr;

			ppstr = pcpstr;
		}

		ppstr++;
	}

	return nullptr;
}

//====================================
// Takes a floating point input and returns
// a sign value, 1.0 for positive, -1.0 for
// negative value, and 0.0 if the value was
// zero 
//
// @param a Input float value
// @return Sign value
//====================================
inline Float sgn( Float a )
{
    if (a > 0.0F) 
		return (1.0F);
    if (a < 0.0F) 
		return (-1.0F);

    return (0.0F);
}

namespace Common
{
	//=============================================
	// @brief Converts bytes to short
	//
	// @param pdata Pointer to data in bytes
	// @return Signed short int value
	//=============================================
	inline Int16 ByteToInt16( const byte *pdata )
	{
		Int16 value;
		memcpy(&value, pdata, sizeof(Int16));
		return value;
	}

	//=============================================
	// @brief Converts bytes to unsigned short
	//
	// @param pdata Pointer to data in bytes
	// @return Unsigned short int value
	//=============================================
	inline Uint16 ByteToUint16( const byte *pdata )
	{
		Uint16 value;
		memcpy(&value, pdata, sizeof(Uint16));
		return value;
	}

	//=============================================
	// @brief Converts bytes to int32
	//
	// @param pdata Pointer to data in bytes
	// @return Signed Int32 value
	//=============================================
	inline Int32 ByteToInt32( const byte *pdata )
	{
		Int32 value;
		memcpy(&value, pdata, sizeof(Int32));
		return value;
	}

	//=============================================
	// @brief Converts bytes to unsigned int32
	//
	// @param pdata Pointer to data in bytes
	// @return Unsigned Int32 value
	//=============================================
	inline Uint32 ByteToUint32( const byte *pdata )
	{
		Uint32 value;
		memcpy(&value, pdata, sizeof(Uint32));
		return value;
	}

	//=============================================
	// @brief Converts bytes to int64
	//
	// @param pdata Pointer to data in bytes
	// @return Signed Int64 value
	//=============================================
	inline Int64 ByteToInt64( const byte *pdata )
	{
		Int64 value;
		memcpy(&value, pdata, sizeof(Int64));
		return value;
	}

	//=============================================
	// @brief Converts bytes to unsigned int64
	//
	// @param pdata Pointer to data in bytes
	// @return Unsigned Int64 value
	//=============================================
	inline Uint64 ByteToUint64( const byte *pdata )
	{
		Uint64 value;
		memcpy(&value, pdata, sizeof(Uint64));
		return value;
	}

	//=============================================
	// @brief Converts bytes to Float
	//
	// @param pdata Pointer to data in bytes
	// @return Float value
	//=============================================
	inline Float ByteToFloat( const byte *pdata )
	{
		Float value;
		memcpy(&value, pdata, sizeof(Float));
		return value;
	}

	//=============================================
	// @brief Converts bytes to int
	//
	// @param pdata Pointer to data in bytes
	// @return Double float value
	//=============================================
	inline Double ByteToDouble( const byte *pdata )
	{
		Double value;
		memcpy(&value, pdata, sizeof(Double));
		return value;
	}

	//=============================================
	// @brief Fast reciprocal square root
	//
	// @param value Value
	//=============================================
	inline Float qrsqrt( Float value )
	{
		// Thanks to Id Software and Carmack
		// for this solution
		Float x = value * 0.5f;
		Float y = value;

		long i = *reinterpret_cast<long*>(&y);
		i = 0x5f3759df - (i>>1);
	
		y = *reinterpret_cast<Float*>(&i);
		y = y*(1.5f-(x * y * y));

		return y;
	}

	//=============================================
	// @brief Tells if a value is NAN
	//
	// @param value Value to check
	// @return TRUE if NAN, FALSE otherwise
	//=============================================
	inline bool IsNAN( Float value )
	{
		return (*reinterpret_cast<Int32*>(&(value)) & NANMASK) == NANMASK ? true : false;
	}

	//=============================================
	// @brief Calculates a smooth curve for interpolation
	//
	// @param value Value to calculate for
	// @param scale Maximum value
	// @return Smooth curve value
	//=============================================
	inline Float SplineFraction( Float value, Float scale )
	{
		// This mathematical solution comes from the HLSDK,
		// so credit goes to Valve for the code I referenced
		Float _value = scale * value;
		Float valueSquared = _value * _value;

		// Nice little ease-in, ease-out spline-like curve
		return 3 * valueSquared - 2 * valueSquared * _value;
	}

	//====================================
	// @brief Takes an input DWORD pointer and modifies it's value to
	// ensure that it's is divisible by 2. Used for parsing WAVs
	//
	// @param nInput Pointer to the input value
	//====================================
	inline void ScaleByte( DWORD *nInput )
	{
		if(*nInput % 2 != 0)
		{
			*nInput += 1;
		}
	}

	//====================================
	// @brief Parses RGB24 byte values and converts them to
	// the 0-1 floating point range into the output parameter.
	//
	// @param pout Pointer to output float[3] array.
	// @param plightmap Pointer to RGB24 values.
	//====================================
	inline void ParseColor (Float* pout, const color24_t *plightmap)
	{
		pout[0] = static_cast<Float>(plightmap->r / 255.0f);
		pout[1] = static_cast<Float>(plightmap->g / 255.0f);
		pout[2] = static_cast<Float>(plightmap->b / 255.0f);
	}

	//====================================
	// @brief Parses RGB24 byte values and converts them into light
	// vectors used by the light vectors map for BSP rendering. The
	// values are outputted to the output float[3] array.
	//
	// @param pout Pointer to output float[3] array.
	// @param plightmap Pointer to RGB24 values.
	//====================================
	inline void ParseVectorColor (Float* pout, const color24_t *plightmap)
	{
		color24_t lightcolor;
		lightcolor.r = plightmap->r;
		lightcolor.b = plightmap->b;

		Int16 greenColor = plightmap->g;
		greenColor = (greenColor-128);
		lightcolor.g = (-1.0 * greenColor)+128;

		pout[0] = static_cast<Float>(lightcolor.r / 255.0f);
		pout[1] = static_cast<Float>(lightcolor.g / 255.0f);
		pout[2] = static_cast<Float>(lightcolor.b / 255.0f);
	}

	//====================================
	// @brief Remaps a value into another range of min/max values.
	//
	// @param value Original input value.
	// @param a Original minimum range.
	// @param b Original maximum range.
	// @param c New minimum range.
	// @param d New maximum range.
	// @return Resulting remapped value.
	//====================================
	inline Float RemapValue( Float value, Float a, Float b, Float c, Float d )
	{
		return c + (d-c)*(value-a)/(b-a);
	}

	//====================================
	// @brief Tells if a value given fits within a minimum/maximum range of values.
	//
	// @param comparisonValue Input value to check.
	// @param rangeMin Minimum of value range.
	// @param rangeMax Maximum value of range.
	// @param rangeShift Value to shift the range by.
	// @return TRUE if the value fits within the range, FALSE if not.
	//====================================
	inline bool ValueInRange( Float comparisonValue, Float rangeMin, Float rangeMax, Float rangeShift )
	{
		Float valueMin = rangeMin + rangeShift;
		Float valueMax = rangeMax + rangeShift;

		return (comparisonValue >= valueMin && comparisonValue <= valueMax) ? true : false;
	}
}

#endif // COMMON_INLINE_H