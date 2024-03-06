/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef AI_NODEGRAPH_H
#define AI_NODEGRAPH_H

#include "statestack.h"
#include "nodeignorelist.h"
#include "ai_basenpc.h"

class CBaseNPC;
class CBaseEntity;
class CEntityHandle;
class CNodeIgnoreList;
class CLogFile;

enum node_hull_types_t;

struct nodefilehdr_t;

// Delay until node compile begins
static const Float NODE_GRAPH_GENERATION_DELAY = 2.5f;

// Node hint types
enum node_hint_types_t
{
	NODE_HINT_NONE = 0,
	NODE_HINT_DOOR,
	NODE_HINT_WINDOW,
	NODE_HINT_BUTTON,
	NODE_HINT_MACHINERY,
	NODE_HINT_LEDGE,
	NODE_HINT_LIGHT_SOURCE,
	NODE_HINT_HEAT_SOURCE,
	NODE_HINT_BLINKING_LIGHT,
	NODE_HINT_BRIGHT_COLORS,
	NODE_HINT_BLOOD,

	NODE_HINT_TACTICAL_EXIT = 100,
	NODE_HINT_VANTAGE,
	NODE_HINT_AMBUSH,

	NODE_HINT_SITTING_SPOT = 400,
	NODE_HINT_WINDOW_SPOT
};

/*
=======================
CAINodeGraph

=======================
*/
class CAINodeGraph
{
public:
	// Node file ID
	static const Int32 NODE_FILE_ID;
	// Node file version
	static const Int32 NODE_FILE_VERSION;

	// Node file extension
	static const Char NODE_FILE_EXTENSION[];
	// Report file extension
	static const Char REPORT_FILE_EXTENSION[];

	// Small test hull min size
	static const Vector HULL_SMALL_MIN;
	// Small test hull max size
	static const Vector HULL_SMALL_MAX;

	// Large test hull min size
	static const Vector HULL_LARGE_MIN;
	// Large test hull max size
	static const Vector HULL_LARGE_MAX;

	// Empty hash link value
	static const Int16 EMPTY_HASH_LINK_VALUE;
	// Link search radius
	static const Float LINK_SEARCH_RADIUS;

	// Node files folder path
	static const Char NODE_FILES_FOLDER_PATH[];

	// Node peek offset
	static const Vector NODE_PEEK_OFFSET;
	// Unstick max distance
	static const Float MAX_UNSTICK_DISTANCE;

public:
	// Number of prime values
	static const Uint32 NUM_PRIME_VALUES = 177;
	// Array of prime values
	static const Int32 PRIME_VALUES[NUM_PRIME_VALUES];
	// Number of hash primes
	static const Uint32 NUM_HASH_PRIMES = 16;
	// Maximum nodes in a level
	static const Uint32 MAX_MAP_NODES = 4096;
	// Bitset array size for MAX_MAP_NODES
	static const Uint32 NB_INTS_FOR_ALL_NODES = (MAX_MAP_NODES/32);
	// Cache size
	static const Uint32 CACHE_SIZE = 512;

public:
	// Max saved string length
	static const Uint32 MAX_STRING_LENGTH = 32;

public:
	enum linktypes_t
	{
		LINK_SMALL_HULL	 = (1<<0),
		LINK_HUMAN_HULL	 = (1<<1),
		LINK_LARGE_HULL	 = (1<<2),
		LINK_FLY_HULL	 = (1<<3),
		LINK_DISABLED	 = (1<<4)
	};

	enum typesflags_t
	{
		AI_NODE_LAND		= (1<<0),
		AI_NODE_AIR			= (1<<1),
		AI_NODE_WATER		= (1<<2),
		AI_NODE_AHEAD		= (1<<3),
		AI_NODE_GROUP_REALM = (AI_NODE_AIR|AI_NODE_LAND|AI_NODE_WATER)
	};

	enum capability_indexes_t
	{
		NODE_CAP_NONE = 0,
		NODE_CAP_DOORS_AND_USE,

		NB_CAP_INDEXES
	};

	enum nodeflags_t
	{
		NODE_FL_NONE		= 0,
		NODE_FL_NO_OPTIMIZE	= (1<<0),
		NODE_FL_SHIFTED		= (1<<1)
	};

	enum shift_node_t
	{
		SHIFT_BOTH = 0,
		SHIFT_FROM,
		SHIFT_TO
	};

	enum shift_direction_t{
		SHIFT_NONE = 0,
		SHIFT_POS,
		SHIFT_NEG
	};

public:
	struct nodepair_t
	{
		nodepair_t(short dstIndex, short srcNode):
			dstNode(dstIndex),
			srcNode(srcNode)
			{}

		Int16 dstNode;
		Int16 srcNode;
	};

	struct link_entity_t
	{
		link_entity_t():
			entityindex(NO_ENTITY_INDEX),
			hulltype(NODE_SMALL_HULL)
			{
				memset(modelname, 0, sizeof(modelname));
			}

		Int32 entityindex;
		Char modelname[MAX_STRING_LENGTH];
		node_hull_types_t hulltype;
	};
	
	struct node_link_t
	{
		node_link_t():
			sourcenode(NO_POSITION),
			destnode(NO_POSITION),
			linkentityindex(0),
			numlinkentities(0),
			linkinfobits(0),
			weight(0)
			{}

		Int32 sourcenode;
		Int32 destnode;

		Int32 linkentityindex;
		Uint32 numlinkentities;

		Uint64 linkinfobits;
		Float weight;
	};

	struct node_t
	{
		node_t():
			index(0),
			origindex(0),
			numlinks(0),
			firstlinkindex(NO_POSITION),
			nodeinfobits(0),
			closestsofar(0),
			previousnode(NO_POSITION),
			linkrange(0),
			hinttype(0),
			hintactivity(0),
			hintyaw(0),
			nodeflags(0)
			{
				memset(nextbestnode, 0, sizeof(nextbestnode));
				memset(noderegionname, 0, sizeof(noderegionname));
			}

		Int32 index;
		Int32 origindex;
		Vector origin;

		Vector mins[NUM_NODE_HULLS];
		Vector maxs[NUM_NODE_HULLS];

		Uint32 numlinks;
		Int32 firstlinkindex;

		Uint64 nodeinfobits;

		Int32 nextbestnode[NUM_NODE_HULLS][NB_CAP_INDEXES];

		Float closestsofar;
		Int32 previousnode;

		Float linkrange;

		Int32 hinttype;
		Int32 hintactivity;
		Float hintyaw;

		Uint64 nodeflags;

		Char noderegionname[MAX_STRING_LENGTH];
	};

	struct cache_entry_t
	{
		cache_entry_t():
			node(0)
			{}

		Vector position;
		Int16 node;
	};

	struct nodefilehdr_t
	{
		nodefilehdr_t():
			id(0),
			version(0),
			nodeindex(0),
			numnodes(0),
			linkoffset(0),
			numlinks(0),
			hashlinkoffset(0),
			numhashlinks(0),
			linkentityoffset(0),
			numlinkentities(0),
			routeinfoindex(0),
			routeinfosize(0),
			filesize(0)
			{
				memset(hashprimes, 0, sizeof(hashprimes));
			}

		Int32 id;
		Int32 version;

		Int32 nodeindex;
		Uint32 numnodes;

		Int32 linkoffset;
		Uint32 numlinks;

		Int32 hashlinkoffset;
		Uint32 numhashlinks;

		Int32 linkentityoffset;
		Uint32 numlinkentities;

		Int32 routeinfoindex;
		Uint32 routeinfosize;

		Int32 hashprimes[NUM_HASH_PRIMES];

		Uint32 filesize;

		node_t* getNode( Int32 nodeIndex )
		{
			if(nodeIndex < 0 || nodeIndex >= (Int32)this->numnodes)
				return nullptr;
			else
				return reinterpret_cast<node_t*>(reinterpret_cast<byte*>(this) + this->nodeindex) + nodeIndex;
		}

		node_link_t* getLink( Int32 linkIndex )
		{
			if(linkIndex < 0 || linkIndex >= (Int32)this->numlinks)
				return nullptr;
			else
				return reinterpret_cast<node_link_t*>(reinterpret_cast<byte*>(this) + this->linkoffset) + linkIndex;
		}

		link_entity_t* getLinkEntity( Int32 linkIndex, Int32 linkEntityIndex )
		{
			node_link_t* plink = getLink(linkIndex);
			if(!plink)
				return nullptr;
			
			if(linkEntityIndex < 0 || linkEntityIndex >= (Int32)plink->numlinkentities)
				return nullptr;

			return reinterpret_cast<link_entity_t*>(reinterpret_cast<byte*>(this) + this->linkentityoffset) + plink->linkentityindex + linkEntityIndex;
		}

		link_entity_t* getLinkEntity( Int32 linkEntityIndex )
		{
			if(linkEntityIndex < 0 || linkEntityIndex >= (Int32)this->numlinkentities)
				return nullptr;

			return reinterpret_cast<link_entity_t*>(reinterpret_cast<byte*>(this) + this->linkentityoffset) + linkEntityIndex;
		}

		node_link_t* getNodeLink( Int32 nodeIndex, Int32 linkIndex )
		{
			if(nodeIndex < 0 || nodeIndex >= (Int32)this->numnodes)
				return nullptr;

			node_t* pnode = getNode(nodeIndex);
			if(!pnode)
				return nullptr;

			if(linkIndex < 0 || linkIndex >= (Int32)pnode->numlinks)
				return nullptr;

			return reinterpret_cast<node_link_t*>(reinterpret_cast<byte*>(this) + this->linkoffset) + pnode->firstlinkindex + linkIndex;
		}

		node_link_t* getNodeLink( node_t* pnode, Int32 linkIndex )
		{
			if(!pnode)
				return nullptr;

			if(linkIndex < 0 || linkIndex >= (Int32)pnode->numlinks)
				return nullptr;

			return reinterpret_cast<node_link_t*>(reinterpret_cast<byte*>(this) + this->linkoffset) + pnode->firstlinkindex + linkIndex;
		}

		const node_link_t* getNodeLink( const node_t* pnode, Int32 linkIndex ) const
		{
			if(!pnode)
				return nullptr;

			if(linkIndex < 0 || linkIndex >= (Int32)pnode->numlinks)
				return nullptr;

			return reinterpret_cast<const node_link_t*>(reinterpret_cast<const byte*>(this) + this->linkoffset) + pnode->firstlinkindex + linkIndex;
		}

		node_t* getDestNodeFromLink( Int32 nodeIndex, Int32 linkIndex )
		{
			node_t* pnode = getNode(nodeIndex);
			if(!pnode)
				return nullptr;

			node_link_t* plink = getNodeLink(pnode, linkIndex);
			if(!plink)
				return nullptr;

			return getNode(plink->destnode);
		}
	};

	enum node_querytype_t
	{
		GRAPH_QUERY_STATIC = 0,
		GRAPH_QUERY_DYNAMIC
	};

public:
	CAINodeGraph( void );
	~CAINodeGraph( void );

public:
	void InitNodeGraph( void );
	void ClearNodeGraph( void );
	void ClearTempArrays( bool clearNodes = true );
	void ShowNodeBBoxes( node_hull_types_t hullType );
	void ShowNodeConnections( Int32 nodeIndex );
	void ShowAllNodeConnections( void );

	bool LoadNodeGraph( void );
	bool IsNodeGraphValid( void ) const;

	Int32 GetNumNodes( void ) const;
	Int32 GetNumTempNodes( void ) const;

	static capability_indexes_t GetCapabilityIndex( Uint64 capabilityBits );

public:
	bool HandleLinkEntity( Int32 nodeIndex, Int32 entityIndex, Uint64 capabilityBits, node_querytype_t queryType, const CBaseEntity* pNPC ) const;

	const node_t* GetNode( Int32 nodeIndex );
	const node_link_t* GetNodeLink( Int32 nodeIndex, Int32 linkIndex );
	const node_link_t* GetNodeLink( const node_t* pnode, Int32 linkIndex ) const;
	bool GetNodeLinkEntities( Int32 srcNode, Int32 linkNodeIndex, link_entity_t*& pLinkPtr, Uint32& numLinks );

	Int32 GetNextNodeInRoute( Int32 currentNode, Int32 destNode, node_hull_types_t hullType, capability_indexes_t capabilityIndex );
	Int32 GetNearestNode( const Vector& position, CBaseEntity* pEntity, const CBaseEntity* pTargetEntity = nullptr, Float minDistance = -1, const CNodeIgnoreList* pIgnoreList = nullptr );
	Int32 GetNearestNode( const Vector& position, Uint64 nodeTypes, CBaseEntity* pEntity, const CBaseEntity* pTargetEntity = nullptr, Float minDistance = -1, const CNodeIgnoreList* pIgnoreList = nullptr );
	Int32 GetNearestNode( const Vector& position );
	Int32 GetShortestPath( Int32 startNode, Int32 endNode, node_hull_types_t hullType, Uint64 capabilityBits, Int32 *pNodeIndexArray, const CBaseEntity* pNPC = nullptr, const CBaseEntity* pTargetEntity = nullptr, const CNodeIgnoreList* pIgnoreList = nullptr, const CBaseEntity* pViewCheckNPC = nullptr );
	Float GetPathLength( Int32 startNode, Int32 endNode, node_hull_types_t hullType, Uint64 capabilityMask );
	bool IsValidCoverPath( Int32 startNode, Int32 endNode, node_hull_types_t hullType, Uint64 capabilityMask, const CBaseEntity* pNPC, const CBaseEntity* pThreatEntity );
	void ShowNearestNodeBBox( const Vector& position, node_hull_types_t hullType );

public:
	void AddNode( const Vector& origin, Float hintYaw, Int32 hintType, Int32 hintActivity, Float range, const Char* pstrRegionName, bool isAirNode, bool disableOptimization );

private:
	void SetGraphPointers( void );
	bool LinkNodes( CBaseEntity* pTestNPC );
	bool WalkLinksAllHulls( CBaseEntity* pTestNPC );
	void OptimizeGraph( void );
	void SortNodes( void );
	void BuildLinkLookups( void );
	void SetNodeMinsMaxs( CBaseEntity* pTestNPC );
	void BuildStaticRoutingTables( Int16*& pRouteInfo, Uint32& routeInfoSize );
	static bool CheckNodeBBox( node_t* pnode, const Vector& origin, CBaseEntity* pNPC );

	void WriteToReportFile( const Char* pstrString );

	void ChooseHashPrimes( Uint32 tableSize );
	void InsertLinkHash( Int16 srcNode, Int16 dstNode, Int32 key ) const;
	Int32 SearchHash( Int16 srcNode, Int16 dstNode );
	static Uint32 GenHash( const byte* pData, Uint32 length );

	void CheckNode( const Vector& origin, Int32 nodeIndex, Uint64 nodeTypes, CBaseEntity* pEntity, const CBaseEntity* pTargetEntity, Float minDistance );
	static bool WalkPath( const Vector& startPosition, const Vector& endPosition, CBaseEntity* pTestNPC, CArray<CBaseEntity*>& linkEntityArray, bool isWaterNode = false );
	static bool IsLinkEntityManaged( CBaseEntity* pEntity );

	static Uint64 GetMaskForHull( node_hull_types_t type );

private:
	// Array of nodes at link time
	CArray<node_t*> m_nodesArray;
	// Array of node links at link time
	CArray<CArray<node_link_t*>> m_nodeLinksArrayOfArrays;
	// Final array of links at link time
	CArray<node_link_t*> m_finalNodeLinksArray;

	// Array of link entities at link time
	CArray<CArray<link_entity_t*>> m_linkEntityArrayOfArrays;
	// Final array of link entities at link time
	CArray<link_entity_t*> m_finalLinkEntitiesArray;

private:
	// Node file header
	nodefilehdr_t* m_pNodeHeader;

	// Nearest distance to a node
	Float m_nearestNodeDistance;
	// Nearest node index
	Int32 m_nearestNodeIndex;

	// Cache for nodes
	cache_entry_t m_cache[CACHE_SIZE];

	// Controls whether debug messages should be printed
	bool m_disableDebugMessages;

	// Log file
	CLogFile* m_pLogFile;

private:
	// Buffer to hold text written to file
	CString m_logFileName;
};
extern CAINodeGraph gNodeGraph;
#endif //AI_NODEGRAPH_H