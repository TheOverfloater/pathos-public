/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef NODESTACK_H
#define NODESTACK_H

/*
=======================
CStack

=======================
*/
class CNodeStack
{
public:
	// Max nodes in the queue
	static const Uint32 MAX_STACK_NODES = 128;

public:
	CNodeStack( void );
	~CNodeStack( void );

public:
	void Push( Int32 value );
	Int32 Pop( void );
	Int32 Top( void ) const;
	bool IsEmpty( void ) const;
	Uint32 GetSize( void ) const;
	void CopyToArray( Int32* pArray, Uint32 maxElements );

private:
	Int32 m_stack[MAX_STACK_NODES];
	Int32 m_level;
};
#endif //NODESTACK_H