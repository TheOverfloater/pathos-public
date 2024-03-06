/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "clientdll.h"
#include "nodedebug.h"
#include "matrix.h"

// Node raise amount
const Vector CNodeDebug::NODE_RAISE_AMOUNT = Vector(0, 0, 4);
// Node raise for waypoints
const Vector CNodeDebug::NODE_RAISE_WAYPOINTS = Vector(0, 0, 16);
// Node color for waypoints
const Vector CNodeDebug::NODE_COLOR_WAYPOINTS = Vector(0, 1, 0);
// Node raise for detours
const Vector CNodeDebug::NODE_RAISE_DETOURS = Vector(0, 0, 8);
// Node color for detours
const Vector CNodeDebug::NODE_COLOR_DETOURS = Vector(0, 0, 1);
// Node raise for errors
const Vector CNodeDebug::NODE_RAISE_ERRORS = Vector(0, 0, 0);
// Node color for errors
const Vector CNodeDebug::NODE_COLOR_ERRORS = Vector(1, 0, 0);

// Object declaration
CNodeDebug gNodeDebug;

//=============================================
//
//
//=============================================
void Cmd_ClearDebug( void )
{
	gNodeDebug.ClearGame();
}

//=============================================
//
//
//=============================================
CNodeDebug::CNodeDebug( void ):
	m_pCvarDrawDebug(nullptr)
{
}

//=============================================
//
//
//=============================================
CNodeDebug::~CNodeDebug( void )
{
}

//=============================================
//
//
//=============================================
bool CNodeDebug::Init( void )
{
	m_pCvarDrawDebug = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_NONE, NODE_DEBUG_CVAR_NAME, "0", "For debugging info_nodes and npc navigation.");

	cl_engfuncs.pfnCreateCommand("clearnodedebug", Cmd_ClearDebug, "Clears node graph debug information");

	return true;
}

//=============================================
//
//
//=============================================
void CNodeDebug::Shutdown( void )
{
	ClearGame();
}

//=============================================
//
//
//=============================================
bool CNodeDebug::InitGame( void )
{
	return true;
}

//=============================================
//
//
//=============================================
void CNodeDebug::ClearGame( void )
{
	if(!m_routesArray.empty())
	{
		for(Uint32 i = 0; i < m_routesArray.size(); i++)
			delete m_routesArray[i];

		m_routesArray.clear();
	}

	if(!m_linksArray.empty())
		m_linksArray.clear();

	if(!m_bboxesArray.empty())
		m_bboxesArray.clear();
}

//=============================================
//
//
//=============================================
bool CNodeDebug::Draw( void )
{
	if(m_pCvarDrawDebug->GetValue() < 1)
		return true;

	// Only render if we have anything to draw
	if(m_routesArray.empty() && m_linksArray.empty() && m_bboxesArray.empty())
		return true;

	// Enable basic draw
	if(!cl_renderfuncs.pfnEnableBasicDraw())
		return false;

	if(!cl_renderfuncs.pfnBasicDrawDisableTextures())
		return false;

	CMatrix& projection = cl_renderfuncs.pfnGetProjectionMatrix();
	CMatrix& modelview = cl_renderfuncs.pfnGetModelViewMatrix();

	cl_renderfuncs.pfnBasicDrawSetProjection(projection.GetMatrix());
	cl_renderfuncs.pfnBasicDrawSetModelView(modelview.GetMatrix());

	glLineWidth(4);
	glPointSize(16);

	// Draw links
	for(Uint32 i = 0; i < m_linksArray.size(); i++)
	{
		node_debug_link_t& link = m_linksArray[i];

		// Set color
		cl_renderfuncs.pfnBasicDrawColor4f(link.color.x, link.color.y, link.color.z, 1.0);
		cl_renderfuncs.pfnValidateBasicDraw();

		cl_renderfuncs.pfnBasicDrawBegin(GL_LINES);
		cl_renderfuncs.pfnBasicDrawVertex3fv(link.node1);
		cl_renderfuncs.pfnBasicDrawVertex3fv(link.node2);
		cl_renderfuncs.pfnBasicDrawEnd();
	}

	// Draw routes
	if(m_pCvarDrawDebug->GetValue() >= 2 && !m_routesArray.empty())
	{
		if(m_pCvarDrawDebug->GetValue() == 3)
			glDisable(GL_DEPTH_TEST);

		for(Uint32 i = 0; i < m_routesArray.size(); i++)
		{
			node_debug_route_t* proute = m_routesArray[i];

			Vector raise, color;
			switch(proute->type)
			{
			case WAYPOINT_DETOUR:
				{
					raise = NODE_RAISE_DETOURS;
					color = NODE_COLOR_DETOURS;
				}
				break;
			case WAYPOINT_ERROR:
				{
					raise = NODE_RAISE_ERRORS;
					color = NODE_COLOR_ERRORS;
				}
				break;
			case WAYPOINT_NORMAL:
			default:
				{
					raise = NODE_RAISE_WAYPOINTS;
					color = NODE_COLOR_WAYPOINTS;
				}
				break;
			}

			// Set color
			cl_renderfuncs.pfnBasicDrawColor4f(color.x, color.y, color.z, 1.0);
			cl_renderfuncs.pfnValidateBasicDraw();

			// Draw the waypoint
			for(Uint32 j = 1; j < proute->points.size(); j++)
			{
				cl_renderfuncs.pfnBasicDrawBegin(GL_LINES);
				cl_renderfuncs.pfnBasicDrawVertex3fv(proute->points[j-1] + raise);
				cl_renderfuncs.pfnBasicDrawVertex3fv(proute->points[j] + raise);
				cl_renderfuncs.pfnBasicDrawEnd();
			}

			// Draw the points themselves
			for(Uint32 j = 0; j < proute->points.size(); j++)
			{
				cl_renderfuncs.pfnBasicDrawBegin(GL_POINTS);
				cl_renderfuncs.pfnBasicDrawVertex3fv(proute->points[j] + raise);
				cl_renderfuncs.pfnBasicDrawEnd();
			}
		}

		if(m_pCvarDrawDebug->GetValue() == 3)
			glEnable(GL_DEPTH_TEST);
	}

	// Draw bboxes
	if(m_pCvarDrawDebug->GetValue() >= 4 && !m_bboxesArray.empty())
	{
		glDisable( GL_CULL_FACE );
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

		for(Uint32 i = 0; i < m_bboxesArray.size(); i++)
		{
			node_debug_bbox_t *pbbox = &m_bboxesArray[i];

			cl_renderfuncs.pfnValidateBasicDraw();

			// Draw origin
			cl_renderfuncs.pfnBasicDrawColor4f(pbbox->color.x, pbbox->color.y, pbbox->color.z, 1.0);
			cl_renderfuncs.pfnBasicDrawBegin(GL_POINTS);
			cl_renderfuncs.pfnBasicDrawVertex3fv(pbbox->origin);
			cl_renderfuncs.pfnBasicDrawEnd();

			// Draw lines
			cl_renderfuncs.pfnBasicDrawBegin(GL_LINES);

			// Draw on X axis
			Vector lineBegin = Vector(pbbox->mins[0], pbbox->origin[1], pbbox->origin[2]);
			Vector lineEnd = Vector(pbbox->maxs[0], pbbox->origin[1], pbbox->origin[2]);
			cl_renderfuncs.pfnBasicDrawVertex3fv(lineBegin);
			cl_renderfuncs.pfnBasicDrawVertex3fv(lineEnd);

			// Draw on Y axis
			lineBegin = Vector(pbbox->origin[0], pbbox->mins[1], pbbox->origin[2]);
			lineEnd = Vector(pbbox->origin[0], pbbox->maxs[1], pbbox->origin[2]);
			cl_renderfuncs.pfnBasicDrawVertex3fv(lineBegin);
			cl_renderfuncs.pfnBasicDrawVertex3fv(lineEnd);

			// Draw on Y axis
			lineBegin = Vector(pbbox->origin[0], pbbox->origin[1], pbbox->mins[2]);
			lineEnd = Vector(pbbox->origin[0], pbbox->origin[1], pbbox->maxs[2]);
			cl_renderfuncs.pfnBasicDrawVertex3fv(lineBegin);
			cl_renderfuncs.pfnBasicDrawVertex3fv(lineEnd);
			cl_renderfuncs.pfnBasicDrawEnd();

			// Draw box
			Vector v[8];
			v[0][0] = pbbox->mins[0]; v[0][1] = pbbox->maxs[1]; v[0][2] = pbbox->mins[2];
			v[1][0] = pbbox->mins[0]; v[1][1] = pbbox->mins[1]; v[1][2] = pbbox->mins[2];
			v[2][0] = pbbox->maxs[0]; v[2][1] = pbbox->maxs[1]; v[2][2] = pbbox->mins[2];
			v[3][0] = pbbox->maxs[0]; v[3][1] = pbbox->mins[1]; v[3][2] = pbbox->mins[2];

			v[4][0] = pbbox->maxs[0]; v[4][1] = pbbox->maxs[1]; v[4][2] = pbbox->maxs[2];
			v[5][0] = pbbox->maxs[0]; v[5][1] = pbbox->mins[1]; v[5][2] = pbbox->maxs[2];
			v[6][0] = pbbox->mins[0]; v[6][1] = pbbox->maxs[1]; v[6][2] = pbbox->maxs[2];
			v[7][0] = pbbox->mins[0]; v[7][1] = pbbox->mins[1]; v[7][2] = pbbox->maxs[2];

			DrawBBox((*pbbox), v);
		}

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glEnable( GL_CULL_FACE );
	}

	cl_renderfuncs.pfnDisableBasicDraw();
	return true;
}

//=============================================
//
//
//=============================================
void CNodeDebug::AddBBox( const Vector& origin, const Vector& mins, const Vector& maxs )
{
	// Make sure this doesn't already exist
	for(Uint32 i = 0; i < m_bboxesArray.size(); i++)
	{
		if(m_bboxesArray[i].mins == mins && m_bboxesArray[i].maxs == maxs)
			return;
	}

	node_debug_bbox_t newbbox;
	newbbox.origin = origin;
	newbbox.mins = mins;
	newbbox.maxs = maxs;

	for(Uint32 i = 0; i < 3; i++)
		newbbox.color[i] = Common::RandomFloat(0, 1);

	m_bboxesArray.push_back(newbbox);
}

//=============================================
//
//
//=============================================
void CNodeDebug::AddPath( const Vector& node1, const Vector& node2, const Vector& color )
{
	Uint32 raiselevel = 0;
	for(Uint32 i = 0; i < m_linksArray.size(); i++)
	{
		if(m_linksArray[i].node1 == node1 && m_linksArray[i].node2 == node2
			|| m_linksArray[i].node2 == node1 && m_linksArray[i].node1 == node2)
		{
			if(color == m_linksArray[i].color)
				return;

			raiselevel++;
		}
	}

	node_debug_link_t newlink;
	newlink.color = color;
	newlink.node1 = node1+raiselevel*NODE_RAISE_AMOUNT;
	newlink.node2 = node2+raiselevel*NODE_RAISE_AMOUNT;

	m_linksArray.push_back(newlink);
}

//=============================================
//
//
//=============================================
void CNodeDebug::AddWaypoint( const CArray<Vector>& pointsArray, entindex_t entindex, waypoint_type_t type )
{
	node_debug_route_t* proute = nullptr;
	for(Uint32 i = 0; i < m_routesArray.size(); i++)
	{
		if(m_routesArray[i]->entindex == entindex && m_routesArray[i]->type == type)
		{
			proute = m_routesArray[i];
			break;
		}
	}

	if(!proute)
	{
		proute = new node_debug_route_t;
		m_routesArray.push_back(proute);
	}
	else if(!proute->points.empty())
	{
		// Clear previous
		proute->points.clear();
	}

	proute->entindex = entindex;
	proute->type = type;
	
	// Fill in the points
	proute->points.resize(pointsArray.size());
	for(Uint32 i = 0; i < pointsArray.size(); i++)
		proute->points[i] = pointsArray[i];
}

//=============================================
//
//
//=============================================
void CNodeDebug::DrawBBox( node_debug_bbox_t& bbox, Vector* pbboxpoints )
{
	cl_renderfuncs.pfnValidateBasicDraw();
	cl_renderfuncs.pfnBasicDrawBegin(GL_TRIANGLES);

	Vector triverts[3];
	for(Uint32 i = 0; i < 3; i++)
	{
		// Remember triverts
		triverts[i] = pbboxpoints[i&7];

		// Add to the draw list
		cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[i]);
	}

	for(Uint32 i = 3; i < 10; i++)
	{
		triverts[0] = triverts[1];
		triverts[1] = triverts[2];
		triverts[2] = pbboxpoints[i&7];

		for(Uint32 j = 0; j < 3; j++)
			cl_renderfuncs.pfnBasicDrawVertex3fv(triverts[j]);
	}

	cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[6]);
	cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[0]);
	cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[4]);

	cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[0]);
	cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[4]);
	cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[2]);

	cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[1]);
	cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[7]);
	cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[3]);

	cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[7]);
	cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[3]);
	cl_renderfuncs.pfnBasicDrawVertex3fv(pbboxpoints[5]);

	cl_renderfuncs.pfnBasicDrawEnd();
}