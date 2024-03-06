/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "animatingentity.h"
#include "vbmformat.h"
#include "studio.h"

//=============================================
// @brief
//
//=============================================
Int32 VBM_FindSequenceByActivity( const cache_model_t* pmodel, Int32 activity )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which is not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return NO_SEQUENCE_VALUE;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return NO_SEQUENCE_VALUE;
	}

	Int32 weightsum = 0;
	Int32 result = NO_SEQUENCE_VALUE;

	for(Int32 i = 0; i < pstudiohdr->numseq; i++)
	{
		const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(i);
		if(pseqdesc->activity != activity)
			continue;
	
		// Add to weight sum
		weightsum += pseqdesc->actweight;

		if(!weightsum || Common::RandomLong(0, weightsum-1) < pseqdesc->actweight)
			result = i;
	}

	return result;
}

//=============================================
// @brief
//
//=============================================
Int32 VBM_FindSequenceWithHeaviestActivity( const cache_model_t* pmodel, Int32 activity )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return NO_SEQUENCE_VALUE;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return NO_SEQUENCE_VALUE;
	}

	Int32 lasthighestweight = 0;
	Int32 result = NO_SEQUENCE_VALUE;

	for(Int32 i = 0; i < pstudiohdr->numseq; i++)
	{
		const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(i);
		if(pseqdesc->activity != activity)
			continue;
	
		if(pseqdesc->actweight > lasthighestweight)
		{
			lasthighestweight = pseqdesc->actweight;
			result = i;
		}
	}

	return result;
}

//=============================================
// @brief
//
//=============================================
void VBM_GetSequenceInfo( const cache_model_t* pmodel, Int32 sequence, Float* pframerate, Float* pgroundspeed )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		if(pframerate)
			(*pframerate) = 0;

		if(pgroundspeed)
			(*pgroundspeed) = 0;

		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	if(sequence < 0 || sequence > pstudiohdr->numseq)
	{
		if(pframerate)
			(*pframerate) = 0;

		if(pgroundspeed)
			(*pgroundspeed) = 0;

		gd_engfuncs.pfnCon_Printf("%s - Sequence %d is out of bounds for model '%s'.\n", __FUNCTION__, sequence, pmodel->name.c_str());
		return;
	}

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequence);
	if(!pseqdesc)
		return;

	// Set fps
	if(pframerate)
	{
		if(pseqdesc->numframes > 1)
			(*pframerate) = (256.0 * pseqdesc->fps) / (pseqdesc->numframes - 1);
		else
			(*pframerate) = 256.0;
	}

	if(pgroundspeed)
	{
		if(pseqdesc->numframes > 1)
		{
			Float groundspeed = SDL_sqrt(Math::DotProduct(pseqdesc->linearmovement, pseqdesc->linearmovement));
			(*pgroundspeed) = groundspeed*pseqdesc->fps/(pseqdesc->numframes-1);
		}
		else
		{
			(*pgroundspeed) = 0;
		}
	}
}

//=============================================
// @brief
//
//=============================================
Int32 VBM_GetSequenceFlags( const cache_model_t* pmodel, Int32 sequence )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	if(sequence < 0 || sequence > pstudiohdr->numseq)
	{
		gd_engfuncs.pfnCon_Printf("%s - Sequence %d is out of bounds for model '%s'.\n", __FUNCTION__, sequence, pmodel->name.c_str());
		return 0;
	}

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequence);
	if(!pseqdesc)
		return 0;
	else
		return pseqdesc->flags;
}

//=============================================
// @brief
//
//=============================================
Float VBM_SetBlending( const cache_model_t* pmodel, Uint32 blendingindex, Int32 sequence, Float value, Float* pblending )
{
	if(blendingindex >= MAX_BLENDING)
	{
		gd_engfuncs.pfnCon_Printf("%s - Blending index out of range.\n", __FUNCTION__);
		return value;
	}

	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return value;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return value;
	}

	if(sequence < 0 || sequence > pstudiohdr->numseq)
	{
		gd_engfuncs.pfnCon_Printf("%s - Sequence %d is out of bounds for model '%s'.\n", __FUNCTION__, sequence, pmodel->name.c_str());
		return value;
	}

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequence);
	if(!pseqdesc)
		return value;
	
	if(pseqdesc->blendtype[blendingindex] == 0)
		return value;

	if(pseqdesc->blendtype[blendingindex] & (STUDIO_XR|STUDIO_YR|STUDIO_ZR))
	{
		if(pseqdesc->blendend[blendingindex] < pseqdesc->blendstart[blendingindex])
			value *= -1;

		if(pseqdesc->blendstart[blendingindex] + 359.0 >= pseqdesc->blendend[blendingindex])
		{
			if(value > ((pseqdesc->blendstart[blendingindex] + pseqdesc->blendend[blendingindex])/2.0) + 180)
				value -= 360;
			if(value < ((pseqdesc->blendstart[blendingindex] + pseqdesc->blendend[blendingindex])/2.0) - 180)
				value += 360;
		}
	}

	Float controllersetting = 255*(value - pseqdesc->blendstart[blendingindex])/(pseqdesc->blendend[blendingindex]-pseqdesc->blendstart[blendingindex]);
	controllersetting = clamp(controllersetting, 0, 255);
	pblending[blendingindex] = controllersetting;

	return controllersetting*(1.0/255.0)*(pseqdesc->blendend[blendingindex]-pseqdesc->blendstart[blendingindex])+pseqdesc->blendstart[blendingindex];
}

//=============================================
// @brief
//
//=============================================
Uint32 VBM_GetNumSequenceBlenders( const cache_model_t* pmodel, Int32 sequence )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	if(sequence < 0 || sequence > pstudiohdr->numseq)
	{
		gd_engfuncs.pfnCon_Printf("%s - Sequence %d is out of bounds for model '%s'.\n", __FUNCTION__, sequence, pmodel->name.c_str());
		return 0;
	}

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequence);
	return (Uint32)pseqdesc->numblends;
}

//=============================================
// @brief
//
//=============================================
void VBM_GetEyePosition( const cache_model_t* pmodel, Vector& eyeposition )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	eyeposition = pstudiohdr->eyeposition;
}

//=============================================
// @brief
//
//=============================================
void VBM_PrecacheSequenceResources( const cache_model_t* pmodel, const Char* pstrsequencename )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	Int32 sequenceindex = VBM_FindSequence(pstudiohdr, pstrsequencename);
	if(sequenceindex == NO_SEQUENCE_VALUE)
	{
		gd_engfuncs.pfnCon_Printf("%s - Sequence '%s' not found in model '%s'.\n", __FUNCTION__, pstrsequencename, pmodel->name.c_str());
		return;
	}

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequenceindex);
	if(!pseqdesc)
		return;

	for(Int32 j = 0; j < pseqdesc->numevents; j++)
	{
		const mstudioevent_t* pevent = pseqdesc->getEvent(pstudiohdr, j);
		if(pevent->event >= EVENT_CLIENT_START_INDEX)
			continue;

		if(pevent->event == EVENT_SCRIPT_SOUND || pevent->event == EVENT_SCRIPT_SOUND_VOICE)
		{
			if(!qstrlen(pevent->options))
			{
				gd_engfuncs.pfnCon_Printf("%s - Empty event %d on sequence '%s' for model '%s'.\n", __FUNCTION__, pevent->event, pstrsequencename, pmodel->name.c_str());
				continue;
			}

			gd_engfuncs.pfnPrecacheSound(pevent->options);
		}
	}
}

//=============================================
// @brief
//
//=============================================
Int32 VBM_FindTransition( const cache_model_t* pmodel, Int32 endanim, Int32 goalanim, Int32* pdirection )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return goalanim;
	}

	// Get pointer to studio data
	const vbmcache_t* pvbmcache = pmodel->getVBMCache();
	studiohdr_t* pstudiohdr = pvbmcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return goalanim;
	}

	// Check for bounds
	if(endanim < 0 || endanim >= pstudiohdr->numseq)
	{
		gd_engfuncs.pfnCon_Printf("%s - End sequence %d is out of bounds for model '%s'.\n", __FUNCTION__, endanim, pmodel->name.c_str());
		return goalanim;
	}
	else if(goalanim < 0 || goalanim >= pstudiohdr->numseq)
	{
		gd_engfuncs.pfnCon_Printf("%s - End sequence %d is out of bounds for model '%s'.\n", __FUNCTION__, goalanim, pmodel->name.c_str());
		return goalanim;
	}

	const mstudioseqdesc_t* pseqdesc = reinterpret_cast<const mstudioseqdesc_t*>(reinterpret_cast<const byte*>(pstudiohdr) + pstudiohdr->seqindex);
	if(pseqdesc[endanim].entrynode == 0 || pseqdesc[goalanim].entrynode == 0)
		return goalanim;

	Int32 endnode = ((*pdirection) > 0) ? endnode = pseqdesc[endanim].exitnode : pseqdesc[endanim].entrynode;
	if(endnode == pseqdesc[goalanim].entrynode)
	{
		(*pdirection) = 1;
		return goalanim;
	}

	byte* ptransition = (reinterpret_cast<byte*>(pstudiohdr) + pstudiohdr->transitionindex);
	Int32 internnode = ptransition[(endnode-1)*pstudiohdr->numtransitions+(pseqdesc[goalanim].entrynode-1)];
	if(internnode == 0)
		return goalanim;

	for(Int32 i = 0; i < pstudiohdr->numseq; i++)
	{
		if(pseqdesc[i].entrynode == endnode && pseqdesc[i].exitnode == internnode)
		{
			(*pdirection) = 1;
			return i;
		}
		else if(pseqdesc[i].nodeflags && pseqdesc[i].exitnode == endnode && pseqdesc[i].entrynode == internnode)
		{
			(*pdirection) = -1;
			return i;
		}
	}

	gd_engfuncs.pfnCon_Printf("%s - Error in transition graph.\n", __FUNCTION__);
	return goalanim;
}

//=============================================
// @brief
//
//=============================================
void VBM_SetBodyGroup( const cache_model_t* pmodel, Int32 groupindex, Int32 value, Int64& body )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	// Get pointer to studio data
	const vbmcache_t* pvbmcache = pmodel->getVBMCache();
	vbmheader_t* pvbmheader = pvbmcache->pvbmhdr;
	if(!pvbmheader)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid vbm data.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	if(groupindex > pvbmheader->numbodyparts)
	{
		gd_engfuncs.pfnCon_Printf("%s - Overindexing bodygroup by %d for model '%s'.\n", __FUNCTION__, groupindex, pmodel->name.c_str());
		return;
	}

	const vbmbodypart_t* pbodypart = pvbmheader->getBodyPart(groupindex);
	if(value >= pbodypart->numsubmodels)
	{
		gd_engfuncs.pfnCon_Printf("%s - Overindexing submodel %d on bodygroup %d for model '%s'.\n", __FUNCTION__, value, groupindex, pmodel->name.c_str());
		return;
	}

	Int32 current = (body / pbodypart->base)%pbodypart->numsubmodels;
	body = (body - (current*pbodypart->base)+(value*pbodypart->base));
}

//=============================================
// @brief
//
//=============================================
Int32 VBM_GetBodyGroupIndexByName( const cache_model_t* pmodel, const Char* pstrBodyGroupName )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	// Get pointer to studio data
	const vbmcache_t* pvbmcache = pmodel->getVBMCache();
	vbmheader_t* pvbmheader = pvbmcache->pvbmhdr;
	if(!pvbmheader)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid vbm data.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	for(Int32 i = 0; i < pvbmheader->numbodyparts; i++)
	{
		vbmbodypart_t* pbodypart = pvbmheader->getBodyPart(i);
		if(!qstrcmp(pbodypart->name, pstrBodyGroupName))
			return i;
	}

	gd_engfuncs.pfnCon_Printf("%s - Couldn't find bodygroup '%s' in model '%s'.\n", __FUNCTION__, pstrBodyGroupName, pmodel->name.c_str());
	return NO_POSITION;
}

//=============================================
// @brief
//
//=============================================
Int32 VBM_GetSubmodelIndexByName( const cache_model_t* pmodel, Int32 bodyGroupIndex, const Char* pstrSubmodelName )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	// Get pointer to vbm data
	const vbmcache_t* pvbmcache = pmodel->getVBMCache();
	vbmheader_t* pvbmheader = pvbmcache->pvbmhdr;
	if(!pvbmheader)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid vbm data.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	if(bodyGroupIndex > pvbmheader->numbodyparts)
	{
		gd_engfuncs.pfnCon_Printf("%s - Overindexing bodygroup by %d for model '%s'.\n", __FUNCTION__, bodyGroupIndex, pmodel->name.c_str());
		return 0;
	}

	CString submodelName(pstrSubmodelName);
	if(submodelName.length() >= MAX_SUBMODEL_NAME_LENGTH)
	{
		Uint32 numRemove = submodelName.length() - (MAX_SUBMODEL_NAME_LENGTH-1);
		submodelName.erase(MAX_SUBMODEL_NAME_LENGTH-1, numRemove);
	}

	const vbmbodypart_t* pbodypart = pvbmheader->getBodyPart(bodyGroupIndex);
	for(Int32 i = 0; i < pbodypart->numsubmodels; i++)
	{
		const vbmsubmodel_t* psubmodel = pbodypart->getSubmodel(pvbmheader, i);
		if(!qstrcmp(psubmodel->name, submodelName)
			|| !qstrcmp(pstrSubmodelName, "blank") && !qstrlen(psubmodel->name))
			return i;
	}

	gd_engfuncs.pfnCon_Printf("%s - Couldn't find submodel '%s' in bodygroup '%s' for model '%s'.\n", __FUNCTION__, pstrSubmodelName, pbodypart->name, pmodel->name.c_str());
	return NO_POSITION;
}

//=============================================
// @brief
//
//=============================================
Int32 VBM_GetBodyGroupValue( const cache_model_t* pmodel, Int32 groupindex, Int64 body )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	// Get pointer to vbm data
	const vbmcache_t* pvbmcache = pmodel->getVBMCache();
	vbmheader_t* pvbmheader = pvbmcache->pvbmhdr;
	if(!pvbmheader)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid vbm data.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	if(groupindex > pvbmheader->numbodyparts)
	{
		gd_engfuncs.pfnCon_Printf("%s - Overindexing bodygroup by %d for model '%s'.\n", __FUNCTION__, groupindex, pmodel->name.c_str());
		return 0;
	}

	const vbmbodypart_t* pbodypart = pvbmheader->getBodyPart(groupindex);
	if(pbodypart->numsubmodels <= 1)
		return 0;
	else
		return (body / pbodypart->base)%pbodypart->numsubmodels;
}

//=============================================
// @brief
//
//=============================================
Uint32 VBM_GetNumSequenceFrames( const cache_model_t* pmodel, Int32 sequence )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	// Get pointer to studio data
	const vbmcache_t* pvbmcache = pmodel->getVBMCache();
	studiohdr_t* pstudiohdr = pvbmcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	// Check for bounds
	if(sequence < 0 || sequence >= pstudiohdr->numseq)
	{
		gd_engfuncs.pfnCon_Printf("%s - Sequence %d is out of bounds for model '%s'.\n", __FUNCTION__, sequence, pmodel->name.c_str());
		return 0;
	}

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequence);
	if(!pseqdesc)
		return 0;
	else
		return pseqdesc->numframes;
}

//=============================================
// @brief
//
//=============================================
Uint32 VBM_GetSequenceNumber( const cache_model_t* pmodel )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	// Get pointer to studio data
	const vbmcache_t* pvbmcache = pmodel->getVBMCache();
	studiohdr_t* pstudiohdr = pvbmcache->pstudiohdr;
	if(!pstudiohdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return 0;
	}

	return pstudiohdr->numseq;
}

//=============================================
// @brief
//
//=============================================
bool VBM_GetSequenceBoundingBox( const cache_model_t* pmodel, Int32 sequence, Vector& mins, Vector& maxs )
{
	if(pmodel->type != MOD_VBM)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return false;
	}

	// Get pointer to studio data
	const vbmcache_t* pvbmcache = pmodel->getVBMCache();
	studiohdr_t* phdr = pvbmcache->pstudiohdr;
	if(!phdr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return false;
	}

	// Check for bounds
	if(sequence < 0 || sequence >= phdr->numseq)
	{
		gd_engfuncs.pfnCon_Printf("%s - Sequence %d is out of bounds for model '%s'.\n", __FUNCTION__, sequence, pmodel->name.c_str());
		return false;
	}

	const mstudioseqdesc_t* pseqdesc = phdr->getSequence(sequence);
	if(!pseqdesc)
		return false;
	
	// Set mins/maxs
	mins = pseqdesc->bbmin;
	maxs = pseqdesc->bbmax;
	return true;
}