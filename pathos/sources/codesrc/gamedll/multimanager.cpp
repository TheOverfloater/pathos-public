/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "multimanager.h"

// TRUE if we're in InitializeEntities
extern bool g_bInInitializeEntities;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(multi_manager, CMultiManager);

//=============================================
// @brief
//
//=============================================
CMultiManager::CMultiManager( edict_t* pedict ):
	CPointEntity(pedict),
	m_isClone(false),
	m_isRunning(false),
	m_startTime(0),
	m_delay(0),
	m_nbTargets(0),
	m_currentIndex(0)
{
	for(Uint32 i = 0; i < MAX_MULTIMANAGER_TARGETS; i++)
	{
		m_targetNamesArray[i] = NO_STRING_VALUE;
		m_targetDelaysArray[i] = 0;
	}
}

//=============================================
// @brief
//
//=============================================
CMultiManager::~CMultiManager( void )
{
}

//=============================================
// @brief
//
//=============================================
void CMultiManager::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CMultiManager, m_isClone, EFIELD_BOOLEAN));	
	DeclareSaveField(DEFINE_DATA_FIELD(CMultiManager, m_isRunning, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CMultiManager, m_startTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CMultiManager, m_delay, EFIELD_FLOAT));

	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CMultiManager, m_targetNamesArray, EFIELD_STRING, MAX_MULTIMANAGER_TARGETS));
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CMultiManager, m_targetDelaysArray, EFIELD_FLOAT, MAX_MULTIMANAGER_TARGETS));
	DeclareSaveField(DEFINE_DATA_FIELD(CMultiManager, m_nbTargets, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CMultiManager, m_currentIndex, EFIELD_UINT32));

	DeclareSaveField(DEFINE_DATA_FIELD(CMultiManager, m_activator, EFIELD_EHANDLE));
}

//=============================================
// @brief
//
//=============================================
bool CMultiManager::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "wait"))
	{
		m_delay = SDL_atof(kv.value);
		return true;
	}
	else
	{
		if(m_nbTargets >= MAX_MULTIMANAGER_TARGETS)
		{
			Util::EntityConPrintf(m_pEdict, "Exceeded MAX_MULTIMANAGER_TARGETS.\n");
			return true;
		}

		CString targetname(kv.keyname);
		Uint32 hashpos = targetname.find(0, "#");
		if(hashpos != -1)
			targetname.erase(hashpos, targetname.length()-hashpos);

		m_targetNamesArray[m_nbTargets] = gd_engfuncs.pfnAllocString(targetname.c_str());
		m_targetDelaysArray[m_nbTargets] = SDL_atof(kv.value);
		m_nbTargets++;

		return true;
	}
}

//=============================================
// @brief
//
//=============================================
bool CMultiManager::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	SortTargets();

	return true;
}

//=============================================
// @brief
//
//=============================================
void CMultiManager::SortTargets( void )
{
	// Do bubble sort
	bool isSwapped = true;
	while(isSwapped)
	{
		isSwapped = false;
		for(Uint32 i = 1; i < m_nbTargets; i++)
		{
			if(m_targetDelaysArray[i] < m_targetDelaysArray[i-1])
			{
				string_t moveName = m_targetNamesArray[i];
				Float moveDelay = m_targetDelaysArray[i];

				// Move it up one position
				m_targetNamesArray[i] = m_targetNamesArray[i-1];
				m_targetDelaysArray[i] = m_targetDelaysArray[i-1];

				// Move previous down one position
				m_targetNamesArray[i-1] = moveName;
				m_targetDelaysArray[i-1] = moveDelay;
				isSwapped = true;
			}
		}
	}
}

//=============================================
// @brief
//
//=============================================
Uint32 CMultiManager::GetNbTargets( void ) const
{
	return m_nbTargets;
}

//=============================================
// @brief
//
//=============================================
string_t CMultiManager::GetTargetNameByIndex( Uint32 index ) const
{
	if(index >= m_nbTargets)
		return NO_STRING_VALUE;

	return m_targetNamesArray[index];
}

//=============================================
// @brief
//
//=============================================
Float CMultiManager::GetTargetDelayByIndex( Uint32 index ) const
{
	if(index >= m_nbTargets)
		return 0;

	return m_targetDelaysArray[index];
}

//=============================================
// @brief
//
//=============================================
Float CMultiManager::GetDelay( void )
{
	return m_delay;
}

//=============================================
// @brief
//
//=============================================
void CMultiManager::AddTarget( const Char* pstrTargetName, Float delay )
{
	m_targetNamesArray[m_nbTargets] = gd_engfuncs.pfnAllocString(pstrTargetName);
	m_targetDelaysArray[m_nbTargets] = delay;
	m_nbTargets++;
}

//=============================================
// @brief
//
//=============================================
bool CMultiManager::IsClone( void ) const
{
	return m_isClone;
}

//=============================================
// @brief
//
//=============================================
bool CMultiManager::ShouldClone( void ) const
{
	if(m_isClone)
		return false;

	return (m_pState->spawnflags & FL_MULTITHREAD) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CMultiManager::SetIsClone( bool isclone )
{
	m_isClone = isclone;
}

//=============================================
// @brief
//
//=============================================
CMultiManager* CMultiManager::CloneManager( void )
{
	CMultiManager* pManager = reinterpret_cast<CMultiManager*>(CBaseEntity::CreateEntity("multi_manager", nullptr));
	if(!pManager)
		return nullptr;

	pManager->CopyEdictData(this);
	pManager->SetIsClone(true);

	if(!pManager->Spawn())
	{
		Util::RemoveEntity(pManager);
		return nullptr;
	}

	for(Uint32 i = 0; i < m_nbTargets; i++)
		pManager->AddTarget(gd_engfuncs.pfnGetString(m_targetNamesArray[i]), m_targetDelaysArray[i]);

	return pManager;
}

//=============================================
// @brief
//
//=============================================
void CMultiManager::TriggerThink( void )
{
	Double time = g_pGameVars->time - m_startTime;
	while(m_currentIndex < m_nbTargets && m_targetDelaysArray[m_currentIndex] <= time)
	{
		Util::FireTargets(gd_engfuncs.pfnGetString(m_targetNamesArray[m_currentIndex]), m_activator, this, USE_TOGGLE, 0);
		m_currentIndex++;
	}

	if(m_currentIndex >= m_nbTargets)
	{
		SetThink(nullptr);

		if(IsClone())
		{
			SetThink(&CBaseEntity::RemoveThink);
			m_pState->nextthink = g_pGameVars->time + 0.1;
			return;
		}

		m_isRunning = false;
		m_startTime = 0;
	}
	else
	{
		// Think only when next one is up
		m_pState->nextthink = m_startTime + m_targetDelaysArray[m_currentIndex];
	}
}

//=============================================
// @brief
//
//=============================================
void CMultiManager::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Do not allow re-triggers while running
	if(m_isRunning)
		return;

	if(ShouldClone())
	{
		CMultiManager* pCloneManager = CloneManager();
		if(pCloneManager)
		{
			pCloneManager->CallUse(pActivator, pCaller, useMode, value);
			return;
		}
	}

	// Save activator
	m_isRunning = true;
	m_activator = pActivator;
	m_startTime = g_pGameVars->time;
	m_currentIndex = 0;

	// Set think fn
	SetThink(&CMultiManager::TriggerThink);

	if(g_bInInitializeEntities)
	{
		// Think immediately if we're called from InitializeEntities, so
		// things like nightstage switches, etc, happen instantly and we
		// don't get a lag several frame lag in the changes
		TriggerThink();
	}
	else
	{
		// Otherwise think normally, as triggering instantly can fuck up 
		// looped multimanagers due to the entities not having time to 
		// reset themselves outside the trigger loops
		m_pState->nextthink = g_pGameVars->time;
	}
}