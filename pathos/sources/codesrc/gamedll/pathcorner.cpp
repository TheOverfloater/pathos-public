/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "pathcorner.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(path_corner, CPathCorner);

//=============================================
// @brief
//
//=============================================
CPathCorner::CPathCorner( edict_t* pedict ):
	CPointEntity(pedict),
	m_waitTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CPathCorner::~CPathCorner( void )
{
}

//=============================================
// @brief
//
//=============================================
void CPathCorner::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CPathCorner, m_waitTime, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CPathCorner::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "wait"))
	{
		m_waitTime = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CPathCorner::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(m_pFields->target != NO_STRING_VALUE && 
		!qstrcicmp(gd_engfuncs.pfnGetString(m_pFields->targetname),
		gd_engfuncs.pfnGetString(m_pFields->target)))
	{
		Util::EntityConPrintf(m_pEdict, "Entity is targeting itself.\n");
		Util::RemoveEntity(this);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
Float CPathCorner::GetDelay( void )
{
	return m_waitTime;
}