/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envmodel.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_model, CEnvModel);

//=============================================
// @brief
//
//=============================================
CEnvModel::CEnvModel( edict_t* pedict ):
	CAnimatingEntity(pedict),
	m_sequence(NO_STRING_VALUE),
	m_lightOrigin(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CEnvModel::~CEnvModel( void )
{
}
//=============================================
// @brief
//
//=============================================
bool CEnvModel::Spawn( void )
{
	// Remove immediately if it has no targetname
	// Means it's completely static
	if(m_pFields->targetname == NO_STRING_VALUE)
	{
		Util::RemoveEntity(this);
		return true;
	}

	// Takes care of setting the model
	if(!CAnimatingEntity::Spawn())
		return false;

	m_pState->movetype = MOVETYPE_NONE;
	m_pState->solid = SOLID_NOT;
	m_pState->flags |= FL_INITIALIZE;

	ResetSequenceInfo();
	InitBoneControllers();

	if(HasSpawnFlag(FL_START_INVISIBLE))
		m_pState->effects |= EF_NODRAW;

	if(HasSpawnFlag(FL_NO_LIGHTTRACES))
		m_pState->effects |= EF_NOELIGHTTRACE;

	if(HasSpawnFlag(FL_NO_VISCHECKS))
		m_pState->effects |= EF_NOVIS;

	return true;
}
//=============================================
// @brief
//
//=============================================
void CEnvModel::Precache( void )
{
	gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_pFields->modelname));
}

//=============================================
// @brief
//
//=============================================
bool CEnvModel::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "seqname"))
	{
		m_sequence = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "lightorigin"))
	{
		m_lightOrigin = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CAnimatingEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CEnvModel::InitEntity( void )
{
	if(m_sequence != NO_STRING_VALUE)
	{
		const Char* pstrsequencename = gd_engfuncs.pfnGetString(m_sequence);
		m_pState->sequence = FindSequence(pstrsequencename);
		if(m_pState->sequence == -1)
		{
			Util::EntityConPrintf(m_pEdict, "At %.0f %.0f %.0f - no such sequence '%s'.\n", m_pState->origin.x, m_pState->origin.y, m_pState->origin.z, pstrsequencename);
			m_pState->sequence = 0;
		}
	}

	if(m_lightOrigin != NO_STRING_VALUE)
	{
		edict_t* pedict = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_lightOrigin));
		if(pedict)
		{
			m_pState->lightorigin = pedict->state.origin;
			m_pState->effects |= EF_ALTLIGHTORIGIN;
		}
	}

	// Set bounding box
	SetSequenceBox(false);
}

//=============================================
// @brief
//
//=============================================
void CEnvModel::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(HasSpawnFlag(FL_START_INVISIBLE))
	{
		m_pState->effects &= ~EF_NODRAW;
		m_pState->spawnflags &= ~FL_START_INVISIBLE;
		return;
	}

	if(HasSpawnFlag(FL_CHANGE_SKIN))
	{
		// Advance the skin count
		m_pState->skin++;
	}
	else
	{
		if((Uint32)(m_pState->sequence+1) < GetSequenceNumber())
		{
			m_pState->sequence++;
			m_pState->frame = 0;
			ResetSequenceInfo();
			InitBoneControllers();
		}
	}
}
