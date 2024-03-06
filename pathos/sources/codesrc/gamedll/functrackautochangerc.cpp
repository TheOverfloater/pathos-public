/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "functrackautochangerc.h"
#include "pathtrack.h"
#include "functracktrain.h"

// Default damage dealt by this entity
const Float CFuncTrackAutoChangeRC::DEFAULT_DAMAGE_DEALT = 1500;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_trackautochange_rc, CFuncTrackAutoChangeRC);

//=============================================
// @brief
//
//=============================================
CFuncTrackAutoChangeRC::CFuncTrackAutoChangeRC( edict_t* pedict ):
	CFuncTrackAutoChange(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncTrackAutoChangeRC::~CFuncTrackAutoChangeRC( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackAutoChangeRC::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!IsUseEnabled())
		return;

	CPathTrack* pTarget = nullptr;
	if(m_toggleState == TS_AT_TOP)
		pTarget = m_pTopTrack;
	else if(m_toggleState == TS_AT_BOTTOM)
		pTarget = m_pBottomTrack;

	m_code = EvaluateTrain(pTarget);
	if((m_code == TRAIN_FOLLOWING || m_code == TRAIN_SAFE) 
		&& m_toggleState != m_targetState)
	{
		DisableUse();

		if(m_toggleState == TS_AT_TOP)
			GoDown();
		else
			GoUp();
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackAutoChangeRC::CallBlocked( CBaseEntity* pBlocker )
{
	Float damageDealt = m_damageDealt > 0 ? m_damageDealt : DEFAULT_DAMAGE_DEALT;
	pBlocker->TakeDamage(this, pBlocker, damageDealt, DMG_CRUSH);
}