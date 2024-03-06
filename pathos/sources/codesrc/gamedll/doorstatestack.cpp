/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "doorstatestack.h"

//=============================================
// @brief
//
//=============================================
CDoorEntityStateStack::CDoorEntityStateStack( void )
{
}

//=============================================
// @brief
//
//=============================================
CDoorEntityStateStack::~CDoorEntityStateStack( void )
{
	if(m_savedEntitiesList.empty())
		return;

	m_savedEntitiesList.begin();
	while(!m_savedEntitiesList.end())
	{
		stack_doorentstate_t& state = m_savedEntitiesList.get();
		state.pEntity->SetSolidity(state.solidity);
		state.pEntity->SetFlags(state.flags);

		// Set toggle state before origin/angles, as origin might
		// be different than what SetToggleState sets
		state.pEntity->SetToggleState((togglestate_t)state.togglestate, false);
		state.pEntity->SetAngles(state.angles);
		state.pEntity->SetOrigin(state.origin);
		
		m_savedEntitiesList.next();
	}
}

//=============================================
// @brief
//
//=============================================
void CDoorEntityStateStack::SaveEntity( CBaseEntity* pEntity )
{
	// Make sure it's not present already
	m_savedEntitiesList.begin();
	while(!m_savedEntitiesList.end())
	{
		const stack_doorentstate_t& state = m_savedEntitiesList.get();
		if(state.pEntity == pEntity)
			return;

		m_savedEntitiesList.next();
	}

	stack_doorentstate_t newState;
	newState.pEntity = pEntity;
	newState.flags = pEntity->GetFlags();
	newState.solidity = pEntity->GetSolidity();
	newState.origin = pEntity->GetOrigin();
	newState.angles = pEntity->GetAngles();
	newState.togglestate = pEntity->GetToggleState();

	m_savedEntitiesList.add(newState);
}