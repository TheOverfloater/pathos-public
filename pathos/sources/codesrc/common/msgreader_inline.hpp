/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MSGREADER_INLINE_HPP
#define MSGREADER_INLINE_HPP

//=============================================
// @brief Begin reading of a raw byte buffer
//
// @param pdata Pointer to data buffer
// @param msgsize Size of data buffer
//=============================================
inline void CMSGReader::BeginRead( const byte* pdata, Uint32 msgsize )
{
	m_pMessageData = pdata;
	m_messageSize = msgsize;
	m_readOffset = 0;
	
	if(!m_errorString.empty())
		m_errorString.clear();
}

//=============================================
// @brief Get error string if any
//
// @return Pointer to error string
//=============================================
inline const Char* CMSGReader::GetError( void ) const
{
	return m_errorString.c_str();
}

//=============================================
// @brief Tells if we had an error in the read
//
// @return TRUE if an error was encountered, FALSE otherwise
//=============================================
inline bool CMSGReader::HasError( void )
{
	if(!m_errorString.empty())
		return true;

	if(m_messageSize < m_readOffset)
	{
		m_errorString << "Short read";
		return true;
	}

	return m_errorString.empty() ? false : true;
}

//=============================================
// @brief Reads a single byte from the buffer
//
// @return Byte value
//=============================================
byte CMSGReader::ReadByte( void )
{
	if(!m_errorString.empty())
		return 0;

	if(m_readOffset + sizeof(byte) > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return 0;
	}

	const byte *pdata = m_pMessageData + m_readOffset;
	const byte value = (*pdata);
	m_readOffset++;

	return value;
}

//=============================================
// @brief Reads a single char from the buffer
//
// @return Char value
//=============================================
Char CMSGReader::ReadChar( void )
{
	if(!m_errorString.empty())
		return '\0';

	if(m_readOffset + sizeof(Char) > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return '\0';
	}

	const byte *pdata = m_pMessageData + m_readOffset;
	const Char value = static_cast<Char>(*pdata);
	m_readOffset++;

	return value;
}

//=============================================
// @brief Reads a single int16 from the buffer
//
// @return Int16 value
//=============================================
Int16 CMSGReader::ReadInt16( void )
{
	if(!m_errorString.empty())
		return 0;

	if(m_readOffset + sizeof(Int16) > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return 0;
	}

	const byte *pdata = m_pMessageData + m_readOffset;
	const Int16 value = Common::ByteToInt16(pdata);
	m_readOffset += sizeof(Int16);

	return value;
}

//=============================================
// @brief Reads a single uint16 from the buffer
//
// @return Uint16 value
//=============================================
Uint16 CMSGReader::ReadUint16( void )
{
	if(!m_errorString.empty())
		return 0;

	if(m_readOffset + sizeof(Uint16) > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return 0;
	}

	const byte *pdata = m_pMessageData + m_readOffset;
	const Uint16 value = Common::ByteToUint16(pdata);
	m_readOffset += sizeof(Uint16);

	return value;
}

//=============================================
// @brief Reads a single int32 from the buffer
//
// @return Int32 value
//=============================================
Int32 CMSGReader::ReadInt32( void )
{
	if(!m_errorString.empty())
		return 0;

	if(m_readOffset + sizeof(Int32) > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return 0;
	}

	const byte *pdata = m_pMessageData + m_readOffset;
	const Int32 value = Common::ByteToInt32(pdata);
	m_readOffset += sizeof(Int32);

	return value;
}

//=============================================
// @brief Reads a single uint32 from the buffer
//
// @return Uint32 value
//=============================================
Uint32 CMSGReader::ReadUint32( void )
{
	if(!m_errorString.empty())
		return 0;

	if(m_readOffset + sizeof(Uint32) > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return 0;
	}

	const byte *pdata = m_pMessageData + m_readOffset;
	const Uint32 value = Common::ByteToUint32(pdata);
	m_readOffset += sizeof(Uint32);

	return value;
}

//=============================================
// @brief Reads a single int64 from the buffer
//
// @return Byte Int64
//=============================================
Int64 CMSGReader::ReadInt64( void )
{
	if(!m_errorString.empty())
		return 0;

	if(m_readOffset + sizeof(Int64) > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return 0;
	}

	const byte *pdata = m_pMessageData + m_readOffset;
	const Int64 value = Common::ByteToInt64(pdata);
	m_readOffset += sizeof(Int64);

	return value;
}

//=============================================
// @brief Reads a single uint64 from the buffer
//
// @return Uint64 value
//=============================================
Uint64 CMSGReader::ReadUint64( void )
{
	if(!m_errorString.empty())
		return 0;

	if(m_readOffset + sizeof(Uint64) > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return 0;
	}

	const byte *pdata = m_pMessageData + m_readOffset;
	const Uint64 value = Common::ByteToUint64(pdata);
	m_readOffset += sizeof(Uint64);

	return value;
}

//=============================================
// @brief Reads a single half-float from the buffer
//
// @return Half-float value
//=============================================
Float CMSGReader::ReadSmallFloat( void )
{
	if(!m_errorString.empty())
		return 0;

	const Int16 value = ReadInt16();
	return value * (1.0f/8.0f);
}

//=============================================
// @brief Reads a single float from the buffer
//
// @return Float value
//=============================================
Float CMSGReader::ReadFloat( void )
{
	if(!m_errorString.empty())
		return 0;

	if(m_readOffset + sizeof(Float) > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return 0;
	}

	Float value;

	const byte *pdata = m_pMessageData + m_readOffset;
	memcpy(&value, pdata, sizeof(Float));
	m_readOffset += sizeof(Float);

	return value;
}

//=============================================
// @brief Reads a single double from the buffer
//
// @return Double value
//=============================================
Double CMSGReader::ReadDouble( void )
{
	if(!m_errorString.empty())
		return 0;

	if(m_readOffset + sizeof(Double) > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return 0;
	}

	Double value;

	const byte *pdata = m_pMessageData + m_readOffset;
	memcpy(&value, pdata, sizeof(Double));
	m_readOffset += sizeof(Double);

	return value;
}


//=============================================
// @brief Reads a string from the buffer
//
// @return Pointer to string
//=============================================
const Char* CMSGReader::ReadString( void )
{
	if(!m_errorString.empty())
		return "";

	// Read string size
	const Uint16 strlength = ReadUint16();

	if(!m_errorString.empty())
		return "";

	if(m_readOffset + strlength > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return 0;
	}

	const Char *pdata = reinterpret_cast<const Char*>(m_pMessageData + m_readOffset);
	m_readOffset += strlength;
	return pdata;
}

//=============================================
// @brief Reads a byte buffer from the message buffer
//
// @return Pointer to data buffer
//=============================================
const byte* CMSGReader::ReadBuffer( Uint32 size )
{
	if(!m_errorString.empty())
		return nullptr;

	if(m_readOffset + size > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return nullptr;
	}

	const byte *pdata = (m_pMessageData + m_readOffset);
	m_readOffset += size;
	return pdata;
}

//=============================================
// @brief Reads a bitset from the data buffer
//
// @return CBitSet object
//=============================================
CBitSet CMSGReader::ReadBitSet( void )
{
	if(!m_errorString.empty())
		return CBitSet();

	Uint32 numBits = ReadUint32();
	if(!m_errorString.empty())
		return CBitSet();

	Uint32 numBytes = ReadUint32();
	if(!m_errorString.empty())
		return CBitSet();

	// Do a consistency check with CBitSet
	Uint32 fullBytesCount = SDL_ceil(static_cast<Float>(numBits) / static_cast<Float>(CBitSet::NB_BITS_IN_BYTE));
	if(fullBytesCount != numBytes)
	{
		m_errorString << "CBitSet consistency check failed: Expected " << fullBytesCount << " bytes for " << numBits << " bits, message specified " << numBytes << " bytes instead";
		return CBitSet();
	}

	const byte* pDataArray = ReadBuffer(numBytes);
	if(!m_errorString.empty())
		return CBitSet();

	return CBitSet(pDataArray, numBits);
}
#endif //MSGREADER_INLINE_HPP