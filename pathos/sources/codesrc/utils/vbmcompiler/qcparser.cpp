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
#include "qcparser.h"
#include "options.h"
#include "main.h"
#include "filefuncs.h"

#define DEFINE_PROCESSOR_FN(name, function)		CQCParser::ProcessorFnMapping_t(#name, function)
#define DEFINE_CONTROLLER_TYPE(id, typeFlag)	CQCParser::ControllerTypeMapping_t(#id, typeFlag)

// Activity mappings filename
const Char CQCParser::ACTIVITY_DEFINITIONS_FILENAME[] = "activitymap.txt";

// Bone controller type mappings
const CQCParser::ControllerTypeMapping_t CQCParser::CONTROLLER_TYPE_MAPPINGS[] = {
	DEFINE_CONTROLLER_TYPE(X,		STUDIO_X),
	DEFINE_CONTROLLER_TYPE(Y,		STUDIO_Y),
	DEFINE_CONTROLLER_TYPE(Z,		STUDIO_Z),
	DEFINE_CONTROLLER_TYPE(XR,		STUDIO_XR),
	DEFINE_CONTROLLER_TYPE(YR,		STUDIO_YR),
	DEFINE_CONTROLLER_TYPE(ZR,		STUDIO_ZR),
	DEFINE_CONTROLLER_TYPE(LX,		STUDIO_LX),
	DEFINE_CONTROLLER_TYPE(LY,		STUDIO_LY),
	DEFINE_CONTROLLER_TYPE(LZ,		STUDIO_LZ),
	DEFINE_CONTROLLER_TYPE(AX,		STUDIO_AX),
	DEFINE_CONTROLLER_TYPE(AY,		STUDIO_AY),
	DEFINE_CONTROLLER_TYPE(AZ,		STUDIO_AZ),
	DEFINE_CONTROLLER_TYPE(AXR,		STUDIO_AXR),
	DEFINE_CONTROLLER_TYPE(AYR,		STUDIO_AYR),
	DEFINE_CONTROLLER_TYPE(AZR,		STUDIO_AZR),
};

// Processor function name->function ptr mappings
const CQCParser::ProcessorFnMapping_t CQCParser::COMMAND_PROCESSOR_FUNCTIONS[] = {
	DEFINE_PROCESSOR_FN($modelname,			&Cmd_ModelName),
	DEFINE_PROCESSOR_FN($modelname,			&Cmd_ModelName),
	DEFINE_PROCESSOR_FN($cd,				&Cmd_ChangeDirectory),
	DEFINE_PROCESSOR_FN($cdtexture,			&Cmd_ChangeTextureDirectory),
	DEFINE_PROCESSOR_FN($scale,				&Cmd_Scale),
	DEFINE_PROCESSOR_FN($scale_movement,	&Cmd_ScaleMovement),
	DEFINE_PROCESSOR_FN($root,				&Cmd_Root),
	DEFINE_PROCESSOR_FN($pivot,				&Cmd_Pivot),
	DEFINE_PROCESSOR_FN($controller,		&Cmd_BoneController),
	DEFINE_PROCESSOR_FN($lod,				&Cmd_LOD),
	DEFINE_PROCESSOR_FN($body,				&Cmd_Body),
	DEFINE_PROCESSOR_FN($flexcontroller,	&Cmd_FlexController),
	DEFINE_PROCESSOR_FN($bodygroup,			&Cmd_BodyGroup),
	DEFINE_PROCESSOR_FN($sequence,			&Cmd_Sequence),
	DEFINE_PROCESSOR_FN($eyeposition,		&Cmd_EyePosition),
	DEFINE_PROCESSOR_FN($origin,			&Cmd_Origin),
	DEFINE_PROCESSOR_FN($bbox,				&Cmd_BBox),
	DEFINE_PROCESSOR_FN($cbox,				&Cmd_CBox),
	DEFINE_PROCESSOR_FN($mirrorbone,		&Cmd_MirrorBone),
	DEFINE_PROCESSOR_FN($gamma,				&Cmd_Gamma),
	DEFINE_PROCESSOR_FN($flags,				&Cmd_Flags),
	DEFINE_PROCESSOR_FN($texturegroup,		&Cmd_TextureGroup),
	DEFINE_PROCESSOR_FN($hgroup,			&Cmd_HitGroup),
	DEFINE_PROCESSOR_FN($hbox,				&Cmd_HitBox),
	DEFINE_PROCESSOR_FN($attachment,		&Cmd_Attachment),
	DEFINE_PROCESSOR_FN($renamebone,		&Cmd_RenameBone),
	DEFINE_PROCESSOR_FN($centerlight,		&Cmd_CenterLight),
	DEFINE_PROCESSOR_FN($noblend,			&Cmd_NoBlendBone),
	DEFINE_PROCESSOR_FN($protected,			&Cmd_ProtectedBone),
	DEFINE_PROCESSOR_FN($texrendermode,		&Cmd_TextureRenderMode),
	DEFINE_PROCESSOR_FN($movement_bone,		&Cmd_MovementBone),
	DEFINE_PROCESSOR_FN($skylight,			&Cmd_SkyLight),
	DEFINE_PROCESSOR_FN($cliptotextures,	&Cmd_ClipToTextures),
};

// Option processor function name->function ptr mappings
const CQCParser::ProcessorFnMapping_t CQCParser::SEQUENCE_OPTION_PROCESSOR_FUNCTIONS[] = {
	DEFINE_PROCESSOR_FN(deform,				&SeqOption_Deform),
	DEFINE_PROCESSOR_FN(event,				&SeqOption_Event),
	DEFINE_PROCESSOR_FN(pivot,				&SeqOption_Pivot),
	DEFINE_PROCESSOR_FN(fps,				&SeqOption_FPS),
	DEFINE_PROCESSOR_FN(origin,				&SeqOption_Origin),
	DEFINE_PROCESSOR_FN(rotate,				&SeqOption_Rotate),
	DEFINE_PROCESSOR_FN(scale,				&SeqOption_Scale),
	DEFINE_PROCESSOR_FN(movement_scale,		&SeqOption_MovementScale),
	DEFINE_PROCESSOR_FN(loop,				&SeqOption_Loop),
	DEFINE_PROCESSOR_FN(frame,				&SeqOption_Frame),
	DEFINE_PROCESSOR_FN(blend,				&SeqOption_Blend),
	DEFINE_PROCESSOR_FN(node,				&SeqOption_Node),
	DEFINE_PROCESSOR_FN(transition,			&SeqOption_Transition),
	DEFINE_PROCESSOR_FN(rtransition,		&SeqOption_ReverseTransition),
	DEFINE_PROCESSOR_FN(animation,			&SeqOption_Animation),
};

//===============================================
// @brief Constructor for CQCParser class
//
// @param studioCompiler Reference to studimodel compiler object
// @param pVBMCompiler Pointer to VBM compiler object
// @param pstrExecutablePath Executable path
//===============================================
CQCParser::CQCParser( CStudioModelCompiler& studioCompiler, CVBMCompiler* pVBMCompiler, const Char* pstrExecutablePath ):
	m_pCurrentSequence(nullptr),
	m_bracketCount(0),
	m_ignoreLineEnds(false),
	m_studioCompiler(studioCompiler),
	m_pVBMCompiler(pVBMCompiler)
{
	Uint32 nbCmds = sizeof(COMMAND_PROCESSOR_FUNCTIONS) / sizeof(ProcessorFnMapping_t);
	for(Uint32 i = 0; i < nbCmds; i++)
		m_cmdProcessorFunctionArray.push_back(COMMAND_PROCESSOR_FUNCTIONS[i]);

	nbCmds = sizeof(SEQUENCE_OPTION_PROCESSOR_FUNCTIONS) / sizeof(ProcessorFnMapping_t);
	for(Uint32 i = 0; i < nbCmds; i++)
		m_sequenceOptionProcessorFunctionArray.push_back(SEQUENCE_OPTION_PROCESSOR_FUNCTIONS[i]);

	Common::GetDirectoryPath(pstrExecutablePath, m_executableFolderPath);
}

//===============================================
// @brief Destructor for CQCParser class
//
//===============================================
CQCParser::~CQCParser( void )
{
	Clear();
}

//===============================================
// @brief Loads activity mapping definitions
//
// @param pstrFilePath Path to definitons file
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::LoadActivityMappings( const Char* pstrFilePath )
{
	CString filePath = Common::CleanupPath(pstrFilePath);

	Uint32 fileSize = 0;
	const byte* pFile = g_fileInterface.pfnLoadFile(filePath.c_str(), &fileSize);
	if(!pFile)
	{
		ErrorMsg("Could not activity mapping file '%s'.\n", filePath.c_str());
		return false; // This is a failure
	}

	Msg("Loading activity mappings from '%s'.\n", filePath.c_str());

	// Begin parsing
	Char token[MAX_PARSE_LENGTH];

	// Parse first token, needs to be '{'
	const Char* pstr = Common::Parse(reinterpret_cast<const Char*>(pFile), token);
	if(!pstr)
	{
		ErrorMsg("Short read on file.\n");
		g_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	if(qstrcmp(token, "{"))
	{
		ErrorMsg("Expected '{', got '%s' instead.\n", token);
		g_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	Uint32 nbAdded = 0;
	Uint32 lineNb = 0;
	Char lineStr[MAX_LINE_LENGTH];
	while(true)
	{
		// Parse next line
		pstr = Common::ReadLine(pstr, lineStr);
		if(qstrlen(lineStr) == 0)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		const Char* plinestr = Common::Parse(lineStr, token);
		
		// Skip over empty lines
		if(qstrlen(token) <= 0 && !plinestr)
		{
			lineNb++;
			continue;
		}

		// Stop if we reached the file end
		if(!qstrcmp(token, "}"))
			break;

		// Check for error
		if(!pstr)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		// First token of the line needs to be another '{'
		if(qstrcmp(token, "{"))
		{
			ErrorMsg("Expected '{', got '%s' instead on line %d.\n", token, lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		// Now read the name of the activity
		plinestr = Common::Parse(plinestr, token);
		if(!plinestr)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		CString activityName = token;

		// Now read the identifier
		plinestr = Common::Parse(plinestr, token);
		if(!plinestr)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		if(!Common::IsNumber(token))
		{
			ErrorMsg("Expected a numerical value, got '%s' instead on line %d.\n", token, lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		Int32 identifier = SDL_atoi(token);

		// Now read the closing bracket
		plinestr = Common::Parse(plinestr, token);
		if(qstrcmp(token, "}"))
		{
			WarningMsg("Expected '{', got '%s' instead on line %d.\n", token, lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		std::map<CString, Int32>::iterator it = m_activityNameIdMap.find(activityName.c_str());
		if(it != m_activityNameIdMap.end())
		{
			ErrorMsg("Activity '%s' already defined with identifier of %d.\n", it->first.c_str(), it->second);
			return false;
		}

		m_activityNameIdMap.insert(std::pair<CString, Int32>(activityName, identifier));
		nbAdded++;
		lineNb++;
	}

	Msg("Added %d activity mappings.\n", nbAdded);

	// Release the file contents
	g_fileInterface.pfnFreeFile(pFile);
	return true;
}

//===============================================
// @brief Clears all data used
//
//===============================================
void CQCParser::Clear( void )
{
	CParserBase::Clear();

	if(!m_activityNameIdMap.empty())
		m_activityNameIdMap.clear();
}

//===============================================
// @brief Reads in the contents of the QC script, and
// processes it.
//
// @param pstrFilename Path to the file to load
// @return TRUE reading was successful, FALSE if it failed
//===============================================
bool CQCParser::ProcessFile( const Char* pstrFilename )
{
	// Clear anything still loaded
	Clear();

	// Load activity mappings first
	CString filePath;
	filePath << m_executableFolderPath << PATH_SLASH_CHAR << ACTIVITY_DEFINITIONS_FILENAME;

	if(!LoadActivityMappings(filePath.c_str()))
		return false;

	// Now load the script
	if(!OpenScriptFile(pstrFilename))
	{
		ErrorMsg("Couldn't open QC file '%s'.\n", pstrFilename);
		return false;
	}

	// Parse the script contents
	while(true)
	{
		// First, skip ahead until we find a command
		// starting with '$'
		while(true)
		{
			ReadNextToken(true);
			if(IsScriptEnd())
				return true;

			if(!m_readTokenBuffer.empty() && m_readTokenBuffer[0] == '$')
				break;

			// Skip line contents
			while(IsTokenAvailable())
				ReadNextToken(false);
		}

		// Mark this
		m_isFirst = true;

		// Now process the command extracted
		CString cmd(m_readTokenBuffer);
		pfnProcessorFunction ptrProcessorFunction = GetCommandProcessorFunctionForName(cmd.c_str());
		if(!ptrProcessorFunction)
		{
			ErrorMsg("Uknown command: %s.\n", cmd.c_str());
			return false;
		}
		else
		{
			// Execute the command
			bool result = (this->*ptrProcessorFunction)();
			if(!result)
			{
				ErrorMsg("Error while processing '%s' command.\n", cmd.c_str());
				return false;
			}
		}
	}

	// Clean us up
	Clear();

	return true;
}

//===============================================
// @brief Function for '$modelname' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_ModelName( void )
{
	if(!ReadNextToken())
	{
		ErrorMsg("No value specified for '$modelname'.\n");
		return false;
	}

	// Strip extension
	CString str = GetCurrentToken();
	Uint32 i = str.length() - 1;
	while(i >= 0)
	{
		if(str[i] == '.')
			break;

		i--;
	}

	Uint32 numerase = (str.length() - i);
	str.erase(i, numerase);

	g_options.outputname = Common::CleanupPath(str.c_str());
	return true;
}

//===============================================
// @brief Function for '$cd' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_ChangeDirectory( void )
{
	const Char* pstrString = nullptr;
	if(!ReadString(pstrString))
	{
		ErrorMsg("Couldn't read value for '$cd'.\n");
		return false;
	}

	g_options.addDirPath(pstrString);
	return true;
}

//===============================================
// @brief Function for '$cdtexture' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_ChangeTextureDirectory( void )
{
	const Char* pstrPath = nullptr;
	while(true)
	{
		bool result = ReadString(pstrPath);
		if(!result)
		{
			if(m_isFirst)
			{
				ErrorMsg("No value specified for '$cdtexture' command.\n");
				return false;
			}
			else
			{
				// Reached end of list
				break;
			}
		}

		m_studioCompiler.AddTextureFolderPath(pstrPath);
		m_isFirst = false;
	}

	return true;
}

//===============================================
// @brief Function for '$scale' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_Scale( void )
{
	Float value;
	if(!ReadFloat(value))
	{
		ErrorMsg("No value set for for '$scale' command.\n");
		return false;
	}

	m_studioCompiler.SetScaleUpValue(value);
	m_studioCompiler.SetDefaultScaleValue(value);

	return true;
}

//===============================================
// @brief Function for '$scale_movement' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_ScaleMovement( void )
{
	Float value;
	if(!ReadFloat(value))
	{
		ErrorMsg("No value set for '$scale_movement' command.\n");
		return false;
	}

	m_studioCompiler.SetScaleUpValue(value);
	m_studioCompiler.SetDefaultScaleValue(value);

	return true;
}

//===============================================
// @brief Function for '$root' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_Root( void )
{
	const Char* pstrString = nullptr;
	if(!ReadString(pstrString))
	{
		ErrorMsg("No value set for '$root' command.\n");
		return false;
	}

	m_studioCompiler.SetRootPivot(pstrString);
	return true;
}

//===============================================
// @brief Function for '$pivot' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_Pivot( void )
{
	const Char* pstrString = nullptr;
	if(!ReadString(pstrString))
	{
		ErrorMsg("No value set for '$pivot' command.\n");
		return false;
	}
	
	m_studioCompiler.AddPivot(pstrString);
	return true;
}

//===============================================
// @brief Function for '$controller' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_BoneController( void )
{
	CString controllerId;
	if(!ReadString(controllerId))
	{
		ErrorMsg("No controller index or name set for '$controller' command.\n");
		return false;
	}

	Int32 controllerIndex = 0;
	if(!Common::IsNumber(controllerId))
	{
		if(!qstrcicmp(controllerId, "mouth"))
		{
			// Hardcoded crap legacy stuff from HL1
			controllerIndex = 4;
		}
		else
		{
			ErrorMsg("String id '%s' not recognized for '$controller' command.\n", controllerId.c_str());
			return false;
		}
	}
	else
	{
		controllerIndex = SDL_atoi(controllerId.c_str());
		if(controllerIndex < 0)
		{
			ErrorMsg("Invalid controller index %d specified for '$controller' command.\n", controllerIndex);
			return false;
		}
	}

	// Get bone name
	CString boneName;
	if(!ReadString(boneName))
	{
		ErrorMsg("No bone name set for '$controller' command.\n");
		return false;
	}

	// Get the type of controller
	const Char* pstrString = nullptr;
	if(!ReadString(pstrString))
	{
		ErrorMsg("No controller type set for '$controller' command.\n");
		return false;
	}

	Int32 controllerTypeFlags = GetMotionTypeFromString(pstrString);
	if(controllerTypeFlags == STUDIO_MT_NONE)
	{
		ErrorMsg("Unknown controller type '%s' specified for '$controller' command with index '%s'.\n", pstrString, controllerId.c_str());
		return false;
	}

	// Read the controller start value
	if(!ReadString(pstrString))
	{
		ErrorMsg("No controller start value set for '$controller' command.\n");
		return false;
	}

	Float startValue = SDL_atof(pstrString);

	// Read the controller end value
	if(!ReadString(pstrString))
	{
		ErrorMsg("No controller end value set for '$controller' command.\n");
		return false;
	}

	Float endValue = SDL_atof(pstrString);

	// If it's a rotating controller, try and check if it wraps around
	// and mark it as being a wrap-around controller
	if(controllerTypeFlags & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		Float startCapped = (static_cast<Int32>(startValue) + 360) % 360;
		Float endCapped = (static_cast<Int32>(endValue) + 360) % 360;
		if(startCapped == endCapped)
			controllerTypeFlags |= STUDIO_RLOOP;

	}

	return m_studioCompiler.AddBoneController(controllerIndex, boneName.c_str(), controllerTypeFlags, startValue, endValue);
}

//===============================================
// @brief Function for '$lod' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_LOD( void )
{
	// Read submodel target
	CString submodelName;
	if(!ReadString(submodelName))
	{
		ErrorMsg("No submodel specified for '$lod' command.\n");
		return false;
	}

	// Read LOD filename
	CString lodfilename;
	if(!ReadString(lodfilename))
	{
		ErrorMsg("No LOD file specified for '$lod' command.\n");
		return false;
	}

	// Get type
	const Char* pstrString = nullptr;
	if(!ReadString(pstrString))
	{
		ErrorMsg("No LOD type set for '$lod' command.\n");
		return false;
	}

	Float lodDistanceValue = 0;
	vbmlod_type_t lodType = VBM_LOD_NONE;
	if(!qstrcicmp(pstrString, "distance"))
	{
		if(!ReadFloat(lodDistanceValue))
		{
			ErrorMsg("No distance value set for '$lod' command.\n");
			return false;
		}

		lodType = VBM_LOD_DISTANCE;
	}
	else if(!qstrcicmp(pstrString, "shadow"))
	{
		lodType = VBM_LOD_SHADOW;
	}
	else
	{
		ErrorMsg("Unknown LOD type '%s' specified.\n", pstrString);
		return false;
	}

	// Default params for current body
	bool flipTriangles = g_options.isFlagSet(CMP_FL_REVERSE_TRIANGLES);
	// Scaling for submodel, -1 means not set
	Float scale = -1;

	// Now parse the params
	while(true)
	{
		if(!ReadString(pstrString))
			break;

		if(!qstrcicmp(pstrString, "reverse"))
		{
			flipTriangles = true;
		}
		else if(!qstrcicmp(pstrString, "scale"))
		{
			if(!ReadFloat(scale))
			{
				ErrorMsg("No value set for 'scale' option on '$lod' command with submodel '%s'.\n", submodelName.c_str());
				return false;
			}
		}
		else
		{
			ErrorMsg("Unrecognized '%s' argument for '$lod' command.\n", pstrString);
			return false;
		}
	}

	return m_studioCompiler.AddLOD(submodelName.c_str(), lodfilename.c_str(), lodType, lodDistanceValue, flipTriangles, scale);
}

//===============================================
// @brief Function for '$body' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_Body( void )
{
	// Read bodygroup name
	CString groupName;
	if(!ReadString(groupName))
	{
		ErrorMsg("No bodygroup name specified for '$body' command.\n");
		return false;
	}

	CString submodelName;
	if(!ReadString(submodelName))
	{
		ErrorMsg("No submodel specified for '$body' command.\n");
		return false;
	}

	// Default params for current body
	bool flipTriangles = g_options.isFlagSet(CMP_FL_REVERSE_TRIANGLES);
	// Scaling for submodel, -1 means not set
	Float scale = -1;
	// Movement scaling, -1 if not set
	// (What is this for even with $body?)
	Float movementScale = -1;
	// Name for VTA if any
	CString vtaName;
	// Name for collision file if any
	CString collisionSmdName;

	// Now parse the params
	const Char* pstrString = nullptr;
	if(ReadString(pstrString))
	{
		if(!qstrcicmp(pstrString, "reverse"))
		{
			flipTriangles = true;
		}
		else if(!qstrcicmp(pstrString, "scale"))
		{
			if(!ReadFloat(scale))
			{
				ErrorMsg("No value set for 'scale' option on '$studio' command with submodel '%s'.\n", submodelName.c_str());
				return false;
			}
		}
		else if(!qstrcicmp(pstrString, "flex"))
		{
			if(!ReadString(vtaName))
			{
				ErrorMsg("No VTA file specified for '$body' command for submodel '%s'.\n", submodelName.c_str());
				return false;
			}
		}
		else if(!qstrcicmp(pstrString, "collision"))
		{
			if(!ReadString(collisionSmdName))
			{
				ErrorMsg("No collision SMD file specified for '$body' command for submodel '%s'.\n", submodelName.c_str());
				return false;
			}
		}
		else
		{
			ErrorMsg("Unrecognized '%s' argument for '$body' command.\n", pstrString);
			return false;
		}
	}

	// Allocate bodygroup
	smdl::bodypart_t* pBodyPart = m_studioCompiler.AddBodyGroup(groupName.c_str());
	if(!pBodyPart)
	{
		ErrorMsg("Failed to allocate bodygroup '%s' for '$body' command.\n", groupName.c_str());
		return false;
	}

	// Now add the submodel
	return m_studioCompiler.AddSubmodel(pBodyPart, submodelName.c_str(), vtaName.c_str(), collisionSmdName.c_str(), flipTriangles, scale);
}

//===============================================
// @brief Function for '$flexcontroller' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_FlexController( void )
{
	// Read submodel target
	CString controllerName;
	if(!ReadString(controllerName))
	{
		ErrorMsg("No controller name specified for '$flexcontroller' command.\n");
		return false;
	}

	Float minValue = 0;
	if(!ReadFloat(minValue))
	{
		ErrorMsg("No minimum value set for '$flexcontroller' command.\n");
		return false;
	}

	Float maxValue = 0;
	if(!ReadFloat(maxValue))
	{
		ErrorMsg("No maximum value set for '$flexcontroller' command.\n");
		return false;
	}

	// Read in interpolation type
	CString interpTypeString;
	if(!ReadString(interpTypeString))
	{
		ErrorMsg("No interpolation type specified for '$flexcontroller' command.\n");
		return false;
	}
	
	vbmflexinterp_t interpType = VBM_FLEX_LINEAR;
	if(!qstrcicmp(interpTypeString, "sine"))
		interpType = VBM_FLEX_SINE;
	else if(!qstrcicmp(interpTypeString, "linear"))
		interpType = VBM_FLEX_LINEAR;
	else
	{
		ErrorMsg("Invalid interpolation '%s' specified for '$flexcontroller' command.\n", interpTypeString.c_str());
		return false;
	}

	// Next token should be a '{' bracket
	const Char* pstrString = nullptr;
	if(!ReadString(pstrString))
	{
		ErrorMsg("No '{' bracket found for '$flexcontroller' command.\n");
		return false;
	}

	if(qstrcmp(pstrString, "{"))
	{
		ErrorMsg("Expected '{' token for '$flexcontroller' command, got '%s' instead.\n", pstrString);
		return false;
	}

	CArray<vbm::flexcontroller_vta_t> vtaArray;
	while(true)
	{
		if(!ReadString(pstrString))
		{
			ErrorMsg("No VTA specified for '$flexcontroller' command.\n");
			return false;
		}

		if(!qstrcmp(pstrString, "}"))
			break;

		CString vtaName = pstrString;
		Int32 flexIndex = 0;
		if(!ReadInt32(flexIndex))
		{
			ErrorMsg("No flex index specified '$flexcontroller' command.\n");
			return false;
		}

		for(Uint32 i = 0; i < vtaArray.size(); i++)
		{
			if(!qstrcicmp(vtaArray[i].name, vtaName))
			{
				ErrorMsg("VTA file '%s' already present in list for '$flexcontroller' command with name '%s'.\n", vtaName.c_str(), controllerName.c_str());
				return false;
			}
		}

		vbm::flexcontroller_vta_t newVTA;
		newVTA.name = vtaName;
		newVTA.flexindex = flexIndex;
		vtaArray.push_back(newVTA);
	}

	if(vtaArray.empty())
	{
		ErrorMsg("Empty VTA list for '$flexcontroller' command.\n");
		return false;
	}

	bool result;
	if(m_pVBMCompiler)
		result = m_pVBMCompiler->AddFlexController(controllerName.c_str(), minValue, maxValue, interpType, vtaArray);
	else
		result = true;

	return result;
}

//===============================================
// @brief Function for '$bodygroup' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_BodyGroup( void )
{
	// Read bodygroup name
	CString groupName;
	if(!ReadString(groupName))
	{
		ErrorMsg("No bodygroup name specified for '$bodygroup' command.\n");
		return false;
	}

	// Next token should be a '{' bracket
	const Char* pstrString = nullptr;
	if(!ReadString(pstrString, true))
	{
		ErrorMsg("No '{' bracket found for '$bodygroup' command.\n");
		return false;
	}

	if(qstrcmp(pstrString, "{"))
	{
		ErrorMsg("Expected '{' token for '$bodygroup' command, got '%s' instead.\n", pstrString);
		return false;
	}

	CArray<smdl::submodelentry_t> submodelEntryArray;
	while(true)
	{
		if(!ReadString(pstrString, true))
		{
			ErrorMsg("No '{' bracket found for '$bodygroup' command.\n");
			return false;
		}

		if(!qstrcmp(pstrString, "}"))
		{
			// Reached end of definition
			break;
		}
		else if(!qstrcmp(pstrString, "{"))
		{
			ErrorMsg("Unexpected '{' token for '$bodygroup' command, was expecting submodel name or closing '}' bracket.\n", pstrString);
			return false;
		}
		else if(!qstrcicmp(pstrString, "blank"))
		{
			smdl::submodelentry_t newEntry;
			newEntry.submodelname = "blank";
			submodelEntryArray.push_back(newEntry);
			continue;
		}
		else if(qstrcicmp(pstrString, "studio"))
		{
			ErrorMsg("Was expecting 'studio' option for '$bodygroup' command with name '%s', got '%s' instead.\n", groupName.c_str(), pstrString);
			return false;
		}

		// Read in submodel name
		if(!ReadString(pstrString))
		{
			ErrorMsg("No '{' bracket found for '$bodygroup' command.\n");
			return false;
		}

		CString submodelName = pstrString;

		// Default params for current body
		bool flipTriangles = g_options.isFlagSet(CMP_FL_REVERSE_TRIANGLES);
		// Scaling for submodel, -1 means not set
		Float scale = -1;
		// Movement scaling, -1 if not set
		// (What is this for even with $body?)
		Float movementScale = -1;
		// Name for VTA if any
		CString vtaName;
		// Name for collision file if any
		CString collisionSmdName;

		// Now parse the params
		if(ReadString(pstrString))
		{
			if(!qstrcicmp(pstrString, "reverse"))
			{
				flipTriangles = true;
			}
			else if(!qstrcicmp(pstrString, "scale"))
			{
				if(!ReadFloat(scale))
				{
					ErrorMsg("No value set for 'scale' option on '$studio' command with submodel '%s'.\n", submodelName.c_str());
					return false;
				}
			}
			else if(!qstrcicmp(pstrString, "flex"))
			{
				if(!ReadString(vtaName))
				{
					ErrorMsg("No VTA file specified for '$body' command for submodel '%s'.\n", submodelName.c_str());
					return false;
				}
			}
			else if(!qstrcicmp(pstrString, "collision"))
			{
				if(!ReadString(collisionSmdName))
				{
					ErrorMsg("No collision SMD file specified for '$body' command for submodel '%s'.\n", submodelName.c_str());
					return false;
				}
			}
			else
			{
				ErrorMsg("Unrecognized '%s' argument for '$body' command.\n", pstrString);
				return false;
			}
		}

		smdl::submodelentry_t newEntry;
		newEntry.submodelname = submodelName;
		newEntry.flexname = vtaName;
		newEntry.collisionsmdname = collisionSmdName;
		newEntry.reverseTriangles = flipTriangles;
		newEntry.scale = scale;
		newEntry.movementScale = movementScale;

		submodelEntryArray.push_back(newEntry);
	}

	if(submodelEntryArray.empty())
	{
		ErrorMsg("Empty sumbodel list for '$bodygroup' command with name '%s'.\n", groupName.c_str());
		return false;
	}

	// Allocate bodygroup
	smdl::bodypart_t* pBodyPart = m_studioCompiler.AddBodyGroup(groupName.c_str());
	if(!pBodyPart)
	{
		ErrorMsg("Failed to allocate bodygroup '%s' for '$body' command.\n", groupName.c_str());
		return false;
	}

	// Now add the submodels
	for(Uint32 i = 0; i < submodelEntryArray.size(); i++)
	{
		smdl::submodelentry_t& entry = submodelEntryArray[i];
		if(!m_studioCompiler.AddSubmodel(pBodyPart, entry.submodelname.c_str(), entry.flexname.c_str(), entry.collisionsmdname.c_str(), entry.reverseTriangles, entry.scale))
			return false;
	}

	return true;
}

//===============================================
// @brief Function for '$sequence' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_Sequence( void )
{
	// Read first animation name
	const Char* pstrString = nullptr;
	if(!ReadString(pstrString))
	{
		ErrorMsg("No sequence name defined for '$sequence' command.\n");
		return false;
	}

	m_pCurrentSequence = m_studioCompiler.AddSequence(pstrString);
	if(!m_pCurrentSequence)
	{
		ErrorMsg("Couldn't add sequence '%s'.\n", pstrString);
		return false;
	}

	// Reset bracket depth
	m_bracketCount = 0;
	// TRUE if we should ignore line ends
	m_ignoreLineEnds = false;
	// Motion type used
	Int32 motionType = STUDIO_MT_NONE;

	// Parse the rest
	while(true)
	{
		if(!ReadString(pstrString, m_ignoreLineEnds))
		{
			if(m_bracketCount > 0)
			{
				ErrorMsg("Missing closing '}' bracket on '$sequence' command for sequence with name '%s'.\n", m_pCurrentSequence->name.c_str());
				return false;
			}
			else
			{
				// Reached end of line
				break;
			}
		}

		if(!qstrcicmp(pstrString, "{"))
		{
			// Entering a bracket
			m_bracketCount++;
			m_ignoreLineEnds = true;

			continue;
		}
		else if(!qstrcicmp(pstrString, "}"))
		{
			// Exiting a bracket
			m_bracketCount--;
			if(!m_bracketCount)
				m_ignoreLineEnds = false;

			continue;
		}
		else
		{
			// See if this is an option token
			pfnProcessorFunction pfnProcessorFn = GetSeqOptionProcessorFunctionForName(pstrString);
			if(pfnProcessorFn)
			{
				bool parseResult = (this->*pfnProcessorFn)();
				if(!parseResult)
				{
					ErrorMsg("Error while parsing '$sequence' command with name '%s'.\n", m_pCurrentSequence->name.c_str());
					return false;
				}
			}
			else if((motionType = GetMotionTypeFromString(pstrString)) != STUDIO_MT_NONE)
			{
				m_pCurrentSequence->motiontype |= motionType;
			}
			else if(!qstrncmp(pstrString, "ACT_", 4))
			{
				std::map<CString, Int32>::iterator it = m_activityNameIdMap.find(pstrString);
				if(it == m_activityNameIdMap.end())
				{
					ErrorMsg("Couldn't find definition for activity '%s'.\n", pstrString);
					return false;
				}

				// Read the weight
				Int32 activityWeight = 0;
				if(!ReadInt32(activityWeight))
				{
					ErrorMsg("No activity weight option on '$sequence' command for sequence named '%s'.\n", m_pCurrentSequence->name.c_str());
					return false;
				}

				m_pCurrentSequence->activity = it->second;
				m_pCurrentSequence->actweight = activityWeight;
			}
			else
			{
				m_pCurrentSequence->smdfilenames.push_back(pstrString);
			}
		}
	}

	if(m_bracketCount > 0)
	{
		ErrorMsg("Missing closing '}' bracket on '$sequence' command for sequence with name '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	// Set default z rotation
	m_pCurrentSequence->zrotation = m_studioCompiler.GetDefaultZRotation();

	Msg("Added sequence definition '%s' with %.2f fps, %d blends and %d events.\n", m_pCurrentSequence->name.c_str(), m_pCurrentSequence->fps, m_pCurrentSequence->panims.size(), m_pCurrentSequence->events.size());
	return true;
}

//===============================================
// @brief Function for '$eyeposition' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_EyePosition( void )
{
	Vector eyePosition;
	for(Uint32 i = 0; i < 3; i++)
	{
		Float value;
		if(!ReadFloat(value))
		{
			ErrorMsg("Couldn't read value %d for '$eyeposition'.\n", (i+1));
			return false;
		}

		eyePosition[i] = value;
	}

	m_studioCompiler.SetEyePosition(eyePosition);
	return true;
}

//===============================================
// @brief Function for '$origin' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_Origin( void )
{
	Vector origin;
	for(Uint32 i = 0; i < 3; i++)
	{
		Float value;
		if(!ReadFloat(value))
		{
			ErrorMsg("Couldn't read value %d for '$origin'.\n", (i+1));
			return false;
		}

		origin[i] = value;
	}

	m_studioCompiler.SetDefaultAdjustVector(origin);

	Float value;
	if(ReadFloat(value))
		m_studioCompiler.SetDefaultZRotation(value);

	return true;
}

//===============================================
// @brief Function for '$bbox' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_BBox( void )
{
	Uint32 validx = 0;

	Vector mins;
	for(Uint32 i = 0; i < 3; i++, validx++)
	{
		Float value;
		if(!ReadFloat(value))
		{
			ErrorMsg("Couldn't read value %d for '$bbox'.\n", (validx+1));
			return false;
		}

		mins[i] = value;
	}

	Vector maxs;
	for(Uint32 i = 0; i < 3; i++, validx++)
	{
		Float value;
		if(!ReadFloat(value))
		{
			ErrorMsg("Couldn't read value %d for '$bbox'.\n", (validx+1));
			return false;
		}

		maxs[i] = value;
	}

	m_studioCompiler.SetBBox(mins, maxs);
	return true;
}

//===============================================
// @brief Function for '$cbox' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_CBox( void )
{
	Uint32 validx = 0;

	Vector mins;
	for(Uint32 i = 0; i < 3; i++, validx++)
	{
		Float value;
		if(!ReadFloat(value))
		{
			ErrorMsg("Couldn't read value %d for '$bbox'.\n", (validx+1));
			return false;
		}

		mins[i] = value;
	}

	Vector maxs;
	for(Uint32 i = 0; i < 3; i++, validx++)
	{
		Float value;
		if(!ReadFloat(value))
		{
			ErrorMsg("Couldn't read value %d for '$bbox'.\n", (validx+1));
			return false;
		}

		maxs[i] = value;
	}

	m_studioCompiler.SetCBox(mins, maxs);
	return true;
}

//===============================================
// @brief Function for '$mirrorbone' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_MirrorBone( void )
{
	const Char* pstrValue = nullptr;
	if(!ReadString(pstrValue))
	{
		ErrorMsg("No value specified for '$mirrorbone'.\n");
		return false;
	}

	m_studioCompiler.AddMirroredBone(pstrValue);
	return true;
}

//===============================================
// @brief Function for '$gamma' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_Gamma( void )
{
	Float value = 0;
	if(!ReadFloat(value))
	{
		ErrorMsg("No value specified for '$gamma'.\n");
		return false;
	}

	m_studioCompiler.SetGamma(value);
	return true;
}

//===============================================
// @brief Function for '$flags' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_Flags( void )
{
	const Char* pstrValue = nullptr;
	while(true)
	{
		bool result = ReadString(pstrValue);
		if(!result)
		{
			if(m_isFirst)
			{
				ErrorMsg("No value specified for '$flags' command.\n");
				return false;
			}
			else
			{
				// Reached end of list
				break;
			}
		}

		Int32 value = 0;
		if(Common::IsNumber(pstrValue))
		{
			// Value was directly specified as an integer
			value = SDL_atoi(pstrValue);
		}
		else
		{
			if(!qstrcicmp(pstrValue, "rocket"))
				value = STUDIO_MF_ROCKET;
			else if(!qstrcicmp(pstrValue, "grenade"))
				value = STUDIO_MF_GRENADE;
			else if(!qstrcicmp(pstrValue, "gib"))
				value = STUDIO_MF_GIB;
			else if(!qstrcicmp(pstrValue, "rotate"))
				value = STUDIO_MF_ROTATE;
			else if(!qstrcicmp(pstrValue, "tracer"))
				value = STUDIO_MF_TRACER;
			else if(!qstrcicmp(pstrValue, "zomgib"))
				value = STUDIO_MF_ZOMGIB;
			else if(!qstrcicmp(pstrValue, "tracer2"))
				value = STUDIO_MF_TRACER2;
			else if(!qstrcicmp(pstrValue, "tracer3"))
				value = STUDIO_MF_TRACER3;
			else if(!qstrcicmp(pstrValue, "dynlight"))
				value = STUDIO_MF_DYNAMIC_LIGHT;
			else if(!qstrcicmp(pstrValue, "hitboxtrace"))
				value = STUDIO_MF_TRACE_HITBOX;
			else if(!qstrcicmp(pstrValue, "skylight"))
				value = STUDIO_MF_SKYLIGHT;
			else if(!qstrcicmp(pstrValue, "centerlight"))
				value = STUDIO_MF_CENTERLIGHT;
			else
			{
				ErrorMsg("Unknown setting '%s' specified for '$flags'.\n", pstrValue);
				return false;
			}
		}

		m_studioCompiler.SetModelFlag(value);
		m_isFirst = false;
	}

	return true;
}

//===============================================
// @brief Function for '$texturegroup' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_TextureGroup( void )
{
	// Read group name
	CString groupName;
	if(!ReadString(groupName))
	{
		ErrorMsg("Couldn't read group name for '$texturegroup' command.\n");
		return false;
	}

	// Next token should be a '{'
	const Char* pstrValue = nullptr;
	if(!ReadString(pstrValue, true))
	{
		ErrorMsg("Couldn't read opening bracket for '$texturegroup' command.\n");
		return false;
	}

	if(qstrcmp(pstrValue, "{"))
	{
		ErrorMsg("Was expecting an outer '{' for '$texturegroup' command, got '%s' instead.\n", pstrValue);
		return false;
	}

	// Parse in the groups
	CArray<CArray<CString>> groupNamesArray;
	while(true)
	{
		// Once again, next token should be '{'
		if(!ReadString(pstrValue, true))
		{
			ErrorMsg("Couldn't read inner opening bracket for '$texturegroup' command.\n");
			return false;
		}

		if(!qstrcmp(pstrValue, "}"))
		{
			// Reached end of group list
			break;
		}
		else if(qstrcmp(pstrValue, "{"))
		{
			ErrorMsg("Was expecting an inner '{' or a closing '}' for '$texturegroup' command, got '%s' instead.\n", pstrValue);
			return false;
		}

		Uint32 groupIndex = groupNamesArray.size();
		groupNamesArray.resize(groupNamesArray.size()+1);
		CArray<CString>& curGroup = groupNamesArray[groupIndex];

		while(true)
		{
			// Read texture name
			if(!ReadString(pstrValue, true))
			{
				ErrorMsg("Couldn't read texture name or closing '}' for '$texturegroup' command.\n");
				return false;
			}

			if(!qstrcmp(pstrValue, "{"))
			{
				ErrorMsg("Was expecting texture name or a closing '}' for '$texturegroup' command, got '%s' instead.\n", pstrValue);
				return false;
			}
			else if(!qstrcmp(pstrValue, "}"))
			{
				// Reached end of texture list
				break;
			}

			curGroup.push_back(pstrValue);
		}

		// Verify group size consistency
		for(Uint32 i = 0; i < groupIndex; i++)
		{
			CArray<CString>& grp = groupNamesArray[i];
			if(grp.size() != curGroup.size())
			{
				ErrorMsg("In texture group '%s', texture count on index %d is inconsistent compared to index %d(%d vs %d).\n", groupName.c_str(), groupIndex, i, curGroup.size(), grp.size());
				return false;
			}
		}
	}

	if(groupNamesArray.size() == 1)
	{
		ErrorMsg("Texture group '%s' does not have any alternate textures listed.\n", groupName.c_str());
		return false;
	}
	else if(groupNamesArray.empty())
	{
		ErrorMsg("Texture group '%s' is empty.\n", groupName.c_str());
		return false;
	}

	return m_studioCompiler.AddTextureGroup(groupName.c_str(), groupNamesArray);
}

//===============================================
// @brief Function for '$hgroup' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_HitGroup( void )
{
	// Read in hitroup ID
	Int32 hitGroupId = 0;
	if(!ReadInt32(hitGroupId))
	{
		ErrorMsg("Couldn't read hitgroup value for '$hgroup' command.\n");
		return false;
	}

	// Read in group name
	CString groupName;
	if(!ReadString(groupName))
	{
		ErrorMsg("Couldn't read bone name for '$hgroup' command.\n");
		return false;
	}

	return m_studioCompiler.AddHitGroup(hitGroupId, groupName.c_str());
}

//===============================================
// @brief Function for '$hbox' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_HitBox( void )
{
	// Read in hitroup
	Int32 hitGroupId = 0;
	if(!ReadInt32(hitGroupId))
	{
		ErrorMsg("Couldn't read hitgroup value for '$hbox' command.\n");
		return false;
	}

	// Read in bone name
	CString bonename;
	if(!ReadString(bonename))
	{
		ErrorMsg("Couldn't read bone name for '$hbox' command.\n");
		return false;
	}

	Vector mins;
	for(Uint32 i = 0; i < 3; i++)
	{
		if(!ReadFloat(mins[i]))
		{
			ErrorMsg("Mising mins value %d for '$hbox' command.\n", (i+1));
			return false;
		}
	}

	Vector maxs;
	for(Uint32 i = 0; i < 3; i++)
	{
		if(!ReadFloat(maxs[i]))
		{
			ErrorMsg("Mising maxs value %d for '$hbox' command.\n", (i+1));
			return false;
		}
	}

	return m_studioCompiler.AddHitBox(hitGroupId, bonename.c_str(), mins, maxs);
}

//===============================================
// @brief Function for '$attachment' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_Attachment( void )
{
	// Read in attachment index
	Int32 attachmentIndex = 0;
	if(!ReadInt32(attachmentIndex))
	{
		ErrorMsg("Missing index for for '$attachment' command.\n");
		return false;
	}

	// Read in attachment name
	CString bonename;
	if(!ReadString(bonename))
	{
		ErrorMsg("Missing bone name for '$attachment' command.\n");
		return false;
	}

	Vector origin;
	for(Uint32 i = 0; i < 3; i++)
	{
		if(!ReadFloat(origin[i]))
		{
			ErrorMsg("Mising origin value %d for '$attachment' command.\n", (i+1));
			return false;
		}
	}

	return m_studioCompiler.AddAttachment(bonename.c_str(), attachmentIndex, origin);
}

//===============================================
// @brief Function for '$renamebone' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_RenameBone( void )
{
	CString originalName;
	if(!ReadString(originalName))
	{
		ErrorMsg("No original bone name specified for '$renamebone'.\n");
		return false;
	}

	CString targetName;
	if(!ReadString(targetName))
	{
		ErrorMsg("No target bone name specified for '$renamebone'.\n");
		return false;
	}

	return m_studioCompiler.AddRenamedBone(originalName.c_str(), targetName.c_str());
}

//===============================================
// @brief Function for '$centerlight' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_CenterLight( void )
{
	// This is really deprecated
	m_studioCompiler.SetModelFlag(STUDIO_MF_CENTERLIGHT);
	return true;
}

//===============================================
// @brief Function for '$noblend' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_NoBlendBone( void )
{
	const Char* pstrValue = nullptr;
	if(!ReadString(pstrValue))
	{
		ErrorMsg("No value specified for '$noblend'.\n");
		return false;
	}

	m_studioCompiler.AddNoBlendBone(pstrValue);
	return true;
}

//===============================================
// @brief Function for '$protected' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_ProtectedBone( void )
{
	const Char* pstrValue = nullptr;
	if(!ReadString(pstrValue))
	{
		ErrorMsg("No value specified for '$protected'.\n");
		return false;
	}

	m_studioCompiler.AddProtectedBone(pstrValue);
	return true;
}

//===============================================
// @brief Function for '$texrendermode' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_TextureRenderMode( void )
{
	CString texname;
	if(!ReadString(texname))
	{
		ErrorMsg("No texture name specified for '$texrendermode'.\n");
		return false;
	}

	CString mode;
	if(!ReadString(mode))
	{
		ErrorMsg("No mode specified for '$texrendermode'.\n");
		return false;
	}

	return m_studioCompiler.AddTextureRenderMode(texname.c_str(), mode.c_str());
}

//===============================================
// @brief Function for '$movement_bone' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_MovementBone( void )
{
	const Char* pstrValue = nullptr;
	if(!ReadString(pstrValue))
	{
		ErrorMsg("No value specified for '$movementbone'.\n");
		return false;
	}

	return m_studioCompiler.SetMovementBone(pstrValue);
}

//===============================================
// @brief Function for '$skylight' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_SkyLight( void )
{
	// This is really deprecated
	m_studioCompiler.SetModelFlag(STUDIO_MF_SKYLIGHT);
	return true;
}

//===============================================
// @brief Function for '$cliptotextures' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_ClipToTextures( void )
{
	WarningMsg("Deprecated command '$cliptotextures' used, ignored.\n");
	return true;
}

//===============================================
// @brief Function for '$sequencegroup' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_SequenceGroup( void )
{
	WarningMsg("Deprecated command '$sequencegroup' used, ignored.\n");
	return true;
}

//===============================================
// @brief Function for '$sequencegroupsize' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_SequenceGroupSize( void )
{
	WarningMsg("Deprecated command '$sequencegroupsize' used, ignored.\n");
	return true;
}

//===============================================
// @brief Function for '$externaltextures' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_ExternalTextures( void )
{
	WarningMsg("Deprecated command '$externaltextures' used, ignored.\n");
	return true;
}

//===============================================
// @brief Function for '$smallfov' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_SmallFOV( void )
{
	WarningMsg("Deprecated command '$smallfov' used, ignored.\n");
	return true;
}

//===============================================
// @brief Function for '$lockmodel' command
//
// @return TRUE if successfully read, FALSE otherwise
//===============================================
bool CQCParser::Cmd_LockModel( void )
{
	WarningMsg("Deprecated command '$lockmodel' used, ignored.\n");
	return true;
}

//===============================================
// @brief Return motion type for string
//
// @param pstrString Name of the type of movement
// @return Controller type bit
//===============================================
Int32 CQCParser::GetMotionTypeFromString( const Char* pstrString )
{
	Uint32 elementSize = sizeof(ControllerTypeMapping_t);
	Uint32 elementCount = sizeof(CONTROLLER_TYPE_MAPPINGS) / elementSize;
	for(Uint32 i = 0; i < elementCount; i++)
	{
		const ControllerTypeMapping_t& mapping = CONTROLLER_TYPE_MAPPINGS[i];
		if(!qstrcicmp(pstrString, mapping.stringId))
			return mapping.typeFlag;
	}

	return STUDIO_MT_NONE;
}

//===============================================
// @brief Get command processor function for name
//
// @param pstrName Name of command
// @return Pointer to processor function, or nullptr if invalid
//===============================================
CQCParser::pfnProcessorFunction CQCParser::GetCommandProcessorFunctionForName( const Char* pstrName )
{
	for(Uint32 i = 0; i < m_cmdProcessorFunctionArray.size(); i++)
	{
		ProcessorFnMapping_t& mapping = m_cmdProcessorFunctionArray[i];
		if(!qstrcicmp(mapping.name, pstrName))
			return mapping.ptrFunction;
	}

	return nullptr;
}

//===============================================
// @brief Get sequence option processor function for name
//
// @param pstrName Name of command
// @return Pointer to processor function, or nullptr if invalid
//===============================================
CQCParser::pfnProcessorFunction CQCParser::GetSeqOptionProcessorFunctionForName( const Char* pstrName )
{
	for(Uint32 i = 0; i < m_sequenceOptionProcessorFunctionArray.size(); i++)
	{
		ProcessorFnMapping_t& mapping = m_sequenceOptionProcessorFunctionArray[i];
		if(!qstrcicmp(mapping.name, pstrName))
			return mapping.ptrFunction;
	}

	return nullptr;
}

//===============================================
// @brief Function for $sequence option 'deform'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_Deform( void )
{
	WarningMsg("Unsupported option 'deform' on '$sequence' command for sequence named '%s'.\n", m_pCurrentSequence->name.c_str());
	return true;
}

//===============================================
// @brief Function for $sequence option 'event'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_Event( void )
{
	Int32 eventId = 0;
	if(!ReadInt32(eventId))
	{
		ErrorMsg("Couldn't read identifier for 'event' option on '$sequence' command for sequence named '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	Int32 eventFrame = 0;
	if(!ReadInt32(eventFrame))
	{
		ErrorMsg("Couldn't read frame for 'event' option with id %d on '$sequence' command for sequence named '%s'.\n", eventId, m_pCurrentSequence->name.c_str());
		return false;
	}

	const Char* pstrString = nullptr;
	if(!ReadString(pstrString))
	{
		ErrorMsg("Expected event option string or closing '}' bracket for event with id %d on '$sequence' command for sequence named '%s'.\n", eventId, m_pCurrentSequence->name.c_str());
		return false;
	}

	CString eventOption;
	if(!qstrcmp(pstrString, "}"))
	{
		// Discount this bracket locally, as it won't 
		// be handled by the code above that tracks this
		m_bracketCount--;
		if(!m_bracketCount)
			m_ignoreLineEnds = false;
	}
	else
	{
		eventOption = pstrString;
	}

	smdl::animevent_t newEvent;
	newEvent.eventid = eventId;
	newEvent.frame = eventFrame;
	newEvent.params = eventOption;

	m_pCurrentSequence->events.push_back(newEvent);
	return true;
}

//===============================================
// @brief Function for $sequence option 'pivot'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_Pivot( void )
{
	Int32 pivotIndex = 0;
	if(!ReadInt32(pivotIndex))
	{
		ErrorMsg("Couldn't read index for 'pivot' option on '$sequence' command for sequence named '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	Int32 pivotStart = 0;
	if(!ReadInt32(pivotStart))
	{
		ErrorMsg("Couldn't read start value for 'pivot' option with index %d on '$sequence' command for sequence named '%s'.\n", pivotIndex, m_pCurrentSequence->name.c_str());
		return false;
	}

	Int32 pivotEnd = 0;
	if(!ReadInt32(pivotEnd))
	{
		ErrorMsg("Couldn't read end value for 'pivot' option with index %d on '$sequence' command for sequence named '%s'.\n", pivotIndex, m_pCurrentSequence->name.c_str());
		return false;
	}

	for(Uint32 i = 0; i < m_pCurrentSequence->pivots.size(); i++)
	{
		smdl::pivot_t pivot = m_pCurrentSequence->pivots[i];
		if(pivot.index == pivotIndex)
		{
			ErrorMsg("Pivot with index %d on '$sequence' command for sequence named '%s' already exists.\n", pivotIndex, m_pCurrentSequence->name.c_str());
			return false;
		}
	}

	smdl::pivot_t newPivot;
	newPivot.index = pivotIndex;
	newPivot.start = pivotStart;
	newPivot.end = pivotEnd;

	m_pCurrentSequence->pivots.push_back(newPivot);
	return true;
}

//===============================================
// @brief Function for $sequence option 'fps'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_FPS( void )
{
	Float value = 0;
	if(!ReadFloat(value))
	{
		ErrorMsg("Failure when reading value for 'fps' on '$sequence' with name '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	m_pCurrentSequence->fps = value;
	return true;
}

//===============================================
// @brief Function for $sequence option 'origin'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_Origin( void )
{
	Vector origin;
	for(Uint32 i = 0; i < 3; i++)
	{
		if(!ReadFloat(origin[i]))
		{
			ErrorMsg("Failure when reading offset %d for 'origin' on '$sequence' with name '%s'.\n", i, m_pCurrentSequence->name.c_str());
			return false;
		}
	}

	m_pCurrentSequence->adjust = origin;
	return true;
}

//===============================================
// @brief Function for $sequence option 'rotate'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_Rotate( void )
{
	Float value = 0;
	if(!ReadFloat(value))
	{
		ErrorMsg("Failure when reading value for 'rotate' on '$sequence' with name '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	m_pCurrentSequence->zrotation = (value + 90.0) * (M_PI / 180.0);
	return true;
}

//===============================================
// @brief Function for $sequence option 'scale'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_Scale( void )
{
	Float value = 0;
	if(!ReadFloat(value))
	{
		ErrorMsg("Failure when reading value for 'scale' on '$sequence' with name '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	m_pCurrentSequence->scaleup = value;
	return true;
}

//===============================================
// @brief Function for $sequence option 'movement_scale'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_MovementScale( void )
{
	Float value = 0;
	if(!ReadFloat(value))
	{
		ErrorMsg("Failure when reading value for 'movement_scale' on '$sequence' with name '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	m_pCurrentSequence->movementscale = value;
	return true;
}

//===============================================
// @brief Function for $sequence option 'loop'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_Loop( void )
{
	m_pCurrentSequence->flags |= STUDIO_LOOPING;
	return true;
}

//===============================================
// @brief Function for $sequence option 'frame'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_Frame( void )
{
	Float firstFrame = 0;
	if(!ReadFloat(firstFrame))
	{
		ErrorMsg("Failure when reading first frame value for 'frame' on '$sequence' with name '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	Float lastFrame = 0;
	if(!ReadFloat(lastFrame))
	{
		ErrorMsg("Failure when reading first frame value for 'frame' on '$sequence' with name '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	m_pCurrentSequence->startframe = firstFrame;
	m_pCurrentSequence->endframe = lastFrame;
	return true;
}

//===============================================
// @brief Function for $sequence option 'blend'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_Blend( void )
{
	CString movementTypeString;
	if(!ReadString(movementTypeString))
	{
		ErrorMsg("Failure when reading blend type for 'blend' option on '$sequence' with name '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	Float blendStart = 0;
	if(!ReadFloat(blendStart))
	{
		ErrorMsg("Failure when reading blend start value for 'blend' on '$sequence' option with name '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	Float blendEnd = 0;
	if(!ReadFloat(blendEnd))
	{
		ErrorMsg("Failure when reading blend end value for 'blend' on '$sequence' option with name '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	Int32 movementType = GetMotionTypeFromString(movementTypeString.c_str());
	if(movementType == STUDIO_MT_NONE)
	{
		ErrorMsg("Invalid motion type '%s' specified for 'blend' option on '$sequence' with name '%s'.\n", movementTypeString.c_str(), m_pCurrentSequence->name.c_str());
		return false;
	}

	m_pCurrentSequence->blendtype[0] = movementType;
	m_pCurrentSequence->blendstart[0] = blendStart;
	m_pCurrentSequence->blendend[0] = blendEnd;

	return true;
}

//===============================================
// @brief Function for $sequence option 'node'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_Node( void )
{
	Int32 nodeIndex = 0;
	if(!ReadInt32(nodeIndex))
	{
		ErrorMsg("Couldn't read node index for 'node' option on '$sequence' command for sequence named '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	m_pCurrentSequence->entrynode = nodeIndex;
	m_pCurrentSequence->exitnode = nodeIndex;
	return true;
}

//===============================================
// @brief Function for $sequence option 'transition'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_Transition( void )
{
	Int32 entryNodeIndex = 0;
	if(!ReadInt32(entryNodeIndex))
	{
		ErrorMsg("Couldn't read entry node index for 'transition' option on '$sequence' command for sequence named '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	Int32 exitNodeIndex = 0;
	if(!ReadInt32(exitNodeIndex))
	{
		ErrorMsg("Couldn't read exit node index for 'transition' option on '$sequence' command for sequence named '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	m_pCurrentSequence->entrynode = entryNodeIndex;
	m_pCurrentSequence->exitnode = exitNodeIndex;
	return true;
}

//===============================================
// @brief Function for $sequence option 'rtransition'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_ReverseTransition( void )
{
	Int32 entryNodeIndex = 0;
	if(!ReadInt32(entryNodeIndex))
	{
		ErrorMsg("Couldn't read entry node index for 'transition' option on '$sequence' command for sequence named '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	Int32 exitNodeIndex = 0;
	if(!ReadInt32(exitNodeIndex))
	{
		ErrorMsg("Couldn't read exit node index for 'transition' option on '$sequence' command for sequence named '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	m_pCurrentSequence->entrynode = entryNodeIndex;
	m_pCurrentSequence->exitnode = exitNodeIndex;
	m_pCurrentSequence->nodeflags |= (1<<0);
	return true;
}

//===============================================
// @brief Function for $sequence option 'animation'
//
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CQCParser::SeqOption_Animation( void )
{
	const Char* pstrString = nullptr;
	if(!ReadString(pstrString))
	{
		ErrorMsg("Couldn't read animation name for option 'animation' on '$sequence' command for sequence named '%s'.\n", m_pCurrentSequence->name.c_str());
		return false;
	}

	m_pCurrentSequence->smdfilenames.push_back(pstrString);
	return true;
}
