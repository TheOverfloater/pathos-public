/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cbitset.h"

//=============================================
// @brief
//
//=============================================
CBitSet::CBitSet():
	m_pDataArray(nullptr),
	m_numBits(0),
	m_numBytes(0)
{
}

//=============================================
// @brief
//
//=============================================
CBitSet::CBitSet( Uint32 size ):
	m_pDataArray(nullptr),
	m_numBits(size),
	m_numBytes(0)
{
	resize(size);
}

//=============================================
// @brief
//
//=============================================
CBitSet::CBitSet( const CBitSet& src ):
	m_pDataArray(nullptr),
	m_numBits(src.m_numBits),
	m_numBytes(src.m_numBytes)
{
	m_numBytes = src.m_numBytes;
	m_numBits = src.m_numBits;

	m_pDataArray = new byte[m_numBytes];
	memcpy(m_pDataArray, src.m_pDataArray, sizeof(byte)*m_numBytes);
}

//=============================================
// @brief
//
//=============================================
CBitSet::CBitSet( const byte* pdataarray, Uint32 numbits ):
	m_pDataArray(nullptr),
	m_numBits(0),
	m_numBytes(0)
{
	Uint32 wholeBytesNumber = (numbits / NB_BITS_IN_BYTE);
	Uint32 fullByteCount = SDL_ceil(static_cast<Float>(numbits) / static_cast<Float>(NB_BITS_IN_BYTE));

	m_pDataArray = new byte[fullByteCount];
	m_numBits = numbits;
	m_numBytes = fullByteCount;

	for(Uint32 i = 0; i < wholeBytesNumber; i++)
		m_pDataArray[i] = pdataarray[i];

	Uint32 i = wholeBytesNumber * NB_BITS_IN_BYTE;
	for(; i < numbits; i++)
	{
		if(pdataarray[i/NB_BITS_IN_BYTE] & (1U<<(i%NB_BITS_IN_BYTE)))
			set(i);
	}
}

//=============================================
// @brief
//
//=============================================
CBitSet::CBitSet( Uint32 bitsetSize, const Uint32 inputBits[], Uint32 arraySize ):
	m_pDataArray(nullptr),
	m_numBits(0),
	m_numBytes(0)
{
	resize(bitsetSize);
	for(Uint32 i = 0; i < arraySize; i++)
		set(inputBits[i]);
}

//=============================================
// @brief
//
//=============================================
CBitSet::~CBitSet()
{
	if(m_pDataArray)
		delete[] m_pDataArray;
}
