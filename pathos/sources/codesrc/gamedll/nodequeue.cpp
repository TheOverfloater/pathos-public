/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "nodequeue.h"

//=============================================
// @brief
//
//=============================================
CNodeQueue::CNodeQueue( void ):
	m_size(0),
	m_head(0),
	m_tail(NO_POSITION)
{
}

//=============================================
// @brief
//
//=============================================
CNodeQueue::~CNodeQueue( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CNodeQueue::IsEmpty( void ) const
{
	return (m_size == 0) ? true : false;
}

//=============================================
// @brief
//
//=============================================
bool CNodeQueue::IsFull( void ) const
{
	return (m_size >= MAX_STACK_NODES) ? true : false;
}

//=============================================
// @brief
//
//=============================================
Uint32 CNodeQueue::GetSize( void ) const
{
	return m_size;
}

//=============================================
// @brief
//
//=============================================
void CNodeQueue::Insert( Int32 value, Float priority )
{
	if(IsFull())
	{
		gd_engfuncs.pfnCon_DPrintf("%s - Queue full.\n", __FUNCTION__);
		return;
	}

	m_tail++;

	m_queue[m_tail].id = value;
	m_queue[m_tail].priority = priority;
	m_size++;
}

//=============================================
// @brief
//
//=============================================
Int32 CNodeQueue::Remove( Float& priority )
{
	if(m_head >= MAX_STACK_NODES)
		m_head = 0;

	m_size--;
	priority = m_queue[m_head].priority;
	Int32 id = m_queue[m_head].id;
	m_head++;

	return id;
}