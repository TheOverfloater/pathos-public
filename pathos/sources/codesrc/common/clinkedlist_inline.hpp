/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CLINKEDLIST_INLINE_HPP
#define CLINKEDLIST_INLINE_HPP

//=============================================
// @brief Default constructor
//
//=============================================
template <typename T> inline CLinkedList<T>::CLinkedList():
	m_pLinkHead(nullptr),
	m_pLinkTail(nullptr),
	m_numLinks(0)
{
}

//=============================================
// @brief Copy constructor
//
// @param src Source list
//=============================================
template <typename T> inline CLinkedList<T>::CLinkedList( CLinkedList& src ):
	m_pLinkHead(nullptr),
	m_pLinkTail(nullptr),
	m_numLinks(0)
{
	src.begin();
	while(!src.end())
	{
		add(src.get());
		src.next();
	}

	if(!m_pushedIteratorsArray.empty())
		m_pushedIteratorsArray.clear();
}

//=============================================
// @brief Destructor
//
//=============================================
template <typename T> inline CLinkedList<T>::~CLinkedList()
{
	clear();
}

//=============================================
// @brief Adds an element to the linked list
//
// @param T Element to add
//=============================================
template <typename T> inline typename CLinkedList<T>::link_t* CLinkedList<T>::add( const T element )
{
	typename CLinkedList<T>::link_t* pNew = new typename CLinkedList<T>::link_t;
	
	// Set values
	pNew->_pnext = m_pLinkHead;
	pNew->_val = element;

	// If there's already a link
	if(m_pLinkHead)
		m_pLinkHead->_pprev = pNew;

	if(!m_pLinkTail)
		m_pLinkTail = pNew;

	m_pLinkHead = pNew;
	m_numLinks++;

	return pNew;
}

//=============================================
// @brief Adds an element to the linked list at the tail
//
// @param T Element to add
//=============================================
template <typename T> inline typename CLinkedList<T>::link_t* CLinkedList<T>::radd( const T element )
{
	typename CLinkedList<T>::link_t* pNew = new typename CLinkedList<T>::link_t;
	
	// Set values
	pNew->_pprev = m_pLinkTail;
	pNew->_val = element;

	// If there's already a link
	if(m_pLinkTail)
		m_pLinkTail->_pnext = pNew;

	if(!m_pLinkHead)
		m_pLinkHead = pNew;

	m_pLinkTail = pNew;
	m_numLinks++;

	return pNew;
}

//=============================================
// @brief Adds an element to the linked list before a given position
//
// @param link Element before which to add
// @param T Element to add
//=============================================
template <typename T> inline typename CLinkedList<T>::link_t* CLinkedList<T>::insert_before( typename CLinkedList<T>::link_t* link, const T element )
{
	typename CLinkedList<T>::link_t* pNew = new typename CLinkedList<T>::link_t;

	if(link->_pprev)
	{
		link->_pprev->_pnext = pNew;
		pNew->_pprev = link->_pprev;
	}
	else
		m_pLinkHead = pNew;

	link->_pprev = pNew;
	pNew->_pnext = link;

	// Set value
	pNew->_val = element;
	m_numLinks++;

	return pNew;
}

//=============================================
// @brief Adds an element to the linked list after a given element
//
// @param Element to add after
// @param T Element to add
//=============================================
template <typename T> inline typename CLinkedList<T>::link_t* CLinkedList<T>::insert_after( typename CLinkedList<T>::link_t* link, const T element )
{
	typename CLinkedList<T>::link_t* pNew = new typename CLinkedList<T>::link_t;

	if(link->_pnext)
	{
		link->_pnext->_pprev = pNew;
		pNew->_pnext = link->_pnext;
	}
	else
		m_pLinkTail = pNew;

	link->_pnext = pNew;
	pNew->_pprev = link;

	// Set value
	pNew->_val = element;
	m_numLinks++;

	return pNew;
}

//=============================================
// @brief Removes an element from the linked list
//
// @param link Pointer to link to remove
//=============================================
template <typename T> inline void CLinkedList<T>::remove( typename CLinkedList<T>::link_t* link )
{
	if(link == m_iterator.plink)
	{
		m_iterator.plink = m_iterator.plink->_pnext;
		m_iterator.skipnext = true;
	}

	for(Uint32 i = 0; i < m_pushedIteratorsArray.size(); i++)
	{
		if(m_pushedIteratorsArray[i].plink == link)
		{
			m_pushedIteratorsArray[i].plink = m_pushedIteratorsArray[i].plink->_pnext;
			m_pushedIteratorsArray[i].skipnext = true;
		}
	}

	if(!link->_pprev)
		m_pLinkHead = link->_pnext;
	else
		link->_pprev->_pnext = link->_pnext;

	if(link->_pnext)
		link->_pnext->_pprev = link->_pprev;

	if(m_pLinkTail == link)
		m_pLinkTail = link->_pprev;

	delete link;
	m_numLinks--;
}

//=============================================
// @brief Removes an element from the linked list
//
// @param link Pointer to element to remove
// @param TRUE if it was found, FALSE otherwise
//=============================================
template <typename T> inline bool CLinkedList<T>::remove( const T element )
{
	if(!m_pLinkHead)
		return false;

	typename CLinkedList<T>::link_t* pnext = m_pLinkHead;
	while(pnext)
	{
		if(pnext->_val == element)
		{
			remove(pnext);
			return true;
		}

		pnext = pnext->_pnext;
	}

	return false;
}

//=============================================
// @brief Returns the iterator for the current link
//
// @return Iterator for current element
//=============================================
template <typename T> inline typename CLinkedList<T>::link_t* CLinkedList<T>::get_link( void )
{
	return m_iterator.plink;
}

//=============================================
// @brief Returns the pointer for the element
//
// @return Pointer for the current element
//=============================================
template <typename T> inline typename T& CLinkedList<T>::get( void )
{
	return m_iterator.plink->_val;
}

//=============================================
// @brief Resets the iterator to the head link
//
//=============================================
template <typename T> inline void CLinkedList<T>::begin( void )
{
	m_iterator.plink = m_pLinkHead;
	m_iterator.skipnext = false;
}

//=============================================
// @brief Resets the iterator to the head link
//
//=============================================
template <typename T> inline void CLinkedList<T>::begin( link_t* link )
{
	m_iterator.plink = link;
	m_iterator.skipnext = false;
}

//=============================================
// @brief Resets the iterator to the back link
//
//=============================================
template <typename T> inline void CLinkedList<T>::rbegin( void )
{
	m_iterator.plink = m_pLinkTail;
	m_iterator.skipnext = false;
}

//=============================================
// @brief Tells if we've reached the end
//
// @return FALSE if we still have a valid link, FALSE otherwise
//=============================================
template <typename T> inline bool CLinkedList<T>::end( void ) const
{
	return (!m_iterator.plink ? true : false);
}

//=============================================
// @brief Moves onto the next link in the chain
//
//=============================================
template <typename T> inline void CLinkedList<T>::next( void )
{
	if(!m_iterator.plink)
		return;

	if(m_iterator.skipnext)
	{
		m_iterator.skipnext = false;
		return;
	}

	m_iterator.plink = m_iterator.plink->_pnext;
}

//=============================================
// @brief Moves onto the previous link in the chain
//
//=============================================
template <typename T> inline void CLinkedList<T>::prev( void )
{
	if(!m_iterator.plink)
		return;

	if(m_iterator.skipnext)
	{
		m_iterator.skipnext = false;
		return;
	}

	m_iterator.plink = m_iterator.plink->_pprev;
}

//=============================================
// @brief Clears the linked list
//
//=============================================
template <typename T> inline void CLinkedList<T>::clear( void )
{
	if(!m_pLinkHead)
		return;

	typename CLinkedList<T>::link_t* pnext = m_pLinkHead;
	while(pnext)
	{
		typename CLinkedList<T>::link_t* pfree = pnext;
		pnext = pfree->_pnext;

		remove(pfree);
	}

	m_numLinks = 0;

	if(!m_pushedIteratorsArray.empty())
		m_pushedIteratorsArray.clear();
}

//=============================================
// @brief Tells if the list is empty
//
// @return TRUE if the list is empty, FALSE otherwise
//=============================================
template <typename T> inline bool CLinkedList<T>::empty( void ) const
{
	return (!m_pLinkHead) ? true : false;
}

//=============================================
// @brief Returns the size of the list
//
// @return Number of elements in the list
//=============================================
template <typename T> inline Uint32 CLinkedList<T>::size( void ) const
{
	return m_numLinks;
}

//=============================================
// @brief Pushes the current iterator into the iterator stack
//
//=============================================
template <typename T> inline void CLinkedList<T>::push_iterator( void )
{
	m_pushedIteratorsArray.push_back(m_iterator);
}

//=============================================
// @brief Pops the previous iterator from the iterator stack
//
//=============================================
template <typename T> inline void CLinkedList<T>::pop_iterator( void )
{
	assert(!m_pushedIteratorsArray.empty());

	Int32 index = m_pushedIteratorsArray.size()-1;
	m_iterator = m_pushedIteratorsArray[index];

	m_pushedIteratorsArray.resize(m_pushedIteratorsArray.size()-1);
}

//=============================================
// @brief Assignment operator
//
//=============================================
template <typename T> inline CLinkedList<T>& CLinkedList<T>::operator=( const CLinkedList<T>& src )
{
	if(!empty())
		clear();

	CLinkedList<T>::iterator_t it;
	it.plink = src.m_pLinkHead;
	while(it.plink)
	{
		add(it.plink->_val);
		it.plink = it.plink->_pnext;
	}

	if(!m_pushedIteratorsArray.empty())
		m_pushedIteratorsArray.clear();

	return *this;
}
#endif // CLINKEDLIST_INLINE_HPP