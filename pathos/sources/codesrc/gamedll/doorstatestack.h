/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef DOORSTATESTACK_H
#define DOORSTATESTACK_H

/*
=======================
CEntityStateStack

=======================
*/
class CDoorEntityStateStack
{
private:
	struct stack_doorentstate_t
	{
		stack_doorentstate_t():
			pEntity(nullptr),
			solidity(SOLID_NOT),
			flags(0),
			togglestate(TS_NONE)
			{
			}

		CBaseEntity* pEntity;
		solid_t solidity;
		Uint64 flags;

		Vector origin;
		Vector angles;
		Int32 togglestate;
	};

public:
	CDoorEntityStateStack( void );
	~CDoorEntityStateStack( void );

public:
	void SaveEntity( CBaseEntity* pEntity );

private:
	CLinkedList<stack_doorentstate_t> m_savedEntitiesList;
};
#endif //DOORSTATESTACK_H