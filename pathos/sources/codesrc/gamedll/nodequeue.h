/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef NODEQUEUE_H
#define NODEQUEUE_H

/*
=======================
CNodeQueue

=======================
*/
class CNodeQueue
{
public:
	// Max nodes in the queue
	static const Uint32 MAX_STACK_NODES = 128;

public:
	struct queue_t
	{
		queue_t():
			id(0),
			priority(0)
		{}

		Int32 id;
		Float priority;
	};
public:
	CNodeQueue( void );
	~CNodeQueue( void );

public:
	bool IsEmpty( void ) const;
	bool IsFull( void ) const;
	Uint32 GetSize( void ) const;

	void Insert( Int32 value, Float priority );
	Int32 Remove( Float& priority );

private:
	queue_t m_queue[MAX_STACK_NODES];
	Uint32 m_size;

	Int32 m_head;
	Int32 m_tail;
};
#endif //NODEQUEUE_H