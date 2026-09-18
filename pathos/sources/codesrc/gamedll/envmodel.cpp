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
	m_lightOrigin(NO_STRING_VALUE),
	m_vertexlightOffset(NO_POSITION),
	m_vertexlightVertexCount(0),
	m_vertexlightHash(NO_STRING_VALUE)
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
void CEnvModel::DeclareSaveFields( void )
{
	// Call base class to do it first
	CAnimatingEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvModel, m_sequence, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvModel, m_lightOrigin, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvModel, m_vertexlightOffset, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvModel, m_vertexlightVertexCount, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvModel, m_vertexlightHash, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CEnvModel::Spawn( void )
{
	// Takes care of setting the model
	if(!CAnimatingEntity::Spawn())
		return false;

	const cache_model_t* pModel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pModel)
		return false;

	// Remove immediately if it has no targetname and the solid spawnflag is not set
	// Means it's completely static
	if(m_pFields->targetname == NO_STRING_VALUE)
	{
		// Remove only if we do not have an MCD file attached,
		// as any prop with an .mcd needs to be engine-handled
		if(!(pModel->cacheflags & CACHE_FL_HAS_MCD) || HasSpawnFlag(FL_NOT_SOLID))
		{
			Util::RemoveEntity(this);
			return true;
		}
	}
	
	if((pModel->cacheflags & CACHE_FL_HAS_MCD) && !HasSpawnFlag(FL_NOT_SOLID))
	{
		m_pState->movetype = MOVETYPE_PUSH;
		m_pState->solid = SOLID_BBOX;
	}
	else
	{
		m_pState->movetype = MOVETYPE_NONE;
		m_pState->solid = SOLID_NOT;
	}

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
	else if(!qstrcmp(kv.keyname, "vlight_offset"))
	{
		m_vertexlightOffset = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "vlight_vertexcount"))
	{
		m_vertexlightVertexCount = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "vlight_styles"))
	{
		ReadLightStyles(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "vlight_hash"))
	{
		m_vertexlightHash = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CAnimatingEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvModel::ShouldOverrideKeyValue( const Char* pstrKeyValue )
{
	// We need special handling for vlight_styles
	if(!qstrcmp(pstrKeyValue, "vlight_styles"))
		return true;
	else
		return false;
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

//=============================================
// @brief
//
//=============================================
void CEnvModel::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(m_vertexlightOffset != NO_POSITION && m_vertexlightVertexCount > 0)
	{
		const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
		if(!pmodel)
		{
			Util::EntityConPrintf(m_pEdict, "Couldn't get model with index '%d' for entity.\n", m_pState->modelindex);
			m_vertexlightOffset = NO_POSITION;
			m_vertexlightVertexCount = 0;
			return;
		}

		const vbmcache_t* pvbmcache = pmodel->getVBMCache();
		const Char* pstrLocalHash = gd_engfuncs.pfnGetString(m_vertexlightHash);
		if(qstrcmp(pvbmcache->vertexhash, pstrLocalHash) != 0)
		{
			gd_engfuncs.pfnCon_Printf("[flags=onlyonce_game]%s - Vertex hash for model '%s' in BSP does not math with cache hash, model has been changed.\nBaked vertex lighting will be discarded for all entities using this model.\n", __FUNCTION__, pmodel->name.c_str());
			m_vertexlightOffset = NO_POSITION;
			m_vertexlightVertexCount = 0;
			return;
		}

		if (pPlayer)
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setupvertexlighting, nullptr, pPlayer->GetEdict());
		else
			gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.setupvertexlighting, nullptr, nullptr);

			gd_engfuncs.pfnMsgWriteInt16(m_pState->modelindex);
			gd_engfuncs.pfnMsgWriteInt32(m_pEdict->entindex);
			gd_engfuncs.pfnMsgWriteInt32(m_vertexlightOffset);
			gd_engfuncs.pfnMsgWriteInt32(m_vertexlightVertexCount);
			for(Uint32 i = 0; i < MAX_SURFACE_STYLES; i++)
				gd_engfuncs.pfnMsgWriteByte(m_pState->vlight_styles[i]);
		gd_engfuncs.pfnUserMessageEnd();
	}
}


//=============================================
// @brief
//
//=============================================
void CEnvModel::ReadLightStyles( const Char* pstrStyles )
{
	CString token;
	Uint32 index = 0;

	const Char* pstr = pstrStyles;
	while(pstr)
	{
		if(index >= MAX_ENTITY_STYLES)
		{
			Util::EntityConPrintf(m_pEdict, "%s - Too many lightstyles on env_model entity '%s'.\n", __FUNCTION__, token.c_str(), GetClassName());
			break;
		}

		pstr = Common::Parse(pstr, token, ";");
		if(pstr && (*pstr) == ';')
			pstr++;

		if(!Common::IsNumber(token))
		{
			Util::EntityConPrintf(m_pEdict, "%s - Numerical value expected for 'vlight_styles', got '%d' instead.\n", __FUNCTION__, token.c_str());
			continue;
		}
				
		Int32 value = SDL_atoi(token.c_str());
		if(value < 0 || value > 255)
		{
			Util::EntityConPrintf(m_pEdict, "%s - Invalid value '%d' specified for 'vlight_styles'.\n", __FUNCTION__, token.c_str());
			continue;
		}

		m_pState->vlight_styles[index] = value;
		index++;
	}
}
