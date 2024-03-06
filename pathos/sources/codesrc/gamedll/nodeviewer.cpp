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
#include "nodeviewer.h"
#include "ai_nodegraph.h"

// Color for fly hull
const Vector CNodeViewer::COLOR_FLY_HULL = Vector(160, 100, 255);
// Color for large hull
const Vector CNodeViewer::COLOR_LARGE_HULL = Vector(100, 255, 160);
// Color for small hull
const Vector CNodeViewer::COLOR_SMALL_HULL = Vector(100, 255, 160);
// Color for human hull
const Vector CNodeViewer::COLOR_HUMAN_HULL = Vector(255, 160, 100);

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(node_viewer_small, CNodeViewer);
LINK_ENTITY_TO_CLASS(node_viewer_human, CNodeViewer);
LINK_ENTITY_TO_CLASS(node_viewer_fly, CNodeViewer);
LINK_ENTITY_TO_CLASS(node_viewer_large, CNodeViewer);

//=============================================
// @brief
//
//=============================================
CNodeViewer::CNodeViewer( edict_t* pedict ):
	CBaseEntity(pedict),
	m_baseNode(NO_POSITION),
	m_numVisited(0),
	m_hullType(NODE_SMALL_HULL),
	m_nodeType(AI_NODE_NONE)
{
	memset(m_fromNodes, 0, sizeof(m_fromNodes));
	memset(m_toNodes, 0, sizeof(m_toNodes));
}

//=============================================
// @brief
//
//=============================================
CNodeViewer::~CNodeViewer( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CNodeViewer::Spawn( void )
{
	if(!gNodeGraph.IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - Graph not ready.\n", __FUNCTION__);
		Util::RemoveEntity(m_pEdict);
		return true;
	}

	if(!qstrcmp(GetClassName(), "node_viewer_fly"))
	{
		m_hullType = NODE_FLY_HULL;
		m_nodeType = AI_NODE_AIR;
		m_color = COLOR_FLY_HULL;
	}
	else if(!qstrcmp(GetClassName(), "node_viewer_large"))
	{
		m_hullType = NODE_LARGE_HULL;
		m_nodeType = AI_NODE_LAND|AI_NODE_WATER;
		m_color = COLOR_LARGE_HULL;
	}
	else if(!qstrcmp(GetClassName(), "node_viewer_small"))
	{
		m_hullType = NODE_SMALL_HULL;
		m_nodeType = AI_NODE_LAND|AI_NODE_WATER;
		m_color = COLOR_SMALL_HULL;
	}
	else
	{
		m_hullType = NODE_HUMAN_HULL;
		m_nodeType = AI_NODE_LAND|AI_NODE_WATER;
		m_color = COLOR_HUMAN_HULL;
	}

	m_baseNode = gNodeGraph.GetNearestNode(m_pState->origin, m_nodeType, this);
	if(m_baseNode == NO_POSITION)
	{
		gd_engfuncs.pfnCon_Printf("No node found.\n");
		Util::RemoveEntity(m_pEdict);
		return true;
	}

	Int32 numnodes = gNodeGraph.GetNumNodes();
	if(numnodes <= MAX_NODES)
	{
		for(Int32 i = 0; i < numnodes; i++)
		{
			Int32 nextNode = gNodeGraph.GetNextNodeInRoute(i, m_baseNode, (node_hull_types_t)m_hullType, CAINodeGraph::NODE_CAP_NONE);
			AddNode(i, nextNode);
		}
	}
	else
	{
		FindNodeConnections(m_baseNode);

		Int32 startIndex = 0;
		Int32 endIndex;
		do
		{
			for(endIndex = m_numVisited; startIndex < endIndex; startIndex++)
			{
				FindNodeConnections(m_fromNodes[startIndex]);
				FindNodeConnections(m_toNodes[startIndex]);
			}
		}
		while(endIndex != (Int32)m_numVisited);
	}

	gd_engfuncs.pfnCon_Printf("%s - %d nodes visited.\n", __FUNCTION__, m_numVisited);
	
	SetThink(&CNodeViewer::DrawThink);
	m_pState->nextthink = g_pGameVars->time;
	return true;
}

//=============================================
// @brief
//
//=============================================
void CNodeViewer::FindNodeConnections( Int32 nodeIndex )
{
	Int32 nextNode = gNodeGraph.GetNextNodeInRoute(nodeIndex, m_baseNode, (node_hull_types_t)m_hullType, CAINodeGraph::NODE_CAP_NONE);
	AddNode(nodeIndex, nextNode);

	const CAINodeGraph::node_t* pnode = gNodeGraph.GetNode(nodeIndex);
	for(Int32 i = 0; i < (Int32)pnode->numlinks; i++)
	{
		const CAINodeGraph::node_link_t* plink = gNodeGraph.GetNodeLink(pnode, i);
		nextNode = gNodeGraph.GetNextNodeInRoute(plink->destnode, m_baseNode, (node_hull_types_t)m_hullType, CAINodeGraph::NODE_CAP_NONE);

		AddNode(plink->destnode, nextNode);
	}
}

//=============================================
// @brief
//
//=============================================
void CNodeViewer::AddNode( Int32 fromNode, Int32 toNode )
{
	if(m_numVisited >= MAX_NODES)
		return;

	if(fromNode == toNode)
		return;

	for(Uint32 i = 0; i < m_numVisited; i++)
	{
		if(m_fromNodes[i] == fromNode && m_toNodes[i] == toNode)
			return;

		if(m_fromNodes[i] == toNode && m_toNodes[i] == fromNode)
			return;
	}

	m_fromNodes[m_numVisited] = fromNode;
	m_toNodes[m_numVisited] = toNode;
	m_numVisited++;
}

//=============================================
// @brief
//
//=============================================
void CNodeViewer::DrawThink( void )
{
	for(Uint32 i = 0; i < m_numVisited; i++)
	{
		const CAINodeGraph::node_t* pfromnode = gNodeGraph.GetNode(m_fromNodes[i]);
		if(!pfromnode)
		{
			gd_engfuncs.pfnCon_Printf("%s - Couldn't find node %d.\n", __FUNCTION__, m_fromNodes[i]);
			return;
		}

		const CAINodeGraph::node_t* ptonode = gNodeGraph.GetNode(m_toNodes[i]);
		if(!ptonode)
		{
			gd_engfuncs.pfnCon_Printf("%s - Couldn't find node %d.\n", __FUNCTION__, m_toNodes[i]);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.nodedebug, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(NODE_DEBUG_PATHS);
			
			Vector pathStart = pfromnode->origin + CAINodeGraph::NODE_PEEK_OFFSET;
			for(Uint32 j = 0; j < 3; j++)
				gd_engfuncs.pfnMsgWriteFloat(pathStart[j]);

			Vector pathEnd = ptonode->origin + CAINodeGraph::NODE_PEEK_OFFSET;
			for(Uint32 j = 0; j < 3; j++)
				gd_engfuncs.pfnMsgWriteFloat(pathEnd[j]);

			gd_engfuncs.pfnMsgWriteByte(m_color.x);
			gd_engfuncs.pfnMsgWriteByte(m_color.y);
			gd_engfuncs.pfnMsgWriteByte(m_color.z);
		gd_engfuncs.pfnUserMessageEnd();
	}
}