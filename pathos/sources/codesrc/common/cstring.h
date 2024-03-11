/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CSTRING_H
#define CSTRING_H

/*
=======================
CString

=======================
*/
class CString
{
public:
	CString();
	CString( const Char* pstr );
	CString( const CString& str );
	CString( const Char* pstr, Uint32 length );
	~CString();
	
public:
	// Assignment operator
	CString& operator=(const CString& str);
	// Comparison operator
	inline bool operator==(const Char* pstr);
	// Comparison operator
	inline bool operator==(const CString& str);

	// Byte shift operator
	inline CString& operator<<(const Char* pstr);
	// Byte shift operator
	inline CString& operator<<(const CString& str);
	// Byte shift operator
	inline CString& operator<<(Char c);
	// Byte shift operator
	inline CString& operator<<(Int32 i);
	// Byte shift operator
	inline CString& operator<<(Uint32 i);
	// Byte shift operator
	inline CString& operator<<(Float f);
	// Byte shift operator
	inline CString& operator<<(Double d);
	// Plus equal operator
	inline CString& operator+=(const Char* pstr);
	// Plus equal operator
	inline CString& operator+=(const CString& str);
	// Plus equal operator
	inline CString& operator+=(Char c);
	// Plus equal operator
	inline CString& operator+=(Int32 i);
	// Plus equal operator
	inline CString& operator+=(Uint32 i);
	// Plus equal operator
	inline CString& operator+=(Float f);
	// Plus equal operator
	inline CString& operator+=(Double d);
	// Plus operator
	inline CString operator+(const Char* pstr);
	// Plus operator
	inline CString operator+(const CString& str);
	// Plus operator
	inline CString operator+(Char c);
	// Plus operator
	inline CString operator+(Int32 i);
	// Plus operator
	inline CString operator+(Uint32 i);
	// Plus operator
	inline CString operator+(Float f);
	// Plus operator
	inline CString operator+(Double d);
	// Indexing operator
	inline Char operator[] (Uint32 n) const;

public:
	// Returns the raw string
	inline const Char* c_str( void ) const;
	// Returns the string length
	inline Uint32 length( void ) const;
	// Returns if the string is empty
	inline bool empty( void ) const;
	// Returns if the string is empty
	inline void clear( void );
	// Assigns a string to use
	inline void assign( const Char* pstr, Uint32 num );
	// Finds the first occurrence of a string in the string
	inline Int32 find( Uint32 offset, const Char* psubstr );
	// Erases part of the string
	inline void erase( Uint32 begin, Uint32 numremove );
	// Converts characters to lowercase
	inline void tolower( void ) const;
	// Inserts a substrinct into the string
	inline void insert( Uint32 begin, const Char* pstrsubstr );
	// Replaces slashes with PATH_SLASH_CHAR
	inline void replaceslashes( void );

public:
	// Appends a string to the current one
	inline void Append( const Char* psrc );
	// Appends a string to the current one
	inline void Append( Char c );
	// Appends a string to the current one
	inline void Append( Int32 i );
	// Appends a string to the current one
	inline void Append( Uint32 i );
	// Appends a string to the current one
	inline void Append( Float f );
	// Appends a string to the current one
	inline void Append( Double d );

private:
	// Pointer to string
	Char* m_pString;
	// Length of the string
	Uint32 m_stringLength;
};
// For adding strings together
extern inline CString operator + ( const CString& lhs, const CString& rhs );

#include "cstring_inline.hpp"
#endif // CSTRING_H