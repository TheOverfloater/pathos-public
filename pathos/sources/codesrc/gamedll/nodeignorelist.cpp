/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "nodeignorelist.h"

//=============================================
// @brief
//
//=============================================
CNodeIgnoreList::CNodeIgnoreList( void )
{
}

//=============================================
// @brief
//
//=============================================
CNodeIgnoreList::~CNodeIgnoreList( void )
{
}

//=============================================
// @brief
//
//=============================================
void CNodeIgnoreList::AddNode( Int32 nodeIndex )
{
	for(Uint32 i = 0; i < m_ignoreNodesArray.size(); i++)
	{
		if(m_ignoreNodesArray[i] == nodeIndex)
			return;
	}

	m_ignoreNodesArray.push_back(nodeIndex);
}

//=============================================
// @brief
//
//=============================================
bool CNodeIgnoreList::IsNodeInList( Int32 nodeIndex ) const
{
	for(Uint32 i = 0; i < m_ignoreNodesArray.size(); i++)
	{
		if(m_ignoreNodesArray[i] == nodeIndex)
			return true;
	}

	return false;
}