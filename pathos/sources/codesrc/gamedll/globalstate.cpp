/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "globalstate.h"

CGlobalState gGlobalStates;

//=============================================
// @brief
//
//=============================================
CGlobalState::CGlobalState( void )
{
}

//=============================================
// @brief
//
//=============================================
CGlobalState::~CGlobalState( void )
{
}

//=============================================
// @brief
//
//=============================================
void CGlobalState::Clear( void )
{
	if(!m_globalStatesList.empty())
		m_globalStatesList.clear();
}

//=============================================
// @brief
//
//=============================================
void CGlobalState::SetGlobalState( const Char* pstrglobalstate, globalstate_state_t state )
{
	if(!m_globalStatesList.empty())
	{
		m_globalStatesList.begin();
		while(!m_globalStatesList.end())
		{
			globalstate_t& globalstate = m_globalStatesList.get();
			if(!qstrcmp(globalstate.globalstate, pstrglobalstate))
			{
				if(state == GLOBAL_DELETED)
					m_globalStatesList.remove(m_globalStatesList.get_link());
				else
					globalstate.state = state;
				break;
			}

			m_globalStatesList.next();
		}
	}

	if(state == GLOBAL_DELETED)
		return;

	globalstate_t newstate;

	// Set the values
	newstate.globalstate = pstrglobalstate;
	newstate.mapname = g_pGameVars->levelname;
	newstate.state = state;

	m_globalStatesList.add(newstate);
}

//=============================================
// @brief
//
//=============================================
void CGlobalState::DeleteGlobalState( const Char* pstrglobalstate )
{
	if(m_globalStatesList.empty())
		return;

	m_globalStatesList.begin();
	while(!m_globalStatesList.end())
	{
		globalstate_t& globalstate = m_globalStatesList.get();
		if(!qstrcmp(globalstate.globalstate, pstrglobalstate))
		{
			m_globalStatesList.remove(m_globalStatesList.get_link());
			return;
		}

		m_globalStatesList.next();
	}
}

//=============================================
// @brief
//
//=============================================
const globalstate_t* CGlobalState::GetGlobalState( const Char* pstrglobalstate )
{
	if(m_globalStatesList.empty())
		return nullptr;

	m_globalStatesList.begin();
	while(!m_globalStatesList.end())
	{
		globalstate_t& globalstate = m_globalStatesList.get();
		if(!qstrcmp(globalstate.globalstate, pstrglobalstate))
			return &globalstate;

		m_globalStatesList.next();
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
globalstate_state_t CGlobalState::GetState( const Char* pstrglobalstate )
{
	const globalstate_t* pstate = GetGlobalState(pstrglobalstate);
	if(pstate)
		return pstate->state;
	else
		return GLOBAL_OFF;
}

//=============================================
// @brief
//
//=============================================
bool CGlobalState::IsGlobalStatePresent( const Char* pstrglobalstate )
{
	if(m_globalStatesList.empty())
		return false;

	m_globalStatesList.begin();
	while(!m_globalStatesList.end())
	{
		globalstate_t& globalstate = m_globalStatesList.get();
		if(!qstrcmp(globalstate.globalstate, pstrglobalstate))
			return true;

		m_globalStatesList.next();
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
void CGlobalState::UpdateGlobalStateMapName( const Char* pstrglobalstate )
{
	if(m_globalStatesList.empty())
		return;

	m_globalStatesList.begin();
	while(!m_globalStatesList.end())
	{
		globalstate_t& globalstate = m_globalStatesList.get();
		if(!qstrcmp(globalstate.globalstate, pstrglobalstate))
		{
			globalstate.mapname = g_pGameVars->levelname;
			return;
		}

		m_globalStatesList.next();
	}

	return;
}

//=============================================
// @brief
//
//=============================================
Uint32 CGlobalState::GetNbGlobalStates( void )
{
	return m_globalStatesList.size();
}

//=============================================
// @brief
//
//=============================================
void CGlobalState::SaveGlobalStates( void )
{
	if(m_globalStatesList.empty())
		return;

	Uint32 index = 0;
	m_globalStatesList.begin();
	while(!m_globalStatesList.end())
	{
		globalstate_t& globalstate = m_globalStatesList.get();
		gd_engfuncs.pfnSaveWriteGlobalState(index, globalstate.globalstate.c_str(), globalstate.mapname.c_str(), globalstate.state);
		index++;

		m_globalStatesList.next();
	}
}

//=============================================
// @brief
//
//=============================================
void CGlobalState::ReadGlobalStateData( const Char* pstrglobalname, const Char* pstrlevelname, globalstate_state_t state )
{
	m_globalStatesList.begin();
	while(!m_globalStatesList.end())
	{
		globalstate_t& globalstate = m_globalStatesList.get();
		if(!qstrcmp(globalstate.globalstate, pstrglobalname))
		{
			globalstate.state = state;
			return;
		}

		m_globalStatesList.next();
	}

	globalstate_t newstate;
	newstate.globalstate = pstrglobalname;
	newstate.mapname = pstrlevelname;
	newstate.state = state;

	m_globalStatesList.add(newstate);
}

//=============================================
// @brief
//
//=============================================
void CGlobalState::Dump( void )
{
	if(m_globalStatesList.empty())
	{
		gd_engfuncs.pfnCon_Printf("No global states.\n");
		return;
	}

	gd_engfuncs.pfnCon_Printf("%d global states:\n", (Int32)m_globalStatesList.size());
	
	Uint32 index = 0;
	m_globalStatesList.begin();
	while(!m_globalStatesList.end())
	{
		globalstate_t& state = m_globalStatesList.get();

		CString msg;
		msg << "\t" << (Int32)++index << " - Global state name: " << state.globalstate << ", level: " << state.mapname << ", state: ";
		
		switch(state.state)
		{
		case GLOBAL_OFF:
			msg << "off";
			break;
		case GLOBAL_ON:
			msg << "on";
			break;
		case GLOBAL_DEAD:
			msg << "dead";
			break;
		}

		gd_engfuncs.pfnCon_Printf("%s.\n", msg.c_str());
		m_globalStatesList.next();
	}
}

//=============================================
// @brief
//
//=============================================
Uint32 GetNbGlobalStates( void )
{
	return gGlobalStates.GetNbGlobalStates();
}

//=============================================
// @brief
//
//=============================================
void SaveGlobalStates( void )
{
	gGlobalStates.SaveGlobalStates();
}

//=============================================
// @brief
//
//=============================================
void ReadGlobalStateData( const Char* pstrglobalname, const Char* pstrlevelname, enum globalstate_state_t state )
{
	gGlobalStates.ReadGlobalStateData(pstrglobalname, pstrlevelname, state);
}