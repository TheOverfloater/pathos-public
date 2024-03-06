/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <cmath>

/*
=======================
Vector

=======================
*/
class Vector
{
public:
	// Constructors
	inline Vector( void );
	inline Vector( Float srcX, Float srcY, Float srcZ );
	inline Vector( const Vector& src );
	inline Vector( Float coords[3] );

	// Negation operator
	inline Vector operator-( void ) const;
	// Equality operator
	inline bool operator==( const Vector& rhv ) const;
	// Not equals operator
	inline bool operator!=( const Vector& rhv ) const;
	// Addition operator
	inline Vector operator+( const Vector& rhv ) const;
	// Plus equals operator
	inline Vector& operator+=( const Vector& rhv );
	// Subtration operator
	inline Vector operator-( const Vector& rhv ) const;
	// Minus equals operator
	inline Vector& operator-=( const Vector& rhv );
	// Multiplication operator
	inline Vector operator*( const Vector& rhv ) const;
	// Multiplication and assignment operator
	inline Vector& operator*=( const Vector& rhv );
	// Division operator
	inline Vector operator/( const Vector& rhv ) const;
	// Division and assignment operator
	inline Vector& operator/=( const Vector& rhv );
	// Assignment operator
	inline Vector& operator=( const Vector& rhv );
	// Indexing operator
	inline Float& operator[]( Uint32 n );
	// Indexing operator
	inline Float operator[]( Uint32 n ) const;
	// Indexing operator
	inline Float& operator[]( Int32 n );
	// Indexing operator
	inline Float operator[]( Int32 n ) const;
	// Multiplication operator
	inline Vector operator*(Float value) const;
	// Multiplication operator
	inline Vector operator/(Float value) const;

	// Operator to convert to float pointer
	operator Float*( void );
	// Operator to convert to const float pointer
	operator const Float*( void ) const;

	// Tells the length of the vector
	inline Float Length( void ) const;
	// Tells the 2d length of the vector
	inline Float Length2D( void ) const;
	// Returns the normalized vector
	inline Vector& Normalize( void );
	// Clears the vector
	inline void Clear( void );
	// Tells if the vector is null
	inline bool IsZero( void ) const;
	// Tells if the vector value is NAN
	inline bool IsNAN( Int32 index ) const;

public:
	// Elements of the vector
	Float x;
	Float y;
	Float z;
};
// For multiplying Vector with float array
inline Vector operator*( Float lhv, const Vector& rhv );

#include "vector_inline.hpp"
#endif // VECTOR_H