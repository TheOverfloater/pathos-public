/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "game.h"
#include "skilldata.h"
#include "materialdefs.h"
#include "playerweapon.h"
#include "globalstate.h"
#include "player.h"
#include "constants.h"
#include "textures_shared.h"
#include "constants.h"
#include "decallist.h"
#include "flexmanager.h"
#include "ai_sounds.h"
#include "sentencesfile.h"
#include "ai_talknpc.h"
#include "ai_sounds.h"
#include "npcclonesoldier.h"
#include "timedamage.h"

// Path to impact effects script
static const Char MATERIAL_DEFINITIONS_SCRIPT_PATH[] = "scripts/materialdefs.txt";

// Cheats cvar
CCVar* g_pCvarCheats = nullptr;
// Trigger debug mode
CCVar* g_pCvarTriggerDebug = nullptr;
// NPC debug mode
CCVar* g_pCvarNPCDebug = nullptr;
// Controls whether weapons play holster animations
CCVar* g_pCvarWeaponHolster = nullptr;
// Controls whether zoom works on a hold-to-zoom basis
CCVar* g_pCvarHoldToZoom = nullptr;
// Gravity cvar ptr
CCVar* g_pCvarGravity = nullptr;
// Autoaim cvar ptr
CCVar* g_pCvarAutoAim = nullptr;

// Decal list object
CDecalList gDecalList;
// Flex manager object
CFlexManager* g_pFlexManager = nullptr;
// Sentences file object
CSentencesFile* g_pSentencesFile = nullptr;

// Max impact positions per FireBullets instance
static const Uint32 IMPACT_POSITION_MAX = 32;
// Max penetrations by NPCs per frame
static const Uint32 MAX_NPC_FRAME_PENETRATIONS = 8;

// Counter for penetrations by NPCs
Uint32 g_nbNPCPenetrations = 0;

//=============================================
// @brief
//
//=============================================
bool InitGameObjects( void )
{
	// Initialize skill cvars
	if(!gSkillData.Init())
		return false;

	// Create cvars used by game dll
	g_pCvarCheats = gd_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, "sv_cheats", "0", "Controls cheats");
	g_pCvarTriggerDebug = gd_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, "dev_triggers", "0", "Show developer trigger prints");
	g_pCvarNPCDebug = gd_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_SV_ONLY, "dev_npcdebug", "0", "Precache all NPCs for debugging");
	g_pCvarWeaponHolster = gd_engfuncs.pfnCreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_SAVE), "sv_weaponholster", "1", "Toggle weapon holster animations");
	g_pCvarHoldToZoom = gd_engfuncs.pfnCreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_SAVE), "sv_holdtozoom", "1", "Make zooming be active only when holding the zoom button");
	g_pCvarGravity = gd_engfuncs.pfnGetCVarPointer(GRAVITY_CVAR_NAME);
	g_pCvarAutoAim = gd_engfuncs.pfnGetCVarPointer(AUTOAIM_CVAR_NAME);

	// Create commands
	gd_engfuncs.pfnCreateCommand("dumpcheats", DumpCheatCodes, "Dumps cheat codes");

	// Create flex manager object
	if(!g_pFlexManager)
		g_pFlexManager = new CFlexManager(gd_filefuncs);

	if(!InitSentences())
		return false;

	if(!InitDecalList())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool InitSentences( void )
{
	const byte* pfile = gd_filefuncs.pfnLoadFile(SENTENCES_FILE_PATH, nullptr);
	if(!pfile)
	{
		gd_engfuncs.pfnCon_Printf("%s - Could not load '%s'.\n", __FUNCTION__, SENTENCES_FILE_PATH);
		return false;
	}

	if(!g_pSentencesFile)
		g_pSentencesFile = new CSentencesFile(gd_engfuncs.pfnPrecacheSound, gd_engfuncs.pfnGetSoundDuration);

	bool result = g_pSentencesFile->Init(pfile);
	
	// Free the file
	gd_filefuncs.pfnFreeFile(pfile);

	// Print any warnings
	Uint32 warningNb = g_pSentencesFile->GetNbWarnings();
	for(Uint32 i = 0; i < warningNb; i++)
		gd_engfuncs.pfnCon_Printf(g_pSentencesFile->GetWarning(i));

	if(!result)
	{
		gd_engfuncs.pfnCon_Printf("%s - Could not init sentences.\n", __FUNCTION__);
		delete g_pSentencesFile;
		g_pSentencesFile = nullptr;
		return false;
	}
	
	return true;
}

//=============================================
// @brief
//
//=============================================
void ClearGameObjects( void )
{
	if(g_pFlexManager)
	{
		delete g_pFlexManager;
		g_pFlexManager = nullptr;
	}
	
	if(g_pSentencesFile)
	{
		delete g_pSentencesFile;
		g_pSentencesFile = nullptr;
	}

	gDecalList.Clear();
}

//=============================================
// @brief
//
//=============================================
bool InitGame( void )
{
	if(!InitMaterialDefinitions())
		return false;

	Util::PrecacheFlexScript( FLEX_NPC_TYPE_HUMAN, "expressions_human.txt" );

	// Reset this
	CTalkNPC::SetTalkWaitTime(0);

	return true;
}

//=============================================
// @brief
//
//=============================================
void ClearGame( void )
{
	gMaterialDefinitions.Clear();
	gGlobalStates.Clear();
	gAISounds.Clear();

	if(g_pFlexManager)
		g_pFlexManager->Clear();

	CNPCCloneSoldier::Reset();
	CTalkNPC::ResetTalkTime();
	CTimeDamage::ClearTimeDamageList();
}

//=============================================
// @brief
//
//=============================================
bool InitDecalList( void )
{
	Uint32 filesize = 0;
	const Char* pfile = reinterpret_cast<const Char*>(gd_filefuncs.pfnLoadFile(DECAL_LIST_FILE_PATH, &filesize));
	if(!pfile)
	{
		gd_engfuncs.pfnCon_Printf("%s - Could not load decal list file '%s'.\n", __FUNCTION__, DECAL_LIST_FILE_PATH);
		return false;
	}

	if(!gDecalList.LoadDecalList(pfile, filesize))
	{
		gd_engfuncs.pfnCon_Printf("%s - Failed to load decal list: %s.\n", __FUNCTION__, DECAL_LIST_FILE_PATH);
		gd_filefuncs.pfnFreeFile(pfile);
		return false;
	}

	gd_filefuncs.pfnFreeFile(pfile);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool InitMaterialDefinitions( void )
{
	const byte* pfile = gd_filefuncs.pfnLoadFile(MATERIAL_DEFINITIONS_SCRIPT_PATH, nullptr);
	if(!pfile)
	{
		gd_engfuncs.pfnCon_EPrintf("%s - Failed to load '%s'.\n", __FUNCTION__, MATERIAL_DEFINITIONS_SCRIPT_PATH);
		return false;
	}
	
	// Try to initialize the class
	bool result = gMaterialDefinitions.Init(reinterpret_cast<const Char*>(pfile));
	gd_filefuncs.pfnFreeFile(pfile);

	if(!result)
	{
		gd_engfuncs.pfnCon_EPrintf("Failed to init impact effects: %s.\n", gMaterialDefinitions.GetError());
		return false;
	}

	// Precache the effects
	Uint32 nbeffects = gMaterialDefinitions.GetNbDefinitions();
	for(Uint32 i = 0; i < nbeffects; i++)
	{
		const CMaterialDefinitions::materialdefinition_t* pdefinition = gMaterialDefinitions.GetDefinition(i);

		if(!pdefinition->sounds.empty())
		{
			for(Uint32 j = 0; j < pdefinition->sounds.size(); j++)
				gd_engfuncs.pfnPrecacheSound(pdefinition->sounds[j].c_str());
		}

		if(!pdefinition->particlescript.empty())
			gd_engfuncs.pfnPrecacheParticleScript(pdefinition->particlescript.c_str(), pdefinition->scripttype);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void DumpGlobals( void )
{
	gGlobalStates.Dump();
}

//=============================================
// @brief
//
//=============================================
void DumpCheatCodes( void )
{
	CPlayerEntity::DumpCheatCodes();
}

//=============================================
// @brief
//
//=============================================
void ToggleBikeBlockers( bool enable )
{
	for(Int32 i = 0; i < g_pGameVars->numentities; i++)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(!pEntity)
			continue;

		if(pEntity->IsBikeBlocker())
			pEntity->SetBikeBlock(enable);
	}
}

//=============================================
// @brief
//
//=============================================
void PrecacheGenericResources( void )
{
	gd_engfuncs.pfnPrecacheSound("common/null.wav");
	gd_engfuncs.pfnPrecacheSound("impact/gib_01.wav");
	gd_engfuncs.pfnPrecacheSound("impact/gib_02.wav");

	Util::PrecacheFixedNbSounds("items/weapon_clatter%d.wav", 3);
	Util::PrecacheFixedNbSounds("debris/shotshell%d.wav", 3);
	Util::PrecacheFixedNbSounds("debris/shell%d.wav", 3);

	Util::PrecacheFixedNbSounds("weapons/grenade_impact%d.wav", 3);
	Util::PrecacheFixedNbSounds("impact/bullet_hit_flesh%d.wav", 2);
	Util::PrecacheFixedNbSounds("weapons/explosion%d.wav", 3);
	Util::PrecacheFixedNbSounds("impact/ricochet%d.wav", 3);

	gd_engfuncs.pfnPrecacheSound("items/pickup_ammo.wav");
	gd_engfuncs.pfnPrecacheSound("items/pickup_weapon.wav");
	gd_engfuncs.pfnPrecacheSound("items/pickup_medkit.wav");
	gd_engfuncs.pfnPrecacheSound("items/pickup_kevlar.wav");
	gd_engfuncs.pfnPrecacheSound("items/medkit_use.wav");

	gd_engfuncs.pfnPrecacheSound("common/wpn_denyselect.wav");
	gd_engfuncs.pfnPrecacheSound("common/wpn_hudoff.wav");
	gd_engfuncs.pfnPrecacheSound("common/wpn_hudon.wav");
	gd_engfuncs.pfnPrecacheSound("common/wpn_moveselect.wav");
	gd_engfuncs.pfnPrecacheSound("common/wpn_select.wav");

	gd_engfuncs.pfnPrecacheModel(SHELLCASINGS_MODEL_FILENAME);
	gd_engfuncs.pfnPrecacheModel(BULLETS_MODEL_FILENAME);
	gd_engfuncs.pfnPrecacheModel("models/grenade.mdl");
	gd_engfuncs.pfnPrecacheModel("models/humangibs.mdl");
	gd_engfuncs.pfnPrecacheModel("sprites/null.spr");

	Util::PrecacheFixedNbSounds("impact/water_splash%d.wav", 3);

	Util::PrecacheFixedNbSounds("debris/concrete_clatter%d.wav", 3);
	Util::PrecacheFixedNbSounds("debris/metal_clatter%d.wav", 3);
	Util::PrecacheFixedNbSounds("debris/shell%d.wav", 3);
	Util::PrecacheFixedNbSounds("debris/shotshell%d.wav", 3);
	Util::PrecacheFixedNbSounds("debris/wood_clatter%d.wav", 3);

	gd_engfuncs.pfnPrecacheParticleScript("bullet_tracer.txt", PART_SCRIPT_SYSTEM);

	gd_engfuncs.pfnPrecacheParticleScript("engine_muzzleflash_cluster1.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("engine_muzzleflash_cluster2.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("engine_muzzleflash_cluster3.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("engine_muzzleflash_cluster4.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("explosion_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("explosion_underwater_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("spark_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("engine_gib_flames_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("engine_gib_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("engine_gib_small_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("engine_gib_explode_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("engine_plasma_trails.txt", PART_SCRIPT_SYSTEM);
	gd_engfuncs.pfnPrecacheParticleScript("engine_muzzle_smoke.txt", PART_SCRIPT_SYSTEM);
	gd_engfuncs.pfnPrecacheParticleScript("blood_effects_cluster_player.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("blood_effects_decap.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("blood_effects_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("blood_effects_cluster_living.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("engine_explosion_smoke.txt", PART_SCRIPT_SYSTEM);

	gd_engfuncs.pfnPrecacheParticleScript("wood_impact_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("water_impact_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("snow_impact_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("gravel_impact_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("glass_impact_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("dirt_impact_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("concrete_impact_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("cluster_impact_metal.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("carpet_impact_cluster.txt", PART_SCRIPT_CLUSTER);
	gd_engfuncs.pfnPrecacheParticleScript("cluster_impact_metal.txt", PART_SCRIPT_CLUSTER);

	// Precache frequently used groups
	gd_engfuncs.pfnPrecacheDecalGroup("scorch");
	gd_engfuncs.pfnPrecacheDecalGroup("shot");
	gd_engfuncs.pfnPrecacheDecalGroup("shot_metal");
	gd_engfuncs.pfnPrecacheDecalGroup("shot_wood");
	gd_engfuncs.pfnPrecacheDecalGroup("shot_glass");
	gd_engfuncs.pfnPrecacheDecalGroup("shot_human");
	gd_engfuncs.pfnPrecacheDecalGroup("shot_alien");
	gd_engfuncs.pfnPrecacheDecalGroup("brains");
	gd_engfuncs.pfnPrecacheDecalGroup("redblood");
	gd_engfuncs.pfnPrecacheDecalGroup("redblood_sm");
	gd_engfuncs.pfnPrecacheDecalGroup("yellowblood");

	gd_engfuncs.pfnPrecacheDecal("bloodbigsplat");
	gd_engfuncs.pfnPrecacheDecal("bloodbigsplat2");

	if(g_pCvarNPCDebug->GetValue() >= 1)
	{
		Util::PrecacheEntity("npc_clone_soldier");
		Util::PrecacheEntity("npc_security");
	}
}

//=============================================
// @brief
//
//=============================================
void PrecachePlayerItems( void )
{
	// Clear previous
	CPlayerWeapon::ClearWeaponInfos();

	// Player items
	Util::PrecacheEntity("item_kevlar");
	Util::PrecacheEntity("item_healthkit");
	Util::PrecacheEntity("item_motorbike");

	// Knife
	Weapon_Precache("weapon_knife");

	// Glock
	Weapon_Precache("weapon_glock");
	Util::PrecacheEntity("ammo_glock_clip");
	Util::PrecacheEntity("item_glock_silencer");
	Util::PrecacheEntity("item_glock_flashlight");

	// Hand grenade
	Weapon_Precache("weapon_handgrenade");
}

//=============================================
// @brief
//
//=============================================
void RadiusDamage( const Vector& vecPosition, 
	CBaseEntity* pInflictor, 
	CBaseEntity* pAttacker,
	Float damageDealt,
	Float damageRadius,
	Int32 classToIgnore,
	Int32 damageFlags )
{
	Float falloff;
	if(damageRadius)
		falloff = damageDealt/damageRadius;
	else
		falloff = 1.0;

	// Check if we're underwater
	bool isInWater = (gd_tracefuncs.pfnPointContents(vecPosition, nullptr) == CONTENTS_WATER) ? true : false;
	// Offset from ground a bit
	Vector explodePosition = vecPosition + Vector(0, 0, 8);

	Vector bboxmins, bboxmaxs;
	bboxmins = explodePosition - Vector(damageRadius, damageRadius, damageRadius);
	bboxmaxs = explodePosition + Vector(damageRadius, damageRadius, damageRadius);

	// Find all entities in the damage radius
	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityInBBox(pedict, bboxmins, bboxmaxs);
		if(!pedict)
			break;

		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(!pEntity || pEntity->GetTakeDamage() == TAKEDAMAGE_NO)
			continue;

		// Do damage based on waterlevel
		waterlevel_t entityWaterLevel = pEntity->GetWaterLevel();
		if(isInWater && entityWaterLevel == WATERLEVEL_NONE
			|| !isInWater && entityWaterLevel == WATERLEVEL_FULL)
			continue;

		Vector entityTarget = pEntity->GetBodyTarget(explodePosition);

		trace_t tr;
		Util::TraceLine(explodePosition, entityTarget, false, true, false, true, pInflictor->GetEdict(), tr);
		if(!tr.noHit() && tr.hitentity != pEntity->GetEntityIndex())
			continue;

		if(tr.allSolid())
		{
			tr.endpos = explodePosition;
			tr.fraction = 0;
		}

		Float adjustedDmg = (explodePosition-tr.endpos).Length()*falloff;
		adjustedDmg = damageDealt - adjustedDmg;
		if(adjustedDmg < 0)
			adjustedDmg = 0;

		if(tr.fraction != 1.0)
		{
			gMultiDamage.Clear();

			Vector dmgDirection = (tr.endpos - explodePosition).Normalize();
			pEntity->TraceAttack(pAttacker, adjustedDmg, dmgDirection, tr, damageFlags);

			gMultiDamage.ApplyDamage(pInflictor, pAttacker);
		}
		else
		{
			pEntity->TakeDamage(pInflictor, pAttacker, adjustedDmg, damageFlags);
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CreateBulletProofImpactModel( const Vector& origin, const Vector& shootDirection, bullet_types_t bulletType )
{
	Vector adjustedOrigin = origin;
	adjustedOrigin += shootDirection*Common::RandomFloat(0.1, 2.0);

	CString submodelName;
	switch(bulletType)
	{
	case BULLET_PLAYER_GLOCK:
	case BULLET_NPC_9MM:
		submodelName = "9mm_reference";
		break;
	case BULLET_NPC_BUCKSHOT:
		submodelName = "pellet_reference";
		break;
	case BULLET_NPC_M249:
	case BULLET_NPC_SIG552:
		submodelName = "762_reference";
		break;
	case BULLET_NPC_TRG42:
		submodelName = "762_reference";
		break;
	case BULLET_PLAYER_KNIFE:
		return;
	}

	if(submodelName.empty())
		return;

	Int64 bodyValue = Util::GetBodyValueForSubmodel(BULLETS_MODEL_FILENAME, submodelName.c_str());
	if(bodyValue == NO_POSITION)
		return;

	Vector angles = Math::VectorToAngles(shootDirection);
	Util::CreateTempModel(adjustedOrigin, angles, Vector(0, 0, 0), 180, 1, BULLETS_MODEL_FILENAME, 0, 0, 0, TE_FL_NOGRAVITY, bodyValue);
}

//=============================================
// @brief
//
//=============================================
void CreateGunshotDecal( const Vector& decalPosition, 
	const Vector& planeNormal,
	CBaseEntity* pEntity )
{
	// Only apply on brush models
	if(!pEntity->IsBrushModel())
		return;

	CString materialname;
	if((pEntity->GetRenderMode() & 255) == RENDER_TRANSADDITIVE
		|| (pEntity->GetRenderMode() & 255) == RENDER_TRANSTEXTURE)
	{
		// Default to glass when rendermode is transparent
		materialname = GLASS_MATERIAL_TYPE_NAME;
	}
	else
	{
		// Get texture
		Vector traceStart = decalPosition + planeNormal * 4;
		Vector traceEnd = decalPosition - planeNormal * 4;

		const Char* pstrTextureName = gd_tracefuncs.pfnTraceTexture(pEntity->GetEntityIndex(), traceStart, traceEnd);
		if(!pstrTextureName || !qstrlen(pstrTextureName))
			return;

		const en_material_t* pmaterial = gd_engfuncs.pfnGetMapTextureMaterialScript(pstrTextureName);
		if(!pmaterial)
			return;

		materialname = pmaterial->materialname;
	}

	const CMaterialDefinitions::materialdefinition_t* pdefinition = gMaterialDefinitions.GetDefinitionForMaterial(materialname.c_str());
	if(!pdefinition)
		return;

	Util::CreateGenericDecal(decalPosition, &planeNormal, pdefinition->decalgroup.c_str(), FL_DECAL_NONE);
}

//=============================================
// @brief
//
//=============================================
bool ShootTrace( const Vector& gunPosition, const Vector& endPos, const Vector& shootDirection, 
	CBaseEntity* pAttacker, CBaseEntity* pIgnoreEntity, CBaseEntity* pWeapon, 
	Float damage, Float dmgMultiplier, Int32 dmgFlags, bullet_types_t bulletType, Int32& hitgroup, trace_t &tr,
	Vector* pImpactPositions, Uint32& numImpactPositions, bool isPenetrationShot, bool isRicochetShot, CBaseEntity* pTraceModel)
{
	if(pTraceModel)
		Util::TraceModel(pTraceModel, gunPosition, endPos, true, HULL_POINT, tr);
	else
		Util::TraceLine(gunPosition, endPos, false, true, false, true, isRicochetShot ? nullptr : pAttacker->GetEdict(), tr);
		
	// Don't bother if we hit nothing
	if(tr.noHit())
		return false;

	// Allow weapon to add special effects to shot
	if(pWeapon)
		pWeapon->OnWeaponShotImpact(gunPosition, tr, (isPenetrationShot || isRicochetShot) ? true : false);

	// Don't bother if we hit the sky
	if(gd_tracefuncs.pfnPointContents(tr.endpos, nullptr) == CONTENTS_SKY)
		return false;

	// Make sure it's valid
	edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(tr.hitentity);
	if(!pedict || Util::IsNullEntity(pedict))
		return false;

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	if(!pEntity)
		return false;

	if(damage > 0)
	{
		pEntity->TraceAttack(pAttacker, damage, shootDirection, tr, dmgFlags);
		CreateGunshotDecal(tr.endpos, tr.plane.normal, pEntity);
	}
	else
	{
		Int32 skillCvarIndex = SKILLCVAR_UNDEFINED;
		switch(bulletType)
		{
		case BULLET_PLAYER_GLOCK:
			skillCvarIndex = g_skillcvars.skillPlayerDmg9MM;
			break;
		case BULLET_PLAYER_KNIFE:
			skillCvarIndex = g_skillcvars.skillPlayerDmgKnife;
			break;
		case BULLET_NPC_9MM:
			skillCvarIndex = g_skillcvars.skillNPC9MMBullet;
			break;
		case BULLET_NPC_BUCKSHOT:
			skillCvarIndex = g_skillcvars.skillNPCBuckshotBullet;
			break;
		case BULLET_NPC_SIG552:
			skillCvarIndex = g_skillcvars.skillNPCSig552Bullet;
			break;
		case BULLET_NPC_M249:
			skillCvarIndex = g_skillcvars.skillNPC556Bullet;
			break;
		case BULLET_NPC_TRG42:
			skillCvarIndex = g_skillcvars.skillNPC762Bullet;
			break;
		}

		if(skillCvarIndex == SKILLCVAR_UNDEFINED)
		{
			gd_engfuncs.pfnCon_Printf("%s - Unhandled bullet type %d.\n", __FUNCTION__, bulletType);
			return false;
		}

		// Apply damage
		Float dmg = gSkillData.GetSkillCVarSetting(skillCvarIndex) * dmgMultiplier;

		// Apply any multipliers
		if(pWeapon)
			dmg *= pWeapon->GetWeaponDmgMultiplier(pEntity);

		pEntity->TraceAttack(pAttacker, dmg, shootDirection, tr, dmgFlags);

		bool playSound;
		if(bulletType == BULLET_NPC_BUCKSHOT)
		{
			// Min distance for sound spam
			const Float minImpactSoundDistance = 128;

			Uint32 i = 0;
			for(; i < numImpactPositions; i++)
			{
				Float distance = (pImpactPositions[i] - tr.endpos).Length();
				if(distance < minImpactSoundDistance)
					break;
			}

			if(i == numImpactPositions && numImpactPositions < IMPACT_POSITION_MAX)
			{
				pImpactPositions[numImpactPositions] = tr.endpos;
				playSound = true;
			}
			else
			{
				playSound = false;
			}
		}
		else
		{
			playSound = true;
		}

		// Create impact effects
		Util::CreateImpactEffects(tr, gunPosition, true, false, playSound);

		// Set hitgroup if it's not head
		if(hitgroup != HITGROUP_HEAD)
			hitgroup = tr.hitgroup;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void FireBullets( Uint32 nbshots, 
	const Vector& gunPosition, 
	const Vector& aimForward, 
	const Vector& aimRight, 
	const Vector& aimUp, 
	const Vector& spread, 
	Float distance, 
	bullet_types_t bulletType, 
	Int32 tracerFrequency, 
	Float damage, 
	CBaseEntity* pAttacker,
	CBaseEntity* pWeapon )
{
	// For tracking tracer counts
	static Int32 tracerCount = 0;
	// Hit group index
	Int32 hitgroup = HITGROUP_GENERIC;

	if(!pAttacker)
	{
		gd_engfuncs.pfnCon_Printf("%s - No attacker specified.\n", __FUNCTION__);
		return;
	}

	// Clear multidamage
	gMultiDamage.Clear();
	
	// Set basic flags
	Int32 dmgFlags = DMG_BULLET;

	// Set gibbing for certain bullet types
	if(bulletType == BULLET_NPC_BUCKSHOT)
		dmgFlags |= (DMG_BULLETGIB | DMG_BUCKSHOT);
	else
		dmgFlags |= DMG_NEVERGIB;

	// Add sound
	if(!pAttacker->IsPlayer())
		gAISounds.AddSound(pAttacker, AI_SOUND_COMBAT, GUN_VOLUME_LOUD, VOL_NORM, 6.0);

	// For reducing sound spam
	static Vector impactPositions[IMPACT_POSITION_MAX];
	Uint32 numImpactPositions = 0;

	// Spawn eachbullet now
	for(Uint32 i = 0; i < nbshots; i++)
	{
		Vector shootDirection;
		Vector gunTracePosition;
		Vector endPos;

		// Remember per-shot
		Int32 shotDmgFlags = dmgFlags;

		Float x, y, z;
		do
		{
			x = Common::RandomFloat(-0.5, 0.5) + Common::RandomFloat(-0.5, 0.5);
			y = Common::RandomFloat(-0.5, 0.5) + Common::RandomFloat(-0.5, 0.5);
			z = x*x + y*y;
		}
		while(z > 1);

		shootDirection = aimForward
			+ x * spread.x * aimRight
			+ y * spread.y * aimUp;

		gunTracePosition = gunPosition;
		endPos = gunTracePosition + shootDirection*distance;

		// Spawn the tracer
		if(tracerFrequency != 0)
		{
			tracerCount++;
			if((tracerCount % tracerFrequency) == 0)
			{
				// Base position is gun position
				Vector tracerOrigin = gunTracePosition;

				// Do special adjustments for player fired bullets
				if(pAttacker->IsPlayer())
					tracerOrigin += Vector(0, 0, -4)+aimRight*2+aimForward*16;

				Util::CreateParticles("bullet_tracer.txt", tracerOrigin, shootDirection, PART_SCRIPT_SYSTEM);
			}
		}

		trace_t tr;
		if(ShootTrace(gunTracePosition, endPos, shootDirection, pAttacker, pAttacker, pWeapon, damage, 1.0, shotDmgFlags, bulletType, hitgroup, tr, impactPositions, numImpactPositions, false, false, nullptr))
		{
			// Number of ricochets
			Uint32 numRicochets = 0;
			// Number of penetrations
			Uint32 numPenetrations = 0;
			// Damage multiplier for penetration shots
			Float dmgMultiplier = 1.0;
			// Chance of penetration
			Int32 penetrationChance = -1;

			// Mark damage type as penetrating
			shotDmgFlags |= DMG_PENETRATION;

			while(true)
			{
				if(pAttacker->IsNPC())
				{
					if(g_nbNPCPenetrations >= MAX_NPC_FRAME_PENETRATIONS)
					{
						// Do not allow too many NPC penetration shots, for performance
						break;
					}
					else
					{
						g_nbNPCPenetrations++;
					}
				}

				// Only penetrate brushmodels, world or npcs
				CBaseEntity* pHitEntity = Util::GetEntityFromTrace(tr);
				if(!pHitEntity->IsBrushModel() 
					&& !pHitEntity->IsWorldSpawn() 
					&& !pHitEntity->IsNPC())
					break;

				CString materialname;
				if(pHitEntity->IsBrushModel() || pHitEntity->IsWorldSpawn())
				{
					// Get material type at entry
					const Char* pstrTextureName = Util::TraceTexture(pHitEntity->GetEntityIndex(), tr.endpos, tr.plane.normal);
					if(!pstrTextureName)
						break;

					const en_material_t* pmaterial = gd_engfuncs.pfnGetMapTextureMaterialScript(pstrTextureName);
					if(!pmaterial)
						break;

					// Check if penetration is allowed
					if(pmaterial->flags & TX_FL_NO_PENETRATION)
					{
						if(pmaterial->flags & TX_FL_BULLETPROOF)
						{
							// Spawn bullet if set to bulletproof
							CreateBulletProofImpactModel(tr.endpos, shootDirection, bulletType);
						}

						break;
					}

					if((pHitEntity->GetRenderMode() & 255) == RENDER_TRANSTEXTURE)
					{
						// All transparents are glass
						materialname = GLASS_MATERIAL_TYPE_NAME;
					}
					else
					{
						// Take from material script file
						materialname = pmaterial->materialname;
					}
				}
				else
				{
					// NPC
					if(tr.hitgroup != HITGROUP_HELMET)
						materialname = ORGANIC_MATERIAL_TYPE_NAME;
					else
						materialname = METAL_MATERIAL_TYPE_NAME;
				}

				const CMaterialDefinitions::materialdefinition_t* pdefinition = gMaterialDefinitions.GetDefinitionForMaterial(materialname.c_str());
				if(!pdefinition)
					break;

				// See if we can ricochet
				bool bulletRicocheted = false;

				CMaterialDefinitions::ricochetinfo_t& ricochetInfo = pdefinition->ricochetinfos[bulletType];
				if(ricochetInfo.maxangle > 0 && ricochetInfo.ricochetchance > 0
					&& ricochetInfo.maxricochets > numRicochets
					&& Common::RandomLong(1, ricochetInfo.ricochetchance) != 1)
				{
					// Get dot product between surface normal and bullet vector
					Float surfDot = Math::DotProduct(-shootDirection, tr.plane.normal);
					if(surfDot < ricochetInfo.maxangle)
					{
						// Determine deviation
						Vector deviation(Common::RandomFloat(-ricochetInfo.maxdeviation, ricochetInfo.maxdeviation) * 0.5,
							Common::RandomFloat(-ricochetInfo.maxdeviation, ricochetInfo.maxdeviation) * 0.5,
							Common::RandomFloat(-ricochetInfo.maxdeviation, ricochetInfo.maxdeviation) * 0.5);

						// Reflect bullet direction off wall
						Vector newDirection;
						Float proj = Math::DotProduct(shootDirection, tr.plane.normal);
						Math::VectorMA(shootDirection, -proj*2, tr.plane.normal, newDirection);
						Math::VectorAdd(newDirection, deviation, shootDirection);
						Math::VectorNormalize(shootDirection);

						// Apply falloff
						if(ricochetInfo.ricochetdmgfalloff > 0)
							dmgMultiplier *= (1.0 - ricochetInfo.ricochetdmgfalloff);

						// Mark as having ricocheted
						bulletRicocheted = true;

						// Calculate new positions
						Vector startPosition = tr.endpos;
						endPos = startPosition + shootDirection * distance;

						// Add ricochet and tracer effect
						Util::Ricochet(tr.endpos, tr.plane.normal, false);
						Util::CreateParticles("bullet_tracer.txt", startPosition, shootDirection, PART_SCRIPT_SYSTEM);

						if(!ShootTrace(startPosition, endPos, shootDirection, pAttacker, 
							pAttacker, pWeapon, damage, dmgMultiplier, shotDmgFlags, bulletType, 
							hitgroup, tr, impactPositions, numImpactPositions, numPenetrations > 0 ? true : false, true, nullptr))
							break;

						numRicochets++;

						// Get next entity from trace
						pHitEntity = Util::GetEntityFromTrace(tr);

						// Only penetrate brushmodels, world or npcs
						if(!pHitEntity->IsBrushModel() 
							&& !pHitEntity->IsWorldSpawn()
							&& !pHitEntity->IsNPC())
							break;
					}
				}

				// Try penetrating if we did not ricochet
				if(!bulletRicocheted)
				{
					const CMaterialDefinitions::penetration_t& penetrationInfo = pdefinition->penetrationinfos[bulletType];
					if(penetrationChance == -1)
						penetrationChance = penetrationInfo.penetrationchance;
					else if(penetrationInfo.chancedecrease > 0)
						penetrationChance += penetrationInfo.chancedecrease;

					if(penetrationChance > 1 
						&& Common::RandomLong(1, penetrationChance) != 1)
						break;

					if(!penetrationInfo.maxpenetration 
						|| !penetrationInfo.penetrationdepth 
						|| numPenetrations >= penetrationInfo.maxpenetration)
						break;

					Vector startPosition;
					Vector endPosition = tr.endpos;
					for(Float distance = 4.0f; distance <= penetrationInfo.penetrationdepth; distance += 4.0f)
					{
						startPosition = tr.endpos + shootDirection*distance;
						if(pHitEntity->IsBrushModel() || pHitEntity->IsWorldSpawn())
							Util::TraceLine(startPosition, endPosition, true, true, false, true, pAttacker->GetEdict(), tr);
						else
							Util::TraceLine(startPosition, endPosition, false, true, pAttacker->GetEdict(), tr);

						if(!tr.startSolid() && !tr.allSolid() && !tr.noHit())
							break;
					}

					if(tr.startSolid() || tr.allSolid() || tr.noHit())
						break;

					bool playSound;
					if(bulletType == BULLET_NPC_BUCKSHOT)
					{
						// Min distance for sound spam
						const Float minImpactSoundDistance = 128;

						Uint32 i = 0;
						for(; i < numImpactPositions; i++)
						{
							Float distance = (impactPositions[i] - tr.endpos).Length();
							if(distance < minImpactSoundDistance)
								break;
						}

						if(i == numImpactPositions && numImpactPositions < IMPACT_POSITION_MAX)
						{
							impactPositions[numImpactPositions] = tr.endpos;
							numImpactPositions++;
							playSound = true;
						}
						else
						{
							playSound = false;
						}
					}
					else
					{
						playSound = true;
					}

					// Create impact effect at exit point
					Util::CreateImpactEffects(tr, startPosition, true, false, playSound, nullptr);

					startPosition = tr.endpos;
					if(penetrationInfo.damagefalloff < 1.0)
						dmgMultiplier *= penetrationInfo.damagefalloff;

					if(!ShootTrace(startPosition, endPos, shootDirection, pAttacker, pAttacker, pWeapon, damage, dmgMultiplier, shotDmgFlags, bulletType, hitgroup, tr, impactPositions, numImpactPositions, true, false, nullptr))
						break;

					// Increase penetration count
					numPenetrations++;

					// Get next entity from trace
					pHitEntity = Util::GetEntityFromTrace(tr);

					// Only penetrate brushmodels, world or npcs
					if(!pHitEntity->IsBrushModel() 
						&& !pHitEntity->IsWorldSpawn()
						&& !pHitEntity->IsNPC())
						break;
				}
			}
		}
	}

	// Apply the damage
	gMultiDamage.ApplyDamage(pAttacker, pAttacker, hitgroup);
}
