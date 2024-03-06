/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcfriction.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_friction, CFuncFriction);

//=============================================
// @brief
//
//=============================================
CFuncFriction::CFuncFriction( edict_t* pedict ):
	CBaseEntity(pedict),
	m_frictionModifier(0)
{
}

//=============================================
// @brief
//
//=============================================
CFuncFriction::~CFuncFriction( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncFriction::DeclareSaveFields( void )
{
	// Call base class to do it first
	CBaseEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncFriction, m_frictionModifier, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CFuncFriction::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "modifier"))
	{
		m_frictionModifier = SDL_atof(kv.value);
		m_frictionModifier = m_frictionModifier/100.0f;
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CFuncFriction::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	m_pState->solid = SOLID_TRIGGER;
	m_pState->movetype = MOVETYPE_NONE;

	if(!SetModel(m_pFields->modelname))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncFriction::CallTouch( CBaseEntity* pOther )
{
	movetype_t movetype = pOther->GetMoveType();
	if(movetype == MOVETYPE_BOUNCE)
		return;

	pOther->SetFriction(m_frictionModifier);
}