/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "texturemanager.h"
#include "r_basicdraw.h"
#include "r_legacyparticles.h"
#include "r_main.h"

#include "cvar.h"
#include "console.h"
#include "system.h"
#include "cl_main.h"
#include "com_math.h"
#include "file.h"
#include "enginestate.h"
#include "r_blackhole.h"

//
// This code was written while referencing the Quake 1 SDK, later expanded with some code by Enko
// to support some HL1-specific effects.
// I didn't need to add this, but I find Quake 1 particles so nostalgic that I couldn't resist.
//

// Ramp 1 values
const Uint32 CLegacyParticles::PARTICLE_RAMP1[8] = {0x6f, 0x6d, 0x6b, 0x69, 0x67, 0x65, 0x63, 0x61};
// Ramp 2 values
const Uint32 CLegacyParticles::PARTICLE_RAMP2[8] = {0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x68, 0x66};
// Ramp 3 values
const Uint32 CLegacyParticles::PARTICLE_RAMP3[8] = {0x6d, 0x6b, 6, 5, 4, 3};

// Path to particle texture
const Char CLegacyParticles::PARTICLE_TEXTURE_FILEPATH[] = "general/particle.tga";
// Path to palette file
const Char CLegacyParticles::PALETTE_FILEPATH[] = "textures/general/palette.lmp";

// Object definition
CLegacyParticles gLegacyParticles;

//====================================
//
//====================================
Int32 Legacy_SortParticles( const void* p1, const void* p2 )
{
	const CLegacyParticles::particle_t **pp1 = static_cast<const CLegacyParticles::particle_t **>(const_cast<void*>(p1));
	const CLegacyParticles::particle_t **pp2 = static_cast<const CLegacyParticles::particle_t **>(const_cast<void*>(p2));

	Float length1 = ((*pp1)->origin - rns.view.v_origin).Length();
	Float length2 = ((*pp2)->origin - rns.view.v_origin).Length();

	if(length1 < length2)
		return 1;
	else if(length1 == length2)
		return 0;
	else
		return -1;
}

//====================================
//
//====================================
CLegacyParticles::CLegacyParticles( void ):
	m_pParticleTexture(nullptr),
	m_pFreeParticleHeader(nullptr),
	m_pActiveParticleHeader(nullptr),
	m_nbSortedParticles(0),
	m_pCvarGravity(nullptr),
	m_pColorPalette(nullptr)
{
	AllocateParticles();
}

//====================================
//
//====================================
CLegacyParticles::~CLegacyParticles( void )
{
	Shutdown();
}

//====================================
//
//====================================
bool CLegacyParticles::Init( void )
{
	m_pCvarGravity = gConsole.GetCVar("sv_gravity");
	if(!m_pCvarGravity)
	{
		Con_Printf("%s - Failed to get cvar 'sv_gravity'.\n", __FUNCTION__);
		return false;
	}

	// Load the palette
	Uint32 filesize = 0;
	const byte* ppalfile = FL_LoadFile(PALETTE_FILEPATH, &filesize);
	if(!ppalfile)
	{
		Con_Printf("%s - Failed to load '%s'.\n", __FUNCTION__, PALETTE_FILEPATH);
		return false;
	}

	m_pColorPalette = new color24_t[256];
	memcpy(m_pColorPalette, ppalfile, sizeof(byte)*filesize);
	FL_FreeFile(ppalfile);

	return true;
}

//====================================
//
//====================================
void CLegacyParticles::Shutdown( void )
{
	ReleaseAllParticles();

	if(m_pColorPalette)
	{
		delete[] m_pColorPalette;
		m_pColorPalette = nullptr;
	}
}

//====================================
//
//====================================
void CLegacyParticles::ReleaseAllParticles( void )
{
	FreeActiveParticles();

	if(!m_pParticleBlocksArray.empty())
	{
		for(Uint32 i = 0; i < m_pParticleBlocksArray.size(); i++)
			delete[] m_pParticleBlocksArray[i];

		m_pParticleBlocksArray.clear();
	}

	m_pSortedParticles.clear();

	if(m_pFreeParticleHeader)
		m_pFreeParticleHeader = nullptr;
}

//====================================
//
//====================================
bool CLegacyParticles::InitGL( void )
{
	return true;
}

//====================================
//
//====================================
void CLegacyParticles::ClearGL( void )
{
}

//====================================
//
//====================================
bool CLegacyParticles::InitGame( void )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	m_pParticleTexture = pTextureManager->LoadTexture(PARTICLE_TEXTURE_FILEPATH, RS_GAME_LEVEL);
	if(!m_pParticleTexture)
		m_pParticleTexture = pTextureManager->GetDummyTexture();

	return true;
}

//====================================
//
//====================================
void CLegacyParticles::ClearGame( void )
{
	// Release all particle allocations
	ReleaseAllParticles();
	// Reset to base size
	AllocateParticles();
}

//====================================
//
//====================================
void CLegacyParticles::AllocateParticles( void )
{
	particle_t* pnewblock = new particle_t[LEGACY_PARTICLE_ALLOC_COUNT];
	m_pParticleBlocksArray.push_back(pnewblock);

	// Allocate particles
	for(Uint32 i = 0; i < LEGACY_PARTICLE_ALLOC_COUNT; i++)
	{
		particle_t* pnew = &pnewblock[i];

		if(m_pFreeParticleHeader)
			m_pFreeParticleHeader->pprev = pnew;
		pnew->pnext = m_pFreeParticleHeader;

		m_pFreeParticleHeader = pnew;
	}

	Uint32 prevSize = m_pSortedParticles.size();
	m_pSortedParticles.resize(prevSize + LEGACY_PARTICLE_ALLOC_COUNT);
	for(Uint32 i = prevSize; i < LEGACY_PARTICLE_ALLOC_COUNT; i++)
		m_pSortedParticles[i] = nullptr;
}

//====================================
//
//====================================
void CLegacyParticles::FreeActiveParticles( void )
{
	if(!m_pActiveParticleHeader)
		return;

	m_pActiveParticleHeader = nullptr;
}

//====================================
//
//====================================
void CLegacyParticles::FreeParticle( particle_t* pparticle )
{
	if(!pparticle->pprev) 
		m_pActiveParticleHeader = pparticle->pnext;
	else 
		pparticle->pprev->pnext = pparticle->pnext;

	if(pparticle->pnext) 
		pparticle->pnext->pprev = pparticle->pprev;

	// Clear this particle
	(*pparticle) = particle_t();

	// Link to freed entities
	pparticle->pnext = m_pFreeParticleHeader;
	if(m_pFreeParticleHeader)
		m_pFreeParticleHeader->pprev = pparticle;

	m_pFreeParticleHeader = pparticle;
}

//====================================
//
//====================================
CLegacyParticles::particle_t* CLegacyParticles::AllocParticle( void )
{
	// Allocate if needed
	if(!m_pFreeParticleHeader)
		AllocateParticles();

	particle_t* pnew = m_pFreeParticleHeader;
	m_pFreeParticleHeader = pnew->pnext;
	if(m_pFreeParticleHeader)
		m_pFreeParticleHeader->pprev = nullptr;

	// Clear particle state
	(*pnew) = particle_t();

	// Add system into pointer array
	if(m_pActiveParticleHeader)
	{
		m_pActiveParticleHeader->pprev = pnew;
		pnew->pnext = m_pActiveParticleHeader;
	}
	m_pActiveParticleHeader = pnew;

	return pnew;
}

//====================================
//
//====================================
void CLegacyParticles::CreateParticleExplosion1( const Vector& origin )
{
	for(Uint32 i = 0; i < 1024; i++)
	{
		particle_t* pnew = AllocParticle();
		if(!pnew)
			return;

		pnew->die = cls.cl_time + 5;
		pnew->color = PARTICLE_RAMP1[0];
		pnew->ramp = Common::RandomLong(0, 3);

		if(i & 1)
			pnew->type = pt_explode1;
		else
			pnew->type = pt_explode2;

		while(true)
		{
			for(Uint32 j = 0; j < 3; j++)
				pnew->velocity[j] = Common::RandomFloat(-512, 512);

			if(pnew->velocity.Length() <= 512)
				break;
		}

		for(Uint32 j = 0; j < 3; j++)
			pnew->origin[j] = origin[j] + pnew->velocity[j] * 0.25;
	}
}

//====================================
//
//====================================
void CLegacyParticles::CreateParticleExplosion2( const Vector& origin, Uint32 colorstart, Uint32 colorlength )
{
	Uint32 colormod = 0;

	for(Uint32 i = 0; i < 512; i++)
	{
		particle_t* pnew = AllocParticle();
		if(!pnew)
			return;

		pnew->die = cls.cl_time + 0.3;
		pnew->color = colorstart + (colormod%colorlength);
		pnew->ramp = 0;
		pnew->type = pt_blob1;
		colormod++;

		for(Uint32 j = 0; j < 3; j++)
		{
			pnew->origin[j] = origin[j] + Common::RandomFloat(-16, 16);
			pnew->velocity[j] = Common::RandomFloat(-256, 256);
		}
	}
}

//====================================
//
//====================================
void CLegacyParticles::CreateBlobExplosion( const Vector& origin )
{
	for(Uint32 i = 0; i < 1024; i++)
	{
		particle_t* pnew = AllocParticle();
		if(!pnew)
			return;

		pnew->die = cls.cl_time + Common::RandomFloat(1, 1.4);
		pnew->ramp = Common::RandomLong(0, 3);

		if(i & 1)
		{
			pnew->type = pt_blob1;
			pnew->color = 66 + Common::RandomLong(0, 6);
		}
		else
		{
			pnew->type = pt_blob2;
			pnew->color = 150 + Common::RandomLong(0, 6);
		}

		for(Uint32 j = 0; j < 3; j++)
		{
			pnew->origin[j] = origin[j] + Common::RandomFloat(-16, 16);
			pnew->velocity[j] = Common::RandomFloat(-256, 256);
		}
	}
}

//====================================
//
//====================================
void CLegacyParticles::CreateRocketExplosion( const Vector& origin, Uint32 color )
{
	for(Uint32 i = 0; i < 1024; i++)
	{
		particle_t* pnew = AllocParticle();
		if(!pnew)
			return;

		pnew->die = cls.cl_time + 5;
		pnew->color = PARTICLE_RAMP1[0];
		pnew->ramp = Common::RandomFloat(0, 3);

		if(i & 1)
			pnew->type = pt_explode1;
		else
			pnew->type = pt_explode2;

		for(Uint32 j = 0; j < 3; j++)
		{
			pnew->origin[j] = origin[j] + Common::RandomFloat(-16, 16);
			pnew->velocity[j] = Common::RandomFloat(-256, 256);
		}
	}
}

//====================================
//
//====================================
void CLegacyParticles::CreateParticleEffect( const Vector& origin, const Vector& velocity, Uint32 color, Uint32 count )
{
	for(Uint32 i = 0; i < count; i++)
	{
		particle_t* pnew = AllocParticle();
		if(!pnew)
			return;

		pnew->die = cls.cl_time + 0.1+Common::RandomLong(0, 5);
		pnew->color = (color&~7) + Common::RandomLong(0, 7);
		pnew->type = pt_slowgravity;

		for(Uint32 j = 0; j < 3; j++)
		{
			pnew->origin[j] = origin[j] + Common::RandomFloat(-16, 16);
			pnew->velocity[j] = velocity[j]*15;
		}
	}
}

//====================================
//
//====================================
void CLegacyParticles::CreateLavaSplash( const Vector& origin )
{
	for(Int32 i = -16; i < 16; i++)
	{
		for(Int32 j = -16; j < 16; j++)
		{
			for(Int32 k = 0; k < 1; k++)
			{
				particle_t* pnew = AllocParticle();
				if(!pnew)
					return;

				pnew->die = cls.cl_time + Common::RandomFloat(2, 2.62);
				pnew->color = Common::RandomLong(224, 231);
				pnew->type = pt_slowgravity;

				Vector dir;
				dir[0] = j*8 + Common::RandomFloat(0, 7);
				dir[1] = i*8 + Common::RandomFloat(0, 7);
				dir[2] = 256;

				pnew->origin[0] = origin[0] + dir[0];
				pnew->origin[1] = origin[1] + dir[1];
				pnew->origin[2] = origin[2] + Common::RandomLong(0, 63);

				Math::VectorNormalize(dir);
				Float velocity = Common::RandomFloat(50, 113);
				Math::VectorScale(dir, velocity, pnew->velocity);
			}
		}
	}
}

//====================================
//
//====================================
void CLegacyParticles::CreateTeleportSplash( const Vector& origin )
{
	for(Int32 i = -16; i < 16; i +=4 )
	{
		for(Int32 j = -16; j < 16; j += 4)
		{
			for(Int32 k = -24; k < 32; k+= 4)
			{
				particle_t* pnew = AllocParticle();
				if(!pnew)
					return;

				pnew->die = cls.cl_time + Common::RandomFloat(0.2, 0.84);
				pnew->color = Common::RandomLong(7, 14);
				pnew->type = pt_slowgravity;

				Vector dir;
				dir[0] = j*8;
				dir[1] = i*8;
				dir[2] = k*8;

				pnew->origin[0] = origin[0] + i + Common::RandomFloat(0, 3);
				pnew->origin[1] = origin[1] + j + Common::RandomFloat(0, 3);
				pnew->origin[2] = origin[2] + k + Common::RandomFloat(0, 3);

				Math::VectorNormalize(dir);
				Float velocity = Common::RandomFloat(50, 113);
				Math::VectorScale(dir, velocity, pnew->velocity);
			}
		}
	}
}

//====================================
//
//====================================
void CLegacyParticles::CreateRocketTrail( const Vector& start, const Vector& end, Uint32 type )
{
	Int32 dec;
	Int32 tc = 0;

	Int32 _type = type;
	if(type < 128)
		dec = 3;
	else
	{
		dec = 1;
		_type -= 128;
	}

	Vector _start = start;
	Vector vec = end - start;
	Float length = Math::VectorNormalize(vec);

	while(length > 0)
	{
		length -= dec;

		particle_t* pnew = AllocParticle();
		if(!pnew)
			return;

		pnew->velocity.Clear();

		switch(type)
		{
		case trail_rocket:
			{
				pnew->ramp = Common::RandomLong(0, 3);
				pnew->color = PARTICLE_RAMP3[static_cast<Uint32>(pnew->ramp)];
				pnew->type = pt_fire;
				pnew->die = cls.cl_time + 2;

				for(Uint32 j = 0; j < 3; j++)
					pnew->origin[j] = _start[j] + Common::RandomFloat(-3, 3);
			}
			break;
		case trail_smoke:
			{
				pnew->ramp = Common::RandomLong(2, 5);
				pnew->color = PARTICLE_RAMP3[static_cast<Uint32>(pnew->ramp)];
				pnew->type = pt_fire;
				pnew->die = cls.cl_time + 2;

				for(Uint32 j = 0; j < 3; j++)
					pnew->origin[j] = _start[j] + Common::RandomFloat(-3, 3);
			}
			break;
		case trail_blood:
			{
				pnew->ramp = Common::RandomLong(2, 5);
				pnew->color = Common::RandomLong(67, 70);
				pnew->type = pt_gravity;
				pnew->die = cls.cl_time + 2;

				for(Uint32 j = 0; j < 3; j++)
					pnew->origin[j] = _start[j] + Common::RandomFloat(-3, 3);
			}
			break;
		case trail_tracer1:
		case trail_tracer2:
			{
				pnew->die = cls.cl_time + 0.5;
				pnew->type = pt_static;
				if(type == 3)
					pnew->color = 52 + ((tc&4)<<1);
				else
					pnew->color = 230 + ((tc&4)<<1);
				tc++;

				pnew->origin = _start;
				if(tc&1)
				{
					pnew->velocity[0] = 30*vec[1];
					pnew->velocity[1] = 30*-vec[0];
				}
				else
				{
					pnew->velocity[0] = 30*-vec[1];
					pnew->velocity[1] = 30*vec[0];
				}
			}
			break;
		case trail_slightblood:
			{
				pnew->type = pt_gravity;
				pnew->color = Common::RandomLong(67, 70);
				pnew->die = cls.cl_time + 2;

				for(Uint32 j = 0; j < 3; j++)
					pnew->origin[j] = _start[j] + Common::RandomFloat(-3, 3);

				length -= 3;
			}
			break;
		case trail_voortrail:
			{
				pnew->color = Common::RandomLong(153, 156);
				pnew->type = pt_static;
				pnew->die = cls.cl_time + 0.3;

				for(Uint32 j = 0; j < 3; j++)
					pnew->origin[j] = _start[j] + Common::RandomFloat(-8, 8);
			}
			break;
		}

		Math::VectorAdd(_start, vec, _start);
	}
}

//====================================
//
//====================================
void CLegacyParticles::CreateLargeFunnel( const Vector& origin, bool reverse )
{
	for(Int32 i = -256; i <= 256; i += 32)
	{
		for(Int32 j = -256; j <= 256; j += 32)
		{
			particle_t* pnew = AllocParticle();
			if(!pnew)
				return;

			Vector dir;
			Float velocity;
			if(reverse)
			{
				pnew->origin = origin;

				// Set final destination
				Vector dest;
				dest[0] = origin[0]+i;
				dest[1] = origin[1]+j;
				dest[2] = origin[2] + Common::RandomFloat(100, 800);

				// Calculate velocity
				dir = dest - pnew->origin;
			}
			else
			{
				// Set tempent origin
				pnew->origin[0] = origin[0] + i;
				pnew->origin[1] = origin[1] + j;
				pnew->origin[2] = origin[2] + Common::RandomFloat(100, 800);

				// Calculate direction and velocity
				dir = origin - pnew->origin;
			}

			pnew->type = pt_static;
			pnew->color = 244;

			velocity = SDL_fabs(dir[2])/8;
			if(velocity < 64)
				velocity = 64;

			velocity += Common::RandomFloat(64, 128);
			Float distance = Math::VectorNormalize(dir);
			pnew->velocity = velocity * dir;

			pnew->die = cls.cl_time + (distance / velocity);
		}
	}
}

//====================================
//
//====================================
void CLegacyParticles::CreateBloodStream( const Vector& origin, const Vector& direction, Uint32 color, Float speed )
{
	Vector normDirection(direction);
	Math::VectorNormalize(normDirection);

	Float arc = 0.05;
	Float particleSpeed = speed;

	for(Uint32 i = 0; i < 100; i++)
	{
		particle_t* pnew = AllocParticle();
		if(!pnew)
			return;

		pnew->die = cls.cl_time + 2.0;
		pnew->color = color + Common::RandomLong(0, 9);
		pnew->type = pt_vox_gravity;
		pnew->origin = origin;

		Vector particleDirection(normDirection);
		particleDirection[2] -= arc;
		arc -= 0.005;

		pnew->velocity = particleDirection * particleSpeed;

		// Reduce speed by 1 per each particle to make it look
		// like the original effect from Half-Life 1
		particleSpeed -= 1;
	}

	arc = 0.075;
	for(Uint32 i = 0; i < (speed/5); i++)
	{
		particle_t* pnew = AllocParticle();
		if(!pnew)
			return;

		pnew->die = cls.cl_time + 3.0;
		pnew->type = pt_vox_slowgravity;
		pnew->color = color + Common::RandomLong(0, 9);
		pnew->origin = origin;

		Vector particleDirection(normDirection);
		particleDirection[2] -= arc;
		arc -= 0.005;

		Float num = Common::RandomFloat(0, 1);
		particleSpeed = speed * num;
		num *= 1.7;

		particleDirection = particleDirection * num;
		pnew->velocity = particleDirection * particleSpeed;

		for(Uint32 j = 0; j < 2; j++)
		{
			pnew = AllocParticle();
			if(!pnew)
				return;

			pnew->die = cls.cl_time + 3.0;
			pnew->type = pt_vox_slowgravity;
			pnew->color = color + Common::RandomLong(0, 9);

			for(Uint32 k = 0; k < 3; k++)
				pnew->origin[k] = origin[k] + Common::RandomFloat(-1, 1);

			particleDirection = normDirection;
			particleDirection[2] -= arc;

			particleDirection = particleDirection * num;
			pnew->velocity = particleDirection * particleSpeed;
		}
	}
}

//====================================
//
//====================================
void CLegacyParticles::CreateBloodParticles( const Vector& origin, const Vector& direction, Uint32 color, Float speed )
{
	Vector normDirection(direction);
	Math::VectorNormalize(normDirection);

	Float arc = 0.06;
	Float particleSpeed = speed * 3;

	for(Uint32 i = 0; i < (speed / 2.0); i++)
	{
		Vector particleOrigin;
		Vector particleDirection;

		for(Uint32 j = 0; j < 3; j++)
		{
			particleOrigin[j] = origin[j] + Common::RandomFloat(-3, 3);
			particleDirection[j] = normDirection[j] + Common::RandomFloat(-arc, arc);
		}

		for(Uint32 j = 0; j < 8; j++)
		{
			particle_t* pnew = AllocParticle();
			if(!pnew)
				return;

			pnew->die = cls.cl_time + 1.5;
			pnew->color = color + Common::RandomLong(0, 9);
			pnew->type = pt_vox_gravity;

			for(Uint32 k = 0; k < 3; k++)
				pnew->origin[k] = particleOrigin[k] + Common::RandomFloat(-1, 1);

			pnew->velocity = particleDirection * particleSpeed;
		}

		particleSpeed -= speed;
	}
}

//====================================
//
//====================================
void CLegacyParticles::UpdateParticles( void )
{
	if(!m_pActiveParticleHeader)
		return;

	// Fetch gravity
	Float gravity = m_pCvarGravity->GetValue();

	// Draw the particles
	particle_t* pnext = m_pActiveParticleHeader;
	while(pnext)
	{
		// Free any particles
		if(pnext->die < cls.cl_time)
		{
			particle_t* pfree = pnext;
			pnext = pfree->pnext;
			FreeParticle(pfree);
			continue;
		}

		// Determine gravity amount
		// This comes from Quake 1, where particle_gravity is particle gravity multiplied by 0.05
		Float particle_gravity = 0;
		switch(pnext->type)
		{
		case pt_fire:
		case pt_explode1:
		case pt_explode2:
		case pt_blob1:
		case pt_slowgravity:
				particle_gravity = 0.05;
			break;
		case pt_gravity:
				particle_gravity = 1.0;
			break;
		case pt_vox_gravity:
				particle_gravity = 0.4;
				break;
		case pt_vox_slowgravity:
				particle_gravity = 0.2;
				break;
		case pt_static:
		default:
			break;
		}

		// Add in black hole gravity
		if(!gBlackHoleRenderer.AffectObject(pnext->origin, pnext->velocity, particle_gravity))
		{
			particle_t* pfree = pnext;
			pnext = pfree->pnext;
			FreeParticle(pfree);
			continue;
		}

		// Move particle by velocity
		Math::VectorMA(pnext->origin, cls.frametime, pnext->velocity, pnext->origin);

		// Apply type specific stuff
		switch(pnext->type)
		{
		case pt_fire:
			{
				pnext->ramp += cls.frametime*5;
				if(pnext->ramp >= 6)
					pnext->die = -1;
				else
					pnext->color = PARTICLE_RAMP3[static_cast<Uint32>(pnext->ramp)];

				// Apply gravity
				pnext->velocity[2] += cls.frametime * particle_gravity * gravity;
			}
			break;
		case pt_explode1:
			{
				pnext->ramp += cls.frametime * 10;
				if(pnext->ramp >= 8)
					pnext->die = -1;
				else
					pnext->color = PARTICLE_RAMP1[static_cast<Uint32>(pnext->ramp)];

				// Speed it up a bit
				for(Uint32 i = 0; i < 3; i++)
					pnext->velocity[i] += pnext->velocity[i]*cls.frametime*4;

				// Apply gravity
				pnext->velocity[2] -= cls.frametime * particle_gravity * gravity;
			}
			break;
		case pt_explode2:
			{
				pnext->ramp += cls.frametime * 15;
				if(pnext->ramp >= 8)
					pnext->die = -1;
				else
					pnext->color = PARTICLE_RAMP2[static_cast<Uint32>(pnext->ramp)];

				// Speed it up a bit
				for(Uint32 i = 0; i < 3; i++)
					pnext->velocity[i] -= pnext->velocity[i]*cls.frametime;

				// Apply gravity
				pnext->velocity[2] -= cls.frametime * particle_gravity * gravity;
			}
			break;
		case pt_blob1:
			{
				for(Uint32 i = 0; i < 2; i++)
					pnext->velocity[i] -= pnext->velocity[i]*cls.frametime*4;

				pnext->velocity[2] -= cls.frametime * particle_gravity * gravity;
			}
			break;
		case pt_gravity:
		case pt_slowgravity:
		case pt_vox_gravity:
		case pt_vox_slowgravity:
			{
				pnext->velocity[2] -= cls.frametime * particle_gravity * gravity;
			}
			break;
		case pt_static:
		default:
			break;
		}

		pnext = pnext->pnext;
	}
}

//====================================
//
//====================================
bool CLegacyParticles::DrawParticles( void )
{
	if(!m_pActiveParticleHeader)
		return true;

	Vector forward, up, right;
	Math::AngleVectors(rns.view.v_angles, &forward, &right, &up);

	Math::VectorScale(right, 1.5, right);
	Math::VectorScale(up, 1.5, up);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	if(!pDraw->Enable() || !pDraw->EnableTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	pDraw->SetProjection(rns.view.projection.GetMatrix());
	pDraw->SetModelview(rns.view.modelview.GetMatrix());

	R_Bind2DTexture(GL_TEXTURE0, m_pParticleTexture->palloc->gl_index);

	if(rns.fog.settings.active)
	{
		if(!pDraw->EnableFog())
		{
			Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
			pDraw->Disable();
			return false;
		}

		// We use black fog on beams
		pDraw->SetFogParams(rns.fog.settings.color, rns.fog.settings.start, rns.fog.settings.end);
	}

	R_ValidateShader(pDraw);

	pDraw->Begin(CBasicDraw::DRAW_QUADS);

	// Add rendered particles to the list
	m_nbSortedParticles = 0;

	particle_t* pnext = m_pActiveParticleHeader;
	while(pnext)
	{
		m_pSortedParticles[m_nbSortedParticles] = pnext;
		m_nbSortedParticles++;

		pnext = pnext->pnext;
	}

	// Compensate for texture size
	Float texscalex = (8.0/static_cast<Float>(m_pParticleTexture->width)) / 1.0f;
	Float texscaley = (8.0/static_cast<Float>(m_pParticleTexture->width)) / 1.0f;

	// Sort by distance
	qsort(&m_pSortedParticles[0], m_nbSortedParticles, sizeof(particle_t*), Legacy_SortParticles);

	// Draw the particles
	Uint32 nbVertexes = 0;
	for(Uint32 i = 0; i < m_nbSortedParticles; i++)
	{
		pnext = m_pSortedParticles[i];

		// Draw the particle
		Float scale = (pnext->origin[0] - rns.view.v_origin[0])*forward[0] + 
			(pnext->origin[1] - rns.view.v_origin[1])*forward[1] +
			(pnext->origin[2] - rns.view.v_origin[2])*forward[2];

		if(scale < 20)
			scale = 1;
		else
			scale = 1 + scale * 0.004;

		Uint32 colorindex = pnext->color;
		if(colorindex > 255)
			colorindex = 0;

		color24_t* pcolor = &m_pColorPalette[colorindex];
		Vector color = Vector(static_cast<Float>(pcolor->r)/255.0f, static_cast<Float>(pcolor->g)/255.0f, static_cast<Float>(pcolor->b)/255.0f);

		// Draw a quad
		Vector vpoint = pnext->origin - forward * 0.01 + up * scale * texscaley;
		vpoint = vpoint + right * -1 * scale * texscalex;
		pDraw->Color4f(color.x, color.y, color.z, 1.0);
		pDraw->TexCoord2f(0.0, 0.0);
		pDraw->Vertex3fv(vpoint);

		vpoint = pnext->origin - forward * 0.01 + up * scale * texscaley;
		vpoint = vpoint + right * scale * texscalex;
		pDraw->Color4f(color.x, color.y, color.z, 1.0);
		pDraw->TexCoord2f(1.0, 0.0);
		pDraw->Vertex3fv(vpoint);

		vpoint = pnext->origin - forward * 0.01 + up * -1 * scale * texscaley;
		vpoint = vpoint + right * scale * texscalex;
		pDraw->Color4f(color.x, color.y, color.z, 1.0);
		pDraw->TexCoord2f(1.0, 1.0);
		pDraw->Vertex3fv(vpoint);

		vpoint = pnext->origin - forward * 0.01 + up * -1 * scale * texscaley;
		vpoint = vpoint + right * -1 * scale * texscalex;
		pDraw->Color4f(color.x, color.y, color.z, 1.0);
		pDraw->TexCoord2f(0.0, 1.0);
		pDraw->Vertex3fv(vpoint);
		nbVertexes += 4;

		if(i < (m_nbSortedParticles-1) && (nbVertexes+4) >= CBasicDraw::BASICDRAW_VERTEX_CACHE_SIZE)
		{
			pDraw->End();
			pDraw->Begin(CBasicDraw::DRAW_QUADS);
			nbVertexes = 0;
		}
	}

	pDraw->End();

	bool result = true;
	if(rns.fog.settings.active)
		result = pDraw->DisableFog();

	pDraw->Disable();

	glDisable(GL_BLEND);

	// Clear any binds
	R_ClearBinds();

	return result;
}