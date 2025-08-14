/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "studio.h"
#include "vbm_shared.h"
#include "com_math.h"
#include "entity_state.h"
#include "constants.h"
#include "texturemanager.h"
#include "cache_model.h"

// Notes:
// Part of this implementation is referenced from the implementation in the Half-Life SDK
// The studiomodel format is Valve's original work, and I take no ownership of it
// No copyright infringement intended

//=============================================
//
//
//=============================================
void VBM_CalculateBoneAdjustments( const studiohdr_t* phdr, Float dadt, Float* padj, const Float* pcontroller1, const Float* pcontroller2, byte mouth )
{
	// Get pointer to bone controller
	for(Int32 i = 0; i < phdr->numbonecontrollers; i++)
	{
		const mstudiobonecontroller_t* pbonecontroller = phdr->getBoneController(i);

		Float value = 0;
		Uint32 index = pbonecontroller->index;
		if(index <= 3)
		{
			if(pbonecontroller->type & STUDIO_RLOOP)
			{
				if(SDL_fabs(pcontroller1[i]-pcontroller2[i]) > 128.0f)
				{
					Int32 a = (static_cast<Int32>(pcontroller1[i])+128)%256;
					Int32 b = (static_cast<Int32>(pcontroller2[i])+128)%256;

					value = (a*dadt + b*(1.0f-dadt)) * (360.0f/256.0f) + pbonecontroller->start;
				}
				else
				{
					value = (pcontroller1[i]*dadt + pcontroller2[i]*(1.0f-dadt))*(360.0f/256.0f)+pbonecontroller->start;
				}
			}
			else
			{
				value = clamp((pcontroller1[i]*dadt+pcontroller2[i]*(1.0-dadt))/255.0f, 0.0, 1.0);
				value = (1.0f-value)*pbonecontroller->start + value*pbonecontroller->end;
			}
		}
		else
		{
			value = static_cast<Float>(mouth)/64.0f;
			clamp(value, 0.0f, 1.0f);

			value = (1.0-value) * pbonecontroller->start + value*pbonecontroller->end;
		}

		switch(pbonecontroller->type)
		{
		case STUDIO_XR:
		case STUDIO_YR:
		case STUDIO_ZR:
			padj[i] = value*(M_PI/180.0f);
			break;
		case STUDIO_X:
		case STUDIO_Y:
		case STUDIO_Z:
			padj[i] = value;
			break;
		}
	}
}

//=============================================
//
//
//=============================================
void VBM_InterpolateBones( const studiohdr_t* phdr, const vec4_t* pquaternions1, const Vector* ppositions1, const vec4_t* pquaternions2, const Vector* ppositions2, Float interpolant, vec4_t *poutquaternions, Vector* poutpositions )
{
	// Cap interpolant
	Float interp = interpolant;
	interp = clamp(interp, 0.0, 1.0);
	Float ninterp = 1.0 - interp;

	for(Int32 i = 0; i < phdr->numbones; i++)
	{
		const mstudiobone_t* pbone = phdr->getBone(i);

		// Don't blend non-blended bones
		if(pbone->flags & STUDIO_DONT_BLEND)
		{
			for(Uint32 j = 0; j < 4; j++)
				poutquaternions[i][j] = pquaternions1[i][j];

			for(Uint32 j = 0; j < 3; j++)
				poutpositions[i][j] = ppositions1[i][j];
		}
		else
		{
			// Blend the quaternions
			Math::QuaternionBlend(pquaternions1[i], pquaternions2[i], interp, poutquaternions[i]);
		
			// Blend the positions
			for(Uint32 j = 0; j < 3; j++)
				poutpositions[i][j] = ppositions1[i][j] * ninterp + ppositions2[i][j] * interp;
		}
	}
}

//=============================================
//
//
//=============================================
const mstudioanim_t* VBM_GetAnimation( const studiohdr_t* phdr, const mstudioseqdesc_t* psequencedesc )
{
	if(psequencedesc->seqgroup != 0)
		return nullptr;

	const mstudioseqgroup_t* pseqgroup = phdr->getSequenceGroup(psequencedesc->seqgroup);
	return reinterpret_cast<const mstudioanim_t*>(reinterpret_cast<const byte*>(phdr) + pseqgroup->data + psequencedesc->animindex);
}

//=============================================
//
//
//=============================================
void VBM_CalculateBoneQuaternion( Int32 frame, Float interpolant, const mstudiobone_t* pbone, const mstudioanim_t* panimation, const Float* padj, vec4_t& quaternion )
{
	Vector angle1, angle2;

	for (Uint32 i = 0; i < 3; i++)
	{
		if (panimation->offset[i+3] == 0)
		{
			angle2[i] = angle1[i] = pbone->value[i+3]; // default;
		}
		else
		{
			const mstudioanimvalue_t* panimvalue = panimation->getAnimationValue(i+3);
			Int32 j = frame;
			if (panimvalue->num.total < panimvalue->num.valid)
				j = 0;

			while (panimvalue->num.total <= j)
			{
				j -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
				if (panimvalue->num.total < panimvalue->num.valid)
					j = 0;
			}

			if (panimvalue->num.valid > j)
			{
				angle1[i] = panimvalue[j+1].value;

				if (panimvalue->num.valid > j + 1)
				{
					angle2[i] = panimvalue[j+2].value;
				}
				else
				{
					if (panimvalue->num.total > j + 1)
						angle2[i] = angle1[i];
					else
						angle2[i] = panimvalue[panimvalue->num.valid+2].value;
				}
			}
			else
			{
				angle1[i] = panimvalue[panimvalue->num.valid].value;
				if (panimvalue->num.total > j + 1)
					angle2[i] = angle1[i];
				else
					angle2[i] = panimvalue[panimvalue->num.valid + 2].value;
			}
			angle1[i] = pbone->value[i+3] + angle1[i] * pbone->scale[i+3];
			angle2[i] = pbone->value[i+3] + angle2[i] * pbone->scale[i+3];
		}

		if (pbone->bonecontroller[i+3] != -1)
		{
			angle1[i] += padj[pbone->bonecontroller[i+3]];
			angle2[i] += padj[pbone->bonecontroller[i+3]];
		}
	}

	if (!Math::VectorCompare( angle1, angle2 ))
	{
		vec4_t q1, q2;
		Math::AngleQuaternion( angle1, q1 );
		Math::AngleQuaternion( angle2, q2 );
		Math::QuaternionBlend( q1, q2, interpolant, quaternion );
	}
	else
	{
		Math::AngleQuaternion( angle1, quaternion );
	}
}

//=============================================
//
//
//=============================================
void VBM_CalculateBonePosition( Int32 frame, Float interpolant, const mstudiobone_t* pbone, const mstudioanim_t* panimation, const Float* padj, Vector& outpos )
{
	for (Uint32 i = 0; i < 3; i++)
	{
		outpos[i] = pbone->value[i]; // default;
		if (panimation->offset[i] != 0)
		{
			const mstudioanimvalue_t* panimvalue = panimation->getAnimationValue(i);
			Int32 j = frame;
			if (panimvalue->num.total < panimvalue->num.valid)
				j = 0;

			// find span of values that includes the frame we want
			while (panimvalue->num.total <= j)
			{
				j -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
				if (panimvalue->num.total < panimvalue->num.valid)
					j = 0;
			}

			// if we're inside the span
			if (panimvalue->num.valid > j)
			{
				if (panimvalue->num.valid > j + 1)
					outpos[i] += (static_cast<Double>(panimvalue[j+1].value) * (1.0 - interpolant) + interpolant * static_cast<Double>(panimvalue[j+2].value)) * pbone->scale[i];
				else
					outpos[i] += static_cast<Double>(panimvalue[j+1].value) * pbone->scale[i];
			}
			else
			{
				// are we at the end of the repeating values section and there'interpolant another section with data?
				if (panimvalue->num.total <= j + 1)
					outpos[i] += (static_cast<Double>(panimvalue[panimvalue->num.valid].value) * (1.0 - interpolant) + interpolant * static_cast<Double>(panimvalue[panimvalue->num.valid + 2].value)) * pbone->scale[i];
				else
					outpos[i] += static_cast<Double>(panimvalue[panimvalue->num.valid].value) * pbone->scale[i];
			}
		}

		if ( pbone->bonecontroller[i] != -1 && padj )
			outpos[i] += padj[pbone->bonecontroller[i]];
	}
}

//=============================================
//
//
//=============================================
Float VBM_EstimateFrame( const mstudioseqdesc_t* pseqdesc, const entity_state_t& entitystate, Double time )
{
	if(entitystate.effects & EF_STATICENTITY)
		return 0;

	Double frame = 0;
	if(pseqdesc->numframes > 1)
		frame = (entitystate.frame * (pseqdesc->numframes-1))/256.0;

	if(time >= entitystate.animtime)
		frame += (time - entitystate.animtime)*entitystate.framerate*pseqdesc->fps;

	if(pseqdesc->flags & STUDIO_LOOPING)
	{
		if(pseqdesc->numframes > 1)
			frame -= static_cast<Int32>(frame/(pseqdesc->numframes - 1))*(pseqdesc->numframes - 1);

		if(frame < 0)
			frame += (pseqdesc->numframes - 1);
	}
	else
	{
		if(frame >= pseqdesc->numframes - 1.001)
			frame = pseqdesc->numframes - 1.001;

		if(frame < 0)
			frame = 0;
	}

	return frame;
}

//=============================================
//
//
//=============================================
bool VBM_HasTransparentParts( vbmheader_t* pvbmheader, Uint64 body, Int32 skin )
{
	const Int16* pskinref = pvbmheader->getSkinFamilies();

	if(skin != 0 && skin < pvbmheader->numskinfamilies)
		pskinref += (skin*pvbmheader->numskinref);

	for(Int32 i = 0; i < pvbmheader->numbodyparts; i++)
	{
		vbmbodypart_t* pbodypart = pvbmheader->getBodyPart(i);
		const Uint32 index = (body / pbodypart->base) % pbodypart->numsubmodels;

		vbmsubmodel_t* psubmodel = pbodypart->getSubmodel(pvbmheader, index);

		for(Int32 j = 0; j < psubmodel->nummeshes; j++)
		{
			const vbmmesh_t* pmesh = psubmodel->getMesh(pvbmheader, j);
			const vbmtexture_t* ptexture = pvbmheader->getTexture(pskinref[pmesh->skinref]);
			if(ptexture->flags & FL_VBM_TEXTURE_BLEND)
				return true;
		}
	}

	return false;
}

//=============================================
//
//
//=============================================
void VBM_NormalizeWeights( Float* pflweights, Uint32 maxweights )
{
	Float totalweight = 0;
	for(Uint32 i = 0; i < maxweights; i++)
		totalweight += pflweights[i];

	const Float scale = 1.0f/totalweight;
	for(Uint32 i = 0; i < maxweights; i++)
	{
		if(pflweights[i] > 0)
			pflweights[i] *= scale;
	}
}

//=============================================
//
//
//=============================================
Float VBM_EstimateInterpolant( Float time, Float animtime, Float prevanimtime )
{
	Float interpolant = 1.0;
	if(animtime >= prevanimtime + 0.01f)
	{
		interpolant = time - animtime;
		interpolant /= 0.1f;
		if(interpolant > 2.0)
			interpolant = 2.0;
	}

	return interpolant;
}

//=============================================
//
//
//=============================================
void VBM_CalculateRotations( const studiohdr_t* phdr, Float time, Float animtime, Float prevanimtime, Vector* ppositions, vec4_t* pquaternions, const mstudioseqdesc_t* pseqdesc, const mstudioanim_t* panim, Float frame, const Float* pcontroller1, const Float* pcontroller2, byte mouth )
{
	// Cap frame value
	Float _frame = frame;
	if(_frame > (pseqdesc->numframes-1))
		_frame = 0;
	else if(_frame < -0.01f)
		_frame = -0.01f;

	// Calculate interpolant value
	const Int32 intframe = static_cast<Int32>(_frame);
	const Float dadt = VBM_EstimateInterpolant(time, animtime, prevanimtime);
	const Float interp = _frame - intframe;
	
	// Estimate controller values
	CArray<Float> controlleradj(phdr->numbonecontrollers);
	for(Uint32 i = 0; i < phdr->numbonecontrollers; i++)
		controlleradj[i] = 0;

	Float* pcontroller = (phdr->numbonecontrollers > 0) ? &controlleradj[0] : nullptr;
	VBM_CalculateBoneAdjustments(phdr, dadt, pcontroller, pcontroller1, pcontroller2, mouth);

	// Calculate quaternions and bone positions for each bone
	for(Int32 i = 0; i < phdr->numbones; i++)
	{
		const mstudiobone_t* pbone = phdr->getBone(i);

		VBM_CalculateBoneQuaternion(intframe, interp, pbone, &panim[i], pcontroller, pquaternions[i]);
		VBM_CalculateBonePosition(intframe, interp, pbone, &panim[i], pcontroller, ppositions[i]);
	}

	// Remove movement from specific sequence motion types
	if(pseqdesc->motiontype & STUDIO_X)
		ppositions[pseqdesc->motionbone][0] = 0.0f;
	if(pseqdesc->motiontype & STUDIO_Y)
		ppositions[pseqdesc->motionbone][1] = 0.0f;
	if(pseqdesc->motiontype & STUDIO_Z)
		ppositions[pseqdesc->motionbone][2] = 0.0f;
}


//=============================================
// @brief
//
//=============================================
Int32 VBM_FindSequence( const studiohdr_t* pstudiohdr, const Char* pstrsequencename )
{
	if(!pstudiohdr)
		return NO_SEQUENCE_VALUE;

	for(Int32 i = 0; i < pstudiohdr->numseq; i++)
	{
		const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(i);
		if(!qstrcmp(pseqdesc->label, pstrsequencename))
			return i;
	}

	return NO_SEQUENCE_VALUE;
}

//=============================================
// @brief
//
//=============================================
Float VBM_GetSequenceTime( const studiohdr_t* pstudiohdr, Int32 sequence, Float framerate )
{
	if(!pstudiohdr)
		return 0;

	// Check for bounds
	if(sequence < 0 || sequence >= pstudiohdr->numseq)
		return 0;

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequence);
	return (pseqdesc->numframes-1)/(pseqdesc->fps*framerate);
}

//=============================================
// @brief
//
//=============================================
bool VBM_MeshBonesCompatible( vbmmesh_t *pgrp1, vbmmesh_t *pgrp2, vbmheader_t* pvbm, en_material_t* (*pfnFindMaterialScriptByIndex)( Int32 index ) )
{
	if(!pvbm->skinoffset)
		return true;

	const Int16 *pskinref = pvbm->getSkinFamilies();
	for(Int32 i = 0; i < pvbm->numskinfamilies; i++)
	{
		const Int32 skinoffs = pvbm->numskinref * i;
		const vbmtexture_t *ptexture1 = pvbm->getTexture(pskinref[skinoffs + pgrp1->skinref]);
		const vbmtexture_t *ptexture2 = pvbm->getTexture(pskinref[skinoffs + pgrp2->skinref]);

		const en_material_t* pmaterial1 = pfnFindMaterialScriptByIndex(ptexture1->index);
		const en_material_t* pmaterial2 = pfnFindMaterialScriptByIndex(ptexture2->index);
		if(!pmaterial1 || !pmaterial2)
			return true;

		if((pmaterial1->flags & TX_FL_CHROME) != (pmaterial2->flags & TX_FL_CHROME))
			return false;

		if((pmaterial1->flags & TX_FL_ADDITIVE) != (pmaterial2->flags & TX_FL_ADDITIVE))
			return false;

		if((pmaterial1->flags & TX_FL_ALPHABLEND) != (pmaterial2->flags & TX_FL_ALPHABLEND))
			return false;

		if((pmaterial1->flags & TX_FL_SCOPE) != (pmaterial2->flags & TX_FL_SCOPE))
			return false;

		if((pmaterial1->flags & TX_FL_FULLBRIGHT) != (pmaterial2->flags & TX_FL_FULLBRIGHT))
			return false;

		if((pmaterial1->flags & TX_FL_ALPHATEST) != (pmaterial2->flags & TX_FL_ALPHATEST))
			return false;

		if((pmaterial1->flags & TX_FL_EYEGLINT) != (pmaterial2->flags & TX_FL_EYEGLINT))
			return false;

		if((pmaterial1->ptextures[MT_TX_SPECULAR] ? true : false) != (pmaterial2->ptextures[MT_TX_SPECULAR] ? true : false))
			return false;

		if((pmaterial1->ptextures[MT_TX_LUMINANCE] ? true : false) != (pmaterial2->ptextures[MT_TX_LUMINANCE] ? true : false))
			return false;

		if((pmaterial1->ptextures[MT_TX_NORMALMAP] ? true : false) != (pmaterial2->ptextures[MT_TX_NORMALMAP] ? true : false))
			return false;
	}

	return true;
}


//=============================================
// @brief
//
//=============================================
bool VBM_PostLoadCheckSubmodel( vbmheader_t* pvbm, vbmsubmodel_t* psubmodel, en_material_t* (*pfnFindMaterialScriptByIndex)( Int32 index ) )
{
	if(!psubmodel->nummeshes)
		return false;

	bool needsChange = false;
	vbmmesh_t *pbonemesh = psubmodel->getMesh(pvbm, 0);

	for(Int32 k = 1; k < psubmodel->nummeshes; k++)
	{
		vbmmesh_t *pcurmesh = psubmodel->getMesh(pvbm, k);

		if(pcurmesh->numbones > 0 
			&& (pbonemesh->numbones != pcurmesh->numbones
			|| pcurmesh->boneoffset != pbonemesh->boneoffset))
		{
			pbonemesh = pcurmesh;
			continue;
		}

		if(!VBM_MeshBonesCompatible(pbonemesh, pcurmesh, pvbm, pfnFindMaterialScriptByIndex))
		{
			pcurmesh->boneoffset = pbonemesh->boneoffset;
			pcurmesh->numbones = pbonemesh->numbones;
			pbonemesh = pcurmesh;
			needsChange = true;
		}
		else if(pbonemesh->numbones != 0 && pbonemesh->numbones == pcurmesh->numbones
			&& pbonemesh->boneoffset != 0 && pbonemesh->boneoffset == pcurmesh->boneoffset)
		{
			pcurmesh->boneoffset = 0;
			pcurmesh->numbones = 0;
			needsChange = true;
		}
	}

	return needsChange;
}

//=============================================
// @brief
//
//=============================================
bool VBM_PostLoadVBMCheck( vbmheader_t* pvbm, en_material_t* (*pfnFindMaterialScriptByIndex)( Int32 index ) )
{
	// 
	// Update bone compatibility settings in VBM data
	//
	bool needsChange = false;

	for (Int32 i = 0; i < pvbm->numbodyparts; i++)
	{
		vbmbodypart_t *pbodypart = pvbm->getBodyPart(i);

		for(Int32 j = 0; j < pbodypart->numsubmodels; j++)
		{
			vbmsubmodel_t *psubmodel = pbodypart->getSubmodel(pvbm, j);
			if(VBM_PostLoadCheckSubmodel(pvbm, psubmodel, pfnFindMaterialScriptByIndex))
				needsChange = true;

			if(psubmodel->numlods)
			{
				for(Int32 k = 0; k < psubmodel->numlods; k++)
				{
					vbmlod_t* plod = psubmodel->getLOD(pvbm, k);
					vbmsubmodel_t* plodsubmodel = plod->getSubmodel(pvbm);
					if(VBM_PostLoadCheckSubmodel(pvbm, plodsubmodel, pfnFindMaterialScriptByIndex))
						needsChange = true;
				}
			}
		}
	}

	return needsChange;
}

//=============================================
// @brief
//
//=============================================
Float VBM_SetController( const cache_model_t* pmodel, Uint32 controllerindex, Float value, Float* pcontrollers, void (*pfnCon_Printf)( const Char *fmt, ... ) )
{
	if(pmodel->type != MOD_VBM)
	{
		pfnCon_Printf("%s - Called on model '%s' that's not a VBM model.\n", __FUNCTION__, pmodel->name.c_str());
		return value;
	}

	// get pointer to studio data
	const vbmcache_t* pcache = pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;
	if(!pstudiohdr)
	{
		pfnCon_Printf("%s - Called on model '%s' which has no valid studio data.\n", __FUNCTION__, pmodel->name.c_str());
		return value;
	}

	// Get bone controller
	const mstudiobonecontroller_t* pbonecontrollers = reinterpret_cast<const mstudiobonecontroller_t*>(reinterpret_cast<const byte*>(pstudiohdr) + pstudiohdr->bonecontrollerindex);

	Int32 i = 0;
	for(; i < pstudiohdr->numbonecontrollers; i++)
	{
		if(pbonecontrollers[i].index == static_cast<Int32>(controllerindex))
			break;
	}

	if(i == pstudiohdr->numbonecontrollers)
		return value;

	if(pbonecontrollers[i].type & (STUDIO_XR|STUDIO_YR|STUDIO_ZR))
	{
		if(pbonecontrollers[i].end < pbonecontrollers[i].start)
			value *= -1;

		if(pbonecontrollers[i].start + 359.0 >= pbonecontrollers[i].end)
		{
			if(value > ((pbonecontrollers[i].start + pbonecontrollers[i].end)/2.0) + 180)
				value -= 360;
			if(value < ((pbonecontrollers[i].start + pbonecontrollers[i].end)/2.0) - 180)
				value += 360;
		}
		else
		{
			if(value > 360)
				value -= static_cast<Int32>((value/360.0)*360.0);
			else if(value < 0)
				value += static_cast<Int32>(((value/-360.0)+1)*360.0);
		}
	}

	Float controllersetting = 255*(value - pbonecontrollers[i].start)/(pbonecontrollers[i].end-pbonecontrollers[i].start);
	controllersetting = clamp(controllersetting, 0, 255);
	pcontrollers[i] = controllersetting;

	return controllersetting*(1.0/255.0)*(pbonecontrollers[i].end-pbonecontrollers[i].start)+pbonecontrollers[i].start;
}