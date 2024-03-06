/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "itemsodacan.h"
#include "player.h"

// Sodacan model
const Char CItemSodaCan::SODACAN_MODEL_FILENAME[] = "models/can.mdl";

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(item_sodacan, CItemSodaCan);

//=============================================
// @brief
//
//=============================================
CItemSodaCan::CItemSodaCan( edict_t* pedict ):
	CPlayerItem(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CItemSodaCan::~CItemSodaCan( void )
{
}

//=============================================
// @brief
//
//=============================================
void CItemSodaCan::PlayClatterSound( void )
{
	CString soundname;
	soundname << "items/sodacan_clatter" << (Int32)Common::RandomLong(1, 3) << ".wav";

	Util::EmitEntitySound(this, soundname.c_str(), SND_CHAN_BODY);
}

//=============================================
// @brief
//
//=============================================
void CItemSodaCan::SetModel( void )
{
	m_pFields->modelname = gd_engfuncs.pfnAllocString(SODACAN_MODEL_FILENAME);
}

//=============================================
// @brief
//
//=============================================
void CItemSodaCan::SetSpawnProperties( void )
{
	m_playImpactSound = true;
}

//=============================================
// @brief
//
//=============================================
bool CItemSodaCan::AddToPlayer( CBaseEntity* pPlayer )
{
	if(!pPlayer || !pPlayer->IsPlayer())
		return false;

	bool result = pPlayer->TakeHealth(gSkillData.GetSkillCVarSetting(g_skillcvars.skillSodaCanHealAmount), DMG_GENERIC);
	if(result)
		Util::EmitEntitySound(pPlayer, "items/sodacan_open.wav", SND_CHAN_ITEM);

	return result;
}

//=============================================
// @brief
//
//=============================================
void CItemSodaCan::Precache( void )
{
	Util::PrecacheFixedNbSounds("items/sodacan_clatter%d.wav", 3);
	gd_engfuncs.pfnPrecacheSound("items/sodacan_open.wav");

	gd_engfuncs.pfnPrecacheModel(SODACAN_MODEL_FILENAME);
}