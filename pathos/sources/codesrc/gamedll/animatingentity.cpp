/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "animatingentity.h"
#include "vbmutils.h"
#include "activitymappings.h"
#include "ai_basenpc.h"

//=============================================
// @brief
//
//=============================================
CAnimatingEntity::CAnimatingEntity( edict_t* pedict ):
	CDelayEntity(pedict),
	m_frameRate(0),
	m_groundSpeed(0),
	m_lastEventCheckFrame(0),
	m_isSequenceFinished(false),
	m_isSequenceLooped(false)
{
}

//=============================================
// @brief
//
//=============================================
CAnimatingEntity::~CAnimatingEntity( void )
{
}

//=============================================
// @brief
//
//=============================================
void CAnimatingEntity::DeclareSaveFields( void )
{
	// Call base class first
	CDelayEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CAnimatingEntity, m_frameRate, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAnimatingEntity, m_groundSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAnimatingEntity, m_lastEventCheckFrame, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAnimatingEntity, m_isSequenceFinished, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CAnimatingEntity, m_isSequenceLooped, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CAnimatingEntity::Spawn( void )
{
	if(!CDelayEntity::Spawn())
		return false;

	if(m_pFields->modelname == NO_STRING_VALUE)
	{
		const Char* pstrclassname = gd_engfuncs.pfnGetString(m_pFields->classname);
		const Vector& origin = m_pState->origin;

		gd_engfuncs.pfnCon_Printf("Animated entity '%s' with no model set at %.0f %.0f %.0f.\n", pstrclassname, origin[0], origin[1], origin[2]);
		return false;
	}
	
	// Set the model
	const Char* pstrModelName = gd_engfuncs.pfnGetString(m_pFields->modelname);
	if(!SetModel(pstrModelName, false))
		return false;

	// Remove invalid body values
	if(m_pState->body < 0)
	{
		Util::EntityConDPrintf(m_pEdict, "Invalid body value %d on entity.\n", m_pState->body);
		m_pState->body = 0;
	}

	// Remove invalid body values
	if(m_pState->skin < 0)
	{
		Util::EntityConDPrintf(m_pEdict, "Invalid skin value %d on entity.\n", m_pState->body);
		m_pState->skin = 0;
	}

	//
	// These two are mostly caused by old levels, where "body" was used
	// to set the head on NPCs, before it got it's own keyvalue. This
	// fixes the decapitated head bug
	//

	return true;
}

//=============================================
// @brief
//
//=============================================
void CAnimatingEntity::Precache( void )
{
	// Call base class to precache
	CDelayEntity::Precache();

	if(m_pFields->modelname != NO_STRING_VALUE)
	{
		Int32 modelIndex = gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_pFields->modelname));
		if(modelIndex != NO_PRECACHE)
			PrecacheModelSounds(modelIndex);
	}
}

//=============================================
// @brief
//
//=============================================
void CAnimatingEntity::PrecacheModelSounds( Int32 modelIndex )
{
	if(modelIndex == NO_POSITION)
		return;

	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(modelIndex);
	if(pmodel && pmodel->type == MOD_VBM)
	{
		studiohdr_t* phdr = pmodel->getVBMCache()->pstudiohdr;
		for(Uint32 i = 0; i < phdr->numseq; i++)
			PrecacheSequenceSounds(modelIndex, i);
	}
}

//=============================================
// @brief
//
//=============================================
Float CAnimatingEntity::FrameAdvance( Double interval, Double* pAnimFinishTime )
{
	if(!interval)
	{
		interval = (g_pGameVars->time - m_pState->animtime);
		if(interval <= 0.001)
		{
			m_pState->animtime = g_pGameVars->time;
			return 0;
		}
	}

	if(!m_pState->animtime)
		interval = 0;

	m_pState->frame += interval*m_frameRate*m_pState->framerate;
	m_pState->animtime = g_pGameVars->time;

	if(m_pState->frame < 0 || m_pState->frame >= 256.0)
	{
		if(m_isSequenceLooped)
			m_pState->frame -= (Int32)(m_pState->frame / 256.0) * 256.0;
		else
			m_pState->frame = (m_pState->frame < 0) ? 0 : 256.0;

		m_isSequenceFinished = true;
	}

	if(pAnimFinishTime)
	{
		if(!m_isSequenceLooped)
			(*pAnimFinishTime) = (256.0 - m_pState->frame) / (m_frameRate*m_pState->framerate);
		else
			(*pAnimFinishTime) = 0;
	}

	return interval;
}

//=============================================
// @brief
//
//=============================================
void CAnimatingEntity::PrecacheSequenceSounds( Int32 sequenceIndex )
{
	PrecacheSequenceSounds(m_pState->modelindex, sequenceIndex);
}

//=============================================
// @brief
//
//=============================================
void CAnimatingEntity::PrecacheSequenceSounds( Int32 modelIndex, Int32 sequenceIndex )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(modelIndex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	if(sequenceIndex < 0 || sequenceIndex >= pstudiohdr->numseq)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid sequence index %d.\n", __FUNCTION__, sequenceIndex);
		return;
	}

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequenceIndex);
	for(Int32 i = 0; i < pseqdesc->numevents; i++)
	{
		const mstudioevent_t* pevent = pseqdesc->getEvent(pstudiohdr, i);
		if(pevent->event != CBaseNPC::NPC_AE_SOUND 
			&& pevent->event != CBaseNPC::NPC_AE_SENTENCE
			&& pevent->event != EVENT_CLIENTDLL_PLAYSOUND_A1)
			continue;

		gd_engfuncs.pfnPrecacheSound(pevent->options);
	}
}

//=============================================
// @brief
//
//=============================================
void CAnimatingEntity::PrecacheSequenceSounds( const Char* pstrSequenceName )
{
	Int32 sequenceIndex = FindSequence(pstrSequenceName);
	if(sequenceIndex == NO_SEQUENCE_VALUE)
	{
		gd_engfuncs.pfnCon_Printf("%s - No such sequence '%s'.\n", __FUNCTION__, pstrSequenceName);
		return;
	}

	PrecacheSequenceSounds(sequenceIndex);
}

//=============================================
// @brief
//
//=============================================
void CAnimatingEntity::ResetSequenceInfo( void )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	VBM_GetSequenceInfo(pmodel, m_pState->sequence, &m_frameRate, &m_groundSpeed);

	m_pState->animtime = g_pGameVars->time;
	m_pState->framerate = 1.0;
	m_pState->effects |= EF_SET_SEQTIME;
	
	m_isSequenceFinished = false;
	m_isSequenceLooped = (GetSequenceFlags() & STUDIO_LOOPING) ? true : false;
	m_lastEventCheckFrame = m_pState->frame;
}

//=============================================
// @brief
//
//=============================================
Uint32 CAnimatingEntity::GetSequenceNumber( void )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return 0;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	return VBM_GetSequenceNumber(pmodel);
}

//=============================================
// @brief
//
//=============================================
Int32 CAnimatingEntity::FindActivity( Int32 activity )
{
	if(activity == ACT_RESET)
		return 0;

	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return NO_SEQUENCE_VALUE;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return NO_SEQUENCE_VALUE;
	}

	return VBM_FindSequenceByActivity(pmodel, activity);
}

//=============================================
// @brief
//
//=============================================
Int32 CAnimatingEntity::FindHeaviestActivity( Int32 activity )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return NO_SEQUENCE_VALUE;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return NO_SEQUENCE_VALUE;
	}

	return VBM_FindSequenceWithHeaviestActivity(pmodel, activity);
}

//=============================================
// @brief
//
//=============================================
Int32 CAnimatingEntity::FindSequence( const char* pstrsequencename )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return NO_SEQUENCE_VALUE;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return NO_SEQUENCE_VALUE;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return NO_SEQUENCE_VALUE;
	}

	return VBM_FindSequence(pstudiohdr, pstrsequencename);
}

//=============================================
// @brief
//
//=============================================
Float CAnimatingEntity::GetSequenceTime( Int32 sequenceIndex )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return 0;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	if(sequenceIndex < 0 || sequenceIndex >= (Int32)GetSequenceNumber())
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid sequence index %d.\n", __FUNCTION__, sequenceIndex);
		return 0;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequenceIndex);
	return pseqdesc->numframes / ( pseqdesc->fps * m_frameRate );
}

//=============================================
// @brief
//
//=============================================
Int32 CAnimatingEntity::FindTransition( Uint32 endingsequence, Uint32 goalsequence, Int32* pdirection )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return NO_SEQUENCE_VALUE;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return NO_SEQUENCE_VALUE;
	}

	if(pdirection)
	{
		Int32 direction;
		Int32 sequence = VBM_FindTransition(pmodel, endingsequence, goalsequence, &direction);
		if(direction != -1)
			return -1;
		else
			return sequence;
	}

	return VBM_FindTransition(pmodel, endingsequence, goalsequence, pdirection);
}

//=============================================
// @brief
//
//=============================================
void CAnimatingEntity::InitBoneControllers( void )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	VBM_SetController(pmodel, 0, 0.0, m_pState->controllers, gd_engfuncs.pfnCon_Printf);
	VBM_SetController(pmodel, 1, 0.0, m_pState->controllers, gd_engfuncs.pfnCon_Printf);
	VBM_SetController(pmodel, 2, 0.0, m_pState->controllers, gd_engfuncs.pfnCon_Printf);
	VBM_SetController(pmodel, 3, 0.0, m_pState->controllers, gd_engfuncs.pfnCon_Printf);
}

//=============================================
// @brief
//
//=============================================
Float CAnimatingEntity::SetBoneController( Int32 controllerindex, Float value )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return 0;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	return VBM_SetController(pmodel, controllerindex, value, m_pState->controllers, gd_engfuncs.pfnCon_Printf);
}

//=============================================
// @brief
//
//=============================================
void CAnimatingEntity::ManageAnimationEvents( void )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	// Check for bounds
	if(m_pState->sequence < 0 || m_pState->sequence >= pstudiohdr->numseq)
	{
		gd_engfuncs.pfnCon_Printf("%s - Sequence %d is out of bounds for model '%s'.\n", __FUNCTION__, m_pState->sequence, pmodel->name.c_str());
		return;
	}

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(m_pState->sequence);
	if(!pseqdesc->numevents)
		return;

	Float frame = VBM_EstimateFrame(pseqdesc, (*m_pState), g_pGameVars->time);

	// Fixes first-frame event bug
	if(!frame)
		m_lastEventCheckFrame = -0.01f;

	if(frame == m_lastEventCheckFrame)
		return;

	bool looped = false;
	if(frame <= m_lastEventCheckFrame)
	{
		if(m_lastEventCheckFrame - frame > 0.5)
			looped = true;
		else
			return;
	}

	if(looped)
	{
		// Play animation events
		for(Int32 i = 0; i < pseqdesc->numevents; i++)
		{
			const mstudioevent_t* pevent = pseqdesc->getEvent(pstudiohdr, i);

			// Do not play clientside events
			if(pevent->event >= EVENT_CLIENT_START_INDEX)
				continue;

			if(pevent->frame <= m_lastEventCheckFrame)
				continue;

			HandleAnimationEvent(pevent);
		}

		m_lastEventCheckFrame = -0.01;
	}

	// Play animation events
	for(Int32 i = 0; i < pseqdesc->numevents; i++)
	{
		const mstudioevent_t* pevent = pseqdesc->getEvent(pstudiohdr, i);

		// Do not play clientside events
		if(pevent->event >= EVENT_CLIENT_START_INDEX)
			continue;

		if(pevent->frame > m_lastEventCheckFrame && pevent->frame <= frame)
			HandleAnimationEvent(pevent);
	}

	m_lastEventCheckFrame = frame;
}

//=============================================
// @brief
//
//=============================================
Int32 CAnimatingEntity::GetSequenceFlags( void )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return 0;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	return VBM_GetSequenceFlags(pmodel, m_pState->sequence);
}

//=============================================
// @brief
//
//=============================================
Uint32 CAnimatingEntity::GetNumFrames( void )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return 0;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	return VBM_GetNumSequenceFrames(pmodel, m_pState->sequence);
}

//=============================================
// @brief
//
//=============================================
Int32 CAnimatingEntity::GetModelFlags( void )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return 0;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	return pstudiohdr->flags;
}

//=============================================
// @brief
//
//=============================================
void CAnimatingEntity::GetAttachment( Uint32 attachmentindex, Vector& origin )
{
	gd_engfuncs.pfnGetAttachment(m_pEdict, attachmentindex, origin);
}

//=============================================
// @brief
//
//=============================================
bool CAnimatingEntity::GetBonePosition( Uint32 boneindex, Vector& origin )
{
	return gd_engfuncs.pfnGetBonePositionByIndex(m_pEdict, boneindex, origin);
}

//=============================================
// @brief
//
//=============================================
bool CAnimatingEntity::GetBonePosition( const Char* pstrbonename, Vector& origin )
{
	return gd_engfuncs.pfnGetBonePositionByName(m_pEdict, pstrbonename, origin);
}

//=============================================
// @brief Returns the bodygroup index by name
//
//=============================================
Int32 CAnimatingEntity::GetBodyGroupIndexByName( const Char* pstrBodygroupName )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return NO_POSITION;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return NO_POSITION;
	}

	return VBM_GetBodyGroupIndexByName(pmodel, pstrBodygroupName);
}

//=============================================
// @brief Returns the submodel index in a bodygroup by name
//
//=============================================
Int32 CAnimatingEntity::GetSubmodelIndexByName( Int32 bodygroupindex, const Char* pstrSubmodelName )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return NO_POSITION;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return NO_POSITION;
	}

	return VBM_GetSubmodelIndexByName(pmodel, bodygroupindex, pstrSubmodelName);
}

//=============================================
// @brief
//
//=============================================
Int32 CAnimatingEntity::GetBodyGroupValue( Int32 groupindex )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return 0;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	return VBM_GetBodyGroupValue(pmodel, groupindex, m_pState->body);
}

//=============================================
// @brief
//
//=============================================
void CAnimatingEntity::SetBodyGroup( Int32 groupindex, Int32 value )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	return VBM_SetBodyGroup(pmodel, groupindex, value, m_pState->body);
}

//=============================================
// @brief
//
//=============================================
bool CAnimatingEntity::SetSequenceBox( bool makeflat )
{
	Vector vMins, vMaxs;
	if(!GetSequenceBox(*m_pState, vMins, vMaxs, makeflat))
		return false;

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, vMins, vMaxs);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CAnimatingEntity::SetSequence( Int32 sequenceIndex )
{
	if(sequenceIndex < 0 || sequenceIndex >= (Int32)GetSequenceNumber())
	{
		Util::EntityConPrintf(m_pEdict, "Invalid sequence index %d.\n", __FUNCTION__, sequenceIndex);
		return false;
	}

	m_pState->sequence = sequenceIndex;
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CAnimatingEntity::SetSequence( const Char* pstrSequenceName )
{
	Int32 sequenceIndex = FindSequence(pstrSequenceName);
	if(sequenceIndex == NO_SEQUENCE_VALUE)
	{
		Util::EntityConPrintf(m_pEdict, "No such sequence named '%s'.\n", pstrSequenceName);
		return false;
	}

	m_pState->sequence = sequenceIndex;
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CAnimatingEntity::IsSequenceLooped( const Char* pstrSequenceName )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return false;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return false;
	}

	Int32 sequence = FindSequence(pstrSequenceName);
	if(sequence == NO_SEQUENCE_VALUE)
		return false;

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return false;
	}

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequence);
	return (pseqdesc->flags & STUDIO_LOOPING) ? true : false;
}

//=============================================
// @brief
//
//=============================================
Float CAnimatingEntity::GetSequenceTime( const Char* pstrsequencename )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return 0;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	Int32 sequence = FindSequence(pstrsequencename);
	if(sequence == NO_SEQUENCE_VALUE)
		return 0;

	return VBM_GetSequenceTime(pstudiohdr, sequence, 1.0);
}

//=============================================
// @brief
//
//=============================================
bool CAnimatingEntity::GetSequenceBox( entity_state_t& state, Vector& mins, Vector& maxs, bool makeflat )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(state.modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return false;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return false;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return false;
	}

	Int32 sequence = state.sequence;
	if (sequence >=  pstudiohdr->numseq) 
		sequence = 0;

	Vector seqmins, seqmaxs;
	if(!ExtractBoundingBox(state, sequence, seqmins, seqmaxs))
		return false;

	Vector vTemp;
	static Vector vBounds[8];
	for (Uint32 i = 0; i < 8; i++)
	{
		if ( i & 1 ) vTemp[0] = seqmins[0];
		else vTemp[0] = seqmaxs[0];
		if ( i & 2 ) vTemp[1] = seqmins[1];
		else vTemp[1] = seqmaxs[1];
		if ( i & 4 ) vTemp[2] = seqmins[2];
		else vTemp[2] = seqmaxs[2];
		Math::VectorCopy( vTemp, vBounds[i] );
	}

	Vector angles = state.angles;
	angles[PITCH] *= -1;

	Float rotationmatrix[3][4];
	Math::AngleMatrix(angles, rotationmatrix);

	for (Uint32 i = 0; i < 8; i++ )
	{
		Math::VectorCopy(vBounds[i], vTemp);
		Math::VectorRotate(vTemp, rotationmatrix, vBounds[i]);
	}

	// Set the bounding box
	Vector vMins = NULL_MINS;
	Vector vMaxs = NULL_MAXS;
	for(Uint32 i = 0; i < 8; i++)
	{
		// Mins
		if(vBounds[i][0] < vMins[0]) vMins[0] = vBounds[i][0];
		if(vBounds[i][1] < vMins[1]) vMins[1] = vBounds[i][1];
		if(vBounds[i][2] < vMins[2]) vMins[2] = vBounds[i][2];

		// Maxs
		if(vBounds[i][0] > vMaxs[0]) vMaxs[0] = vBounds[i][0];
		if(vBounds[i][1] > vMaxs[1]) vMaxs[1] = vBounds[i][1];
		if(vBounds[i][2] > vMaxs[2]) vMaxs[2] = vBounds[i][2];
	}

	// Make sure stuff like barnacles work fine
	if(pstudiohdr->numbonecontrollers)
	{
		for (Int32 j = 0; j < pstudiohdr->numbonecontrollers; j++)
		{
			const mstudiobonecontroller_t *pbonecontroller = pstudiohdr->getBoneController(j);
			if(!(pbonecontroller->type & STUDIO_Y))
				continue;

			if(pbonecontroller->type & STUDIO_RLOOP)
				continue;

			Uint32 iIndex = pbonecontroller->index;
			Float flValue = state.controllers[iIndex]/255.0;
		
			if (flValue < 0) flValue = 0;
			if (flValue > 1) flValue = 1;
			
			vMins[2] += (1.0-flValue)*pbonecontroller->start+flValue*pbonecontroller->end;
		}
	}

	if(makeflat)
	{
		vMins[2] = 0;
		vMaxs[2] = 1;
	}

	mins = vMins;
	maxs = vMaxs;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CAnimatingEntity::ExtractBoundingBox( entity_state_t& state, Uint32 sequence, Vector& mins, Vector& maxs )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(state.modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return false;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return false;
	}

	return VBM_GetSequenceBoundingBox(pmodel, state.sequence, mins, maxs);
}

//=============================================
// @brief Sets sequence blending
//
//=============================================
Float CAnimatingEntity::SetBlending( Int32 blenderIndex, Float value )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return false;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return false;
	}

	return VBM_SetBlending(pmodel, blenderIndex, m_pState->sequence, value, m_pState->blending);
}

//=============================================
// @brief Returns the number of sequence blenders
//
//=============================================
Uint32 CAnimatingEntity::GetNbBlenders( void )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return false;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return false;
	}

	return VBM_GetNumSequenceBlenders(pmodel, m_pState->sequence);
}