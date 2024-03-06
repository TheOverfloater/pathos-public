/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "skilldata.h"

// Skill cvar settings file
const Char CSkillData::SKILLCVAR_FILE[] = "skill.cfg";

// Object declaration
CSkillData gSkillData;

// Holds skill cvar indexes
skillcvars_t g_skillcvars;

//=============================================
// @brief
//
//=============================================
CSkillData::CSkillData( void ):
	m_pSkillCvar(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CSkillData::~CSkillData( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CSkillData::Init( void )
{
	// Get skill cvar
	m_pSkillCvar = gd_engfuncs.pfnGetCVarPointer("sv_skill");
	if(!m_pSkillCvar)
	{
		gd_engfuncs.pfnCon_Printf("%s - Failed to get cvar 'sv_skill'.\n", __FUNCTION__);
		return false;
	}

	// NPC bullet damages
	g_skillcvars.skillNPC556Bullet = RegisterSkillCVar("sk_556_bullet");
	g_skillcvars.skillNPC762Bullet = RegisterSkillCVar("sk_762_bullet");
	g_skillcvars.skillNPCSig552Bullet = RegisterSkillCVar("sk_sig552_bullet");
	g_skillcvars.skillNPCBuckshotBullet = RegisterSkillCVar("sk_buckshot_bullet");
	g_skillcvars.skillNPC9MMBullet = RegisterSkillCVar("sk_9mm_bullet");

	// Throwable damages
	g_skillcvars.skillGrenadeDmg = RegisterSkillCVar("sk_grenade_dmg");
	g_skillcvars.skillGrenadeRadius = RegisterSkillCVar("sk_grenade_radius");

	// Replica
	g_skillcvars.skillReplicaHealth = RegisterSkillCVar("sk_replica_health");
	g_skillcvars.skillReplicaKickDmg = RegisterSkillCVar("sk_replica_kick");
	g_skillcvars.skillReplicaPellets = RegisterSkillCVar("sk_replica_pellets");
	g_skillcvars.skillReplicaGrenadeSpeed = RegisterSkillCVar("sk_replica_gspeed");
	g_skillcvars.skillReplicaSig552ConeSize = RegisterSkillCVar("sk_replica_sig552_cone_size");
	g_skillcvars.skillReplicaSig552ConeSizePrecise = RegisterSkillCVar("sk_replica_sig552_cone_size_precise");
	g_skillcvars.skillReplicaShotgunConeSize = RegisterSkillCVar("sk_replica_shotgun_cone_size");
	g_skillcvars.skillReplicaTRG42ConeSize = RegisterSkillCVar("sk_replica_trg42_cone_size");
	g_skillcvars.skillReplicaM249ConeSize = RegisterSkillCVar("sk_replica_m249_cone_size");
	g_skillcvars.skillReplicaReactionTime = RegisterSkillCVar("sk_replica_reaction_time");

	// Security guard
	g_skillcvars.skillSecurityHealth = RegisterSkillCVar("sk_security_health");
	g_skillcvars.skillSecurityGlockConeSize = RegisterSkillCVar("sk_security_glock_cone_size");
	g_skillcvars.skillSecurityDeagleConeSize = RegisterSkillCVar("sk_security_deagle_cone_size");
	g_skillcvars.skillSecurityTRG42ConeSize = RegisterSkillCVar("sk_security_trg42_cone_size");

	// Player weapons
	g_skillcvars.skillPlayerDmgKnife = RegisterSkillCVar("sk_plr_knife");
	g_skillcvars.skillPlayerDmg9MM = RegisterSkillCVar("sk_plr_9mm_bullet");
	g_skillcvars.skillPlayerDmgHandGrenade = RegisterSkillCVar("sk_plr_hand_grenade");
	g_skillcvars.skillPlayerHandGrenadeRadius = RegisterSkillCVar("sk_plr_grenade_radius");

	// Kevlar and healthkits
	g_skillcvars.skillKevlarAmount = RegisterSkillCVar("sk_kevlar");
	g_skillcvars.skillHealthkitHealAmount = RegisterSkillCVar("sk_healthkit");
	g_skillcvars.skillMaxHealthkits = RegisterSkillCVar("sk_max_healthkits");
	g_skillcvars.skillSodaCanHealAmount = RegisterSkillCVar("sk_sodacan");

	// Skill cvars for npcs
	g_skillcvars.skillNPCDmgMultiplierHead = RegisterSkillCVar("sk_npc_head");
	g_skillcvars.skillNPCDmgMultiplierChest = RegisterSkillCVar("sk_npc_chest");
	g_skillcvars.skillNPCDmgMultiplierStomach = RegisterSkillCVar("sk_npc_stomach");
	g_skillcvars.skillNPCDmgMultiplierArm = RegisterSkillCVar("sk_npc_arm");
	g_skillcvars.skillNPCDmgMultiplierLeg = RegisterSkillCVar("sk_npc_leg");
	g_skillcvars.skillNPCReactionTime = RegisterSkillCVar("sk_npc_reaction_time");

	// Damage multipliers for player
	g_skillcvars.skillPlayerDmgMultiplierHead = RegisterSkillCVar("sk_player_head");
	g_skillcvars.skillPlayerDmgMultiplierChest = RegisterSkillCVar("sk_player_chest");
	g_skillcvars.skillPlayerDmgMultiplierStomach = RegisterSkillCVar("sk_player_stomach");
	g_skillcvars.skillPlayerDmgMultiplierArm = RegisterSkillCVar("sk_player_arm");
	g_skillcvars.skillPlayerDmgMultiplierLeg = RegisterSkillCVar("sk_player_leg");

	// Player stamina
	g_skillcvars.skillStaminaSprintDrain = RegisterSkillCVar("sk_stamina_sprint_drain");
	g_skillcvars.skillStaminaJumpDrain = RegisterSkillCVar("sk_stamina_jump_drain");
	g_skillcvars.skillStaminaGainSpeed = RegisterSkillCVar("sk_stamina_gain_speed");

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CSkillData::InitGame( void )
{
	CString cmd;
	cmd << "exec " << SKILLCVAR_FILE << "\n";

	gd_engfuncs.pfnServerCommand(cmd.c_str());
	return true;
}

//=============================================
// @brief
//
//=============================================
Int32 CSkillData::RegisterSkillCVar( const Char* pstrcvarname )
{
	// Check if it's already present
	for(Uint32 i = 0; i < m_skillCVarsArray.size(); i++)
	{
		if(!qstrcmp(m_skillCVarsArray[i].cvarbasename, pstrcvarname))
		{
			gd_engfuncs.pfnCon_Printf("Skill cvar '%s' already registered.\n", pstrcvarname);
			return i;
		}
	}

	// Create a new one
	skillcvar_t newskill;
	newskill.cvarbasename = pstrcvarname;

	// Create "Easy" cvar
	CString cvarname;
	cvarname << pstrcvarname << "1";
	newskill.pskillcvareasy = gd_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, cvarname.c_str(), "0", "Skill cvar");

	// Create "normal" cvar
	cvarname.clear();
	cvarname << pstrcvarname << "2";
	newskill.pskillcvarnormal = gd_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, cvarname.c_str(), "0", "Skill cvar");

	// Create "hard" cvar
	cvarname.clear();
	cvarname << pstrcvarname << "3";
	newskill.pskillcvarhard = gd_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, cvarname.c_str(), "0", "Skill cvar");

	Int32 returnindex = m_skillCVarsArray.size();
	m_skillCVarsArray.push_back(newskill);

	return returnindex;
}

//=============================================
// @brief
//
//=============================================
Float CSkillData::GetSkillCVarSetting( Int32 skillcvarindex, force_skillcvar_t forceskill )
{
	if(skillcvarindex < 0 || skillcvarindex > (Int32)m_skillCVarsArray.size())
	{
		gd_engfuncs.pfnCon_Printf("%s - Bogus skill cvar index %d specified.\n", __FUNCTION__, skillcvarindex);
		return SKILLCVAR_DEFAULT_VALUE;
	}

	Int32 skilllevel;
	switch(forceskill)
	{
	case FORCE_SKILL_EASY:
		skilllevel = SKILL_EASY;
		break;
	case FORCE_SKILL_NORMAL:
		skilllevel = SKILL_NORMAL;
		break;
	case FORCE_SKILL_HARD:
		skilllevel = SKILL_HARD;
		break;
	case FORCE_SKILL_LESS_THAN_HARD:
		{
			skilllevel = (Int32)m_pSkillCvar->GetValue();
			if(skilllevel >= SKILL_HARD)
				skilllevel = SKILL_NORMAL;
		}
		break;
	case FORCE_SKILL_OFF:
	default:
		skilllevel = (Int32)m_pSkillCvar->GetValue();
		break;
	}

	Float value;
	skillcvar_t& skillcvar = m_skillCVarsArray[skillcvarindex];
	switch(skilllevel)
	{
	case SKILL_HARD:
		value = skillcvar.pskillcvarhard->GetValue();
		break;
	case SKILL_NORMAL:
		value = skillcvar.pskillcvarnormal->GetValue();
		break;
	case SKILL_EASY:
		value = skillcvar.pskillcvareasy->GetValue();
		break;
	default:
		gd_engfuncs.pfnCon_Printf("Invalid setting %d for '%s'.\n", (Int32)m_pSkillCvar->GetValue(), m_pSkillCvar->GetName());
		value = skillcvar.pskillcvareasy->GetValue();
		break;
	}

	if(value <= 0)
	{
		gd_engfuncs.pfnCon_EPrintf("%s - Cvar '%s' not defined.\n", __FUNCTION__, m_skillCVarsArray[skillcvarindex].cvarbasename.c_str());
		value = SKILLCVAR_DEFAULT_VALUE;
	}

	return value;
}

//=============================================
// @brief
//
//=============================================
skill_level_t CSkillData::GetSkillLevel( void ) const
{
	switch((Int32)m_pSkillCvar->GetValue())
	{
	case SKILL_HARD:
		return SKILL_HARD;
		break;
	case SKILL_NORMAL:
		return SKILL_NORMAL;
		break;
	case SKILL_EASY:
	default:
		return SKILL_EASY;
		break;
	}
}