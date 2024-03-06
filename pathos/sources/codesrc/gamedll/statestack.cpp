/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "statestack.h"

//=============================================
// @brief
//
//=============================================
CEntityStateStack::CEntityStateStack( void )
{
}

//=============================================
// @brief
//
//=============================================
CEntityStateStack::~CEntityStateStack( void )
{
	if(m_savedEntitiesList.empty())
		return;

	m_savedEntitiesList.begin();
	while(!m_savedEntitiesList.end())
	{
		stack_entstate_t& state = m_savedEntitiesList.get();
		state.pEntity->SetSolidity(state.solidity);
		state.pEntity->SetFlags(state.flags);

		m_savedEntitiesList.next();
	}
}

//=============================================
// @brief
//
//=============================================
void CEntityStateStack::SaveEntity( CBaseEntity* pEntity )
{
	// Make sure it's not present already
	m_savedEntitiesList.begin();
	while(!m_savedEntitiesList.end())
	{
		const stack_entstate_t& state = m_savedEntitiesList.get();
		if(state.pEntity == pEntity)
			return;

		m_savedEntitiesList.next();
	}

	stack_entstate_t newState;
	newState.pEntity = pEntity;
	newState.flags = pEntity->GetFlags();
	newState.solidity = pEntity->GetSolidity();

	pEntity->RemoveFlags(newState.flags);
	pEntity->SetSolidity(SOLID_NOT);

	m_savedEntitiesList.add(newState);
}