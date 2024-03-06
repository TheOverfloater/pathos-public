/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CARRAY_INLINE_HPP
#define CARRAY_INLINE_HPP

//=============================================
// @brief Default constructor
//
//=============================================
template <typename T> inline CArray<T>::CArray( void ):
	m_pArray(nullptr),
	m_arraySize(0),
	m_reservedSize(0)
{
}

//=============================================
// @brief Constructor with pre-defined size
//
// @param size Size to allocate
//=============================================
template <typename T> inline CArray<T>::CArray( Uint32 size ):
	m_pArray(nullptr),
	m_arraySize(0),
	m_reservedSize(0)
{
	resize(size);
}

//=============================================
// @brief Copy constructor
//
// @param src Source array
//=============================================
template <typename T> inline CArray<T>::CArray( const CArray& src ):
	m_pArray(nullptr),
	m_arraySize(0),
	m_reservedSize(0)
{
	resize(src.size());
	for(Uint32 i = 0; i < size(); i++)
		m_pArray[i] = src[i];
}

//=============================================
// @brief Destructor
//
//=============================================
template <typename T> inline CArray<T>::~CArray( void )
{
	if(m_pArray)
		delete[] m_pArray;
}

//=============================================
// @brief Assignment operator
//
//=============================================
template <typename T> inline void CArray<T>::operator=( const CArray& src )
{
	resize(src.size());
	for(Uint32 i = 0; i < size(); i++)
		m_pArray[i] = src[i];
}

//=============================================
// @brief Resizes the array whilst keeping the original elements up till size
//
// @param size New size to set
//=============================================
template <typename T> inline void CArray<T>::resize( Uint32 size )
{
	if(!size)
	{
		if(m_pArray)
		{
			delete[] m_pArray;
			m_pArray = nullptr;
		}

		m_arraySize = 0;
		m_reservedSize = 0;
	}
	else if(!m_pArray)
	{
		// just allocate a new array
		m_pArray = new T[size]();
		m_arraySize = size;
		m_reservedSize = size;
	}
	else
	{
		// Copy elements to the new array
		T* newArray = new T[size]();
		Uint32 numCopy = size;
		if(numCopy > m_arraySize)
			numCopy = m_arraySize;

		for(Uint32 i = 0; i < numCopy; i++)
			newArray[i] = m_pArray[i];

		// delete the old array
		delete[] m_pArray;
		m_pArray = newArray;

		// set final size
		m_arraySize = size;
		m_reservedSize = size;
	}
}

//=============================================
// @brief Reserves elements for an array, does not set m_arraySize
//
// @param size Size to be reserved
//=============================================
template <typename T> inline void CArray<T>::reserve( Uint32 size )
{
	assert(size > m_arraySize);

	if(!m_pArray)
	{
		// just allocate a new array
		m_pArray = new T[size]();
		m_reservedSize = size;
	}
	else
	{
		// Copy elements to the new array
		T* newArray = new T[size]();
		for(Uint32 i = 0; i < m_arraySize; i++)
			newArray[i] = m_pArray[i];

		// delete the old array
		delete[] m_pArray;
		m_pArray = newArray;

		// set final size
		m_reservedSize = size;
	}
}

//=============================================
// @brief Clears the array
//
//=============================================
template <typename T> inline void CArray<T>::clear( void )
{
	if(m_pArray)
	{
		delete[] m_pArray;
		m_pArray = nullptr;

		m_arraySize = 0;
		m_reservedSize = 0;
	}
}

//=============================================
// @brief Tells the size of the array
//
//=============================================
template <typename T> inline Uint32 CArray<T>::size( void ) const
{
	return m_arraySize;
}

//=============================================
// @brief Tells if the array is empty
//
//=============================================
template <typename T> inline bool CArray<T>::empty( void ) const
{
	if(!m_pArray || !m_arraySize)
		return true;
	else
		return false;
}

//=============================================
// @brief Resizes the array
//
// @param size Size to set
//=============================================
template <typename T> inline void CArray<T>::push_back( const T& element )
{
	Uint32 addIndex = m_arraySize;
	if(m_reservedSize <= m_arraySize)
		resize(m_arraySize+1);

	m_pArray[addIndex] = element;

	if(m_arraySize != m_reservedSize)
		m_arraySize++;
}

//=============================================
// @brief Erases an element from the array
//
// @param index Index of the element to remove
//=============================================
template <typename T> inline void CArray<T>::erase( Uint32 index )
{
	T* newArray = new T[m_reservedSize-1]();
	if(index > 0)
	{
		for(Uint32 i = 0; i < index; i++)
			newArray[i] = m_pArray[i];
	}
	
	if(index < (m_arraySize-1))
	{
		for(Uint32 i = m_arraySize-1; i > index; i--)
			newArray[i-1] = m_pArray[i];
	}

	delete[] m_pArray;
	m_pArray = newArray;
	m_arraySize--;
	m_reservedSize--;
}

//=============================================
// @brief Erases an element from the array
//
// @param element Element to remove
//=============================================
template <typename T> inline void CArray<T>::erase( const T& element )
{
	Uint32 i = 0;
	for(; i < m_arraySize; i++)
	{
		if(m_pArray[i] == element)
			break;
	}

	if(i != m_arraySize)
		erase(i);
}

//=============================================
// @brief Resizes the array
//
// @param n Index of element to return
//=============================================
template <typename T> inline T& CArray<T>::operator[] ( Uint32 n ) const
{
	assert(n < m_arraySize);
	return m_pArray[n];
}
#endif // CARRAY_INLINE_HPP