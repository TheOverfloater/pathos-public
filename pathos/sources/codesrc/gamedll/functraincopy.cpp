/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "functraincopy.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_train_copy, CFuncTrainCopy);

//=============================================
// @brief
//
//=============================================
CFuncTrainCopy::CFuncTrainCopy( edict_t* pedict ):
	CFuncTrain(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncTrainCopy::~CFuncTrainCopy( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncTrainCopy::TrainSetModel( void )
{
	// Don't set model just yet in spawn
	return true;
}

//=============================================
// @brief Tells if this is a brush model
//
//=============================================
bool CFuncTrainCopy::IsBrushModel( void ) const
{
	// This needs to be overridden, as trigger_toggletarget
	// can check this function before InitEntity is called
	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrainCopy::InitEntity( void ) 
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

	// Initialize rest with original entity
	CFuncTrain::InitEntity();
}