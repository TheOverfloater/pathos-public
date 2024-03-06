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
// @brief
//
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
// @brief
//
//=============================================
inline const Char* CMSGReader::GetError( void ) const
{
	return m_errorString.c_str();
}

//=============================================
// @brief
//
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
// @brief
//
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
	byte value = (*pdata);
	m_readOffset++;

	return value;
}

//=============================================
// @brief
//
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
	Char value = (Char)(*pdata);
	m_readOffset++;

	return value;
}

//=============================================
// @brief
//
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
	Int16 value = Common::ByteToInt16(pdata);
	m_readOffset += sizeof(Int16);

	return value;
}

//=============================================
// @brief
//
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
	Uint16 value = Common::ByteToUint16(pdata);
	m_readOffset += sizeof(Uint16);

	return value;
}

//=============================================
// @brief
//
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
	Int32 value = Common::ByteToInt32(pdata);
	m_readOffset += sizeof(Int32);

	return value;
}

//=============================================
// @brief
//
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
	Uint32 value = Common::ByteToUint32(pdata);
	m_readOffset += sizeof(Uint32);

	return value;
}

//=============================================
// @brief
//
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
	Int64 value = Common::ByteToInt64(pdata);
	m_readOffset += sizeof(Int64);

	return value;
}

//=============================================
// @brief
//
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
	Uint64 value = Common::ByteToUint64(pdata);
	m_readOffset += sizeof(Uint64);

	return value;
}

//=============================================
// @brief
//
//=============================================
Float CMSGReader::ReadSmallFloat( void )
{
	if(!m_errorString.empty())
		return 0;

	Int16 value = ReadInt16();
	return value * (1.0f/8.0f);
}

//=============================================
// @brief
//
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
// @brief
//
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
// @brief
//
//=============================================
const Char* CMSGReader::ReadString( void )
{
	if(!m_errorString.empty())
		return "";

	// Read string size
	Uint16 strlength = ReadUint16();

	if(!m_errorString.empty())
		return "";

	if(m_readOffset + strlength > m_messageSize)
	{
		m_errorString = "Read out of bounds on message";
		return 0;
	}

	const Char *pdata = (Char*)(m_pMessageData + m_readOffset);
	m_readOffset += strlength;
	return pdata;
}

//=============================================
// @brief
//
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

#endif //MSGREADER_INLINE_HPP