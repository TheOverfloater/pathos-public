/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "infonode.h"
#include "ai_nodegraph.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(info_node, CInfoNode);

//=============================================
// @brief
//
//=============================================
CInfoNode::CInfoNode( edict_t* pedict ):
	CPointEntity(pedict),
	m_hintType(0),
	m_activity(0),
	m_range(0)
{
}

//=============================================
// @brief
//
//=============================================
CInfoNode::~CInfoNode( void )
{
}

//=============================================
// @brief
//
//=============================================
void CInfoNode::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CInfoNode, m_hintType, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CInfoNode, m_activity, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CInfoNode, m_range, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CInfoNode::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "hinttype"))
	{
		m_hintType = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "activity"))
	{
		m_activity = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "range"))
	{
		m_range = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "nodeid"))
	{
		// Shut up engine messages
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CInfoNode::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	bool disableOptimizations = (m_pState->spawnflags & SF_DISABLE_OPTIMIZATION) ? true : false;
	gNodeGraph.AddNode(m_pState->origin, m_pState->angles[YAW], m_hintType, m_activity, m_range, gd_engfuncs.pfnGetString(m_pFields->netname), IsAirNode(), disableOptimizations);

	Util::RemoveEntity(this);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CInfoNode::IsAirNode( void )
{
	return false;
}
