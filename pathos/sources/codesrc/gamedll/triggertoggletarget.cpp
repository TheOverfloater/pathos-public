/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggertoggletarget.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_toggletarget, CTriggerToggleTarget);

//=============================================
// @brief
//
//=============================================
CTriggerToggleTarget::CTriggerToggleTarget( edict_t* pedict ):
	CPointEntity(pedict),
	m_triggerMode(0)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerToggleTarget::~CTriggerToggleTarget( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerToggleTarget::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerToggleTarget, m_triggerMode, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerToggleTarget::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "triggermode"))
	{
		m_triggerMode = SDL_atoi(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerToggleTarget::Spawn( void )
{
	if(m_pFields->target == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CPointEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerToggleTarget::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	const Char* pstrTarget = gd_engfuncs.pfnGetString(m_pFields->target);
	edict_t* pedict = Util::FindEntityByTargetName(nullptr, pstrTarget);
	if(!pedict)
	{
		Util::EntityConPrintf(m_pEdict, "Target '%s' not found.\n", pstrTarget);
		return;
	}

	while(pedict)
	{
		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(pEntity->IsBrushModel())
		{
			switch(m_triggerMode)
			{
			case TRIGGERTOGGLE_DISABLE:
				pEntity->SetSolidity(SOLID_NOT);
				pEntity->SetEffectFlag(EF_NODRAW);
				break;
			case TRIGGERTOGGLE_ENABLE:
				pEntity->SetSolidity(SOLID_BSP);
				pEntity->RemoveEffectFlag(EF_NODRAW);
				break;
			}

			// Link to world
			pEntity->SetOrigin(pEntity->GetOrigin());
		}
		else
		{
			Util::EntityConPrintf(m_pEdict, "Target '%s' is not a brushmodel.\n", pstrTarget);
		}

		pedict = Util::FindEntityByTargetName(pedict, pstrTarget);
	}
}