/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CBUFFER_H
#define CBUFFER_H

/*
=======================
CBuffer

=======================
*/
class CBuffer
{
public:
	CBuffer( void );
	explicit CBuffer( Uint32 allocsize );
	CBuffer( const void* pdata, Uint32 datasize );
	CBuffer( Uint32 allocsize, const void* pdata, Uint32 datasize );
	~CBuffer( void );

public:
	// Appends data to the buffer
	inline bool append( const void* pdata, Uint32 datasize );
	// Returns the buffer pointer
	inline void*& getbufferdata( void );
	// Retreives the buffer size
	inline Uint32 getsize( void ) const;

	// Adds a pointer to the list
	inline void addpointer( void** ptr );
	// Removes a pointer from the list
	inline void removepointer( void** ptr );

private:
	// initializes the buffer
	inline void initbuffer( void );

public:
	// Pointer to buffer
	void* m_pBufferData;
	// Size of buffer
	Uint32 m_bufferSize;
	// Buffer usage
	Uint32 m_bufferDataPosition;
	// Buffer allocation unit size
	Uint32 m_bufferAllocSize;

	// List of pointers
	CArray<void**> m_pointersArray;
};
#include "cbuffer_inline.hpp"
#endif