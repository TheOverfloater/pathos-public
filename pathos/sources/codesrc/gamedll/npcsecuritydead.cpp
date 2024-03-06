/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "npcsecuritydead.h"

// Pose sequence names for corpse
const Char* CNPCSecurityDead::NPC_CORPSE_POSES[NPC_NB_CORPSE_POSES] = 
{
	"lying_on_back",
	"lying_on_side",
	"lying_on_stomach",
	"lying_suicide"
};

// Model name for the npc
const Char CNPCSecurityDead::NPC_MODEL_NAME[] = "models/security.mdl";

// Bodygroup name for guns
const Char CNPCSecurityDead::NPC_BODYGROUP_WEAPONS_NAME[] = "weapons";
// Submodel name for blank weapon
const Char CNPCSecurityDead::NPC_SUBMODEL_WEAPON_BLANK_NAME[] = "blank";

LINK_ENTITY_TO_CLASS( npc_security_dead, CNPCSecurityDead );

//=============================================
// @brief Constructor
//
//=============================================
CNPCSecurityDead::CNPCSecurityDead( edict_t* pedict ):
	CBaseNPC(pedict),
	m_corpsePose(0)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CNPCSecurityDead::~CNPCSecurityDead( void )
{
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CNPCSecurityDead::Spawn( void )
{
	// Set modelname
	m_pFields->modelname = gd_engfuncs.pfnAllocString(NPC_MODEL_NAME);

	if(!CBaseNPC::Spawn())
		return false;

	if(m_corpsePose >= NPC_NB_CORPSE_POSES)
		m_corpsePose = Common::RandomLong(0, NPC_NB_CORPSE_POSES-1);

	m_pState->sequence = FindSequence(NPC_CORPSE_POSES[m_corpsePose]);
	if(m_pState->sequence == NO_SEQUENCE_VALUE)
		Util::EntityConPrintf(m_pEdict, "Corpse pose '%s' not found in '%s'.\n", NPC_CORPSE_POSES[m_corpsePose], NPC_MODEL_NAME);

	m_pState->health = NPC_CORPSE_HEALTH;

	// Call base class to init dead npc
	InitDeadNPC();

	Int32 weaponsBodyGroupIndex = GetBodyGroupIndexByName(NPC_BODYGROUP_WEAPONS_NAME);
	if(weaponsBodyGroupIndex != NO_POSITION)
	{
		Int32 weaponBlankSubmodelIndex = GetSubmodelIndexByName(weaponsBodyGroupIndex, NPC_SUBMODEL_WEAPON_BLANK_NAME);
		SetBodyGroup(weaponsBodyGroupIndex, weaponBlankSubmodelIndex);
	}

	return true;
}

//=============================================
// @brief Manages a keyvalue
//
//=============================================
bool CNPCSecurityDead::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "pose"))
	{
		m_corpsePose = SDL_atoi(kv.value);
		return true;
	}
	else
		return CBaseNPC::KeyValue(kv);
}

//=============================================
// @brief Returns the sound mask for the NPC
//
//=============================================
Uint64 CNPCSecurityDead::GetSoundMask( void )
{
	return AI_SOUND_NONE;
}
