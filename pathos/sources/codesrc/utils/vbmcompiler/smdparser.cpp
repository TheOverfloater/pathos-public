/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <SDL.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include "includes.h"
#include "studiocompiler.h"
#include "smdparser.h"
#include "options.h"
#include "main.h"
#include "compiler_math.h"
#include "main.h"
#include "logfile.h"

//===============================================
// @brief Constructor for CSMDParser class
//
// @param compiler Reference to studiomodel compiler object
//===============================================
CSMDParser::CSMDParser( CStudioModelCompiler& compiler ):
	m_studioCompiler(compiler)
{
}

//===============================================
// @brief Destructor for CSMDParser class
//
//===============================================
CSMDParser::~CSMDParser( void )
{
}

//===============================================
// @brief Parses bone nodes from the SMD file loaded
//
// @param destArray Destination array for nodes
// @return TRUE if successful, FALSE if an error occurred
//===============================================
bool CSMDParser::ParseNodes( CArray<smdl::bone_node_t>& destArray )
{
	const Char* pstrString = nullptr;
	while(true)
	{
		// Needs to be read as string, as it can either be a number
		// or the 'end' command that terminates parsing
		if(!ReadString(pstrString, true))
		{
			ErrorMsg("Incomplete 'nodes' block, missing 'end'.\n");
			return false;
		}

		// Break if reached end
		if(!qstrcicmp(pstrString, "end"))
			break;

		if(!Common::IsNumber(pstrString))
		{
			ErrorMsg("Expected numerical value for node index, got '%s' instead.\n", pstrString);
			return false;
		}

		Int32 nodeIndex = SDL_atoi(pstrString);

		CString nodeName;
		if(!ReadString(nodeName))
		{
			ErrorMsg("Couldn't read node name for index %d.\n", nodeIndex);
			return false;
		}

		Int32 parentIndex = NO_POSITION;
		if(!ReadInt32(parentIndex))
		{
			ErrorMsg("Couldn't read parent index for node '%s' with index '%d'.\n", nodeName.c_str(), nodeIndex);
			return false;
		}

		// Make sure it's not a duplicate
		if(g_pLogFile)
		{
			for(Uint32 i = 0; i < destArray.size(); i++)
			{
				if(destArray[i].bonename.empty())
					continue;

				if(!qstrcicmp(destArray[i].bonename, nodeName) && g_pLogFile)
					g_pLogFile->Printf("Node with name '%s' already defined at position %d.\n", nodeName.c_str(), i);
			}
		}

		// Ensure size is valid
		if(nodeIndex >= destArray.size())
			destArray.resize(nodeIndex+1);

		smdl::bone_node_t newNode;
		newNode.bonename = nodeName;
		newNode.parentindex = parentIndex;

		if(m_studioCompiler.IsMirroredBone(nodeName.c_str()))
			newNode.ismirrored = true;

		destArray[nodeIndex] = newNode;
	}

	// Check consistency for loaded nodes
	for(Uint32 i = 0; i < destArray.size(); i++)
	{
		smdl::bone_node_t& node = destArray[i];
		if(node.parentindex == NO_POSITION)
			continue;

		if(node.parentindex >= destArray.size())
		{
			ErrorMsg("Invalid parent index %d for node '%s' at index %d.\n", node.parentindex, node.bonename.c_str(), i);
			return false;
		}
		else if(node.bonename.empty())
		{
			ErrorMsg("Parent of node '%s' at index %d was not defined.\n", node.bonename.c_str(), i);
			return false;
		}

		// Ensure consistency with mirroring
		if(!node.ismirrored)
		{
			smdl::bone_node_t& nodeParent = destArray[node.parentindex];
			if(nodeParent.ismirrored)
				node.ismirrored = true;
		}
	}

	return true;
}

//===============================================
// @brief Parses skeleton from the SMD file loaded
//
// @param nodesarray Nodes that we are loading for
// @param bonesarray Destination bones array
// @param boneimap Array that'll hold our inverse bone map
// @param transformsarray Array that'll hold our transform matrix info
// @return TRUE if successful, FALSE if an error occurred
//===============================================
bool CSMDParser::ParseSkeleton( CArray<smdl::bone_node_t>& nodesarray, 
	CArray<smdl::bone_t>& bonesarray, 
	CArray<Int32>& boneimap, 
	CArray<smdl::bone_transforminfo_t>& transformsarray )
{
	if(nodesarray.empty())
	{
		ErrorMsg("No nodes were loaded prior to 'skeleton' block.\n");
		return false;
	}

	// Resize these to match
	bonesarray.resize(nodesarray.size());
	boneimap.resize(nodesarray.size());

	for(Uint32 i = 0; i < boneimap.size(); i++)
		boneimap[i] = NO_POSITION;

	// First we expect a time setting of '0'
	const Char* pstrString = nullptr;
	if(!ReadString(pstrString, true))
	{
		ErrorMsg("Could not read 'time' token for 'skeleton' block.\n");
		return false;
	}

	if(qstrcicmp(pstrString, "time"))
	{
		ErrorMsg("Expected 'time' token after 'skeleton', got '%s' instead.\n", pstrString);
		return false;
	}

	// Now read the frame index value
	Int32 frameIndex = 0;
	if(!ReadInt32(frameIndex))
	{
		ErrorMsg("Couldn't read frame index value for 'skeleton'.\n");
		return false;
	}

	if(frameIndex != 0)
	{
		ErrorMsg("Frame index was %d instead of 0.\n", frameIndex);
		return false;
	}

	while(true)
	{
		// Needs to be read as raw string, as it can be either
		// a number, or the 'end' that terminates parsing
		if(!ReadString(pstrString, true))
		{
			ErrorMsg("Incomplete 'nodes' block, missing 'end'.\n");
			return false;
		}
		
		if(!qstrcicmp(pstrString, "end"))
		{
			// Break if reached end
			break;
		}
		else if(!qstrcicmp(pstrString, "time"))
		{
			ErrorMsg("Multiple frames encountered in skeleton block, SMD is not valid.\n", pstrString);
			return false;
		}

		// Read node index
		if(!Common::IsNumber(pstrString))
		{
			ErrorMsg("Expected node index, got '%s' instead.\n", pstrString);
			return false;
		}

		Int32 nodeIndex = SDL_atoi(pstrString);
		if(nodeIndex < 0 || nodeIndex >= nodesarray.size())
		{
			ErrorMsg("Invalid node index %d in 'skeleton' block.\n", nodeIndex);
			return false;
		}

		smdl::bone_node_t& node = nodesarray[nodeIndex];

		// Read position
		Vector position;
		for(Uint32 i = 0; i < 3; i++)
		{
			if(!ReadFloat(position[i]))
			{
				ErrorMsg("Couldn't read node position value %d for node '%s' in 'skeleton' block.\n", (i+1), node.bonename.c_str());
				return false;
			}
		}

		// Read rotation
		Vector rotation;
		for(Uint32 i = 0; i < 3; i++)
		{
			if(!ReadFloat(rotation[i]))
			{
				ErrorMsg("Couldn't read node rotation value %d for node '%s' in 'skeleton' block.\n", (i+1), node.bonename.c_str());
				return false;
			}
		}

		// Check if we need to reverse it
		if(node.ismirrored)
			Math::VectorScale(position, -1, position);

		// Set values in array
		smdl::bone_t& bone = bonesarray[nodeIndex];
		bone.position = m_studioCompiler.ApplyScaling(position);
		bone.rotation = m_studioCompiler.ClipRotations(rotation);
	}

	// Set up the skeleton
	SetupSkeleton(nodesarray, bonesarray, boneimap, transformsarray);

	return true;
}

//===============================================
// @brief Sets up the skeleton and it's matrices
//
// @param nodesarray Nodes that we are setting up for
// @param bonesarray Array of bones
// @param boneimap Array holds our inverse bone map
// @param transformsarray Array holds our transform matrix info
//===============================================
void CSMDParser::SetupSkeleton( 
	CArray<smdl::bone_node_t>& nodesarray, 
	CArray<smdl::bone_t>& bonesarray, 
	CArray<Int32>& boneimap, 
	CArray<smdl::bone_transforminfo_t>& transformsarray )
{
	transformsarray.resize(bonesarray.size());

	for(Uint32 i = 0; i < transformsarray.size(); i++)
	{
		const smdl::bone_node_t& boneNode = nodesarray[i];
		const smdl::bone_t& bone = bonesarray[i];

		CompilerMath::SetupBoneTransform(i, boneNode.parentindex, bone.position, bone.rotation, transformsarray);
	}
}