/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "npcgeneric.h"

// Special renderfx value for mirror only(for legacy support)
const Int32 CNPCGeneric::NPC_GENERIC_MIRROR_ONLY_FX_VALUE = 666;
// Yaw speed for npc
const Uint32 CNPCGeneric::NPC_YAW_SPEED = 180;
// Default health for NPC
const Float CNPCGeneric::NPC_DEFAULT_HEALTH = 10;

LINK_ENTITY_TO_CLASS( npc_generic, CNPCGeneric );

//=============================================
// @brief Constructor
//
//=============================================
CNPCGeneric::CNPCGeneric( edict_t* pedict ):
	CBaseNPC(pedict),
	m_hullType(NPC_GENERIC_HULL_HUMAN)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CNPCGeneric::~CNPCGeneric( void )
{
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CNPCGeneric::Spawn( void )
{
	if(!CBaseNPC::Spawn())
		return false;

	switch(m_hullType)
	{
	case NPC_GENERIC_HULL_PLAYER:
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_HULL_MIN, VEC_HULL_MAX);
		break;
	case NPC_GENERIC_HULL_HUMAN:
	default:
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
		break;
	}

	if(!HasSpawnFlag(FL_GENERICNPC_NOT_SOLID))
		m_pState->solid = SOLID_SLIDEBOX;
	else
		m_pState->solid = SOLID_NOT;

	m_pState->movetype = MOVETYPE_STEP;
	if(!m_pState->health)
		m_pState->health = NPC_DEFAULT_HEALTH;

	m_bloodColor = BLOOD_RED;
	m_fieldOfView = VIEW_FIELD_NARROW;
	m_npcState = NPC_STATE_NONE;

	// Set capabilities
	m_capabilityBits |= AI_CAP_GROUP_DOORS;

	// Legacy support(TODO: Get rid of this)
	if(m_pState->renderfx == NPC_GENERIC_MIRROR_ONLY_FX_VALUE)
		m_pState->renderfx = RenderFx_MirrorOnly;

	// Initialize the NPC
	InitNPC();

	return true;
}

//=============================================
// @brief Manages a keyvalue
//
//=============================================
bool CNPCGeneric::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "hulltype"))
	{
		Int32 value = (SDL_atoi(kv.value) == 1) ? true : false;
		switch(value)
		{
		case 1:
			m_hullType = NPC_GENERIC_HULL_PLAYER;
			break;
		case 0:
		default:
			m_hullType = NPC_GENERIC_HULL_PLAYER;
			break;

		}
		return true;
	}
	else
		return CBaseNPC::KeyValue(kv);
}

//=============================================
// @brief Sets the ideal yaw speed
//
//=============================================
void CNPCGeneric::SetYawSpeed( void )
{
	m_pState->yawspeed = NPC_YAW_SPEED;
}

//=============================================
// @brief Returns the sound mask for the NPC
//
//=============================================
Uint64 CNPCGeneric::GetSoundMask( void )
{
	return (AI_SOUND_WORLD|AI_SOUND_COMBAT|AI_SOUND_DANGER|AI_SOUND_PLAYER);
}

//=============================================
// @brief Returns the classification
//
//=============================================
Int32 CNPCGeneric::GetClassification( void ) const
{
	return CLASS_NONE;
}