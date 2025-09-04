/*/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef STUDIOCOMPILER_H
#define STUDIOCOMPILER_H

#include <map>
#include <set>

#include "compiler_types.h"
#include "cbuffer.h"

/*
=======================
CStudioModelCompiler

=======================
*/
class CStudioModelCompiler
{
public:
	// Default allocation size for MDL file buffer
	static const Uint32 DEFAULT_MDL_ALLOCATION_SIZE;
	// Default minimum weight treshold
	static const Float MINIMUM_WEIGHT_TRESHOLD;
	// Default texture gamma
	static const Float DEFAULT_TEXTURE_GAMMA_VALUE;
	// Default FPS for sequences
	static const Float DEFAULT_SEQUENCE_FPS;
	// Rendermode definitions file name
	static const Char RENDERMODE_DEFINITIONS_FILENAME[];
	// Default rendermode definitions
	static const smdl::rendermode_definition_t DEFAULT_RENDERMODE_DEFINITIONS[];
	// Hitgroup definitions file
	static const Char HITGROUP_DEFINITIONS_FILENAME[];
	// Hitgroup->bone automapping file
	static const Char BONE_HITGROUP_AUTOMAP_FILENAME[];

public:
	CStudioModelCompiler( void );
	~CStudioModelCompiler( void );

public:
	// Initializes the class
	bool Init( const Char* pstrModuleFileName );
	// Completely clear the class
	void Clear( void );

	// Processes the input data into finalized data
	bool ProcessInputData( void );
	// Writes MDL data to the output file
	bool WriteMDLFile( void );

public:
	// Set default values for scales, etc
	void SetDefaultValues( void );

	// Add texture folder to the list
	void AddTextureFolderPath( const Char* pstrPath );
	// Sets scale value
	void SetScaleUpValue( Float value );
	// Sets scale value
	void SetDefaultScaleValue( Float value );

	// Add pivot bone name
	void AddPivot( const Char* pstrBoneName );
	// Set root pivot bone name
	void SetRootPivot( const Char* pstrBoneName );

	// Sets CBox(How is this different from "bbox"?)
	void SetCBox( const Vector& mins, const Vector& maxs );
	// Sets BBox(How is this different from "cbox"?)
	void SetBBox( const Vector& mins, const Vector& maxs );
	// Sets the eye position
	void SetEyePosition( const Vector& eyePosition );

	// Set default adjustment value
	void SetDefaultAdjustVector( const Vector& adjustmentVector );
	// Set default z rotation value
	void SetDefaultZRotation( Float value );
	// Returns the default Z rotation value
	Float GetDefaultZRotation( void );

	// Add a bone to the list of mirrored bones
	void AddMirroredBone( const Char* pstrBoneName );
	// Tells if this bone is marked to be mirrored
	bool IsMirroredBone( const Char* pstrBoneName );
	// Add a bone to the list of protected bones
	void AddProtectedBone( const Char* pstrBoneName );
	// Add a bone to the list of no-blend bones
	void AddNoBlendBone( const Char* pstrBoneName );
	// Sets the movement bone
	bool SetMovementBone( const Char* pstrBoneName );
	// Adds a renamed bone to the mappings
	bool AddRenamedBone( const Char* pstrOriginalName, const Char* pstrNewName );
	// Adds a new attachment
	bool AddAttachment( const Char* pstrBoneName, Int32 index, const Vector& origin );
	// Adds a hitbox to the list
	bool AddHitBox( Int32 hitgroup, const Char* pstrBoneName, const Vector& mins, const Vector& maxs );
	// Adds a hitgroup to the list
	bool AddHitGroup( Int32 hitgroup, const Char* pstrGroupName );
	// Adds a texture rendermode setting
	bool AddTextureRenderMode( const Char* pstrTextureName, const Char* pstrRenderMode );
	// Adds a texture group to the list
	bool AddTextureGroup( const Char* pstrGroupName, CArray<CArray<CString>>& textureGroupsArray );
	// Adds a new bone controller
	bool AddBoneController( Int32 controllerIndex, const Char* pstrBoneName, Int32 typeFlags, Float startValue, Float endValue );
	// Adds a new LOD definition
	bool AddLOD( const Char* pstrSubmodelName, const Char* pstrLODFileName, vbmlod_type_t lodType, Float distance, bool reverseTriangles, Float scale );

	// Adds a renamed texture
	void AddRenamedTexture( const Char* pstrOriginalName, const Char* pstrNewName );
	// Get rename for an original texture name
	bool GetTextureRename( const Char* pstrOriginalName, const Char*& pstrNewName );
	// Return a skinref for an existing texture, or add a new one and return that
	smdl::texture_t* GetTextureForName( const Char* pstrTexture, bool noCreate = false );

	// Adds a new bodygroup
	smdl::bodypart_t* AddBodyGroup( const Char* pstrGroupName );
	// Adds a new submodel
	bool AddSubmodel( smdl::bodypart_t* pBodyGroup, const Char* pstrSubmodelName, const Char* pstrVTAName, bool reverseTriangles, Float scale );

	// Adds a new sequence and returns it's pointer
	smdl::sequence_t* AddSequence( const Char* pstrSequenceName );

	// Applies offsets to a vector
	Vector ApplyOffset( const Vector& input );
	// Applies scaling on a coordinate
	Vector ApplyScaling( const Vector& input );
	// Clips values on a rotation vector
	Vector ClipRotations( const Vector& input );

	// Set gamma
	void SetGamma( Float gamma );
	// Sets a studio flag value
	void SetModelFlag( Int32 flag );

	// Get normal merge treshold
	Float GetNormalMergeTreshold( void ) { return m_normalMergeTreshold; }
	// Get weight treshold
	Float GetWeightTreshold( void ) { return m_weightTreshold; }
	// Returns the skinrefs array
	const CArray<CArray<Int32>>& GetSkinRefsArray( void ) const { return m_skinRefsArray; }
	// Returns the number of skin families
	Uint32 GetNbSkinFamilies( void ) const { return m_numSkinFamilies; }
	// Returns the number of skin families
	Uint32 GetNbSkinRefs( void ) const { return m_numSkinRefs; }

	// Returns a bone info from the table
	const smdl::boneinfo_t* GetBone( Int32 boneIndex ) const { return m_pBoneTableArray[boneIndex]; };
	// Returns a bone info from the table
	const smdl::boneinfo_t* GetBone( const Char* pstrBoneName ) const;
	// Return the number of bones
	Uint32 GetNbBones( void ) { return m_pBoneTableArray.size(); }

	// Get submodel by index
	const smdl::submodel_t* GetSubModel( Int32 index ) const { return m_pSubmodelsArray[index]; }
	// Return the number of submodels
	Uint32 GetNbSubmodels( void ) const { return m_pSubmodelsArray.size(); }

	// Return a bodypart by index
	const smdl::bodypart_t* GetBodyPart( Int32 index ) const { return m_pBodyPartsArray[index]; }
	// Returns the number of body parts
	Uint32 GetNbBodyParts( void ) { return m_pBodyPartsArray.size(); }

	// Returns a texture by index
	const smdl::texture_t* GetTexture( Int32 index ) const { return m_pTexturesArray[index]; }
	// Returns the number of textures
	Uint32 GetNbTextures( void ) const { return m_pTexturesArray.size(); }

private:
	// Set up textures to use
	bool SetupTextures( void );
	// Load a texuture
	bool LoadTexture( smdl::texture_t* ptexture );

	// Checks animations for looping and event validity
	void CheckAnimations( void );
	// Extract motion from sequences
	void ExtractSequenceMotion( void );
	// Create node transition mappings
	void CreateNodeTransitionMappings( void );
	// Mark all used bones on meshes
	void MarkUsedBones( void );
	// Mark used bones in a submodel
	void MarkSubmodelUsedBones( smdl::submodel_t* psubmodel );
	// Rename any bones
	void RenameBones( void );
	// Renames sequence bone
	void RenameSequenceBones( void );
	// Maps each sequence bone to the global list
	bool RemapSequenceBones( void );
	// Link bone controllers with their bones
	bool LinkBoneControllers( void );
	// Link attachments
	bool LinkAttachments( void );
	// Sets final bone indexes on all elements of all submodels
	void SetFinalBoneIndexes( void );
	// Sets final bone indexes on all elements of a submodel
	void SetFinalSubmodelBoneIndexes( smdl::submodel_t* psubmodel );
	// Set hitgroups
	bool SetupHitGroups( void );
	// Sets up Hitboxes
	bool SetupHitBoxes( void );
	// Builds up hitboxes if there were none defined
	void BuildHitBoxes( void );
	// Sets final animation bone positions
	void SetFinalSequenceBonePositions( void );
	// Determine bone scale values
	void CalculateBoneScales( void );
	// Determine bounding box sizes for each sequence
	void CalculateSequenceBoundingBoxes( void );
	// Compresses animation data using RLE compression
	void CompressAnimationData( void );
	// Set flags for textures
	bool SetTextureFlags( void );
	// Calculate submodel s/t coordinates
	void CalculateSubmodelSTCoords( smdl::submodel_t* psubmodel );

	// Determine hitbox time based on bone name
	Int32 DetermineBoneHitGroupType( const Char* pstrBoneName );

	// Poplates the bone table with the union of all bones used
	bool CreateBoneTable( void );
	// Adds a submodel's bones to the bone table
	bool AddSubmodelBonesToTable( smdl::submodel_t* psubmodel );
	// Returns the index of a bone in the bone table array
	Int32 GetBoneIndex( const Char* pstrBoneName );

	// Loads the rendermode definitions file
	bool LoadRenderModeDefinitions( const Char* pstrFilePath );
	// Loads hitgroup definitions file
	bool LoadHitGroupDefinitions( const Char* pstrFilePath );
	// Loads hitgroup definitions file
	bool LoadBoneHitGroupAutoMappings( const Char* pstrFilePath );

private:
	// Writes all data related to bones
	bool WriteBoneData( void );
	// Writes all animation data
	bool WriteAnimationData( void );
	// Writes all model geometry data
	bool WriteGeometryData( void );
	// Writes all texture data
	void WriteTextures( void );

private:
	// Bone table array
	CArray<smdl::boneinfo_t*> m_pBoneTableArray;
	// Bone rename mappings
	CStringMap_t m_boneRenameMap;
	// Mirrored bones list
	CStringList_t m_mirroredBonesList;
	// 'No-blend' bones list
	CStringList_t m_noBlendBonesList;
	// Protected bones list
	CStringList_t m_protectedBonesList;

	// Transition nodes array
	CArray<CArray<Int32>> m_transitionNodesArray;

	// Array of bone controllers
	CArray<smdl::bonecontroller_t> m_boneControllersArray;
	// Array of attachments
	CArray<smdl::attachment_t> m_attachmentsArray;

	// Array of animations
	CArray<smdl::animation_t*> m_pAnimationsArray;
	// Array of sequences
	CArray<smdl::sequence_t*> m_pSequencesArray;

	// Array of hitboxes
	CArray<smdl::hitbox_t> m_hitBoxesArray;
	// Hitgroup mappings
	CArray<smdl::hitgroup_t> m_hitGroupArray;
	
	// Submodels array
	CArray<smdl::submodel_t*> m_pSubmodelsArray;
	// Bodyparts array
	CArray<smdl::bodypart_t*> m_pBodyPartsArray;
	// LODs array
	CArray<smdl::lod_t*> m_pLODsArray;

	// Texture folders array
	CArray<CString> m_textureFoldersArray;

	// Names of pivot points
	// Note: Cannot change to set because we need positions
	CArray<CString> m_pivotNamesArray;
	
	// Array of textures
	CArray<smdl::texture_t*> m_pTexturesArray;
	// Skin reference array
	CArray<CArray<Int32>> m_skinRefsArray;
	// Texture groups array
	CArray<smdl::texturegroup_t> m_textureGroupsArray;
	// Renamed textures map
	CStringMap_t m_renamedTextureMap;

	// Number of skin families
	Uint32 m_numSkinFamilies;
	// Number of skinrefs
	Uint32 m_numSkinRefs;

	// Rendermode definitions
	CArray<smdl::rendermode_definition_t> m_renderModeDefinitionsArray;
	// Texture render mode settings
	CArray<smdl::rendermode_t> m_textureRenderModesArray;

	// Hitgroup map bone discard list
	CArray<CString> m_hitgroupBoneDiscardList;
	// Bone name->hitgroup mappings
	CArray<smdl::hitgroup_bone_mapping_t> m_boneHitgroupAutoMappingsArray;

private:
	// File output buffer
	CBuffer* m_pFileOutputBuffer;
	// Studiomodel file header
	studiohdr_t* m_pStudioHeader;

	// Default scale value
	Float m_defaultScale;
	// Scale up value
	Float m_scaleUpValue;

	// Adjustment vector
	Vector m_adjustmentVector;
	// Adjustment vector
	Vector m_defaultAdjustVector;
	// Z default rotation
	Float m_defaultZRotation;

	// Default movement scale
	Float m_defaultMovementScale;
	// Current movement scale
	Float m_movementScale;

	// Eye position of model
	Vector m_eyePosition;
	// Studiomodel flags
	Int32 m_studioFlags;

	// Min/maxs for model
	Vector m_bBoxMins;
	Vector m_bBoxMaxs;

	// Bounding box mins/maxs(What's the point of this?)
	Vector m_cBoxMins;
	Vector m_cBoxMaxs;

	// Name of the bone that defines linear movement
	CString m_movementBoneName;

	// Weight minimum treshold for discard/merge
	Float m_weightTreshold;
	// Normal merge treshold
	Float m_normalMergeTreshold;
	// Texture gamma value
	Float m_textureGamma;
};
#endif // STUDIOCOMPILER_H