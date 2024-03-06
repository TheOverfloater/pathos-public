/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SKILLDATA_H
#define SKILLDATA_H

#include "cvar.h"

// Undefined skill cvar value
static const Int32 SKILLCVAR_UNDEFINED = -1;
// Default skill cvar value if null
static const Float SKILLCVAR_DEFAULT_VALUE = 1;

struct skillcvars_t
{
	skillcvars_t():
		skillNPC556Bullet(SKILLCVAR_UNDEFINED),
		skillNPC762Bullet(SKILLCVAR_UNDEFINED),
		skillNPCSig552Bullet(SKILLCVAR_UNDEFINED),
		skillNPCBuckshotBullet(SKILLCVAR_UNDEFINED),
		skillNPC9MMBullet(SKILLCVAR_UNDEFINED),
		skillGrenadeDmg(SKILLCVAR_UNDEFINED),
		skillGrenadeRadius(SKILLCVAR_UNDEFINED),
		skillReplicaHealth(SKILLCVAR_UNDEFINED),
		skillReplicaKickDmg(SKILLCVAR_UNDEFINED),
		skillReplicaPellets(SKILLCVAR_UNDEFINED),
		skillReplicaGrenadeSpeed(SKILLCVAR_UNDEFINED),
		skillReplicaSig552ConeSize(SKILLCVAR_UNDEFINED),
		skillReplicaSig552ConeSizePrecise(SKILLCVAR_UNDEFINED),
		skillReplicaShotgunConeSize(SKILLCVAR_UNDEFINED),
		skillReplicaTRG42ConeSize(SKILLCVAR_UNDEFINED),
		skillReplicaM249ConeSize(SKILLCVAR_UNDEFINED),
		skillReplicaReactionTime(SKILLCVAR_UNDEFINED),
		skillSecurityHealth(SKILLCVAR_UNDEFINED),
		skillSecurityGlockConeSize(SKILLCVAR_UNDEFINED),
		skillSecurityDeagleConeSize(SKILLCVAR_UNDEFINED),
		skillSecurityTRG42ConeSize(SKILLCVAR_UNDEFINED),
		skillPlayerDmgKnife(SKILLCVAR_UNDEFINED),
		skillPlayerDmg9MM(SKILLCVAR_UNDEFINED),
		skillPlayerDmgHandGrenade(SKILLCVAR_UNDEFINED),
		skillPlayerHandGrenadeRadius(SKILLCVAR_UNDEFINED),
		skillKevlarAmount(SKILLCVAR_UNDEFINED),
		skillHealthkitHealAmount(SKILLCVAR_UNDEFINED),
		skillMaxHealthkits(SKILLCVAR_UNDEFINED),
		skillSodaCanHealAmount(SKILLCVAR_UNDEFINED),
		skillNPCDmgMultiplierHead(SKILLCVAR_UNDEFINED),
		skillNPCDmgMultiplierChest(SKILLCVAR_UNDEFINED),
		skillNPCDmgMultiplierStomach(SKILLCVAR_UNDEFINED),
		skillNPCDmgMultiplierArm(SKILLCVAR_UNDEFINED),
		skillNPCDmgMultiplierLeg(SKILLCVAR_UNDEFINED),
		skillNPCReactionTime(SKILLCVAR_UNDEFINED),
		skillPlayerDmgMultiplierHead(SKILLCVAR_UNDEFINED),
		skillPlayerDmgMultiplierChest(SKILLCVAR_UNDEFINED),
		skillPlayerDmgMultiplierStomach(SKILLCVAR_UNDEFINED),
		skillPlayerDmgMultiplierArm(SKILLCVAR_UNDEFINED),
		skillPlayerDmgMultiplierLeg(SKILLCVAR_UNDEFINED),
		skillStaminaSprintDrain(SKILLCVAR_UNDEFINED),
		skillStaminaJumpDrain(SKILLCVAR_UNDEFINED),
		skillStaminaGainSpeed(SKILLCVAR_UNDEFINED)
	{
	}

	Int32 skillNPC556Bullet;
	Int32 skillNPC762Bullet;
	Int32 skillNPCSig552Bullet;
	Int32 skillNPCBuckshotBullet;
	Int32 skillNPC9MMBullet;

	Int32 skillGrenadeDmg;
	Int32 skillGrenadeRadius;

	Int32 skillReplicaHealth;
	Int32 skillReplicaKickDmg;
	Int32 skillReplicaPellets;
	Int32 skillReplicaGrenadeSpeed;
	Int32 skillReplicaSig552ConeSize;
	Int32 skillReplicaSig552ConeSizePrecise;
	Int32 skillReplicaShotgunConeSize;
	Int32 skillReplicaTRG42ConeSize;
	Int32 skillReplicaM249ConeSize;
	Int32 skillReplicaReactionTime;

	Int32 skillSecurityHealth;
	Int32 skillSecurityGlockConeSize;
	Int32 skillSecurityDeagleConeSize;
	Int32 skillSecurityTRG42ConeSize;

	Int32 skillPlayerDmgKnife;
	Int32 skillPlayerDmg9MM;
	Int32 skillPlayerDmgHandGrenade;
	Int32 skillPlayerHandGrenadeRadius;

	Int32 skillKevlarAmount;
	Int32 skillHealthkitHealAmount;
	Int32 skillMaxHealthkits;
	Int32 skillSodaCanHealAmount;

	Int32 skillNPCDmgMultiplierHead;
	Int32 skillNPCDmgMultiplierChest;
	Int32 skillNPCDmgMultiplierStomach;
	Int32 skillNPCDmgMultiplierArm;
	Int32 skillNPCDmgMultiplierLeg;
	Int32 skillNPCReactionTime;

	Int32 skillPlayerDmgMultiplierHead;
	Int32 skillPlayerDmgMultiplierChest;
	Int32 skillPlayerDmgMultiplierStomach;
	Int32 skillPlayerDmgMultiplierArm;
	Int32 skillPlayerDmgMultiplierLeg;

	Int32 skillStaminaSprintDrain;
	Int32 skillStaminaJumpDrain;
	Int32 skillStaminaGainSpeed;
};

//=============================================
//
//=============================================
class CSkillData
{
public:
	// Skill cvar settings file
	static const Char SKILLCVAR_FILE[];

public:
	struct skillcvar_t
	{
		skillcvar_t():
			pskillcvareasy(nullptr),
			pskillcvarnormal(nullptr),
			pskillcvarhard(nullptr)
			{}

		CString cvarbasename;

		CCVar* pskillcvareasy;
		CCVar* pskillcvarnormal;
		CCVar* pskillcvarhard;
	};

public:
	CSkillData( void );
	~CSkillData( void );

public:
	// Initializes skill cvars
	bool Init( void );
	// Initializes for game
	bool InitGame( void );

public:
	// Registers a skilldata cvar
	Int32 RegisterSkillCVar( const Char* pstrcvarname );
	// Retrieves setting of a skill cvar
	Float GetSkillCVarSetting( Int32 skillcvarindex, force_skillcvar_t forceskill = FORCE_SKILL_OFF );

	// Returns the difficulty setting
	skill_level_t GetSkillLevel( void ) const;

private:
	// Array of skill cvars
	CArray<skillcvar_t> m_skillCVarsArray;
	// Pointer to sv_skill cvar
	CCVar* m_pSkillCvar;
};

extern skillcvars_t g_skillcvars;
extern CSkillData gSkillData;
#endif