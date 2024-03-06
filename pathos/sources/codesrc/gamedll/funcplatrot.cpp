/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcplatrot.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_platrot, CFuncPlatRot);

//=============================================
// @brief
//
//=============================================
CFuncPlatRot::CFuncPlatRot( edict_t* pedict ):
	CFuncPlat(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncPlatRot::~CFuncPlatRot( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncPlatRot::DeclareSaveFields( void )
{
	// Call base class to do it first
	CFuncPlat::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPlatRot, m_startAngles, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPlatRot, m_endAngles, EFIELD_VECTOR));
}

//=============================================
// @brief
//
//=============================================
bool CFuncPlatRot::Spawn( void )
{
	if(!CFuncPlat::Spawn())
		return false;

	SetupRotation();
	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncPlatRot::GoUp( void )
{
	CFuncPlat::GoUp();
	RotateMove(m_endAngles, m_pState->nextthink - m_pState->ltime);
}

//=============================================
// @brief
//
//=============================================
void CFuncPlatRot::GoDown( void )
{
	CFuncPlat::GoDown();
	RotateMove(m_startAngles, m_pState->nextthink - m_pState->ltime);
}

//=============================================
// @brief
//
//=============================================
void CFuncPlatRot::HitTop( void )
{
	CFuncPlat::HitTop();
	m_pState->avelocity.Clear();
	m_pState->angles = m_endAngles;
}

//=============================================
// @brief
//
//=============================================
void CFuncPlatRot::HitBottom( void )
{
	CFuncPlat::HitBottom();
	m_pState->avelocity.Clear();
	m_pState->angles = m_startAngles;
}

//=============================================
// @brief
//
//=============================================
void CFuncPlatRot::RotateMove( const Vector& destAngle, Double time )
{
	Vector destDelta = destAngle - m_pState->angles;
	if(time >= 0.1)
	{
		m_pState->avelocity = destDelta/time;
	}
	else
	{
		m_pState->avelocity = destDelta;
		m_pState->nextthink = m_pState->ltime + 1.0f;
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncPlatRot::SetupRotation( void )
{
	if(m_finalAngle.x != 0)
	{
		Util::SetAxisDirection(*m_pState, m_pState->spawnflags, FL_ROTATE_Z, FL_ROTATE_X);
		m_startAngles = m_pState->angles;
		m_endAngles = m_startAngles + m_pState->movedir * m_finalAngle.x;
	}
	else
	{
		m_startAngles.Clear();
		m_endAngles.Clear();
	}

	// Start at top if we have a targetname
	if(m_pFields->targetname != NO_STRING_VALUE)
		m_pState->angles = m_endAngles;
}
