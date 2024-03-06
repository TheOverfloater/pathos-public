/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "plattrigger.h"
#include "funcplat.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(plattrigger, CPlatTrigger);

//=============================================
// @brief
//
//=============================================
CPlatTrigger::CPlatTrigger( edict_t* pedict ):
	CBaseEntity(pedict),
	m_pPlatform(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CPlatTrigger::~CPlatTrigger( void )
{
}

//=============================================
// @brief
//
//=============================================
void CPlatTrigger::DeclareSaveFields( void )
{
	// Call base class to do it first
	CBaseEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CPlatTrigger, m_pPlatform, EFIELD_ENTPOINTER));
}

//=============================================
// @brief
//
//=============================================
bool CPlatTrigger::Spawn( void )
{
	if(!m_pPlatform)
	{
		gd_engfuncs.pfnCon_Printf("CPlatTrigger entity with no platform set.\n");
		return false;
	}

	if(!CBaseEntity::Spawn())
		return false;

	m_pState->solid = SOLID_TRIGGER;
	m_pState->movetype = MOVETYPE_NONE;

	Vector platformMins = m_pPlatform->GetMins() + m_pPlatform->GetOrigin();
	Vector platformMaxs = m_pPlatform->GetMaxs() + m_pPlatform->GetOrigin();

	Float xInset = (platformMaxs.x - platformMins.x)*0.25;
	Float yInset = (platformMaxs.y - platformMins.y)*0.25;

	Vector platPosition1 = m_pPlatform->GetPosition1();
	Vector platPosition2 = m_pPlatform->GetPosition2();

	Float minZ, maxZ;
	if(platPosition1.z < platPosition2.z)
	{
		maxZ = platformMaxs.z;
		minZ = platformMins.z - platPosition2.z;
	}
	else
	{
		maxZ = platformMaxs.z + platPosition1.z;
		minZ = platformMins.z;
	}

	Float platHeight = platformMaxs.z - platformMins.z;

	Vector triggerMins = Vector(platformMins.x + xInset, platformMins.y + yInset, minZ - platHeight);
	Vector triggerMaxs = Vector(platformMaxs.x - xInset, platformMaxs.y - yInset, maxZ + platHeight);

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, triggerMins, triggerMaxs);
	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlatTrigger::CallTouch( CBaseEntity* pOther )
{
	// Only living players can interact with this entity
	if(!pOther->IsPlayer() || !pOther->IsAlive())
		return;

	if(m_pPlatform->GetToggleState() == TS_AT_BOTTOM)
		m_pPlatform->GoUp();
	else if(m_pPlatform->GetToggleState() == TS_AT_TOP)
		m_pPlatform->SetNextThinkTime(m_pPlatform->GetLocalTime() + 1);
}

//=============================================
// @brief
//
//=============================================
void CPlatTrigger::SetPlatform( CFuncPlat* pPlatform )
{
	m_pPlatform = pPlatform;
}

//=============================================
// @brief
//
//=============================================
void CPlatTrigger::SpawnPlatTrigger( CFuncPlat* pPlatform )
{
	CPlatTrigger* pTrigger = reinterpret_cast<CPlatTrigger*>(CBaseEntity::CreateEntity("plattrigger", nullptr));
	if(!pTrigger)
		return;

	pTrigger->SetPlatform(pPlatform);

	if(!pTrigger->Spawn())
		Util::RemoveEntity(pTrigger);
}