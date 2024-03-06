/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "functrackautochange.h"
#include "pathtrack.h"
#include "functracktrain.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_trackautochange, CFuncTrackAutoChange);

//=============================================
// @brief
//
//=============================================
CFuncTrackAutoChange::CFuncTrackAutoChange( edict_t* pedict ):
	CFuncTrackChange(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncTrackAutoChange::~CFuncTrackAutoChange( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackAutoChange::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!IsUseEnabled())
		return;

	CPathTrack* pTarget = nullptr;
	if(m_toggleState == TS_AT_TOP)
		pTarget = m_pTopTrack;
	else if(m_toggleState == TS_AT_BOTTOM)
		pTarget = m_pBottomTrack;

	if(pActivator->IsFuncTrackTrainEntity())
	{
		m_code = EvaluateTrain(pTarget);
		if(m_code == TRAIN_FOLLOWING && m_toggleState != m_targetState)
		{
			DisableUse();

			if(m_toggleState == TS_AT_TOP)
				GoDown();
			else
				GoUp();
		}
	}
	else
	{
		if(pTarget)
			pTarget = pTarget->GetNext();

		if(pTarget && m_pTrain->GetPath() != pTarget 
			&& ShouldToggle(useMode, (m_targetState != TS_AT_TOP) ? true : false))
		{
			if(m_targetState == TS_AT_TOP)
				m_targetState = TS_AT_BOTTOM;
			else
				m_targetState = TS_AT_TOP;
		}
		
		UpdateAutoTargets((togglestate_t)m_targetState);
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackAutoChange::UpdateAutoTargets( togglestate_t state )
{
	if(!m_pTopTrack || !m_pBottomTrack)
		return;

	CPathTrack* pTarget = nullptr;
	CPathTrack* pNextTarget = nullptr;

	if(m_toggleState == TS_AT_TOP)
	{
		pTarget = m_pTopTrack->GetNext();
		pNextTarget = m_pBottomTrack->GetNext();
	}
	else
	{
		pTarget = m_pBottomTrack->GetNext();
		pNextTarget = m_pTopTrack->GetNext();
	}

	if(pTarget)
	{
		pTarget->RemoveSpawnFlag(CPathTrack::FL_DISABLED);
		if(m_code == TRAIN_FOLLOWING && m_pTrain && !m_pTrain->GetSpeed())
			m_pTrain->CallUse(this, this, USE_ON, 0);
	}

	if(pNextTarget)
		pNextTarget->SetSpawnFlag(CPathTrack::FL_DISABLED);
}