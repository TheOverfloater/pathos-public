/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "ai_nodegraph.h"
#include "npctesthull.h"

LINK_ENTITY_TO_CLASS(npc_testhull, CNPCTestHull);

//=============================================
// @brief
//
//=============================================
CNPCTestHull::CNPCTestHull( edict_t* pedict ):
	CBaseNPC(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CNPCTestHull::~CNPCTestHull( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CNPCTestHull::Spawn( void )
{
	// Set null modelname
	m_pFields->modelname = gd_engfuncs.pfnAllocString(ERROR_MODEL_FILENAME);

	if(!CBaseNPC::Spawn())
		return false;

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	m_pState->movetype = MOVETYPE_STEP;
	m_pState->solid = SOLID_SLIDEBOX;
	m_pState->effects |= EF_NODRAW;

	m_pState->health = 1;
	m_pState->yawspeed = 90;
	m_pState->flags |= FL_NPC_CLIP;

	m_valuesParsed = true;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CNPCTestHull::SetYawSpeed( void )
{
	m_pState->yawspeed = 90;
}

//=============================================
// @brief
//
//=============================================
Uint64 CNPCTestHull::GetSoundMask( void )
{
	return AI_SOUND_NONE;
}

//=============================================
// @brief Makes the entity non-solid
//
//=============================================
void CNPCTestHull::MakeInert( void )
{
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->solid = SOLID_NOT;
}

//=============================================
// @brief Makes the entity solid
//
//=============================================
void CNPCTestHull::MakeSolid( void )
{
	m_pState->movetype = MOVETYPE_STEP;
	m_pState->solid = SOLID_SLIDEBOX;
}