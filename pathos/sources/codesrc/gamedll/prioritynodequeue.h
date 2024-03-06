/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef PRIORITYNODEQUEUE_H
#define PRIORITYNODEQUEUE_H

/*
=======================
CPriorityNodeQueue

=======================
*/
class CPriorityNodeQueue
{
public:
	// Max nodes in the queue
	static const Uint32 MAX_QUEUE_NODES = 128;

public:
	struct heapnode_t
	{
		heapnode_t():
			id(0),
			priority(0)
			{}

		Int32 id;
		Float priority;
	};

public:
	CPriorityNodeQueue( void );
	~CPriorityNodeQueue( void );

public:
	void Insert( Int32 value, Float priority );
	Int32 Remove( Float& priority );

	bool IsFull( void ) const;
	bool IsEmpty( void ) const;
	Uint32 GetSize( void ) const;

private:
	void ShiftDown( Int32 subRoot );
	void ShiftUp( void );

private:
	heapnode_t m_heap[MAX_QUEUE_NODES];
	Uint32 m_size;
};
#endif //PRIORITYNODEQUEUE_H