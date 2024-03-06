/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "prioritynodequeue.h"

#define HEAP_LEFT_CHILD(x)	(2*(x)+1)
#define HEAP_RIGHT_CHILD(x)	(2*(x)+2)
#define HEAP_PARENT(x)		(((x)-1)/2)

//=============================================
// @brief
//
//=============================================
CPriorityNodeQueue::CPriorityNodeQueue( void ):
	m_size(0)
{
}

//=============================================
// @brief
//
//=============================================
CPriorityNodeQueue::~CPriorityNodeQueue( void )
{
}

//=============================================
// @brief
//
//=============================================
void CPriorityNodeQueue::Insert( Int32 value, Float priority )
{
	if(IsFull())
	{
		gd_engfuncs.pfnCon_DPrintf("%s - Queue full.\n", __FUNCTION__);
		return;
	}

	heapnode_t& node = m_heap[m_size];
	m_size++;

	node.priority = priority;
	node.id = value;

	ShiftUp();
}

//=============================================
// @brief
//
//=============================================
Int32 CPriorityNodeQueue::Remove( Float& priority )
{
	Int32 returnvalue = m_heap[0].id;
	priority = m_heap[0].priority;
	m_size--;

	m_heap[0] = m_heap[m_size];

	ShiftDown(0);
	return returnvalue;
}

//=============================================
// @brief
//
//=============================================
bool CPriorityNodeQueue::IsFull( void ) const
{
	return (m_size >= MAX_QUEUE_NODES) ? true : false;
}

//=============================================
// @brief
//
//=============================================
bool CPriorityNodeQueue::IsEmpty( void ) const
{
	return (m_size == 0) ? true : false;
}

//=============================================
// @brief
//
//=============================================
Uint32 CPriorityNodeQueue::GetSize( void ) const
{
	return m_size;
}

//=============================================
// @brief
//
//=============================================
void CPriorityNodeQueue::ShiftDown( Int32 subRoot )
{
	Int32 parent = subRoot;
	Int32 child = HEAP_LEFT_CHILD(parent);

	heapnode_t reference = m_heap[parent];
	while(child < (Int32)m_size)
	{
		Int32 rightchild = HEAP_RIGHT_CHILD(parent);
		if(rightchild < (Int32)m_size && m_heap[rightchild].priority < m_heap[child].priority)
			child = rightchild;

		if(reference.priority <= m_heap[child].priority)
			break;

		m_heap[parent] = m_heap[child];
		parent = child;

		child = HEAP_LEFT_CHILD(parent);
	}

	m_heap[parent] = reference;
}

//=============================================
// @brief
//
//=============================================
void CPriorityNodeQueue::ShiftUp( void )
{
	Int32 child = m_size-1;
	while(child)
	{
		Int32 parent = HEAP_PARENT(child);
		if(m_heap[parent].priority <= m_heap[child].priority)
			break;

		heapnode_t tmp;
		tmp = m_heap[child];
		m_heap[child] = m_heap[parent];
		m_heap[parent] = tmp;

		child = parent;
	}
}