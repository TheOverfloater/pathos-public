/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "aiwandercontroller.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(ai_wandercontroller, CAIWanderController);

//=============================================
// @brief
//
//=============================================
CAIWanderController::CAIWanderController( edict_t* pedict ):
	CPointEntity(pedict),
	m_mode(WC_TOGGLE)
{
}

//=============================================
// @brief
//
//=============================================
CAIWanderController::~CAIWanderController( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CAIWanderController::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->target == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	return true;
}

//=============================================
// @brief Declares save-restore fields
//
//=============================================
void CAIWanderController::DeclareSaveFields( void )
{
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CAIWanderController, m_mode, EFIELD_INT32));
}

//=============================================
// @brief Manages keyvalues
//
//=============================================
bool CAIWanderController::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "mode"))
	{
		m_mode = SDL_atoi(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CAIWanderController::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	const Char* pstrTargetEntityName = gd_engfuncs.pfnGetString(m_pFields->target);

	edict_t* pTargetEdict = Util::FindEntityByTargetName(nullptr, pstrTargetEntityName);
	if(Util::IsNullEntity(pTargetEdict))
	{
		Util::EntityConPrintf(m_pEdict, "Could not find target '%s'.\n", pstrTargetEntityName);
		return;
	}

	CBaseEntity* pTargetEntity = CBaseEntity::GetClass(pTargetEdict);
	if(!pTargetEntity->IsNPC() || !pTargetEntity->IsWanderNPC())
	{
		Util::EntityConPrintf(m_pEdict, "Entity '%s' was not an NPC.\n", pstrTargetEntityName);
		return;
	}

	if( m_mode == WC_TOGGLE )
		pTargetEntity->SetWanderState( pTargetEntity->GetWanderState() ? false : true );
	else if( m_mode == WC_ENABLE )
		pTargetEntity->SetWanderState( true );
	else if( m_mode == WC_DISABLE )
		pTargetEntity->SetWanderState( false );
	else
		Util::EntityConPrintf(m_pEdict, "Invalid mode set: %d\n", m_mode);
}