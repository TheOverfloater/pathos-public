/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envladder.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_ladder, CEnvLadder);

//=============================================
// @brief
//
//=============================================
CEnvLadder::CEnvLadder( edict_t* pedict ):
	CAnimatingEntity(pedict),
	m_isActive(false),
	m_numSegments(0),
	m_pPlayer(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CEnvLadder::~CEnvLadder( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvLadder::DeclareSaveFields( void )
{
	// Call base class to do it first
	CAnimatingEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLadder, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLadder, m_numSegments, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLadder, m_pPlayer, EFIELD_ENTPOINTER));
}

//=============================================
// @brief
//
//=============================================
bool CEnvLadder::Spawn( void )
{
	if(m_pFields->target == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	// Takes care of precache and setting the model
	if(!CAnimatingEntity::Spawn())
		return false;

	m_pState->movetype = MOVETYPE_NONE;
	m_pState->effects |= EF_LADDER;
	m_pState->solid = SOLID_BBOX;
	m_pState->rendermode = RENDER_NORMAL;
	m_pState->flags |= (FL_INITIALIZE|FL_NO_HITBOX_TRACE);

	if(!HasSpawnFlag(FL_START_OFF))
		m_isActive = true;
	
	// Set top bodypart
	if(HasSpawnFlag(FL_TOP_ACCESS))
		m_pState->body = 1;

	// Round down origin
	for(Uint32 i = 0; i < 3; i++)
	{
		float flfrac = m_pState->origin[i] - floor(m_pState->origin[i]);
		m_pState->origin[i] = (flfrac > 0.5) ? ceil(m_pState->origin[i]) : floor(m_pState->origin[i]);
	}

	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvLadder::InitEntity( void )
{
	if(m_numSegments)
		return;

	// Find endpoint and verify accessibility
	const Char* pstrTargetname = gd_engfuncs.pfnGetString(m_pFields->target);
	if(!pstrTargetname)
	{
		Util::RemoveEntity(this);
		return;
	}

	edict_t *pEnd = Util::FindEntityByTargetName(nullptr, pstrTargetname);
	if(!pEnd)
	{
		Util::EntityConPrintf(m_pEdict, "Ladder can't find bottom marker '%s'\n", pstrTargetname);
		Util::RemoveEntity(this);
		return;
	}

	// Store number of segments in skin
	float height = m_pState->origin.z - pEnd->state.origin.z;
	if(height < 0)
	{
		Util::EntityConPrintf(m_pEdict, "Bottom marker '%s' is above the ladder entity.\n", pstrTargetname);
		Util::RemoveEntity(this);
		return;	
	}

	m_numSegments = floor(height/(float)LADDER_PIECE_HEIGHT);
	m_pState->numsegments = m_numSegments;
	
	Vector seqMins, seqMaxs;
	GetSequenceBox((*m_pState), seqMins, seqMaxs, false);

	Float rounded_height = m_pState->numsegments * LADDER_PIECE_HEIGHT;
	Vector vMins = Vector(seqMins.x, seqMins.y, -rounded_height);
	Vector vMaxs = Vector(seqMaxs.x, seqMaxs.y, LADDER_PIECE_HEIGHT);

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, vMins, vMaxs);
}

//=============================================
// @brief
//
//=============================================
void CEnvLadder::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!pCaller->IsPlayer())
	{
		if(useMode == USE_ON)
			m_isActive = TRUE;
		else if(useMode == USE_OFF)
			m_isActive = FALSE;
		else
		{
			if(m_isActive)
				m_isActive = FALSE;
			else
				m_isActive = TRUE;
		}
		return;
	}

	if(!pActivator->IsPlayer())
		return;

	if(!m_isActive && pActivator->IsPlayer())
		return;

	if(pActivator->GetWaterLevel() < WATERLEVEL_LOW && !(pActivator->GetFlags() & FL_ONGROUND))
		return;

	Vector forward, right, up;
	Math::AngleVectors(m_pState->angles, &forward, &right, &up);

	// See if we can use it at this momment
	Vector vOrigin, vAngles;
	RepositionPlayer(pActivator, &vOrigin, &vAngles);

	// make non-solid and relink
	m_pState->solid = SOLID_NOT;
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);

	if(!VerifyMove(vOrigin, pActivator, 1) && !VerifyMove(vOrigin, pActivator, -1)
		|| (HasSpawnFlag(FL_TOP_ACCESS) 
		&& (m_pState->origin.z < pActivator->GetOrigin().z) && !IsTopAccessible(pActivator)))
	{
		// Make solid and relink
		m_pState->solid = SOLID_BBOX;
		gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);

		// Ladder is stuck in a solid or similar, can't move
		Util::EntityConPrintf(m_pEdict, "Ladder is stuck in a solid!\n");
		return;
	}

	//Player can enter
	pActivator->EnterLadder(this);
}

//=============================================
// @brief
//
//=============================================
void CEnvLadder::EnterLadder( CBaseEntity* pPlayer )
{
	m_pPlayer = pPlayer;
}

//=============================================
// @brief
//
//=============================================
void CEnvLadder::ExitLadder( void )
{
	m_pPlayer = nullptr;
	m_pState->solid = SOLID_BBOX;

	// Relink
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
}

//=============================================
// @brief
//
//=============================================
bool CEnvLadder::IsTopAccessible( CBaseEntity* pPlayer )
{
	// Reposition player to the ladder
	Vector origin;
	RepositionPlayer(pPlayer, &origin, nullptr);

	// Estimate offset to top exit
	Vector topOffset = origin + Vector(0, 0, LADDER_STEP_SIZE);

	// Disable solidity for check
	Int32 savedSolidity = m_pState->solid;
	m_pState->solid = SOLID_NOT;

	trace_t tr;
	Util::TraceHull(origin, topOffset, true, false, HULL_HUMAN, pPlayer->GetEdict(), tr);

	m_pState->solid = savedSolidity;

	if(tr.noHit() && !tr.startSolid() && !tr.allSolid())
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
void CEnvLadder::RepositionPlayer( CBaseEntity* pPlayer, Vector* porigin, Vector* pangles )
{
	Vector vForward;
	Math::AngleVectors(m_pState->angles, &vForward, nullptr, nullptr);
	Vector vOffset = vForward * LADDER_FORWARD_OFFSET;

	Int32 iLastClosestSegment = -1;
	Float flLastClosestDist = 0;

	// Never allow entry at the two end segments
	Float flPlayerZ = pPlayer->GetOrigin().z;
	for(Int32 i = 1; i < m_numSegments; i++)
	{
		Float flSegmentZ = m_pState->origin.z - i*LADDER_PIECE_HEIGHT;
		Float flDist = abs(flSegmentZ-flPlayerZ);

		if(iLastClosestSegment == -1 || flLastClosestDist > flDist)
		{
			flLastClosestDist = flDist;
			iLastClosestSegment = i;
		}
	}

	if(porigin)
	{
		Float flOffset = iLastClosestSegment * LADDER_PIECE_HEIGHT;
		(*porigin) = m_pState->origin + vOffset;
		(*porigin).z += - flOffset;
	}

	if(pangles)
	{
		// Calculate angles
		Vector vDir = (m_pState->origin-(m_pState->origin + vOffset));
		vDir[2] = 0; 
		vDir.Normalize();

		Vector up, right;
		Math::GetUpRight(vDir, up, right);
		Math::VectorScale(right, -1, right);

		(*pangles) = Math::VectorToAngles(vDir, right);
	}
}

//=============================================
// @brief
//
//=============================================
bool CEnvLadder::GetExitVectors( ladder_exitpoints_t exit, Vector* porigin, Vector* pangles, Float* pfldiff )
{
	Vector vOffset, vAngles;
	Vector vForward, vRight, vUp;
	Math::AngleVectors(m_pState->angles, &vForward, &vRight, &vUp);

	switch(exit)
	{
		case LADDER_EXIT_TOP: 
			vOffset = LADDER_TP_EXIT_FW*vForward + LADDER_TP_EXIT_UP*vUp; 
			vAngles = Math::VectorToAngles(-vForward);
			break;
		case LADDER_EXIT_LEFT: 
			vOffset = LADDER_LF_EXIT_RT*vRight + LADDER_LF_EXIT_FW*vForward;
			vAngles = Math::VectorToAngles(-vRight);
			break;
		case LADDER_EXIT_RIGHT: 
			vOffset = LADDER_RT_EXIT_RT*vRight + LADDER_RT_EXIT_FW*vForward;
			vAngles = Math::VectorToAngles(vRight);
			break;
		case LADDER_EXIT_BOTTOM:
			vOffset = LADDER_BT_EXIT_FW*vForward;
			vAngles = Math::VectorToAngles(-vForward);
			break;
	}

	// Estimate exit point from player's z
	const Vector& playerOrigin = m_pPlayer->GetOrigin();
	Vector vExitOrigin = m_pState->origin;
	vExitOrigin.z = playerOrigin.z;
	vExitOrigin = vExitOrigin + vOffset;

	Float diff = 0;
	Vector finalAngles;
	Vector finalOrigin;
	bool result = false;

	for(Uint32 i = 0; i < 3; i++)
	{
		// Determine if the player can fit there
		Vector vStart = vExitOrigin;
		Vector vEnd = vExitOrigin;

		if(i == 0)
			vStart = vExitOrigin + Vector(0, 0, LADDER_PIECE_HEIGHT+16);
		else if(i == 1)
			vEnd = vExitOrigin - Vector(0, 0, LADDER_PIECE_HEIGHT+8);
		else
			vEnd = vExitOrigin - Vector(0, 0, LADDER_PIECE_HEIGHT+16);

		trace_t tr;
		Util::TraceHull(vStart, vEnd, false, false, HULL_HUMAN, m_pPlayer->GetEdict(), tr);

		if(tr.fraction < 1 && !tr.startSolid())
		{
			finalOrigin = tr.endpos;
			finalAngles = vAngles;
			diff = tr.endpos.z-vExitOrigin.z;
			result = true;
		}
		else if(m_pPlayer->GetWaterLevel() > WATERLEVEL_NONE && !tr.allSolid())
		{
			finalOrigin = vExitOrigin;
			finalAngles = vAngles;
			diff = 0;
			result = true;
		}

		if(result)
			break;
	}

	if(result)
	{
		/*
		Vector testPosition(m_pState->origin.x,
			m_pState->origin.y,
			playerOrigin.z + LADDER_PIECE_HEIGHT);

		trace_t tr;
		Util::TraceLine(testPosition, finalOrigin, false, false, m_pPlayer->GetEdict(), tr);
		if(tr.allSolid() || tr.startSolid() || !tr.noHit())
		{
			// Hit a solid
			return false;
		}*/

		if(porigin) 
			(*porigin) = finalOrigin;

		if(pangles) 
			(*pangles) = finalAngles;

		if(pfldiff) 
			(*pfldiff) = diff;
	}

	return result;
}

//=============================================
// @brief
//
//=============================================
ladder_exitpoints_t CEnvLadder::CanUseExit( Vector* pangles, Vector* porigin, Float* pfltime, Float* pfldiff )
{
	if(GetExitVectors(LADDER_EXIT_LEFT, porigin, pangles, pfldiff))
	{
		*pfltime = LADDER_LEAVE_LEFT_TIME;
		return LADDER_EXIT_LEFT;
	}

	if(GetExitVectors(LADDER_EXIT_RIGHT, porigin, pangles, pfldiff))
	{
		*pfltime = LADDER_LEAVE_RIGHT_TIME;
		return LADDER_EXIT_RIGHT;
	}

	if(HasSpawnFlag(FL_TOP_ACCESS))
	{
		Float flMoveHeight = m_pPlayer->GetOrigin().z+LADDER_PIECE_HEIGHT;
		if((flMoveHeight >= m_pState->origin.z) && GetExitVectors(LADDER_EXIT_TOP, porigin, pangles, pfldiff))
		{
			*pfltime = LADDER_LEAVE_TOP_TIME;
			return LADDER_EXIT_TOP;
		}
	}
	
	if(GetExitVectors(LADDER_EXIT_BOTTOM, porigin, pangles, pfldiff))
	{
		*pfltime = LADDER_LEAVE_BOTTOM_TIME;
		return LADDER_EXIT_BOTTOM;
	}

	return LADDER_EXIT_UNAVAILABLE;
}

//=============================================
// @brief
//
//=============================================
ladder_verify_codes_t CEnvLadder::VerifyMove( const Vector& origin, CBaseEntity* pPlayer, Int32 direction )
{
	Float flPlayerMove = direction * LADDER_STEP_SIZE;

	trace_t tr;
	Util::TraceHull(origin, origin+Vector(0, 0, flPlayerMove), m_pPlayer ? false : true, false, HULL_HUMAN, pPlayer->GetEdict(), tr);

	if(!m_pPlayer)
	{
		// This is just an entry check, don't bother checking exits
		if(tr.allSolid() || tr.fraction != 1.0)
			return LADDER_VR_NOMOVE;
		else
			return LADDER_VR_MOVE_VALID;
	}

	if(tr.allSolid() || tr.fraction != 1.0)
	{
		float checkHeight = origin.z + flPlayerMove - 4;
		if(checkHeight < m_pState->absmin.z)
		{
			if(GetExitVectors(LADDER_EXIT_BOTTOM) || GetExitVectors(LADDER_EXIT_RIGHT) || GetExitVectors(LADDER_EXIT_LEFT))
				return LADDER_VR_MOVE_EXIT_BOTTOM;	

			return LADDER_VR_NOMOVE;
		}
		else if(GetExitVectors(LADDER_EXIT_RIGHT) || GetExitVectors(LADDER_EXIT_LEFT))
			return LADDER_VR_MOVE_EXIT_BOTTOM;

		return LADDER_VR_NOMOVE;
	}

	if(origin.z+flPlayerMove >= m_pState->origin.z)
	{
		if(HasSpawnFlag(FL_TOP_ACCESS))
		{
			if(GetExitVectors(LADDER_EXIT_TOP))
				return LADDER_VR_MOVE_EXIT_TOP;	
		}

		if(GetExitVectors(LADDER_EXIT_RIGHT) || GetExitVectors(LADDER_EXIT_LEFT))
			return LADDER_VR_MOVE_EXIT_TOP;	

		return LADDER_VR_NOMOVE;
	}
	else if((origin.z+flPlayerMove-3) < m_pState->absmin.z)
	{
		if(GetExitVectors(LADDER_EXIT_BOTTOM) || GetExitVectors(LADDER_EXIT_RIGHT) || GetExitVectors(LADDER_EXIT_LEFT))
			return LADDER_VR_MOVE_EXIT_BOTTOM;	

		return LADDER_VR_NOMOVE;
	}

	return LADDER_VR_MOVE_VALID;
}

//=============================================
// @brief
//
//=============================================
ladder_entrypoints_t CEnvLadder::GetEntryAnimation( void )
{
	// TODO: Check if top is actually accessible
	if((m_pState->origin.z < m_pPlayer->GetOrigin().z) && HasSpawnFlag(FL_TOP_ACCESS))
	{
		// Player can enter from right up
		return LADDER_ENTER_TOP;
	}

	if(m_pPlayer->GetWaterLevel() > WATERLEVEL_NONE && !(m_pPlayer->GetFlags() & FL_ONGROUND))
		return LADDER_ENTER_BOTTOM;

	// Check if there's ground where the player wants to enter
	Vector vForward, vRight;
	Math::AngleVectors(m_pState->angles, &vForward, &vRight, nullptr);
	Vector vOffset = vForward * LADDER_FORWARD_OFFSET;

	Vector vStart = m_pState->origin + vOffset;
	vStart.z = m_pPlayer->GetOrigin().z;
	Vector vEnd = vStart - Vector(0, 0, 16);

	trace_t tr;
	Util::TraceHull(vStart, vEnd, true, false, (m_pPlayer->GetFlags() & FL_DUCKING) ? HULL_SMALL : HULL_HUMAN, m_pEdict, tr);

	if(tr.fraction != 1.0)
	{
		// Player can enter from right up
		return LADDER_ENTER_BOTTOM;
	}
	else
	{
		// Remove z and normalize
		Vector vDir = m_pState->origin - m_pPlayer->GetOrigin();
		vDir[2] = 0; vDir.Normalize();

		// Right is left to the player
		float flDot = Math::DotProduct(vRight, vDir);
		return (flDot < 0) ? LADDER_ENTER_LEFT : LADDER_ENTER_RIGHT;
	}
}

//=============================================
// @brief
//
//=============================================
usableobject_type_t CEnvLadder::GetUsableObjectType( void )
{ 
	if(!m_isActive)
		return USABLE_OBJECT_LOCKED;
	else
		return USABLE_OBJECT_DEFAULT;
}

//=============================================
// @brief
//
//=============================================
void CEnvLadder::GetUseReticleMinsMaxs( Vector& outMins, Vector& outMaxs, CBaseEntity* pPlayer )
{
	Vector playerEyePosition = pPlayer->GetEyePosition(true);
	Vector playerViewAngles = pPlayer->GetViewAngles();

	Vector forward;
	Math::AngleVectors(playerViewAngles, &forward);

	Float playerUseDistance = Vector(CPlayerEntity::PLAYER_USE_RADIUS, CPlayerEntity::PLAYER_USE_RADIUS, CPlayerEntity::PLAYER_USE_RADIUS).Length();
	Vector endPosition = playerEyePosition + playerUseDistance * forward;

	trace_t tr;
	Util::TraceLine(playerEyePosition, endPosition, false, true, pPlayer->GetEdict(), tr);

	if(tr.noHit() || tr.startSolid() || tr.allSolid() || tr.hitentity != GetEntityIndex())
	{
		outMins = GetAbsMins();
		outMaxs = GetAbsMaxs();
	}
	else
	{
		outMins = Vector(m_pState->absmin.x, m_pState->absmin.y, tr.endpos.z - 8);
		outMaxs = Vector(m_pState->absmax.x, m_pState->absmax.y, tr.endpos.z + 8);
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvLadder::TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags )
{
	Util::Ricochet( tr.endpos, tr.plane.normal, true );
	Util::CreateVBMDecal( tr.endpos, tr.plane.normal, "shot_metal", m_pEdict, FL_DECAL_NORMAL_PERMISSIVE);
}