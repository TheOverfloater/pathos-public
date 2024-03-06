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
	static const Uint32 NB_BITS_IN_INT = 8*sizeof(Int32);

public:
	CBitSet( void );
	explicit CBitSet( Uint32 size );
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
	// Tells the size of the bitset
	inline Uint32 size( void ) const;
	// Clears the bitset
	inline void clear( void );

private:
	// Array of bits
	Int32* m_pDataArray;
	// Number of bits allocated for
	Uint32 m_numBits;
};
#include "cbitset_inline.hpp"
#endif //CBITSET_H