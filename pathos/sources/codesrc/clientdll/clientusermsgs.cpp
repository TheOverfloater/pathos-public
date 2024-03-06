/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "clientusermsgs.h"
#include "constants.h"
#include "msgreader.h"
#include "clientdll.h"
#include "cl_entity.h"
#include "decallist.h"
#include "viewmodel.h"
#include "dlight.h"
#include "saytext.h"
#include "hud.h"
#include "shake.h"
#include "view.h"
#include "ladder.h"
#include "motorbike.h"
#include "viewcontroller.h"
#include "nodedebug.h"
#include "messages.h"
#include "tempentity.h"
#include "screentext.h"
#include "beam_shared.h"

#include "gameui_shared.h"
#include "gameuimanager.h"
#include "gameuikeypadwindow.h"
#include "gameuiloginwindow.h"
#include "gameuisubwaywindow.h"
#include "gameuitextwindow.h"
#include "gameuiobjectiveswindow.h"

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SayText( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	const Char* pstrPlayerName = reader.ReadString();
	const Char* pstrText = reader.ReadString();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gSayText.AddText(pstrPlayerName, pstrText);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SetFog( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	Vector color;
	entindex_t entindex = reader.ReadInt16();

	for(Uint32 i = 0; i < 3; i++)
		color[i] = (Float)reader.ReadByte()/255.0f;

	Float startdist = reader.ReadFloat();
	Float enddist = reader.ReadFloat();
	bool affectsky = (reader.ReadByte() == 1) ? false : true;
	Float blendtime = reader.ReadSmallFloat();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	// Apply the fog effect
	cl_efxapi.pfnSetFogParameters(entindex, color, startdist, enddist, affectsky, blendtime);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_CreateGenericDecal( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Uint16 flags = reader.ReadUint16();
	
	Vector origin;
	for(Uint32 i = 0; i < 3; i++)
		origin[i] = reader.ReadFloat();

	CString name = reader.ReadString();

	Vector normal;
	if(flags & FL_DECAL_HAS_NORMAL)
	{
		for(Uint32 i = 0; i < 3; i++)
			normal[i] = reader.ReadSmallFloat()/360.0f;
	}

	entindex_t entindex = NO_ENTITY_INDEX;
	if(flags & FL_DECAL_TIED_TO_ENTITY)
		entindex = reader.ReadInt32();

	Float life = 0;
	Float fadetime = 0;
	Float growthtime = 0;

	if(!(flags & FL_DECAL_PERSISTENT))
	{
		if(flags & (FL_DECAL_DIE|FL_DECAL_DIE_FADE))
			life = reader.ReadSmallFloat();

		if(flags & FL_DECAL_DIE_FADE)
			fadetime = reader.ReadSmallFloat();

		if(flags & FL_DECAL_GROW)
			growthtime = reader.ReadSmallFloat();
	}

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnCreateGenericDecal(name.c_str(), origin, normal, flags, entindex, life, fadetime, growthtime);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_CreateParticleSystem( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	Vector origin;
	for(Uint32 i = 0; i < 3; i++)
		origin[i] = reader.ReadFloat();

	Vector angles;
	for(Uint32 i = 0; i < 3; i++)
		angles[i] = reader.ReadSmallFloat()/360.0f;

	part_script_type_t scripttype = (part_script_type_t)reader.ReadByte();
	if(scripttype != PART_SCRIPT_SYSTEM && scripttype != PART_SCRIPT_CLUSTER)
	{
		cl_engfuncs.pfnCon_Printf("%s - Bogus script type '%d'.\n", __FUNCTION__, (Int32)scripttype);
		return true;
	}

	CString scriptname = reader.ReadString();
	if(scriptname.empty())
	{
		cl_engfuncs.pfnCon_Printf("%s - Invalid script file '%s' specified.\n", __FUNCTION__, scriptname.c_str());
		return true;
	}

	Int32 id = reader.ReadInt32();
	entindex_t entindex = reader.ReadInt32();
	Int32 attachment = reader.ReadByte();
	Int32 boneindex = reader.ReadInt16();
	Int32 attachflags = reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnSpawnParticleSystem(origin, angles, scripttype, scriptname.c_str(), id, entindex, attachment, boneindex, attachflags);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_PrecacheParticleSystem( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	part_script_type_t scripttype = (part_script_type_t)reader.ReadByte();
	if(scripttype != PART_SCRIPT_SYSTEM && scripttype != PART_SCRIPT_CLUSTER)
	{
		cl_engfuncs.pfnCon_Printf("%s - Bogus script type '%d'.\n", __FUNCTION__, (Int32)scripttype);
		return true;
	}

	CString scriptname = reader.ReadString();
	if(scriptname.empty())
	{
		cl_engfuncs.pfnCon_Printf("%s - Invalid script file '%s' specified.\n", __FUNCTION__, scriptname.c_str());
		return true;
	}

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnPrecacheParticleScript(scripttype, scriptname.c_str());
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_RemoveParticleSystem( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Int32 id = reader.ReadInt32();
	entindex_t entindex = reader.ReadInt32();
	bool keepcached = reader.ReadByte() == 1 ? true : false;

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnRemoveParticleSystem(id, entindex, keepcached);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_CreateVBMDecal( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	entindex_t entindex = reader.ReadInt16();
	if(!entindex)
		return true;

	cl_entity_t* pentity = cl_engfuncs.pfnGetEntityByIndex(entindex);
	if(!pentity || !pentity->pmodel)
		return true;

	Vector origin, normal;
	for(Uint32 i = 0; i < 3; i++)
		origin[i] = reader.ReadFloat();

	for(Uint32 i = 0; i < 3; i++)
		normal[i] = reader.ReadSmallFloat()/360.0f;

	CString decalgrpname = reader.ReadString();
	Int32 flags = reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	// Get decal list
	CDecalList& decalList = cl_efxapi.pfnGetDecalList();
	decalgroupentry_t* pentry = decalList.GetRandom(decalgrpname.c_str());
	if(!pentry)
		return true;

	// Apply an extra decal on the view model if it's local player
	if(pentity == cl_engfuncs.pfnGetLocalPlayer())
		cl_efxapi.pfnDecalVBMEntity(origin, normal, pentry, gViewModel.GetViewModel(), flags);

	cl_efxapi.pfnDecalVBMEntity(origin, normal, pentry, pentity, flags);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SkyboxParameters( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	bool isactive = (reader.ReadByte() == 1) ? true : false;

	Vector worldorigin;
	Vector fogcolor;
	Float skysize = 0;
	Float fogstart = 0;
	Float fogend = 0;
	bool affectskybox = false;
	Int32 skytextureset = NO_POSITION;

	if(isactive)
	{
		for(Uint32 i = 0; i < 3; i++)
			worldorigin[i] = reader.ReadFloat();

		skysize = reader.ReadSmallFloat();
		fogstart = reader.ReadFloat();
		fogend = reader.ReadFloat();
	
		for(Uint32 i = 0; i < 3; i++)
			fogcolor[i] = (Float)reader.ReadByte()/255.0f;

		affectskybox = (reader.ReadByte() == 1) ? false : true;
		skytextureset = reader.ReadChar();
	}

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnSetSkyboxParameters(worldorigin, skysize, fogend, fogstart, fogcolor, skytextureset, affectskybox, isactive);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_DynamicLight( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	Vector origin;
	for(Uint32 i = 0; i < 3; i++)
		origin[i] = reader.ReadFloat();

	Float radius = reader.ReadSmallFloat();
	
	Vector color;
	for(Uint32 i = 0; i < 3; i++)
		color[i] = (Float)reader.ReadByte()/255.0f;

	Float life = reader.ReadSmallFloat();
	Float decay = reader.ReadSmallFloat();
	Float decaydelay = reader.ReadSmallFloat();
	Int32 flags = reader.ReadByte();

	Int32 key = 0;
	Int32 subkey = 0;
	Int32 attachment = NO_POSITION;
	Int32 lightstyle = 0;

	if(flags & FL_DLIGHT_FOLLOW_ENTITY)
	{
		key = reader.ReadInt16();

		if(flags & FL_DLIGHT_USE_SUBKEY)
			subkey = reader.ReadInt16();

		if(flags & FL_DLIGHT_USE_ATTACHMENT)
			attachment = reader.ReadInt16();
	}

	if(flags & FL_DLIGHT_USE_LIGHTSTYLES)
		lightstyle = reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_dlight_t* pdl = cl_efxapi.pfnAllocDynamicPointLight(key, subkey, (flags & FL_DLIGHT_STATICSHADOWS) ? true : false, (flags & FL_DLIGHT_NOSHADOWS) ? true : false, nullptr);
	pdl->die = cl_engfuncs.pfnGetClientTime() + life;
	pdl->decay = decay;
	pdl->color = color;
	pdl->origin = origin;
	pdl->radius = radius;
	pdl->decay_delay = decaydelay;
	pdl->lightstyle = lightstyle;

	pdl->followentity = (flags & FL_DLIGHT_FOLLOW_ENTITY) ? true : false;
	pdl->attachment = attachment;
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SetDayStage( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	daystage_t stage = (daystage_t)reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnSetDayStage(stage);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SetSpecialFog( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	bool enablespecialfog = (reader.ReadByte() == 1) ? true : false;

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnSetSpecialFog(enablespecialfog);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_FreeEntityData( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	entindex_t entindex = reader.ReadInt16();
	Int16 flags = reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnFreeEntityData(entindex, flags);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_AddLightStyle( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	bool interpolate = (reader.ReadByte() == 1) ? true : false;
	Int32 styleindex = reader.ReadByte();
	CString pattern = reader.ReadString();
	Float framerate = reader.ReadSmallFloat();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnSetLightStyle(styleindex, pattern.c_str(), interpolate, framerate);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_PrecacheFlexScript( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	flextypes_t type = (flextypes_t)reader.ReadByte();
	const Char* pstrscriptpath = reader.ReadString();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_engfuncs.pfnPrecacheFlexScript(type, pstrscriptpath);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SetEntityFlexScript( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	entindex_t entindex = reader.ReadInt16();
	const Char* pstrscriptpath = reader.ReadString();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_engfuncs.pfnSetFlexScript(entindex, pstrscriptpath);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_CreateTempEntity( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	byte type = reader.ReadByte();

	switch(type)
	{
	case TE_BREAKMODEL:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();

			Vector size;
			for(Uint32 i = 0; i < 3; i++)
				size[i] = reader.ReadSmallFloat();

			Vector velocity;
			for(Uint32 i = 0; i < 3; i++)
				velocity[i] = reader.ReadSmallFloat();

			Uint32 random = reader.ReadUint16();
			Float life = reader.ReadSmallFloat();
			Uint32 num = reader.ReadUint16();
			Uint32 modelindex = reader.ReadUint16();
			Int32 sound = reader.ReadByte();
			Float bouyancy = reader.ReadSmallFloat();
			Float waterfriction = reader.ReadSmallFloat();
			Int32 flags = reader.ReadInt32();
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnBreakModel(origin, size, velocity, random, life, num, modelindex, sound, bouyancy, waterfriction, flags);
		}
		break;
	case TE_BUBBLES:
		{
			Vector mins;
			for(Uint32 i = 0; i < 3; i++)
				mins[i] = reader.ReadFloat();

			Vector maxs;
			for(Uint32 i = 0; i < 3; i++)
				maxs[i] = reader.ReadFloat();

			Float height = reader.ReadSmallFloat();
			Uint32 modelindex = reader.ReadUint16();
			Uint32 num = reader.ReadUint16();
			Float speed = reader.ReadSmallFloat();
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnBubbles(mins, maxs, height, modelindex, num, speed);
		}
		break;
	case TE_BUBBLETRAIL:
		{
			Vector start;
			for(Uint32 i = 0; i < 3; i++)
				start[i] = reader.ReadFloat();

			Vector end;
			for(Uint32 i = 0; i < 3; i++)
				end[i] = reader.ReadFloat();

			Float height = reader.ReadSmallFloat();
			Uint32 modelindex = reader.ReadUint16();
			Uint32 num = reader.ReadUint16();
			Float speed = reader.ReadSmallFloat();

			cl_efxapi.pfnBubbleTrail(start, end, height, modelindex, num, speed);
		}
		break;
	case TE_FUNNELSPRITE:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();

			Vector color;
			for(Uint32 i = 0; i < 3; i++)
				color[i] = reader.ReadSmallFloat();

			Float alpha = reader.ReadSmallFloat();

			Uint32 modelindex = reader.ReadUint16();
			bool reverse = (reader.ReadByte() == 1) ? true : false;
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnFunnelSprite(origin, color, alpha, modelindex, reverse);
		}
		break;
	case TE_SPHEREMODEL:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();

			Float speed = reader.ReadSmallFloat();
			Float life = reader.ReadSmallFloat();
			Uint32 num = reader.ReadUint16();
			Uint32 modelindex = reader.ReadUint16();
			Int32 sound = reader.ReadByte();
			Float bouyancy = reader.ReadSmallFloat();
			Float waterfriction = reader.ReadSmallFloat();

			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnSphereModel(origin, speed, life, num, modelindex, sound, bouyancy, waterfriction);
		}
		break;
	case TE_TEMPMODEL:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();

			Vector angles;
			for(Uint32 i = 0; i < 3; i++)
				angles[i] = reader.ReadSmallFloat();

			Vector velocity;
			for(Uint32 i = 0; i < 3; i++)
				velocity[i] = reader.ReadSmallFloat();

			Float life = reader.ReadSmallFloat();
			Uint32 num = reader.ReadUint16();
			Uint32 modelindex = reader.ReadUint16();
			Int32 sound = reader.ReadByte();
			Float bouyancy = reader.ReadSmallFloat();
			Float waterfriction = reader.ReadSmallFloat();
			Int32 flags = reader.ReadInt32();
			Int32 body = reader.ReadInt16();
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			for(Uint32 i = 0; i < num; i++)
			{
				tempentity_t* ptemp = cl_efxapi.pfnTempModel(origin, velocity, angles, life, modelindex, sound, bouyancy, waterfriction, flags);
				if(body != -1 && ptemp)
					ptemp->entity.curstate.body = body;
			}
		}
		break;
	case TE_TEMPSPRITE:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();

			Vector velocity;
			for(Uint32 i = 0; i < 3; i++)
				velocity[i] = reader.ReadSmallFloat();

			Float life = reader.ReadSmallFloat();
			Float scale = reader.ReadSmallFloat();
			Uint32 modelindex = reader.ReadUint16();
			rendermode_t rendermode = (rendermode_t)reader.ReadUint16();
			Uint32 renderfx = reader.ReadByte();
			Float alpha = reader.ReadSmallFloat();
			Int32 sound = reader.ReadByte();
			Int32 flags = reader.ReadByte();
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnTempSprite(origin, velocity, scale, modelindex, rendermode, renderfx, alpha, life, sound, flags);
		}
		break;
	case TE_PARTICLEEXPLOSION1:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnParticleExplosion1(origin);
		}
		break;
	case TE_PARTICLEEXPLOSION2:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();

			Int32 colorstart = reader.ReadByte();
			Int32 colorlength = reader.ReadByte();
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnParticleExplosion2(origin, colorstart, colorlength);
		}
		break;
	case TE_BLOBEXPLOSION:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnBlobExplosion(origin);
		}
		break;
	case TE_ROCKETEXPLOSION:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();

			Int32 color = reader.ReadByte();
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnRocketExplosion(origin, color);
		}
		break;
	case TE_PARTICLEEFFECT:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();

			Vector velocity;
			for(Uint32 i = 0; i < 3; i++)
				velocity[i] = reader.ReadSmallFloat();

			Int32 color = reader.ReadByte();
			Int32 count = reader.ReadUint16();
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnParticleEffect(origin, velocity, color, count);
		}
		break;
	case TE_LAVASPLASH:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnLavaSplash(origin);
		}
		break;
	case TE_TELEPORTSPLASH:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnTeleportSplash(origin);
		}
		break;
	case TE_ROCKETTRAIL:
		{
			Vector start;
			for(Uint32 i = 0; i < 3; i++)
				start[i] = reader.ReadFloat();

			Vector end;
			for(Uint32 i = 0; i < 3; i++)
				end[i] = reader.ReadFloat();

			Uint32 parttype = reader.ReadByte();

			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			cl_efxapi.pfnRocketTrail(start, end, parttype);
		}
		break;
	case TE_UNDEFINED:
	default:
			cl_engfuncs.pfnCon_Printf("%s - Unknown tempentity type '%d'.\n", __FUNCTION__, (Int32)type);
		break;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_Stamina( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Float stamina = (Float)reader.ReadByte()/255.0f;

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.SetStamina(stamina*100);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_Health( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Float healthamount = (Float)reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.SetHealth(healthamount);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_Kevlar( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Float kevlaramount = (Float)reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.SetKevlar(kevlaramount);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_MovementNoise( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Float speed = (Float)reader.ReadInt16();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.SetMovementNoise(speed);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_HealthKit( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	bool ismedkitcount = (reader.ReadByte() == 1) ? true : false;
	if(ismedkitcount)
	{
		Uint32 healthkitnb = reader.ReadByte();

		if(reader.HasError())
		{
			cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
			return false;
		}

		// Only contains medkit count
		gHUD.SetHealthkitNumber(healthkitnb);
	}
	else
	{
		// Get heal progress
		Float healprogress = reader.ReadSmallFloat()/100.0f;

		if(reader.HasError())
		{
			cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
			return false;
		}

		gHUD.SetHealProgress(healprogress);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_CurrentWeapon( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	// Read info
	Int32 state = reader.ReadByte();
	Int32 id = reader.ReadByte();
	Uint32 clip = reader.ReadInt16();
	Uint32 cone = reader.ReadByte();
	Int32 clipright = reader.ReadByte();
	Int32 clipleft = reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.SetCurrentWeapon(state, id, clip, clipright, clipleft, cone);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_WeaponList( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	
	const Char* pstrweaponname = reader.ReadString();
	Int32 ammotype = (Int32)reader.ReadChar();
	Int32 maxammo = reader.ReadInt16();
	Int32 maxclip = reader.ReadInt16();
	Uint32 slot = reader.ReadByte();
	Uint32 slotposition = reader.ReadByte();
	Int32 id = reader.ReadByte();
	Int32 flags = reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.AddWeaponList(pstrweaponname, ammotype, maxammo, maxclip, slot, slotposition, id, flags);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_AmmoCount( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	Uint32 index = reader.ReadByte();
	Uint32 count = reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.SetAmmoCount(index, count);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_AmmoPickup( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	const Char* pstrname = reader.ReadString();
	Uint32 count = reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.AmmoPickup(pstrname, count);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_WeaponPickup( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Int32 id = reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.WeaponPickup(id);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_ItemPickup( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	const Char *szName = reader.ReadString();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	// Add the weapon to the history
	gHUD.ItemPickup(szName);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SetHUDActive( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	gHUD.SetActive(reader.ReadByte() == 1 ? true : false);

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SetUsableObject( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	
	usableobject_type_t type = USABLE_OBJECT_NONE;
	byte readType = reader.ReadByte();
	switch(readType)
	{
	case 0:
		type = USABLE_OBJECT_NONE;
		break;
	case 1:
		type = USABLE_OBJECT_DEFAULT;
		break;
	case 2:
		type = USABLE_OBJECT_LOCKED;
		break;
	case 3:
		type = USABLE_OBJECT_UNUSABLE;
		break;
	default:
		cl_engfuncs.pfnCon_EPrintf("%s - Unknown type '%d' read.\n", __FUNCTION__, (Int32)type);
		return true;
	}
	
	Vector mins, maxs;
	if(type != USABLE_OBJECT_NONE)
	{
		for(Uint32 i = 0; i < 3; i++)
			mins[i] = reader.ReadFloat();

		for(Uint32 i = 0; i < 3; i++)
			maxs[i] = reader.ReadFloat();
	}

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.SetUsableObjectMinsMaxs(mins, maxs, type);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SetCountdownTimer( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	CString title;
	Double time = reader.ReadDouble();
	if(time)
		title = reader.ReadString();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.SetCountdownTime(time, title.c_str());
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SetAutoAimVector( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	Float autoAimX = reader.ReadFloat();
	Float autoAimY = reader.ReadFloat();
	bool isOnTarget = (reader.ReadByte() == 1) ? true : false;

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gDefaultView.SetAutoAim(autoAimX, autoAimY);
	gHUD.SetAutoaimVector(autoAimX, autoAimY, isOnTarget);

	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN  MsgFunc_SetFlashlight( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	bool istacticalon = (reader.ReadByte() == 1) ? true : false;
	Float tacticalamount = (Float)reader.ReadByte()/255.0f;
	
	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.SetTacticalBattery(istacticalon, tacticalamount*100);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_ScreenShake( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Float amplitude = (Float)reader.ReadSmallFloat()/10.0f;
	Float duration = (Float)reader.ReadSmallFloat()/10.0f;
	Float frequency = (Float)reader.ReadSmallFloat()/10.0f;

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gShake.AddScreenShake(amplitude, duration, frequency);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_RadioMessage( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	color32_t color;

	entindex_t entindex = reader.ReadInt16();
	CString callername = reader.ReadString();
	Float lifetime = reader.ReadSmallFloat();

	color.r = reader.ReadByte();
	color.g = reader.ReadByte();
	color.b = reader.ReadByte();
	color.a = reader.ReadByte();
	
	if(color.a == 0)
		color.a = 255;

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.AddRadioMessage(callername.c_str(), color, lifetime, entindex);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_CreateGameUIWindow( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	gameui_windows_t type = (gameui_windows_t)reader.ReadByte();
	switch(type)
	{
	case GAMEUI_TEXTWINDOW:
		{
			// Read text file path
			CString textfilepath = reader.ReadString();
			if(textfilepath.empty())
			{
				cl_engfuncs.pfnCon_EPrintf("%s - No text file specified for 'CGameUITextWindow'.\n", __FUNCTION__);
				return true;
			}

			// Read code
			CString passcode;
			if(reader.ReadByte() != 0)
				passcode = reader.ReadString();

			// Check for errors
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			// Spawn the window
			CGameUITextWindow* pWindow = reinterpret_cast<CGameUITextWindow*>(gGameUIManager.SpawnWindow(type));
			if(!pWindow)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Failed to create 'CGameUITextWindow'.\n", __FUNCTION__);
				return true;
			}

			if(!pWindow->initData(textfilepath.c_str(), passcode.c_str()))
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Failed to initialize 'CGameUITextWindow'.\n", __FUNCTION__);
				return true;
			}

			// Check for errors
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}
		}
		break;
	case GAMEUI_KEYPADWINDOW:
		{
			// Read text file path
			CString passcode = reader.ReadString();
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			// Spawn the window
			CGameUIKeypadWindow* pWindow = reinterpret_cast<CGameUIKeypadWindow*>(gGameUIManager.SpawnWindow(type));
			if(!pWindow)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Failed to create 'CGameUIKeypadWindow'.\n", __FUNCTION__);
				return true;
			}

			if(!pWindow->initData(passcode.c_str(), nullptr, (reader.ReadByte() == 1) ? true : false))
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Failed to initialize 'CGameUIKeypadWindow'.\n", __FUNCTION__);
				return true;
			}

			// Check for errors
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}
		}
		break;
	case GAMEUI_LOGINWINDOW:
		{
			// Read text file path
			CString username = reader.ReadString();
			if(username.empty())
			{
				cl_engfuncs.pfnCon_EPrintf("%s - No username specified for 'CGameUILoginWindow'.\n", __FUNCTION__);
				return true;
			}

			// Read text file path
			CString password = reader.ReadString();

			// Spawn the window
			CGameUILoginWindow* pWindow = reinterpret_cast<CGameUILoginWindow*>(gGameUIManager.SpawnWindow(type));
			if(!pWindow)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Failed to create 'CGameUILoginWindow'.\n", __FUNCTION__);
				return true;
			}

			if(!pWindow->initData(username.c_str(), password.c_str(), nullptr, nullptr, (reader.ReadByte() == 1) ? true : false))
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Failed to initialize 'CGameUILoginWindow'.\n", __FUNCTION__);
				return true;
			}

			// Check for errors
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}
		}
		break;
	case GAMEUI_SUBWAYWINDOW:
		{
			// Read text file path
			CString scriptfilepath = reader.ReadString();
			if(scriptfilepath.empty())
			{
				cl_engfuncs.pfnCon_EPrintf("%s - No text file specified for 'CGameUISubwayWindow'.\n", __FUNCTION__);
				return true;
			}

			// Read flags
			Int32 flags = reader.ReadByte();

			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			// Spawn the window
			CGameUISubwayWindow* pWindow = reinterpret_cast<CGameUISubwayWindow*>(gGameUIManager.SpawnWindow(type));
			if(!pWindow)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Failed to create 'CGameUISubwayWindow'.\n", __FUNCTION__);
				return true;
			}

			if(!pWindow->initData(scriptfilepath.c_str(), flags))
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Failed to initialize 'CGameUISubwayWindow'.\n", __FUNCTION__);
				return true;
			}
		}
		break;
	case GAMEUI_OBJECTIVESWINDOW:
		{
			// Read nb of objectives
			Uint32 nbObjectives = reader.ReadByte();
			// Read new objective bits
			Int16 newObjectiveBits = reader.ReadByte();

			// Read each objective in
			CArray<CString> objectivesArray;
			for(Uint32 i = 0; i < nbObjectives; i++)
			{
				const Char* pString = reader.ReadString();
				if(!qstrlen(pString))
				{
					cl_engfuncs.pfnCon_EPrintf("%s - No identifier specified for 'CGameUIObjectivesWindow' objective at index %d.\n", __FUNCTION__, i);
					return true;
				}

				objectivesArray.push_back(pString);
			}

			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			// Spawn the window
			CGameUIObjectivesWindow* pWindow = reinterpret_cast<CGameUIObjectivesWindow*>(gGameUIManager.SpawnWindow(type));
			if(!pWindow)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Failed to create 'CGameUIObjectivesWindow'.\n", __FUNCTION__);
				return true;
			}

			if(!pWindow->initData(objectivesArray, nullptr, newObjectiveBits))
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Failed to initialize 'CGameUIObjectivesWindow'.\n", __FUNCTION__);
				return true;
			}
		}
		break;
	default:
		cl_engfuncs.pfnCon_Printf("% - Unknown window type '%d' specified.\n", __FUNCTION__, (Int32)type);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SetFOV( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Uint32 desiredFOV = reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gDefaultView.SetFOV(desiredFOV);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_TriggerZoom( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Uint32 desiredFOV = reader.ReadByte();
	Float blenddelta = reader.ReadSmallFloat();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gDefaultView.SetFOVZoom(desiredFOV, blenddelta);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_Ladder( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	gLadder.ProcessMessage(pdata, msgsize);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_MotorBike( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	gMotorBike.ProcessMessage(pdata, msgsize);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_ViewController( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	gViewController.ProcessMessage(pdata, msgsize);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_ViewModel( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	gViewModel.ProcessMessage(pdata, msgsize);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_NodeDebug( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	node_msg_type_t msgtype = (node_msg_type_t)reader.ReadByte();
	switch(msgtype)
	{
	case NODE_DEBUG_PATHS:
		{
			Vector node1;
			for(Uint32 i = 0; i < 3; i++)
				node1[i] = reader.ReadFloat();

			Vector node2;
			for(Uint32 i = 0; i < 3; i++)
				node2[i] = reader.ReadFloat();

			Vector color;
			for(Uint32 i = 0; i < 3; i++)
				color[i] = (Float)reader.ReadByte()/255.0f;

			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			gNodeDebug.AddPath(node1, node2, color);
		}
		break;
	case NODE_DEBUG_BBOX:
		{
			Vector origin;
			for(Uint32 i = 0; i < 3; i++)
				origin[i] = reader.ReadFloat();

			Vector mins;
			for(Uint32 i = 0; i < 3; i++)
				mins[i] = reader.ReadFloat();

			Vector maxs;
			for(Uint32 i = 0; i < 3; i++)
				maxs[i] = reader.ReadFloat();
			
			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			gNodeDebug.AddBBox(origin, mins, maxs);
		}
		break;
	case NODE_DEBUG_WAYPOINT:
		{
			waypoint_type_t type = (waypoint_type_t)reader.ReadByte();
			entindex_t entindex = reader.ReadInt32();
			Uint32 numpoints = reader.ReadUint16();

			// Allocate
			CArray<Vector> points;
			points.resize(numpoints);

			for(Uint32 i = 0; i < numpoints; i++)
			{
				Vector point;
				for(Uint32 j = 0; j < 3; j++)
					point[j] = reader.ReadFloat();

				points[i] = point;
			}

			if(reader.HasError())
			{
				cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}

			gNodeDebug.AddWaypoint(points, entindex, type);
		}
		break;
	default:
		cl_engfuncs.pfnCon_Printf("%s - Unknown message type %d.\n", __FUNCTION__, (Int32)msgtype);
		break;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_MotionBlur( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	Float blurfade = 0;
	bool ison = (reader.ReadByte() == 1) ? true : false;
	if(ison)
		blurfade = reader.ReadSmallFloat();
	bool bluroverride = (reader.ReadByte() == 1) ? true : false;

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnSetMotionBlur(ison, blurfade, bluroverride);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_ShowMessage( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	const Char* pstrmsgname = reader.ReadString();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gMessages.ShowMessage(pstrmsgname);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_ShowCustomMessage( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	Float xpos = reader.ReadSmallFloat();
	Float ypos = reader.ReadSmallFloat();

	Int32 effect = reader.ReadByte();
	
	color24_t color1;
	color1.r = reader.ReadByte();
	color1.g = reader.ReadByte();
	color1.b = reader.ReadByte();

	color24_t color2;
	color2.r = reader.ReadByte();
	color2.g = reader.ReadByte();
	color2.b = reader.ReadByte();

	Float fadein = reader.ReadSmallFloat();
	Float fadeout = reader.ReadSmallFloat();
	Float holdtime = reader.ReadSmallFloat();
	Float fxtime = reader.ReadSmallFloat();
	Int32 channel = reader.ReadByte();

	const Char* pstrText = reader.ReadString();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gMessages.ShowMessage(pstrText, 
		fadein, 
		fadeout, 
		fxtime, 
		holdtime, 
		(CScreenMessages::effects_t)effect, 
		channel,
		xpos,
		ypos,
		color1, color2);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_ScreenFade( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	Float duration = reader.ReadSmallFloat()/100.0f;
	Float holdtime = reader.ReadSmallFloat()/100.0f;
	Int32 flags = reader.ReadInt16();

	color24_t color;
	color.r = reader.ReadByte();
	color.g = reader.ReadByte();
	color.b = reader.ReadByte();
	byte a = reader.ReadByte();

	byte layer = reader.ReadByte();
	Float timeoffset = reader.ReadSmallFloat();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnSetFade(layer, duration, holdtime, flags, color, a, timeoffset);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SetViewEntity( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	entindex_t entityIndex = reader.ReadInt16();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gDefaultView.SetViewEntity(entityIndex);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_ScreenText( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	const Char* pstrText = reader.ReadString();
	Int32 xcoord = reader.ReadInt16();
	Int32 ycoord = reader.ReadInt16();
	Float lifetime = reader.ReadSmallFloat();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gScreenText.AddText(pstrText, xcoord, ycoord, lifetime);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_TempBeam( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	beam_msgtype_t type = (beam_msgtype_t)reader.ReadByte();
	switch(type)
	{
	case BEAM_MSG_BEAMPOINTS:
	case BEAM_MSG_BEAMENTPOINT:
	case BEAM_MSG_BEAMENTS:
		{
			Vector startpos, endpos;
			entindex_t startentity = NO_ENTITY_INDEX;
			entindex_t endentity = NO_ENTITY_INDEX;
			Int32 attachment1 = NO_ATTACHMENT_INDEX;
			Int32 attachment2 = NO_ATTACHMENT_INDEX;

			if(type == BEAM_MSG_BEAMENTS)
			{
				startentity = reader.ReadInt16();
				attachment1 = reader.ReadChar();
				endentity = reader.ReadInt16();
				attachment2 = reader.ReadChar();
			}
			else if(type == BEAM_MSG_BEAMENTPOINT)
			{
				startentity = reader.ReadInt16();
				attachment1 = reader.ReadChar();

				for(Uint32 i = 0; i < 3; i++)
					endpos[i] = reader.ReadFloat();
			}
			else
			{
				for(Uint32 i = 0; i < 3; i++)
					startpos[i] = reader.ReadFloat();

				for(Uint32 i = 0; i < 3; i++)
					endpos[i] = reader.ReadFloat();
			}

			Int32 modelindex = reader.ReadInt16();
			Uint32 startframe = reader.ReadByte();
			Float framerate = reader.ReadSmallFloat()/10.0f;
			Float life = reader.ReadSmallFloat()/10.0f;
			Float width = reader.ReadSmallFloat()/10.0f;
			Float amplitude = (Float)reader.ReadByte()/100.0f;

			Float r = (Float)reader.ReadByte()/255.0f;
			Float g = (Float)reader.ReadByte()/255.0f;
			Float b = (Float)reader.ReadByte()/255.0f;

			Float brightness = (Float)reader.ReadByte()/255.0f;
			Float speed = reader.ReadSmallFloat()/10.0f;
			Float noisespeed = reader.ReadSmallFloat()/10.0f;
			
			Int32 flags = reader.ReadInt32();

			switch(type)
			{
			case BEAM_MSG_BEAMPOINTS:
				cl_efxapi.pfnBeamPoints(startpos, endpos, modelindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, flags);
				break;
			case BEAM_MSG_BEAMENTPOINT:
				cl_efxapi.pfnBeamEntityPoint(startentity, attachment1, endpos, modelindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, flags);
				break;
			case BEAM_MSG_BEAMENTS:
				cl_efxapi.pfnBeamEntities(startentity, endentity, attachment1, attachment2, modelindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, flags);
				break;
			}
		}
		break;
	case BEAM_MSG_BEAMSPRITE:
		{
			Vector startpos, endpos;
			for(Uint32 i = 0; i < 3; i++)
				startpos[i] = reader.ReadFloat();

			for(Uint32 i = 0; i < 3; i++)
				endpos[i] = reader.ReadFloat();

			Int32 modelindex = reader.ReadInt16();
			Uint32 startframe = reader.ReadByte();
			Float framerate = reader.ReadSmallFloat()/10.0f;
			Float life = reader.ReadSmallFloat()/10.0f;
			Float width = reader.ReadSmallFloat()/10.0f;
			Float amplitude = (Float)reader.ReadByte()/100.0f;

			Float r = (Float)reader.ReadByte()/255.0f;
			Float g = (Float)reader.ReadByte()/255.0f;
			Float b = (Float)reader.ReadByte()/255.0f;

			Float brightness = (Float)reader.ReadByte()/255.0f;
			Float speed = reader.ReadSmallFloat()/10.0f;
			Float noisespeed = reader.ReadSmallFloat()/10.0f;

			Int32 flags = reader.ReadInt32();

			Int32 sprmodelindex = reader.ReadInt16();
			Float spritescale = reader.ReadSmallFloat();
			rendermode_t rendermode = (rendermode_t)reader.ReadByte();
			Float spralpha = (Float)reader.ReadByte()/255.0f;

			cl_efxapi.pfnBeamPoints(startpos, endpos, modelindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, flags);
			tempentity_t* pSprite = cl_efxapi.pfnTempSprite(endpos, ZERO_VECTOR, spritescale, sprmodelindex, rendermode, RenderFx_None, spralpha, life, TE_BOUNCE_NONE, TE_FL_NONE);
			pSprite->entity.curstate.rendercolor = Vector(r, g, b);
		}
		break;
	case BEAM_MSG_BEAMDISK:
	case BEAM_MSG_BEAMCYLINDER:
	case BEAM_MSG_BEAMTORUS:
		{
			Vector startpos, endpos;
			for(Uint32 i = 0; i < 3; i++)
				startpos[i] = reader.ReadFloat();

			for(Uint32 i = 0; i < 3; i++)
				endpos[i] = reader.ReadFloat();

			Int32 modelindex = reader.ReadInt16();
			Uint32 startframe = reader.ReadByte();
			Float framerate = reader.ReadSmallFloat()/10.0f;
			Float life = reader.ReadSmallFloat()/10.0f;
			Float width = reader.ReadSmallFloat()/10.0f;
			Float amplitude = (Float)reader.ReadByte()/100.0f;

			Float r = (Float)reader.ReadByte()/255.0f;
			Float g = (Float)reader.ReadByte()/255.0f;
			Float b = (Float)reader.ReadByte()/255.0f;

			Float brightness = (Float)reader.ReadByte()/255.0f;
			Float speed = reader.ReadSmallFloat()/10.0f;
			Float noisespeed = reader.ReadSmallFloat()/10.0f;

			Int32 flags = reader.ReadInt32();

			beam_types_t beamtype;
			switch(type)
			{
				case BEAM_MSG_BEAMDISK:
					beamtype = BEAM_DISK;
					break;
				case BEAM_MSG_BEAMCYLINDER:
					beamtype = BEAM_CYLINDER;
					break;
				case BEAM_MSG_BEAMTORUS:
					beamtype = BEAM_TORUS;
					break;
				default:
					return false;
			}

			cl_efxapi.pfnBeamCirclePoints(beamtype, startpos, endpos, modelindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, flags);
		}
		break;
	case BEAM_MSG_BEAMFOLLOW:
		{
			entindex_t startentity = reader.ReadInt16();
			Int32 attachment = reader.ReadChar();

			Int32 modelindex = reader.ReadInt16();
			Float life = reader.ReadSmallFloat()/10.0f;
			Float width = reader.ReadSmallFloat()/10.0f;

			Float r = (Float)reader.ReadByte()/255.0f;
			Float g = (Float)reader.ReadByte()/255.0f;
			Float b = (Float)reader.ReadByte()/255.0f;
			Float brightness = (Float)reader.ReadByte()/255.0f;
			
			cl_efxapi.pfnBeamFollow(startentity, attachment, modelindex, life, width, brightness, r, g, b);
		}
		break;
	case BEAM_MSG_BEAMRING:
		{
			entindex_t startentity = reader.ReadInt16();
			Int32 attachment1 = reader.ReadChar();
			entindex_t endentity = reader.ReadInt16();
			Int32 attachment2 = reader.ReadChar();

			Int32 modelindex = reader.ReadInt16();
			Uint32 startframe = reader.ReadByte();
			Float framerate = reader.ReadSmallFloat()/10.0f;
			Float life = reader.ReadSmallFloat()/10.0f;
			Float width = reader.ReadSmallFloat()/10.0f;
			Float amplitude = (Float)reader.ReadByte()/100.0f;

			Float r = (Float)reader.ReadByte()/255.0f;
			Float g = (Float)reader.ReadByte()/255.0f;
			Float b = (Float)reader.ReadByte()/255.0f;

			Float brightness = (Float)reader.ReadByte()/255.0f;
			Float speed = reader.ReadSmallFloat()/10.0f;
			Float noisespeed = reader.ReadSmallFloat()/10.0f;

			Int32 flags = reader.ReadInt32();

			cl_efxapi.pfnBeamRing(startentity, endentity, attachment1, attachment2, modelindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, flags);
		}
		break;
	case BEAM_MSG_KILLENTITYBEAMS:
		{
			entindex_t entityindex = reader.ReadInt16();
			cl_efxapi.pfnKillEntityBeams(entityindex);
		}
		break;
	default:
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Unknown beam message type '%d'.\n", __FUNCTION__, type);
			return false;
		}
		break;
	}

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_BlackHole( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	blackhole_msg_types_t type = (blackhole_msg_types_t)reader.ReadByte();
	Int32 key = reader.ReadInt16();

	switch(type)
	{
	case MSG_BLACKHOLE_SPAWN:
		{
			Vector origin;
			origin.x = reader.ReadFloat();
			origin.y = reader.ReadFloat();
			origin.z = reader.ReadFloat();

			Float life = reader.ReadFloat();
			Float scale = reader.ReadFloat();
			Float strength = reader.ReadFloat();
			Float rotation = reader.ReadFloat();
			Float growthtime = reader.ReadSmallFloat();
			Float shrinktime = reader.ReadSmallFloat();

			cl_efxapi.pfnCreateBlackHole( key, origin, life, scale, strength, rotation, growthtime, shrinktime);
		}
		break;
	case MSG_BLACKHOLE_KILL:
		{
			cl_efxapi.pfnKillBlackHole(key);
		}
		break;
	default:
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Unknown black hole message type '%d'.\n", __FUNCTION__, (Int32)type);
			return false;
		}
		break;
	}

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SunFlare( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	entindex_t entindex = reader.ReadInt16();
	bool isActive = (reader.ReadByte() == 0) ? false : true;
	
	Float pitch = 0;
	Float roll = 0;
	Float scale = 0;
	Vector color;
	bool portalSunFlare = false;

	if(isActive)
	{
		pitch = reader.ReadSmallFloat();
		roll = reader.ReadSmallFloat();
		scale = reader.ReadSmallFloat();

		for(Uint32 i = 0; i < 3; i++)
			color[i] = reader.ReadByte();

		portalSunFlare = (reader.ReadByte() == 0) ? false : true;
	}

	cl_efxapi.pfnSetSunFlare(entindex, isActive, pitch, roll, scale, color, portalSunFlare);

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_VaporTrail( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	Vector startpos;
	entindex_t entindex = reader.ReadInt16();
	Int32 attachment = 0;

	if(entindex != NO_ENTITY_INDEX)
	{
		// Use player attachment
		attachment = reader.ReadByte();
	}
	else
	{
		for(Uint32 i = 0; i < 3; i++)
			startpos[i] = reader.ReadFloat();
	}

	Vector endpos;
	for(Uint32 i = 0; i < 3; i++)
		endpos[i] = reader.ReadFloat();

	Int32 modelindex1 = reader.ReadInt16();
	Int32 modelindex2 = reader.ReadInt16();
	Float life = reader.ReadByte() * 0.1;
	Float width = reader.ReadByte() * 0.1;
	Float brightness = (Float)reader.ReadByte() / 255.0f;

	Float colorfadedelay = 0;
	Float colorfadetime = 0;

	Vector color1, color2;
	if(modelindex1 == modelindex2)
	{
		for(Uint32 i = 0; i < 3; i++)
			color1[i] = color2[i] = (Float)reader.ReadByte() / 255.0f;
	}
	else
	{
		for(Uint32 i = 0; i < 3; i++)
			color1[i] = (Float)reader.ReadByte() / 255.0f;

		for(Uint32 i = 0; i < 3; i++)
			color2[i] = (Float)reader.ReadByte() / 255.0f;

		colorfadedelay = (Float)reader.ReadByte() * 0.1;
		colorfadetime = (Float)reader.ReadByte() * 0.1;
	}

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	if(entindex != NO_ENTITY_INDEX)
	{
		cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
		if(entindex == pplayer->entindex)
		{
			cl_entity_t* pviewmodel = gViewModel.GetViewModel();
			if(!pviewmodel->pmodel)
				return true;

			cl_engfuncs.pfnUpdateAttachments(pviewmodel);
			startpos = pviewmodel->getAttachment(attachment);
		}
		else
		{
			cl_entity_t* pentity = cl_engfuncs.pfnGetEntityByIndex(entindex);
			if(!pentity)
				return true;

			startpos = pentity->curstate.origin + pentity->curstate.view_offset;
		}
	}

	cl_efxapi.pfnBeamVaporTrail(startpos, endpos, modelindex1, modelindex2, colorfadedelay, colorfadetime, life, width, brightness, color1.x, color1.y, color1.z, color2.x, color2.y, color2.z, FL_BEAM_NONE);

	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_NPCAwareness( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Float awareness = (Float)reader.ReadSmallFloat();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.SetNPCAwareness(awareness);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_NewObjective( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	bool newObjective = reader.ReadByte() ? true : false;

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	gHUD.SetNewObjective(newObjective);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_AddSkyTextureSet( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	
	CString textureName = reader.ReadString();
	Int32 setIndex = reader.ReadByte();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnAddSkyTextureSet(textureName.c_str(), setIndex);
	return true;
}

//=============================================
// @brief
//
//=============================================
MSGFN MsgFunc_SetSkyTexture( const Char* pstrName, const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);
	Int32 setIndex = reader.ReadChar();

	if(reader.HasError())
	{
		cl_engfuncs.pfnCon_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	cl_efxapi.pfnSetSkyTexture(setIndex);
	return true;
}