/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "beam.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(beam, CBeam);

//=============================================
// @brief
//
//=============================================
CBeam::CBeam( edict_t* pedict ):
	CBaseEntity(pedict),
	m_lastTime(0),
	m_maxFrame(0),
	m_beamDamage(0),
	m_dmgTime(0),
	m_attachment1Index(NO_ATTACHMENT_INDEX),
	m_attachment2Index(NO_ATTACHMENT_INDEX)
{
}

//=============================================
// @brief
//
//=============================================
CBeam::~CBeam( void )
{
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CBeam::Spawn( void )
{
	// Check for model
	if(m_pFields->modelname == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CBaseEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname, false))
		return false;

	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->rendertype = RT_BEAM;

	// Set base values
	m_pState->iuser3 = NO_ENTITY_INDEX;
	m_pState->iuser4 = NO_ENTITY_INDEX;

	m_pState->iuser5 = NO_ATTACHMENT_INDEX;
	m_pState->iuser6 = NO_ATTACHMENT_INDEX;

	return true;
}

//=============================================
// @brief Called after save-restoring an entity
//
//=============================================
bool CBeam::Restore( void )
{
	if(!CBaseEntity::Restore())
		return false;

	// Re-set the user values after restore
	m_pState->vuser1 = m_endPosition;

	if(m_startEntity)
		m_pState->iuser3 = m_startEntity->GetEntityIndex();
	else
		m_pState->iuser3 = NO_ENTITY_INDEX;

	if(m_endEntity)
		m_pState->iuser4 = m_endEntity->GetEntityIndex();
	else
		m_pState->iuser4 = NO_ENTITY_INDEX;

	if(m_startEntity)
	{
		m_pState->aiment = m_startEntity->GetEntityIndex();
		m_pState->movetype = MOVETYPE_FOLLOW;
	}

	// Always reset these
	m_pState->iuser5 = m_attachment1Index;
	m_pState->iuser6 = m_attachment2Index;

	return true;
}

//=============================================
// @brief Performs precache functions
//
//=============================================
void CBeam::Precache( void )
{
	if(m_pFields->modelname != NO_STRING_VALUE)
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_pFields->modelname));
		if(modelindex != NO_PRECACHE)
			m_maxFrame = gd_engfuncs.pfnGetModelFrameCount(modelindex) - 1;
	}

	CBaseEntity::Precache();
}

//=============================================
// @brief Calls for classes and their children
//
//=============================================
void CBeam::DeclareSaveFields( void )
{
	CBaseEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CBeam, m_lastTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CBeam, m_beamDamage, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CBeam, m_dmgTime, EFIELD_DOUBLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CBeam, m_startEntity, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CBeam, m_endEntity, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CBeam, m_endPosition, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD_FLAGS(CBeam, m_attachment1Index, EFIELD_INT32, EFIELD_SAVE_ALWAYS));
	DeclareSaveField(DEFINE_DATA_FIELD_FLAGS(CBeam, m_attachment2Index, EFIELD_INT32, EFIELD_SAVE_ALWAYS));
}

//=============================================
// @brief Returns entity's flags
//
//=============================================
Int32 CBeam::GetEntityFlags( void )
{
	Int32 flags = CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION;
	if(HasSpawnFlag(FL_TEMPORARY))
		flags |= FL_ENTITY_TRANSITION;

	return flags;
}

//=============================================
// @brief Returns the entity's center
//
//=============================================
Vector CBeam::GetCenter( void )
{
	return (GetBeamStartPosition() + GetBeamEndPosition()) * 0.5;
}

//=============================================
// @brief Called when touched
//
//=============================================
void CBeam::TriggerTouch( CBaseEntity* pOther )
{
	if(!(pOther->GetFlags() & (FL_CLIENT|FL_NPC)))
		return;

	if(m_startEntity)
		m_startEntity->CallUse(pOther, this, USE_TOGGLE, 0);
}

//=============================================
// @brief Relinks the beam to the world
//
//=============================================
void CBeam::RelinkBeam( void )
{
	const Vector& startPosition = GetBeamStartPosition();
	const Vector& endPosition = GetBeamEndPosition();

	for(Uint32 i = 0; i < 3; i++)
	{
		m_pState->mins[i] = _max(startPosition[i], endPosition[i]) - 1;
		m_pState->maxs[i] = _min(startPosition[i], endPosition[i]) + 1;
	}

	m_pState->mins = m_pState->mins - m_pState->origin;
	m_pState->maxs = m_pState->maxs - m_pState->origin;

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, m_pState->mins, m_pState->maxs);
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
}

//=============================================
// @brief Creates random sparks
//
//=============================================
void CBeam::BeamSparks( const Vector& start, const Vector& end )
{
	if(!HasSpawnFlag(FL_START_SPARKS|FL_END_SPARKS))
		return;

	if(Common::RandomFloat(0, 100) > 70)
	{
		if(HasSpawnFlag(FL_START_SPARKS))
			Util::CreateSparks(start);

		if(HasSpawnFlag(FL_END_SPARKS))
			Util::CreateSparks(end);
	}
}

//=============================================
// @brief Picks a random entity with the targetname
//
//=============================================
CBaseEntity* CBeam::GetRandomTargetName( const Char* pstrName )
{
	Uint32 total = 0;

	edict_t* pEntity = nullptr;
	edict_t* pNewEntity = nullptr;

	while(true)
	{
		pNewEntity = Util::FindEntityByTargetName(pNewEntity, pstrName);
		if(!pNewEntity)
			break;

		if(Util::IsNullEntity(pNewEntity))
			continue;

		total++;
		if(Common::RandomLong(0, (total-1)) < 1)
			pEntity = pNewEntity;
	}

	if(!pEntity)
		return nullptr;
	else
		return CBaseEntity::GetClass(pEntity);
}

//=============================================
// @brief Deals beam damage
//
//=============================================
void CBeam::BeamDamage( trace_t& tr )
{
	RelinkBeam();

	if(tr.noHit() || tr.hitentity == NO_ENTITY_INDEX)
		return;

	CBaseEntity* pEntity = Util::GetEntityFromTrace(tr);
	if(!pEntity)
		return;

	gMultiDamage.Clear();

	Vector attackDirection = (tr.endpos - m_pState->origin).Normalize();
	Float damage = m_beamDamage * (g_pGameVars->time - m_dmgTime);
	pEntity->TraceAttack(this, damage, attackDirection, tr, DMG_ELECTRICITY);
	gMultiDamage.ApplyDamage(this, this);

	if(HasSpawnFlag(FL_DECAL_END))
	{
		if(pEntity->IsBrushModel())
		{
			Util::CreateGenericDecal(tr.endpos, &tr.plane.normal, "shot", FL_DECAL_NONE);
			gd_engfuncs.pfnAddSavedDecal(tr.endpos, tr.plane.normal, tr.hitentity, "shot", FL_DECAL_NONE);
		}
		else if(pEntity->IsNPC())
		{
			Util::CreateVBMDecal(tr.endpos, tr.plane.normal, "shot_human", pEntity->GetEdict(), FL_DECAL_NORMAL_PERMISSIVE);
		}
	}

	m_dmgTime = g_pGameVars->time;
}

//=============================================
// @brief Initializes a beam
//
//=============================================
bool CBeam::BeamInit( const Char* pstrSpriteName, Float width )
{
	m_pState->rendertype = RT_BEAM;

	SetBeamColor(255, 255, 255);
	SetBeamBrightness(255);
	SetBeamAmplitude(0);
	SetBeamFrame(0);
	SetBeamScrollRate(0);
	SetBeamNoiseSpeed(10.0);
	SetBeamWidth(width);

	if(!gd_engfuncs.pfnSetModel(m_pEdict, pstrSpriteName, false))
		return false;

	m_pState->iuser4 = NO_ENTITY_INDEX;
	m_pState->iuser5 = NO_ENTITY_INDEX;
	m_pState->iuser2 = 0;

	return true;
}

//=============================================
// @brief Initializes a points beam
//
//=============================================
void CBeam::BeamInitPoints( const Vector& start, const Vector& end )
{
	SetBeamType(BEAM_MSG_BEAMPOINTS);
	SetBeamStartPosition(start);
	SetBeamEndPosition(end);
	SetBeamStartEntityAttachment(NO_ATTACHMENT_INDEX);
	SetBeamEndEntityAttachment(NO_ATTACHMENT_INDEX);
	RelinkBeam();
}

//=============================================
// @brief Initializes a beam connecting two entities
//
//=============================================
void CBeam::BeamInitEntities( CBaseEntity* pStartEntity, CBaseEntity* pEndEntity )
{
	SetBeamType(BEAM_MSG_BEAMENTS);
	SetBeamStartEntity(pStartEntity);
	SetBeamEndEntity(pEndEntity);
	SetBeamStartEntityAttachment(NO_ATTACHMENT_INDEX);
	SetBeamEndEntityAttachment(NO_ATTACHMENT_INDEX);
	RelinkBeam();
}

//=============================================
// @brief Initializes a beam connecting an entity and a point
//
//=============================================
void CBeam::BeamInitPointEntity( CBaseEntity* pStartEntity, const Vector& end )
{
	SetBeamType(BEAM_MSG_BEAMENTPOINT);
	SetBeamStartEntity(pStartEntity);
	SetBeamEndPosition(end);
	SetBeamStartEntityAttachment(NO_ATTACHMENT_INDEX);
	SetBeamEndEntityAttachment(NO_ATTACHMENT_INDEX);
	RelinkBeam();
}

//=============================================
// @brief Creates a beam object
//
//=============================================
CBeam* CBeam::CreateBeam( const Char* pstrSpriteName, Float width )
{
	CBeam* pBeam = reinterpret_cast<CBeam*>(CBaseEntity::CreateEntity("beam", nullptr));
	if(!pBeam)
		return nullptr;

	if(!pBeam->BeamInit(pstrSpriteName, width))
	{
		Util::RemoveEntity(pBeam);
		return nullptr;
	}

	return pBeam;
}