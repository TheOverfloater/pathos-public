/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CBITSET_H
#define CBITSET_H

/*
=======================
CBitSet

=======================
*/
class CBitSet
{
public:
	// Number of bits in a byte
	static const Uint32 NB_BITS_IN_BYTE = 8*sizeof(byte);

public:
	CBitSet( void );
	explicit CBitSet( Uint32 size );
	CBitSet( const CBitSet& src );
	CBitSet( const byte* pdataarray, Uint32 numbits );
	CBitSet( Uint32 bitsetSize, const Uint32 inputBits[], Uint32 arraySize );
	~CBitSet( void );

public:
	// Sets a bit
	inline void set( Uint32 index );
	// Resets a bit
	inline void reset( Uint32 index );
	// Flips a bit's state
	inline void flip( Uint32 index );
	// Returns a bit's state
	inline bool test( Uint32 index ) const;
	// Checks if all bits are set
	inline bool all( void ) const;
	// Checks if any of the bits are set
	inline bool any( void ) const;
	// Resizes the bitset
	inline void resize( Uint32 size );
	// Tells the count of set bits
	inline Uint32 count( void ) const;
	// Sets all bits in the bitset
	inline void setall( void );
	// Clears all bits in the bitset
	inline void reset( void );
	// Clears the bitset
	inline void clear( void );

public:
	// Implements assignment operator
	inline CBitSet& operator = ( const CBitSet& other );
	// Implements the bitset OR operator
	inline CBitSet operator | ( const CBitSet& other ) const;
	// Implements the bitset OR assignment operator
	inline CBitSet& operator |= ( const CBitSet& other );
	// Implements the bitset AND operator
	inline CBitSet operator & ( const CBitSet& other ) const;
	// Implements the bitset AND assignment operator
	inline CBitSet& operator &= ( const CBitSet& other );

	// Implements the bitset inverse operator
	inline CBitSet operator ~ ( void ) const;

	// Implements equality check operator
	inline bool operator == ( const CBitSet& other );
	// Implements non-equality check operator
	inline bool operator != ( const CBitSet& other );

public:
	// Get raw bitset data
	byte* getData( void );
	// Get raw bitset data
	const byte* getDataConst( void ) const;
	// Tells the size of the bitset
	inline Uint32 size( void ) const;
	// Tells the number of bytes in the bitset
	inline Uint32 numbytes( void ) const;

private:
	// Array of bits
	byte* m_pDataArray;
	// Number of bits allocated for
	Uint32 m_numBits;
	// Number of bytes
	Uint32 m_numBytes;
};

#include "cbitset_inline.hpp"
#endif //CBITSET_H