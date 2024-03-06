/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cbuffer.h"

// Default allocation size is 1kb
static const Uint32 DEFAULT_ALLOC_SIZE = 1024;

//=============================================
// @brief
//
//=============================================
CBuffer::CBuffer( void ):
	m_pBufferData(nullptr),
	m_bufferSize(0),
	m_bufferDataPosition(0),
	m_bufferAllocSize(DEFAULT_ALLOC_SIZE)
{
	// Initialize buffer
	initbuffer();
}

//=============================================
// @brief
//
//=============================================
CBuffer::CBuffer( Uint32 allocsize ):
	m_pBufferData(nullptr),
	m_bufferSize(0),
	m_bufferDataPosition(0),
	m_bufferAllocSize(allocsize)
{
	assert(m_bufferAllocSize > 0);

	// Initialize buffer
	initbuffer();
}

//=============================================
// @brief
//
//=============================================
CBuffer::CBuffer( const void* pdata, Uint32 datasize ):
	m_pBufferData(nullptr),
	m_bufferSize(0),
	m_bufferDataPosition(0),
	m_bufferAllocSize(DEFAULT_ALLOC_SIZE)
{
	// Initialize buffer
	initbuffer();

	if(pdata && datasize > 0)
	{
		// Now append to it
		append(pdata, datasize);
	}
	else
	{
		// Just reserve the space
		m_bufferDataPosition = datasize;
	}
}

//=============================================
// @brief
//
//=============================================
CBuffer::CBuffer( Uint32 allocsize, const void* pdata, Uint32 datasize ):
	m_pBufferData(nullptr),
	m_bufferSize(0),
	m_bufferDataPosition(0),
	m_bufferAllocSize(allocsize)
{
	assert(m_bufferAllocSize > 0);

	// Initialize buffer
	initbuffer();

	if(pdata && datasize > 0)
	{
		// Now append to it
		append(pdata, datasize);
	}
	else
	{
		// Just reserve the space
		m_bufferDataPosition = datasize;
	}
}

//=============================================
// @brief
//
//=============================================
CBuffer::~CBuffer( void )
{
	if(m_pBufferData)
	{
		byte* pdata = reinterpret_cast<byte*>(m_pBufferData);
		delete[] pdata;
	}
}