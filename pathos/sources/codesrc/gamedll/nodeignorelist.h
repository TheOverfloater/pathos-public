/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef NODEIGNORELIST_H
#define NODEIGNORELIST_H

/*
=======================
CNodeIgnoreList

=======================
*/
class CNodeIgnoreList
{
public:
	CNodeIgnoreList( void );
	~CNodeIgnoreList( void );

public:
	void AddNode( Int32 nodeIndex );
	bool IsNodeInList( Int32 nodeIndex ) const;

private:
	CArray<Int32> m_ignoreNodesArray;
};
#endif //NODEIGNORELIST_H