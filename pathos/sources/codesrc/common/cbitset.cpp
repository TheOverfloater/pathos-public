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
	m_numBits(0)
{
}

//=============================================
// @brief
//
//=============================================
CBitSet::CBitSet( Uint32 size ):
	m_pDataArray(nullptr),
	m_numBits(size)
{
	resize(size);
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
