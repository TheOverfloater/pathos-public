/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envrender.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_render, CEnvRender);

//=============================================
// @brief
//
//=============================================
CEnvRender::CEnvRender( edict_t* pedict ):
	CPointEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvRender::~CEnvRender( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CEnvRender::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE 
		|| m_pFields->target == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvRender::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	const Char* pstrTargetName = gd_engfuncs.pfnGetString(m_pFields->target);
	if(!pstrTargetName || !qstrlen(pstrTargetName))
		return;

	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, pstrTargetName);
		if(Util::IsNullEntity(pedict))
			break;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(!pEntity)
			continue;
		
		if(!HasSpawnFlag(FL_DONT_SET_RENDERFX))
			pEntity->SetRenderFx(m_pState->renderfx);

		if(!HasSpawnFlag(FL_DONT_SET_RENDERAMT))
			pEntity->SetRenderAmount(m_pState->renderamt);

		if(!HasSpawnFlag(FL_DONT_SET_RENDERMODE))
			pEntity->SetRenderMode((rendermode_t)m_pState->rendermode);

		if(!HasSpawnFlag(FL_DONT_SET_RENDERCOLOR))
			pEntity->SetRenderColor(m_pState->rendercolor);
	}
}