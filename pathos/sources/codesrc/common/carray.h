/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CARRAY_H
#define CARRAY_H

/*
=======================
CArray

=======================
*/
template <typename T> class CArray
{
public:
	inline CArray( void );
	inline explicit CArray( Uint32 size );
	inline CArray( const CArray& src );
	inline ~CArray( void );

public:
	// Assignment operator
	inline void operator=( const CArray& src );

	// Resizes the array
	inline void resize( Uint32 size );
	// Resizes the array
	inline void reserve( Uint32 size );
	// Deallocates the array
	inline void clear( void );
	// Returns the size of the array
	inline Uint32 size( void ) const;
	// Returns if the array is empty
	inline bool empty( void ) const;
	// pushes an element back
	inline void push_back( const T& element );
	// Erases an element from the array
	inline void erase( Uint32 index );
	// Erases an element from the array
	inline void erase( const T& element );

	// Indexing operator
	inline T& operator[] ( Uint32 n ) const;

private:
	// Elements in array
	T* m_pArray;
	// Size of array
	Uint32 m_arraySize;
	// Actual size of the array
	Uint32 m_reservedSize;
};

#include "carray_inline.hpp"
#endif // CARRAY_H