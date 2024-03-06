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

	return pdest;
}

//=============================================
// @brief Compares two strings
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
// @brief Compares two strings
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
// @brief Compares two strings
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
// @brief Compares two strings
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
		Char c1 = SDL_tolower(*ppstr1);
		Char c2 = SDL_tolower(*ppstr2);

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
// @param psrc Source string pointer
// @param pdest Destination to insert source into
// @param offset
// @return Position in string where substring was found
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
// 
//
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
	//=============================================
	inline Uint16 ByteToUint16( const byte *pdata )
	{
		Uint16 value;
		memcpy(&value, pdata, sizeof(Uint16));
		return value;
	}

	//=============================================
	// @brief Converts bytes to int
	//
	// @param pdata Pointer to data in bytes
	//=============================================
	inline Int32 ByteToInt32( const byte *pdata )
	{
		Int32 value;
		memcpy(&value, pdata, sizeof(Int32));
		return value;
	}

	//=============================================
	// @brief Converts bytes to unsigned int
	//
	// @param pdata Pointer to data in bytes
	//=============================================
	inline Uint32 ByteToUint32( const byte *pdata )
	{
		Uint32 value;
		memcpy(&value, pdata, sizeof(Uint32));
		return value;
	}

	//=============================================
	// @brief Converts bytes to int
	//
	// @param pdata Pointer to data in bytes
	//=============================================
	inline Int64 ByteToInt64( const byte *pdata )
	{
		Int64 value;
		memcpy(&value, pdata, sizeof(Int64));
		return value;
	}

	//=============================================
	// @brief Converts bytes to unsigned int
	//
	// @param pdata Pointer to data in bytes
	//=============================================
	inline Uint64 ByteToUint64( const byte *pdata )
	{
		Uint64 value;
		memcpy(&value, pdata, sizeof(Uint64));
		return value;
	}

	//=============================================
	// @brief Converts bytes to int
	//
	// @param pdata Pointer to data in bytes
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
		long i;
		Float x, y;

		x = value * 0.5f;
		y = value;

		i = *(long*)&y;
		i = 0x5f3759df - (i>>1);
	
		y = *(Float*)&i;
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
	//=============================================
	inline Float SplineFraction( Float value, Float scale )
	{
		Float valueSquared;

		value = scale * value;
		valueSquared = value * value;

		// Nice little ease-in, ease-out spline-like curve
		return 3 * valueSquared - 2 * valueSquared * value;
	}

	//====================================
	// 
	//
	//====================================
	inline void ScaleByte( DWORD *nInput )
	{
		if(*nInput % 2 != 0)
		{
			*nInput += 1;
		}
	}

	//====================================
	// 
	//
	//====================================
	inline void ParseColor (Float* pout, const color24_t *plightmap)
	{
		pout[0] = (Float)plightmap->r / 255.0f;
		pout[1] = (Float)plightmap->g / 255.0f;
		pout[2] = (Float)plightmap->b / 255.0f;
	}

	//====================================
	// 
	//
	//====================================
	inline Float RemapValue( Float value, Float a, Float b, Float c, Float d )
	{
		return c + (d-c)*(value-a)/(b-a);
	}
}

#endif // COMMON_INLINE_H