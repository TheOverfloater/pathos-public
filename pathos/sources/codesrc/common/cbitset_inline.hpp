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
	m_pDataArray[index/NB_BITS_IN_BYTE] |= (1U << (index%NB_BITS_IN_BYTE));
}

//=============================================
// @brief Resets a bit in the bitset
//
// @param index Bit to reset
//=============================================
inline void CBitSet::reset( Uint32 index )
{
	assert(index < m_numBits);
	m_pDataArray[index/NB_BITS_IN_BYTE] &= ~(1U << (index%NB_BITS_IN_BYTE));
}

//=============================================
// @brief Flips a bit's state in the bitset
//
// @param index Bit to flip
//=============================================
inline void CBitSet::flip( Uint32 index )
{
	assert(index < m_numBits);
	m_pDataArray[index/NB_BITS_IN_BYTE] ^= (1U << (index%NB_BITS_IN_BYTE));
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
	return ((m_pDataArray[index/NB_BITS_IN_BYTE] & (1U<<(index%NB_BITS_IN_BYTE))) != 0);
}

//=============================================
// @brief Tells if all bits are set
//
// @return TRUE if all bits are set, FALSE otherwise
//=============================================
inline bool CBitSet::all( void ) const
{
	if(!m_pDataArray)
		return false;

	Uint32 numFullBytes = m_numBits / NB_BITS_IN_BYTE;
	for(Uint32 i = 0; i < numFullBytes; i++)
	{
		if(m_pDataArray[i] != 0xFF)
			return false;
	}

	Uint32 i = numFullBytes * NB_BITS_IN_BYTE;
	for(; i < m_numBits; i++)
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
	if(!m_pDataArray)
		return false;

	for(Uint32 i = 0; i < m_numBytes; i++)
	{
		if(m_pDataArray[i] != 0)
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
	// Remember these for porting values over
	Uint32 originalBitCount = m_numBits;
	byte* pOriginalArray = m_pDataArray;

	// Create new data
	m_numBytes = size/NB_BITS_IN_BYTE;
	if(m_numBytes*NB_BITS_IN_BYTE < size)
		m_numBytes++;

	m_pDataArray = new byte[m_numBytes];
	memset(m_pDataArray, 0, sizeof(byte)*m_numBytes);

	if(pOriginalArray)
	{
		Uint32 refBitCount = (originalBitCount < m_numBits) ? originalBitCount : m_numBits;
		Uint32 numFullBytes = refBitCount / NB_BITS_IN_BYTE;
		for(Uint32 i = 0; i < numFullBytes; i++)
			m_pDataArray[i] = pOriginalArray[i];

		Uint32 i = numFullBytes * NB_BITS_IN_BYTE;
		for(; i < refBitCount; i++)
			m_pDataArray[i/NB_BITS_IN_BYTE] |= (1U << (i%NB_BITS_IN_BYTE));

		delete[] pOriginalArray;
	}

	m_numBits = size;
}

//=============================================
// @brief Sets all bits in the bitset
//
//=============================================
inline void CBitSet::setall( void )
{
	if(!m_pDataArray)
		return;

	Uint32 numFullBytes = m_numBits / NB_BITS_IN_BYTE;
	for(Uint32 i = 0; i < numFullBytes; i++)
		m_pDataArray[i] = 0xFF;

	Uint32 i = numFullBytes * NB_BITS_IN_BYTE;
	for(; i < m_numBits; i++)
		m_pDataArray[i/NB_BITS_IN_BYTE] |= (1U << (i%NB_BITS_IN_BYTE));
}

//=============================================
// @brief Clears all bits in the bitset
//
//=============================================
inline void CBitSet::reset( void )
{
	if(!m_pDataArray)
		return;

	for(Uint32 i = 0; i < m_numBytes; i++)
		m_pDataArray[i] = 0x00;
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

//=============================================
// @brief
//
//=============================================
inline Uint32 CBitSet::numbytes( void ) const
{
	return m_numBytes;
}

//=============================================
// @brief
//
//=============================================
inline byte* CBitSet::getData( void )
{
	return m_pDataArray;
}

//=============================================
// @brief
//
//=============================================
inline const byte* CBitSet::getDataConst( void ) const
{
	return m_pDataArray;
}

//=============================================
// @brief Implements assignment operator
//
//=============================================
inline CBitSet& CBitSet::operator = ( const CBitSet& other )
{
	if(m_pDataArray)
		delete[] m_pDataArray;

	m_numBytes = other.m_numBytes;
	m_numBits = other.m_numBits;

	m_pDataArray = new byte[m_numBytes];
	memcpy(m_pDataArray, other.m_pDataArray, sizeof(byte)*m_numBytes);
	return *this;
}

//=============================================
// @brief Implements the bitset OR operator
//
//=============================================
inline CBitSet CBitSet::operator | ( const CBitSet& other ) const
{
	CBitSet resultSet(*this);
	resultSet |= other;
	return resultSet;
}

//=============================================
// @brief Implements the bitset OR assignment operator
//
//=============================================
inline CBitSet& CBitSet::operator |= ( const CBitSet& other )
{
	Uint32 srcSize = other.size();
	Uint32 myOriginalSize = size();
	if(srcSize > myOriginalSize)
		resize(srcSize);

	// Copy only the smaller scope
	const byte* pOtherBytes = other.getDataConst();
	Uint32 mergeSize = (myOriginalSize > srcSize) ? srcSize : myOriginalSize;
	Uint32 mergeCopyBytes = (mergeSize / NB_BITS_IN_BYTE);
	Uint32 mergeFullByteCount = SDL_ceil(static_cast<Float>(mergeSize) / static_cast<Float>(NB_BITS_IN_BYTE));

	// Directly merge with the full byte range
	for(Uint32 i = 0; i < mergeCopyBytes; i++)
		m_pDataArray[i] = (pOtherBytes[i] | m_pDataArray[i]);

	// Watch for partial bytes at the end of the merge range
	if(mergeFullByteCount > mergeCopyBytes)
	{
		Uint32 i = mergeCopyBytes * NB_BITS_IN_BYTE;
		for(; i < mergeSize; i++)
		{
			if(m_pDataArray[i/NB_BITS_IN_BYTE] & (1U<<(i%NB_BITS_IN_BYTE))
				|| pOtherBytes[i/NB_BITS_IN_BYTE] & (1U<<(i%NB_BITS_IN_BYTE)))
				m_pDataArray[i/NB_BITS_IN_BYTE] |= (1U << (i%NB_BITS_IN_BYTE));
		}
	}

	// Note: Bytes trailing the merge range need not be touched
	return *this;
}

//=============================================
// @brief Implements the bitset AND operator
//
//=============================================
inline CBitSet CBitSet::operator & ( const CBitSet& other ) const
{
	CBitSet resultSet(*this);
	resultSet &= other;
	return resultSet;
}

//=============================================
// @brief Implements the bitset AND assignment operator
//
//=============================================
inline CBitSet& CBitSet::operator &= ( const CBitSet& other )
{
	Uint32 srcSize = other.size();
	Uint32 myOriginalSize = size();
	if(srcSize > myOriginalSize)
		resize(srcSize);

	// Only check till the smallest size, since the
	// OR will not be valid past the minimum size
	const byte* pOtherBytes = other.getDataConst();
	Uint32 mergeSize = (myOriginalSize > srcSize) ? srcSize : myOriginalSize;
	Uint32 mergeCopyBytes = (mergeSize / NB_BITS_IN_BYTE);
	Uint32 mergeFullByteCount = SDL_ceil(static_cast<Float>(mergeSize) / static_cast<Float>(NB_BITS_IN_BYTE));

	// Directly merge with the full byte range
	for(Uint32 i = 0; i < mergeCopyBytes; i++)
		m_pDataArray[i] = (pOtherBytes[i] & m_pDataArray[i]);

	// Watch for partial bytes at the end of the merge range
	if(mergeFullByteCount > mergeCopyBytes)
	{
		// Fill in partial data for the particular byte
		Uint32 i = mergeCopyBytes * NB_BITS_IN_BYTE;
		for(; i < mergeSize; i++)
		{
			if(m_pDataArray[i/NB_BITS_IN_BYTE] & (1U<<(i%NB_BITS_IN_BYTE))
				&& pOtherBytes[i/NB_BITS_IN_BYTE] & (1U<<(i%NB_BITS_IN_BYTE)))
				m_pDataArray[i/NB_BITS_IN_BYTE] |= (1U << (i%NB_BITS_IN_BYTE));
			else
				m_pDataArray[i/NB_BITS_IN_BYTE] &= ~(1U << (i%NB_BITS_IN_BYTE));
		}

		// Remove any remaining bits after the merge range of this byte
		Uint32 totalMergeBitCount = mergeFullByteCount * NB_BITS_IN_BYTE;
		for(; i < totalMergeBitCount; i++)
			m_pDataArray[i/NB_BITS_IN_BYTE] &= ~(1U << (i%NB_BITS_IN_BYTE));
	}

	// We need to clear the trailing bytes after the merge range as well
	if(m_numBytes > mergeFullByteCount)
	{
		for(Uint32 i = mergeFullByteCount; i < m_numBytes; i++)
			m_pDataArray[i] = 0x00;
	}

	return *this;
}

//=============================================
// @brief Implements the bitset inverse operator
//
//=============================================
inline CBitSet CBitSet::operator ~ ( void ) const
{
	// Create a new bitset and reverse it's bits
	CBitSet resultSet(*this);

	Uint32 wholeBytesCount = (resultSet.m_numBits / NB_BITS_IN_BYTE);

	for(Uint32 i = 0; i < wholeBytesCount; i++)
		resultSet.m_pDataArray[i] = ~resultSet.m_pDataArray[i];

	Uint32 i = wholeBytesCount*NB_BITS_IN_BYTE;
	for(; i < resultSet.m_numBits; i++)
	{
		if(!(resultSet.m_pDataArray[i/NB_BITS_IN_BYTE] & (1U<<(i%NB_BITS_IN_BYTE))))
			resultSet.m_pDataArray[i/NB_BITS_IN_BYTE] |= (1U << (i%NB_BITS_IN_BYTE));
		else
			resultSet.m_pDataArray[i/NB_BITS_IN_BYTE] &= ~(1U << (i%NB_BITS_IN_BYTE));
	}

	return resultSet;
}

//=============================================
// @brief Implements equality check operator
//
//=============================================
inline bool CBitSet::operator == ( const CBitSet& other )
{
	assert(other.size() == size());
	if(other.size() != size())
		return false;

	Uint32 wholeBytesCount = (m_numBits / NB_BITS_IN_BYTE);
	for(Uint32 i = 0; i < wholeBytesCount; i++)
	{
		if(m_pDataArray[i] != other.m_pDataArray[i])
			return false;
	}

	Uint32 i = wholeBytesCount*NB_BITS_IN_BYTE;
	for(; i < m_numBits; i++)
	{
		if((m_pDataArray[i/NB_BITS_IN_BYTE] & (1U<<(i%NB_BITS_IN_BYTE)))
			!= (other.m_pDataArray[i/NB_BITS_IN_BYTE] & (1U<<(i%NB_BITS_IN_BYTE))))
			return false;
	}

	return true;
}

//=============================================
// @brief Implements non-equality check operator
//
//=============================================
inline bool CBitSet::operator != ( const CBitSet& other )
{
	assert(other.size() == size());
	if(other.size() != size())
		return true;

	Uint32 wholeBytesCount = (m_numBits / NB_BITS_IN_BYTE);
	for(Uint32 i = 0; i < wholeBytesCount; i++)
	{
		if(m_pDataArray[i] != other.m_pDataArray[i])
			return true;
	}

	Uint32 i = wholeBytesCount*NB_BITS_IN_BYTE;
	for(; i < m_numBits; i++)
	{
		if((m_pDataArray[i/NB_BITS_IN_BYTE] & (1U<<(i%NB_BITS_IN_BYTE)))
			!= (other.m_pDataArray[i/NB_BITS_IN_BYTE] & (1U<<(i%NB_BITS_IN_BYTE))))
			return true;
	}

	return false;
}
#endif // CBITSET_INLINE_H