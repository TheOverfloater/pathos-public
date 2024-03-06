/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef NODEDEBUG_H
#define NODEDEBUG_H

class CCVar;

struct node_debug_link_t
{
	Vector node1;
	Vector node2;
	Vector color;
};

struct node_debug_bbox_t
{
	Vector origin;
	Vector mins;
	Vector maxs;
	Vector color;
};

struct node_debug_route_t
{
	node_debug_route_t():
		entindex(0),
		type(WAYPOINT_NORMAL)
		{}

	entindex_t entindex;
	waypoint_type_t type;

	CArray<Vector> points;
};

/*
====================
CNodeDebug

====================
*/
class CNodeDebug
{
public:
	// Node raise amount
	static const Vector NODE_RAISE_AMOUNT;
	// Node raise for waypoints
	static const Vector NODE_RAISE_WAYPOINTS;
	// Node color for waypoints
	static const Vector NODE_COLOR_WAYPOINTS;
	// Node raise for detours
	static const Vector NODE_RAISE_DETOURS;
	// Node color for detours
	static const Vector NODE_COLOR_DETOURS;
	// Node raise for errors
	static const Vector NODE_RAISE_ERRORS;
	// Node color for errors
	static const Vector NODE_COLOR_ERRORS;

public:
	CNodeDebug( void );
	~CNodeDebug( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes for game
	bool InitGame( void );
	// Clears game
	void ClearGame( void );

	// Draws debug objects
	bool Draw( void );

public:
	// Adds a bbox object
	void AddBBox( const Vector& origin, const Vector& mins, const Vector& maxs );
	// Adds a path object
	void AddPath( const Vector& node1, const Vector& node2, const Vector& color );
	// Adds a waypoint object
	void AddWaypoint( const CArray<Vector>& pointsArray, entindex_t entindex, waypoint_type_t type );

private:
	// Draws a bbox
	static void DrawBBox( node_debug_bbox_t& bbox, Vector* pbboxpoints );

private:
	// Array of debug routes
	CArray<node_debug_route_t*> m_routesArray;
	// Array of links
	CArray<node_debug_link_t> m_linksArray;
	// Array of bboxes
	CArray<node_debug_bbox_t> m_bboxesArray;

	// Cvar for rendering
	CCVar* m_pCvarDrawDebug;
};
extern CNodeDebug gNodeDebug;
#endif //NODEDEBUG_H