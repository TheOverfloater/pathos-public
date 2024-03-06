/*
===============================================
Pathos Engine - Created by Andrew "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/


#ifndef R_VBMBASIC_H
#define R_VBMBASIC_H

#include "includes.h"

#include "studio.h"
#include "vbmformat.h"
#include "flex_shared.h"

class CGLSLShader;
class CVBO;
class CFlexManager;
class CMatrix;

struct en_texture_t;
struct en_texalloc_t;

struct vbm_glvertex_t
{
	vbm_glvertex_t()
	{
		memset(texcoord, 0, sizeof(texcoord));
		memset(boneindexes, 0, sizeof(boneindexes));
		memset(boneweights, 0, sizeof(boneweights));
		memset(flexcoord, 0, sizeof(flexcoord));
	}
		
	Vector origin;
	Vector tangent;
	Vector binormal;
	Vector normal;
	Float texcoord[2];
	Float boneindexes[MAX_VBM_BONEWEIGHTS];
	Float boneweights[MAX_VBM_BONEWEIGHTS];
	Float flexcoord[2];
};

struct vbm_attribs
{
	vbm_attribs():
		a_origin(0),
		a_tangent(0),
		a_binormal(0),
		a_normal(0),
		a_texcoord(0),
		a_boneindexes(0),
		a_boneweights(0),
		a_flexcoord(0),
		u_projection(0),
		u_modelview(0),
		u_normalmatrix(0),
		u_flextexture(0),
		u_flextexturesize(0),
		u_scroll(0),
		u_vorigin(0),
		u_vright(0),
		u_texture0(0),
		u_rectangle(0),
		u_spectexture(0),
		u_lumtexture(0),
		u_normalmap(0),
		u_sky_ambient(0),
		u_sky_diffuse(0),
		u_sky_dir(0),
		u_color(0),
		u_scope_scale(0),
		u_scope_scrsize(0),
		u_phong_exponent(0),
		u_specularfactor(0),
		d_shadertype(0),
		d_chrome(0),
		d_alphatest(0),
		d_flexes(0),
		d_specular(0),
		d_luminance(0),
		d_bumpmapping(0)
		{
			for(Uint32 i = 0; i < MAX_SHADER_BONES; i++)
				boneindexes[i] = 0;
		}

	Int32 a_origin;
	Int32 a_tangent;
	Int32 a_binormal;
	Int32 a_normal;
	Int32 a_texcoord;
	Int32 a_boneindexes;
	Int32 a_boneweights;
	Int32 a_flexcoord;

	Int32 u_projection;
	Int32 u_modelview;
	
	Int32 u_normalmatrix;

	Int32 u_flextexture;
	Int32 u_flextexturesize;

	Int32 u_scroll;

	Int32 boneindexes[MAX_SHADER_BONES];

	Int32 u_vorigin;
	Int32 u_vright;

	Int32 u_texture0;
	Int32 u_rectangle;
	Int32 u_spectexture;
	Int32 u_lumtexture;
	Int32 u_normalmap;

	Int32 u_sky_ambient;
	Int32 u_sky_diffuse;
	Int32 u_sky_dir;

	Int32 u_color;

	Int32 u_scope_scale;
	Int32 u_scope_scrsize;

	Int32 u_phong_exponent;
	Int32 u_specularfactor;

	Int32 d_shadertype;
	Int32 d_chrome;
	Int32 d_alphatest;
	Int32 d_flexes;
	Int32 d_specular;
	Int32 d_luminance;
	Int32 d_bumpmapping;
};

/*
=================================
CBasicVBMRenderer

=================================
*/
class CBasicVBMRenderer
{
private:
	// Max bone controllers
	static const Uint32 MAX_BONECONTROLLERS = 4;
	// Max blenders
	static const Uint32 MAX_BLENDERS = 2;
	// Number of random colors
	static const Uint32 NB_RANDOM_COLORS = 15;
	// Temporary vertex number for drawing special stuff
	static const Uint32 TEMP_VERTEX_COUNT = MAXSTUDIOBONES*2;

public:
	// Array of RANDOM colors
	static const Vector RANDOM_COLOR_ARRAY[NB_RANDOM_COLORS];
	// Mouth controller index
	static const Int32 MOUTH_CONTROLLER_INDEX;
	// Shader file name
	static const Char SHADER_FILE_NAME[];
	// Materials scripts base path
	static const Char MODEL_MATERIALS_BASE_PATH[];
	// Eyeglint texture path
	static const Char EYEGLINT_TEXTURE_PATH[];
	// Bone lines color setting
	static const Vector BONE_LINES_COLOR;
	// Bone origin color setting
	static const Vector BONE_ORIGIN_COLOR;
	// Attachment lines color setting
	static const Vector ATTACHMENT_LINES_COLOR;
	// Attachment origin color setting
	static const Vector ATTACHMENT_ORIGIN_COLOR;

public:
	typedef void (*pfnSetFlexValues_t)( void );
	typedef void (*pfnErrorPopup_t)( const Char *fmt, ... );
	typedef const Char* (*pfnGetFlexLabel_t)( Int32 flexindex );
	typedef Int32 (*pfnGetFlexControllerIndex_t)( flexscript_t* pflexscript, const Char* pstrName );
	typedef void (*pfnVBMEvent_t)( const mstudioevent_t* pevent, const CArray<Vector>* pAttachmentsVector );

	enum rendermode_t
	{
		RENDER_WIREFRAME = 0,
		RENDER_SMOOTHSHADED,
		RENDER_TEXTURED
	};

private:
	CBasicVBMRenderer( CGLExtF& glExtF, const file_interface_t& fileFuncs, pfnErrorPopup_t pfnErrorPopupFn );
	~CBasicVBMRenderer( void );

public:
	// Initializes the renderer
	bool Init( void );
	// Shuts down the renderer
	void Shutdown( void );	

	// Intializes the VBM object
	void InitVBM( void );
	// Destroys the VBM object
	void DestroyVBM( void );
	// Initializes shader objects
	bool InitShader( void );

	// Renders the model
	bool DrawModel( CMatrix& modelview, CMatrix& projection );
	// Advances the frame
	void AdvanceFrame( Float dt );

	// Sets up bones
	void SetUpBones ( void );
	// Calculates flexes
	void CalculateFlexes ( void );

public:
	// Extracts the bounding box
	void GetSequenceBBox( Vector& mins, Vector& maxs );
	// Returns the current sequence
	Int32 GetSequence( void );
	// Gets sequence information
	void GetSequenceInfo( Float& frameRate, Int32& numFrames, Float& groundSpeed );
	// Returns the current frame
	Float GetFrame( void );
	// Returns the current body number
	Uint64 GetBodyNum() { return m_bodyValue; }
	// Returns the current ground speed
	Float GetGroundSpeed( void );
	// Returns mouth value
	Float GetMouthOpen( void );

public:
	// Sets the current sequence
	Int32 SetSequence( Int32 sequenceIndex );
	// Sets controller value
	Float SetController( Int32 controller, Float value );
	// Returns mouth value
	void SetMouthOpen( Float value );
	// Sets mouth value
	Float SetMouthController( Float flValue );
	// Sets blending on a blender
	Float SetBlending( Int32 blender, Float value );
	// Sets bodygroup value
	Int32 SetBodyGroup( Int32 group, Int32 value );
	// Sets skin value
	Int32 SetSkin( Int32 iValue );
	// Sets the current frame
	Float SetFrame( Float nFrame );
	// Sets the transparency value
	void SetTransparency( Float transparency );
	// Sets whether mesh divisions should be shown
	void SetShowMeshDivisions( bool showDivisions );
	// Sets whether mesh divisions should be shown
	void SetDrawWireframeOverlay( bool drawWireframeOverlay );
	// Sets whether bones should be shown
	void SetShowBones( bool showBones );
	// Sets whether bones should be shown
	void SetShowAttachments( bool showAttachments );
	// Sets whether bones should be shown
	void SetShowHitBoxes( bool showHitBoxes );
	// Sets the light color
	void SetLightColor( const Vector& lightColor );
	// Sets a flex value
	void SetFlexValue( Uint32 index, Float value );
	// Sets the FOV value
	void SetFOVValue( Float fov );
	// Sets screen size
	void SetScreenSize( Float screenWidth, Float screenHeight );
	// Sets the "SetFlexValues" function pointer
	void SetSetFlexValuesFunctionPointer( pfnSetFlexValues_t pfnSetFlexValues );
	// Sets the VBMEvent function pointer
	void SetVBMEventFunctionPointer( pfnVBMEvent_t pfnVBMEvent );
	// Sets the render mode
	void SetRenderMode( rendermode_t rendermode );

	// Sets current time
	void SetTime( Double time );
	// Sets frametime
	void SetFrameTime( Double frametime );
	// Sets the flex state pointer
	void SetFlexStatePointer( flexstate_t* pFlexState );
	// Sets the studiomdl file pointer
	void SetStudioHeader( studiohdr_t* pstudiohdr );
	// Sets the vbm model file header
	void SetVBMHeader( vbmheader_t* pvbmheader );
	// Sets the view origin
	void SetViewOrigin( const Vector& viewOrigin );
	// Sets the view origin
	void SetViewAngles( const Vector& viewAngles );
	// Sets whether we're playing a script
	void SetScriptPlayback( bool isScriptPlayback );
	// Sets the flex manager pointer
	void SetFlexManager( CFlexManager* pFlexManager );

	// Returns the shader's error message
	const Char* GetShaderError( void ) const;
	// Tells if the shader has an error
	bool HasError( void ) const;
	
	// Calculates chrome for UV viewer
	void Chrome( const Vector& viewOrigin, const Vector& viewRight, const byte* pboneindexes, const vbmvertex_t* pvertex, Float *pchromecoords );

	// Calculates attachment positions
	void CalculateAttachments( void );
	// Plays back animation events
	void PlayEvents( void );

public:
	// Creates an instance of this class
	static CBasicVBMRenderer* CreateInstance( CGLExtF& glExtF, const file_interface_t& fileFuncs, pfnErrorPopup_t pfnErrorPopupFn );
	// Returns the current instance of this class
	static CBasicVBMRenderer* GetInstance( void );
	// Deletes the current instance of this class
	static void DeleteInstance( void );

private:
	// Returns the ideal LOD for the lod type specified
	const vbmsubmodel_t* GetIdealLOD( const vbmsubmodel_t* psubmodel, vbmlod_type_t type );
	// Draws meshes for a submodel
	bool DrawSubmodel( void );
	// Draws solid meshes for a submodel
	bool DrawSubmodelSolid( bool setFlexes );
	// Draws a single mesh
	bool DrawMesh( en_material_t *pmaterial, const vbmmesh_t *pmesh, bool drawBlended );
	// Draws the bounding box
	void DrawBox( const Vector& bbmin, const Vector& bbmax);

	// Sets up lighting
	void SetupLighting( void );
	// Sets up the submodel to render
	void SetupModel( int bodypart );
	// Draws bones
	bool DrawBones( void );
	// Draws hitboxes
	bool DrawHitBoxes( void );
	// Draws bounding box
	bool DrawBoundingBox( void );
	// Draws attachemnts
	bool DrawAttachments( void );

private:
	// Pointer to VBM data
	vbmheader_t *m_pVBMHeader;
	// Pointer to studiomdl data
	studiohdr_t *m_pStudioHeader;
	// Pointer to current VBM submodel
	const vbmsubmodel_t *m_pVBMSubmodel;

	// Transparency
	Float m_transparency;
	// Bone controllers
	Float m_boneController[MAX_BONECONTROLLERS];
	// Blender values
	Float m_blending[MAX_BLENDERS];
	// Mouth value
	Float m_mouth;

	// TRUE if hardware supports vertex texture fetch
	bool m_vertexfetchsupport;

	// Model's origin
	Vector m_origin;
	// Model's angles
	Vector m_angles;
	// Adjuster
	Float m_adj[4];
	// Light vector
	Vector m_lightDirection;
	// Light color
	Vector m_lightColor;

	// Ambient light strength
	Int32 m_ambientlight;
	// Current sequence index
	Int32 m_sequence;
	// Body setting value
	Uint64 m_bodyValue;
	// Current skin value
	Int32 m_skinNumber;
	// Mouth open value
	byte m_mouthOpen;

	// Current ground speed
	Float m_groundSpeed;
	// Last frame anim events played on
	Float m_lastEventFrame;
	// Frame on previous frame
	Int32 m_prevFrameSequence;
	// Sequence on previous frame
	Float m_prevFrameFrame;

	// Current frame
	Float m_currentFrame;
	// Direct world light
	Float m_shadeLight;
	// Bone transformation matrix
	Float m_boneTransform[MAXSTUDIOBONES][3][4];
	// Vertex weight bone transformation matrix
	Float m_weightBoneTransform[MAXSTUDIOBONES][3][4];

	// Attachment array
	CArray<Vector> m_attachmentsArray;

	// Triangle counter
	GLuint m_triangleCounter;

	// Current time
	Double m_time;
	// Current frametime
	Double m_frameTime;

	// View origin
	Vector m_viewOrigin;
	// View angles
	Vector m_viewAngles;

	// Flex state pointer
	flexstate_t* m_pFlexState;
	// Flex values arra
	Float* m_pFlexValues;
	// Tells if we're playing a script
	bool m_scriptPlayback;
	// If we're in flex scripting mode
	bool m_isInFlexScriptingMode;

	// Tells if mesh divisions should be rendered
	bool m_showMeshDivisions;
	// Tells if wireframe overlay should be drawn
	bool m_wireframeOverlay;
	// Tells if bones should be rendered
	bool m_showBones;
	// Tells if attachments should be shown
	bool m_showAttachments;
	// Tells if hitboxes should be shown
	bool m_showHitBoxes;

	// Render mode used
	rendermode_t m_renderMode;
	// FOV value
	Float m_fovValue;
	// Screen width
	Float m_screenWidth;
	// Screen height
	Float m_screenHeight;

private:
	// Quaternion and vector arrays used for bone transforms
	Vector	m_bonePositions1[MAXSTUDIOBONES];
	vec4_t	m_boneQuaternions1[MAXSTUDIOBONES];
	// Quaternion and vector arrays used for bone transforms
	Vector	m_bonePositions2[MAXSTUDIOBONES];
	vec4_t	m_boneQuaternions2[MAXSTUDIOBONES];
	// Quaternion and vector arrays used for bone transforms
	Vector	m_bonePositions3[MAXSTUDIOBONES];
	vec4_t	m_boneQuaternions3[MAXSTUDIOBONES];
	// Quaternion and vector arrays used for bone transforms
	Vector	m_bonePositions4[MAXSTUDIOBONES];
	vec4_t	m_boneQuaternions4[MAXSTUDIOBONES];
	// Quaternion and vector arrays used for bone transforms
	Vector	m_bonePositions5[MAXSTUDIOBONES];
	vec4_t	m_boneQuaternions5[MAXSTUDIOBONES];
	// Used for bone transform calculations
	Float	m_boneMatrix[3][4];

private:
	// Pointer to shader object
	CGLSLShader* m_pShader;
	// Pointer to VBO object
	CVBO* m_pVBO;
	// Shader attribs
	vbm_attribs m_attribs;

	// Draw buffer offset
	Uint32 m_drawBufferOffset;
	// Temp draw buffer
	vbm_glvertex_t m_tempVertexes[TEMP_VERTEX_COUNT];
	// Temp vertex count
	Uint32 m_numTempVertexes;

	// Eye glint texture
	en_texture_t* m_pGlintTexture;
	// Screen texture allocation
	en_texalloc_t* m_pScreenTexture;
	// Flex texture allocation
	en_texalloc_t* m_pFlexTexture;
	// Screen texture width
	Uint32 m_screenTextureWidth;
	// Screen texture height
	Uint32 m_screenTextureHeight;

	// Flex manager instance
	CFlexManager* m_pFlexManager;
	// Pointer to CGLExtF class
	CGLExtF& m_glExtF;

private:
	// File functions structure
	const file_interface_t& m_fileFuncs;
	// Error popup function
	pfnErrorPopup_t m_pfnErrorPopup;
	// Sets flex values(mdl viewer)
	pfnSetFlexValues_t m_pfnSetFlexValues;
	// Function to call for animation events
	pfnVBMEvent_t m_pfnVBMEvent;

private:
	// Current instance of this renderer object
	static CBasicVBMRenderer* g_pInstance;
};
extern CBasicVBMRenderer gBasicVBMRenderer;
#endif // R_VBMBASIC_H