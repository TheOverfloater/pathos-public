/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcbrushcopy.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_brushcopy, CFuncBrushCopy);

//=============================================
// @brief
//
//=============================================
CFuncBrushCopy::CFuncBrushCopy( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncBrushCopy::~CFuncBrushCopy( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncBrushCopy::Spawn( void )
{
	if(!HasSpawnFlag(FL_TAKE_ANGLES))
		m_pState->angles = ZERO_VECTOR;

	m_pState->movetype = MOVETYPE_PUSH;
	m_pState->solid = SOLID_BSP;
	m_pState->flags |= FL_POINTHULL_ONLY;

	if(m_pFields->targetname == NO_STRING_VALUE
		&& m_pFields->parent == NO_STRING_VALUE)
		m_pState->effects |= EF_STATICENTITY;

	if(m_pState->rendermode == RENDER_NORMAL
		|| (m_pState->rendermode & RENDERMODE_BITMASK) == RENDER_TRANSALPHA)
		m_pState->flags |= FL_WORLDBRUSH;

	m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief Tells if this is a brush model
//
//=============================================
bool CFuncBrushCopy::IsBrushModel( void ) const
{
	// This needs to be overridden, as trigger_toggletarget
	// can check this function before InitEntity is called
	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncBrushCopy::InitEntity( void ) 
{
	if(m_pFields->netname == NO_STRING_VALUE)
	{
		Util::EntityConPrintf(m_pEdict, "No copy target specified.\n");
		Util::RemoveEntity(this);
		return;
	}

	const Char* pstrCopyTargetName = gd_engfuncs.pfnGetString(m_pFields->netname);
	edict_t* pCopyEdict = Util::FindEntityByTargetName(nullptr, pstrCopyTargetName);
	if(!pCopyEdict)
	{
		Util::EntityConPrintf(m_pEdict, "Copy target '%s' couldn't be found.\n", pstrCopyTargetName);
		Util::RemoveEntity(this);
		return;
	}

	CBaseEntity* pCopyEntity = CBaseEntity::GetClass(pCopyEdict);
	if(!pCopyEntity)
	{
		Util::EntityConPrintf(m_pEdict, "Copy target '%s' is not a valid entity.\n", pstrCopyTargetName);
		Util::RemoveEntity(this);
		return;
	}

	if(!pCopyEntity->IsBrushModel())
	{
		Util::EntityConPrintf(m_pEdict, "Copy target '%s' is not a brush model entity.\n", pstrCopyTargetName);
		Util::RemoveEntity(this);
		return;
	}

	const Char* pstrModelName = pCopyEntity->GetModelName();
	m_pFields->modelname = gd_engfuncs.pfnAllocString(pCopyEntity->GetModelName());

	if(!SetModel(pstrModelName))
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't set copy target '%s''s model.\n", pstrCopyTargetName);
		Util::RemoveEntity(this);
		return;
	}

	if(pCopyEntity->GetFlags() & FL_POINTHULL_ONLY)
		SetFlags(FL_POINTHULL_ONLY);

	// Initialize rest with original entity
	CBaseEntity::InitEntity();
}