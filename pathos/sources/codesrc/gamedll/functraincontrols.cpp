/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "functraincontrols.h"
#include "functracktrain.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_traincontrols, CFuncTrainControls);

//=============================================
// @brief
//
//=============================================
CFuncTrainControls::CFuncTrainControls( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncTrainControls::~CFuncTrainControls( void )
{
}
//=============================================
// @brief
//
//=============================================
bool CFuncTrainControls::Spawn( void )
{
	if(m_pFields->target == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CBaseEntity::Spawn())
		return false;

	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_NONE;

	if(!SetModel(m_pFields->modelname))
		return false;

	m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrainControls::InitEntity( void )
{
	CFuncTrackTrain* pTrackTrain = nullptr;

	// Get func_tracktrain entity
	const Char* pstrEntityName = gd_engfuncs.pfnGetString(m_pFields->target);
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
			pTrackTrain = reinterpret_cast<CFuncTrackTrain*>(pEntity);
			break;
		}
	}

	if(!pTrackTrain)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find func_tracktrain entity '%s'.\n", pstrEntityName);
		return;
	}
	else
	{
		pTrackTrain->SetControls(this);
	}
	
	Util::RemoveEntity(this);
}
