/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MSGREADER_H
#define MSGREADER_H

/*
====================
CMSGReader

====================
*/
class CMSGReader
{
public:
	CMSGReader():
		m_pMessageData(nullptr),
		m_messageSize(0),
		m_readOffset(0)
	{
	}
	CMSGReader( const byte* pdata, Uint32 msgsize ):
		m_pMessageData(nullptr),
		m_messageSize(0),
		m_readOffset(0)
	{
		BeginRead(pdata, msgsize);
	}
	~CMSGReader()
	{
	}

public:
	// Begins reading a message
	inline void BeginRead( const byte* pdata, Uint32 msgsize );
	// Returns the error code if any
	inline const Char* GetError( void ) const;
	// Returns if we have an error
	inline bool HasError( void );

public:
	// Reads a single byte
	inline byte ReadByte( void );
	// Reads a single char
	inline Char ReadChar( void );
	// Reats an short
	inline Int16 ReadInt16( void );
	// Reads a unsigned short
	inline Uint16 ReadUint16( void );
	// Reads a 32-bit integer
	inline Int32 ReadInt32( void );
	// Reads a 32-bit unsigned integer
	inline Uint32 ReadUint32( void );
	// Reads a 64-bit integer
	inline Int64 ReadInt64( void );
	// Reads a 64-bit unsigned integer
	inline Uint64 ReadUint64( void );
	// Reads a 2-byte small float
	inline Float ReadSmallFloat( void );
	// Reads a float
	inline Float ReadFloat( void );
	// Reads a Double
	inline Double ReadDouble( void );
	// Reads a string
	inline const Char* ReadString( void );
	// Reads a buffer
	inline const byte* ReadBuffer( Uint32 size );

private:
	// Message data pointer
	const byte* m_pMessageData;
	// Message size
	Uint32 m_messageSize;
	// Message read offset
	Uint32 m_readOffset;

	// Error string
	CString m_errorString;
};
#include "msgreader_inline.hpp"
#endif //MSGREADER_H