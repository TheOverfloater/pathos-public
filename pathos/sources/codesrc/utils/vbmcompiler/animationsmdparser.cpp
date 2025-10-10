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
#include "animationsmdparser.h"
#include "options.h"
#include "main.h"

//===============================================
// @brief Constructor for CAnimationSMDParser class
//
// @param compiler Reference to studiomodel compiler object
// @param panimation Pointer to destination animation object
// @param zrotation The z rotation value to transform with
//===============================================
CAnimationSMDParser::CAnimationSMDParser( CStudioModelCompiler& compiler, smdl::animation_t* panimation, Float zrotation ):
	CSMDParser(compiler),
	m_zRotation(zrotation),
	m_pAnimation(panimation)
{
}

//===============================================
// @brief Destructor for CAnimationSMDParser class
//
//===============================================
CAnimationSMDParser::~CAnimationSMDParser( void )
{
}

//===============================================
// @brief Loads and processes an SMD file
//
// @param pstrFilename File path
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CAnimationSMDParser::ProcessFile( const Char* pstrFilename )
{
	CString filePath;
	filePath << pstrFilename;
	
	if(filePath.find(0, ".smd", true) == CString::CSTRING_NO_POSITION)
		filePath << ".smd";

	if(!OpenScriptFile(filePath.c_str()))
	{
		ErrorMsg("Failed to open '%s' for processing.\n", filePath.c_str());
		Clear();
		return false;
	}

	m_pAnimation->name = pstrFilename;

	Msg("Processing '%s'.\n", m_scriptFileName.c_str());

	// First read the version
	const Char* pstrString = nullptr;
	if(!ReadString(pstrString))
	{
		ErrorMsg("Failed to read 'version' token in %s'.\n", m_scriptFileName.c_str());
		Clear();
		return false;
	}

	if(qstrcicmp(pstrString, "version"))
	{
		ErrorMsg("Expected 'version', got '%s' instead in '%s'.\n", pstrString, m_scriptFileName.c_str());
		Clear();
		return false;
	}

	// Now read the version number
	Int32 versionNumber = 0;
	if(!ReadInt32(versionNumber))
	{
		ErrorMsg("Failed to read 'version' number token in %s'.\n", m_scriptFileName.c_str());
		Clear();
		return false;
	}

	if(versionNumber != SMD_VERSION)
	{
		ErrorMsg("SMD file '%s' has invalid version %d, %d expected.\n", m_scriptFileName.c_str(), versionNumber, SMD_VERSION);
		Clear();
		return false;
	}

	// Now parse the contents of the script
	while(true)
	{
		if(!ReadString(pstrString, true))
			break;

		bool result = false;
		if(!qstrcicmp(pstrString, "nodes"))
		{
			result = ParseNodes(m_pAnimation->nodes);
		}
		else if(!qstrcicmp(pstrString, "skeleton"))
		{
			result = ParseAnimation();
		}
		else
		{
			ErrorMsg("Unknown token '%s' in %s'.\n", pstrString, m_scriptFileName.c_str());
			Clear();
			return false;
		}

		if(!result)
		{
			ErrorMsg("Failure encountered when parsing '%s'.\n", m_scriptFileName.c_str());
			Clear();
			return false;
		}
	}

	return true;
}

//===============================================
// @brief Parses the animation data from the SMD file
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CAnimationSMDParser::ParseAnimation( void )
{
	Float cosz = SDL_cos(m_zRotation);
	Float sinz = SDL_sin(m_zRotation);

	Int32 frameMin = -1;
	Int32 frameMax = -1;
	Int32 frameIndex = -1;

	const Char* pstrString = nullptr;
	while(true)
	{
		// Needs to be read as raw string, as it can be either
		// a number, or the 'end' that terminates parsing
		if(!ReadString(pstrString, true))
		{
			ErrorMsg("Incomplete 'nodes' block, missing 'end'.\n");
			return false;
		}

		// Break if reached end
		if(!qstrcicmp(pstrString, "end"))
		{
			// Reached end of animation block
			break;
		}
		else if(!qstrcicmp(pstrString, "time"))
		{
			// Read in the frame index
			if(!ReadInt32(frameIndex))
			{
				ErrorMsg("Couldn't read frame index value.\n");
				return false;
			}

			// Resize arrays to match new frame count
			if(frameIndex >= m_pAnimation->startframe && (m_pAnimation->endframe == -1 || frameIndex <= m_pAnimation->endframe))
			{
				if(m_pAnimation->pos_values.size() <= frameIndex)
				{
					Uint32 newStart = m_pAnimation->pos_values.size() < m_pAnimation->startframe ? m_pAnimation->startframe : m_pAnimation->pos_values.size();
					m_pAnimation->pos_values.resize(frameIndex+1);

					for(Uint32 j = newStart; j < m_pAnimation->pos_values.size(); j++)
						m_pAnimation->pos_values[j] = new CArray<Vector>(m_pAnimation->nodes.size());
				}

				if(m_pAnimation->rot_values.size() <= frameIndex)
				{
					Uint32 newStart = m_pAnimation->rot_values.size() < m_pAnimation->startframe ? m_pAnimation->startframe : m_pAnimation->rot_values.size();
					m_pAnimation->rot_values.resize(frameIndex+1);

					for(Uint32 j = newStart; j < m_pAnimation->rot_values.size(); j++)
						m_pAnimation->rot_values[j] = new CArray<Vector>(m_pAnimation->nodes.size());
				}
			}
		}
		else
		{
			// Read node index
			if(!Common::IsNumber(pstrString))
			{
				ErrorMsg("Expected node index, got '%s' instead.\n", pstrString);
				return false;
			}

			Int32 nodeIndex = SDL_atoi(pstrString);
			if(nodeIndex < 0 || nodeIndex >= m_pAnimation->nodes.size())
			{
				ErrorMsg("Invalid node index %d in 'skeleton' block.\n", nodeIndex);
				return false;
			}

			smdl::bone_node_t& node = m_pAnimation->nodes[nodeIndex];

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

			// Only store it if it's in the cap
			if(frameIndex >= m_pAnimation->startframe && (m_pAnimation->endframe == -1 || frameIndex <= m_pAnimation->endframe))
			{
				Vector& positionValue = (*m_pAnimation->pos_values[frameIndex])[nodeIndex];
				if(node.parentindex == NO_POSITION)
				{
					position = m_studioCompiler.ApplyOffset(position);

					// Rotate the position
					positionValue[0] = cosz * position[0] - sinz * position[1];
					positionValue[1] = sinz * position[0] + cosz * position[1];
					positionValue[2] = position[2];

					// Rotate the value
					rotation[2] += m_zRotation;
				}
				else
				{
					// Just set the position
					positionValue = position;
				}

				// Mark cap values
				if(frameMin == -1 || frameIndex < frameMin)
					frameMin = frameIndex;

				if(frameMax == -1 || frameIndex > frameMax)
					frameMax = frameIndex;

				if(node.ismirrored)
					Math::VectorScale(positionValue, -1, positionValue);

				// Apply scaling to position
				positionValue = m_studioCompiler.ApplyScaling(positionValue);

				// Set rotation
				(*m_pAnimation->rot_values[frameIndex])[nodeIndex] = m_studioCompiler.ClipRotations(rotation);
			}
		}
	}

	// Shift animation frames if needed
	if(frameMin > 0)
	{
		for(Int32 i = frameMin - 1; i >= 0; i--)
		{
			if(m_pAnimation->pos_values[i])
				delete m_pAnimation->pos_values[i];

			m_pAnimation->pos_values.erase(i);

			if(m_pAnimation->rot_values[i])
				delete m_pAnimation->rot_values[i];

			m_pAnimation->rot_values.erase(i);
		}
	}

	// Set min/max values for frames
	m_pAnimation->startframe = frameMin;
	m_pAnimation->endframe = frameMax;

	return true;
}