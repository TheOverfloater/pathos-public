/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envdecal.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_decal, CEnvDecal);

//=============================================
// @brief
//
//=============================================
CEnvDecal::CEnvDecal( edict_t* pedict ):
	CPointEntity(pedict),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CEnvDecal::~CEnvDecal( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvDecal::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvDecal, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CEnvDecal::Spawn( void )
{
	// env_decals with no targetnames, that are not
	// randomized are handled by the client side
	if(m_pFields->targetname == NO_STRING_VALUE 
		&& !HasSpawnFlag(FL_ENVDECAL_RANDOM))
	{
		Util::RemoveEntity(this);
		return true;
	}
	
	// Warn about empty decals
	if(m_pFields->message == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		Util::RemoveEntity(this);
		return true;
	}

	// Pick random decal from group at spawn
	if(HasSpawnFlag(FL_ENVDECAL_RANDOM))
	{
		if(m_pFields->targetname == NO_STRING_VALUE)
			m_isActive = true;

		const Char* pstrGroupName = gd_engfuncs.pfnGetString(m_pFields->message);
		decalgroupentry_t* pentry = gDecalList.GetRandom(pstrGroupName);
		if(!pentry)
		{
			Util::EntityConPrintf(m_pEdict, "decal at %.0f %.0f %.0f with unknown decal group '%s' specified.\n", 
				m_pState->origin.x, m_pState->origin.y, m_pState->origin.z, pstrGroupName);
			Util::RemoveEntity(this);
			return false;
		}

		// Set the new decal name
		m_pFields->message = gd_engfuncs.pfnAllocString(pentry->name.c_str());
	}

	return CPointEntity::Spawn();
}

//=============================================
// @brief
//
//=============================================
void CEnvDecal::Precache( void )
{
	gd_engfuncs.pfnPrecacheDecal(gd_engfuncs.pfnGetString(m_pFields->message));

	CPointEntity::Precache();
}

//=============================================
// @brief
//
//=============================================
Int32 CEnvDecal::GetEntityFlags( void ) 
{ 
	Int32 flags = CBaseEntity::GetEntityFlags();
	if(!HasSpawnFlag(FL_ENVDECAL_TRANSITION))
		flags &= ~FL_ENTITY_TRANSITION;

	return flags;
}

//=============================================
// @brief
//
//=============================================
void CEnvDecal::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(m_isActive)
		return;

	m_isActive = true;
	SendInitMessage(nullptr);
}

//=============================================
// @brief
//
//=============================================
void CEnvDecal::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(!m_isActive)
		return;

	Uint16 flags = FL_DECAL_PERSISTENT|FL_DECAL_SERVER;

	trace_t tr;
	for(Uint32 i = 0; i < 3; i++)
	{
		Uint32 j = 0;
		for(; j < 2; j++)
		{
			Vector traceStartOffset;
			traceStartOffset[i] = j == 0 ? 1 : -1;

			Vector traceEndOffset;
			traceEndOffset[i] = j == 0 ? -1 : 1;

			Vector traceStart = m_pState->origin + traceStartOffset;
			Vector traceEnd = m_pState->origin + traceEndOffset;

			Util::TraceLine(traceStart, traceEnd, true, false, nullptr, tr);
			if(!tr.startSolid() && !tr.allSolid() && !tr.noHit())
				break;
		}

		if(j != 2)
			break;
	}

	if(tr.startSolid() || tr.allSolid() || tr.noHit())
	{
		flags |= FL_DECAL_BOGUS;
	}
	else
	{
		flags |= FL_DECAL_HAS_NORMAL;
		if(tr.hitentity > WORLDSPAWN_ENTITY_INDEX)
			flags |= FL_DECAL_TIED_TO_ENTITY;
	}

	if(!HasSpawnFlag(FL_ENVDECAL_NO_VBM))
		flags |= FL_DECAL_VBM;

	if(HasSpawnFlag(FL_ENVDECAL_PERMISSIVE))
		flags |= FL_DECAL_NORMAL_PERMISSIVE;

	if(pPlayer)
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.creategenericdecal, nullptr, pPlayer->GetEdict());
	else
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.creategenericdecal, nullptr, nullptr);

	gd_engfuncs.pfnMsgWriteUint16(flags);
	for(Uint32 i = 0; i < 3; i++)
		gd_engfuncs.pfnMsgWriteFloat(m_pState->origin[i]);

	gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_pFields->message));

	if(flags & FL_DECAL_HAS_NORMAL)
	{
		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteSmallFloat(tr.plane.normal[i]*360.0f);
	}
			
	if(flags & FL_DECAL_TIED_TO_ENTITY)
		gd_engfuncs.pfnMsgWriteInt32(tr.hitentity);

	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief
//
//=============================================
void CEnvDecal::SpawnDecal( const Vector& origin, const Char* pstrdecalname )
{
	edict_t* pedict = gd_engfuncs.pfnCreateEntity("env_decal");
	if(!pedict)
		return;

	CEnvDecal* pdecal = reinterpret_cast<CEnvDecal*>(CBaseEntity::GetClass(pedict));
	if(!pdecal)
		return;

	pedict->fields.targetname = gd_engfuncs.pfnAllocString("deaddecal");
	pedict->fields.message = gd_engfuncs.pfnAllocString(pstrdecalname);
	pedict->state.origin = origin;
	pedict->state.spawnflags |= FL_ENVDECAL_TRANSITION;

	DispatchSpawn(pedict);

	pdecal->CallUse(nullptr, nullptr, USE_ON, 0);
}