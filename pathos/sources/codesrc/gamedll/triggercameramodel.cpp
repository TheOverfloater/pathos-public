/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggercameramodel.h"
#include "player.h"
#include "vcontroller_shared.h"

// TRUE if we are blocking saving
bool CTriggerCameraModel::g_bIsBlockingSaving = false;

// v_sequences model file path
const Char CTriggerCameraModel::V_SEQUENCES_MODELNAME[] = "models/v_sequences.mdl";

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_cameramodel, CTriggerCameraModel);

//=============================================
// @brief
//
//=============================================
CTriggerCameraModel::CTriggerCameraModel( edict_t* pedict ):
	CAnimatingEntity(pedict),
	m_loopTarget(NO_STRING_VALUE),
	m_entrySequence(NO_STRING_VALUE),
	m_loopSequence(NO_STRING_VALUE),
	m_restSequence(NO_STRING_VALUE),
	m_triggerSequence(NO_STRING_VALUE),
	m_blendTime(0),
	m_triggerState(CAMMODEL_STATE_OFF),
	m_fov(0),
	m_pPlayer(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerCameraModel::~CTriggerCameraModel( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerCameraModel::DeclareSaveFields( void )
{
	// Call base class to do it first
	CAnimatingEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCameraModel, m_loopTarget, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCameraModel, m_entrySequence, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCameraModel, m_loopSequence, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCameraModel, m_restSequence, EFIELD_STRING)); 
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCameraModel, m_triggerSequence, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCameraModel, m_blendTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCameraModel, m_triggerState, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCameraModel, m_fov, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCameraModel, m_pPlayer, EFIELD_ENTPOINTER));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerCameraModel::KeyValue( const keyvalue_t& kv )
{
	if (!qstrcmp(kv.keyname, "triggersequence"))
	{
		m_triggerSequence = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if (!qstrcmp(kv.keyname, "restsequence"))
	{
		m_restSequence = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if (!qstrcmp(kv.keyname, "loopsequence"))
	{
		m_loopSequence = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if (!qstrcmp(kv.keyname, "entrysequence"))
	{
		m_entrySequence = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if (!qstrcmp(kv.keyname, "looptarget"))
	{
		m_loopTarget = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if (!qstrcmp(kv.keyname, "blendtime"))
	{
		m_blendTime = SDL_atof(kv.value);
		return true;
	}
	else if (!qstrcmp(kv.keyname, "fov"))
	{
		m_fov = SDL_atoi(kv.value);
		return true;
	}
	else
		return CAnimatingEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerCameraModel::ShouldOverrideKeyValue( const Char* pstrKeyValue )
{
	// Fov is handled by this entity specially
	if(!qstrcmp(pstrKeyValue, "fov"))
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
bool CTriggerCameraModel::Spawn( void )
{
	// Manages precaching and setting model
	if(!CAnimatingEntity::Spawn())
		return false;

	// Make sure sequences are valid
	if(!CheckSequence(m_entrySequence)
		|| !CheckSequence(m_loopSequence)
		|| !CheckSequence(m_restSequence)
		|| !CheckSequence(m_triggerSequence))
		return false;

	m_pState->movetype = MOVETYPE_NOCLIP;
	m_pState->effects |= EF_VIEWONLY|EF_NOVIS|EF_NOLERP|EF_NOINTERP;

	if(!m_blendTime)
		m_blendTime = 1;

	m_pState->sequence = FindSequence(gd_engfuncs.pfnGetString(m_restSequence));
	ResetSequenceInfo();

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCameraModel::SendInitMessage( const CBaseEntity* pPlayer )
{
	// Tell client
	if(m_triggerState != CAMMODEL_STATE_OFF)
	{
		m_pPlayer->SetCameraEntity(this);
		m_pPlayer->SetViewModel(nullptr);

		gd_engfuncs.pfnUserMessageBegin( MSG_ONE, g_usermsgs.viewcontroller, nullptr, m_pPlayer->GetEdict() );
			gd_engfuncs.pfnMsgWriteByte( CONTROLLER_MODE_VIEWCAM );
			gd_engfuncs.pfnMsgWriteInt16( GetEntityIndex() );
			gd_engfuncs.pfnMsgWriteByte( m_fov );
			gd_engfuncs.pfnMsgWriteByte( 0 );
			gd_engfuncs.pfnMsgWriteSmallFloat( m_blendTime );
			gd_engfuncs.pfnMsgWriteByte( 0 );
		gd_engfuncs.pfnUserMessageEnd();

		if (m_triggerState == CAMMODEL_STATE_SECOND)
		{
			SetThink(&CTriggerCameraModel::FinishThink);
			m_pState->nextthink = g_pGameVars->time + GetSequenceTime(gd_engfuncs.pfnGetString(m_triggerSequence));
		}
	}
}

//=============================================
// @brief
//
//=============================================
bool CTriggerCameraModel::CheckSequence( string_t& sequence )
{
	if(sequence == NO_STRING_VALUE)
		return true;

	const char* pstrSequence = gd_engfuncs.pfnGetString(sequence);
	if(FindSequence(gd_engfuncs.pfnGetString(sequence)) == NO_SEQUENCE_VALUE)
	{
		Util::EntityConPrintf(m_pEdict, "Sequence '%s' couldn't be found.\n", pstrSequence);
		sequence = NO_STRING_VALUE;
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCameraModel::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Either use activator, or assume it's local player
	CBaseEntity* pentity;
	if(pActivator && pActivator->IsPlayer())
		pentity = pActivator;
	else
		pentity = Util::GetHostPlayer();

	m_pPlayer = reinterpret_cast<CPlayerEntity*>(pentity);
	if(m_pPlayer->IsFlashlightOn())
		m_pPlayer->TurnFlashlightOff();

	m_pPlayer->SetPunchAmount(ZERO_VECTOR);
	m_pPlayer->SetPunchAngle(ZERO_VECTOR);

	if(!HasSpawnFlag(FL_DONT_UNDUCK))
		m_pPlayer->UnDuckPlayer();

	if(m_triggerState == CAMMODEL_STATE_OFF)
	{
		// Do not allow saving the game
		g_bIsBlockingSaving = true;

		m_pPlayer->SetControlEnable(false);
		m_pPlayer->SetCameraEntity(this);

		if(HasSpawnFlag(FL_FOLLOW_PLAYER))
		{
			Vector traceStart = m_pPlayer->GetOrigin();
			if(m_pPlayer->GetFlags() & FL_DUCKING)
				traceStart[2] += VEC_HULL_MAX[2];

			Vector traceEnd = traceStart - Vector(0, 0, 1024);

			// Trace to the floor
			trace_t tr;
			Util::TraceHull(traceStart, traceEnd, true, false, HULL_HUMAN, m_pEdict, tr);
			if(tr.startSolid() || tr.allSolid() || tr.noHit())
			{
				Util::EntityConPrintf(m_pEdict, "Invalid position.\n");
				return;
			}
			
			// Set endpos as origin
			Vector resultOrigin = tr.endpos + Vector(0, 0, VEC_HULL_MIN[2]);
			gd_engfuncs.pfnSetOrigin(m_pEdict, resultOrigin);

			if(!HasSpawnFlag(FL_KEEP_ANGLES))
			{
				Vector playerAngles = m_pPlayer->GetAngles();
				if(!HasSpawnFlag(FL_ADJUST_VIEW))
				{
					m_pPlayer->SetViewAngles(playerAngles);
					m_pPlayer->SetVelocity(ZERO_VECTOR);
				}

				m_pState->angles = Vector(0, playerAngles[YAW], 0);
			}
		}

		gd_engfuncs.pfnUserMessageBegin( MSG_ONE, g_usermsgs.viewcontroller, nullptr, m_pPlayer->GetEdict() );
			gd_engfuncs.pfnMsgWriteByte( CONTROLLER_MODE_VIEWCAM );
			gd_engfuncs.pfnMsgWriteInt16( GetEntityIndex() );
			gd_engfuncs.pfnMsgWriteByte( m_fov );
			gd_engfuncs.pfnMsgWriteByte( HasSpawnFlag(FL_ADJUST_VIEW) ? 1 : 0 );
			gd_engfuncs.pfnMsgWriteSmallFloat( m_blendTime );
			gd_engfuncs.pfnMsgWriteByte( HasSpawnFlag(FL_ALWAYS_DRAW) ? 1 : 0 );
		gd_engfuncs.pfnUserMessageEnd();

		m_pState->sequence = FindSequence(gd_engfuncs.pfnGetString(m_restSequence)); 
		ResetSequenceInfo();
		m_pState->frame = 0;

		if(!HasSpawnFlag(FL_INTERPOLATE))
			m_pState->effects |= (EF_NOLERP|EF_NOINTERP);

		if(m_restSequence != NO_STRING_VALUE && m_restSequence == m_triggerSequence && m_entrySequence == NO_STRING_VALUE)
		{
			SetThink(&CTriggerCameraModel::FinishThink);
			m_pState->nextthink = g_pGameVars->time + GetSequenceTime(gd_engfuncs.pfnGetString(m_triggerSequence));
			m_triggerState = CAMMODEL_STATE_SECOND;
		}
		else
		{
			m_triggerState = CAMMODEL_STATE_FIRST;
		}
	}
	else if (m_triggerState == CAMMODEL_STATE_FIRST || m_triggerState == CAMMODEL_STATE_LOOP)
	{
		if(HasSpawnFlag(FL_FOLLOW_PLAYER))
		{
			if(HasSpawnFlag(FL_ADJUST_VIEW))
			{
				m_pPlayer->SetViewAngles(m_pState->angles);
				m_pPlayer->SetVelocity(ZERO_VECTOR);
			}
		}

		if(m_loopSequence != NO_STRING_VALUE && m_entrySequence != NO_STRING_VALUE 
			&& m_triggerState != CAMMODEL_STATE_LOOP)
		{
			const Char* pstrSeqName = gd_engfuncs.pfnGetString(m_entrySequence);
			m_pState->sequence = FindSequence(pstrSeqName);
			m_pState->frame = 0;

			ResetSequenceInfo();

			// Never interpolate change from rest sequence to new seq
			m_pState->effects |= EF_NOLERP;

			SetThink(&CTriggerCameraModel::EnterLoopThink);
			m_pState->nextthink = g_pGameVars->time + GetSequenceTime(pstrSeqName);
			m_triggerState = CAMMODEL_STATE_LOOP;
		}
		else if(m_triggerSequence != NO_STRING_VALUE)
		{
			// Set sequence to play
			const Char* pstrSeqName = gd_engfuncs.pfnGetString(m_triggerSequence);
			m_pState->sequence = FindSequence(pstrSeqName);
			m_pState->frame = 0;

			ResetSequenceInfo();

			m_pState->effects &= ~EF_NODRAW;
			m_triggerState = CAMMODEL_STATE_SECOND;

			SetThink(&CTriggerCameraModel::FinishThink);
			m_pState->nextthink = g_pGameVars->time + GetSequenceTime(pstrSeqName);

			gd_engfuncs.pfnUserMessageBegin( MSG_ONE, g_usermsgs.viewcontroller, nullptr, m_pPlayer->GetEdict() );
				gd_engfuncs.pfnMsgWriteByte( CONTROLLER_MODE_VIEWCAM );
				gd_engfuncs.pfnMsgWriteInt16( GetEntityIndex() );
				gd_engfuncs.pfnMsgWriteByte( m_fov );
				gd_engfuncs.pfnMsgWriteByte( 0 );
				gd_engfuncs.pfnMsgWriteSmallFloat( 0 );
				gd_engfuncs.pfnMsgWriteByte( 0 );
			gd_engfuncs.pfnUserMessageEnd();
		}
		else
		{
			m_triggerState = CAMMODEL_STATE_SECOND;
			SetThink(&CTriggerCameraModel::FinishThink);
			m_pState->nextthink = g_pGameVars->time + 0.1;
		}
	}
	else if (m_triggerState == CAMMODEL_STATE_SECOND)
	{
		ResetThink();
	}
}

//=============================================
// @brief
//
//=============================================
CTriggerCameraModel* CTriggerCameraModel::CreateCameraModel( const CBaseEntity* pOwner, Float bendTime, const Char* pstrRestSequence, const Char* pstrEntrySequence, const Char* pstrLoopSequence, const Char* pstrTriggerSequence )
{
	edict_t* pedict = gd_engfuncs.pfnCreateEntity("trigger_cameramodel");
	if(!pedict)
	{
		gd_engfuncs.pfnCon_Printf("%s - Failed to create entity 'trigger_cameramodel'.\n", __FUNCTION__);
		return nullptr;
	}

	pedict->state.spawnflags |= FL_FOLLOW_PLAYER|FL_ADJUST_VIEW;
	pedict->fields.modelname = gd_engfuncs.pfnAllocString(V_SEQUENCES_MODELNAME);

	if(pOwner)
	{
		pedict->state.owner = pOwner->GetEntityIndex();
		pedict->state.origin = pOwner->GetOrigin();
		gd_engfuncs.pfnSetOrigin(pedict, pedict->state.origin);
	}

	CTriggerCameraModel* pCameraModel = reinterpret_cast<CTriggerCameraModel*>(CBaseEntity::GetClass(pedict));

	// Set sequences
	pCameraModel->SetBlendTime(bendTime);
	pCameraModel->SetRestSequence(pstrRestSequence);
	pCameraModel->SetEntrySequence(pstrEntrySequence);
	pCameraModel->SetLoopSequence(pstrLoopSequence);
	pCameraModel->SetTriggerSequence(pstrTriggerSequence);

	DispatchSpawn(pedict);
	return pCameraModel;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCameraModel::FinishThink( void )
{
	if(m_pFields->target != NO_STRING_VALUE)
		Util::FireTargets(gd_engfuncs.pfnGetString(m_pFields->target), this, this, USE_TOGGLE, 0);

	if(!HasSpawnFlag(FL_LEAVE_ON_TRIGGER))
	{
		SetThink(&CTriggerCameraModel::ResetThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
}

//=============================================
// @brief
//
//=============================================
void CTriggerCameraModel::ResetThink( void )
{
	m_pPlayer->SetControlEnable(true);
	m_pPlayer->SetCameraEntity(nullptr);

	if(HasSpawnFlag(FL_FOLLOW_PLAYER))
	{
		m_pState->angles[ROLL] = 0;
		m_pState->angles[PITCH] = 0;

		m_pPlayer->SetAngles(m_pState->angles);
		m_pPlayer->SetViewAngles(m_pState->angles);
	}

	gd_engfuncs.pfnUserMessageBegin( MSG_ONE, g_usermsgs.viewcontroller, nullptr, m_pPlayer->GetEdict() );
		gd_engfuncs.pfnMsgWriteByte( CONTROLLER_MODE_OFF );
	gd_engfuncs.pfnUserMessageEnd();

	m_pState->effects |= EF_NOLERP;

	m_triggerState = CAMMODEL_STATE_OFF;

	if(m_pState->owner != NO_ENTITY_INDEX)
	{
		CBaseEntity *pEntOwner = GetOwner();
		pEntOwner->AdvanceState();
	}

	// Enable saving
	g_bIsBlockingSaving = false;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCameraModel::EnterLoopThink( void )
{
	if(m_loopTarget != NO_STRING_VALUE)
		Util::FireTargets(gd_engfuncs.pfnGetString(m_loopTarget), this, this, USE_TOGGLE, 0);

	m_pState->sequence = FindSequence(gd_engfuncs.pfnGetString(m_loopSequence));
	m_pState->frame = 0;
	m_pState->effects &= ~EF_NOLERP;
	ResetSequenceInfo();

	SetThink(nullptr);
	m_pState->nextthink = 0;

	if(m_pState->owner != NO_ENTITY_INDEX)
	{
		CBaseEntity *pEntOwner = GetOwner();
		pEntOwner->AdvanceState();
	}
}

//=============================================
// @brief
//
//=============================================
void CTriggerCameraModel::SetEntrySequence( const Char* pstrseqname )
{
	if(pstrseqname && qstrlen(pstrseqname))
		m_entrySequence = gd_engfuncs.pfnAllocString(pstrseqname);
	else
		m_entrySequence = NO_STRING_VALUE;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCameraModel::SetLoopSequence( const Char* pstrseqname )
{
	if(pstrseqname && qstrlen(pstrseqname))
		m_loopSequence = gd_engfuncs.pfnAllocString(pstrseqname);
	else
		m_entrySequence = NO_STRING_VALUE;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCameraModel::SetRestSequence( const Char* pstrseqname )
{
	if(pstrseqname && qstrlen(pstrseqname))
		m_restSequence = gd_engfuncs.pfnAllocString(pstrseqname);
	else
		m_entrySequence = NO_STRING_VALUE;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCameraModel::SetTriggerSequence( const Char* pstrseqname )
{
	if(pstrseqname && qstrlen(pstrseqname))
		m_triggerSequence = gd_engfuncs.pfnAllocString(pstrseqname);
	else
		m_entrySequence = NO_STRING_VALUE;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCameraModel::SetBlendTime( Float time )
{
	m_blendTime = time;
}

//=============================================
// @brief
//
//=============================================
bool CTriggerCameraModel::IsBlockingSaving( void )
{
	return g_bIsBlockingSaving;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCameraModel::SetBlockingSaving( bool blocking )
{
	g_bIsBlockingSaving = blocking;
}