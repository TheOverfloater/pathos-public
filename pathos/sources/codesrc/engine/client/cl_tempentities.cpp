/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cl_tempentities.h"
#include "cl_pmove.h"
#include "cl_main.h"
#include "r_main.h"
#include "trace.h"
#include "cvar.h"
#include "console.h"
#include "cl_snd.h"
#include "r_dlights.h"
#include "modelcache.h"
#include "system.h"
#include "cl_snd.h"
#include "file.h"
#include "r_legacyparticles.h"
#include "r_blackhole.h"

// Tempentity allocation size
const Uint32 CTempEntityManager::TEMPENT_ALLOC_SIZE = 4096;
// One shard every n^3 units
const Float CTempEntityManager::SHARD_VOLUME = 12.0;

// Object definition
CTempEntityManager gTempEntities;

//====================================
//
//====================================
CTempEntityManager::CTempEntityManager( void ):
	m_pFreeTempEntityHeader(nullptr),
	m_pActiveTempEntityHeader(nullptr),
	m_pCvarGravity(nullptr)
{
	// Allocate initial size
	AllocateTempEntities();
}

//====================================
//
//====================================
CTempEntityManager::~CTempEntityManager( void )
{
	DeleteAllTempEntities();
}

//====================================
//
//====================================
bool CTempEntityManager::Init( void )
{
	m_pCvarGravity = gConsole.GetCVar("sv_gravity");
	if(!m_pCvarGravity)
	{
		Con_Printf("%s - Failed to get cvar 'sv_gravity'.\n", __FUNCTION__);
		return false;
	}

	return true;
}

//====================================
//
//====================================
void CTempEntityManager::Shutdown( void )
{
	// Clear tempentities
	ClearGame();
}

//====================================
//
//====================================
bool CTempEntityManager::InitGame( void )
{
	return true;
}

//====================================
//
//====================================
void CTempEntityManager::ClearGame( void )
{
	// Clear tempentities
	DeleteAllTempEntities();
	// Allocate base size
	AllocateTempEntities();
}

//====================================
//
//====================================
void CTempEntityManager::DeleteAllTempEntities( void )
{
	// Make sure all of these are freed
	FreeActiveTempEntities();

	// Delete every single entry
	tempentity_t* pnext = m_pFreeTempEntityHeader;
	while(pnext)
	{
		tempentity_t* pfree = pnext;
		pnext = pfree->pnext;

		delete pfree;
	}

	m_pFreeTempEntityHeader = nullptr;
}

//====================================
//
//====================================
void CTempEntityManager::AllocateTempEntities( void )
{
	for(Uint32 i = 0; i < TEMPENT_ALLOC_SIZE; i++)
	{
		tempentity_t* pnew = new tempentity_t;

		if(m_pFreeTempEntityHeader)
		{
			m_pFreeTempEntityHeader->pprev = pnew;
			pnew->pnext = m_pFreeTempEntityHeader;
		}
		else
			pnew->pnext = nullptr;

		m_pFreeTempEntityHeader = pnew;
	}
}

//====================================
//
//====================================
void CTempEntityManager::FreeActiveTempEntities( void )
{
	if(!m_pActiveTempEntityHeader)
		return;

	tempentity_t* pnext = m_pActiveTempEntityHeader;
	while(pnext)
	{
		tempentity_t* pfree = pnext;
		pnext = pfree->pnext;

		FreeTempEntity(pfree);
	}
}

//====================================
//
//====================================
void CTempEntityManager::FreeTempEntity( tempentity_t* pfree )
{
	if(!pfree->pprev) 
		m_pActiveTempEntityHeader = pfree->pnext;
	else 
		pfree->pprev->pnext = pfree->pnext;

	if(pfree->pnext) 
		pfree->pnext->pprev = pfree->pprev;

	// Clear this entity
	(*pfree) = tempentity_t();

	// Link to freed entities
	pfree->pnext = m_pFreeTempEntityHeader;
	if(m_pFreeTempEntityHeader)
		m_pFreeTempEntityHeader->pprev = pfree;

	m_pFreeTempEntityHeader = pfree;
}

//====================================
//
//====================================
tempentity_t* CTempEntityManager::AllocTempEntity( const Vector& origin, const cache_model_t* pmodel )
{
	// Allocate if needed
	if(!m_pFreeTempEntityHeader)
		AllocateTempEntities();

	tempentity_t* ptemp = m_pFreeTempEntityHeader;
	m_pFreeTempEntityHeader = ptemp->pnext;
	if(m_pFreeTempEntityHeader)
		m_pFreeTempEntityHeader->pprev = nullptr;

	// Clear tempentity state
	(*ptemp) = tempentity_t();

	// Add system into pointer array
	if(m_pActiveTempEntityHeader)
	{
		m_pActiveTempEntityHeader->pprev = ptemp;
		ptemp->pnext = m_pActiveTempEntityHeader;
	}
	m_pActiveTempEntityHeader = ptemp;

	// Set basic params
	cl_entity_t* pentity = &ptemp->entity;

	pentity->pmodel = pmodel;
	pentity->curstate.modelindex = pmodel->cacheindex;

	pentity->curstate.rendermode = RENDER_NORMAL;
	pentity->curstate.renderfx = RenderFx_None;
	pentity->curstate.rendercolor = Vector(255, 255, 255);
	pentity->curstate.body = 0;
	pentity->curstate.skin = 0;

	pentity->curstate.origin = origin;
	pentity->prevstate.origin = origin;
	pentity->curstate.scale = 1.0;

	ptemp->framemax = Cache_GetModelFrameCount(*pmodel) - 1;
	ptemp->fadespeed = 0.5f;
	ptemp->soundtype = TE_BOUNCE_NONE;
	ptemp->bouncefactor = 1.0;

	return ptemp;
}

//====================================
//
//====================================
void CTempEntityManager::UpdateTempEntities( void )
{
	// Don't simulate if no ents are around
	if(!m_pActiveTempEntityHeader)
		return;

	if(cls.paused)
	{
		tempentity_t* pnext = m_pActiveTempEntityHeader;
		while(pnext)
		{
			if(pnext->entity.pmodel)
				R_AddEntity(&pnext->entity);

			pnext = pnext->pnext;
		}
		// If paused, don't simulate
		return;
	}

	// Simulate and update entities
	tempentity_t* pnext = m_pActiveTempEntityHeader;
	while(pnext)
	{
		bool active = true;

		// Manage freeing tempentities
		if(pnext->die < cls.cl_time)
		{
			if(pnext->flags & TE_FL_FADEOUT)
			{
				if(pnext->entity.curstate.rendermode == RENDER_NORMAL)
					pnext->entity.curstate.rendermode = RENDER_TRANSTEXTURE_LIT;

				// Calculate alpha value
				Float life = pnext->die - cls.cl_time;
				pnext->entity.curstate.renderamt = pnext->startrenderamt * (1+life*pnext->fadespeed);

				// Kill if faded out
				if(pnext->entity.curstate.renderamt <= 0)
					active = false;
			}
			else
			{
				// Not active
				active = false;
			}
		}

		// Update the tempentity
		if(!active || !UpdateTempEntity(pnext))
		{
			tempentity_t* pfree = pnext;
			pnext = pfree->pnext;
			FreeTempEntity(pfree);
			continue;
		}

		// Move forward
		pnext = pnext->pnext;
	}
}

//====================================
//
//====================================
bool CTempEntityManager::UpdateTempEntity( tempentity_t* ptemp ) const
{
	// For short
	cl_entity_t& entity = ptemp->entity;

	// Copy current origin to prevstate
	Vector prevorigin = ptemp->entity.curstate.origin;

	if(ptemp->flags & TE_FL_SINEWAVE)
	{
		ptemp->add.x += entity.curstate.velocity[0] * cls.frametime;
		ptemp->add.y += entity.curstate.velocity[1] * cls.frametime;

		entity.curstate.origin[0] = ptemp->add.x + SDL_sin(entity.curstate.velocity[2] + cls.cl_time * entity.prevstate.frame)*(10*entity.curstate.frame);
		entity.curstate.origin[1] = ptemp->add.y + SDL_sin(entity.curstate.velocity[2] + cls.cl_time * 5)*(8*entity.curstate.framerate);
		entity.curstate.origin[2] += entity.curstate.velocity[2]*cls.frametime;
	}
	else if(ptemp->flags & TE_FL_SPIRAL)
	{
		entity.curstate.origin[0] += entity.curstate.velocity[0] * cls.frametime + 8*SDL_sin(cls.cl_time * 20 + ptemp->die*20);
		entity.curstate.origin[1] += entity.curstate.velocity[1] * cls.frametime + 4*SDL_sin(cls.cl_time * 30 + ptemp->die*20);
		entity.curstate.origin[2] += entity.curstate.velocity[2]*cls.frametime;
	}
	else
	{
		// Calculate standard velocity
		Math::VectorMA(entity.curstate.origin, cls.frametime, entity.curstate.velocity, entity.curstate.origin);
	}

	// Animate frames if needed
	if(ptemp->flags & TE_FL_SPRITE_ANIMATE)
	{
		entity.curstate.frame += cls.frametime * entity.curstate.framerate;
		if(entity.curstate.frame >= ptemp->framemax)
		{
			if(!(ptemp->flags & TE_FL_SPRITE_ANIM_LOOP))
				return false;

			entity.curstate.frame -= SDL_floor(entity.curstate.frame);
		}
	}
	else if(ptemp->flags & TE_FL_SPRITE_CYCLE)
	{
		entity.curstate.frame += cls.frametime*10;
		if(entity.curstate.frame > ptemp->framemax)
			entity.curstate.frame -= SDL_floor(entity.curstate.frame);
	}

	// Rotate if needed
	if(ptemp->flags & TE_FL_ROTATE)
	{
		// Rotate by velocity
		Math::VectorMA(entity.curstate.angles, cls.frametime, entity.curstate.avelocity, entity.curstate.angles);
	}

	// Manage collisions
	if(ptemp->flags & (TE_FL_COLLIDEALL|TE_FL_COLLIDEWORLD))
	{
		trace_t tr;

		Int32 traceflags = FL_TRACE_NORMAL;
		if(ptemp->flags & TE_FL_COLLIDEALL)
			traceflags |= FL_TRACE_NORMAL;
		else
			traceflags |= FL_TRACE_WORLD_ONLY;

		CL_PlayerTrace(entity.prevstate.origin, entity.curstate.origin, traceflags, HULL_POINT, NO_ENTITY_INDEX, tr);

		if(tr.fraction != 1.0)
		{
			// Place entity at collision point
			Math::VectorMA(entity.prevstate.origin, tr.fraction*cls.frametime, entity.curstate.velocity, entity.curstate.origin);

			// get velocity damping
			Float damp = ptemp->bouncefactor;
			if(ptemp->flags & (TE_FL_GRAVITY|TE_FL_SLOWGRAVITY))
			{
				// Lower it a bit
				damp *= 0.5;

				if(tr.plane.normal[2] > 0.9 && entity.curstate.velocity[2] <= 0 && entity.curstate.velocity[2] >= -cls.frametime*m_pCvarGravity->GetValue()*3)
				{
					damp = 0;
					ptemp->flags &= ~(TE_FL_ROTATE|TE_FL_GRAVITY|TE_FL_SLOWGRAVITY|TE_FL_COLLIDEALL|TE_FL_COLLIDEKILL);
					entity.curstate.angles[0] = entity.curstate.angles[2] = 0;
				}
			}

			if(ptemp->soundtype && ptemp->entity.curstate.origin != ptemp->entity.prevstate.origin)
			{
				// Play sounds if any
				PlayTempEntitySound(ptemp, damp);
			}

			if(!(ptemp->flags & TE_FL_COLLIDEKILL))
			{
				if(damp != 0)
				{
					Float proj = Math::DotProduct(entity.curstate.velocity, tr.plane.normal);
					Math::VectorMA(entity.curstate.velocity, -proj*2, tr.plane.normal, entity.curstate.velocity);
					entity.curstate.angles[1] = -entity.curstate.angles[1];
				}

				if(damp != 1.0)
				{
					Math::VectorScale(entity.curstate.velocity, damp, entity.curstate.velocity);
					Math::VectorScale(entity.curstate.angles, 0.9, entity.curstate.angles);
				}
			}
			else
			{
				// Just kill it
				return false;
			}
		}
	}

	// Set after sounds have been called to play
	ptemp->entity.prevstate.origin = prevorigin;

	// Add lighting effects if any
	if(ptemp->flags & TE_FL_FLICKER && ptemp->flickertime <= cls.cl_time)
	{
		if(Common::RandomLong(0, 255) > 150)
		{
			cl_dlight_t* pdl = gDynamicLights.AllocDynamicPointLight(0, 0, false, true, nullptr);
			pdl->origin = entity.curstate.origin + Vector(0, 0, 8);
			pdl->radius = Common::RandomFloat(40, 60);
			pdl->color.x = 1.0;
			pdl->color.y = 0.5;
			pdl->color.z = 0.2;
			pdl->die = cls.cl_time + 0.4;
			pdl->decay = 250;
		}

		ptemp->flickertime = cls.cl_time + Common::RandomFloat(0.1, 0.8);
	}

	if (ptemp->flags & TE_FL_SMOKETRAIL)
		gLegacyParticles.CreateRocketTrail(ptemp->entity.prevstate.origin, ptemp->entity.curstate.origin, trail_smoke);

	// Manage gravity
	Float gravityAmount = 0;
	if(ptemp->flags & (TE_FL_GRAVITY|TE_FL_SLOWGRAVITY))
	{
		// Determine amount
		if(ptemp->flags & TE_FL_SLOWGRAVITY)
			gravityAmount = 0.5;
		else
			gravityAmount = 1.0;

		// Add in the gravity
		entity.curstate.velocity[2] -= m_pCvarGravity->GetValue() * cls.frametime * gravityAmount;

		// Manage bouyancy if in water
		if(ptemp->buoyancy)
		{
			Int32 contents = CL_PointContents(entity.curstate.origin, nullptr);
			if(contents == CONTENTS_WATER || contents == CONTENTS_SLIME || contents == CONTENTS_LAVA)
			{
				entity.curstate.velocity[2] += cls.frametime * ptemp->buoyancy;

				if(ptemp->waterfriction > 0)
				{
					Float speed = entity.curstate.velocity.Length();
					if(speed > 0)
					{
						Float newspeed = speed - cls.frametime * speed * ptemp->waterfriction;
						if(newspeed < 0)
							newspeed = 0;

						Math::VectorScale(entity.curstate.velocity, newspeed/speed, entity.curstate.velocity);
					}
				}

				for(Uint32 i = 0; i < 3; i++)
				{
					if(i == YAW)
						continue;

					if(entity.curstate.angles[i] > 0)
					{
						entity.curstate.angles[i] -= cls.frametime * ptemp->buoyancy * 0.5;
						if(entity.curstate.angles[i] < 0)
							entity.curstate.angles[i] = 0;
					}
					else if(entity.curstate.angles[i] < 0)
					{
						entity.curstate.angles[i] += cls.frametime * ptemp->buoyancy * 0.5;
						if(entity.curstate.angles[i] > 0)
							entity.curstate.angles[i] = 0;
					}
				}
			}
			else if(ptemp->prevcontents != contents)
			{
				// Make it lay flat on the water
				if(entity.curstate.velocity[2] > 0)
					entity.curstate.velocity[2] = 0;
			}

			ptemp->prevcontents = contents;
		}

		// Add in effects of black holes, and kill the entity if the black hole swallowed it
		if(!gBlackHoleRenderer.AffectObject(ptemp->entity.curstate.origin, ptemp->entity.curstate.velocity, gravityAmount))
			return false;
	}

	if(!R_AddTempEntity(&entity) && !(ptemp->flags & TE_FL_PERSIST))
		return false;
	else
		return true;
}

//=============================================
// @brief
//
//=============================================
void CTempEntityManager::PlayTempEntitySound( tempentity_t *ptempentity, Float volume )
{
	CString filepath;
	switch(ptempentity->soundtype)
	{
	case TE_BOUNCE_GLASS:
		filepath << DEBRIS_SOUND_BASE_DIR << PATH_SLASH_CHAR << "glass_clatter" << (Int32)(Common::RandomLong(1, 3)) << ".wav";
		break;
	case TE_BOUNCE_METAL:
		filepath << DEBRIS_SOUND_BASE_DIR << PATH_SLASH_CHAR << "metal_clatter" << (Int32)(Common::RandomLong(1, 3)) << ".wav";
		break;
	case TE_BOUNCE_FLESH:
		filepath << DEBRIS_SOUND_BASE_DIR << PATH_SLASH_CHAR << "flesh_splatter" << (Int32)(Common::RandomLong(1, 7)) << ".wav";
		break;
	case TE_BOUNCE_WOOD:
		filepath << DEBRIS_SOUND_BASE_DIR << PATH_SLASH_CHAR << "wood_clatter" << (Int32)(Common::RandomLong(1, 3)) << ".wav";
		break;
	case TE_BOUNCE_SHELL:
		filepath << DEBRIS_SOUND_BASE_DIR << PATH_SLASH_CHAR << "shell" << (Int32)(Common::RandomLong(1, 3)) << ".wav";
		break;
	case TE_BOUNCE_CONCRETE:
		filepath << DEBRIS_SOUND_BASE_DIR << PATH_SLASH_CHAR << "concrete_clatter" << (Int32)(Common::RandomLong(1, 3)) << ".wav";
		break;
	case TE_BOUNCE_SHOTSHELL:
		filepath << DEBRIS_SOUND_BASE_DIR << PATH_SLASH_CHAR << "shotshell" << (Int32)(Common::RandomLong(1, 3)) << ".wav";
		break;
	default:
		Con_Printf("%s - Unknown sound type '%d'.\n", __FUNCTION__, ptempentity->soundtype);
		return;
	}

	gSoundEngine.PlaySound(filepath.c_str(), &ptempentity->entity.curstate.origin, SND_FL_TEMPENT, SND_CHAN_AUTO, volume, PITCH_NORM);
}

//====================================
//
//====================================
void CTempEntityManager::CreateFunnel( const Vector& origin, const Vector& color, Float alpha, Uint32 modelindex, bool reverse )
{
	if(!modelindex)
	{
		Con_Printf("%s - No model specified.\n", __FUNCTION__);
		return;
	}

	cache_model_t* pmodel = gModelCache.GetModelByIndex(modelindex);
	if(!pmodel)
	{
		Con_Printf("%s - %d is not a valid model index.\n", __FUNCTION__, (Int32)modelindex);
		return;
	}

	if(pmodel->type != MOD_SPRITE)
	{
		Con_Printf("%s - %d is not a sprite model.\n", __FUNCTION__, (Int32)modelindex);
		return;
	}

	for(Int32 i = -256; i <= 256; i += 32)
	{
		for(Int32 j = -256; j <= 256; j += 32)
		{
			tempentity_t* ptemp = nullptr;

			Vector dir;
			Float velocity;

			if(reverse)
			{
				// Set particle origin
				ptemp = AllocTempEntity(origin, pmodel);
				if(!ptemp)
					return;

				// Set final destination
				Vector dest;
				dest[0] = origin[0]+i;
				dest[1] = origin[1]+j;
				dest[2] = origin[2] + Common::RandomFloat(100, 800);

				// Calculate velocity
				dir = dest - ptemp->entity.curstate.origin;
				velocity = SDL_fabs(dir[2])/8;
			}
			else
			{
				// Set tempent origin
				Vector entorigin;
				entorigin[0] = origin[0] + i;
				entorigin[1] = origin[1] + j;
				entorigin[2] = origin[2] + Common::RandomFloat(100, 800);

				ptemp = AllocTempEntity(entorigin, pmodel);
				if(!ptemp)
					return;

				// Calculate direction and velocity
				dir = origin-ptemp->entity.curstate.origin;
				velocity = SDL_fabs(dir[2])/8;
			}

			if(color.IsZero())
				ptemp->entity.curstate.rendercolor = Vector(0, 255, 0);
			else
				ptemp->entity.curstate.rendercolor = color;

			ptemp->entity.curstate.rendermode = RENDER_TRANSADDITIVE;
			ptemp->entity.curstate.renderamt = (alpha > 0) ? alpha : 255;
			ptemp->startrenderamt = ptemp->entity.curstate.renderamt;
			ptemp->entity.curstate.framerate = 10;
			ptemp->entity.pmodel = pmodel;

			ptemp->flags = TE_FL_ROTATE|TE_FL_FADEOUT;
			ptemp->fadespeed = 2.0f;

			Float dist = Math::VectorNormalize(dir);
			if(velocity < 64)
				velocity = 64;

			velocity += Common::RandomFloat(64, 128);
			Math::VectorScale(dir, velocity, ptemp->entity.curstate.velocity);

			// Die when getting there
			Float particlelife = dist / velocity;
			ptemp->die = cls.cl_time + particlelife - 0.5f;
		}
	}
}

//====================================
//
//====================================
void CTempEntityManager::CreateBreakModel( const Vector& origin, const Vector& size, const Vector& velocity, Uint32 random, Float life, Uint32 num, Uint32 modelindex, Int32 sound, Float bouyancy, Float waterfriction, Int32 flags )
{
	if(!modelindex)
	{
		Con_Printf("%s - No model specified.\n", __FUNCTION__);
		return;
	}

	cache_model_t* pmodel = gModelCache.GetModelByIndex(modelindex);
	if(!pmodel)
	{
		Con_Printf("%s - %d is not a valid model index.\n", __FUNCTION__, (Int32)modelindex);
		return;
	}

	Uint32 _num = num;
	if(_num == 0)
		_num = (size[0]*size[1]+size[1]*size[2]+size[2]*size[0])/(3*SHARD_VOLUME*SHARD_VOLUME);

	if(_num > 100)
		_num = 100;

	// Get number of variations
	Uint64 framecount = Cache_GetModelFrameCount(*pmodel);

	// Create the pieces
	for(Uint32 i = 0; i < _num; i++)
	{
		Vector pieceorigin;
		for(Uint32 j = 0; j < 3; j++)
			pieceorigin[j] = origin[j] + Common::RandomFloat(-0.5, 0.5)*size[j];

		tempentity_t* ptemp = AllocTempEntity(pieceorigin, pmodel);
		if(!ptemp)
			return;

		ptemp->entity.pmodel = pmodel;
		ptemp->entity.curstate.origin = pieceorigin;

		if(pmodel->type == MOD_SPRITE)
			ptemp->entity.curstate.frame = Common::RandomLong(0, framecount-1);
		else
			ptemp->entity.curstate.body = Common::RandomLong(0, framecount-1);

		if(Common::RandomLong(0, 255) < 200)
		{
			ptemp->flags |= TE_FL_ROTATE;
			for(Uint32 j = 0; j < 3; j++)
				ptemp->entity.curstate.angles[j] = Common::RandomFloat(-360, 360);
		}

		if(sound == TE_BOUNCE_GLASS)
		{
			ptemp->entity.curstate.rendermode = RENDER_TRANSTEXTURE_LIT;
			ptemp->entity.curstate.renderamt = Common::RandomFloat(120, 160);
		}
		else
		{
			ptemp->entity.curstate.rendermode = RENDER_NORMAL;
			ptemp->entity.curstate.renderamt = 255;
		}

		ptemp->entity.curstate.velocity[0] = velocity[0] + Common::RandomFloat(-(Int32)random, random);
		ptemp->entity.curstate.velocity[1] = velocity[1] + Common::RandomFloat(-(Int32)random, random);
		ptemp->entity.curstate.velocity[2] = velocity[2] + Common::RandomFloat(0, random);

		ptemp->startrenderamt = ptemp->entity.curstate.renderamt;
		ptemp->flags |= (flags|TE_FL_COLLIDEWORLD|TE_FL_FADEOUT|TE_FL_SLOWGRAVITY);
		ptemp->die = cls.cl_time + life + Common::RandomFloat(-1, 1);
		ptemp->soundtype = sound;
		ptemp->buoyancy = bouyancy;
		ptemp->waterfriction = waterfriction;
	}
}

//====================================
//
//====================================
void CTempEntityManager::CreateBubbles( const Vector& mins, const Vector& maxs, Float height, Uint32 modelindex, Uint32 num, Float speed )
{
	if(!modelindex)
	{
		Con_Printf("%s - No model specified.\n", __FUNCTION__);
		return;
	}

	cache_model_t* pmodel = gModelCache.GetModelByIndex(modelindex);
	if(!pmodel)
	{
		Con_Printf("%s - %d is not a valid model index.\n", __FUNCTION__, (Int32)modelindex);
		return;
	}

	if(pmodel->type != MOD_SPRITE)
	{
		Con_Printf("%s - %d is not a sprite model.\n", __FUNCTION__, (Int32)modelindex);
		return;
	}

	// Get number of variations
	Uint64 framecount = Cache_GetModelFrameCount(*pmodel);

	for(Uint32 i = 0; i < num; i++)
	{
		Vector origin;
		for(Uint32 j = 0; j < 3; j++)
			origin[j] = Common::RandomFloat(mins[j], maxs[j]);

		tempentity_t* ptemp = AllocTempEntity(origin, pmodel);
		if(!ptemp)
			return;

		ptemp->entity.curstate.origin = origin;
		ptemp->entity.pmodel = pmodel;

		ptemp->flags |= TE_FL_SINEWAVE;
		ptemp->add = origin;

		Float rand = Common::RandomFloat(-M_PI, M_PI);
		Float s = SDL_sin(rand);
		Float c = SDL_cos(rand);

		Float zspeed = Common::RandomFloat(80, 140);
		ptemp->entity.curstate.velocity = Vector(speed*c, speed*s, zspeed);
		ptemp->die = cls.cl_time + ((height-(origin[2]-mins[2]))/zspeed)-0.1;
		ptemp->entity.curstate.frame = Common::RandomLong(0, framecount-1);

		ptemp->entity.curstate.scale = 1.0 / Common::RandomFloat(4, 16);
		ptemp->entity.curstate.rendermode = RENDER_TRANSADDITIVE;
		ptemp->entity.curstate.rendercolor = Vector(255, 255, 255);
		ptemp->entity.curstate.renderamt = Common::RandomFloat(150, 255);
		ptemp->startrenderamt = ptemp->entity.curstate.renderamt;
	}
}

//====================================
//
//====================================
void CTempEntityManager::CreateBubbleTrail( const Vector& start, const Vector& end, Float height, Uint32 modelindex, Uint32 num, Float speed )
{
	if(!modelindex)
	{
		Con_Printf("%s - No model specified.\n", __FUNCTION__);
		return;
	}

	cache_model_t* pmodel = gModelCache.GetModelByIndex(modelindex);
	if(!pmodel)
	{
		Con_Printf("%s - %d is not a valid model index.\n", __FUNCTION__, (Int32)modelindex);
		return;
	}

	if(pmodel->type != MOD_SPRITE)
	{
		Con_Printf("%s - %d is not a sprite model.\n", __FUNCTION__, (Int32)modelindex);
		return;
	}

	// Get number of variations
	Uint64 framecount = Cache_GetModelFrameCount(*pmodel);

	for(Uint32 i = 0; i < num; i++)
	{
		Float dist = Common::RandomFloat(0.0, 1.0);
		Vector origin;
		for(Uint32 j = 0; j < 3; j++)
			origin[j] = start[j] + dist * (end[j]-start[j]);

		tempentity_t* ptemp = AllocTempEntity(origin, pmodel);
		if(!ptemp)
			return;

		ptemp->entity.curstate.origin = origin;
		ptemp->entity.pmodel = pmodel;

		ptemp->flags |= TE_FL_SINEWAVE;
		ptemp->add = origin;

		Float rand = Common::RandomFloat(-M_PI, M_PI);
		Float s = SDL_sin(rand);
		Float c = SDL_cos(rand);

		Float zspeed = Common::RandomFloat(80, 140);
		ptemp->entity.curstate.velocity = Vector(speed*c, speed*s, zspeed);
		ptemp->die = cls.cl_time + ((height-(origin[2]-start[2]))/zspeed)-0.1;
		ptemp->entity.curstate.frame = Common::RandomLong(0, framecount-1);

		ptemp->entity.curstate.scale = 1.0 / Common::RandomFloat(4, 8);
		ptemp->entity.curstate.rendermode = RENDER_TRANSADDITIVE;
		ptemp->entity.curstate.rendercolor = Vector(255, 255, 255);
		ptemp->entity.curstate.renderamt = Common::RandomFloat(150, 255);
		ptemp->startrenderamt = ptemp->entity.curstate.renderamt;
	}
}

//====================================
//
//====================================
tempentity_t* CTempEntityManager::CreateTempModel( const Vector& origin, const Vector& velocity, const Vector& angles, Float life, Uint32 modelindex, Int32 sound, Float bouyancy, Float waterfriction, Int32 flags )
{
	if(!modelindex)
	{
		Con_Printf("%s - No model specified.\n", __FUNCTION__);
		return nullptr;
	}

	cache_model_t* pmodel = gModelCache.GetModelByIndex(modelindex);
	if(!pmodel)
	{
		Con_Printf("%s - %d is not a valid model index.\n", __FUNCTION__, (Int32)modelindex);
		return nullptr;
	}

	if(pmodel->type != MOD_VBM)
	{
		Con_Printf("%s - %d is not a vbm model.\n", __FUNCTION__, (Int32)modelindex);
		return nullptr;
	}

	// Create the tempentity
	tempentity_t* ptemp = AllocTempEntity(origin, pmodel);
	if(!ptemp)
		return nullptr;

	ptemp->entity.curstate.rendermode = RENDER_NORMAL;
	ptemp->entity.curstate.renderamt = 255;
	ptemp->startrenderamt = ptemp->entity.curstate.renderamt;
	ptemp->entity.curstate.velocity = velocity;
	ptemp->entity.curstate.angles = angles;
	ptemp->entity.curstate.body = Common::RandomLong(0, Cache_GetModelFrameCount(*pmodel)-1);

	if(!(flags & TE_FL_NOGRAVITY))
	{
		if(sound & (TE_BOUNCE_SHELL|TE_BOUNCE_SHOTSHELL))
		{
			for(Uint32 i = 0; i < 3; i++)
				ptemp->entity.curstate.avelocity[i] = Common::RandomFloat(-200, 200);

			ptemp->flags |= TE_FL_ROTATE;
			if(sound & TE_BOUNCE_SHOTSHELL)
				ptemp->flags |= TE_FL_SLOWGRAVITY;
		}

		// Set basic flags, if client wants to override them, he can do it later
		ptemp->flags |= (flags|TE_FL_COLLIDEWORLD|TE_FL_GRAVITY);
	}

	ptemp->soundtype = sound;
	ptemp->die = cls.cl_time + life;
	ptemp->buoyancy = bouyancy;
	ptemp->waterfriction = waterfriction;

	return ptemp;
}

//====================================
//
//====================================
tempentity_t* CTempEntityManager::CreateTempSprite( const Vector& origin, const Vector& velocity, Float scale, Uint32 modelindex, Int32 rendermode, Int32 renderfx, Float alpha, Float life, Int32 sound, Int32 flags )
{
	if(!modelindex)
	{
		Con_Printf("%s - No model specified.\n", __FUNCTION__);
		return nullptr;
	}

	cache_model_t* pmodel = gModelCache.GetModelByIndex(modelindex);
	if(!pmodel)
	{
		Con_Printf("%s - %d is not a valid model index.\n", __FUNCTION__, (Int32)modelindex);
		return nullptr;
	}

	if(pmodel->type != MOD_SPRITE)
	{
		Con_Printf("%s - %d is not a sprite model.\n", __FUNCTION__, (Int32)modelindex);
		return nullptr;
	}

	// Get number of variations
	Uint64 framecount = Cache_GetModelFrameCount(*pmodel);

	// Create the tempentity
	tempentity_t* ptemp = AllocTempEntity(origin, pmodel);
	if(!ptemp)
		return nullptr;

	ptemp->entity.curstate.framerate = 10;
	ptemp->entity.curstate.rendermode = (rendermode_t)rendermode;
	ptemp->entity.curstate.renderamt = alpha * 255;
	ptemp->startrenderamt = ptemp->entity.curstate.renderamt;
	ptemp->entity.curstate.rendercolor = Vector(255, 255, 255);
	ptemp->entity.curstate.velocity = velocity;
	ptemp->entity.curstate.frame = 0;
	ptemp->entity.curstate.scale = scale;

	ptemp->flags |= flags;
	ptemp->soundtype = sound;
	ptemp->framemax = framecount - 1;

	if(life)
		ptemp->die = cls.cl_time + life;
	else
		ptemp->die = cls.cl_time + (framecount / ptemp->entity.curstate.framerate);

	return ptemp;
}

//====================================
//
//====================================
void CTempEntityManager::CreateSphereModel( const Vector& origin, Float speed, Float life, Uint32 num, Uint32 modelindex, Float bouyancy, Float waterfriction, Int32 sound )
{
	if(!modelindex)
	{
		Con_Printf("%s - No model specified.\n", __FUNCTION__);
		return;
	}

	cache_model_t* pmodel = gModelCache.GetModelByIndex(modelindex);
	if(!pmodel)
	{
		Con_Printf("%s - %d is not a valid model index.\n", __FUNCTION__, (Int32)modelindex);
		return;
	}

	if(pmodel->type != MOD_VBM)
	{
		Con_Printf("%s - %d is not a vbm model.\n", __FUNCTION__, (Int32)modelindex);
		return;
	}

	// Get number of variations
	Uint64 framecount = Cache_GetModelFrameCount(*pmodel);

	// Create the pieces
	for(Uint32 i = 0; i < num; i++)
	{
		tempentity_t* ptemp = AllocTempEntity(origin, pmodel);
		if(!ptemp)
			return;

		ptemp->entity.pmodel = pmodel;
		ptemp->entity.curstate.origin = origin;
		ptemp->entity.curstate.body = Common::RandomLong(0, framecount-1);

		if(Common::RandomLong(0, 255) < 100)
			ptemp->flags |= TE_FL_SLOWGRAVITY;
		else
			ptemp->flags |= TE_FL_GRAVITY;

		if(Common::RandomLong(0, 255) < 200)
		{
			ptemp->flags |= TE_FL_ROTATE;
			for(Uint32 j = 0; j < 3; j++)
				ptemp->entity.curstate.angles[j] = Common::RandomFloat(-360, 360);
		}

		if(Common::RandomLong(0, 255) < 100)
			ptemp->flags |= TE_FL_SMOKETRAIL;

		if(sound == TE_BOUNCE_GLASS)
		{
			ptemp->entity.curstate.rendermode = RENDER_TRANSTEXTURE_LIT;
			ptemp->entity.curstate.renderamt = Common::RandomFloat(120, 160);
			ptemp->startrenderamt = ptemp->entity.curstate.renderamt;
		}
		else
		{
			ptemp->entity.curstate.rendermode = RENDER_NORMAL;
			ptemp->entity.curstate.renderamt = 255;
		}

		for(Uint32 j = 0; j < 3; j++)
			ptemp->entity.curstate.velocity[j] = Common::RandomFloat(-1.0, 1.0);

		ptemp->entity.curstate.velocity.Normalize();
		Math::VectorScale(ptemp->entity.curstate.velocity, speed, ptemp->entity.curstate.velocity);

		ptemp->flickertime = cls.cl_time + Common::RandomFloat(0.1, 0.8);
		ptemp->startrenderamt = ptemp->entity.curstate.renderamt;
		ptemp->flags |= (TE_FL_COLLIDEWORLD|TE_FL_FLICKER);
		ptemp->die = cls.cl_time + life;
		ptemp->soundtype = sound;
		ptemp->buoyancy = bouyancy;
		ptemp->waterfriction = waterfriction;
	}
}
