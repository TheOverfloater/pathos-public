/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "sprite.h"
#include "system.h"

//=============================================
// @brief
//
//=============================================
mspriteframe_t* Sprite_LoadFrame( const dspriteframe_t* pdframe, const byte* pdata )
{
	mspriteframe_t* pframe = new mspriteframe_t();
			
	pframe->width = pdframe->width;
	pframe->height = pdframe->height;
	pframe->up = pdframe->originy;
	pframe->left = pdframe->originx;
	pframe->down = pdframe->originy - pframe->height;
	pframe->right = pframe->width + pdframe->originx;

	byte* ptexturedata = new byte[pframe->width*pframe->height];
	const byte* pdatasrc = pdata + sizeof(dspriteframe_t);
	memcpy(ptexturedata, pdatasrc, sizeof(byte)*pframe->width*pframe->height);
	pframe->pdata = ptexturedata;

	return pframe;
}

//=============================================
// @brief
//
//=============================================
msprite_t* Sprite_Load( const byte* pfile, Uint32 filesize )
{
	const dsprite_t* pheader = reinterpret_cast<const dsprite_t*>(pfile);
	if(pheader->id != IDSPRITEHEADER)
	{
		Con_Printf("%s - Wrong header id for sprite.\n", __FUNCTION__);
		return nullptr;
	}

	if(pheader->version != SPRITE_VERSION)
	{
		Con_Printf("%s - Wrong version for sprite.\n", __FUNCTION__);
		return nullptr;
	}

	// Create header
	msprite_t* psprite = new msprite_t();
	psprite->type = pheader->type;
	psprite->format = pheader->format;
	psprite->maxwidth = pheader->maxwidth;
	psprite->maxheight = pheader->maxheight;
	psprite->radius = pheader->boundingradius;
	psprite->beamlength = pheader->beamlength;
	psprite->synctype = pheader->synctype;
	const byte* pdata = pfile + sizeof(dsprite_t);

	// Retreive palette size and allocate data
	Uint32 palettesize = Common::ByteToUint16(pfile+sizeof(dsprite_t));
	pdata += sizeof(Uint16);

	// Copy palette data
	psprite->palette = new byte[palettesize*sizeof(color24_t)];
	const byte* ppalettedata = pfile+sizeof(dsprite_t)+sizeof(Uint16);
	memcpy(psprite->palette, ppalettedata, sizeof(color24_t)*palettesize);
	pdata += palettesize*sizeof(color24_t);

	// Retreive list of frames
	psprite->frames.resize(pheader->numframes);
	for(Uint32 i = 0; i < pheader->numframes; i++)
	{
		mspriteframedesc_t* pframedesc = &psprite->frames[i];
		pframedesc->type = (spr_frametype_t)Common::ByteToUint32(pdata);
		pdata += sizeof(Uint32);

		if(pframedesc->type == SPR_SINGLE)
		{
			const dspriteframe_t* pframedata = reinterpret_cast<const dspriteframe_t*>(pdata);
			mspriteframe_t* pframe = Sprite_LoadFrame(pframedata, pdata);
			pframedesc->pframeptr = pframe;

			Uint32 texturedatasize = pframe->width*pframe->height;
			pdata += sizeof(dspriteframe_t)+sizeof(byte)*texturedatasize;
		}
		else if(pframedesc->type == SPR_GROUP)
		{
			Uint32 numgroupframes = Common::ByteToUint32(pdata);
			pdata += sizeof(Uint32);

			mspritegroup_t* pgroup = new mspritegroup_t();
			pframedesc->pgroupptr = pgroup;

			pgroup->intervals.resize(numgroupframes);

			const Float* pintervals = reinterpret_cast<const Float*>(pdata);
			pdata += sizeof(Float)*numgroupframes;

			for(Uint32 j = 0; j < numgroupframes; j++)
			{
				// Set interval value
				pgroup->intervals[j] = pintervals[j];
				if(pgroup->intervals[j] <= 0)
					pgroup->intervals[j] = 1;

				const dspriteframe_t* pframedata = reinterpret_cast<const dspriteframe_t*>(pdata);
				mspriteframe_t* pframe = Sprite_LoadFrame(pframedata, pdata);
				pgroup->frames.push_back(pframe);

				Uint32 texturedatasize = pframe->width*pframe->height;
				pdata += sizeof(dspriteframe_t)+sizeof(byte)*texturedatasize;
			}
		}
		else
		{
			// Error case
			Con_Printf("%s - Invalid sprite frametype %d.\n", __FUNCTION__, (Int32)pframedesc->type);
			delete psprite;
			return nullptr;
		}
	}

	return psprite;
}

//====================================
//
//====================================
const mspriteframe_t* Sprite_GetFrame( const msprite_t* psprite, Uint32 frame, Float time )
{
	if(frame >= psprite->frames.size())
	{
		Con_Printf ("%s - No such frame %d\n", __FUNCTION__, frame);
		frame = 0;
	}

	const mspriteframe_t *pspriteframe;
	if (psprite->frames[frame].type == SPR_SINGLE)
	{
		pspriteframe = psprite->frames[frame].pframeptr;
	}
	else
	{
		mspritegroup_t* pspritegroup = reinterpret_cast<mspritegroup_t*>(psprite->frames[frame].pgroupptr);
		Uint32 numframes = pspritegroup->frames.size();
		Float fullinterval = pspritegroup->intervals[numframes-1];

		// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
		// are positive, so we don't have to worry about division by 0
		Float targettime = time - ((Int32)(time / fullinterval)) * fullinterval;

		Uint32 i = 0;
		for(; i < (numframes-1); i++)
		{
			if (pspritegroup->intervals[i] > targettime)
				break;
		}

		pspriteframe = pspritegroup->frames[i];
	}

	return pspriteframe;
}
