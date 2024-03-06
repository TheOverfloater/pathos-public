/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "ai_nodegraph.h"
#include "npctesthull.h"
#include "constants.h"
#include "crc32.h"
#include "funcdoor.h"
#include "funcdoorrotating.h"
#include "funcwalltoggle.h"
#include "functrain.h"
#include "nodequeue.h"
#include "prioritynodequeue.h"
#include "nodestack.h"
#include "doorstatestack.h"
#include "logfile.h"

#define NODE_FROM_TO(x, y, z) ((x)*(z)+(y))

// Node file ID
const Int32 CAINodeGraph::NODE_FILE_ID = (('1'<<24)+('D'<<16)+('O'<<8)+'N');
// Node file version
const Int32 CAINodeGraph::NODE_FILE_VERSION = 3;
// Node file extension
const Char CAINodeGraph::NODE_FILE_EXTENSION[] = ".nod";
// Report file extension
const Char CAINodeGraph::REPORT_FILE_EXTENSION[] = ".log";

// Small test hull min size
const Vector CAINodeGraph::HULL_SMALL_MIN = Vector(-16, -16, 0);
// Small test hull max size
const Vector CAINodeGraph::HULL_SMALL_MAX = Vector(16, 16, 36);

// Large test hull min size
const Vector CAINodeGraph::HULL_LARGE_MIN = Vector(-12, -12, 0);
// Large test hull max size
const Vector CAINodeGraph::HULL_LARGE_MAX = Vector(12, 12, 24);

// Empty hash link value
const Int16 CAINodeGraph::EMPTY_HASH_LINK_VALUE = -1;
// Link search radius
const Float CAINodeGraph::LINK_SEARCH_RADIUS = 128;

// Node peek offset
const Vector CAINodeGraph::NODE_PEEK_OFFSET = Vector(0, 0, 8);
// Unstick max distance
const Float CAINodeGraph::MAX_UNSTICK_DISTANCE = 16.0f;

// Node files folder path
const Char CAINodeGraph::NODE_FILES_FOLDER_PATH[] = "maps/graphs";

// Array of prime values
const Int32 CAINodeGraph::PRIME_VALUES[NUM_PRIME_VALUES] = {
	1, 2, 3, 5, 7, 11, 13, 17, 19, 
	23, 29, 31, 37, 41, 43, 47, 53, 
	59, 61, 67, 71, 73, 79, 83, 89, 
	97, 101, 103, 107, 109, 113, 127, 
	131, 137, 139, 149, 151, 157, 163, 
	167, 173, 179, 181, 191, 193, 197, 
	199, 211, 223, 227, 229, 233, 239,
	241, 251, 257, 263, 269, 271, 277, 
	281, 283, 293, 307, 311, 313, 317, 
	331, 337, 347, 349, 353, 359, 367, 
	373, 379, 383, 389, 397, 401, 409, 
	419, 421, 431, 433, 439, 443, 449, 
	457, 461, 463, 467, 479, 487, 491, 
	499, 503, 509, 521, 523, 541, 547, 
	557, 563, 569, 571, 577, 587, 593, 
	599, 601, 607, 613, 617, 619, 631, 
	641, 643, 647, 653, 659, 661, 673, 
	677, 683, 691, 701, 709, 719, 727, 
	733, 739, 743, 751, 757, 761, 769, 
	773, 787, 797, 809, 811, 821, 823, 
	827, 829, 839, 853, 857, 859, 863, 
	877, 881, 883, 887, 907, 911, 919, 
	929, 937, 941, 947, 953, 967, 971,
	977, 983, 991, 997, 1009, 1013, 1019, 
	1021, 1031, 1033, 1039, 0
};

// Object definition
CAINodeGraph gNodeGraph;

//=============================================
// @brief
//
//=============================================
CAINodeGraph::CAINodeGraph( void ):
	m_pNodeHeader(nullptr),
	m_nearestNodeDistance(0),
	m_nearestNodeIndex(NO_POSITION),
	m_disableDebugMessages(false)
{
}

//=============================================
// @brief
//
//=============================================
CAINodeGraph::~CAINodeGraph( void )
{
	ClearNodeGraph();
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::ClearNodeGraph( void )
{
	if(!m_pNodeHeader)
		return;

	delete[] reinterpret_cast<byte*>(m_pNodeHeader);
	m_pNodeHeader = nullptr;
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::ClearTempArrays( bool clearNodes )
{
	if(clearNodes && !m_nodesArray.empty())
	{
		for(Uint32 i = 0; i < m_nodesArray.size(); i++)
			delete m_nodesArray[i];

		m_nodesArray.clear();
	}

	if(!m_nodeLinksArrayOfArrays.empty())
	{
		for(Uint32 i = 0; i < m_nodeLinksArrayOfArrays.size(); i++)
		{
			for(Uint32 j = 0; j < m_nodeLinksArrayOfArrays[i].size(); j++)
				delete m_nodeLinksArrayOfArrays[i][j];

			m_nodeLinksArrayOfArrays[i].clear();
		}

		m_nodeLinksArrayOfArrays.clear();
	}

	if(!m_finalNodeLinksArray.empty())
	{
		for(Uint32 i = 0; i < m_finalNodeLinksArray.size(); i++)
			delete m_finalNodeLinksArray[i];

		m_finalNodeLinksArray.clear();
	}

	if(!m_linkEntityArrayOfArrays.empty())
	{
		for(Uint32 i = 0; i < m_linkEntityArrayOfArrays.size(); i++)
		{
			for(Uint32 j = 0; j < m_linkEntityArrayOfArrays[i].size(); i++)
				delete m_linkEntityArrayOfArrays[i][j];

			m_linkEntityArrayOfArrays[i].clear();
		}

		m_linkEntityArrayOfArrays.clear();
	}

	if(!m_finalLinkEntitiesArray.empty())
	{
		for(Uint32 i = 0; i < m_finalLinkEntitiesArray.size(); i++)
			delete m_finalLinkEntitiesArray[i];

		m_finalLinkEntitiesArray.clear();
	}
}

//=============================================
// @brief
//
//=============================================
bool CAINodeGraph::LoadNodeGraph( void )
{
	// Get the file name from the world model
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(WORLD_MODEL_INDEX);
	assert(pmodel != nullptr);

	file_dateinfo_t bspFileDate;
	if(!gd_filefuncs.pfnGetFileDate(pmodel->name.c_str(), bspFileDate))
		return false;

	// Get file date for the NOD file
	CString basename;
	Common::Basename(pmodel->name.c_str(), basename);

	CString filePath;
	filePath << NODE_FILES_FOLDER_PATH << PATH_SLASH_CHAR << basename << NODE_FILE_EXTENSION;

	file_dateinfo_t nodFileDate;
	if(!gd_filefuncs.pfnGetFileDate(filePath.c_str(), nodFileDate))
		return false;

	if(gd_filefuncs.pfnCompareFileDates(bspFileDate, nodFileDate) < 0)
		return false;

	Uint32 fileSize = 0;
	const byte* pfile = gd_filefuncs.pfnLoadFile(filePath.c_str(), &fileSize);
	if(!pfile)
		return false;

	const nodefilehdr_t* phdr = reinterpret_cast<const nodefilehdr_t*>(pfile);
	if(phdr->id != NODE_FILE_ID)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid nod file '%s'.\n", __FUNCTION__, filePath.c_str());
		gd_filefuncs.pfnFreeFile(pfile);
		return false;
	}

	if(phdr->version != NODE_FILE_VERSION)
	{
		gd_engfuncs.pfnCon_Printf("%s - Node graph file '%s' has wrong version(%d instead of %d).\n", __FUNCTION__, filePath.c_str(), phdr->version, NODE_FILE_VERSION);
		gd_filefuncs.pfnFreeFile(pfile);
		return false;
	}

	byte* pbuffer = new byte[fileSize];
	memcpy(pbuffer, pfile, sizeof(byte)*fileSize);
	gd_filefuncs.pfnFreeFile(pfile);

	m_pNodeHeader = reinterpret_cast<nodefilehdr_t*>(pbuffer);

	// Set the graph pointers
	SetGraphPointers();

	// Clear temp nodes
	if(!m_nodesArray.empty())
	{
		for(Uint32 i = 0; i < m_nodesArray.size(); i++)
			delete m_nodesArray[i];

		m_nodesArray.clear();
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::SetGraphPointers( void )
{
	for(Uint32 i = 0; i < m_pNodeHeader->numlinkentities; i++)
	{
		link_entity_t* pLinkEntity = m_pNodeHeader->getLinkEntity(i);
		if(!qstrlen(pLinkEntity->modelname))
		{
			pLinkEntity->entityindex = NO_ENTITY_INDEX;
			continue;
		}

		// Find the entity with this model index
		Int32 j = 0;
		for(; j < g_pGameVars->numentities; j++)
		{
			edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(j);
			if(!pedict || Util::IsNullEntity(pedict))
				continue;

			if(!pedict->state.modelindex)
				continue;

			const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(pedict->state.modelindex);
			if(!pmodel)
				continue;

			if(!qstrcmp(pmodel->name, pLinkEntity->modelname))
			{
				pLinkEntity->entityindex = pedict->entindex;
				pedict->state.flags |= FL_GRAPH_ENTITY;
				break;
			}
		}

		// If we didn't find anything, disable this link entity object
		if(j == g_pGameVars->numentities)
		{
			gd_engfuncs.pfnCon_DPrintf("%s - Couldn't find entity with model '%s'.\n", __FUNCTION__, pLinkEntity->modelname);
			pLinkEntity->entityindex = NO_ENTITY_INDEX;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::InitNodeGraph( void )
{
	// Make sure these are cleared
	ClearTempArrays(false);
	ClearNodeGraph();

	if(m_nodesArray.empty())
		return;

	// Mark begin time
	Double beginTime = gd_engfuncs.pfnFloatTime();

	// Try and create the node file folder
	if(!gd_filefuncs.pfnCreateDirectory(NODE_FILES_FOLDER_PATH))
	{
		gd_engfuncs.pfnCon_EPrintf("%s - Failed to create directory '%s'.\n", __FUNCTION__, NODE_FILES_FOLDER_PATH);
		return;
	}

	// Set the log file name
	m_logFileName.clear();
	m_logFileName << NODE_FILES_FOLDER_PATH << PATH_SLASH_CHAR << g_pGameVars->levelname << REPORT_FILE_EXTENSION;

	// Create the folder for the node files
	if(!gd_filefuncs.pfnCreateDirectory(NODE_FILES_FOLDER_PATH))
	{
		gd_engfuncs.pfnCon_Printf("%s - Failed to create folder '%s'.\n", __FUNCTION__, NODE_FILES_FOLDER_PATH);
		return;
	}

	// Delete previous log
	if(gd_filefuncs.pfnFileExists(m_logFileName.c_str()))
		gd_filefuncs.pfnDeleteFile(m_logFileName.c_str());

	// Create new log file
	m_pLogFile = new CLogFile(m_logFileName.c_str(), gd_engfuncs.pfnCon_Printf, gd_filefuncs);
	if(!m_pLogFile->Init())
	{
		gd_engfuncs.pfnCon_Printf("%s - Failed to init log file '%s'.\n", __FUNCTION__, m_logFileName.c_str());
		return;
	}

	// Delete existing file
	CString filestr;
	filestr << "=============================================" << NEWLINE;
	filestr << " Node graph file begin" << NEWLINE;
	filestr << "=============================================" << NEWLINE;
	filestr << "\t- Level name: " << g_pGameVars->levelname << NEWLINE;
	filestr << "\t- Number of nodes: " << (Int32)m_nodesArray.size() << NEWLINE << NEWLINE;

	filestr << "Initial node array contents:" << NEWLINE;
	WriteToReportFile(filestr.c_str());
	filestr.clear();

	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
	{
		node_t* pNode = m_nodesArray[i];

		filestr << " - Node " << (Int32)i << NEWLINE;
		filestr << "\tOrigin: " << pNode->origin[0] << " " << pNode->origin[1] << " " << pNode->origin[2] << NEWLINE;
		filestr << "\tHint type: " << pNode->hinttype << NEWLINE;
		filestr << "\tHint yaw: " << pNode->hintyaw << NEWLINE;

		if(pNode->linkrange)
			filestr << "\tLink range: " << pNode->linkrange << NEWLINE;

		if(qstrlen(pNode->noderegionname) > 0)
			filestr << "\tRegion name: " << pNode->noderegionname << NEWLINE << NEWLINE;

		WriteToReportFile(filestr.c_str());
		filestr.clear();
	}

	filestr << NEWLINE << NEWLINE;

	// Find every single NPC and disable solidity on them
	CEntityStateStack npcStack;

	for(Int32 i = 1; i < g_pGameVars->numentities; i++)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
		if(!pedict)
			break;

		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pentity = CBaseEntity::GetClass(pedict);
		if(!pentity)
			continue;

		if(pentity->IsPlayer() || pentity->IsNPC())
			npcStack.SaveEntity(pentity);
	}

	// Create the node hull npc
	CBaseEntity* pTestNPCEntity = CBaseEntity::CreateEntity("npc_testhull", nullptr);
	if(!pTestNPCEntity || !pTestNPCEntity->Spawn())
	{
		WriteToReportFile("Failed to create test hull npc!\n");
		ClearTempArrays();
		return;
	}

	// Identify which nodes are water, land, etc
	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
	{
		node_t* pNode = m_nodesArray[i];

		if(pNode->nodeinfobits & AI_NODE_AIR)
			continue;

		if(gd_tracefuncs.pfnPointContents(pNode->origin, nullptr) == CONTENTS_WATER)
		{
			// Mark this as water
			pNode->nodeinfobits |= AI_NODE_WATER;
		}
		else
		{
			// This node is on land
			pNode->nodeinfobits |= AI_NODE_LAND;

			// Offset by hull mins
			Vector initialPosition = pNode->origin + Vector(0, 0, 18);

			// Unstick node's position if small hull is inside a solid
			trace_t tr;
			Util::TraceHull(initialPosition, initialPosition, true, false, HULL_SMALL, nullptr, tr);
			if(!tr.noHit() || tr.startSolid() || tr.allSolid())
			{
				bool wasUnstuck = false;

				for(Float distance = 0.1; distance < MAX_UNSTICK_DISTANCE; distance += 0.1f)
				{
					Vector testPosition = initialPosition + Vector(0, 0, distance);
					Util::TraceHull(testPosition, testPosition, true, false, HULL_SMALL, nullptr, tr);
					if(tr.noHit() && !tr.allSolid() && !tr.startSolid())
					{
						pNode->origin = tr.endpos;
						wasUnstuck = true;
						break;
					}

					testPosition = initialPosition - Vector(0, 0, distance);
					Util::TraceHull(testPosition, testPosition, true, false, HULL_SMALL, nullptr, tr);
					if(tr.noHit() && !tr.allSolid() && !tr.startSolid())
					{
						pNode->origin = tr.endpos;
						wasUnstuck = true;
						break;
					}
				}

				// Restore the normal position
				Math::VectorSubtract(pNode->origin, Vector(0, 0, 18), pNode->origin);

				if(!wasUnstuck)
				{
					filestr << "Node " << i << " at  " << pNode->origin[0] 
						<< " " << pNode->origin[1] << " " 
						<< pNode->origin[2] << " was not unstuck";

					CBaseEntity* pEntity = Util::GetEntityFromTrace(tr);
					if(pEntity)
					{
						filestr << " - Stuck inside solid entity " << pEntity->GetClassName() 
							<< "(entindex " << pEntity->GetEntityIndex() 
							<< ") with model " << pEntity->GetModelName();

						if(pEntity->HasTargetName())
							filestr << " and targetname " << pEntity->GetTargetName();
					}
					filestr << "\n";
						
					WriteToReportFile(filestr.c_str());
					filestr.clear();

					// Erase from array
					m_nodesArray.erase(i);
					i--;
					continue;
				}
				else
				{
					filestr << "Node " << i << " at  " << pNode->origin[0] << " " << pNode->origin[1] << " " << pNode->origin[2] << " was successfully unstuck\n";
					WriteToReportFile(filestr.c_str());
					filestr.clear();
				}
			}

			// Drop this to the ground
			initialPosition = pNode->origin + Vector(0, 0, 18);
			Vector traceEndPosition = pNode->origin - Vector(0, 0, 512);
			Util::TraceHull(initialPosition, traceEndPosition, true, false, HULL_SMALL, nullptr, tr);

			// Mark original index
			pNode->origindex = i;

			if(!tr.noHit())
			{
				pNode->origin = tr.endpos - Vector(0, 0, 18);
				pTestNPCEntity->SetOrigin(pNode->origin);
				pTestNPCEntity->DropToFloor();

				Uint32 j = 0;
				for(; j < NUM_NODE_HULLS; j++)
				{
					CString hullName;
					switch(j)
					{
					case NODE_SMALL_HULL:
						pTestNPCEntity->SetMinsMaxs(HULL_SMALL_MIN, HULL_SMALL_MAX);
						hullName = "small hull";
						break;
					case NODE_HUMAN_HULL:
						pTestNPCEntity->SetMinsMaxs(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
						hullName = "human hull";
						break;
					case NODE_LARGE_HULL:
						pTestNPCEntity->SetMinsMaxs(HULL_LARGE_MIN, HULL_LARGE_MAX);
						hullName = "large hull";
						break;
					case NODE_FLY_HULL:
						pTestNPCEntity->SetMinsMaxs(HULL_LARGE_MIN, HULL_LARGE_MAX);
						hullName = "fly hull";
						break;
					}

					if(!pTestNPCEntity->WalkMove(0, 0, WALKMOVE_CHECKONLY))
					{
						CBaseEntity* pEntity = Util::GetEntityFromTrace(g_pGameVars->globaltrace);
						if(!pEntity)
							continue;

						filestr << "Node " << i << " at  " << pNode->origin[0] 
						<< " " << pNode->origin[1] << " " << pNode->origin[2] 
							<< " is not navigable in " << hullName;
						
						filestr << " - Stuck inside solid entity " << pEntity->GetClassName() 
							<< "(entindex " << pEntity->GetEntityIndex() 
							<< ") with model " << pEntity->GetModelName();

						if(pEntity->HasTargetName())
							filestr << " and targetname " << pEntity->GetTargetName();

						filestr << "\n";

						WriteToReportFile(filestr.c_str());
						filestr.clear();

						// This should NEVER happen
						if(j == 0)
							break;
					}
				}

				if(j != NUM_NODE_HULLS)
				{
					filestr << "Inconsistent un-sticking result for node " << i << " during post-unstuck testing. Node was removed.\n";
					WriteToReportFile(filestr.c_str());
					filestr.clear();

					gd_engfuncs.pfnCon_EPrintf("Inconsistent un-sticking result for node '%d'(%f, %f, %f) during post-unstuck testing. Node was removed.\n", i, pNode->origin[0], pNode->origin[1], pNode->origin[2] );

					// Erase from array
					m_nodesArray.erase(i);
					i--;
					continue;
				}
			}
		}
	}

	WriteToReportFile(filestr.c_str());
	filestr.clear();

	// Perform the initial links
	if(!LinkNodes(pTestNPCEntity))
	{
		ClearTempArrays();
		return;
	}

	// Now get more detailed link infos
	if(!WalkLinksAllHulls(pTestNPCEntity))
	{
		ClearTempArrays();
		return;
	}

	// Optimize the nodes list
	OptimizeGraph();
	// Sort the nodes
	SortNodes();
	// Determine node mins/maxs
	SetNodeMinsMaxs(pTestNPCEntity);

	filestr << "Final node array contents:" << NEWLINE;
	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
	{
		node_t* pNode = m_nodesArray[i];

		filestr << " - Node " << (Int32)i << NEWLINE;
		filestr << "\tOrigin: " << pNode->origin[0] << " " << pNode->origin[1] << " " << pNode->origin[2] << NEWLINE;
		filestr << "\tHint type: " << pNode->hinttype << NEWLINE;
		filestr << "\tHint yaw: " << pNode->hintyaw << NEWLINE;

		if(pNode->linkrange)
			filestr << "\tLink range: " << pNode->linkrange << NEWLINE;

		if(qstrlen(pNode->noderegionname) > 0)
			filestr << "\tRegion name: " << pNode->noderegionname << NEWLINE << NEWLINE;
	}

	WriteToReportFile(filestr.c_str());
	filestr.clear();

	// Remove the temp entity
	pTestNPCEntity->RemoveThink();
	pTestNPCEntity = nullptr;

	// Calculate number of hash links
	Uint32 numHashLinks = (3*m_finalNodeLinksArray.size()/2)+3;

	// Estimate the final size of the output file
	Uint32 outputFileSize = sizeof(nodefilehdr_t) 
		+ m_nodesArray.size()*sizeof(node_t)
		+ m_finalNodeLinksArray.size()*sizeof(node_link_t)
		+ m_finalLinkEntitiesArray.size()*sizeof(link_entity_t)
		+ sizeof(short)*numHashLinks;

	// Allocate output file data
	byte *pbuffer = new byte[outputFileSize];
	if(!pbuffer)
	{
		filestr << "Failed to allocate " << (Int32)outputFileSize << " bytes" << NEWLINE;
		WriteToReportFile(filestr.c_str());
		ClearTempArrays();
		return;
	}

	// Clear the data
	memset(pbuffer, 0, sizeof(byte)*outputFileSize);

	// Fill the header
	nodefilehdr_t* phdr = reinterpret_cast<nodefilehdr_t*>(pbuffer);
	phdr->id = NODE_FILE_ID;
	phdr->version = NODE_FILE_VERSION;
	phdr->filesize = outputFileSize;

	// Copy in the nodes
	Uint32 fileoffset = sizeof(nodefilehdr_t);

	if(!m_nodesArray.empty())
	{
		phdr->nodeindex = fileoffset;
		phdr->numnodes = m_nodesArray.size();
		fileoffset += phdr->numnodes*sizeof(node_t);

		node_t* pdestnodes = reinterpret_cast<node_t*>(pbuffer + phdr->nodeindex);
		for(Uint32 i = 0; i < m_nodesArray.size(); i++)
			memcpy(&pdestnodes[i], m_nodesArray[i], sizeof(node_t));
	}

	// Copy the links
	if(!m_finalNodeLinksArray.empty())
	{
		phdr->linkoffset = fileoffset;
		phdr->numlinks = m_finalNodeLinksArray.size();
		fileoffset += phdr->numlinks*sizeof(node_link_t);

		node_link_t* pdestlinks = reinterpret_cast<node_link_t*>(pbuffer + phdr->linkoffset);
		for(Uint32 i = 0; i < m_finalNodeLinksArray.size(); i++)
			memcpy(&pdestlinks[i], m_finalNodeLinksArray[i], sizeof(node_link_t));
	}

	// Copy the link entities
	if(!m_finalLinkEntitiesArray.empty())
	{
		phdr->linkentityoffset = fileoffset;
		phdr->numlinkentities = m_finalLinkEntitiesArray.size();
		fileoffset += phdr->numlinkentities*sizeof(link_entity_t);

		link_entity_t* pdestlinkents = reinterpret_cast<link_entity_t*>(pbuffer + phdr->linkentityoffset);
		for(Uint32 i = 0; i < m_finalLinkEntitiesArray.size(); i++)
			memcpy(&pdestlinkents[i], m_finalLinkEntitiesArray[i], sizeof(link_entity_t));
	}

	// Set this
	m_pNodeHeader = phdr;

	if(numHashLinks > 0)
	{
		// Allocate space for hash links
		phdr->hashlinkoffset = fileoffset;
		phdr->numhashlinks = numHashLinks;
		fileoffset += sizeof(Int16)*numHashLinks;

		// Build link lookups
		BuildLinkLookups();
	}

	// Create the static routing tables
	Uint32 routeInfoSize = 0;
	Int16* pRouteInfo = nullptr;
	BuildStaticRoutingTables(pRouteInfo, routeInfoSize);

	// Expand the buffer to house the routing data
	Uint32 routeInfoSizeBytes = routeInfoSize * sizeof(Int16);
	byte* ptempbuffer = new byte[outputFileSize + routeInfoSizeBytes];
	memcpy(ptempbuffer, pbuffer, sizeof(byte)*outputFileSize);
	outputFileSize += routeInfoSizeBytes;
	
	delete[] pbuffer;
	pbuffer = ptempbuffer;

	phdr = reinterpret_cast<nodefilehdr_t*>(pbuffer);
	m_pNodeHeader = phdr;

	phdr->routeinfoindex = fileoffset;
	phdr->routeinfosize = routeInfoSizeBytes;

	byte* pdestrouteinfo = pbuffer + phdr->routeinfoindex;
	memcpy(pdestrouteinfo, pRouteInfo, sizeof(byte)*phdr->routeinfosize);
	fileoffset += sizeof(byte)*phdr->routeinfosize;

	// Clear temp arrays
	ClearTempArrays();

	CString outputFileName;
	outputFileName << NODE_FILES_FOLDER_PATH << PATH_SLASH_CHAR << g_pGameVars->levelname << NODE_FILE_EXTENSION;

	if(fileoffset != outputFileSize)
	{
		gd_engfuncs.pfnCon_EPrintf("%s - Mismatch in estimated and final file size for '%s'(%d != %d).\n", __FUNCTION__, outputFileName.c_str(), fileoffset, outputFileSize);
		return;
	}

	if(!gd_filefuncs.pfnWriteFile(pbuffer, outputFileSize, outputFileName.c_str(), false))
	{
		gd_engfuncs.pfnCon_EPrintf("%s - Failed to write file '%s'.\n", __FUNCTION__, outputFileName.c_str());
		return;
	}

	// Mark end time
	Double endTime = gd_engfuncs.pfnFloatTime();
	Double duration = endTime - beginTime;
	Int32 minutes = SDL_floor(duration) / 60;
	Int32 seconds = duration - minutes * 60;

	filestr << "=============================================" << NEWLINE;
	filestr << " - File size: " << (Int32)outputFileSize << " bytes" << NEWLINE;
	filestr << " - File name: " << outputFileName << NEWLINE;
	filestr << " - Node graph generation time: " << minutes << " minutes " << seconds <<  " seconds" << NEWLINE;
	filestr << " Node graph file end " << NEWLINE;
	filestr << "=============================================" << NEWLINE;

	WriteToReportFile(filestr.c_str());

	if(!m_pLogFile->Close())
		gd_engfuncs.pfnCon_EPrintf("%s - Failed to close log file.\n", __FUNCTION__);

	delete m_pLogFile;
	m_pLogFile = nullptr;
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::SetNodeMinsMaxs( CBaseEntity* pTestNPC )
{
	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
	{
		node_t* pNode = m_nodesArray[i];

		for(Uint32 j = 0; j < NUM_NODE_HULLS; j++)
		{
			pNode->mins[j] = NULL_MINS;
			pNode->maxs[j] = NULL_MAXS;

			Vector mins, maxs;
			switch(j)
			{
			case NODE_SMALL_HULL:
				{
					mins = HULL_SMALL_MIN;
					maxs = HULL_SMALL_MAX;
				}
				break;
			case NODE_HUMAN_HULL:
				{
					mins = VEC_HUMAN_HULL_MIN;
					maxs = VEC_HUMAN_HULL_MAX;
				}
				break;
			case NODE_LARGE_HULL:
			case NODE_FLY_HULL:
				{
					mins = HULL_LARGE_MIN;
					maxs = HULL_LARGE_MAX;
				}
				break;
			}

			pTestNPC->SetMinsMaxs(mins, maxs);

			Uint64 addedFlags = 0;
			if(pNode->nodeinfobits & AI_NODE_WATER)
				addedFlags |= FL_SWIM;
			else if(pNode->nodeinfobits & AI_NODE_AIR)
				addedFlags |= FL_FLY;

			pTestNPC->SetFlags(addedFlags);

			for(Float yaw = 0; yaw <= 360.0f; yaw += 15)
			{
				// Place entity at node origin
				pTestNPC->SetOrigin(pNode->origin);

				// Place on ground firmly if not fly/swim
				if(!(addedFlags & (FL_SWIM|FL_FLY)))
					pTestNPC->GroundEntityNudge();

				Float distance = 0;
				while(pTestNPC->WalkMove(yaw, CBaseNPC::NPC_STEP_SIZE, WALKMOVE_CHECKONLY) 
					&& distance < CBaseNPC::NPC_MAX_NAVIGATION_DISTANCE)
					distance += CBaseNPC::NPC_STEP_SIZE;

				Vector entityOrigin = pTestNPC->GetOrigin();

				for(Uint32 k = 0; k < 3; k++)
				{
					if((entityOrigin[k] + mins[k]) < pNode->mins[j][k])
						pNode->mins[j][k] = entityOrigin[k] + mins[k];

					if((entityOrigin[k] + maxs[k]) > pNode->maxs[j][k])
						pNode->maxs[j][k] = entityOrigin[k] + maxs[k];
				}

				// With fly hull, factor in the ground and ceiling
				if(j == NODE_FLY_HULL)
				{
					trace_t tr;
					Util::TraceHull(pNode->origin, pNode->origin + Vector(0, 0, 8192), true, false, HULL_POINT, nullptr, tr);
					if(!tr.startSolid() && !tr.allSolid())
						pNode->maxs[j][2] = tr.endpos[2] + maxs[2];

					Util::TraceHull(pNode->origin, pNode->origin - Vector(0, 0, 8192), true, false, HULL_POINT, nullptr, tr);
					if(!tr.startSolid() && !tr.allSolid())
						pNode->mins[j][2] = tr.endpos[2] + mins[2];
				}
			}

			// Remove the flags we've changed
			pTestNPC->RemoveFlags(addedFlags);

			for(Uint32 k = 0; k < pNode->numlinks; k++)
			{
				node_link_t* pLink = m_finalNodeLinksArray[pNode->firstlinkindex + k];
				node_t* pLinkNode = m_nodesArray[pLink->sourcenode];

				if(pLinkNode == pNode)
					continue;

				// TODO: is this even needed?
				if(Math::PointInMinsMaxs(pLinkNode->origin, pNode->mins[j], pNode->maxs[j]))
				{
					for(Uint32 l = 0; l < 3; l++)
					{
						if(pNode->mins[j][l] < pLinkNode->origin[l])
							pNode->mins[j][l] = pLinkNode->origin[l];

						if(pNode->maxs[j][l] > pLinkNode->origin[l])
							pNode->maxs[j][l] = pLinkNode->origin[l];
					}
				}
			}
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::ChooseHashPrimes( Uint32 tableSize )
{
	Int32 largestPrimeNb = (Int32)(tableSize/2);
	if(largestPrimeNb > PRIME_VALUES[NUM_PRIME_VALUES-2])
		largestPrimeNb = PRIME_VALUES[NUM_PRIME_VALUES-2];

	Int32 spacing = (Int32)(largestPrimeNb/16);

	for(Int32 zone = 1, iprime = 0; iprime < 16; zone += spacing)
	{
		Int32 lowerPrime = PRIME_VALUES[0];
		for(Int32 jprime = 0; PRIME_VALUES[jprime] != 0; jprime++)
		{
			if(jprime != 0 && (tableSize % PRIME_VALUES[jprime]) == 0)
				continue;

			Int32 upperPrime = PRIME_VALUES[jprime];
			if(lowerPrime <= zone && zone <= upperPrime)
			{
				Uint32 lowerDist = zone - lowerPrime;
				Uint32 upperDist = upperPrime - zone;
				
				if(lowerDist <= upperDist)
					m_pNodeHeader->hashprimes[iprime] = lowerPrime;
				else
					m_pNodeHeader->hashprimes[iprime] = upperPrime;

				iprime++;
				break;
			}

			lowerPrime = upperPrime;
		}
	}

	for(Uint32 i = 0; i < NUM_HASH_PRIMES; i+=2)
		m_pNodeHeader->hashprimes[i] = tableSize - m_pNodeHeader->hashprimes[i];

	// Shuffle the primes
	for(Int32 i = 0; i < NUM_HASH_PRIMES-1; i++)
	{
		Int32 randomMax = (NUM_HASH_PRIMES-1) - i;
		Int32 pick = Common::RandomLong(0, randomMax);
		Int32 tmp = m_pNodeHeader->hashprimes[pick];

		m_pNodeHeader->hashprimes[pick] = m_pNodeHeader->hashprimes[(NUM_HASH_PRIMES-1)-i];
		m_pNodeHeader->hashprimes[(NUM_HASH_PRIMES-1)-i] = tmp;
	}
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::InsertLinkHash( Int16 srcNode, Int16 dstNode, Int32 key ) const
{
	// Generate the hash
	nodepair_t pair(srcNode, dstNode);
	CCRC32Hash crc32((const byte*)&pair, sizeof(pair));
	Uint32 hash = crc32.GetHashResult();

	Int32 di = m_pNodeHeader->hashprimes[hash & (NUM_HASH_PRIMES-1)];
	Int32 i = (hash >> 4) % m_pNodeHeader->numhashlinks;

	Int16* pHashLinks = reinterpret_cast<Int16*>(reinterpret_cast<byte*>(m_pNodeHeader) + m_pNodeHeader->hashlinkoffset);
	while(pHashLinks[i] != EMPTY_HASH_LINK_VALUE)
	{
		i += di;

		if(i >= (Int32)m_pNodeHeader->numhashlinks)
			i -= (Int32)m_pNodeHeader->numhashlinks;
	}

	pHashLinks[i] = key;
}

//=============================================
// @brief
//
//=============================================
Int32 CAINodeGraph::SearchHash( Int16 srcNode, Int16 dstNode )
{
	// Generate the hash
	nodepair_t pair(srcNode, dstNode);
	CCRC32Hash crc32((const byte*)&pair, sizeof(pair));
	Uint32 hash = crc32.GetHashResult();

	Int32 di = m_pNodeHeader->hashprimes[hash & (NUM_HASH_PRIMES-1)];
	Int32 i = (hash >> 4) % m_pNodeHeader->numhashlinks;

	node_link_t* pNodeLinks = reinterpret_cast<node_link_t*>(reinterpret_cast<byte*>(m_pNodeHeader) + m_pNodeHeader->linkoffset);
	Int16* pHashLinks = reinterpret_cast<Int16*>(reinterpret_cast<byte*>(m_pNodeHeader) + m_pNodeHeader->hashlinkoffset);
	while(pHashLinks[i] != EMPTY_HASH_LINK_VALUE)
	{
		node_link_t* pLink = &pNodeLinks[pHashLinks[i]];

		if(srcNode == pLink->sourcenode && dstNode == pLink->destnode)
			break;

		i += di;

		if(i >= (Int32)m_pNodeHeader->numhashlinks)
			i -= (Int32)m_pNodeHeader->numhashlinks;
	}

	return pHashLinks[i];
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::BuildLinkLookups( void )
{
	ChooseHashPrimes(m_pNodeHeader->numhashlinks);

	Int16* pHashLinks = reinterpret_cast<Int16*>(reinterpret_cast<byte*>(m_pNodeHeader) + m_pNodeHeader->hashlinkoffset);
	for(Uint32 i = 0; i < m_pNodeHeader->numhashlinks; i++)
		pHashLinks[i] = EMPTY_HASH_LINK_VALUE;

	for(Uint32 i = 0; i < m_finalNodeLinksArray.size(); i++)
	{
		node_link_t* pLink = m_finalNodeLinksArray[i];
		InsertLinkHash(pLink->sourcenode, pLink->destnode, i);
	}
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::SortNodes( void )
{
	if(m_nodesArray.empty())
		return;

	Uint32 nodecount = 0;
	m_nodesArray[0]->previousnode = nodecount;
	nodecount++;

	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
	{
		node_t* pNode = m_nodesArray[i];
		for(Uint32 j = 0; j < pNode->numlinks; j++)
		{
			node_link_t* pNodeLink = m_finalNodeLinksArray[pNode->firstlinkindex + j];
			node_t* pDestNode = m_nodesArray[pNodeLink->destnode];

			if(pDestNode->previousnode == NO_POSITION)
			{
				pDestNode->previousnode = nodecount;
				nodecount++;
			}
		}
	}

	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
	{
		node_t* pNode = m_nodesArray[i];
		if(pNode->previousnode == NO_POSITION)
		{
			pNode->previousnode = nodecount;
			nodecount++;
		}
	}

	// Reassign link node indexes
	for(Uint32 i = 0; i < m_finalNodeLinksArray.size(); i++)
	{
		node_link_t* pNodeLink = m_finalNodeLinksArray[i];

		pNodeLink->sourcenode = m_nodesArray[pNodeLink->sourcenode]->previousnode;
		pNodeLink->destnode = m_nodesArray[pNodeLink->destnode]->previousnode;
	}

	// Reposition the nodes impacted
	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
	{
		while(m_nodesArray[i]->previousnode != (Int32)i)
		{
			Int32 destPosition = m_nodesArray[i]->previousnode;
			node_t* pNode1 = m_nodesArray[destPosition];
			m_nodesArray[destPosition] = m_nodesArray[i];
			m_nodesArray[i] = pNode1;
		}
	}

	// Set indexes for nodes
	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
		m_nodesArray[i]->index = i;
}

//=============================================
// @brief
//
//=============================================
bool CAINodeGraph::LinkNodes( CBaseEntity* pTestNPC )
{
	// Log beginning
	CString filestr;
	filestr << "=============================================" << NEWLINE;
	filestr << " LinkNodes begin" << NEWLINE;
	filestr << "=============================================" << NEWLINE;

	WriteToReportFile(filestr.c_str());
	filestr.clear();

	// Count usable and unusable nodes
	Uint32 nbNodeLinks = 0;
	Uint32 nbLinkEntities = 0;

	// Set small node hull for the hulltest npc, as we do initial
	// linking with the smallest hull NPCs use
	pTestNPC->SetMinsMaxs(HULL_SMALL_MIN, HULL_SMALL_MAX);

	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
	{
		node_t* pNodeFrom = m_nodesArray[i];

		// Mark down first node
		m_nodesArray[i]->firstlinkindex = m_nodeLinksArrayOfArrays.size();
		m_nodeLinksArrayOfArrays.resize(m_nodeLinksArrayOfArrays.size()+1);

		for(Uint32 j = 0; j < m_nodesArray.size(); j++)
		{
			// Do not try for self
			if(i == j)
				continue;

			node_t* pNodeTo = m_nodesArray[j];

			// Check ranges
			if(pNodeFrom->linkrange > 0 || pNodeTo->linkrange > 0)
			{
				Float nodeDistance = (pNodeFrom->origin - pNodeTo->origin).Length();
				if(pNodeFrom->linkrange && nodeDistance > pNodeFrom->linkrange
					|| pNodeTo->linkrange && nodeDistance > pNodeTo->linkrange)
					continue;
			}

			if((pNodeFrom->nodeinfobits & AI_NODE_GROUP_REALM) != (pNodeTo->nodeinfobits & AI_NODE_GROUP_REALM))
				continue;

			// Remember which flags we changed
			Uint64 npcFlagsAdded = 0;
			if(pNodeFrom->nodeinfobits & AI_NODE_WATER)
				npcFlagsAdded = FL_SWIM;
			else if(pNodeFrom->nodeinfobits & AI_NODE_AIR)
				npcFlagsAdded = FL_FLY;

			// Set on the entity
			pTestNPC->SetFlags(npcFlagsAdded);

			const Float SHIFT_MAX = 0.5;
			shift_direction_t shiftdirection = SHIFT_NONE;
			shift_node_t shiftnode = SHIFT_FROM;
			
			Int32 shiftBits = 0;
			Float shiftAmount = 0;
			Vector baseFrom = pNodeFrom->origin;
			Vector baseTo = pNodeTo->origin;

			CArray<CBaseEntity*> linkEntityArray;
			bool walkResult = false;
			while(!walkResult)
			{
				Vector testFrom;
				Vector testTo;

				Vector shiftVector;
				if(shiftdirection != SHIFT_NONE)
				{
					shiftAmount += 0.1;

					Vector forward = (pNodeTo->origin - pNodeFrom->origin);
					forward[2] = 0;
					forward.Normalize();

					Vector up, right;
					Math::GetUpRight(forward, up, right);

					Float sign = (shiftdirection == SHIFT_NEG) ? -1.0 : 1.0;
					shiftVector = right*shiftAmount*sign;

					testFrom = baseFrom;
					if(shiftnode == SHIFT_FROM || shiftnode == SHIFT_BOTH)
						testFrom += shiftVector;

					testTo = baseTo;
					if(shiftnode == SHIFT_TO || shiftnode == SHIFT_BOTH)
						testTo += shiftVector;
				}
				else
				{
					testFrom = baseFrom;
					testTo = baseTo;
				}

				// Try and go there
				walkResult = WalkPath(testFrom, testTo, pTestNPC, linkEntityArray, (pNodeFrom->nodeinfobits & AI_NODE_WATER) ? true : false);

				if(!walkResult)
				{
					if(shiftdirection == SHIFT_NONE)
					{
						if(g_pGameVars->globaltrace.hitentity <= WORLDSPAWN_ENTITY_INDEX)
							break;

						CBaseEntity* pHitEntity = Util::GetEntityFromTrace(g_pGameVars->globaltrace);
						if(!pHitEntity || !pHitEntity->IsFuncDoorEntity() && !pHitEntity->IsFuncDoorRotatingEntity())
							break;

						// If func_door or func_door_rotating, try shifting a bit
						shiftdirection = SHIFT_POS;

						// Choose most ideal node shift
						if(((pNodeFrom->nodeflags & NODE_FL_SHIFTED)
							&& !(pNodeTo->nodeflags & NODE_FL_SHIFTED)))
							shiftnode = SHIFT_TO;
						else if(((pNodeTo->nodeflags & NODE_FL_SHIFTED)
							&& !(pNodeFrom->nodeflags & NODE_FL_SHIFTED)))
							shiftnode = SHIFT_FROM;
						else
							shiftnode = SHIFT_BOTH;

						continue;
					}
					else
					{
						if(shiftAmount >= SHIFT_MAX)
						{
							// Store current shift
							shiftBits |= (1<<shiftnode);

							if(!(shiftBits & (1<<SHIFT_BOTH)) 
								|| !(shiftBits & (1<<SHIFT_FROM))
								|| !(shiftBits & (1<<SHIFT_TO)))
							{
								// Choose ideal node shift, or last available one
								if(((pNodeFrom->nodeflags & NODE_FL_SHIFTED)
									&& !(pNodeTo->nodeflags & NODE_FL_SHIFTED))
									&& !(shiftBits & (1<<SHIFT_TO)))
									shiftnode = SHIFT_TO;
								else if(((pNodeTo->nodeflags & NODE_FL_SHIFTED)
									&& !(pNodeFrom->nodeflags & NODE_FL_SHIFTED))
									&& !(shiftBits & (1<<SHIFT_FROM)))
									shiftnode = SHIFT_FROM;
								else if(!(shiftBits & (1<<SHIFT_BOTH)))
									shiftnode = SHIFT_BOTH;
								else if(!(shiftBits & (1<<SHIFT_FROM)))
									shiftnode = SHIFT_FROM;
								else if(!(shiftBits & (1<<SHIFT_TO)))
									shiftnode = SHIFT_TO;

								// Switch shifting mode and reset amount
								shiftAmount = 0;
							}
							else if(shiftdirection == SHIFT_POS)
							{
								// Choose most ideal node shift
								if(((pNodeFrom->nodeflags & NODE_FL_SHIFTED)
									&& !(pNodeTo->nodeflags & NODE_FL_SHIFTED)))
									shiftnode = SHIFT_TO;
								else if(((pNodeTo->nodeflags & NODE_FL_SHIFTED)
									&& !(pNodeFrom->nodeflags & NODE_FL_SHIFTED)))
									shiftnode = SHIFT_FROM;
								else
									shiftnode = SHIFT_BOTH;

								// Switch to negative
								shiftdirection = SHIFT_NEG;
								shiftAmount = 0;
								shiftBits = 0;
							}
							else
							{
								// couldn't shift past blocker
								break;
							}
						}
					}
				}
				else if(shiftdirection != SHIFT_NONE)
				{
					if(shiftnode == SHIFT_FROM || shiftnode == SHIFT_BOTH)
					{
						filestr << " - Node " << pNodeFrom->origindex << " got shifted by " << shiftVector.x << ", " << shiftVector.y << ", " << shiftVector.z << " because of a door." << NEWLINE;
						pNodeFrom->nodeflags |= NODE_FL_SHIFTED;
						pNodeFrom->origin = testFrom;
					}
					
					if(shiftnode == SHIFT_TO || shiftnode == SHIFT_BOTH)
					{
						filestr << " - Node " << pNodeTo->origindex << " got shifted by " << shiftVector.x << ", " << shiftVector.y << ", " << shiftVector.z << " because of a door." << NEWLINE;
						pNodeTo->nodeflags |= NODE_FL_SHIFTED;
						pNodeTo->origin = testTo;
					}

					WriteToReportFile(filestr.c_str());
					filestr.clear();
				}
			}

			// Remove any flags we added
			pTestNPC->RemoveFlags(npcFlagsAdded);

			if(!walkResult)
				continue;

			// Log this connection
			filestr << " - Node with index " << (Int32)i << "(" << pNodeFrom->origin[0] << " " << pNodeFrom->origin[1] << " " << pNodeFrom->origin[2] 
				<< ") connected to node with index " << (Int32)j << "(" << pNodeTo->origin[0] << " " << pNodeTo->origin[1] << " " << pNodeTo->origin[2] << ")"
				<< NEWLINE;

			WriteToReportFile(filestr.c_str());
			filestr.clear();

			// Create a link object, as this is a valid link
			node_link_t* plink = new node_link_t;
			nbNodeLinks++;

			plink->sourcenode = i;
			plink->destnode = j;

			// Add to the array
			m_nodeLinksArrayOfArrays[pNodeFrom->firstlinkindex].push_back(plink);
			pNodeFrom->numlinks++;

			if(!linkEntityArray.empty())
			{
				plink->linkentityindex = m_linkEntityArrayOfArrays.size();
				m_linkEntityArrayOfArrays.resize(m_linkEntityArrayOfArrays.size()+1);

				plink->numlinkentities = linkEntityArray.size();

				// Log info
				filestr << "\t " << (Int32)linkEntityArray.size() << " link entities:" << NEWLINE;

				WriteToReportFile(filestr.c_str());
				filestr.clear();

				for(Uint32 k = 0; k < linkEntityArray.size(); k++)
				{
					CBaseEntity* pEntity = linkEntityArray[k];

					link_entity_t* plinkentity = new link_entity_t;
					m_linkEntityArrayOfArrays[plink->linkentityindex].push_back(plinkentity);
					nbLinkEntities++;

					plinkentity->entityindex = pEntity->GetEntityIndex();
					qstrcpy(plinkentity->modelname, pEntity->GetModelName());
					plinkentity->hulltype = NODE_SMALL_HULL;

					// Mark entity as a graph entity
					pEntity->SetFlags(FL_GRAPH_ENTITY);

					// Log information about entity
					filestr << "\t - Link entity classname: " << pEntity->GetClassName() << ")";
					if(pEntity->HasTargetName())
						filestr << ", targetname: " << pEntity->GetTargetName();

					filestr << ", model name: " << pEntity->GetModelName()
						<< ", link entity minimum hull: ";

					switch(plinkentity->hulltype)
					{
					case NODE_SMALL_HULL:
						filestr << "NODE_SMALL_HULL";
						break;
					case NODE_HUMAN_HULL:
						filestr << "NODE_HUMAN_HULL";
						break;
					case NODE_LARGE_HULL:
						filestr << "NODE_LARGE_HULL";
						break;
					case NODE_FLY_HULL:
						filestr << "NODE_FLY_HULL";
						break;
					}

					filestr << NEWLINE;
					WriteToReportFile(filestr.c_str());
					filestr.clear();
				}
			}
			else
			{
				filestr << "\tNo link entities." << NEWLINE;
				WriteToReportFile(filestr.c_str());
				filestr.clear();

				// This link has no entities tied to it
				plink->linkentityindex = NO_POSITION;
			}
		}
	}

	// Check shifted nodes for connections still being OK
	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
	{
		node_t* pNodeFrom = m_nodesArray[i];

		for(Uint32 j = 0; j < pNodeFrom->numlinks; j++)
		{
			// Get pointer to the link
			node_link_t* plink = m_nodeLinksArrayOfArrays[pNodeFrom->firstlinkindex][j];
			// Get destinatio node
			node_t* pNodeTo = m_nodesArray[plink->destnode];

			// Only check for shifted nodes
			if(!(pNodeFrom->nodeflags & NODE_FL_SHIFTED) && !(pNodeTo->nodeflags & NODE_FL_SHIFTED))
				continue;

			// Remember which flags we changed
			Uint64 npcFlagsAdded = 0;
			if(pNodeFrom->nodeinfobits & AI_NODE_WATER)
				npcFlagsAdded = FL_SWIM;
			else if(pNodeFrom->nodeinfobits & AI_NODE_AIR)
				npcFlagsAdded = FL_FLY;

			// Set on the entity
			pTestNPC->SetFlags(npcFlagsAdded);

			// Try and go there
			CArray<CBaseEntity*> linkEntityArray;
			bool walkResult = WalkPath(pNodeFrom->origin, pNodeTo->origin, pTestNPC, linkEntityArray, (pNodeFrom->nodeinfobits & AI_NODE_WATER) ? true : false);
			if(!walkResult)
			{
				// Log this rejection
				filestr << " - Link between node " << (Int32)plink->sourcenode << " to node " << (Int32)plink->destnode << ", rejected as it was not traversable after shifting." << NEWLINE;
				WriteToReportFile(filestr.c_str());
				filestr.clear();

				// Clear link entities relevant to this link
				if(plink->numlinkentities > 0 && plink->linkentityindex != NO_POSITION)
				{
					CArray<link_entity_t*>& linkEntityArray = m_linkEntityArrayOfArrays[plink->linkentityindex];
					for(Uint32 l = 0; l < linkEntityArray.size(); l++)
						delete linkEntityArray[l];

					nbLinkEntities -= linkEntityArray.size();
					linkEntityArray.clear();
				}

				// Delete the link itself
				delete m_nodeLinksArrayOfArrays[pNodeFrom->firstlinkindex][j];
				m_nodeLinksArrayOfArrays[pNodeFrom->firstlinkindex].erase(j);

				pNodeFrom->numlinks--;
				nbNodeLinks--;
				j--;
			}
		}
	}

	// Log end
	filestr << "=============================================" << NEWLINE;
	filestr << " LinkNodes end" << NEWLINE;
	filestr << " - Initial number of nodes: " << (Int32)m_nodesArray.size() << NEWLINE;
	filestr << " - Initial number of links: " << (Int32)nbNodeLinks << NEWLINE;
	filestr << " - Initial number of link entities " << (Int32)nbLinkEntities << NEWLINE;
	filestr << "=============================================" << NEWLINE;
	WriteToReportFile(filestr.c_str());

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CAINodeGraph::WalkLinksAllHulls( CBaseEntity* pTestNPC )
{
	// Log beginning
	CString filestr;
	filestr << "=============================================" << NEWLINE;
	filestr << " WalkLinksAllHulls begin" << NEWLINE;
	filestr << "=============================================" << NEWLINE;

	WriteToReportFile(filestr.c_str());
	filestr.clear();

	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
	{
		node_t* pNodeFrom = m_nodesArray[i];

		// Log the node
		filestr << " - Node " << (Int32)i << ":" << NEWLINE;
		WriteToReportFile(filestr.c_str());
		filestr.clear();

		for(Uint32 j = 0; j < pNodeFrom->numlinks; j++)
		{
			// Get pointer to the link
			node_link_t* plink = m_nodeLinksArrayOfArrays[pNodeFrom->firstlinkindex][j];

			// Assign all types initially, then we'll remove them
			plink->linkinfobits = (LINK_SMALL_HULL|LINK_HUMAN_HULL|LINK_LARGE_HULL|LINK_FLY_HULL);

			// If a smaller hull can't fit through, then it means a larger won't either,
			// so skip the rest with this flag
			bool skipLargerHulls = false;

			// Skip small hull, as it's already been tested
			// at the initial linking
			for(Uint32 k = NODE_HUMAN_HULL; k < NUM_NODE_HULLS; k++)
			{
				// Do not skip the fly hull
				if(skipLargerHulls && k < NODE_FLY_HULL)
					continue;

				switch(k)
				{
				case NODE_HUMAN_HULL:
					pTestNPC->SetMinsMaxs(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
					break;
				case NODE_LARGE_HULL:
				case NODE_FLY_HULL:
					pTestNPC->SetMinsMaxs(HULL_LARGE_MIN, HULL_LARGE_MAX);
					break;
				}

				// Get destinatio node
				node_t* pNodeTo = m_nodesArray[plink->destnode];

				// Fly hull is a simple trace
				if(k == NODE_FLY_HULL)
				{
					Vector traceStart = pNodeFrom->origin + Vector(0, 0, 40);
					Vector traceEnd = pNodeTo->origin + Vector(0, 0, 40);

					trace_t tr;
					Util::TraceHull(traceStart, traceEnd, true, false, HULL_LARGE, pTestNPC->GetEdict(), tr);
					if(tr.startSolid() || !tr.noHit())
						plink->linkinfobits &= ~LINK_FLY_HULL;

					continue;
				}

				// Disable link entities we already tested for
				CEntityStateStack stack;
				if(plink->numlinkentities > 0)
				{
					for(Uint32 l = 0; l < plink->numlinkentities; l++)
					{
						link_entity_t* plinkentity = m_linkEntityArrayOfArrays[plink->linkentityindex][l];
						CBaseEntity* pEntity = Util::GetEntityByIndex(plinkentity->entityindex);
						
						// IMPORTANT - ALWAYS retest door entities
						if(pEntity->IsFuncDoorEntity() || pEntity->IsFuncDoorRotatingEntity())
							continue;

						if(pEntity && !pEntity->IsFuncDoorEntity() && !pEntity->IsFuncDoorRotatingEntity())
							stack.SaveEntity(pEntity);
					}
				}

				// Remember which flags we changed
				Uint64 npcFlagsAdded = 0;
				if(pNodeFrom->nodeinfobits & AI_NODE_WATER)
					npcFlagsAdded = FL_SWIM;
				else if(pNodeFrom->nodeinfobits & AI_NODE_AIR)
					npcFlagsAdded = FL_FLY;

				// Add these flags to the NPC
				pTestNPC->SetFlags(npcFlagsAdded);

				CArray<CBaseEntity*> linkEntitiesArray;
				bool walkResult = WalkPath(pNodeFrom->origin, pNodeTo->origin, pTestNPC, linkEntitiesArray, (pNodeFrom->nodeinfobits & AI_NODE_WATER) ? true : false);

				// Remove the flags we set
				pTestNPC->RemoveFlags(npcFlagsAdded);

				if(!walkResult)
				{
					filestr << "\tNode " << (Int32)j << " unreachable by node " << (Int32)i << " with hull ";

					// remove this hull's flag from the link's flags
					switch(k)
					{
					case NODE_HUMAN_HULL:
						filestr << "NODE_HUMAN_HULL, LINK_LARGE_HULL";
						plink->linkinfobits &= ~(LINK_HUMAN_HULL|LINK_LARGE_HULL);
						skipLargerHulls = true;
						break;
					case NODE_LARGE_HULL:
						filestr << "NODE_LARGE_HULL";
						plink->linkinfobits &= ~LINK_LARGE_HULL;
						skipLargerHulls = true;
						break;
					}

					filestr << NEWLINE;
					WriteToReportFile(filestr.c_str());
					filestr.clear();
					continue;
				}

				// Check if this entity is already present. if not, then add it
				// to the array of linkents with the appropriate hull
				for(Uint32 l = 0; l < linkEntitiesArray.size(); l++)
				{
					CBaseEntity* pEntity = linkEntitiesArray[l];

					Int32 m;
					if(plink->linkentityindex != -1)
					{
						m = 0;

						for(; m < (Int32)m_linkEntityArrayOfArrays[plink->linkentityindex].size(); m++)
						{
							if(m_linkEntityArrayOfArrays[plink->linkentityindex][m]->entityindex == pEntity->GetEntityIndex())
								break;
						}
					}
					else
					{
						m = -1;
					}

					if(plink->linkentityindex == -1)
					{
						plink->linkentityindex = m_linkEntityArrayOfArrays.size();
						m_linkEntityArrayOfArrays.resize(m_linkEntityArrayOfArrays.size()+1);
					}

					if(m != (Int32)m_linkEntityArrayOfArrays[plink->linkentityindex].size())
					{
						link_entity_t* plinkentity = new link_entity_t;
						plinkentity->entityindex = pEntity->GetEntityIndex();
						qstrcpy(plinkentity->modelname, pEntity->GetModelName());
						plinkentity->hulltype = (node_hull_types_t)k;

						m_linkEntityArrayOfArrays[plink->linkentityindex].push_back(plinkentity);
						plink->numlinkentities++;
					}
				}
			}
		}
	}

	// Log end
	filestr << "=============================================" << NEWLINE;
	filestr << " WalkLinksAllHulls end" << NEWLINE;
	filestr << "=============================================" << NEWLINE;

	WriteToReportFile(filestr.c_str());
	filestr.clear();

	return true;
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::OptimizeGraph( void )
{
	// Log beginning
	CString filestr;
	filestr << "=============================================" << NEWLINE;
	filestr << " OptimizeGraph begin" << NEWLINE;
	filestr << "=============================================" << NEWLINE;

	WriteToReportFile(filestr.c_str());
	filestr.clear();

	bool restartLoop = false;

	// Number of rejected links
	Uint32 numDeletedLinks = 0;
	Uint32 numDeletedNodes = 0;
	Uint32 numDeletedLinkEntities = 0;

	// Remove any inline links
	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
	{
		node_t* pNodeFrom = m_nodesArray[i];
		if(pNodeFrom->nodeflags & NODE_FL_NO_OPTIMIZE)
			continue;

		for(Uint32 j = 0; j < pNodeFrom->numlinks; j++)
		{
			node_link_t* pNodeLink = m_nodeLinksArrayOfArrays[pNodeFrom->firstlinkindex][j];
			node_t* pNodeTo = m_nodesArray[pNodeLink->destnode];
			if(pNodeTo->nodeflags & NODE_FL_NO_OPTIMIZE)
				continue;

			Vector dirToCheckNode = (pNodeTo->origin - pNodeFrom->origin);
			dirToCheckNode[2] = 0;

			Float distToCheckNode = dirToCheckNode.Length2D();
			if(distToCheckNode > 0)
				Math::VectorScale(dirToCheckNode, (1.0f/distToCheckNode), dirToCheckNode);

			pNodeLink->weight = distToCheckNode;

			// Reset this
			restartLoop = false;

			for(Uint32 k = 0; k < pNodeFrom->numlinks && !restartLoop; k++)
			{
				if(j == k)
					continue;

				node_link_t* pTestNodeLink = m_nodeLinksArrayOfArrays[pNodeFrom->firstlinkindex][k];
				node_t* pTestNode = m_nodesArray[pTestNodeLink->destnode];

				Vector dirToTestNode = (pTestNode->origin - pNodeFrom->origin);
				dirToTestNode[2] = 0;

				Float distToTestNode = dirToTestNode.Length2D();
				if(distToTestNode > 0)
					Math::VectorScale(dirToTestNode, (1.0f/distToTestNode), dirToTestNode);

				Float dp = Math::DotProduct(dirToCheckNode, dirToTestNode);
				if(dp > 0.998 && distToTestNode < distToCheckNode)
				{
					// Log this rejection
					filestr << " - Rejected link from node " << (Int32)pNodeLink->destnode << " to node " << (Int32)pTestNodeLink->destnode << ", DP: " << dp << NEWLINE;
					WriteToReportFile(filestr.c_str());
					filestr.clear();

					// Clear link entities relevant to this link
					if(pNodeLink->numlinkentities > 0 && pNodeLink->linkentityindex != NO_POSITION)
					{
						CArray<link_entity_t*>& linkEntityArray = m_linkEntityArrayOfArrays[pNodeLink->linkentityindex];
						for(Uint32 l = 0; l < linkEntityArray.size(); l++)
							delete linkEntityArray[l];

						// Now clear the array of linkents
						numDeletedLinkEntities += linkEntityArray.size();
						linkEntityArray.clear();
					}

					// Delete the link itself
					delete m_nodeLinksArrayOfArrays[pNodeFrom->firstlinkindex][j];
					m_nodeLinksArrayOfArrays[pNodeFrom->firstlinkindex].erase(j);

					pNodeFrom->numlinks--;
					numDeletedLinks++;
					j--;

					restartLoop = true;
				}
			}
		}
	}

	// Create final link array
	Uint32 finalNbLinks = 0;
	for(Uint32 i = 0; i < m_nodeLinksArrayOfArrays.size(); i++)
		finalNbLinks += m_nodeLinksArrayOfArrays[i].size();

	// Create indexes map, as well as insert links to final array
	CArray<Int32> nodeLinkIndexMap(m_nodeLinksArrayOfArrays.size());
	// Current offset into node links array
	Uint32 linkArrayOffset = 0;

	// Allocate final array
	m_finalNodeLinksArray.resize(finalNbLinks);

	for(Uint32 i = 0; i < m_nodeLinksArrayOfArrays.size(); i++)
	{
		if(m_nodeLinksArrayOfArrays[i].empty())
		{
			nodeLinkIndexMap[i] = NO_POSITION;
			continue;
		}

		for(Uint32 j = 0; j < m_nodeLinksArrayOfArrays[i].size(); j++)
			m_finalNodeLinksArray[linkArrayOffset + j] = m_nodeLinksArrayOfArrays[i][j];

		// Set the map position
		nodeLinkIndexMap[i] = linkArrayOffset;
		linkArrayOffset += m_nodeLinksArrayOfArrays[i].size();
	}

	if(!m_nodeLinksArrayOfArrays.empty())
		m_nodeLinksArrayOfArrays.clear();

	// Print link array contents
	filestr << "Link array contents:" << NEWLINE;
	for(Uint32 i = 0; i < m_finalNodeLinksArray.size(); i++)
		filestr << " - Link from node " << m_finalNodeLinksArray[i]->sourcenode << " to node " << m_finalNodeLinksArray[i]->destnode << "." << NEWLINE;

	WriteToReportFile(filestr.c_str());
	filestr.clear();

	// Now curb down the nodes to only the valid ones
	CArray<Int32> nodeIndexesMap(m_nodesArray.size());
	CArray<node_t*> finalNodesArray;

	for(Uint32 i = 0; i < m_nodesArray.size(); i++)
	{
		if(!m_nodesArray[i]->numlinks)
		{
			Uint32 j = 0;
			for(; j < m_finalNodeLinksArray.size(); j++)
			{
				if(m_finalNodeLinksArray[j]->destnode == (Int32)i)
					break;
			}

			if(j == m_finalNodeLinksArray.size())
			{
				delete m_nodesArray[i];
				nodeIndexesMap[i] = NO_POSITION;
				numDeletedNodes++;
				continue;
			}
		}

		nodeIndexesMap[i] = finalNodesArray.size();
		finalNodesArray.push_back(m_nodesArray[i]);

		// Set final link index
		m_nodesArray[i]->firstlinkindex = nodeLinkIndexMap[m_nodesArray[i]->firstlinkindex];
	}

	// Set final contents
	m_nodesArray = finalNodesArray;

	// Realign link indexes
	for(Uint32 i = 0; i < m_finalNodeLinksArray.size(); i++)
	{
		node_link_t* plink = m_finalNodeLinksArray[i];

		// This should never, ever be the case
		assert(nodeIndexesMap[plink->sourcenode] != NO_POSITION);
		assert(nodeIndexesMap[plink->destnode] != NO_POSITION);

		plink->sourcenode = nodeIndexesMap[plink->sourcenode];
		plink->destnode = nodeIndexesMap[plink->destnode];
	}

	// Count total link entities
	Uint32 finalLinkEntityCount = 0;
	for(Uint32 i = 0; i < m_linkEntityArrayOfArrays.size(); i++)
		finalLinkEntityCount += m_linkEntityArrayOfArrays[i].size();

	if(finalLinkEntityCount > 0)
	{
		// Holds the map for link->link entity mappings
		CArray<Int32> linkLinkEntityIndexesMap(m_linkEntityArrayOfArrays.size());

		// Allocate final size link entities array
		m_finalLinkEntitiesArray.resize(finalLinkEntityCount);
		
		// Go through the link entities sequentially
		Uint32 finalLinkEntityOffset = 0;
		for(Uint32 i = 0; i < m_linkEntityArrayOfArrays.size(); i++)
		{
			if(m_linkEntityArrayOfArrays[i].empty())
			{
				linkLinkEntityIndexesMap[i] = NO_POSITION;
				continue;
			}

			for(Uint32 j = 0; j < m_linkEntityArrayOfArrays[i].size(); j++)
				m_finalLinkEntitiesArray[finalLinkEntityOffset + j] = m_linkEntityArrayOfArrays[i][j];

			linkLinkEntityIndexesMap[i] = finalLinkEntityOffset;
			finalLinkEntityOffset += m_linkEntityArrayOfArrays[i].size();
		}

		// Realign the indexes
		for(Uint32 i = 0; i < m_finalNodeLinksArray.size(); i++)
		{
			node_link_t* plink = m_finalNodeLinksArray[i];
			if(plink->linkentityindex == NO_POSITION)
				continue;

			plink->linkentityindex = linkLinkEntityIndexesMap[plink->linkentityindex];
		}
	}

	if(!m_linkEntityArrayOfArrays.empty())
		m_linkEntityArrayOfArrays.clear();

	// Log end
	filestr << "=============================================" << NEWLINE;
	filestr << " - Number of deleted links: " << (Int32)numDeletedLinks << NEWLINE;
	filestr << " - Number of deleted nodes: " << (Int32)numDeletedNodes << NEWLINE;
	filestr << " - Number of deleted link entities: " << (Int32)numDeletedLinkEntities << NEWLINE;
	filestr << " OptimizeGraph end" << NEWLINE;
	filestr << "=============================================" << NEWLINE;

	WriteToReportFile(filestr.c_str());
	filestr.clear();
}

//=============================================
// @brief
//
//=============================================
bool CAINodeGraph::WalkPath( const Vector& startPosition, const Vector& endPosition, CBaseEntity* pTestNPC, CArray<CBaseEntity*>& linkEntityArray, bool isWaterNode )
{
	// For storing linkentity states
	CEntityStateStack stack;

	// Walk mode depends on whether this is a water node
	walkmove_t walkMode = isWaterNode ? WALKMOVE_NORMAL : WALKMOVE_CHECKONLY;

	// Door entities arrays
	CDoorEntityStateStack doorStack;
	CArray<CBaseEntity*> doorEntitiesArray;
	CArray<CBaseEntity*> rotatingDoorEntitiesArray;
	bool needCheckReverse = false;
	bool checkingReverse = false;

	// True if a the walk failed
	bool walkFailed = false;

	// Do the walk
	while(true)
	{
		// Reset this
		walkFailed = false;

		// Move NPC to the node's position
		pTestNPC->SetOrigin(startPosition);

		// Only put onground if not fly/swim
		if(!(pTestNPC->GetFlags() & (FL_FLY|FL_SWIM)))
		{
			pTestNPC->GroundEntityNudge();
			pTestNPC->DropToFloor();
		}

		// Make sure we're not in a solid
		if(!pTestNPC->WalkMove(0, 0, walkMode))
		{
			walkFailed = true;
			break;
		}

		// Open func_door entities
		for(Uint32 i = 0; i < doorEntitiesArray.size(); i++)
		{
			CBaseEntity* pEntity = doorEntitiesArray[i];
			pEntity->SetToggleState(TS_AT_TOP, false);
		}

		// Open func_door_rotating entities
		for(Uint32 i = 0; i < rotatingDoorEntitiesArray.size(); i++)
		{
			CBaseEntity* pEntity = rotatingDoorEntitiesArray[i];
			pEntity->SetToggleState(TS_AT_TOP, checkingReverse);
		}

		if(!(pTestNPC->GetFlags() & (FL_FLY|FL_SWIM)))
		{
			// Get yaw and set angles on npc
			Vector forward = (endPosition - pTestNPC->GetOrigin()).Normalize();
			Float yaw = Util::VectorToYaw(forward);
			Vector testAngles = Math::VectorToAngles(forward);
			pTestNPC->SetAngles(testAngles);

			Float distance = (endPosition - pTestNPC->GetOrigin()).Length2D();
			for(Float step = 0; step < distance && !walkFailed; step += CBaseNPC::NPC_STEP_SIZE)
			{
				// Cap step size to remaining distance
				Float stepSize = CBaseNPC::NPC_STEP_SIZE;
				if((step + stepSize) >= (distance+1))
					stepSize = (distance-step) - 1;

				if(!pTestNPC->WalkMove(yaw, stepSize, walkMode))
				{
					walkFailed = true;
					break;
				}
			}

			// Make sure we actually made it there on Z too
			if(!walkFailed && !(pTestNPC->GetFlags() & FL_FLY) && SDL_fabs(endPosition.z - pTestNPC->GetOrigin().z) > CBaseNPC::NPC_TRIANGULATION_MAX_HEIGHT)
				walkFailed = true;
		}
		else
		{
			trace_t tr;
			Util::TraceHull(startPosition, endPosition, true, false, HULL_LARGE, nullptr, tr);
			if(!tr.noHit() || tr.startSolid() || tr.allSolid())
				walkFailed = true;
		}

		if(needCheckReverse && !walkFailed)
		{
			if(!checkingReverse)
			{
				// Start from beginning
				checkingReverse = true;
				continue;
			}
			else
			{
				// All walks succeeded
				checkingReverse = false;
			}
		}

		// If we didn't fail the walk, or if we failed it and the hit entity is not valid
		// then exit right now
		if(!walkFailed || Util::IsNullEntity(g_pGameVars->globaltrace.hitentity))
			break;

		CBaseEntity* pBlockingEntity = Util::GetEntityByIndex(g_pGameVars->globaltrace.hitentity);
		if(!pBlockingEntity || !IsLinkEntityManaged(pBlockingEntity))
			break;

		if(pBlockingEntity->IsFuncDoorEntity() || pBlockingEntity->IsFuncDoorRotatingEntity())
		{
			if(pBlockingEntity->IsFuncDoorRotatingEntity())
			{
				Uint32 i = 0;
				for(; i < rotatingDoorEntitiesArray.size(); i++)
				{
					if(rotatingDoorEntitiesArray[i] == pBlockingEntity)
						break;
				}
				
				if(i != rotatingDoorEntitiesArray.size())
					break;

				if(!needCheckReverse)
				{
					if(!(pBlockingEntity->GetSpawnFlags() & CFuncDoor::FL_START_OPEN)
						|| (pBlockingEntity->GetSpawnFlags() & CFuncDoorRotating::FL_ROTATE_REVERSE))
						needCheckReverse = true;
				}

				rotatingDoorEntitiesArray.push_back(pBlockingEntity);

				// Reset this
				if(needCheckReverse)
					checkingReverse = false;
			}
			else
			{
				Uint32 i = 0;
				for(; i < doorEntitiesArray.size(); i++)
				{
					if(doorEntitiesArray[i] == pBlockingEntity)
						break;
				}
				
				if(i != doorEntitiesArray.size())
					break;

				doorEntitiesArray.push_back(pBlockingEntity);
			}

			doorStack.SaveEntity(pBlockingEntity);
		}
		else
		{
			// Add blocker to the stack
			stack.SaveEntity(pBlockingEntity);

			// Get the entities relevant to this door link entity
			CArray<CBaseEntity*> secondaryLinkEntitesArray;
			Util::FindLinkEntities(pBlockingEntity, secondaryLinkEntitesArray, nullptr);

			// If we have something, hide those entities
			if(!secondaryLinkEntitesArray.empty())
			{
				for(Uint32 i = 0; i < secondaryLinkEntitesArray.size(); i++)
					stack.SaveEntity(secondaryLinkEntitesArray[i]);
			}
		}

		// Add the blocker to the list, if not already present
		Uint32 i = 0;
		for(; i < linkEntityArray.size(); i++)
		{
			if(linkEntityArray[i] == pBlockingEntity)
				break;
		}

		if(i == linkEntityArray.size())
			linkEntityArray.push_back(pBlockingEntity);
	}

	return walkFailed ? false : true;
}

//=============================================
// @brief
//
//=============================================
bool CAINodeGraph::HandleLinkEntity( Int32 nodeIndex, Int32 entityIndex, Uint64 capabilityBits, node_querytype_t queryType, const CBaseEntity* pNPC ) const
{
	if(Util::IsNullEntity(entityIndex))
		return true;

	// Don't spam about worldspawn
	if(entityIndex == WORLDSPAWN_ENTITY_INDEX)
		return false;

	// Get pointer to entity
	CBaseEntity* pEntity = Util::GetEntityByIndex(entityIndex);
	if(!pEntity || pEntity->GetSolidity() == SOLID_NOT || !pEntity->HasModelName())
		return true;

	if(!IsNodeGraphValid())
		return false;

	if(pEntity->IsFuncDoorEntity())
	{
		if(pEntity->IsLockedByMaster())
			return false;

		Int32 toggleState = pEntity->GetToggleState();
		if(pEntity->HasSpawnFlag(CFuncDoor::FL_USE_ONLY|CFuncDoor::FL_TOUCH_OPENS) || !pEntity->HasTargetName())
		{
			if((capabilityBits & CBaseNPC::AI_CAP_OPEN_DOORS) && !pEntity->HasSpawnFlag(CFuncDoor::FL_NO_NPCS) 
				|| toggleState == TS_AT_TOP && (pEntity->HasSpawnFlag(CFuncDoor::FL_NO_AUTO_RETURN) || pEntity->GetWaitTime() == -1))
				return true;
			else
				return false;
		}
		else
		{
			if(toggleState == TS_AT_TOP && (pEntity->HasSpawnFlag(CFuncDoor::FL_NO_AUTO_RETURN) || pEntity->GetWaitTime() == -1)
				|| (capabilityBits & CBaseNPC::AI_CAP_OPEN_DOORS) && (!pEntity->HasSpawnFlag(CFuncDoor::FL_NO_NPCS) || toggleState == TS_AT_TOP && (pEntity->HasSpawnFlag(CFuncDoor::FL_NO_AUTO_RETURN) || pEntity->GetWaitTime() == -1)))
				return true;
			else
				return false; 
		}
	}
	else if(pEntity->IsFuncNPCClipEntity() && (!pNPC || !(pNPC->GetFlags() & FL_NPC_CLIP) || pEntity->GetSolidity() == SOLID_NOT))
	{
		// This npc clip entity is passable now
		return true;
	}
	else if(pEntity->IsFuncIllusionaryEntity())
	{
		// Always pass func_illusionaries
		return true;
	}
	else if(pEntity->IsFuncWallToggleEntity() && pEntity->HasSpawnFlag(CFuncWallToggle::FL_NODE_IGNORE) && pEntity->GetSolidity() == SOLID_NOT)
	{
		// This func_wall_toggle is passable
		return true;
	}
	else if(pEntity->IsFuncTrainEntity() && pEntity->HasSpawnFlag(CFuncTrain::FL_DONT_BLOCK_NODES))
	{
		// This train doesn't block nodes
		return true;
	}

	if(!m_disableDebugMessages)
	{
		if(pEntity->HasTargetName())
			gd_engfuncs.pfnCon_VPrintf("%s - Unhandled link entity with classname '%s' and targetname '%s'.\n", __FUNCTION__, pEntity->GetClassName(), pEntity->GetTargetName());
		else
			gd_engfuncs.pfnCon_VPrintf("%s - Unhandled link entity with classname '%s'.\n", __FUNCTION__, pEntity->GetClassName());
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
const CAINodeGraph::node_t* CAINodeGraph::GetNode( Int32 nodeIndex )
{
	if(!m_pNodeHeader)
		return nullptr;

	if(nodeIndex >= (Int32)m_pNodeHeader->numnodes)
		return nullptr;

	return m_pNodeHeader->getNode(nodeIndex);
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::BuildStaticRoutingTables( Int16*& pRouteInfo, Uint32& routeInfoSize )
{
	Uint32 nbNodes = m_nodesArray.size();
	Int32 numRoutes = nbNodes*nbNodes;
	Int16* pRoutes = new Int16[numRoutes];

	Int32* pMyPath = new Int32[nbNodes];
	Uint16* pBestNextNodes = new Uint16[nbNodes];
	Int16* pRoute = new Int16[numRoutes];

	// Avoid spam about unhandled link entities
	m_disableDebugMessages = true;

	// Get pointer to nodes
	node_t* pNodes = reinterpret_cast<node_t*>(reinterpret_cast<byte*>(m_pNodeHeader) + m_pNodeHeader->nodeindex);

	Uint32 totalCompressedSize = 0;
	for(Uint32 hullType = 0; hullType < NUM_NODE_HULLS; hullType++)
	{
		for(Uint32 capability = NODE_CAP_NONE; capability < NB_CAP_INDEXES; capability++)
		{
			Uint64 capMask = 0;
			if(capability == NODE_CAP_DOORS_AND_USE)
				capMask |= (CBaseNPC::AI_CAP_OPEN_DOORS|CBaseNPC::AI_CAP_AUTO_OPEN_DOORS|CBaseNPC::AI_CAP_USE);

			for(Int32 fromNode = 0; fromNode < (Int32)nbNodes; fromNode++)
			{
				for(Int32 toNode = 0; toNode < (Int32)nbNodes; toNode++)
					pRoutes[NODE_FROM_TO(fromNode, toNode, nbNodes)] = NO_POSITION;
			}

			for(Int32 fromNode = 0; fromNode < (Int32)nbNodes; fromNode++)
			{
				for(Int32 toNode = nbNodes-1; toNode >= 0; toNode--)
				{
					if(pRoutes[NODE_FROM_TO(fromNode, toNode, nbNodes)] != NO_POSITION)
						continue;

					Uint32 nodeCount = GetShortestPath(fromNode, toNode, (node_hull_types_t)hullType, capMask, pMyPath);
					if(nodeCount > 1)
					{
						for(Uint32 node = 0; node < nodeCount-1; node++)
						{
							Int32 startNode = pMyPath[node];
							Int32 nextNode = pMyPath[node+1];

							for(Int32 node1 = node+1; node1 < (Int32)nodeCount; node1++)
							{
								Int32 endNode = pMyPath[node1];
								pRoutes[NODE_FROM_TO(startNode, endNode, nbNodes)] = nextNode;
							}
						}
					}
					else
					{
						pRoutes[NODE_FROM_TO(fromNode, toNode, nbNodes)] = fromNode;
						pRoutes[NODE_FROM_TO(toNode, fromNode, nbNodes)] = toNode;
					}
				}
			}

			for(Int32 fromNode = 0; fromNode < (Int32)nbNodes; fromNode++)
			{
				for(Int32 toNode = 0; toNode < (Int32)nbNodes; toNode++)
					pBestNextNodes[toNode] = pRoutes[NODE_FROM_TO(fromNode, toNode, nbNodes)];

				Int32 lastNode = 99999999;
				Int32 sequenceNb = 0;
				Int32 nbRepeats = 0;
				Uint32 compressedSize = 0;
				Int16* pData = pRoute;

				for(Int32 i = 0; i < (Int32)nbNodes; i++)
				{
					bool canRepeat = (pBestNextNodes[i] == lastNode && nbRepeats < 127);
					bool canSequence = (pBestNextNodes[i] == i && sequenceNb < 128);

					if(nbRepeats)
					{
						if(canRepeat)
						{
							nbRepeats++;
						}
						else
						{
							compressedSize += 2;
							(*pData) = nbRepeats - 1;
							pData++;

							Int32 a = (lastNode - fromNode);
							Int32 b = (lastNode - fromNode) + nbNodes;
							Int32 c = (lastNode - fromNode) - nbNodes;

							if(a >= -32768 && a <= 32767)
							{
								(*pData) = a;
								pData++;
							}
							else if(b >= -32768 && b <= 32767)
							{
								(*pData) = b;
								pData++;
							}
							else if(c >= -32768 && c <= 32767)
							{
								(*pData) = c;
								pData++;
							}
							else
								gd_engfuncs.pfnCon_Printf("%s - Nodes %d and %d need sorting.\n", __FUNCTION__, lastNode, fromNode);

							nbRepeats = 0;
							if(canSequence)
								sequenceNb++;
							else
								nbRepeats++;
						}
					}
					else if(sequenceNb)
					{
						if(canSequence)
						{
							sequenceNb++;
						}
						else
						{
							if(sequenceNb == 1 && canRepeat)
							{
								nbRepeats = 2;
								sequenceNb = 0;
							}
							else
							{
								compressedSize++;
								(*pData) = -sequenceNb;
								pData++;
								sequenceNb = 0;

								nbRepeats++;
							}
						}
					}
					else
					{
						if(canSequence)
							sequenceNb++;
						else
							nbRepeats++;
					}
					lastNode = pBestNextNodes[i];
				}

				if(nbRepeats)
				{
					compressedSize += 2;
					(*pData) = nbRepeats - 1;
					pData++;

					Int32 a = (lastNode - fromNode);
					Int32 b = (lastNode - fromNode) + nbNodes;
					Int32 c = (lastNode - fromNode) - nbNodes;

					if(a >= -32768 && a <= 32767)
					{
						(*pData) = a;
						pData++;
					}
					else if(b >= -32768 && b <= 32767)
					{
						(*pData) = b;
						pData++;
					}
					else if(c >= -32768 && c <= 32767)
					{
						(*pData) = c;
						pData++;
					}
					else
						gd_engfuncs.pfnCon_Printf("%s - Nodes %d and %d need sorting.\n", __FUNCTION__, lastNode, fromNode);
				}

				if(sequenceNb)
				{
					compressedSize++;
					(*pData) = -sequenceNb;
					pData++;
				}

				Int32 nbRoute = (pData-pRoute);
				if(pRouteInfo)
				{
					Int32 i = 0;
					for(; i < (Int32)routeInfoSize - nbRoute; i++)
					{
						if(!memcmp(pRouteInfo + i, pRoute, sizeof(Int16)*nbRoute))
							break;
					}

					if(i < (Int32)routeInfoSize - nbRoute)
					{
						pNodes[fromNode].nextbestnode[hullType][capability] = i;
					}
					else
					{
						Uint32 allocSize = routeInfoSize + nbRoute;
						Int16* pTemp = new Int16[allocSize];
						memcpy(pTemp, pRouteInfo, sizeof(Int16)*routeInfoSize);
						delete[] pRouteInfo;
						pRouteInfo = pTemp;

						memcpy(pRouteInfo + routeInfoSize, pRoute, sizeof(Int16)*nbRoute);
						pNodes[fromNode].nextbestnode[hullType][capability] = routeInfoSize;
						routeInfoSize += nbRoute;

						totalCompressedSize += compressedSize;
					}
				}
				else
				{
					routeInfoSize = nbRoute;
					pRouteInfo = new Int16[nbRoute];
					memcpy(pRouteInfo, pRoute, sizeof(Int16)*nbRoute);

					pNodes[fromNode].nextbestnode[hullType][capability] = 0;
					totalCompressedSize += compressedSize;
				}
			}
		}
	}

	CString filestr;
	filestr << "Routing info size: " << (Int32)totalCompressedSize << " bytes" << NEWLINE;
	WriteToReportFile(filestr.c_str());

	m_disableDebugMessages = false;

	delete[] pRoutes;
	delete[] pBestNextNodes;
	delete[] pRoute;
	delete[] pMyPath;
}

//=============================================
// @brief
//
//=============================================
CAINodeGraph::capability_indexes_t CAINodeGraph::GetCapabilityIndex( Uint64 capabilityBits )
{
	if(capabilityBits & (CBaseNPC::AI_CAP_OPEN_DOORS|CBaseNPC::AI_CAP_AUTO_OPEN_DOORS|CBaseNPC::AI_CAP_USE))
		return NODE_CAP_DOORS_AND_USE;
	else
		return NODE_CAP_NONE;
}

//=============================================
// @brief
//
//=============================================
bool CAINodeGraph::IsLinkEntityManaged( CBaseEntity* pEntity )
{
	if(pEntity->IsFuncIllusionaryEntity()
		|| pEntity->IsFuncDoorEntity()
		|| pEntity->IsFuncBreakableEntity()
		|| pEntity->IsFuncNPCClipEntity()
		|| pEntity->IsFuncWallToggleEntity()
		|| pEntity->IsFuncWallToggleEntity() && pEntity->HasSpawnFlag(CFuncWallToggle::FL_NODE_IGNORE)
		|| pEntity->IsFuncTrainEntity() && pEntity->HasSpawnFlag(CFuncTrain::FL_DONT_BLOCK_NODES))
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
Uint32 CAINodeGraph::GenHash( const byte* pData, Uint32 length )
{
	return CCRC32Hash(pData, length).GetHashResult();
}

//=============================================
// @brief
//
//=============================================
bool CAINodeGraph::IsNodeGraphValid( void ) const
{
	return (m_pNodeHeader == nullptr) ? false : true;
}

//=============================================
// @brief
//
//=============================================
Int32 CAINodeGraph::GetNumNodes( void ) const
{
	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - Graph not ready.\n", __FUNCTION__);
		return 0;
	}

	return (Int32)m_pNodeHeader->numnodes;
}

//=============================================
// @brief
//
//=============================================
Int32 CAINodeGraph::GetNumTempNodes( void ) const
{
	return m_nodesArray.size();
}

//=============================================
// @brief
//
//=============================================
const CAINodeGraph::node_link_t* CAINodeGraph::GetNodeLink( Int32 nodeIndex, Int32 linkIndex )
{
	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - Graph not ready.\n", __FUNCTION__);
		return nullptr;
	}

	node_t* pnode = m_pNodeHeader->getNode(nodeIndex);
	if(!pnode)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid node index %d.\n", __FUNCTION__, nodeIndex);
		return nullptr;
	}

	node_link_t* plink = m_pNodeHeader->getNodeLink(pnode, linkIndex);
	if(!plink)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid link index %d on node index %d.\n", __FUNCTION__, linkIndex, nodeIndex);
		return nullptr;
	}

	return plink;
}

//=============================================
// @brief
//
//=============================================
const CAINodeGraph::node_link_t* CAINodeGraph::GetNodeLink( const CAINodeGraph::node_t* pnode, Int32 linkIndex ) const
{
	assert(pnode != nullptr);

	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - Graph not ready.\n", __FUNCTION__);
		return nullptr;
	}

	const node_link_t* plink = m_pNodeHeader->getNodeLink(pnode, linkIndex);
	if(!plink)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid link index %d on node index %d.\n", __FUNCTION__, linkIndex, pnode->index);
		return nullptr;
	}

	return plink;
}

//=============================================
// @brief
//
//=============================================
bool CAINodeGraph::GetNodeLinkEntities( Int32 srcNode, Int32 linkNodeIndex, link_entity_t*& pLinkPtr, Uint32& numLinks )
{
	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - Graph not ready.\n", __FUNCTION__);
		return false;
	}

	if(srcNode == linkNodeIndex)
		return false;

	const node_t* pnode = GetNode(srcNode);
	if(!pnode)
	{
		gd_engfuncs.pfnCon_Printf("%s - Node with index %d not found.\n", __FUNCTION__, srcNode);
		return false;
	}

	const node_link_t* plink = nullptr;
	for(Int32 i = 0; i < (Int32)pnode->numlinks; i++)
	{
		plink = GetNodeLink(srcNode, i);
		if(!plink)
			continue;

		if(plink->destnode == linkNodeIndex)
			break;
	}

	if(!plink)
	{
		gd_engfuncs.pfnCon_Printf("%s - Node %d has no link to node %d.\n", __FUNCTION__, srcNode, linkNodeIndex);
		return false;
	}

	if(!plink->numlinkentities)
	{
		numLinks = 0;
		return true;
	}

	pLinkPtr = reinterpret_cast<link_entity_t*>(reinterpret_cast<byte*>(m_pNodeHeader) + m_pNodeHeader->linkentityoffset) + plink->linkentityindex;
	if(!pLinkPtr)
	{
		gd_engfuncs.pfnCon_Printf("%s - Couldn't get link entities for link node %d on node index %d.\n", __FUNCTION__, linkNodeIndex, srcNode);
		return false;
	}

	numLinks = plink->numlinkentities;
	return true;
}

//=============================================
// @brief
//
//=============================================
Int32 CAINodeGraph::GetNearestNode( const Vector& position, CBaseEntity* pEntity, const CBaseEntity* pTargetEntity, Float minDistance, const CNodeIgnoreList* pIgnoreList )
{
	Uint64 nodeTypes = pEntity ? Util::GetNodeTypeForNPC(pEntity) : AI_NODE_LAND;
	return GetNearestNode(position, nodeTypes, pEntity, pTargetEntity, minDistance, pIgnoreList);
}

//=============================================
// @brief
//
//=============================================
Int32 CAINodeGraph::GetNearestNode( const Vector& position )
{
	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - Graph not ready.\n", __FUNCTION__);
		return NO_POSITION;
	}

	Float lastClosestDistance = -1;
	Int32 lastClosestNodeIndex = NO_POSITION;

	for(Uint32 i = 0; i < m_pNodeHeader->numnodes; i++)
	{
		const node_t* pNode = GetNode(i);
		if(!pNode)
			return NO_POSITION;

		Float distance = (position - pNode->origin).Length();
		if(lastClosestNodeIndex == -1 || distance < lastClosestDistance)
		{
			lastClosestDistance = distance;
			lastClosestNodeIndex = i;
		}
	}

	return lastClosestNodeIndex;
}

//=============================================
// @brief
//
//=============================================
Int32 CAINodeGraph::GetNearestNode( const Vector& position, Uint64 nodeTypes, CBaseEntity* pEntity, const CBaseEntity* pTargetEntity, Float minDistance, const CNodeIgnoreList* pIgnoreList )
{
	if(!pEntity)
	{
		gd_engfuncs.pfnCon_Printf("%s - No entity specified.\n", __FUNCTION__);
		return NO_POSITION;
	}

	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - Graph not ready.\n", __FUNCTION__);
		return NO_POSITION;
	}

	Uint32 hash = (CACHE_SIZE-1) & GenHash((const byte*)&position, sizeof(Vector));

	cache_entry_t& cache = m_cache[hash];
	if(cache.position == position && cache.node != NO_POSITION && (!pIgnoreList || !pIgnoreList->IsNodeInList(cache.node)))
		return cache.node;

	m_nearestNodeDistance = -1;
	m_nearestNodeIndex = NO_POSITION;

	for(Int32 i = 0; i < (Int32)m_pNodeHeader->numnodes; i++)
	{
		if(pIgnoreList && pIgnoreList->IsNodeInList(i))
			continue;

		node_t* pnode = m_pNodeHeader->getNode(i);
		if(CheckNodeBBox(pnode, position, pEntity))
			continue;

		CheckNode(position, i, nodeTypes, pEntity, pTargetEntity, minDistance);
	}

	if(m_nearestNodeDistance == -1)
	{
		if(minDistance != -1)
			return GetNearestNode(position, nodeTypes, pEntity, pTargetEntity, -1, pIgnoreList);
		else if(nodeTypes & AI_NODE_AHEAD)
			return GetNearestNode(position, (nodeTypes & ~AI_NODE_AHEAD), pEntity, pTargetEntity, -1, pIgnoreList);
	}

	cache.position = position;
	cache.node = m_nearestNodeIndex;

	return m_nearestNodeIndex;
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::ShowNearestNodeBBox( const Vector& position, node_hull_types_t hullType )
{
	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - Graph not ready.\n", __FUNCTION__);
		return;
	}

	Int32 closestNodeIndex = NO_POSITION;
	Float closestNodeDistance = 0;

	for(Uint32 i = 0; i < m_pNodeHeader->numnodes; i++)
	{
		node_t* pNode = m_pNodeHeader->getNode(i);
		
		trace_t tr;
		Util::TraceLine(position, pNode->origin, true, false, nullptr, tr);
		if(!tr.noHit())
			continue;

		Float nodeDistance = (pNode->origin - position).Length();
		if(closestNodeIndex == NO_POSITION || nodeDistance < closestNodeDistance)
		{
			closestNodeDistance = nodeDistance;
			closestNodeIndex = i;
		}
	}

	if(closestNodeIndex == NO_POSITION)
	{
		gd_engfuncs.pfnCon_Printf("%s - No nearby node found.\n", __FUNCTION__);
		return;
	}

	node_t* pnode = m_pNodeHeader->getNode(closestNodeIndex);

	gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.nodedebug, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteByte(NODE_DEBUG_BBOX);
		for(Uint32 j = 0; j < 3; j++)
			gd_engfuncs.pfnMsgWriteFloat(pnode->origin[j]);
		for(Uint32 j = 0; j < 3; j++)
			gd_engfuncs.pfnMsgWriteFloat(pnode->mins[hullType][j]);
		for(Uint32 j = 0; j < 3; j++)
			gd_engfuncs.pfnMsgWriteFloat(pnode->maxs[hullType][j]);
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::ShowNodeBBoxes( node_hull_types_t hullType )
{
	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - Graph not ready.\n", __FUNCTION__);
		return;
	}

	if(hullType < 0 || hullType > NUM_NODE_HULLS)
		return;

	for(Int32 i = 0; i < (Int32)m_pNodeHeader->numnodes; i++)
	{
		node_t* pnode = m_pNodeHeader->getNode(i);

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.nodedebug, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(NODE_DEBUG_BBOX);
			for(Uint32 j = 0; j < 3; j++)
				gd_engfuncs.pfnMsgWriteFloat(pnode->origin[j]);
			for(Uint32 j = 0; j < 3; j++)
				gd_engfuncs.pfnMsgWriteFloat(pnode->mins[hullType][j]);
			for(Uint32 j = 0; j < 3; j++)
				gd_engfuncs.pfnMsgWriteFloat(pnode->maxs[hullType][j]);
		gd_engfuncs.pfnUserMessageEnd();
	}
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::ShowNodeConnections( Int32 nodeIndex )
{
	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - Graph not ready.\n", __FUNCTION__);
		return;
	}

	if(nodeIndex < 0 || nodeIndex > (Int32)m_pNodeHeader->numnodes)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid node index %d.\n", __FUNCTION__, nodeIndex);
		return;
	}

	node_t* pnode = m_pNodeHeader->getNode(nodeIndex);
	if(!pnode)
	{
		gd_engfuncs.pfnCon_Printf("%s - Couldn't get node %d.\n", __FUNCTION__, nodeIndex);
		return;
	}

	if(!pnode->numlinks)
	{
		gd_engfuncs.pfnCon_Printf("%s - Node %d has no links.\n", __FUNCTION__, nodeIndex);
		return;
	}

	for(Int32 i = 0; i < (Int32)pnode->numlinks; i++)
	{
		node_t* plinknode = m_pNodeHeader->getDestNodeFromLink(nodeIndex, pnode->firstlinkindex + i);
		if(!plinknode)
			continue;

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.nodedebug, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(NODE_DEBUG_PATHS);
			
			Vector pathStart = pnode->origin + NODE_PEEK_OFFSET;
			for(Uint32 j = 0; j < 3; j++)
				gd_engfuncs.pfnMsgWriteFloat(pathStart[j]);

			Vector pathEnd = plinknode->origin + NODE_PEEK_OFFSET;
			for(Uint32 j = 0; j < 3; j++)
				gd_engfuncs.pfnMsgWriteFloat(pathEnd[j]);

			gd_engfuncs.pfnMsgWriteByte(255);
			gd_engfuncs.pfnMsgWriteByte(0);
			gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnUserMessageEnd();
	}
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::ShowAllNodeConnections( void )
{
	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - Graph not ready.\n", __FUNCTION__);
		return;
	}

	for(Int32 i = 0; i < (Int32)m_pNodeHeader->numlinks; i++)
	{
		node_link_t* plink = m_pNodeHeader->getLink(i);
		if(!plink)
			continue;

		node_t* pstartnode = m_pNodeHeader->getNode(plink->sourcenode);
		if(!pstartnode)
			continue;

		node_t* pendnode = m_pNodeHeader->getNode(plink->destnode);
		if(!pendnode)
			continue;

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.nodedebug, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(NODE_DEBUG_PATHS);
			
			Vector pathStart = pstartnode->origin + NODE_PEEK_OFFSET;
			for(Uint32 j = 0; j < 3; j++)
				gd_engfuncs.pfnMsgWriteFloat(pathStart[j]);

			Vector pathEnd = pendnode->origin + NODE_PEEK_OFFSET;
			for(Uint32 j = 0; j < 3; j++)
				gd_engfuncs.pfnMsgWriteFloat(pathEnd[j]);

			gd_engfuncs.pfnMsgWriteByte(255);
			gd_engfuncs.pfnMsgWriteByte(0);
			gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnUserMessageEnd();
	}
}

//=============================================
// @brief
//
//=============================================
Int32 CAINodeGraph::GetShortestPath( Int32 startNode, Int32 endNode, node_hull_types_t hullType, Uint64 capabilityBits, Int32 *pNodeIndexArray, const CBaseEntity* pNPC, const CBaseEntity* pTargetEntity, const CNodeIgnoreList* pIgnoreList, const CBaseEntity* pViewCheckNPC )
{
	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - No graph available.\n", __FUNCTION__);
		return 0;
	}

	if(startNode < 0 || startNode >= (Int32)m_pNodeHeader->numnodes)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid value %d for startNode.\n", __FUNCTION__, startNode);
		return 0;
	}

	if(endNode < 0 || endNode >= (Int32)m_pNodeHeader->numnodes)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid value %d for endNode.\n", __FUNCTION__, endNode);
		return 0;
	}

	if(startNode == endNode)
	{
		if(pNodeIndexArray)
		{
			pNodeIndexArray[0] = startNode;
			pNodeIndexArray[1] = endNode;
		}
		return 2;
	}

	CPriorityNodeQueue queue;

	// Mark all nodes as unvisited
	for(Int32 i = 0; i < (Int32)m_pNodeHeader->numnodes; i++)
		m_pNodeHeader->getNode(i)->closestsofar = -1.0f;

	node_t* pstartnode = m_pNodeHeader->getNode(startNode);
	pstartnode->closestsofar = 0.0f;
	pstartnode->previousnode = startNode;
	queue.Insert(startNode, 0.0f);

	// Get hull mask for npc
	Uint64 hullMask = GetMaskForHull(hullType);

	while(!queue.IsEmpty())
	{
		Float currentDistance;
		Int32 currentNode = queue.Remove(currentDistance);

		if(currentNode == endNode)
			break;

		node_t* pcurrentnode = m_pNodeHeader->getNode(currentNode);

		Int32 i = 0;
		for(; i < (Int32)pcurrentnode->numlinks; i++)
		{
			// Get pointer to node we want to visit
			node_t* pvisitnode = m_pNodeHeader->getDestNodeFromLink(currentNode, i);
			if(!pvisitnode)
			{
				gd_engfuncs.pfnCon_Printf("%s - Invalid node index %d.\n", __FUNCTION__, currentNode);
				return 0;
			}

			// Check if it's on an ignore list if present
			if(pIgnoreList && pIgnoreList->IsNodeInList(pvisitnode->index))
				continue;

			// Get link for current node and link
			Int32 linkIndex = pcurrentnode->firstlinkindex + i;
			node_link_t* plink = m_pNodeHeader->getLink(linkIndex);
			if(!plink)
			{
				gd_engfuncs.pfnCon_Printf("%s - Invalid link index %d.\n", __FUNCTION__, linkIndex);
				return 0;
			}

			// This node's hull mask needs to match ours
			if((plink->linkinfobits & hullMask) != hullMask)
				continue;

			// check link entities if any
			if(plink->numlinkentities > 0)
			{
				Int32 j = 0;
				for(; j < (Int32)plink->numlinkentities; j++)
				{
					// Get the link entity data
					Int32 linkentityindex = plink->linkentityindex + j;
					link_entity_t* plinkentity = m_pNodeHeader->getLinkEntity(linkentityindex);
					if(!plinkentity)
					{
						gd_engfuncs.pfnCon_Printf("%s - Invalid link entity index %d.\n", __FUNCTION__, linkentityindex);
						return 0;
					}

					// If entity was removed, don't bother
					if(plinkentity->entityindex == NO_ENTITY_INDEX)
						continue;

					// Do not bother if link entity is on bigger hull than us
					if(plinkentity->hulltype > hullType)
						continue;

					if(!HandleLinkEntity(currentNode, plinkentity->entityindex, capabilityBits, GRAPH_QUERY_STATIC, pNPC))
						break;
				}

				// If an entity check failed, then ignore this path
				if(j != (Int32)plink->numlinkentities)
					continue;
			}

			// If there is a view check npc, see if they see this node or it's path
			if(pViewCheckNPC)
			{
				Vector vectorToNode = pvisitnode->origin - pcurrentnode->origin;

				Float fr = 0.0;
				for(; fr <= 1.0f; fr += 0.1)
				{
					Vector testPosition = pcurrentnode->origin + vectorToNode * fr;
					if(pViewCheckNPC->IsInView(testPosition))
						break;
				}

				// Path is visible by the threat, so ignore
				if(fr < 1.0)
					continue;
			}

			Float ourDistance = currentDistance + plink->weight;
			if(pvisitnode->closestsofar < -0.5f || ourDistance < pvisitnode->closestsofar - 0.001)
			{
				pvisitnode->closestsofar = ourDistance;
				pvisitnode->previousnode = currentNode;
				queue.Insert(pvisitnode->index, ourDistance);
			}
		}
	}

	// See if the destination node was touched at all
	node_t* pdestnode = m_pNodeHeader->getNode(endNode);
	if(pdestnode->closestsofar < -0.5f)
		return 0;

	if(!pNodeIndexArray)
		return 1;

	Int32 currentNode = endNode;
	Int32 numPathNodes = 1;

	while(currentNode != startNode)
	{
		node_t* pnode = m_pNodeHeader->getNode(currentNode);
		if(!pnode)
		{
			gd_engfuncs.pfnCon_Printf("%s - Invalid node index %d.\n", __FUNCTION__, currentNode);
			return 0;
		}

		currentNode = pnode->previousnode;
		numPathNodes++;
	}

	currentNode = endNode;
	for(Int32 i = numPathNodes - 1; i >= 0; i--)
	{
		node_t* pnode = m_pNodeHeader->getNode(currentNode);
		if(!pnode)
		{
			gd_engfuncs.pfnCon_Printf("%s - Invalid node index %d.\n", __FUNCTION__, currentNode);
			return 0;
		}

		pNodeIndexArray[i] = currentNode;
		currentNode = pnode->previousnode;
	}

	return numPathNodes;
}

//=============================================
// @brief
//
//=============================================
Float CAINodeGraph::GetPathLength( Int32 startNode, Int32 endNode, node_hull_types_t hullType, Uint64 capabilityMask )
{
	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - No graph available.\n", __FUNCTION__);
		return 0;
	}

	if(startNode < 0 || startNode >= (Int32)m_pNodeHeader->numnodes)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid value %d for startNode.\n", __FUNCTION__, startNode);
		return 0;
	}

	if(endNode < 0 || endNode >= (Int32)m_pNodeHeader->numnodes)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid value %d for endNode.\n", __FUNCTION__, endNode);
		return 0;
	}

	Float length = 0;
	Int32 maxLoop = m_pNodeHeader->numnodes;

	Int32 currentNode = startNode;
	capability_indexes_t capabilityIndex = GetCapabilityIndex(capabilityMask);

	while(currentNode != endNode)
	{
		maxLoop--;
		if(maxLoop <= 0)
		{
			gd_engfuncs.pfnCon_DPrintf("%s - Route failure.\n", __FUNCTION__);
			return 0;
		}

		Int32 nextNode = GetNextNodeInRoute(currentNode, endNode, hullType, capabilityIndex);
		if(currentNode == nextNode)
			return 0;

		Int32 linkIndex = SearchHash(currentNode, nextNode);
		if(linkIndex < 0)
		{
			gd_engfuncs.pfnCon_Printf("%s - Hash links are broken from %d to %d.\n", __FUNCTION__, currentNode, endNode);
			return 0;
		}

		node_link_t* plink = m_pNodeHeader->getLink(linkIndex);
		if(!plink)
		{
			gd_engfuncs.pfnCon_Printf("%s - Invalid node link with index %d.\n", __FUNCTION__, linkIndex);
			return 0;
		}

		length += plink->weight;
		currentNode = nextNode;
	}

	return length;
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::CheckNode( const Vector& origin, Int32 nodeIndex, Uint64 nodeTypes, CBaseEntity* pEntity, const CBaseEntity* pTargetEntity, Float minDistance )
{
	node_t* pnode = m_pNodeHeader->getNode(nodeIndex);
	if(!pnode)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid node index %d.\n", __FUNCTION__, nodeIndex);
		return;
	}

	// If we only want nodes ahead, don't allow nodes behind us
	if(nodeTypes & AI_NODE_AHEAD)
	{
		Vector forward;
		Math::AngleVectors(pEntity->GetAngles(), &forward);

		Vector direction = (origin - pnode->origin).Normalize();
		if(Math::DotProduct(forward, direction) < 0)
			return;
	}

	// Make sure types match if we have specified a type
	if(nodeTypes != 0 && !(pnode->nodeinfobits & nodeTypes))
		return;

	// Check min/max distance if set
	bool result = false;
	Float distance = (origin-pnode->origin).Length();
	if(distance > m_nearestNodeDistance && m_nearestNodeDistance != -1)
		return;

	// Check mindistance if set
	if(minDistance != -1 && distance < minDistance)
		return;

	// Factor in NPCs for path testing
	if(pEntity && pEntity->IsNPC())
	{
		localmove_t moveResult = pEntity->CheckLocalMove(origin, pnode->origin, pTargetEntity, nullptr, true);
		if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
			result = true;
	}
	else
	{
		const edict_t* pedict = pEntity ? pEntity->GetEdict() : nullptr;

		trace_t tr;
		Util::TraceLine(origin, pnode->origin + NODE_PEEK_OFFSET, false, false, pedict, tr);
		if(tr.noHit() || tr.hitentity != NO_ENTITY_INDEX && pTargetEntity && tr.hitentity == pTargetEntity->GetEntityIndex())
			result = true;
	}

	if(result)
	{
		m_nearestNodeIndex = nodeIndex;
		m_nearestNodeDistance = distance;
	}
}

//=============================================
// @brief
//
//=============================================
bool CAINodeGraph::CheckNodeBBox( node_t* pnode, const Vector& origin, CBaseEntity* pNPC )
{
	node_hull_types_t hullType = Util::GetNodeHullForNPC(pNPC);

	Vector mins = origin + pNPC->GetMins();
	Vector maxs = origin + pNPC->GetMaxs();

	return Math::CheckMinsMaxs(mins, maxs, pnode->mins[hullType], pnode->maxs[hullType]);
}

//=============================================
// @brief
//
//=============================================
bool CAINodeGraph::IsValidCoverPath( Int32 startNode, Int32 endNode, node_hull_types_t hullType, Uint64 capabilityMask, const CBaseEntity* pNPC, const CBaseEntity* pThreatEntity )
{
	Int32 result = GetShortestPath(startNode, endNode, hullType, capabilityMask, nullptr, pNPC, nullptr, nullptr, pThreatEntity);
	if(result == 0)
		return false;
	else
		return true;
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::AddNode( const Vector& origin, Float hintYaw, Int32 hintType, Int32 hintActivity, Float range, const Char* pstrRegionName, bool isAirNode, bool disableOptimization )
{
	node_t* pnode = new node_t;
	pnode->origin = origin;
	pnode->hintyaw = hintYaw;
	pnode->hinttype = hintType;
	pnode->hintactivity = hintActivity;
	pnode->linkrange = range;
	
	if(disableOptimization)
		pnode->nodeflags |= NODE_FL_NO_OPTIMIZE;

	qstrcpy(pnode->noderegionname, pstrRegionName);

	if(isAirNode)
		pnode->nodeinfobits |= AI_NODE_AIR;

	m_nodesArray.push_back(pnode);
}

//=============================================
// @brief
//
//=============================================
Int32 CAINodeGraph::GetNextNodeInRoute( Int32 currentNode, Int32 destNode, node_hull_types_t hullType, capability_indexes_t capabilityIndex )
{
	if(!IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("%s - No graph available.\n", __FUNCTION__);
		return currentNode;
	}

	if(currentNode < 0 || currentNode >= (Int32)m_pNodeHeader->numnodes)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid value %d for currentNode.\n", __FUNCTION__, currentNode);
		return currentNode;
	}

	if(destNode < 0 || destNode >= (Int32)m_pNodeHeader->numnodes)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid value %d for destNode.\n", __FUNCTION__, destNode);
		return currentNode;
	}

	node_t* pnode = m_pNodeHeader->getNode(currentNode);
	if(!pnode)
		return currentNode;

	Int32 nextNode = currentNode;
	Int16* pRoute = reinterpret_cast<Int16*>(reinterpret_cast<byte*>(m_pNodeHeader) + m_pNodeHeader->routeinfoindex);
	pRoute += pnode->nextbestnode[hullType][capabilityIndex];

	Int32 nodeCount = destNode+1;
	while(nodeCount > 0)
	{
		Int16 data = (*pRoute);
		pRoute++;

		if(data < 0)
		{
			data = -data;
			if(nodeCount <= data)
			{
				nextNode = destNode;
				nodeCount = 0;
			}
			else
				nodeCount = nodeCount - data;
		}
		else
		{
			if(nodeCount <= (data+1))
			{
				nextNode = currentNode + (*pRoute);
				if(nextNode >= (Int32)m_pNodeHeader->numnodes)
					nextNode -= (Int32)m_pNodeHeader->numnodes;
				else if(nextNode < 0)
					nextNode += (Int32)m_pNodeHeader->numnodes;

				nodeCount = 0;
			}
			else
				nodeCount = nodeCount - data - 1;

			pRoute++;
		}
	}

	return nextNode;
}

//=============================================
// @brief
//
//=============================================
void CAINodeGraph::WriteToReportFile( const Char* pstrString )
{
	if(!m_pLogFile->Write(pstrString))
		gd_engfuncs.pfnCon_EPrintf("%s - Error writing to log file.\n", __FUNCTION__);
}

//=============================================
// @brief
//
//=============================================
Uint64 CAINodeGraph::GetMaskForHull( node_hull_types_t type )
{
	Uint64 hullMask;
	switch(type)
	{
	case NODE_SMALL_HULL:
		hullMask = LINK_SMALL_HULL;
		break;
	case NODE_HUMAN_HULL:
		hullMask = LINK_HUMAN_HULL;
		break;
	case NODE_LARGE_HULL:
		hullMask = LINK_LARGE_HULL;
		break;
	case NODE_FLY_HULL:
		hullMask = LINK_FLY_HULL;
		break;
	default:
		hullMask = 0;
		break;
	}

	return hullMask;
}
