/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "gib.h"
#include "game.h"

// Number of gibs globally
Uint32 CGib::m_numGibs = 0;

// Gib default lifetime
const Float CGib::GIB_DEFAULT_LIFETIME = 25;
// Maximum gibs present ingame
const Uint32 CGib::MAX_ACTIVE_GIBS = 64;
// Max gib velocity
const Float CGib::MAX_GIB_VELOCITY = 1500;
// Maximum decals a gib can spawn
const Uint32 CGib::MAX_NB_DECALS = 4;
// Full Z velocity at full volume
const Float CGib::FULL_VOLUME_VELOCITY = 450.0f;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(gib, CGib);

//=============================================
// @brief
//
//=============================================
CGib::CGib( edict_t* pedict ):
	CBaseEntity(pedict),
	m_bloodColor(BLOOD_NONE),
	m_numBloodDecals(0),
	m_material(0),
	m_lifetime(0)
{
}

//=============================================
// @brief
//
//=============================================
CGib::~CGib( void )
{
}

//=============================================
// @brief
//
//=============================================
void CGib::FreeEntity( void )
{
	if(m_numGibs > 0)
		m_numGibs--;
}

//=============================================
// @brief
//
//=============================================
void CGib::DeclareSaveFields( void )
{
	// Call base class to do it first
	CBaseEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CGib, m_bloodColor, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CGib, m_numBloodDecals, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CGib, m_material, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CGib, m_lifetime, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CGib::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	m_pState->renderamt = 255;
	m_pState->renderfx = RenderFx_None;
	m_pState->rendermode = RENDER_NORMAL;
	m_pState->friction = 0.5;

	m_pState->solid = SOLID_SLIDEBOX;
	m_pState->movetype = MOVETYPE_BOUNCE;

	if(!SetModel(m_pFields->modelname, false))
		return false;

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, ZERO_VECTOR, ZERO_VECTOR);

	SetThink(&CGib::WaitTillLand);
	SetTouch(&CGib::BounceTouch);
	m_pState->nextthink = g_pGameVars->time + 4;

	m_lifetime = GIB_DEFAULT_LIFETIME;
	m_material = MAT_NONE;
	m_numBloodDecals = MAX_NB_DECALS;

	// Increment
	m_numGibs++;

	return true;
}

//=============================================
// @brief
//
//=============================================
Int32 CGib::GetEntityFlags( void )
{
	return (CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION) | FL_ENTITY_DONT_SAVE;
}

//=============================================
// @brief
//
//=============================================
void CGib::BounceTouch( CBaseEntity* pOther )
{
	if(m_pState->flags & FL_ONGROUND)
	{
		m_pState->angles[0] = 0;
		m_pState->angles[2] = 0;

		m_pState->velocity = m_pState->velocity*0.9;

		m_pState->avelocity[0] = 0;
		m_pState->avelocity[2] = 0;
	}
	else
	{
		if(m_numBloodDecals > 0)
		{
			Vector traceStart = m_pState->origin + Vector(0, 0, 8);
			Vector traceEnd = m_pState->origin - Vector(0, 0, 16);

			trace_t tr;
			Util::TraceLine(traceStart, traceEnd, true, false, m_pEdict, tr);
			Util::SpawnBloodDecal(tr, (bloodcolor_t)m_bloodColor, false);

			m_numBloodDecals--;
		}

		if(m_material != MAT_NONE && Common::RandomLong(0, 2) == 0)
		{
			Float zvelocity = SDL_fabs(m_pState->velocity.z);
			Float volume = clamp(zvelocity/FULL_VOLUME_VELOCITY, 0.0, VOL_NORM);

			CString sound = Util::GetDebrisSound((breakmaterials_t)m_material);
			Util::EmitEntitySound(this, sound.c_str(), SND_CHAN_BODY, volume);
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CGib::WaitTillLand( void )
{
	if(m_pState->velocity.IsZero())
	{
		SetThink(&CBaseEntity::FadeBeginThink);
		m_pState->nextthink = g_pGameVars->time + m_lifetime;
	}
	else
	{
		m_pState->nextthink = g_pGameVars->time + 0.5;
	}
}

//=============================================
// @brief
//
//=============================================
void CGib::LimitVelocity( void )
{
	if(m_pState->velocity.Length() > MAX_GIB_VELOCITY)
		m_pState->velocity = m_pState->velocity.Normalize() * MAX_GIB_VELOCITY;
}

//=============================================
// @brief
//
//=============================================
void CGib::SetBloodColor( bloodcolor_t color )
{
	m_bloodColor = color;
}

//=============================================
// @brief
//
//=============================================
void CGib::SetLifeTime( Float lifetime )
{
	m_lifetime = lifetime;
}

//=============================================
// @brief
//
//=============================================
void CGib::SetMaterial( Int32 material )
{
	m_material = material;
}

//=============================================
// @brief
//
//=============================================
void CGib::SpawnRandomGibs( CBaseEntity* pVictim, Uint32 nbGibs, const Vector* pflCenter, Float minvel, Float maxvel )
{
	if(!pVictim)
		return;

	for(Uint32 i = 0; i < nbGibs; i++)
	{
		if(m_numGibs >= MAX_ACTIVE_GIBS)
			return;

		const Vector& absmins = pVictim->GetAbsMins();
		const Vector& size = pVictim->GetSize();
		const Vector& angles = pVictim->GetAngles();

		Vector origin;
		origin[0] = absmins[0] + size[0]*Common::RandomFloat(0, 1);
		origin[1] = absmins[1] + size[1]*Common::RandomFloat(0, 1);
		origin[2] = absmins[2] + size[2]*Common::RandomFloat(0, 1) + 1.0;

		CGib* pGib = reinterpret_cast<CGib*>(CBaseEntity::CreateEntity("gib", origin, angles, nullptr));
		if(!pGib || !pGib->InitGib(HUMAN_GIBS_MODEL_FILENAME))
			return;

		Uint32 randomGibsBegin = GIB_SKULL+1;
		pGib->SetBody(Common::RandomLong(randomGibsBegin, CGib::NB_GIBS-randomGibsBegin));

		Vector velocity;
		if(pflCenter)
		{
			Vector direction = (origin - (*pflCenter));
			velocity = direction.Normalize();
		}
		else
		{
			Math::VectorScale(gMultiDamage.GetAttackDirection(), -1, velocity);
		}

		for(Uint32 j = 0; j < 3; j++)
			velocity[j] += Common::RandomFloat(-0.25, 0.25);

		Math::VectorScale(velocity, Common::RandomFloat(minvel, maxvel), velocity);

		Vector avelocity;
		avelocity[0] = Common::RandomFloat(100, 200);
		avelocity[1] = Common::RandomFloat(100, 300);

		if(pVictim->GetHealth() > -50)
			Math::VectorScale(velocity, 0.7, velocity);
		else if(pVictim->GetHealth() > -200)
			Math::VectorScale(velocity, 2.0, velocity);
		else
			Math::VectorScale(velocity, 4.0, velocity);

		pGib->SetVelocity(velocity);
		pGib->SetAngularVelocity(avelocity);
		pGib->SetBloodColor(pVictim->GetBloodColor());
		pGib->LimitVelocity();

		pVictim->OnGibSpawnCallback(pGib);
	}
}

//=============================================
// @brief
//
//=============================================
void CGib::SpawnHeadGib( CBaseEntity* pVictim, const Vector* pflCenter, Float minvel, Float maxvel )
{
	if(m_numGibs >= MAX_ACTIVE_GIBS)
		return;

	if(!pVictim)
		return;

	// Create gib at eye position
	Vector origin = pVictim->GetEyePosition();
	Vector angles = pVictim->GetAngles();

	CGib* pGib = reinterpret_cast<CGib*>(CBaseEntity::CreateEntity("gib", origin, angles, nullptr));
	if(!pGib || !pGib->InitGib(HUMAN_GIBS_MODEL_FILENAME))
		return;

	Vector velocity;
	if(pflCenter)
	{
		Vector direction = (origin - (*pflCenter));
		direction.Normalize();

		velocity = direction * Common::RandomFloat(minvel, maxvel);
	}
	else
	{
		velocity = Vector(Common::RandomFloat(-minvel, maxvel),
			Common::RandomFloat(-minvel, maxvel),
			Common::RandomFloat(-minvel, maxvel));
	}

	Vector avelocity(Common::RandomFloat(100, 200),
		Common::RandomFloat(100, 300), 0);

	pGib->SetBloodColor(pVictim->GetBloodColor());

	if(pVictim->GetHealth() > -50)
		Math::VectorScale(velocity, 0.7, velocity);
	else if(pVictim->GetHealth() > -200)
		Math::VectorScale(velocity, 2.0, velocity);
	else
		Math::VectorScale(velocity, 4.0, velocity);

	pGib->SetAngularVelocity(avelocity);
	pGib->SetVelocity(velocity);
	pGib->SetBody(CGib::GIB_SKULL);

	pGib->LimitVelocity();

	pVictim->OnGibSpawnCallback(pGib);
}

//=============================================
// @brief
//
//=============================================
void CGib::SpawnChestGib( CBaseEntity* pVictim, const Vector* pflCenter, Float minvel, Float maxvel )
{
	if(m_numGibs >= MAX_ACTIVE_GIBS)
		return;

	if(!pVictim)
		return;

	// Create gib at eye position
	Vector origin = pVictim->GetEyePosition();
	Vector angles = pVictim->GetAngles();

	CGib* pGib = reinterpret_cast<CGib*>(CBaseEntity::CreateEntity("gib", origin, angles, nullptr));
	if(!pGib || !pGib->InitGib(HUMAN_GIBS_MODEL_FILENAME))
		return;

	Vector velocity;
	if(pflCenter)
	{
		Vector direction = (origin - (*pflCenter));
		velocity = direction.Normalize();
	}
	else
	{
		Math::VectorScale(gMultiDamage.GetAttackDirection(), -1, velocity);
	}

	Vector avelocity(Common::RandomFloat(50, 100),
		Common::RandomFloat(50, 150), 0);

	pGib->SetBloodColor(pVictim->GetBloodColor());

	if(pVictim->GetHealth() > -50)
		Math::VectorScale(velocity, 0.7, velocity);
	else if(pVictim->GetHealth() > -200)
		Math::VectorScale(velocity, 2.0, velocity);
	else
		Math::VectorScale(velocity, 4.0, velocity);

	pGib->SetAngularVelocity(avelocity);
	pGib->SetVelocity(velocity);
	pGib->SetBody(CGib::GIB_RIBCAGE);

	pGib->LimitVelocity();

	pVictim->OnGibSpawnCallback(pGib);
}

//=============================================
// @brief
//
//=============================================
bool CGib::InitGib( const Char* pstrModelname )
{
	m_pFields->modelname = gd_engfuncs.pfnAllocString(pstrModelname);

	if(!Spawn())
		return false;
	else
		return true;
}
