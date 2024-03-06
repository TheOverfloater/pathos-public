/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CLINKEDLIST_H
#define CLINKEDLIST_H

/*
=======================
CLinkedList

=======================
*/
template <typename T> class CLinkedList
{
public:
	struct link_t
	{
		friend class CLinkedList;
		link_t():
			_val(),
			_pnext(nullptr),
			_pprev(nullptr)
		{}
		bool rlast( void ) const { return (_pprev == nullptr ? true : false); }
		bool last( void ) const { return (_pnext == nullptr ? true : false); }

	public:
		T _val;

	private:
		link_t* _pnext;
		link_t* _pprev;
	};

	struct iterator_t
	{
		iterator_t():
			plink(nullptr),
			skipnext(false)
			{
			}

		link_t *plink;
		bool skipnext;
	};

public:
	inline CLinkedList( void );
	inline CLinkedList( CLinkedList& src );
	inline ~CLinkedList( void );

	// Adds an element to the list
	inline link_t* add( T element );
	// Adds an element to the list at the tail
	inline link_t* radd( T element );
	// Adds an element to the list before the link
	inline link_t* insert_before( link_t* link, T element );
	// Adds an element to the list after the link
	inline link_t* insert_after( link_t* link, T element );
	// Removes an element from the list
	inline void remove( link_t* link );
	// Removes an element from the list
	inline bool remove( const T element );
	// Returns the current element link
	inline link_t* get_link( void );
	// Returns the current element
	inline T& get( void );
	// Sets up for iteration
	inline void begin( void );
	// Sets up for iteration
	inline void begin( link_t* link );
	// Sets up for iteration from the end
	inline void rbegin( void );
	// Tells if we've reached the end
	inline bool end( void ) const;
	// Goes to the next element in the link
	inline void next( void );
	// Goes to the previous element in the link
	inline void prev( void );
	// Clears the linked list
	inline void clear( void );
	// Tells if the list is empty
	inline bool empty( void ) const;
	// Returns the size of the list
	inline Uint32 size( void ) const;
	// Pushes an iterator
	inline void push_iterator( void );
	// Pushes an iterator
	inline void pop_iterator( void );

	// assignment operator
	inline CLinkedList<T>& operator=( const CLinkedList<T>& src );

private:
	// Pointer to first link in the chain
	link_t* m_pLinkHead;
	// Pointer to the last link in the chain
	link_t* m_pLinkTail;
	// Current link iterator
	iterator_t m_iterator;
	// Number of elements in the chain
	Uint32 m_numLinks;
	// Array of pushed iterators
	CArray<iterator_t> m_pushedIteratorsArray;
};
#include "clinkedlist_inline.hpp"
#endif //CLINKEDLIST_H