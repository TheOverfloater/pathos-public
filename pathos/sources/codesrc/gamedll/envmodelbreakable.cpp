/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envmodelbreakable.h"
#include "animatingentity.h"
#include "materialdefs.h"

// Material names for material types
const Char* CEnvModelBreakable::MAT_TYPE_MAT_NAMES[NB_BREAK_MATERIALS] = 
{
	"glass", //MAT_GLASS
	"wood", //MAT_WOOD
	"metal", //MAT_METAL
	"organic", //MAT_FLESH
	"concrete", //MAT_CINDERBLOCK
	"concrete", //MAT_CEILINGTILE
	"computer", //MAT_COMPUTER
	"glass", //MAT_UNBREAKABLE_GLASS
	"concrete", //MAT_ROCKS
	""//MAT_NONE
};

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_model_breakable, CEnvModelBreakable);

//=============================================
// @brief
//
//=============================================
CEnvModelBreakable::CEnvModelBreakable( edict_t* pedict ):
	CFuncBreakable(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CEnvModelBreakable::~CEnvModelBreakable( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvModelBreakable::Precache( void )
{
	gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_pFields->modelname));

	CFuncBreakable::Precache();
}

//=============================================
// @brief
//
//=============================================
void CEnvModelBreakable::SetSpawnProperties( void )
{
	m_pState->movetype = MOVETYPE_PUSH;
	m_pState->solid = SOLID_BBOX;
}

//=============================================
// @brief
//
//=============================================
void CEnvModelBreakable::SetBoundingBox( void )
{
	Vector mins, maxs;
	if(CAnimatingEntity::GetSequenceBox(*m_pState, mins, maxs, false))
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, mins, maxs);
}

//=============================================
// @brief
//
//=============================================
void CEnvModelBreakable::TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags )
{
	// Let base class handle it's own
	CFuncBreakable::TraceAttack(pAttacker, damage, direction, tr, damageFlags);

	// Get impact effect for material
	if(m_material >= MAT_GLASS && m_material < MAT_NONE)
	{
		const CMaterialDefinitions::materialdefinition_t* peffect = gMaterialDefinitions.GetDefinitionForMaterial(MAT_TYPE_MAT_NAMES[m_material]);
		if(!peffect)
			return;

		if(!peffect->decalgroup.empty())
			Util::CreateVBMDecal(tr.endpos, tr.plane.normal, peffect->decalgroup.c_str(), m_pEdict, FL_DECAL_NONE);

		if(!peffect->particlescript.empty())
			Util::CreateParticles(peffect->particlescript.c_str(), tr.endpos, tr.plane.normal, peffect->scripttype);

		if(!peffect->sounds.empty())
		{
			Int32 soundindex = Common::RandomLong(0, peffect->sounds.size()-1);
			const CString& sound = peffect->sounds[soundindex];

			Util::EmitEntitySound(this, sound.c_str(), SND_CHAN_ITEM);
		}
	}
}