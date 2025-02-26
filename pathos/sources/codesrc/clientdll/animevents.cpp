/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "clientdll.h"
#include "dlight.h"
#include "cl_entity.h"
#include "cache_model.h"
#include "com_math.h"
#include "tempentity.h"
#include "stepsound.h"
#include "snd_shared.h"
#include "com_math.h"
#include "weapons_shared.h"
#include "beam_shared.h"

// Vapor trail smoke sprite
static const Char VAPORTRAIL_SMOKE_SPRITE[] = "sprites/smoke.spr";
// Vapor trail glow sprite
static const Char VAPORTRAIL_GLOW_SPRITE[] = "sprites/laserbeam.spr";

//=============================================
// @brief
//
//=============================================
void EV_MuzzleFlashLight( const Vector& position, Int32 key )
{
	Float radius = Common::RandomFloat( 110, 180 );

	cl_dlight_t *pdl = cl_efxapi.pfnAllocDynamicPointLight( key, 0, false, true, nullptr );
	pdl->die = cl_engfuncs.pfnGetClientTime() + 0.1;
	pdl->origin = position;
	pdl->color.x = 0.98;
	pdl->color.y = 0.75;
	pdl->color.z = 0.25;
	pdl->radius = radius;
	pdl->decay = -radius*2;
}

//=============================================
// @brief
//
//=============================================
void EV_SimpleMuzzleFlash( const Vector& position, const Vector& angles, entindex_t entindex, Int32 attachment )
{
	EV_MuzzleFlashLight(position, entindex);

	Vector forward;
	Math::AngleVectors(angles, &forward);

	cl_efxapi.pfnRemoveParticleSystem(entindex, entindex, true);
	cl_efxapi.pfnSpawnParticleSystem(position, forward, PART_SCRIPT_CLUSTER, MUZZLEFLASH_PARTICLE_SCRIPT_SIMPLE, attachment+1, entindex, attachment, NO_POSITION, PARTICLE_ATTACH_ATTACHMENT_VECTOR|PARTICLE_ATTACH_RELATIVE);
	cl_efxapi.pfnSpawnParticleSystem(position, forward, PART_SCRIPT_SYSTEM, MUZZLE_PARTICLE_SMOKE, attachment+1, entindex, attachment, NO_POSITION, PARTICLE_ATTACH_ATTACHMENT_VECTOR|PARTICLE_ATTACH_RELATIVE);
}

//=============================================
// @brief
//
//=============================================
void EV_AttachmentDirectionMuzzleFlash( const cl_entity_t *pentity, const Char* pstroptions, Int32 attachment1, Int32 attachment2 )
{
	Vector attach1 = pentity->getAttachment(attachment1);
	Vector attach2 = pentity->getAttachment(attachment2);
	Vector direction = (attach2 - attach1).Normalize();

	if (!qstrstr(pstroptions, "silencer"))
		EV_MuzzleFlashLight(attach1, pentity->entindex);

	CString filepath(MUZZLEFLASH_PARTICLE_SCRIPT_BASENAME);
	Int32 tokenpos = filepath.find(0, "%s");
	if(tokenpos == CString::CSTRING_NO_POSITION)
	{
		cl_engfuncs.pfnCon_Printf("%s - String token not found in string '%s'.\n", __FUNCTION__, MUZZLEFLASH_PARTICLE_SCRIPT_BASENAME);
		return;
	}

	CString replacestring(filepath);
	replacestring.erase(tokenpos, 2);
	replacestring.insert(tokenpos, pstroptions);

	cl_efxapi.pfnSpawnParticleSystem(attach1, direction, PART_SCRIPT_CLUSTER, replacestring.c_str(), attachment1+1, pentity->entindex, attachment1, NO_POSITION, (PARTICLE_ATTACH_TO_PARENT|PARTICLE_ATTACH_TO_ATTACHMENT|PARTICLE_ATTACH_RELATIVE));
}

//=============================================
// @brief
//
//=============================================
void EV_AngleForwardMuzzleFlash( Int32 attachment, const Vector origin, const cl_entity_t *pentity, const mstudioevent_t *pevent )
{
	Vector angles, forward;
	angles = pentity->curstate.angles;

	// Reverse pitch on view model
	angles[PITCH] = -angles[PITCH];
	Math::AngleVectors(angles, &forward, nullptr, nullptr);
	forward.Normalize();

	// Create dynamic light
	if (!qstrstr(pevent->options, "silencer"))
		EV_MuzzleFlashLight(origin, pentity->entindex);

	CString filepath(MUZZLEFLASH_PARTICLE_SCRIPT_BASENAME);
	Int32 tokenpos = filepath.find(0, "%s");
	if(tokenpos == CString::CSTRING_NO_POSITION)
	{
		cl_engfuncs.pfnCon_Printf("%s - String token not found in string '%s'.\n", __FUNCTION__, MUZZLEFLASH_PARTICLE_SCRIPT_BASENAME);
		return;
	}

	CString replacestring(filepath);
	replacestring.erase(tokenpos, 2);
	replacestring.insert(tokenpos, pevent->options);

	// Delete previous attached particle systems
	cl_efxapi.pfnRemoveParticleSystem(attachment+1, pentity->entindex, true);
	cl_efxapi.pfnSpawnParticleSystem(origin, forward, PART_SCRIPT_CLUSTER, replacestring.c_str(), attachment+1, pentity->entindex, attachment, NO_POSITION, (PARTICLE_ATTACH_TO_PARENT|PARTICLE_ATTACH_TO_ATTACHMENT|PARTICLE_ATTACH_RELATIVE));
	cl_efxapi.pfnSpawnParticleSystem(origin, forward, PART_SCRIPT_SYSTEM, MUZZLE_PARTICLE_SMOKE, attachment+1, pentity->entindex, attachment, NO_POSITION, PARTICLE_ATTACH_NONE);
}

//=============================================
// @brief
//
//=============================================
void EV_EjectBullet( const Vector& attachment, const mstudioevent_t *pevent, const cl_entity_t *pentity )
{
	// Load model
	const cache_model_t* pmodel = cl_engfuncs.pfnLoadModel(BULLET_CASINGS_MODEL);
	if(!pmodel)
	{
		cl_engfuncs.pfnCon_Printf("%s - Model '%s' could not be loaded.\n", __FUNCTION__, BULLET_CASINGS_MODEL);
		return;
	}

	// Get the proper entity for velocity
	Vector velocity, angles;
	if(pentity->entindex == VIEWMODEL_ENTITY_INDEX)
	{
		cl_entity_t* pvelentity = cl_engfuncs.pfnGetLocalPlayer();
		velocity = pvelentity->curstate.velocity;
		angles = pvelentity->curstate.viewangles;
	}
	else
	{
		velocity = pentity->curstate.velocity;
		angles = pentity->curstate.angles;
	}

	Vector forward, right, up;
	Math::AngleVectors(angles, &forward, &right, &up);

	Vector origin(attachment);
	if(pentity->entindex != VIEWMODEL_ENTITY_INDEX)
		origin -= forward * 8;

	Float upvel = Common::RandomFloat(50, 70);
	Float rightvel = Common::RandomFloat(70, 90);

	for(Uint32 i = 0; i < 3; i++)
		velocity[i] = velocity[i] + right[i] * rightvel + up[i] * upvel + forward[i] * 20;

	const vbmheader_t* pvbmheader = pmodel->getVBMCache()->pvbmhdr;

	Int64 bodyvalue = -1;
	for(Uint32 i = 0; i < pvbmheader->numbodyparts; i++)
	{
		const vbmbodypart_t* pbodypart = pvbmheader->getBodyPart(i);
		Uint32 j = 0;
		for(; j < pbodypart->numsubmodels; j++)
		{
			const vbmsubmodel_t* psubmodel = pbodypart->getSubmodel(pvbmheader, j);
			if(!qstrcmp(psubmodel->name, pevent->options))
			{
				bodyvalue = j*pbodypart->base;
				break;
			}
		}

		if(j != (Uint32)pbodypart->numsubmodels)
			break;
	}

	if(bodyvalue == -1)
	{
		cl_engfuncs.pfnCon_Printf("%s - Submodel '%s' not found in '%s'.\n", __FUNCTION__, pevent->options, BULLETS_MODEL_FILENAME);
		return;
	}

	bool isBuckshot = !qstrcmp(pevent->options, "buckshot_reference") ? true : false;

	// Set params
	Int32 soundtype = isBuckshot ? TE_BOUNCE_SHOTSHELL : TE_BOUNCE_SHELL;
	Float bouyancy = isBuckshot ? 500 : 300;
	Float waterfriction = isBuckshot ? 1.0 : 0.3;

	tempentity_t* ptempentity = cl_efxapi.pfnTempModel(origin, velocity, pentity->curstate.angles, 10, pmodel->cacheindex, soundtype, bouyancy, waterfriction, 0);
	if(ptempentity)
		ptempentity->entity.curstate.body = bodyvalue;
}

//=============================================
// @brief
//
//=============================================
void EV_NPC_StepSound( cl_entity_t *pentity, const Char* pstrsteptype )
{
	// Don't play for entities outside the PVS
	cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
	if(pplayer->curstate.msg_num != pentity->curstate.msg_num)
		return;

	trace_t tr;
	Vector traceStart = pentity->curstate.origin + Vector(0, 0, 32);
	Vector traceEnd = pentity->curstate.origin - Vector(0, 0, 32);

	cl_tracefuncs.pfnPlayerTrace(traceStart, traceEnd, FL_TRACE_NO_MODELS, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.noHit() || tr.allSolid() || tr.startSolid())
		return;

	const Char* pstrTextureName = cl_tracefuncs.pfnTraceTexture(tr.hitentity, tr.endpos+tr.plane.normal, tr.endpos-tr.plane.normal);
	if(!pstrTextureName)
		return;

	const en_material_t* pmaterial = cl_engfuncs.pfnGetMapTextureMaterialScript(pstrTextureName);
	if(!pmaterial)
		return;

	if(!qstrcmp(pstrsteptype, "enlightened") && !qstrcmp(pmaterial->materialname, "concrete"))
		return;

	// Determine foot and also set
	CStepSound::foot_t foot = pentity->curstate.stepleft ? CStepSound::FOOT_LEFT : CStepSound::FOOT_RIGHT;
	if(pentity->curstate.stepleft)
		pentity->curstate.stepleft = false;
	else
		pentity->curstate.stepleft = true;

	if(pmaterial->flags & TX_FL_NO_STEPSOUND)
		return;

	const CArray<CString>* pstepsounds = g_stepSound.GetFootSoundList(foot, pmaterial->materialname.c_str(), pstrsteptype);
	if(!pstepsounds || pstepsounds->empty())
		return;

	Uint32 soundindex = Common::RandomLong(0, pstepsounds->size()-1);
	const CString& stepsnd = (*pstepsounds)[soundindex];

	// Play the random sound
	cl_engfuncs.pfnPlayEntitySound(pentity->entindex, SND_CHAN_BODY, stepsnd.c_str(), VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_NONE, 0);
}

//=============================================
// @brief
//
//=============================================
void EV_NPC_StepSound_Legacy( cl_entity_t *pentity, const mstudioevent_t *pevent )
{
	if(!Common::IsNumber(pevent->options))
	{
		cl_engfuncs.pfnCon_Printf("%s - Option '%s' for event %d is not a valid numerical value.\n", __FUNCTION__, pevent->options, pevent->event);
		return;
	}

	Int32 optionvalue = SDL_atoi(pevent->options);
	if(optionvalue < 0 || optionvalue >= NB_NPC_LEGACY_STEPTYPES)
	{
		cl_engfuncs.pfnCon_Printf("%s - Option value '%d' for event %d is not a valid value.\n", __FUNCTION__, optionvalue, pevent->event);
		return;
	}

	EV_NPC_StepSound(pentity, NPC_LEGACY_STEPTYPE_NAMES[optionvalue]);
}

//=============================================
// @brief
//
//=============================================
void EV_ViewModelMuzzleflash( cl_entity_t* pentity, const mstudioevent_t *pevent, const Vector& position, Int32 attachment )
{
	// Create complex muzzleflash
	EV_AttachmentDirectionMuzzleFlash(pentity, pevent->options, attachment, attachment+1);

	Vector forward;
	Math::AngleVectors(pentity->curstate.angles, &forward);
	cl_efxapi.pfnSpawnParticleSystem(pentity->getAttachment(0), forward, PART_SCRIPT_SYSTEM, MUZZLE_PARTICLE_SMOKE, attachment+1, pentity->entindex, attachment, NO_POSITION, PARTICLE_ATTACH_NONE);
}

//=============================================
// @brief
//
//=============================================
void EV_ViewModelMuzzleflash_UserSpecifiedAttachment(cl_entity_t* pentity, const mstudioevent_t* pevent, const Vector& position)
{
	const Char* pstrString = pevent->options;
	const Char* pstrSeparator = qstrstr(pstrString, ";");
	if (!pstrSeparator)
	{
		cl_engfuncs.pfnCon_Printf("%s - Event option string '%s' is missing ';' separator.\n", __FUNCTION__, pstrString);
		return;
	}

	Uint32 length = pstrSeparator - pstrString;
	CString attachmentString(pstrString, length);
	if (attachmentString.empty())
	{
		cl_engfuncs.pfnCon_Printf("%s - Event option string '%s' is missing attachment index before ';' separator.\n", __FUNCTION__, pstrString);
		return;
	}

	if (!Common::IsNumber(attachmentString.c_str()))
	{
		cl_engfuncs.pfnCon_Printf("%s - Attachment string '%s' in event option string '%s' is not a valid number.\n", __FUNCTION__, attachmentString.c_str(), pstrString);
		return;
	}

	// Create complex muzzleflash
	Int32 attachmentIndex = SDL_atoi(attachmentString.c_str());
	EV_AttachmentDirectionMuzzleFlash(pentity, pstrSeparator+1, attachmentIndex, attachmentIndex + 1);

	Vector forward;
	Math::AngleVectors(pentity->curstate.angles, &forward);

	cl_efxapi.pfnSpawnParticleSystem(pentity->getAttachment(0), forward, PART_SCRIPT_SYSTEM, MUZZLE_PARTICLE_SMOKE, attachmentIndex + 1, pentity->entindex, attachmentIndex, NO_POSITION, PARTICLE_ATTACH_NONE);
}

//=============================================
// @brief
//
//=============================================
void EV_WeaponVaporTrail_Smoke( const Vector& attachment, const mstudioevent_t *pevent, const cl_entity_t *pentity )
{
	cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
	if(!pplayer)
		return;

	Vector vieworg = pplayer->curstate.origin + pplayer->curstate.view_offset;
	Vector angles = pplayer->curstate.viewangles + pplayer->curstate.punchangles;

	Vector forward;
	Math::AngleVectors(angles, &forward, nullptr, nullptr);

	Vector end = vieworg + forward*8192;

	trace_t tr;
	cl_tracefuncs.pfnPlayerTrace(vieworg, end, FL_TRACE_NORMAL, HULL_POINT, pplayer->entindex, tr);
	if(tr.allSolid() || tr.noHit() || tr.startSolid())
		return;

	CString option(pevent->options);
	
	CString sprit1name;
	Int32 colonPos = option.find(0, ";");
	if(colonPos == CString::CSTRING_NO_POSITION)
	{
		cl_engfuncs.pfnCon_Printf("%s - Colon character not found in '%s'.\n", __FUNCTION__, option.c_str());
		return;
	}

	CString sprname1;
	sprname1.assign(option.c_str(), colonPos);

	const cache_model_t* pmodel1 = cl_engfuncs.pfnLoadModel(sprname1.c_str());
	if(!pmodel1)
	{
		cl_engfuncs.pfnCon_Printf("Could not load sprite '%s'.\n", VAPORTRAIL_SMOKE_SPRITE);
		return;
	}

	CString sprname2;
	sprname2.assign(option.c_str() + (colonPos+1) , (option.length() - colonPos - 1));

	const cache_model_t* pmodel2 = cl_engfuncs.pfnLoadModel(sprname2.c_str());
	if(!pmodel2)
	{
		cl_engfuncs.pfnCon_Printf("Could not load sprite '%s'.\n", VAPORTRAIL_SMOKE_SPRITE);
		return;
	}

	cl_efxapi.pfnBeamVaporTrail(attachment, tr.endpos, pmodel1->cacheindex, pmodel2->cacheindex, 0, 0, 3, 2, 1.0, 1.0, 1.0, 1.0, 0.3, 0.3, 0.3, FL_BEAM_NONE);
}

//=============================================
// @brief
//
//=============================================
void EV_WeaponVaporTrail_Glow( const Vector& attachment, const mstudioevent_t *pevent, const cl_entity_t *pentity )
{
	cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
	if(!pplayer)
		return;

	Vector vieworg = pplayer->curstate.origin + pplayer->curstate.view_offset;
	Vector angles = pplayer->curstate.viewangles + pplayer->curstate.punchangles;

	Vector forward;
	Math::AngleVectors(angles, &forward, nullptr, nullptr);

	Vector end = vieworg + forward*8192;

	trace_t tr;
	cl_tracefuncs.pfnPlayerTrace(vieworg, end, FL_TRACE_NORMAL, HULL_POINT, pplayer->entindex, tr);
	if(tr.allSolid() || tr.noHit() || tr.startSolid())
		return;

	CString option(pevent->options);
	
	CString sprit1name;
	Int32 colonPos = option.find(0, ";");
	if(colonPos == CString::CSTRING_NO_POSITION)
	{
		cl_engfuncs.pfnCon_Printf("%s - Colon character not found in '%s'.\n", __FUNCTION__, option.c_str());
		return;
	}

	CString sprname1;
	sprname1.assign(option.c_str(), colonPos);

	const cache_model_t* pmodel1 = cl_engfuncs.pfnLoadModel(sprname1.c_str());
	if(!pmodel1)
	{
		cl_engfuncs.pfnCon_Printf("Could not load sprite '%s'.\n", VAPORTRAIL_SMOKE_SPRITE);
		return;
	}

	CString sprname2;
	sprname2.assign(option.c_str() + (colonPos+1) , (option.length() - colonPos - 1));

	const cache_model_t* pmodel2 = cl_engfuncs.pfnLoadModel(sprname2.c_str());
	if(!pmodel2)
	{
		cl_engfuncs.pfnCon_Printf("Could not load sprite '%s'.\n", VAPORTRAIL_SMOKE_SPRITE);
		return;
	}

	cl_efxapi.pfnBeamVaporTrail(attachment, tr.endpos, pmodel1->cacheindex, pmodel2->cacheindex, 0.5, 1, 3, 2, 1.0, 1.0, 0.8, 0.2, 0.7, 0.7, 0.7, FL_BEAM_NONE);
}