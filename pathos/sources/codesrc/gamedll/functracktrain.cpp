/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "functracktrain.h"

// Train starting pitch
const Int32 CFuncTrackTrain::START_PITCH = 60;
// Train starting pitch
const Int32 CFuncTrackTrain::START_MAX_PITCH = 200;
// Train starting pitch
const Int32 CFuncTrackTrain::TRAIN_MAX_SPEED = 1000;
// Default speed
const Float CFuncTrackTrain::DEFAULT_SPEED = 100;
// Number of train default move sounds
const Uint32 CFuncTrackTrain::NB_DEFAULT_MOVE_SOUNDS = 7;
// Default train move sounds
const Char* CFuncTrackTrain::DEFAULT_MOVE_SOUNDS[NB_DEFAULT_MOVE_SOUNDS] = 
{
	"plats/ttrain1.wav",
	"plats/ttrain2.wav",
	"plats/ttrain3.wav",
	"plats/ttrain4.wav",
	"plats/ttrain5.wav",
	"plats/ttrain6.wav",
	"plats/ttrain7.wav"
};
// Default train stop sound
const Char CFuncTrackTrain::DEFAULT_STOP_SOUND[] = "plats/ttrain_brake1.wav";
// Start train stop sound
const Char CFuncTrackTrain::DEFAULT_START_SOUND[] = "plats/ttrain_start1.wav";

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_tracktrain, CFuncTrackTrain);

//=============================================
// @brief
//
//=============================================
CFuncTrackTrain::CFuncTrackTrain( edict_t* pedict ):
	CBaseEntity(pedict),
	m_pPath(nullptr),
	m_length(0),
	m_height(0),
	m_speed(0),
	m_direction(0),
	m_startSpeed(0),
	m_blockDamage(0),
	m_onTrackChange(false),
	m_soundPlaying(false),
	m_sounds(0),
	m_volume(0),
	m_bank(0),
	m_oldSpeed(0),
	m_lastPitch(0),
	m_moveSound(NO_STRING_VALUE),
	m_stopSound(NO_STRING_VALUE),
	m_startSound(NO_STRING_VALUE),
	m_pathTrackName(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CFuncTrackTrain::~CFuncTrackTrain( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::DeclareSaveFields( void )
{
	// Call base class to do it first
	CBaseEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_pPath, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_length, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_height, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_speed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_direction, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_startSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_blockDamage, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_onTrackChange, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_controlMins, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_controlMaxs, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_sounds, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_volume, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_bank, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_oldSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_moveSound, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_stopSound, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_startSound, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrackTrain, m_pathTrackName, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CFuncTrackTrain::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "wheels"))
	{
		m_length = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "height"))
	{
		m_height = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "startspeed"))
	{
		m_startSpeed = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "sounds"))
	{
		m_sounds = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "volume"))
	{
		m_volume = SDL_atof(kv.value)*0.1;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "dmg"))
	{
		m_blockDamage = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "bank"))
	{
		m_bank = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "movesound_custom"))
	{
		m_moveSound = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "startsound_custom"))
	{
		m_startSound = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "stopsound_custom"))
	{
		m_stopSound = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::Precache( void )
{
	if(m_moveSound == NO_STRING_VALUE && m_sounds > 0)
	{
		Uint32 soundindex = m_sounds - 1;
		if(soundindex >= NB_DEFAULT_MOVE_SOUNDS)
		{
			Util::EntityConPrintf(m_pEdict, "Invalid sound setting '%d'.\n", m_sounds);
			soundindex = 0;
		}

		m_moveSound = gd_engfuncs.pfnAllocString(DEFAULT_MOVE_SOUNDS[soundindex]);
		gd_engfuncs.pfnPrecacheSound(DEFAULT_MOVE_SOUNDS[soundindex]);
	}

	if(m_stopSound == NO_STRING_VALUE)
	{
		m_stopSound = gd_engfuncs.pfnAllocString(DEFAULT_STOP_SOUND);
		gd_engfuncs.pfnPrecacheSound(DEFAULT_STOP_SOUND);
	}

	if(m_startSound == NO_STRING_VALUE)
	{
		m_startSound = gd_engfuncs.pfnAllocString(DEFAULT_START_SOUND);
		gd_engfuncs.pfnPrecacheSound(DEFAULT_START_SOUND);
	}
}

//=============================================
// @brief
//
//=============================================
bool CFuncTrackTrain::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	if(!m_volume)
		m_volume = 1.0;

	if(!m_pState->speed)
		m_speed = DEFAULT_SPEED;
	else
		m_speed = m_pState->speed;

	m_pState->speed = 0;
	m_pState->velocity.Clear();
	m_pState->avelocity.Clear();
	m_pState->impulse = m_speed;

	m_direction = 1;

	if(m_pFields->target == NO_STRING_VALUE)
		Util::WarnEmptyEntity(m_pEdict);

	if(HasSpawnFlag(FL_NOT_SOLID))
		m_pState->solid = SOLID_NOT;
	else
		m_pState->solid = SOLID_BSP;

	m_pState->movetype = MOVETYPE_PUSH;

	if(!SetModel(m_pFields->modelname))
		return false;

	m_controlMins = m_pState->mins;
	m_controlMaxs = m_pState->maxs;
	m_controlMaxs.z += 64;

	SetNextThink(m_pState->ltime + 0.1, false);
	SetThink(&CFuncTrackTrain::Find);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(useMode != USE_SET)
	{
		if(!ShouldToggle(useMode, (m_pState->speed) ? true : false))
			return;

		if(!m_pState->speed)
		{
			m_pState->speed = m_speed * m_direction;
			Next();
		}
		else
		{
			m_pState->speed = 0;
			m_pState->velocity.Clear();
			m_pState->avelocity.Clear();

			StopSound();
			SetThink(nullptr);
		}
	}
	else
	{
		Float delta = value;
		delta = ((Int32)(m_pState->speed*4)/(Int32)m_speed)*0.25 + 0.25*delta;
		delta = clamp(delta, -1.0, 1.0);

		// Cap at zero if we can only go forward
		if(HasSpawnFlag(FL_FORWARD_ONLY) && delta < 0)
			delta = 0;

		m_pState->speed = m_speed*delta;
		Next();
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::CallBlocked( CBaseEntity* pBlocker )
{
	if((pBlocker->GetFlags() & FL_ONGROUND) && pBlocker->GetGroundEntity() == this)
	{
		// Nudge NPCs
		if(pBlocker->IsNPC() && !pBlocker->IsPlayer())
			pBlocker->GroundEntityNudge();

		Float deltaspeed = SDL_fabs(m_pState->speed);
		if(deltaspeed > 50)
			deltaspeed = 50;
		
		Vector blockerVelocity = pBlocker->GetVelocity();
		if(!blockerVelocity.z)
		{
			blockerVelocity.z += deltaspeed;
			pBlocker->SetVelocity(blockerVelocity);
			return;
		}
	}
	else
	{
		Vector blockerVelocity;
		blockerVelocity = (pBlocker->GetOrigin()-m_pState->origin);
		blockerVelocity = blockerVelocity.Normalize() * m_blockDamage;

		pBlocker->SetVelocity(blockerVelocity);
	}

	if(m_blockDamage > 0)
		pBlocker->TakeDamage(this, pBlocker, m_blockDamage, DMG_CRUSH);
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::OnOverrideEntity( void )
{
	// Restore the path_track entity
	SetNextThink(m_pState->ltime + 0.1, false);
	SetThink(&CFuncTrackTrain::RestorePath);
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::Next( void )
{
	if(m_onTrackChange)
		return;

	if(!m_pState->speed)
	{
		Util::EntityConDPrintf(m_pEdict, "Speed is zero.\n");
		StopSound();
		return;
	}

	if(!m_pPath)
	{
		Util::EntityConDPrintf(m_pEdict, "Train lost path.\n");
		StopSound();
		return;
	}

	// Update sound
	UpdateSound();

	Vector nextPosition = m_pState->origin;
	
	// Find the next position
	nextPosition.z -= m_height;
	CPathTrack* pNext = m_pPath->GetLookAhead(nextPosition, m_pState->speed*0.1, true);
	nextPosition.z += m_height;

	m_pState->velocity = (nextPosition - m_pState->origin)*10;
	Vector nextFront = m_pState->origin;

	// Find the next one
	nextFront.z -= m_height;
	if(m_length > 0)
		m_pPath->GetLookAhead(nextFront, m_length, false);
	else
		m_pPath->GetLookAhead(nextFront, 100, false);
	nextFront.z += m_height;

	Vector delta = nextFront-m_pState->origin;
	Vector angles = Math::VectorToAngles(delta.Normalize());

	// Correct the angles
	angles.y += 180;

	// Fix the angles
	Common::NormalizeAngles(angles);
	Common::NormalizeAngles(m_pState->angles);

	if(!pNext || (!delta.x && !delta.y))
		angles = m_pState->angles;

	Float vy, vx;
	if(!HasSpawnFlag(FL_NO_PITCH))
		vx = Util::AngleDistance(angles.x, m_pState->angles.x);
	else
		vx = 0;

	vy = Util::AngleDistance(angles.y, m_pState->angles.y);

	m_pState->avelocity.x = vx * 10;
	m_pState->avelocity.y = vy * 10;

	if(m_bank != 0)
	{
		if(m_pState->avelocity.y < -5)
			m_pState->avelocity.z = Util::AngleDistance(Util::ApproachAngle(-m_bank, m_pState->angles.z, m_bank*2), m_pState->angles.z);
		else if(m_pState->avelocity.y > 5)
			m_pState->avelocity.z = Util::AngleDistance(Util::ApproachAngle(m_bank, m_pState->angles.z, m_bank*2), m_pState->angles.z);
		else
			m_pState->avelocity.z = Util::AngleDistance(Util::ApproachAngle(0, m_pState->angles.z, m_bank*4), m_pState->angles.z) * 4;
	}

	if(pNext)
	{
		if(pNext != m_pPath)
		{
			CPathTrack* pFire = (m_pState->speed >= 0) ? pNext : m_pPath;

			// Set current path
			SetPathTrack(pNext);

			// Fire the track's fire target
			if(pFire->HasMessage())
			{
				const Char* pstrTrackFireTarget = pFire->GetMessage();
				Util::FireTargets(pstrTrackFireTarget, this, this, USE_TOGGLE, 0);
				if(pFire->HasSpawnFlag(CPathTrack::FL_FIRE_ONCE))
					pFire->SetMessage(nullptr);
			}

			// Disable control if set
			if(pFire->HasSpawnFlag(CPathTrack::FL_DISABLE_TRAIN))
				m_pState->spawnflags |= FL_NO_CONTROL;

			// Modify speed is not controlled by player
			if(HasSpawnFlag(FL_NO_CONTROL))
			{
				if(pFire->GetSpeed() > 0)
				{
					m_pState->speed = pFire->GetSpeed();
					Util::EntityConDPrintf(m_pEdict, "Tracktrain speed set to %.2f.\n", m_pState->speed);
				}
			}
		}

		SetThink(&CFuncTrackTrain::Next);
		SetNextThink(m_pState->ltime+0.5, true);
	}
	else
	{
		// End of path, so stop
		StopSound();

		m_pState->velocity = (nextPosition-m_pState->origin);
		m_pState->avelocity.Clear();

		Float distance = m_pState->velocity.Length();
		m_oldSpeed = m_pState->speed;
		m_pState->speed = 0;

		if(distance > 0)
		{
			Double time = distance/m_oldSpeed;
			Math::VectorScale(m_pState->velocity, m_oldSpeed/distance, m_pState->velocity);

			SetThink(&CFuncTrackTrain::DeadEnd);
			SetNextThink(m_pState->ltime + time, false);
		}
		else
		{
			// Reached a dead end
			DeadEnd();
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::Find( void )
{
	SetPathTrack(FindPathTrack(gd_engfuncs.pfnGetString(m_pFields->target)));
	if(!m_pPath)
		return;

	Vector nextPosition = m_pPath->GetOrigin();
	nextPosition.z += m_height;

	Vector lookPosition = m_pPath->GetOrigin();
	
	m_pPath->GetLookAhead(lookPosition, m_length, false);
	lookPosition.z += m_height;

	Vector vecdir = (lookPosition-nextPosition).Normalize();
	m_pState->angles = Math::VectorToAngles(vecdir);

	// Fix angle
	m_pState->angles.y += 180;

	// Clear pitch if set to
	if(HasSpawnFlag(FL_NO_PITCH))
		m_pState->angles.x = 0;

	// Set origin and link us up
	gd_engfuncs.pfnSetOrigin(m_pEdict, nextPosition);

	SetNextThink(m_pState->ltime + 0.1, false);
	SetThink(&CFuncTrackTrain::Next);
	m_pState->speed = m_startSpeed;

	UpdateSound();
}

//=============================================
// @brief
//
//=============================================
CPathTrack* CFuncTrackTrain::FindPathTrack( const Char* pstrPathTrackName )
{
	// Find the path_track with the same name
	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, pstrPathTrackName);
		if(!pedict)
			break;
		
		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(pEntity->IsPathTrackEntity())
			return reinterpret_cast<CPathTrack*>(pEntity);
	}
	
	Util::EntityConPrintf(m_pEdict, "Couldn't find path_track with name '%s' after level transition.\n", pstrPathTrackName);
	return nullptr;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::RestorePath( void )
{
	if(m_onTrackChange)
		return;

	// Watch for errors
	if(m_pathTrackName != NO_STRING_VALUE)
	{
		// Find the path_track with the same name
		SetPathTrack(FindPathTrack(gd_engfuncs.pfnGetString(m_pathTrackName)));
		if(m_pPath)
		{
			// Set movement if speed is non-zero
			if(m_pState->speed != 0)
			{
				SetNextThink(m_pState->ltime + 0.1, false);
				SetThink(&CFuncTrackTrain::Next);
			}
		}
	}
	
	// Restore using the old method(like GoldSrc)
	if(!m_pPath)
	{
		// Distance we will search for path_tracks
		const Float searchDistance = 1024;

		Float nearestDistance = searchDistance;
		CPathTrack* pNearest = nullptr;

		Vector mins, maxs;
		for(Uint32 i = 0; i < 3; i++)
		{
			mins[i] = m_pState->origin[i] - searchDistance;
			maxs[i] = m_pState->origin[i] + searchDistance;
		}

		edict_t* pedict = nullptr;
		while(true)
		{
			pedict = Util::FindEntityInBBox(pedict, mins, maxs);
			if(!pedict)
				break;

			CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
			if(!pEntity->IsPathTrackEntity())
				continue;

			Float distance = (m_pState->origin - pEntity->GetOrigin()).Length();
			if(distance < nearestDistance)
			{
				nearestDistance = distance;
				pNearest = reinterpret_cast<CPathTrack*>(pEntity);
			}
		}

		if(!pNearest)
		{
			Util::EntityConPrintf(m_pEdict, "Couldn't find any path_track entities.\n");
			DeadEnd();
			return;
		}

		CPathTrack* pTrack = pNearest->GetNext();
		if(pTrack && (m_pState->origin - pTrack->GetOrigin()).Length() < (m_pState->origin - pNearest->GetOrigin()).Length())
			pNearest = pTrack;

		m_pPath = pNearest;

		if(m_pState->speed)
		{
			SetNextThink(m_pState->ltime + 0.1, false);
			SetThink(&CFuncTrackTrain::Next);
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::DeadEnd( void )
{
	CPathTrack* pNext = nullptr;
	CPathTrack* pTrack = m_pPath;
	if(pTrack)
	{
		if(m_oldSpeed < 0)
		{
			do
			{
				pNext = pTrack->GetValidPath(pTrack->GetPrevious(), true);
				if(pNext)
					pTrack = pNext;

			} while(pNext);
		}
		else
		{
			do
			{
				pNext = pTrack->GetValidPath(pTrack->GetNext(), true);
				if(pNext)
					pTrack = pNext;

			} while(pNext);
		}
	}

	m_pState->velocity.Clear();
	m_pState->avelocity.Clear();

	CString printmsg;
	printmsg << "Reached dead end ";

	if(pTrack)
	{
		printmsg << "at " << pTrack->GetTargetName();

		if(pTrack->HasNetname())
		{
			const Char* pstrPathTrackTarget = pTrack->GetNetname();
			Util::FireTargets(pstrPathTrackTarget, this, this, USE_TOGGLE, 0);
		}
	}

	printmsg << "\n";
	Util::EntityConDPrintf(m_pEdict, printmsg.c_str());
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::SetNextThink( Double thinkTime, bool alwaysThink )
{
	if(alwaysThink)
		m_pState->flags |= FL_ALWAYSTHINK;
	else
		m_pState->flags &= ~FL_ALWAYSTHINK;

	m_pState->nextthink = thinkTime;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::SetTrack( CPathTrack* pTrack )
{
	m_pPath = pTrack->GetNearest(m_pState->origin);
	m_pFields->netname = gd_engfuncs.pfnAllocString(m_pPath->GetTargetName());
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::SetControls( CBaseEntity* pControls )
{
	Vector offset = pControls->GetOrigin() - m_pState->prevorigin;

	m_controlMins = pControls->GetMins() + offset;
	m_controlMaxs = pControls->GetMaxs() + offset;
}

//=============================================
// @brief
//
//=============================================
bool CFuncTrackTrain::IsOnControls( CBaseEntity* pEntity )
{
	if(HasSpawnFlag(FL_NO_CONTROL))
		return false;

	// Get offset
	Vector localPosition = pEntity->GetOrigin() - m_pState->origin;

	// Rotate by angles
	if(!m_pState->angles.IsZero())
		Math::RotateToEntitySpace(m_pState->angles, localPosition);

	if(Math::PointInMinsMaxs(localPosition, m_controlMins, m_controlMaxs))
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::StopSound( void )
{
	if(m_moveSound != NO_STRING_VALUE)
	{
		const Char* pstrMoveSound = gd_engfuncs.pfnGetString(m_moveSound);
		gd_engfuncs.pfnPlayEntitySound(m_pEdict->entindex, pstrMoveSound, SND_FL_STOP, SND_CHAN_BODY, 0, 0, 0, 0, NO_CLIENT_INDEX);
	}

	if(m_stopSound != NO_STRING_VALUE)
		Util::EmitEntitySound(this, m_stopSound, SND_CHAN_ITEM, m_volume);

	m_soundPlaying = false;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::UpdateSound( void )
{
	if(m_moveSound == NO_STRING_VALUE)
		return;

	// Calculate pitch
	Int32 pitch = START_PITCH + (SDL_fabs(m_pState->speed) *
		(MAX_PITCH - START_PITCH)/TRAIN_MAX_SPEED);

	if(!m_soundPlaying)
	{
		if(m_startSound != NO_STRING_VALUE)
			Util::EmitEntitySound(this, m_startSound, SND_CHAN_ITEM, m_volume);

		if(m_moveSound != NO_STRING_VALUE)
			Util::EmitEntitySound(this, m_moveSound, SND_CHAN_BODY, m_volume, ATTN_NORM, pitch);

		m_soundPlaying = true;
	}
	else if(m_moveSound != NO_STRING_VALUE && pitch != m_lastPitch)
	{
		// Update pitch
		const Char* pstrMoveSound = gd_engfuncs.pfnGetString(m_moveSound);
		gd_engfuncs.pfnPlayEntitySound(m_pEdict->entindex, pstrMoveSound, SND_FL_CHANGE_PITCH, SND_CHAN_BODY, m_volume, ATTN_NORM, pitch, 0, NO_CLIENT_INDEX);
	}

	// Set pitch
	m_lastPitch = pitch;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::SetPathTrack( CPathTrack* pPathTrack )
{
	if(!pPathTrack)
	{
		m_pPath = nullptr;
		m_pathTrackName = NO_STRING_VALUE;
	}
	else
	{
		m_pPath = pPathTrack;
		m_pathTrackName = gd_engfuncs.pfnAllocString(pPathTrack->GetTargetName());
	}
}

//=============================================
// @brief
//
//=============================================
CPathTrack* CFuncTrackTrain::GetPath( void )
{
	return m_pPath;
}

//=============================================
// @brief
//
//=============================================
Float CFuncTrackTrain::GetLength( void ) const
{
	return m_length;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrackTrain::SetIsOnTrackChange( bool isOnTrackChange )
{
	m_onTrackChange = isOnTrackChange;
}