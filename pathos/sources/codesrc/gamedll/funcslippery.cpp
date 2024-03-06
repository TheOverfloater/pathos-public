/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcslippery.h"
#include "playermove.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_slippery, CFuncSlippery);

//=============================================
// @brief
//
//=============================================
CFuncSlippery::CFuncSlippery( edict_t* pedict ):
	CBaseEntity(pedict),
	m_planeZCap(0)
{
}

//=============================================
// @brief
//
//=============================================
CFuncSlippery::~CFuncSlippery( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncSlippery::DeclareSaveFields( void )
{
	// Call base class to do it first
	CBaseEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncSlippery, m_planeZCap, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CFuncSlippery::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "factor"))
	{
		Float factor = SDL_atof(kv.value);
		if(factor > 10)
			factor = 10;
		else if(factor < 0)
			factor = 0;

		Float factorAdd = ((1.0 - DEFAULT_ONGROUND_LOWER_CAP) - 0.1) * (factor / 10.0f);
		m_planeZCap = DEFAULT_ONGROUND_LOWER_CAP + factorAdd;
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CFuncSlippery::Spawn( void )
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
void CFuncSlippery::CallTouch( CBaseEntity* pOther )
{
	movetype_t movetype = pOther->GetMoveType();
	if(movetype == MOVETYPE_BOUNCE)
		return;

	pOther->SetPlaneZCap(m_planeZCap);
}