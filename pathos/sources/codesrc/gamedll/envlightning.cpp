/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envlightning.h"

// Number of random loops
const Uint32 CEnvLightning::NUM_RANDOM_LOOPS = 10;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_lightning, CEnvLightning);

//=============================================
// @brief
//
//=============================================
CEnvLightning::CEnvLightning( edict_t* pedict ):
	CBeam(pedict),
	m_isActive(false),
	m_startEntityName(NO_STRING_VALUE),
	m_endEntityName(NO_STRING_VALUE),
	m_life(0),
	m_beamWidth(0),
	m_noiseAmplitude(0),
	m_brightness(0),
	m_speed(0),
	m_noiseSpeed(0),
	m_strikeMaxDelay(0),
	m_spriteModelIndex(0),
	m_spriteModelName(NO_STRING_VALUE),
	m_startFrame(0),
	m_radius(0),
	m_isTeslaBeam(false)
{
}

//=============================================
// @brief
//
//=============================================
CEnvLightning::~CEnvLightning( void )
{
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CEnvLightning::Spawn( void )
{
	// Check for model
	if(m_spriteModelName == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CBaseEntity::Spawn())
		return false;

	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->rendertype = RT_BEAM;

	m_dmgTime = g_pGameVars->time;

	SetBeamStartEntityAttachment(NO_ATTACHMENT_INDEX);
	SetBeamEndEntityAttachment(NO_ATTACHMENT_INDEX);

	if(!m_noiseSpeed)
		m_noiseSpeed = 10.0;

	if(IsServerSide())
	{
		SetThink(nullptr);

		if(m_beamDamage > 0)
		{
			SetThink(&CEnvLightning::DamageThink);
			m_pState->nextthink = g_pGameVars->time + 0.1;
		}
		
		if(m_pFields->targetname != NO_STRING_VALUE)
		{
			if(!HasSpawnFlag(FL_START_ON))
			{
				m_pState->effects |= EF_NODRAW;
				m_isActive = false;
				m_pState->nextthink = 0;
			}
			else
			{
				m_isActive = true;
			}

			SetUse(&CEnvLightning::ToggleUse);
		}

		if(m_isTeslaBeam)
			m_pState->iuser2 |= FL_BEAM_TESLA;
	}
	else
	{
		m_isActive = false;

		if(m_pFields->targetname != NO_STRING_VALUE)
		{
			SetUse(&CEnvLightning::StrikeUse);
		}

		if(m_pFields->targetname == NO_STRING_VALUE || HasSpawnFlag(FL_START_ON))
		{
			SetThink(&CEnvLightning::StrikeThink);
			m_pState->nextthink = g_pGameVars->time + 1.0;
		}
	}

	if(IsServerSide())
	{
		// Always re-initialize after reload
		m_pState->flags |= FL_INITIALIZE;
	}

	return true;
}

//=============================================
// @brief Performs precache functions
//
//=============================================
void CEnvLightning::Precache( void )
{
	CBeam::Precache();

	m_spriteModelIndex = gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_spriteModelName));
}

//=============================================
// @brief Performs precache functions
//
//=============================================
bool CEnvLightning::Restore( void )
{
	if(!CBeam::Restore())
		return false;

	if(IsServerSide())
	{
		// Always re-initialize after reload
		m_pState->flags |= FL_INITIALIZE;
	}

	return true;
}

//=============================================
// @brief Initializes the entity after map has done loading
//
//=============================================
void CEnvLightning::InitEntity( void )
{
	UpdateBeamVars();
}

//=============================================
// @brief Calls for classes and their children
//
//=============================================
void CEnvLightning::DeclareSaveFields( void )
{
	CBeam::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_startEntityName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_endEntityName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_life, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_beamWidth, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_noiseAmplitude, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_brightness, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_speed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_noiseSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_strikeMaxDelay, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_spriteModelName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_startFrame, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLightning, m_radius, EFIELD_FLOAT));
}

//=============================================
// @brief Manages keyvalues
//
//=============================================
bool CEnvLightning::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "LightningStart"))
	{
		m_startEntityName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "LightningEnd"))
	{
		m_endEntityName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "Attachment1"))
	{
		SetBeamStartEntityAttachment(SDL_atoi(kv.value));
		return true;
	}
	else if(!qstrcmp(kv.keyname, "LightningEnd"))
	{
		m_endEntityName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "Attachment2"))
	{
		SetBeamEndEntityAttachment(SDL_atoi(kv.value));
		return true;
	}
	else if(!qstrcmp(kv.keyname, "life"))
	{
		m_life = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "BoltWidth"))
	{
		m_beamWidth = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "NoiseAmplitude"))
	{
		m_noiseAmplitude = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "NoiseSpeed"))
	{
		m_noiseSpeed = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "TextureScroll"))
	{
		m_speed = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "StrikeTime"))
	{
		m_strikeMaxDelay = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "texture"))
	{
		m_spriteModelName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "framestart"))
	{
		m_pState->frame = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "damage"))
	{
		m_beamDamage = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "Radius"))
	{
		m_radius = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "teslabeam"))
	{
		m_isTeslaBeam = (SDL_atoi(kv.value) == 1) ? true : false;
		return true;
	}
	else 
		return CBeam::KeyValue(kv);
}

//=============================================
// @brief Use function for striking the beam
//
//=============================================
void CEnvLightning::StrikeUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!ShouldToggle(useMode, m_isActive))
		return;

	if(m_isActive)
	{
		m_isActive = false;
		SetThink(nullptr);
	}
	else
	{
		SetThink(&CEnvLightning::StrikeThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}

	if(!HasSpawnFlag(FL_TOGGLE))
		SetUse(nullptr);
}

//=============================================
// @brief Use function for toggling the beam
//
//=============================================
void CEnvLightning::ToggleUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!ShouldToggle(useMode, m_isActive))
		return;

	if(m_isActive)
	{
		m_isActive = false;
		m_pState->effects |= EF_NODRAW;
		m_pState->nextthink = 0;
	}
	else
	{
		m_isActive = true;
		m_pState->effects &= ~EF_NODRAW;
		BeamSparks(GetBeamStartPosition(), GetBeamEndPosition());

		if(m_beamDamage > 0)
		{
			SetThink(&CEnvLightning::DamageThink);
			m_pState->nextthink = g_pGameVars->time;
			m_dmgTime = g_pGameVars->time;
		}
	}
}

//=============================================
// @brief Called when striking
//
//=============================================
void CEnvLightning::StrikeThink( void )
{
	if(m_life)
	{
		if(HasSpawnFlag(FL_RANDOM))
			m_pState->nextthink = g_pGameVars->time + m_life + Common::RandomFloat(0, m_strikeMaxDelay);
		else
			m_pState->nextthink = g_pGameVars->time + m_life + m_strikeMaxDelay;
	}

	m_isActive = true;

	if(m_endEntityName == NO_STRING_VALUE)
	{
		if(m_startEntityName == NO_STRING_VALUE)
		{
			RandomArea();
		}
		else
		{
			const Char* pstrStartEntityName = gd_engfuncs.pfnGetString(m_startEntityName);
			CBaseEntity* pStart = GetRandomTargetName(pstrStartEntityName);
			if(pStart)
				RandomPoint(pStart->GetOrigin());
			else
				Util::EntityConPrintf(m_pEdict, "Could not find start entity '%s'.\n", pstrStartEntityName);
		}
	}
	else
	{
		const Char* pstrStartEntityName = gd_engfuncs.pfnGetString(m_startEntityName);
		CBaseEntity* pStart = GetRandomTargetName(pstrStartEntityName);
		if(!pStart)
		{
			Util::EntityConPrintf(m_pEdict, "Could not find start entity '%s'.\n", pstrStartEntityName);
			return;
		}

		const Char* pstrEndEntityName = gd_engfuncs.pfnGetString(m_endEntityName);
		CBaseEntity* pEnd = GetRandomTargetName(pstrEndEntityName);
		if(!pEnd)
		{
			Util::EntityConPrintf(m_pEdict, "Could not find end entity '%s'.\n", pstrEndEntityName);
			return;
		}

		if(IsPointEntity(pStart) || IsPointEntity(pEnd))
		{
			if(HasSpawnFlag(FL_RING))
			{
				Util::EntityConPrintf(m_pEdict, "Ring entity cannot work with point entities.\n", pstrEndEntityName);
				return;
			}

			if(!IsPointEntity(pEnd))
			{
				CBaseEntity* pTmp = pStart;
				pStart = pEnd;
				pEnd = pTmp;
			}

			if(!IsPointEntity(pStart))
				Util::CreateBeamEntityPoint(pStart, pEnd->GetOrigin(), NO_ATTACHMENT_INDEX, m_spriteModelIndex, 
					m_startFrame, m_pState->framerate, m_life, m_beamWidth, m_noiseAmplitude, m_pState->renderamt, m_speed, 
					m_noiseSpeed, m_pState->rendercolor.x, m_pState->rendercolor.y, m_pState->rendercolor.z,
					(FL_BEAM_VARIABLE_NOISE|FL_BEAM_VARIABLE_DIR));
			else
				Util::CreateBeamPoints(pStart->GetOrigin(), pEnd->GetOrigin(), m_spriteModelIndex,  m_startFrame, 
					m_pState->framerate, m_life, m_beamWidth, m_noiseAmplitude, m_pState->renderamt, m_speed, 
					m_noiseSpeed, m_pState->rendercolor.x, m_pState->rendercolor.y, m_pState->rendercolor.z,
					(FL_BEAM_VARIABLE_NOISE|FL_BEAM_VARIABLE_DIR));
		}
		else
		{
			if(HasSpawnFlag(FL_RING))
				Util::CreateBeamRing(pStart, pEnd, NO_ATTACHMENT_INDEX, NO_ATTACHMENT_INDEX, m_spriteModelIndex, 
					m_startFrame, m_pState->framerate, m_life, m_beamWidth, m_noiseAmplitude, m_pState->renderamt, m_speed, 
					m_noiseSpeed, m_pState->rendercolor.x, m_pState->rendercolor.y, m_pState->rendercolor.z,
					(FL_BEAM_VARIABLE_NOISE|FL_BEAM_VARIABLE_DIR));
			else
				Util::CreateBeamEntities(pStart, pEnd, NO_ATTACHMENT_INDEX, NO_ATTACHMENT_INDEX, m_spriteModelIndex, 
					m_startFrame, m_pState->framerate, m_life, m_beamWidth, m_noiseAmplitude, m_pState->renderamt, m_speed, 
					m_noiseSpeed, m_pState->rendercolor.x, m_pState->rendercolor.y, m_pState->rendercolor.z,
					(FL_BEAM_VARIABLE_NOISE|FL_BEAM_VARIABLE_DIR));
		}

		BeamSparks(pStart->GetOrigin(), pEnd->GetOrigin());

		if(m_beamDamage > 0)
		{
			trace_t tr;
			Util::TraceLine(pStart->GetOrigin(), pEnd->GetOrigin(), false, true, nullptr, tr);
			BeamDamageInstant(tr, m_beamDamage);
		}
	}
}

//=============================================
// @brief Called when doing damage
//
//=============================================
void CEnvLightning::DamageThink( void )
{
	m_pState->nextthink = g_pGameVars->time + 0.1;

	trace_t tr;
	Util::TraceLine(GetBeamStartPosition(), GetBeamEndPosition(), false, true, nullptr, tr);
	BeamDamage(tr);
}

//=============================================
// @brief Random spot finding
//
//=============================================
void CEnvLightning::RandomArea( void )
{
	for(Uint32 i = 0; i < NUM_RANDOM_LOOPS; i++)
	{
		Vector src = m_pState->origin;
		Vector dir1 = Vector(Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1));
		dir1.Normalize();

		Vector endpos = src + dir1*m_radius;

		trace_t tr;
		Util::TraceLine(src, endpos, true, false, m_pEdict, tr);
		if(tr.noHit())
			continue;

		// Remember
		Vector trace1end = tr.endpos;

		Vector dir2;
		do
		{
			dir2 = Vector(Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1));
			dir2.Normalize();
		}
		while(Math::DotProduct(dir1, dir2) > 0);

		endpos = src + dir2*m_radius;

		Util::TraceLine(src, endpos, true, false, m_pEdict, tr);
		if(tr.noHit())
			continue;

		// Remember
		Vector trace2end = tr.endpos;

		// Do not hit too close to home
		if((trace1end-trace2end).Length() < m_radius * 0.1)
			continue;

		Util::TraceLine(trace1end, trace2end, true, false, m_pEdict, tr);
		if(!tr.noHit())
			continue;

		Zap(trace1end, trace2end);
		break;
	}
}

//=============================================
// @brief Random point finding
//
//=============================================
void CEnvLightning::RandomPoint( const Vector& src )
{
	for(Uint32 i = 0; i < NUM_RANDOM_LOOPS; i++)
	{
		Vector dir1 = Vector(Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1));
		dir1.Normalize();

		Vector endpos = src + dir1*m_radius;

		trace_t tr;
		Util::TraceLine(src, endpos, true, false, m_pEdict, tr);
		if(tr.noHit())
			continue;

		// Do not hit too close to home
		if((src-tr.endpos).Length() < m_radius * 0.1)
			continue;

		Zap(src, tr.endpos);
		break;
	}
}

//=============================================
// @brief Zaps between start and end
//
//=============================================
void CEnvLightning::Zap( const Vector& src, const Vector& end )
{
	Util::CreateBeamPoints(src, end, m_spriteModelIndex,  m_startFrame, m_pState->framerate, 
		m_life, m_beamWidth, m_noiseAmplitude, m_pState->renderamt, m_speed, m_noiseSpeed,
		m_pState->rendercolor.x, m_pState->rendercolor.y, m_pState->rendercolor.z,
		(FL_BEAM_VARIABLE_NOISE|FL_BEAM_VARIABLE_DIR));

	BeamSparks(src, end);
}

//=============================================
// @brief Tells if beam is server side 
//
//=============================================
bool CEnvLightning::IsServerSide( void )
{
	if(!m_life && !HasSpawnFlag(FL_RING))
		return true;
	else
		return false;
}

//=============================================
// @brief Updates beam variables
//
//=============================================
void CEnvLightning::UpdateBeamVars( void )
{
	const Char* pstrStartEntityName = gd_engfuncs.pfnGetString(m_startEntityName);
	CBaseEntity* pStart = GetRandomTargetName(pstrStartEntityName);
	if(!pStart)
	{
		Util::EntityConPrintf(m_pEdict, "Could not find start entity '%s'.\n", pstrStartEntityName);
		return;
	}

	const Char* pstrEndEntityName = gd_engfuncs.pfnGetString(m_endEntityName);
	CBaseEntity* pEnd = GetRandomTargetName(pstrEndEntityName);
	if(!pEnd)
	{
		Util::EntityConPrintf(m_pEdict, "Could not find end entity '%s'.\n", pstrEndEntityName);
		return;
	}

	bool isStartPoint = IsPointEntity(pStart);
	bool isEndPoint = IsPointEntity(pEnd);

	SetBeamStartEntity(nullptr);
	SetBeamEndEntity(nullptr);
	
	const Char* pstrSpriteModelName = gd_engfuncs.pfnGetString(m_spriteModelName);
	if(!gd_engfuncs.pfnSetModel(m_pEdict, pstrSpriteModelName, false))
	{
		Util::EntityConPrintf(m_pEdict, "Beam sprite '%s' not found.\n", pstrSpriteModelName);
		Util::RemoveEntity(this);
		return;
	}

	beam_msgtype_t type = BEAM_MSG_BEAMENTS;
	if(isStartPoint || isEndPoint)
	{
		if(!isStartPoint)
		{
			CBaseEntity* pTmp = pStart;
			pStart = pEnd;
			pEnd = pTmp;

			bool swap = isStartPoint;
			isEndPoint = swap;
		}

		if(!isEndPoint)
			type = BEAM_MSG_BEAMENTPOINT;
		else
			type = BEAM_MSG_BEAMPOINTS;
	}

	SetBeamType(type);
	if(type == BEAM_MSG_BEAMPOINTS || type == BEAM_MSG_BEAMENTPOINT)
	{
		SetBeamStartPosition(pStart->GetOrigin());
		if(type == BEAM_MSG_BEAMPOINTS)
			SetBeamEndPosition(pEnd->GetOrigin());
		else
			SetBeamEndEntity(pEnd);
	}
	else
	{
		SetBeamStartEntity(pStart);
		SetBeamEndEntity(pEnd);
	}

	SetBeamWidth(m_beamWidth);
	SetBeamAmplitude(m_noiseAmplitude);
	SetBeamFrame(m_startFrame);
	SetBeamScrollRate(m_speed);
	SetBeamNoiseSpeed(m_noiseSpeed);

	if(HasSpawnFlag(FL_SHADE_START))
		SetBeamFlags(FL_BEAM_SHADEIN);
	else if(HasSpawnFlag(FL_SHADE_END))
		SetBeamFlags(FL_BEAM_SHADEOUT);

	RelinkBeam();
}

//=============================================
// @brief Tells if the entity is a point entity
//
//=============================================
bool CEnvLightning::IsPointEntity( CBaseEntity* pEntity )
{
	if(!pEntity->HasModelName())
		return true;

	// TODO: refactor this
	if(!qstrcmp(pEntity->GetClassName(), "info_target") 
		|| !qstrcmp(pEntity->GetClassName(), "info_landmark") 
		|| !qstrcmp(pEntity->GetClassName(), "path_corner"))
		return true;

	return false;
}