/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "functrackchange.h"
#include "functracktrain.h"
#include "pathtrack.h"

// Blocked alarm sound
const Char CFuncTrackChange::BLOCKED_ALARM_SND[] = "buttons/button11.wav";

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_trackchange, CFuncTrackChange);

//=============================================
// @brief
//
//=============================================
CFuncTrackChange::CFuncTrackChange( edict_t* pedict ):
	CFuncPlatRot(pedict),
	m_pTopTrack(nullptr),
	m_pBottomTrack(nullptr),
	m_pTrain(nullptr),
	m_topTrackName(NO_STRING_VALUE),
	m_bottomTrackName(NO_STRING_VALUE),
	m_trainName(NO_STRING_VALUE),
	m_code(TRAIN_SAFE),
	m_targetState(0),
	m_isUsable(false)
{
}

//=============================================
// @brief
//
//=============================================
CFuncTrackChange::~CFuncTrackChange( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::DeclareSaveFields( void )
{
	// Call base class to do it first
	CFuncPlatRot::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD_GLOBAL(CFuncTrackChange, m_pTopTrack, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD_GLOBAL(CFuncTrackChange, m_pBottomTrack, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD_GLOBAL(CFuncTrackChange, m_pTrain, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD_GLOBAL(CFuncTrackChange, m_topTrackName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD_GLOBAL(CFuncTrackChange, m_bottomTrackName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD_GLOBAL(CFuncTrackChange, m_trainName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackChange, m_code, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackChange, m_targetState, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackChange, m_isUsable, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CFuncTrackChange::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "train"))
	{
		m_trainName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "toptrack"))
	{
		m_topTrackName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "bottomtrack"))
	{
		m_bottomTrackName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CFuncPlatRot::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::Precache( void )
{
	gd_engfuncs.pfnPrecacheSound(BLOCKED_ALARM_SND);

	CFuncPlatRot::Precache();
}

//=============================================
// @brief
//
//=============================================
bool CFuncTrackChange::Spawn( void )
{
	if(m_topTrackName == NO_STRING_VALUE)
	{
		Util::EntityConPrintf(m_pEdict, "No top path_track set for entity.\n");
		return false;
	}

	if(m_bottomTrackName == NO_STRING_VALUE)
	{
		Util::EntityConPrintf(m_pEdict, "No bottom path_track set for entity.\n");
		return false;
	}

	if(m_trainName == NO_STRING_VALUE)
	{
		Util::EntityConPrintf(m_pEdict, "No train name set for entity.\n");
		return false;
	}

	if(!CFuncPlatRot::Spawn())
		return false;

	// Do regular setup
	Setup();

	// Do this after setup of func_plat stuff
	if(HasSpawnFlag(FL_ROTATE_ONLY))
		m_position2.z = m_position1.z;

	SetupRotation();

	if(HasSpawnFlag(FL_START_AT_BOTTOM))
	{
		gd_engfuncs.pfnSetOrigin(m_pEdict, m_position2);
		m_toggleState = TS_AT_BOTTOM;
		m_pState->angles = m_startAngles;
		m_targetState = TS_AT_TOP;
	}
	else
	{
		gd_engfuncs.pfnSetOrigin(m_pEdict, m_position1);
		m_toggleState = TS_AT_TOP;
		m_pState->angles = m_endAngles;
		m_targetState = TS_AT_BOTTOM;
	}

	EnableUse();

	m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Do not allow use while moving
	if(m_toggleState != TS_AT_TOP && m_toggleState != TS_AT_BOTTOM)
		return;

	// If train is in the safe area, but not on this, then play alarm sound
	if(m_toggleState == TS_AT_TOP)
		m_code = EvaluateTrain(m_pTopTrack);
	else if(m_toggleState == TS_AT_BOTTOM)
		m_code = EvaluateTrain(m_pBottomTrack);
	else
		m_code = TRAIN_BLOCKING;

	if(m_code == TRAIN_BLOCKING)
	{
		Util::EmitEntitySound(this, BLOCKED_ALARM_SND, SND_CHAN_WEAPON);
		return;
	}

	DisableUse();

	if(m_toggleState == TS_AT_TOP)
		GoDown();
	else
		GoUp();
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::OnOverrideEntity( void )
{
	m_pState->flags |= FL_INITIALIZE;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::CallTouch( CBaseEntity* pOther )
{
	// Half-Life does nothing here, so I'll do the same for now
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::GoUp( void )
{
	if(m_code == TRAIN_BLOCKING)
		return;

	UpdateAutoTargets(TS_GOING_UP);

	if(HasSpawnFlag(FL_ROTATE_ONLY))
	{
		SetMoveDone(&CFuncPlat::CallHitTop);
		m_toggleState = TS_GOING_UP;
		AngularMove(m_endAngles, m_pState->speed);
	}
	else
	{
		CFuncPlat::GoUp();
		SetMoveDone(&CFuncPlat::CallHitTop);
		RotateMove(m_endAngles, m_pState->nextthink - m_pState->ltime);
	}

	if(m_code == TRAIN_FOLLOWING)
	{
		UpdateTrain(m_endAngles);
		m_pTrain->SetIsOnTrackChange(true);
		m_pTrain->SetPathTrack(nullptr);
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::GoDown( void )
{
	if(m_code == TRAIN_BLOCKING)
		return;

	UpdateAutoTargets(TS_GOING_DOWN);

	if(HasSpawnFlag(FL_ROTATE_ONLY))
	{
		SetMoveDone(&CFuncPlat::CallHitBottom);
		m_toggleState = TS_GOING_DOWN;
		AngularMove(m_startAngles, m_pState->speed);
	}
	else
	{
		CFuncPlat::GoDown();
		SetMoveDone(&CFuncPlat::CallHitBottom);
		RotateMove(m_startAngles, m_pState->nextthink - m_pState->ltime);
	}

	if(m_code == TRAIN_FOLLOWING)
	{
		UpdateTrain(m_startAngles);
		m_pTrain->SetIsOnTrackChange(true);
		m_pTrain->SetPathTrack(nullptr);
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::InitEntity( void )
{
	// Get top track entity
	const Char* pstrEntityName = gd_engfuncs.pfnGetString(m_topTrackName);
	m_pTopTrack = FindPathTrack(pstrEntityName);
	if(!m_pTopTrack)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find top path_track entity '%s'.\n", pstrEntityName);
		return;
	}

	// Get bottom track entity
	pstrEntityName = gd_engfuncs.pfnGetString(m_bottomTrackName);
	m_pBottomTrack = FindPathTrack(pstrEntityName);
	if(!m_pBottomTrack)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find bottom path_track entity '%s'.\n", pstrEntityName);
		return;
	}

	// Get func_tracktrain entity
	pstrEntityName = gd_engfuncs.pfnGetString(m_trainName);
	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, pstrEntityName);
		if(!pedict)
			break;

		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(pEntity && pEntity->IsFuncTrackTrainEntity())
		{
			m_pTrain = reinterpret_cast<CFuncTrackTrain*>(pEntity);
			break;
		}
	}

	if(!m_pTrain)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find func_tracktrain entity '%s'.\n", pstrEntityName);
		return;
	}

	Vector center = (m_pState->absmin+m_pState->absmax)*0.5;
	m_pBottomTrack = m_pBottomTrack->GetNearest(center);
	m_pTopTrack = m_pTopTrack->GetNearest(center);

	UpdateAutoTargets((togglestate_t)m_toggleState);
	SetThink(nullptr);

	if(m_toggleState == TS_GOING_DOWN || m_toggleState == TS_GOING_UP)
	{
		m_pTrain->RemoveFlags(FL_INITIALIZE);

		if(m_toggleState == TS_GOING_DOWN)
		{
			m_toggleState = TS_AT_TOP;
			GoDown();
		}
		else
		{
			m_toggleState = TS_AT_BOTTOM;
			GoUp();
		}
	}
}

//=============================================
// @brief
//
//=============================================
CFuncTrackChange::traincode_t CFuncTrackChange::EvaluateTrain( CPathTrack* pCurrent )
{
	// Do the stuff if we don't switch anything actually
	if(!pCurrent || !m_pTrain)
		return TRAIN_SAFE;

	CPathTrack* pTrainPath = m_pTrain->GetPath();
	CPathTrack* pCurrentPathPrevPath = pCurrent->GetPrevious();
	CPathTrack* pCurrentPathNextPath = pCurrent->GetNext();
	if(pTrainPath == pCurrent 
		|| pCurrentPathPrevPath && pTrainPath == pCurrentPathPrevPath
		|| pCurrentPathNextPath && pCurrentPathNextPath == pTrainPath)
	{
		if(m_pTrain->GetSpeed() != 0)
			return TRAIN_BLOCKING;

		Vector distance = m_pState->origin - m_pTrain->GetOrigin();
		Float length = distance.Length2D();
		Float trainlength = m_pTrain->GetLength();

		if(length < m_pTrain->GetLength())
			return TRAIN_FOLLOWING;
		else if(length > (150+trainlength))
			return TRAIN_SAFE;
		else
			return TRAIN_BLOCKING;
	}

	return TRAIN_SAFE;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::UpdateTrain( const Vector& destAngles )
{
	// Get delta
	Double time = m_pState->nextthink-m_pState->ltime;

	m_pTrain->SetVelocity(m_pState->velocity);
	m_pTrain->SetAngularVelocity(m_pState->avelocity);
	m_pTrain->SetNextThink(m_pTrain->GetLocalTime() + time, false);

	if(time <= 0)
		return;

	Vector offset = m_pTrain->GetOrigin() - m_pState->origin;
	Vector delta = destAngles - m_pState->angles;

	Vector forward, right, up;
	Math::AngleVectorsTranspose(delta, &forward, &right, &up);

	Vector local;
	local[0] = Math::DotProduct(offset, forward);
	local[1] = Math::DotProduct(offset, right);
	local[2] = Math::DotProduct(offset, up);

	local = local - offset;
	Vector trainVelocity = m_pState->velocity+(local*(1.0/time));
	m_pTrain->SetVelocity(trainVelocity);
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::HitBottom( void )
{
	CFuncPlatRot::HitBottom();
	if(m_code == TRAIN_FOLLOWING)
	{
		m_pTrain->SetPathTrack(m_pBottomTrack);
		m_pTrain->SetIsOnTrackChange(false);
		m_pTrain->RestorePath();
	}

	SetThink(nullptr);
	m_pState->nextthink = 0;

	UpdateAutoTargets((togglestate_t)m_toggleState);
	EnableUse();
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::HitTop( void )
{
	CFuncPlatRot::HitTop();
	if(m_code == TRAIN_FOLLOWING)
	{
		m_pTrain->SetPathTrack(m_pTopTrack);
		m_pTrain->SetIsOnTrackChange(false);
		m_pTrain->RestorePath();
	}

	SetThink(nullptr);
	m_pState->nextthink = 0;

	UpdateAutoTargets((togglestate_t)m_toggleState);
	EnableUse();
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::UpdateAutoTargets( togglestate_t state )
{
	if(!m_pTopTrack || !m_pBottomTrack)
		return;

	if(m_toggleState == TS_AT_TOP)
		m_pTopTrack->RemoveSpawnFlag(CPathTrack::FL_DISABLED);
	else
		m_pTopTrack->SetSpawnFlag(CPathTrack::FL_DISABLED);

	if(m_toggleState == TS_AT_BOTTOM)
		m_pBottomTrack->RemoveSpawnFlag(CPathTrack::FL_DISABLED);
	else
		m_pBottomTrack->SetSpawnFlag(CPathTrack::FL_DISABLED);
}

//=============================================
// @brief
//
//=============================================
bool CFuncTrackChange::IsTogglePlat( void ) 
{
	return true; 
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::DisableUse( void )
{
	m_isUsable = false;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackChange::EnableUse( void )
{
	m_isUsable = true;
}

//=============================================
// @brief
//
//=============================================
bool CFuncTrackChange::IsUseEnabled( void ) const
{
	return m_isUsable;
}

//=============================================
// @brief
//
//=============================================
CPathTrack* CFuncTrackChange::FindPathTrack( const Char* pstrTargetname )
{
	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, pstrTargetname);
		if(!pedict)
			break;

		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(pEntity && pEntity->IsPathTrackEntity())
			return reinterpret_cast<CPathTrack*>(pEntity);
	}

	return nullptr;
}