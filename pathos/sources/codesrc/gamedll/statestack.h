/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef STATESTACK_H
#define STATESTACK_H

/*
=======================
CEntityStateStack

=======================
*/
class CEntityStateStack
{
private:
	struct stack_entstate_t
	{
		stack_entstate_t():
			pEntity(nullptr),
			solidity(SOLID_NOT),
			flags(0)
			{
			}

		CBaseEntity* pEntity;
		solid_t solidity;
		Uint64 flags;
	};

public:
	CEntityStateStack( void );
	~CEntityStateStack( void );

public:
	void SaveEntity( CBaseEntity* pEntity );

private:
	CLinkedList<stack_entstate_t> m_savedEntitiesList;
};
#endif //STATESTACK_H