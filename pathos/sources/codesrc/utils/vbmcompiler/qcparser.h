/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef QCPARSER_H
#define QCPARSER_H

#include "parserbase.h"
#include "studiocompiler.h"
#include "vbmcompiler.h"

/*
=======================
CQCParser

=======================
*/
class CQCParser : public CParserBase
{
public:
	typedef bool (CQCParser::*pfnProcessorFunction)( void );

	struct ProcessorFnMapping_t
	{
		ProcessorFnMapping_t():
			ptrFunction(nullptr)
		{}

		ProcessorFnMapping_t( const Char* pstrFnName, pfnProcessorFunction pFunction ):
			name(pstrFnName),
			ptrFunction(pFunction)
		{}

		CString name;
		pfnProcessorFunction ptrFunction;
	};

	typedef CArray<ProcessorFnMapping_t> ProcessorFnArray_t;

	struct ControllerTypeMapping_t
	{
		ControllerTypeMapping_t( const Char* pstrId, Int32 _typeFlag ):
			stringId(pstrId),
			typeFlag(_typeFlag)
		{}

		CString stringId;
		Int32 typeFlag;
	};

public:
	// Command processor function name->function ptr mappings
	static const ProcessorFnMapping_t COMMAND_PROCESSOR_FUNCTIONS[];
	// Option processor function name->function ptr mappings
	static const ProcessorFnMapping_t SEQUENCE_OPTION_PROCESSOR_FUNCTIONS[];
	// Bone controller type mappings
	static const ControllerTypeMapping_t CONTROLLER_TYPE_MAPPINGS[];
	// Activity mappings filename
	static const Char ACTIVITY_DEFINITIONS_FILENAME[];

public:
	CQCParser( CStudioModelCompiler& studioCompiler, CVBMCompiler* pVBMCompiler, const Char* pstrExecutablePath );
	~CQCParser( void );

public:
	// Processes a file
	virtual bool ProcessFile( const Char* pstrFilename ) override;
	// Clears all data used
	virtual void Clear( void ) override;

private:
	// Loads activity mapping definitions
	bool LoadActivityMappings( const Char* pstrFilePath );

	// Return motion type for string
	Int32 GetMotionTypeFromString( const Char* pstrString );

	// Note: Added these to make stepping easier in debugger
	// Get command processor function for name
	pfnProcessorFunction GetCommandProcessorFunctionForName( const Char* pstrName );
	// Get sequence option processor function for name
	pfnProcessorFunction GetSeqOptionProcessorFunctionForName( const Char* pstrName );

private:
	// Function for '$modelname' command
	bool Cmd_ModelName( void );
	// Function for '$cd' command
	bool Cmd_ChangeDirectory( void );
	// Function for '$cdtexture' command
	bool Cmd_ChangeTextureDirectory( void );
	// Function for '$scale' command
	bool Cmd_Scale( void );
	// Function for '$scale_movement' command
	bool Cmd_ScaleMovement( void );
	// Function for '$root' command
	bool Cmd_Root( void );
	// Function for '$pivot' command
	bool Cmd_Pivot( void );
	// Function for '$controller' command
	bool Cmd_BoneController( void );
	// Function for '$lod' command
	bool Cmd_LOD( void );
	// Function for '$body' command
	bool Cmd_Body( void );
	// Function for '$flexcontroller' command
	bool Cmd_FlexController( void );
	// Function for '$bodygroup' command
	bool Cmd_BodyGroup( void );
	// Function for '$sequence' command
	bool Cmd_Sequence( void );
	// Function for '$eyeposition' command
	bool Cmd_EyePosition( void );
	// Function for '$origin' command
	bool Cmd_Origin( void );
	// Function for '$bbox' command
	bool Cmd_BBox( void );
	// Function for '$cbox' command
	bool Cmd_CBox( void );
	// Function for '$mirrorbone' command
	bool Cmd_MirrorBone( void );
	// Function for '$gamma' command
	bool Cmd_Gamma( void );
	// Function for '$flags' command
	bool Cmd_Flags( void );
	// Function for '$texturegroup' command
	bool Cmd_TextureGroup( void );
	// Function for '$hgroup' command
	bool Cmd_HitGroup( void );
	// Function for '$hbox' command
	bool Cmd_HitBox( void );
	// Function for '$attachment' command
	bool Cmd_Attachment( void );
	// Function for '$renamebone' command
	bool Cmd_RenameBone( void );
	// Function for '$centerlight' command
	bool Cmd_CenterLight( void );
	// Function for '$noblend' command
	bool Cmd_NoBlendBone( void );
	// Function for '$protected' command
	bool Cmd_ProtectedBone( void );
	// Function for '$texrendermode' command
	bool Cmd_TextureRenderMode( void );
	// Function for '$movement_bone' command
	bool Cmd_MovementBone( void );
	// Function for '$skylight' command
	bool Cmd_SkyLight( void );

	// Function for '$cliptotextures' command
	bool Cmd_ClipToTextures( void );
	// Function for '$sequencegroup' command
	bool Cmd_SequenceGroup( void );
	// Function for '$sequencegroupsize' command
	bool Cmd_SequenceGroupSize( void );
	// Function for '$externaltextures' command
	bool Cmd_ExternalTextures( void );
	// Function for '$smallfov' command
	bool Cmd_SmallFOV( void );
	// Function for '$lockmodel' command
	bool Cmd_LockModel( void );

private:
	// Function for $sequence option 'deform'
	bool SeqOption_Deform( void );
	// Function for $sequence option 'event'
	bool SeqOption_Event( void );
	// Function for $sequence option 'pivot'
	bool SeqOption_Pivot( void );
	// Function for $sequence option 'fps'
	bool SeqOption_FPS( void );
	// Function for $sequence option 'origin'
	bool SeqOption_Origin( void );
	// Function for $sequence option 'rotate'
	bool SeqOption_Rotate( void );
	// Function for $sequence option 'scale'
	bool SeqOption_Scale( void );
	// Function for $sequence option 'movement_scale'
	bool SeqOption_MovementScale( void );
	// Function for $sequence option 'loop'
	bool SeqOption_Loop( void );
	// Function for $sequence option 'frame'
	bool SeqOption_Frame( void );
	// Function for $sequence option 'blend'
	bool SeqOption_Blend( void );
	// Function for $sequence option 'node'
	bool SeqOption_Node( void );
	// Function for $sequence option 'transition'
	bool SeqOption_Transition( void );
	// Function for $sequence option 'rtransition'
	bool SeqOption_ReverseTransition( void );
	// Function for $sequence option 'animation'
	bool SeqOption_Animation( void );

private:
	// Sequence being processed
	smdl::sequence_t* m_pCurrentSequence;
	// Current bracket count in $sequence command
	Int32 m_bracketCount;
	// TRUE if we should cross over newlines
	bool m_ignoreLineEnds;

	// Activity mappings
	std::map<CString, Int32> m_activityNameIdMap;

	// Executable path
	CString m_executableFolderPath;

private:
	// Command name->Processor function map for QC file processing
	ProcessorFnArray_t m_cmdProcessorFunctionArray;
	// Command name->Processor function map for QC file processing
	ProcessorFnArray_t m_sequenceOptionProcessorFunctionArray;

	// Studiomodel compiler we are loading for
	CStudioModelCompiler& m_studioCompiler;
	// VBM compiler we are loading for
	CVBMCompiler* m_pVBMCompiler;
};
#endif //QCPARSER_H