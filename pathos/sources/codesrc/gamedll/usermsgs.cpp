/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "constants.h"
#include "usermsgs.h"
#include "gd_includes.h"
#include "player.h"
#include "msgreader.h"
#include "gameui_shared.h"

// Usermessage list
usermsglist_t g_usermsgs;

//=============================================
// @brief
//
//=============================================
void InitClientUserMessages( void )
{
	g_usermsgs.saytext = gd_engfuncs.pfnRegisterUserMessage("SayText", -1);
	g_usermsgs.setfog = gd_engfuncs.pfnRegisterUserMessage("SetFog", -1);
	g_usermsgs.creategenericdecal = gd_engfuncs.pfnRegisterUserMessage("CreateGenericDecal", -1);
	g_usermsgs.createvbmdecal = gd_engfuncs.pfnRegisterUserMessage("CreateVBMDecal", -1);
	g_usermsgs.createparticlesystem = gd_engfuncs.pfnRegisterUserMessage("CreateParticleSystem", -1);
	g_usermsgs.precacheparticlesystem = gd_engfuncs.pfnRegisterUserMessage("PrecacheParticleSystem", -1);
	g_usermsgs.removeparticlesystem = gd_engfuncs.pfnRegisterUserMessage("RemoveParticleSystem", -1);
	g_usermsgs.skyboxparameters = gd_engfuncs.pfnRegisterUserMessage("SkyboxParameters", -1);
	g_usermsgs.dynamiclight = gd_engfuncs.pfnRegisterUserMessage("DynamicLight", -1);
	g_usermsgs.setdaystage = gd_engfuncs.pfnRegisterUserMessage("SetDayStage", -1);
	g_usermsgs.setspecialfog = gd_engfuncs.pfnRegisterUserMessage("SetSpecialFog", -1);
	g_usermsgs.freeentitydata = gd_engfuncs.pfnRegisterUserMessage("FreeEntityData", -1);
	g_usermsgs.addlightstyle = gd_engfuncs.pfnRegisterUserMessage("AddLightStyle", -1);
	g_usermsgs.precacheflexscript = gd_engfuncs.pfnRegisterUserMessage("PrecacheFlexScript", -1);
	g_usermsgs.setentityflexscript = gd_engfuncs.pfnRegisterUserMessage("SetEntityFlexScript", -1);
	g_usermsgs.createtempentity = gd_engfuncs.pfnRegisterUserMessage("CreateTempEntity", -1);
	g_usermsgs.hudstamina = gd_engfuncs.pfnRegisterUserMessage("Stamina", -1);
	g_usermsgs.hudhealth = gd_engfuncs.pfnRegisterUserMessage("Health", -1);
	g_usermsgs.hudkevlar = gd_engfuncs.pfnRegisterUserMessage("Kevlar", -1);
	g_usermsgs.movementnoise = gd_engfuncs.pfnRegisterUserMessage("MovementNoise", -1);
	g_usermsgs.hudhealthkit = gd_engfuncs.pfnRegisterUserMessage("HealthKit", -1);
	g_usermsgs.hudcurrentweapon = gd_engfuncs.pfnRegisterUserMessage("CurrentWeapon", -1);
	g_usermsgs.hudweaponlist = gd_engfuncs.pfnRegisterUserMessage("WeaponList", -1);
	g_usermsgs.hudammocount = gd_engfuncs.pfnRegisterUserMessage("AmmoCount", -1);
	g_usermsgs.hudammopickup = gd_engfuncs.pfnRegisterUserMessage("AmmoPickup", -1);
	g_usermsgs.hudweaponpickup = gd_engfuncs.pfnRegisterUserMessage("WeaponPickup", -1);
	g_usermsgs.huditempickup = gd_engfuncs.pfnRegisterUserMessage("ItemPickup", -1);
	g_usermsgs.hudsetactive = gd_engfuncs.pfnRegisterUserMessage("SetHUDActive", -1);
	g_usermsgs.hudsetusableobject = gd_engfuncs.pfnRegisterUserMessage("SetUsableObject", -1);
	g_usermsgs.hudsetcountdowntimer = gd_engfuncs.pfnRegisterUserMessage("SetCountdownTimer", -1);
	g_usermsgs.setautoaimvector = gd_engfuncs.pfnRegisterUserMessage("SetAutoAimVector", -1);
	g_usermsgs.setflashlight = gd_engfuncs.pfnRegisterUserMessage("SetFlashlight", -1);
	g_usermsgs.screenshake = gd_engfuncs.pfnRegisterUserMessage("ScreenShake", -1);
	g_usermsgs.creategameuiwindow = gd_engfuncs.pfnRegisterUserMessage("CreateGameUIWindow", -1);
	g_usermsgs.setfov = gd_engfuncs.pfnRegisterUserMessage("SetFOV", -1);
	g_usermsgs.triggerzoom = gd_engfuncs.pfnRegisterUserMessage("TriggerZoom", -1);
	g_usermsgs.radiomessage = gd_engfuncs.pfnRegisterUserMessage("RadioMessage", -1);
	g_usermsgs.ladder = gd_engfuncs.pfnRegisterUserMessage("Ladder", -1);
	g_usermsgs.motorbike = gd_engfuncs.pfnRegisterUserMessage("MotorBike", -1);
	g_usermsgs.viewcontroller = gd_engfuncs.pfnRegisterUserMessage("ViewController", -1);
	g_usermsgs.viewmodel = gd_engfuncs.pfnRegisterUserMessage("ViewModel", -1);
	g_usermsgs.motionblur = gd_engfuncs.pfnRegisterUserMessage("MotionBlur", -1);
	g_usermsgs.showmessage = gd_engfuncs.pfnRegisterUserMessage("ShowMessage", -1);
	g_usermsgs.showcustommessage =  gd_engfuncs.pfnRegisterUserMessage("ShowCustomMessage", -1);
	g_usermsgs.screenfade = gd_engfuncs.pfnRegisterUserMessage("ScreenFade", -1);
	g_usermsgs.setviewentity = gd_engfuncs.pfnRegisterUserMessage("SetViewEntity", -1);
	g_usermsgs.nodedebug = gd_engfuncs.pfnRegisterUserMessage("NodeDebug", -1);
	g_usermsgs.screentext = gd_engfuncs.pfnRegisterUserMessage("ScreenText", -1);
	g_usermsgs.tempbeam = gd_engfuncs.pfnRegisterUserMessage("TempBeam", -1);
	g_usermsgs.blackhole = gd_engfuncs.pfnRegisterUserMessage("BlackHole", -1);
	g_usermsgs.sunflare = gd_engfuncs.pfnRegisterUserMessage("SunFlare", -1);
	g_usermsgs.vaportrail = gd_engfuncs.pfnRegisterUserMessage("VaporTrail", -1);
	g_usermsgs.npcawareness = gd_engfuncs.pfnRegisterUserMessage("NPCAwareness", -1);
	g_usermsgs.newobjective = gd_engfuncs.pfnRegisterUserMessage("NewObjective", -1);
	g_usermsgs.addskytextureset = gd_engfuncs.pfnRegisterUserMessage("AddSkyTextureSet", -1);
	g_usermsgs.setskytexture = gd_engfuncs.pfnRegisterUserMessage("SetSkyTexture", -1);
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_GameUIMessage( edict_t* pclient, const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	if(Util::IsNullEntity(pclient))
	{
		gd_engfuncs.pfnCon_Printf("%s - Client was null.\n", __FUNCTION__);
		return false;
	}

	// Get player
	CPlayerEntity* pPlayer = reinterpret_cast<CPlayerEntity*>(CBaseEntity::GetClass(pclient));
	if(!pPlayer || !pPlayer->IsPlayer())
	{
		gd_engfuncs.pfnCon_Printf("%s - Not a valid client.\n", __FUNCTION__);
		return false;
	}

	// Read data
	CMSGReader reader(pdata, msgsize);
	pPlayer->ManageGameUIEvent(reader);

	if(reader.HasError())
	{
		gd_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SelectWeapon( edict_t* pclient, const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	if(Util::IsNullEntity(pclient))
	{
		gd_engfuncs.pfnCon_Printf("%s - Client was null.\n", __FUNCTION__);
		return false;
	}

	// Get player
	CPlayerEntity* pPlayer = reinterpret_cast<CPlayerEntity*>(CBaseEntity::GetClass(pclient));
	if(!pPlayer || !pPlayer->IsPlayer())
	{
		gd_engfuncs.pfnCon_Printf("%s - Not a valid client.\n", __FUNCTION__);
		return false;
	}

	// Read data
	CMSGReader reader(pdata, msgsize);

	// Get weapon name and select
	const Char* pstrWeapon = reader.ReadString();
	pPlayer->SelectWeapon(pstrWeapon);

	if(reader.HasError())
	{
		gd_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SpawnClientDecal( edict_t* pclient, const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Int32 flags = reader.ReadUint16();
	flags |= FL_DECAL_SERVER;

	Vector origin;
	for(Uint32 i = 0; i < 3; i++)
		origin[i] = reader.ReadFloat();

	CString name = reader.ReadString();

	if(reader.HasError())
	{
		gd_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	trace_t tr;
	for(Uint32 i = 0; i < 3; i++)
	{
		Uint32 j = 0;
		for(; j < 2; j++)
		{
			Vector traceStartOffset;
			traceStartOffset[i] = j == 0 ? 1 : -1;

			Vector traceEndOffset;
			traceEndOffset[i] = j == 0 ? -1 : 1;

			Vector traceStart = origin + traceStartOffset;
			Vector traceEnd = origin + traceEndOffset;

			Util::TraceLine(traceStart, traceEnd, true, false, nullptr, tr);
			if(!tr.startSolid() && !tr.allSolid() && !tr.noHit())
				break;
		}

		if(j != 2)
			break;
	}

	entindex_t entindex = NO_ENTITY_INDEX;
	if(tr.startSolid() || tr.allSolid() || tr.noHit()
		|| tr.hitentity <= WORLDSPAWN_ENTITY_INDEX)
	{
		flags |= FL_DECAL_BOGUS;
	}
	else
	{
		entindex = tr.hitentity;
		origin = tr.endpos;
	}

	Util::CreateGenericDecal(origin, &tr.plane.normal, name.c_str(), flags, entindex, 0, 0, 0, pclient);

	return true;
}