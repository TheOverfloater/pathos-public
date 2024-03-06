/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "nodestack.h"

//=============================================
// @brief
//
//=============================================
CNodeStack::CNodeStack( void ):
	m_level(0)
{
	memset(m_stack, 0, sizeof(m_stack));
}

//=============================================
// @brief
//
//=============================================
CNodeStack::~CNodeStack( void )
{
}

//=============================================
// @brief
//
//=============================================
void CNodeStack::Push( Int32 value )
{
	if(m_level >= MAX_STACK_NODES)
	{
		gd_engfuncs.pfnCon_DPrintf("%s - Stack full.\n", __FUNCTION__);
		return;
	}

	m_stack[m_level] = value;
	m_level++;
}

//=============================================
// @brief
//
//=============================================
Int32 CNodeStack::Pop( void )
{
	if(m_level <= 0)
		return NO_POSITION;

	m_level--;
	return m_stack[m_level];
}

//=============================================
// @brief
//
//=============================================
Int32 CNodeStack::Top( void ) const
{
	if(m_level <= 0)
		return NO_POSITION;
	else
		return m_stack[m_level-1];
}

//=============================================
// @brief
//
//=============================================
bool CNodeStack::IsEmpty( void ) const
{
	return (m_level <= 0) ? true : false;
}

//=============================================
// @brief
//
//=============================================
Uint32 CNodeStack::GetSize( void ) const
{
	return m_level;
}

//=============================================
// @brief
//
//=============================================
void CNodeStack::CopyToArray( Int32* pArray, Uint32 maxElements )
{
	Int32 numCopy = _max((Int32)maxElements, m_level);
	for(Int32 i = 0; i < numCopy; i++)
		pArray[i] = m_stack[i];
}