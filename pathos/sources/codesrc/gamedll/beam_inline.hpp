/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef BEAM_INLINE_H
#define BEAM_INLINE_H

//=============================================
// @brief Sets the beam type
//
//=============================================
inline void CBeam::SetBeamType( beam_msgtype_t type )
{
	m_pState->iuser1 = type;
}

//=============================================
// @brief Returns the beam type
//
//=============================================
inline beam_msgtype_t CBeam::GetBeamType( void ) const
{
	return (beam_msgtype_t)m_pState->iuser1;
}

//=============================================
// @brief Sets a beam flag
//
//=============================================
inline void CBeam::SetBeamFlags( Int32 flags )
{
	m_pState->iuser2 = flags;
}

//=============================================
// @brief Returns the beam flags
//
//=============================================
inline Int32 CBeam::GetBeamFlags( void ) const
{
	return m_pState->iuser2;
}

//=============================================
// @brief Sets the start position
//
//=============================================
inline void CBeam::SetBeamStartPosition( const Vector& position )
{
	m_pState->origin = position;
}

//=============================================
// @brief Returns the beam's starting position
//
//=============================================
inline const Vector& CBeam::GetBeamStartPosition( void ) const
{
	if(GetBeamType() == BEAM_MSG_BEAMENTS && m_startEntity)
		return m_startEntity->GetOrigin();	
	else
		return m_pState->origin;
}

//=============================================
// @brief Sets the end position
//
//=============================================
inline void CBeam::SetBeamEndPosition( const Vector& position )
{
	m_pState->vuser1 = position;
}

//=============================================
// @brief Returns the beam's starting position
//
//=============================================
inline const Vector& CBeam::GetBeamEndPosition( void ) const
{
	beam_msgtype_t type = GetBeamType();
	if(type != BEAM_MSG_BEAMPOINTS && m_endEntity)
		return m_endEntity->GetOrigin();	
	else
		return m_pState->vuser1;
}

//=============================================
// @brief Sets the start entity attachment
//
//=============================================
inline void CBeam::SetBeamStartEntityAttachment( Int32 attachmentindex )
{
	m_attachment1Index = attachmentindex;
	m_pState->iuser5 = attachmentindex;
}

//=============================================
// @brief Returns the start entity attachment
//
//=============================================
inline Int32 CBeam::GetBeamStartEntityAttachment( void )
{
	return m_pState->iuser5;
}

//=============================================
// @brief Sets the end entity attachment
//
//=============================================
inline void CBeam::SetBeamEndEntityAttachment( Int32 attachmentindex )
{
	m_attachment2Index = attachmentindex;
	m_pState->iuser6 = attachmentindex;
}

//=============================================
// @brief Returns the end entity attachment
//
//=============================================
inline Int32 CBeam::GetBeamEndEntityAttachment( void )
{
	return m_pState->iuser5;
}

//=============================================
// @brief Sets the start entity
//
//=============================================
inline void CBeam::SetBeamStartEntity( CBaseEntity* pEntity )
{
	m_startEntity = pEntity;
	if(pEntity)
		m_pState->iuser3 = pEntity->GetEntityIndex();
	else
		m_pState->iuser3 = NO_ENTITY_INDEX;

	if(m_startEntity)
	{
		m_pState->aiment = m_startEntity->GetEntityIndex();
		m_pState->movetype = MOVETYPE_FOLLOW;
	}
}

//=============================================
// @brief Returns the start entity index
//
//=============================================
inline CBaseEntity* CBeam::GetBeamStartEntity( void )
{
	return m_startEntity;
}

//=============================================
// @brief Sets the end entity
//
//=============================================
inline void CBeam::SetBeamEndEntity( CBaseEntity* pEntity )
{
	m_endEntity = pEntity;
	if(pEntity)
		m_pState->iuser4 = pEntity->GetEntityIndex();
	else
		m_pState->iuser4 = NO_ENTITY_INDEX;
}

//=============================================
// @brief Returns the start entity index
//
//=============================================
inline CBaseEntity* CBeam::GetBeamEndEntity( void )
{
	return m_endEntity;
}

//=============================================
// @brief Sets the texture of the beam
//
//=============================================
inline bool CBeam::SetBeamTexture( Int32 spriteindex )
{
	const cache_model_t* pModel = gd_engfuncs.pfnGetModel(spriteindex);
	if(!pModel)
		return false;

	return gd_engfuncs.pfnSetModel(m_pEdict, pModel->name.c_str(), false);
}

//=============================================
// @brief Sets the texture of the beam
//
//=============================================
inline bool CBeam::SetBeamTexture( const Char* pstrSpriteName )
{
	return gd_engfuncs.pfnSetModel(m_pEdict, pstrSpriteName, false);
}

//=============================================
// @brief Returns the sprite index
//
//=============================================
inline Int32 CBeam::GetBeamTexture( void ) const
{
	return m_pState->modelindex;
}

//=============================================
// @brief Sets the width of the beam
//
//=============================================
inline void CBeam::SetBeamWidth( Float width )
{
	m_pState->scale = width * 0.1;
}

//=============================================
// @brief Returns the beam width
//
//=============================================
inline Float CBeam::GetBeamWidth( void ) const
{
	return m_pState->scale;
}

//=============================================
// @brief Sets the amplitude of the beam
//
//=============================================
inline void CBeam::SetBeamAmplitude( Float amplitude )
{
	m_pState->fuser2 = amplitude;
}

//=============================================
// @brief Returns the beam amplitude
//
//=============================================
inline Float CBeam::GetBeamAmplitude( void ) const
{
	return m_pState->fuser2;
}

//=============================================
// @brief Sets the color of the beam
//
//=============================================
inline void CBeam::SetBeamColor( Int32 r, Int32 g, Int32 b )
{
	m_pState->rendercolor.x = clamp(r, 0, 255);
	m_pState->rendercolor.y = clamp(g, 0, 255);
	m_pState->rendercolor.z = clamp(b, 0, 255);
}

//=============================================
// @brief Returns the beam color
//
//=============================================
inline const Vector& CBeam::GetBeamColor( void ) const
{
	return m_pState->rendercolor;
}

//=============================================
// @brief Sets the brightness of the beam
//
//=============================================
inline void CBeam::SetBeamBrightness( Float brightness )
{
	m_pState->renderamt = clamp(brightness, 0, 255);
}

//=============================================
// @brief Returns the beam brightness
//
//=============================================
inline Float CBeam::GetBeamBrightness( void ) const
{
	return m_pState->renderamt;
}

//=============================================
// @brief Sets the current frame
//
//=============================================
inline void CBeam::SetBeamFrame( Float frame )
{
	m_pState->frame = frame;
}

//=============================================
// @brief Returns the beam frame
//
//=============================================
inline Float CBeam::GetBeamFrame( void ) const
{
	return m_pState->frame;
}

//=============================================
// @brief Sets the scroll rate
//
//=============================================
inline void CBeam::SetBeamScrollRate( Float speed )
{
	m_pState->fuser1 = speed;
}

//=============================================
// @brief Returns the scroll rate
//
//=============================================
inline Float CBeam::GetBeamScrollRate( void ) const
{
	return m_pState->fuser1;
}

//=============================================
// @brief Sets the noise speed
//
//=============================================
inline void CBeam::SetBeamNoiseSpeed( Float speed )
{
	m_pState->fuser3 = speed;
}

//=============================================
// @brief Returns the noise speed
//
//=============================================
inline Float CBeam::GetBeamNoiseSpeed( void ) const
{
	return m_pState->fuser3;
}

//=============================================
// @brief Sets the beam to die after the given amount of time
//
//=============================================
inline void CBeam::LiveForTime( Float time )
{
	SetThink(&CBaseEntity::RemoveThink);
	m_pState->nextthink = g_pGameVars->time + time;
}

//=============================================
// @brief Deal instant damage
//
//=============================================
inline void CBeam::BeamDamageInstant( trace_t& tr, Float damage )
{
	m_beamDamage = damage;
	m_dmgTime = g_pGameVars->time - 1;
	BeamDamage(tr);
}
#endif //BEAM_INLINE_H