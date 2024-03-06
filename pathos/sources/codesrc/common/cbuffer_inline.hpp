/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CBUFFER_INLINE_HPP
#define CBUFFER_INLINE_HPP

//=============================================
// @brief Appends data to the buffer
//
// @param pdata Pointer to data
// @param datasize Size of data to append
// @return TRUE if buffer was resized, FALSE otherwise
//=============================================
inline bool CBuffer::append( const void* pdata, Uint32 datasize )
{
	bool wasresized = false;
	Uint32 finalSize = m_bufferDataPosition + datasize;
	if(finalSize >= m_bufferSize)
	{
		CArray<Int64> pointerOffsetsArray;
		if(!m_pointersArray.empty())
		{
			byte* pbaseptr = reinterpret_cast<byte*>(m_pBufferData);

			pointerOffsetsArray.resize(m_pointersArray.size());
			for(Uint32 i = 0; i < m_pointersArray.size(); i++)
			{
				byte* pointer = reinterpret_cast<byte*>(*m_pointersArray[i]);
				Int64 offset = pointer - pbaseptr;
				pointerOffsetsArray[i] = offset;
			}
		}

		Int32 multiplier = 1;
		Uint32 memNeeded = finalSize - (m_bufferSize - m_bufferDataPosition);
		if(memNeeded > m_bufferAllocSize)
		{
			Float nbTimes = (Float)((Float)memNeeded/(Float)m_bufferAllocSize);
			multiplier = (Int32)ceil(nbTimes);
		}

		// Resize the message data buffer
		m_pBufferData = Common::ResizeArray(m_pBufferData, sizeof(byte), m_bufferSize, m_bufferAllocSize*multiplier);
		m_bufferSize = m_bufferSize + m_bufferAllocSize*multiplier;

		// Restore pointers
		if(!pointerOffsetsArray.empty())
		{
			byte* pbaseptr = reinterpret_cast<byte*>(m_pBufferData);
			for(Uint32 i = 0; i < pointerOffsetsArray.size(); i++)
			{
				void* pfinal = reinterpret_cast<void*>(pbaseptr + pointerOffsetsArray[i]);
				(*m_pointersArray[i]) = pfinal;
			}
		}

		wasresized = true;
	}

	if(pdata)
	{
		// Only write data if the pointer is not null, otherwise just reserve it
		byte *pdest = reinterpret_cast<byte*>(m_pBufferData) + m_bufferDataPosition;
		memcpy(pdest, pdata, sizeof(byte)*datasize);
	}

	// Add to offset
	m_bufferDataPosition += datasize;
	return wasresized;
}

//=============================================
// @brief Returns the data in the buffer
//
// @return Reference to data pointer
//=============================================
inline void*& CBuffer::getbufferdata( void )
{
	return m_pBufferData;
}

//=============================================
// @brief Returns the data load size
//
// @return Data load
//=============================================
inline Uint32 CBuffer::getsize( void ) const
{
	return m_bufferDataPosition;
}

//=============================================
// @brief Adds a pointer to the list
//
// @param ptr Pointer to add
//=============================================
inline void CBuffer::addpointer( void** ptr )
{
	byte* pbaseptr = reinterpret_cast<byte*>(m_pBufferData);
	byte* pointer = reinterpret_cast<byte*>(*ptr);

	Int64 offset = (pointer - pbaseptr);
	if(offset < 0 || offset > m_bufferSize)
		assert(false);

	for(Uint32 i = 0; i < m_pointersArray.size(); i++)
	{
		if(m_pointersArray[i] == ptr)
			return;
	}

	m_pointersArray.push_back(ptr);
}

//=============================================
// @brief Removes a pointer to the list
//
// @param ptr Pointer to remove
//=============================================
inline void CBuffer::removepointer( void** ptr )
{
	for(Uint32 i = 0; i < m_pointersArray.size(); i++)
	{
		if(m_pointersArray[i] == ptr)
		{
			m_pointersArray.erase(i);
			return;
		}
	}
}

//=============================================
// @brief Initializes the buffer with the allocation size
//
//=============================================
inline void CBuffer::initbuffer( void )
{
	m_pBufferData = new byte[m_bufferAllocSize];
	memset(m_pBufferData, 0, sizeof(byte)*m_bufferAllocSize);

	m_bufferSize = m_bufferAllocSize;
}
#endif //CBUFFER_INLINE_HPP