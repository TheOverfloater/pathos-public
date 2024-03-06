/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envsetbodygroup.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_setbodygroup, CEnvSetBodyGroup);

//=============================================
// @brief
//
//=============================================
CEnvSetBodyGroup::CEnvSetBodyGroup( edict_t* pedict ):
	CPointEntity(pedict),
	m_bodyGroupName(NO_STRING_VALUE),
	m_submodelName(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSetBodyGroup::~CEnvSetBodyGroup( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvSetBodyGroup::DeclareSaveFields( void )
{
	// Call base class to handle it's own
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSetBodyGroup, m_bodyGroupName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSetBodyGroup, m_submodelName, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CEnvSetBodyGroup::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "bodygroup"))
	{
		m_bodyGroupName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "submodel"))
	{
		m_submodelName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvSetBodyGroup::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE || m_pFields->target == NO_STRING_VALUE
		|| m_bodyGroupName == NO_STRING_VALUE || m_submodelName == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvSetBodyGroup::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool isFirst = false;
	edict_t* pedict = nullptr;
	const Char* pstrEntityName = gd_engfuncs.pfnGetString(m_pFields->target);
	const Char* pstrGroupName = gd_engfuncs.pfnGetString(m_bodyGroupName);
	const Char* pstrSubmodelName = gd_engfuncs.pfnGetString(m_submodelName);

	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, pstrEntityName);
		if(!pedict)
		{
			if(isFirst)
				Util::EntityConPrintf(m_pEdict, "Couldn't find target '%s'.\n", pstrEntityName);

			return;
		}

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(!pEntity->IsAnimatingEntity())
		{
			Util::EntityConPrintf(m_pEdict, "Target '%s' is not an animating entity.\n", pstrEntityName);
			continue;
		}

		// Find the bodygroup specified
		Int32 bodyGroupIndex = pEntity->GetBodyGroupIndexByName(pstrGroupName);
		if(bodyGroupIndex == NO_POSITION)
		{
			Util::EntityConPrintf(m_pEdict, "Target '%s' has no bodygroup named '%s'.\n", pstrEntityName, pstrGroupName);
			continue;
		}

		// Find the submodel specified
		Int32 submodelIndex = pEntity->GetSubmodelIndexByName(bodyGroupIndex, pstrSubmodelName);
		if(submodelIndex == NO_POSITION)
		{
			Util::EntityConPrintf(m_pEdict, "Target '%s' has no submodel named '%s' in bodygroup '%s'.\n", pstrEntityName, pstrSubmodelName, pstrGroupName);
			continue;
		}

		pEntity->SetBodyGroup(bodyGroupIndex, submodelIndex);
		isFirst = false;
	}
}