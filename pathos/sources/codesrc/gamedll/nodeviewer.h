/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef NODEVIEWER_H
#define NODEVIEWER_H

//=============================================
//
//=============================================
class CNodeViewer : public CBaseEntity
{
public:
	// Color for fly hull
	static const Vector COLOR_FLY_HULL;
	// Color for large hull
	static const Vector COLOR_LARGE_HULL;
	// Color for small hull
	static const Vector COLOR_SMALL_HULL;
	// Color for human hull
	static const Vector COLOR_HUMAN_HULL;

public:
	// Max nodes
	static const Uint32 MAX_NODES = 128;

public:
	explicit CNodeViewer( edict_t* pedict );
	virtual ~CNodeViewer( void );

public:
	virtual bool Spawn( void ) override;

public:
	void FindNodeConnections( Int32 nodeIndex );
	void AddNode( Int32 fromNode, Int32 toNode );
	void EXPORTFN DrawThink( void );

private:
	Int32 m_baseNode;
	Uint32 m_numVisited;
	Int32 m_fromNodes[MAX_NODES];
	Int32 m_toNodes[MAX_NODES];

	Int32 m_hullType;
	Uint64 m_nodeType;
	Vector m_color;
};

#endif //NODEVIEWER_H