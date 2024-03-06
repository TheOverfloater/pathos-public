/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CBITSET_INLINE_H
#define CBITSET_INLINE_H

//=============================================
// @brief Sets a bit in the bitset
//
// @param index Bit to set
//=============================================
inline void CBitSet::set( Uint32 index )
{
	assert(index < m_numBits);
	m_pDataArray[index/NB_BITS_IN_INT] |= (1U << (index%NB_BITS_IN_INT));
}

//=============================================
// @brief Resets a bit in the bitset
//
// @param index Bit to reset
//=============================================
inline void CBitSet::reset( Uint32 index )
{
	assert(index < m_numBits);
	m_pDataArray[index/NB_BITS_IN_INT] &= ~(1U << (index%NB_BITS_IN_INT));
}

//=============================================
// @brief Flips a bit's state in the bitset
//
// @param index Bit to flip
//=============================================
inline void CBitSet::flip( Uint32 index )
{
	assert(index < m_numBits);
	m_pDataArray[index/NB_BITS_IN_INT] ^= (1U << (index%NB_BITS_IN_INT));
}

//=============================================
// @brief Tells if a bit is set
//
// @param index Bit to check for
// @return TRUE if bit is set, FALSE otherwise
//=============================================
inline bool CBitSet::test( Uint32 index ) const
{
	assert(index < m_numBits);
	return ((m_pDataArray[index/NB_BITS_IN_INT] & (1U<<(index%NB_BITS_IN_INT))) != 0);
}

//=============================================
// @brief Tells if all bits are set
//
// @return TRUE if all bits are set, FALSE otherwise
//=============================================
inline bool CBitSet::all( void ) const
{
	for(Uint32 i = 0; i < m_numBits; i++)
	{
		if(!test(i))
			return false;
	}

	return true;
}

//=============================================
// @brief Tells if any of the bits are set
//
// @return TRUE if all bits are set, FALSE otherwise
//=============================================
inline bool CBitSet::any( void ) const
{
	for(Uint32 i = 0; i < m_numBits; i++)
	{
		if(test(i))
			return true;
	}

	return false;
}

//=============================================
// @brief Resizes the bitset
//
// @param size New size to set
//=============================================
inline void CBitSet::resize( Uint32 size )
{
	if(m_pDataArray)
		delete[] m_pDataArray;

	Uint32 arraySize = size/NB_BITS_IN_INT;
	if(arraySize*NB_BITS_IN_INT < size)
		arraySize += 1;

	m_pDataArray = new Int32[arraySize];
	memset(m_pDataArray, 0, sizeof(Int32)*arraySize);

	m_numBits = size;
}

//=============================================
// @brief Clears the object
//
//=============================================
inline void CBitSet::clear( void )
{
	if(m_pDataArray)
	{
		delete[] m_pDataArray;
		m_pDataArray = nullptr;
	}

	m_numBits = 0;
}

//=============================================
// @brief
//
//=============================================
inline Uint32 CBitSet::count( void ) const
{
	assert(m_pDataArray != nullptr && m_numBits > 0);

	Uint32 count = 0;
	for(Uint32 i = 0; i < m_numBits; i++)
	{
		if(test(i))
			count++;
	}

	return count;
}

//=============================================
// @brief
//
//=============================================
inline Uint32 CBitSet::size( void ) const
{
	return m_numBits;
}
#endif // CBITSET_INLINE_H