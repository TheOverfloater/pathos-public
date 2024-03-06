/*
===============================================
Pathos Engine - Created by Andrew "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "mdlviewer.h"
#include "r_vbmbasic.h"
#include "texturemanager.h"
#include "flex_shared.h"
#include "flexmanager.h"

#include "r_vbo.h"
#include "r_glsl.h"
#include "r_common.h"
#include "matrix.h"
#include "vbm_shared.h"
#include "viewerstate.h"

// Color array for mesh colors
const Vector CBasicVBMRenderer::RANDOM_COLOR_ARRAY[NB_RANDOM_COLORS] = 
{
	Vector(1.0, 0.0, 0.0),
	Vector(0.0, 1.0, 0.0),
	Vector(0.0, 0.0, 1.0),
	Vector(1.0, 1.0, 0.0),
	Vector(0.0, 1.0, 1.0),
	Vector(0.5, 1.0, 0.5),
	Vector(0.0, 1.0, 0.5),
	Vector(0.5, 1.0, 0.0),
	Vector(0.1, 0.6, 0.9),
	Vector(0.5, 0.2, 0.5),
	Vector(0.3, 0.8, 0.1),
	Vector(0.5, 0.0, 0.4),
	Vector(0.8, 0.1, 0.2),
	Vector(0.8, 0.8, 0.3),
	Vector(0.9, 0.5, 0.1),
};

enum vbm_shtype
{
	vbm_texture = 0,
	vbm_notexture,
	vbm_solid,
	vbm_scope,
	vbm_texonly,
};

// Mouth controller index
const Int32 CBasicVBMRenderer::MOUTH_CONTROLLER_INDEX = 4;
// Shader file name
const Char CBasicVBMRenderer::SHADER_FILE_NAME[] = "vbmbasic.bss";
// Materials scripts base path
const Char CBasicVBMRenderer::MODEL_MATERIALS_BASE_PATH[] = "models/";
// Eyeglint texture path
const Char CBasicVBMRenderer::EYEGLINT_TEXTURE_PATH[] = "general/eyeglint.tga";
// Bone lines color setting
const Vector CBasicVBMRenderer::BONE_LINES_COLOR = Vector(0.0, 1.0, 0.0);
// Bone origin color setting
const Vector CBasicVBMRenderer::BONE_ORIGIN_COLOR = Vector(1.0, 0.0, 0.0);
// Attachment lines color setting
const Vector CBasicVBMRenderer::ATTACHMENT_LINES_COLOR = Vector(0.0, 0.0, 1.0);
// Attachment origin color setting
const Vector CBasicVBMRenderer::ATTACHMENT_ORIGIN_COLOR = Vector(1.0, 0.0, 0.0);

// Current instance of this renderer object
CBasicVBMRenderer* CBasicVBMRenderer::g_pInstance = nullptr;

//=============================================
// @brief Constructor
//
//=============================================
CBasicVBMRenderer::CBasicVBMRenderer( CGLExtF& glExtF, const file_interface_t& fileFuncs, pfnErrorPopup_t pfnErrorPopupFn ):
	m_pVBMHeader(nullptr),
	m_pStudioHeader(nullptr),
	m_pVBMSubmodel(nullptr),
	m_transparency(1.0),
	m_mouth(0),
	m_vertexfetchsupport(true),
	m_ambientlight(0),
	m_sequence(0),
	m_bodyValue(0),
	m_skinNumber(0),
	m_mouthOpen(0),
	m_groundSpeed(0),
	m_lastEventFrame(0),
	m_prevFrameSequence(0),
	m_prevFrameFrame(0),
	m_currentFrame(0),
	m_shadeLight(0),
	m_triangleCounter(0),
	m_time(0),
	m_frameTime(0),
	m_pFlexState(nullptr),
	m_pFlexValues(nullptr),
	m_scriptPlayback(false),
	m_isInFlexScriptingMode(false),
	m_showMeshDivisions(false),
	m_wireframeOverlay(false),
	m_showBones(false),
	m_showAttachments(false),
	m_showHitBoxes(false),
	m_renderMode(RENDER_TEXTURED),
	m_fovValue(0),
	m_screenWidth(0),
	m_screenHeight(0),
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_drawBufferOffset(0),
	m_numTempVertexes(0),
	m_pGlintTexture(nullptr),
	m_pScreenTexture(nullptr),
	m_pFlexTexture(nullptr),
	m_screenTextureWidth(0),
	m_screenTextureHeight(0),
	m_pFlexManager(nullptr),
	m_glExtF(glExtF),
	m_fileFuncs(fileFuncs),
	m_pfnErrorPopup(pfnErrorPopupFn),
	m_pfnSetFlexValues(nullptr),
	m_pfnVBMEvent(nullptr)
{
	memset(m_adj, 0, sizeof(m_adj));
	memset(m_boneController, 0, sizeof(m_boneController));
	memset(m_blending, 0, sizeof(m_blending));
	memset(m_boneTransform, 0, sizeof(m_boneTransform));
	memset(m_weightBoneTransform, 0, sizeof(m_weightBoneTransform));
	memset(m_bonePositions1, 0, sizeof(m_bonePositions1));
	memset(m_boneQuaternions1, 0, sizeof(m_boneQuaternions1));
	memset(m_bonePositions2, 0, sizeof(m_bonePositions2));
	memset(m_boneQuaternions2, 0, sizeof(m_boneQuaternions2));
	memset(m_bonePositions3, 0, sizeof(m_bonePositions3));
	memset(m_boneQuaternions3, 0, sizeof(m_boneQuaternions3));
	memset(m_bonePositions4, 0, sizeof(m_bonePositions4));
	memset(m_boneQuaternions4, 0, sizeof(m_boneQuaternions4));
	memset(m_bonePositions5, 0, sizeof(m_bonePositions5));
	memset(m_boneQuaternions5, 0, sizeof(m_boneQuaternions5));
	memset(m_boneMatrix, 0, sizeof(m_boneMatrix));
}

//=============================================
// @brief Destructor
//
//=============================================
CBasicVBMRenderer::~CBasicVBMRenderer( void )
{
	Shutdown();

	if(m_pFlexValues)
		delete[] m_pFlexValues;
}

//=============================================
// @brief Sets the current sequence
//
//=============================================
Int32 CBasicVBMRenderer::SetSequence( Int32 sequenceIndex )
{
	if (!m_pStudioHeader)
		return m_sequence;

	if (sequenceIndex > m_pStudioHeader->numseq)
		return m_sequence;

	m_sequence = sequenceIndex;
	m_currentFrame = 0;
	m_prevFrameSequence = m_sequence;
	m_prevFrameFrame = 0;

	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_sequence);
	m_groundSpeed = sqrt( pseqdesc->linearmovement[0]*pseqdesc->linearmovement[0]+ pseqdesc->linearmovement[1]*pseqdesc->linearmovement[1]+ pseqdesc->linearmovement[2]*pseqdesc->linearmovement[2] );
	m_groundSpeed = m_groundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);

	return m_sequence;
}

//=============================================
// @brief Extracts the bounding box
//
//=============================================
void CBasicVBMRenderer::GetSequenceBBox( Vector& mins, Vector& maxs )
{
	if (!m_pStudioHeader)
		return;

	if (m_sequence < 0 || m_sequence >= m_pStudioHeader->numseq)
		return;

	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_sequence);
	
	mins[0] = pseqdesc[ m_sequence ].bbmin[0];
	mins[1] = pseqdesc[ m_sequence ].bbmin[1];
	mins[2] = pseqdesc[ m_sequence ].bbmin[2];

	maxs[0] = pseqdesc[ m_sequence ].bbmax[0];
	maxs[1] = pseqdesc[ m_sequence ].bbmax[1];
	maxs[2] = pseqdesc[ m_sequence ].bbmax[2];
}

//=============================================
// @brief Gets sequence information
//
//=============================================
void CBasicVBMRenderer::GetSequenceInfo( Float& frameRate, Int32& numFrames, Float& groundSpeed )
{
	if (!m_pStudioHeader)
		return;

	if (m_sequence < 0 || m_sequence >= m_pStudioHeader->numseq)
		return;

	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_sequence);

	if (pseqdesc->numframes > 1)
	{
		frameRate = pseqdesc->fps;
		numFrames = pseqdesc->numframes;

		groundSpeed = sqrt( pseqdesc->linearmovement[0]*pseqdesc->linearmovement[0]+ pseqdesc->linearmovement[1]*pseqdesc->linearmovement[1]+ pseqdesc->linearmovement[2]*pseqdesc->linearmovement[2] );
		groundSpeed = groundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		frameRate = 0;
		numFrames = 0;
		groundSpeed = 0;
	}
}

//=============================================
// @brief Returns the current frame
//
//=============================================
Float CBasicVBMRenderer::GetFrame( void ) 
{ 
	return m_currentFrame; 
}

//=============================================
// @brief Returns the current ground speed
//
//=============================================
Float CBasicVBMRenderer::GetGroundSpeed( void ) 
{ 
	return m_groundSpeed; 
}

//=============================================
// @brief Returns mouth value
//
//=============================================
Float CBasicVBMRenderer::GetMouthOpen( void )
{
	return m_mouthOpen;
}

//=============================================
// @brief Returns the current sequence
//
//=============================================
Int32 CBasicVBMRenderer::GetSequence( void )
{
	return m_sequence;
}

//=============================================
// @brief Sets controller value
//
//=============================================
Float CBasicVBMRenderer::SetController( Int32 controller, Float value )
{
	if (!m_pStudioHeader)
		return value;

	// find first controller that matches the index
	const mstudiobonecontroller_t *pbonecontroller = nullptr;
	for (Int32 i = 0; i < m_pStudioHeader->numbonecontrollers; i++)
	{
		const mstudiobonecontroller_t *pcontroller = m_pStudioHeader->getBoneController(i);
		if (pcontroller->index == controller)
		{
			pbonecontroller = pcontroller;
			break;
		}
	}

	if (!pbonecontroller)
		return value;

	// wrap 0..360 if it's a rotational controller
	if (pbonecontroller->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pbonecontroller->end < pbonecontroller->start)
			value = -value;

		// does the controller not wrap?
		if (pbonecontroller->start + 359.0 >= pbonecontroller->end)
		{
			if (value > ((pbonecontroller->start + pbonecontroller->end) / 2.0) + 180)
				value = value - 360;
			if (value < ((pbonecontroller->start + pbonecontroller->end) / 2.0) - 180)
				value = value + 360;
		}
		else
		{
			if (value > 360)
				value = value - (value / 360.0) * 360.0;
			else if (value < 0)
				value = value + ((value / -360.0) + 1) * 360.0;
		}
	}

	Float setting = (255 * (value - pbonecontroller->start) / (pbonecontroller->end - pbonecontroller->start));
	setting = clamp(setting, 0, 255);

	m_boneController[controller] = setting;

	return setting * (1.0 / 255.0) * (pbonecontroller->end - pbonecontroller->start) + pbonecontroller->start;
}

//=============================================
// @brief Sets blending on a blender
//
//=============================================
Float CBasicVBMRenderer::SetBlending( Int32 blender, Float value )
{
	if (!m_pStudioHeader)
		return 0;

	if (m_sequence < 0 || m_sequence >= m_pStudioHeader->numseq)
		return 0;

	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_sequence);

	if (pseqdesc->blendtype[blender] == 0)
		return value;

	if (pseqdesc->blendtype[blender] & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pseqdesc->blendend[blender] < pseqdesc->blendstart[blender])
			value = -value;

		// does the controller not wrap?
		if (pseqdesc->blendstart[blender] + 359.0 >= pseqdesc->blendend[blender])
		{
			if (value > ((pseqdesc->blendstart[blender] + pseqdesc->blendend[blender]) / 2.0) + 180)
				value = value - 360;
			if (value < ((pseqdesc->blendstart[blender] + pseqdesc->blendend[blender]) / 2.0) - 180)
				value = value + 360;
		}
	}

	Int32 setting = (255 * (value - pseqdesc->blendstart[blender]) / (pseqdesc->blendend[blender] - pseqdesc->blendstart[blender]));
	setting = clamp(setting, 0, 255);
	m_blending[blender] = setting;

	return setting * (1.0 / 255.0) * (pseqdesc->blendend[blender] - pseqdesc->blendstart[blender]) + pseqdesc->blendstart[blender];
}

//=============================================
// @brief Sets bodygroup value
//
//=============================================
Int32 CBasicVBMRenderer::SetBodyGroup( Int32 group, Int32 value )
{
	if (!m_pStudioHeader)
		return -1;

	if (group > m_pStudioHeader->numbodyparts)
		return -1;

	const mstudiobodyparts_t *pbodypart = m_pStudioHeader->getBodyPart(group);
	Uint64 currentValue = (m_bodyValue / pbodypart->base) % pbodypart->nummodels;
	if (value >= pbodypart->nummodels)
		return currentValue;

	m_bodyValue = (m_bodyValue - (currentValue * pbodypart->base) + (value * pbodypart->base));

	return value;
}

//=============================================
// @brief Sets skin value
//
//=============================================
Int32 CBasicVBMRenderer::SetSkin( Int32 value )
{
	if (!m_pStudioHeader)
		return 0;

	if (value >= m_pStudioHeader->numskinfamilies)
		return m_skinNumber;

	m_skinNumber = value;
	return m_skinNumber;
}

//=============================================
// @brief Sets current time
//
//=============================================
void CBasicVBMRenderer::SetTime( Double time )
{
	m_time = time;
}
 
//=============================================
// @brief Sets frametime
//
//=============================================
void CBasicVBMRenderer::SetFrameTime( Double frametime )
{
	m_frameTime = frametime;
}

//=============================================
// @brief Sets the flex state pointer
//
//=============================================
void CBasicVBMRenderer::SetFlexStatePointer( flexstate_t* pFlexState )
{
	m_pFlexState = pFlexState;
}

//=============================================
// @brief Sets the studiomdl file pointer
//
//=============================================
void CBasicVBMRenderer::SetStudioHeader( studiohdr_t* pstudiohdr )
{
	m_pStudioHeader = pstudiohdr;
}

//=============================================
// @brief Sets the vbm model file header
//
//=============================================
void CBasicVBMRenderer::SetVBMHeader( vbmheader_t* pvbmheader )
{
	m_pVBMHeader = pvbmheader;
}

//=============================================
// @brief Sets the view origin
//
//=============================================
void CBasicVBMRenderer::SetViewOrigin( const Vector& viewOrigin )
{
	m_viewOrigin = viewOrigin;
}

//=============================================
// @brief Sets the view angles
//
//=============================================
void CBasicVBMRenderer::SetViewAngles( const Vector& viewAngles )
{
	m_viewAngles = viewAngles;
}

//=============================================
// @brief Sets whether we're playing a script
//
//=============================================
void CBasicVBMRenderer::SetScriptPlayback( bool isScriptPlayback )
{
	m_scriptPlayback = isScriptPlayback;
}

//=============================================
// @brief Sets the flex manager pointer
//
//=============================================
void CBasicVBMRenderer::SetFlexManager( CFlexManager* pFlexManager )
{
	m_pFlexManager = pFlexManager;
}

//=============================================
// @brief Sets the transparency value
//
//=============================================
void CBasicVBMRenderer::SetTransparency( Float transparency )
{
	m_transparency = transparency;
}

//=============================================
// @brief Sets whether mesh divisions should be shown
//
//=============================================
void CBasicVBMRenderer::SetShowMeshDivisions( bool showDivisions )
{
	m_showMeshDivisions = showDivisions;
}

//=============================================
// @brief Sets whether mesh divisions should be shown
//
//=============================================
void CBasicVBMRenderer::SetDrawWireframeOverlay( bool drawWireframeOverlay )
{
	m_wireframeOverlay = drawWireframeOverlay;
}

//=============================================
// @brief Sets whether bones should be shown
//
//=============================================
void CBasicVBMRenderer::SetShowBones( bool showBones )
{
	m_showBones = showBones;
}

//=============================================
// @brief Sets whether bones should be shown
//
//=============================================
void CBasicVBMRenderer::SetShowAttachments( bool showAttachments )
{
	m_showAttachments = showAttachments;
}

//=============================================
// @brief Sets whether bones should be shown
//
//=============================================
void CBasicVBMRenderer::SetShowHitBoxes( bool showHitBoxes )
{
	m_showHitBoxes = showHitBoxes;
}

//=============================================
// @brief Sets the light color
//
//=============================================
void CBasicVBMRenderer::SetLightColor( const Vector& lightColor )
{
	m_lightColor = lightColor;
}

//=============================================
// @brief Sets a flex value
//
//=============================================
void CBasicVBMRenderer::SetFlexValue( Uint32 index, Float value )
{
	if(!m_pFlexValues)
		return;

	if(index >= m_pVBMHeader->numflexcontrollers)
		return;

	m_pFlexValues[index] = value;
}

//=============================================
// @brief Sets the FOV value
//
//=============================================
void CBasicVBMRenderer::SetFOVValue( Float fov )
{
	m_fovValue = fov;
}

//=============================================
// @brief Sets screen size
//
//=============================================
void CBasicVBMRenderer::SetScreenSize( Float screenWidth, Float screenHeight )
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
}

//=============================================
// @brief Destroys the VBM object
//
//=============================================
void CBasicVBMRenderer::DestroyVBM( void )
{
	if(!m_pVBO)
		return;

	m_pShader->ResetShader();
	m_pShader->SetVBO(nullptr);

	delete m_pVBO;
	m_pVBO = nullptr;
}

//=============================================
// @brief Intializes the VBM object
//
//=============================================
void CBasicVBMRenderer::InitVBM ( void )
{
	if(!m_pVBMHeader)
		return;

	if(m_pVBO)
	{
		delete m_pVBO;
		m_pVBO = nullptr;
	}

	Int32 tcx, tcy;
	Int32 row_verts = VBM_FLEXTEXTURE_SIZE/3;

	m_pVBO = new CVBO(m_glExtF, true, true);
	m_pShader->SetVBO(m_pVBO);

	const vbmvertex_t *pvertexes = m_pVBMHeader->getVertexes();

	Uint32 vertexCount = m_pVBMHeader->numverts + TEMP_VERTEX_COUNT;
	vbm_glvertex_t *pvbovertexes = new vbm_glvertex_t[vertexCount];
	memset(pvbovertexes, 0, sizeof(vbm_glvertex_t)*vertexCount);

	for(Uint32 i = 0; i < m_pVBMHeader->numverts; i++)
	{
		Math::VectorCopy(pvertexes[i].origin, pvbovertexes[i].origin);
		Math::VectorCopy(pvertexes[i].normal, pvbovertexes[i].normal);
		Math::VectorCopy(pvertexes[i].tangent, pvbovertexes[i].tangent);

		pvbovertexes[i].texcoord[0] = pvertexes[i].texcoord[0];
		pvbovertexes[i].texcoord[1] = pvertexes[i].texcoord[1];

		tcy = (pvertexes[i].flexvertindex + 1) / row_verts;
		tcx = (pvertexes[i].flexvertindex + 1) % row_verts;

		pvbovertexes[i].flexcoord[0] = (Float)(tcx*3) / (Float)VBM_FLEXTEXTURE_SIZE;
		pvbovertexes[i].flexcoord[1] = (Float)(tcy*3) / (Float)VBM_FLEXTEXTURE_SIZE;

		for(Int32 j = 0; j < MAX_VBM_BONEWEIGHTS; j++)
		{
			pvbovertexes[i].boneindexes[j] = pvertexes[i].boneindexes[j];
			pvbovertexes[i].boneweights[j] = ((Float)pvertexes[i].boneweights[j])/255.0f;
		}

		VBM_NormalizeWeights(pvbovertexes[i].boneweights, MAX_VBM_BONEWEIGHTS);
	}

	m_pVBO->Append(pvbovertexes, vertexCount*sizeof(vbm_glvertex_t), m_pVBMHeader->getIndexes(), m_pVBMHeader->numindexes*sizeof(Uint32));
	delete [] pvbovertexes;

	// Set draw buffer beginning
	m_drawBufferOffset = m_pVBMHeader->numverts;

	if(m_pVBMHeader->numflexcontrollers > 0)
		m_pFlexValues = new Float[m_pVBMHeader->numflexcontrollers];
}

//=============================================
// @brief Initializes shader objects
//
//=============================================
bool CBasicVBMRenderer::InitShader( void )
{
	if(m_pShader)
	{
		delete m_pShader;
		m_pShader = nullptr;
	}

	m_pShader = new CGLSLShader(m_fileFuncs, m_glExtF, SHADER_FILE_NAME, CGLSLShader::FL_GLSL_SHADER_NONE);
	if(m_pShader->HasError())
		return false;

	m_attribs.a_origin = m_pShader->InitAttribute("in_position", 3, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, origin));
	m_attribs.a_tangent = m_pShader->InitAttribute("in_tangent", 3, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, tangent));
	m_attribs.a_normal = m_pShader->InitAttribute("in_normal", 3, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, normal));
	m_attribs.a_texcoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, texcoord));
	m_attribs.a_boneindexes = m_pShader->InitAttribute("in_boneindexes", MAX_VBM_BONEWEIGHTS, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, boneindexes));
	m_attribs.a_boneweights = m_pShader->InitAttribute("in_boneweights", MAX_VBM_BONEWEIGHTS, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, boneweights));
	m_attribs.a_flexcoord = m_pShader->InitAttribute("in_flexcoord", 2, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, flexcoord));

	if(!R_CheckShaderVertexAttribute(m_attribs.a_origin, "in_position", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderVertexAttribute(m_attribs.a_tangent, "in_tangent", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderVertexAttribute(m_attribs.a_normal, "in_normal", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord, "in_texcoord", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderVertexAttribute(m_attribs.a_boneindexes, "in_boneindexes", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderVertexAttribute(m_attribs.a_boneweights, "in_boneweights", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderVertexAttribute(m_attribs.a_flexcoord, "in_flexcoord", m_pShader, m_pfnErrorPopup))
		return false;

	for(Uint32 i = 0; i < MAX_SHADER_BONES; i++)
	{
		CString uniformname;
		uniformname << "bones[" << (Int32)i*3 << "]";
		m_attribs.boneindexes[i] = m_pShader->InitUniform(uniformname.c_str(), CGLSLShader::UNIFORM_NOSYNC);
		if(!R_CheckShaderUniform(m_attribs.boneindexes[i], uniformname.c_str(), m_pShader, m_pfnErrorPopup))
			return false;
	}

	m_attribs.u_flextexture = m_pShader->InitUniform("flextexture", CGLSLShader::UNIFORM_INT1);
	m_attribs.u_flextexturesize = m_pShader->InitUniform("flextexture_size", CGLSLShader::UNIFORM_FLOAT1);
	m_attribs.u_phong_exponent = m_pShader->InitUniform("phong_exponent", CGLSLShader::UNIFORM_FLOAT1);
	m_attribs.u_specularfactor = m_pShader->InitUniform("specfactor", CGLSLShader::UNIFORM_FLOAT1);
	m_attribs.u_color = m_pShader->InitUniform("color", CGLSLShader::UNIFORM_FLOAT4);
	m_attribs.u_texture0 = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);
	m_attribs.u_rectangle = m_pShader->InitUniform("rectangle", CGLSLShader::UNIFORM_INT1);
	m_attribs.u_spectexture = m_pShader->InitUniform("spectexture", CGLSLShader::UNIFORM_INT1);
	m_attribs.u_lumtexture = m_pShader->InitUniform("lumtexture", CGLSLShader::UNIFORM_INT1);
	m_attribs.u_normalmap = m_pShader->InitUniform("normalmap", CGLSLShader::UNIFORM_INT1);
	m_attribs.u_normalmatrix = m_pShader->InitUniform("normalmatrix", CGLSLShader::UNIFORM_MATRIX4);
	m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
	m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
	m_attribs.u_scope_scale = m_pShader->InitUniform("scope_scale", CGLSLShader::UNIFORM_NOSYNC);
	m_attribs.u_scope_scrsize = m_pShader->InitUniform("scope_scrsize", CGLSLShader::UNIFORM_NOSYNC);
	m_attribs.u_sky_ambient = m_pShader->InitUniform("skylight_ambient", CGLSLShader::UNIFORM_FLOAT3);
	m_attribs.u_sky_diffuse = m_pShader->InitUniform("skylight_diffuse", CGLSLShader::UNIFORM_FLOAT3);
	m_attribs.u_sky_dir = m_pShader->InitUniform("skylight_dir", CGLSLShader::UNIFORM_FLOAT3);
	m_attribs.u_vorigin = m_pShader->InitUniform("v_origin", CGLSLShader::UNIFORM_FLOAT3);
	m_attribs.u_vright = m_pShader->InitUniform("v_right", CGLSLShader::UNIFORM_FLOAT3);
	m_attribs.u_scroll = m_pShader->InitUniform("scroll", CGLSLShader::UNIFORM_FLOAT2);

	if(!R_CheckShaderUniform(m_attribs.u_flextexture, "flextexture", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_flextexturesize, "flextexture_size", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_phong_exponent, "phong_exponent", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_specularfactor, "specfactor", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_color, "color", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_texture0, "texture0", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_rectangle, "rectangle", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_spectexture, "spectexture", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_lumtexture, "lumtexture", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_normalmap, "normalmap", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_normalmatrix, "normalmatrix", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_scope_scale, "scope_scale", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_scope_scrsize, "scope_scrsize", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_sky_ambient, "skylight_ambient", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_sky_diffuse, "skylight_diffuse", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_sky_dir, "skylight_dir", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_vorigin, "v_origin", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_vright, "v_right", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_scroll, "scroll", m_pShader, m_pfnErrorPopup))
		return false;

	m_attribs.d_chrome = m_pShader->GetDeterminatorIndex("chrome");
	m_attribs.d_shadertype = m_pShader->GetDeterminatorIndex("shadertype");
	m_attribs.d_alphatest = m_pShader->GetDeterminatorIndex("alphatest");
	m_attribs.d_flexes = m_pShader->GetDeterminatorIndex("flex");
	m_attribs.d_specular = m_pShader->GetDeterminatorIndex("specular");
	m_attribs.d_luminance = m_pShader->GetDeterminatorIndex("luminance");
	m_attribs.d_bumpmapping = m_pShader->GetDeterminatorIndex("bumpmapping");

	if(!R_CheckShaderDeterminator(m_attribs.d_chrome, "chrome", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderDeterminator(m_attribs.d_shadertype, "shadertype", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderDeterminator(m_attribs.d_alphatest, "alphatest", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderDeterminator(m_attribs.d_flexes, "flex", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderDeterminator(m_attribs.d_specular, "specular", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderDeterminator(m_attribs.d_luminance, "luminance", m_pShader, m_pfnErrorPopup)
		|| !R_CheckShaderDeterminator(m_attribs.d_bumpmapping, "bumpmapping", m_pShader, m_pfnErrorPopup))
		return false;

	return true;
}

//=============================================
// @brief Initializes the renderer
//
//=============================================
bool CBasicVBMRenderer::Init( void )
{
	if(!InitShader())
		return false;

	m_pGlintTexture = CTextureManager::GetInstance()->LoadTexture(EYEGLINT_TEXTURE_PATH, RS_WINDOW_LEVEL);
	if(!m_pGlintTexture)
	{
		m_pfnErrorPopup("Failed to load '%s'.", EYEGLINT_TEXTURE_PATH);
		return false;
	}

	m_pFlexTexture = CTextureManager::GetInstance()->GenTextureIndex(RS_WINDOW_LEVEL);

	glBindTexture(GL_TEXTURE_2D, m_pFlexTexture->gl_index);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, VBM_FLEXTEXTURE_SIZE, VBM_FLEXTEXTURE_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);

	return true;
}

//=============================================
// @brief Shuts down the renderer
//
//=============================================
void CBasicVBMRenderer::Shutdown( void )
{
	if(m_pShader)
	{
		delete m_pShader;
		m_pShader = nullptr;
	}

	if(m_pVBO)
	{
		delete m_pVBO;
		m_pVBO = nullptr;
	}
}

//=============================================
// @brief Returns the ideal LOD for the lod type specified
//
//=============================================
const vbmsubmodel_t* CBasicVBMRenderer::GetIdealLOD( const vbmsubmodel_t* psubmodel, vbmlod_type_t type )
{
	if(!psubmodel->numlods)
		return psubmodel;

	Vector vtmp;
	Math::VectorSubtract(m_origin, m_viewOrigin, vtmp);
	
	Float lastdistance = 0;
	Float distance = vtmp.Length();

	const vbmsubmodel_t* preturn = psubmodel;
	for(int i = 0; i < psubmodel->numlods; i++)
	{
		const vbmlod_t* plod = psubmodel->getLOD(m_pVBMHeader, i);
		if(plod->type != type)
			continue;

		// Shadow LODs are simple
		if(type == VBM_LOD_SHADOW)
		{
			preturn = plod->getSubmodel(m_pVBMHeader);
			break;
		}
		else if(type == VBM_LOD_DISTANCE)
		{
			// Get the most ideal lod for this distance
			if(distance >= plod->distance && plod->distance > lastdistance)
			{
				preturn = plod->getSubmodel(m_pVBMHeader);
				lastdistance = plod->distance;
			}
		}
	}

	return preturn;
}

//=============================================
// @brief Advances the frame
//
//=============================================
void CBasicVBMRenderer::AdvanceFrame( Float dt )
{
	if (!m_pStudioHeader)
		return;

	if (m_sequence < 0 || m_sequence >= m_pStudioHeader->numseq)
		return;

	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_sequence);

	if (dt > 0.1)
		dt = 0.1f;

	m_currentFrame += dt * pseqdesc->fps;

	if (pseqdesc->numframes <= 1)
		m_currentFrame = 0;
	else
		m_currentFrame -= (Int32)(m_currentFrame / (pseqdesc->numframes - 1)) * (pseqdesc->numframes - 1);
}

//=============================================
// @brief Sets the current frame
//
//=============================================
Float CBasicVBMRenderer::SetFrame( Float nFrame )
{
	if (nFrame == -1)
		return m_currentFrame;

	if (!m_pStudioHeader)
		return 0;

	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_sequence);

	m_currentFrame = nFrame;
	m_prevFrameFrame = nFrame;

	if (pseqdesc->numframes <= 1)
		m_currentFrame = 0;
	else
		m_currentFrame -= (Int32)(m_currentFrame / (pseqdesc->numframes - 1)) * (pseqdesc->numframes - 1);

	return m_currentFrame;
}

//=============================================
// @brief Sets up bones
//
//=============================================
void CBasicVBMRenderer::SetUpBones ( void )
{
	if (m_sequence >= m_pStudioHeader->numseq) 
		m_sequence = 0;

	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_sequence);
	const mstudioanim_t* panim = VBM_GetAnimation( m_pStudioHeader, pseqdesc );

	VBM_CalculateRotations( m_pStudioHeader, 0, 0, 0, m_bonePositions1, m_boneQuaternions1, pseqdesc, panim, m_currentFrame, m_boneController, m_boneController, m_mouthOpen);

	if (pseqdesc->numblends > 1)
	{
		panim += m_pStudioHeader->numbones;
		VBM_CalculateRotations( m_pStudioHeader, 0, 0, 0, m_bonePositions2, m_boneQuaternions2, pseqdesc, panim, m_currentFrame, m_boneController, m_boneController, m_mouthOpen);
		Float s = m_blending[0] / 255.0;

		VBM_InterpolateBones(m_pStudioHeader, m_boneQuaternions1, m_bonePositions1, m_boneQuaternions2, m_bonePositions2, s, m_boneQuaternions1, m_bonePositions1);

		if (pseqdesc->numblends == 4)
		{
			panim += m_pStudioHeader->numbones;
			VBM_CalculateRotations( m_pStudioHeader, 0, 0, 0, m_bonePositions3, m_boneQuaternions3, pseqdesc, panim, m_currentFrame, m_boneController, m_boneController, m_mouthOpen);

			panim += m_pStudioHeader->numbones;
			VBM_CalculateRotations( m_pStudioHeader, 0, 0, 0, m_bonePositions4, m_boneQuaternions4, pseqdesc, panim, m_currentFrame, m_boneController, m_boneController, m_mouthOpen);

			s = m_blending[0] / 255.0;
			VBM_InterpolateBones(m_pStudioHeader, m_boneQuaternions3, m_bonePositions3, m_boneQuaternions4, m_bonePositions4, s, m_boneQuaternions3, m_bonePositions3);

			s = m_blending[1] / 255.0;
			VBM_InterpolateBones(m_pStudioHeader, m_boneQuaternions1, m_bonePositions1, m_boneQuaternions3, m_bonePositions3, s, m_boneQuaternions1, m_bonePositions1);
		}
	}

	for(Int32 i = 0; i < m_pStudioHeader->numbones; i++) 
	{
		const mstudiobone_t* pbone = m_pStudioHeader->getBone(i);
		Math::QuaternionMatrix( m_boneQuaternions1[i], m_boneMatrix );

		m_boneMatrix[0][3] = m_bonePositions1[i][0];
		m_boneMatrix[1][3] = m_bonePositions1[i][1];
		m_boneMatrix[2][3] = m_bonePositions1[i][2];

		if (pbone->parent == -1)
			memcpy(m_boneTransform[i], m_boneMatrix, sizeof(Float)*12);
		else
			Math::ConcatTransforms (m_boneTransform[pbone->parent], m_boneMatrix, m_boneTransform[i]);
	}

	if(m_pVBMHeader)
	{
		// Set up weight transforms
		vbmboneinfo_t* pboneinfos = (vbmboneinfo_t*)((byte *)m_pVBMHeader + m_pVBMHeader->boneinfooffset);
		for (Int32 i = 0; i < m_pVBMHeader->numboneinfo; i++)
			Math::ConcatTransforms (m_boneTransform[i], pboneinfos[i].bindtransform, m_weightBoneTransform[i]);
	}
}

//=============================================
// @brief Sets up the submodel to render
//
//=============================================
void CBasicVBMRenderer::SetupModel ( Int32 bodypart )
{
	Int32 _bodypart = bodypart;
	if(bodypart > m_pVBMHeader->numbodyparts)
		_bodypart = 0;

	const vbmbodypart_t *pbodypart = m_pVBMHeader->getBodyPart(_bodypart);

	Uint64 index = m_bodyValue / pbodypart->base;
	index = index % pbodypart->numsubmodels;

	m_pVBMSubmodel = pbodypart->getSubmodel(m_pVBMHeader, index);
}

//=============================================
// @brief Draws the bounding box
//
//=============================================
void CBasicVBMRenderer::DrawBox( const Vector& bbmin, const Vector& bbmax )
{
	Vector bboxpoints[8];

	bboxpoints[0][0] = bbmin[0];
	bboxpoints[0][1] = bbmax[1];
	bboxpoints[0][2] = bbmin[2];

	bboxpoints[1][0] = bbmin[0];
	bboxpoints[1][1] = bbmin[1];
	bboxpoints[1][2] = bbmin[2];

	bboxpoints[2][0] = bbmax[0];
	bboxpoints[2][1] = bbmax[1];
	bboxpoints[2][2] = bbmin[2];

	bboxpoints[3][0] = bbmax[0];
	bboxpoints[3][1] = bbmin[1];
	bboxpoints[3][2] = bbmin[2];

	bboxpoints[4][0] = bbmax[0];
	bboxpoints[4][1] = bbmax[1];
	bboxpoints[4][2] = bbmax[2];

	bboxpoints[5][0] = bbmax[0];
	bboxpoints[5][1] = bbmin[1];
	bboxpoints[5][2] = bbmax[2];

	bboxpoints[6][0] = bbmin[0];
	bboxpoints[6][1] = bbmax[1];
	bboxpoints[6][2] = bbmax[2];

	bboxpoints[7][0] = bbmin[0];
	bboxpoints[7][1] = bbmin[1];
	bboxpoints[7][2] = bbmax[2];

	Vector triverts[3];

	m_numTempVertexes = 0;
	for(Uint32 i = 0; i < 3; i++)
	{
		// Remember triverts
		triverts[i] = bboxpoints[i&7];

		// Add to the draw list
		m_tempVertexes[m_numTempVertexes].origin = bboxpoints[i];
		m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
		m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;

		for(Uint32 j = 1; j < MAX_VBM_BONEWEIGHTS; j++)
			m_tempVertexes[m_numTempVertexes].boneweights[j] = 0;

		m_numTempVertexes++;
	}

	for(Uint32 i = 3; i < 10; i++)
	{
		triverts[0] = triverts[1];
		triverts[1] = triverts[2];
		triverts[2] = bboxpoints[i&7];

		for(Uint32 j = 0; j < 3; j++)
		{
			m_tempVertexes[m_numTempVertexes].origin = triverts[j];
			m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
			m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;

			for(Uint32 k = 1; k < MAX_VBM_BONEWEIGHTS; k++)
				m_tempVertexes[m_numTempVertexes].boneweights[k] = 0;

			m_numTempVertexes++;
		}
	}

	m_tempVertexes[m_numTempVertexes].origin = bboxpoints[6];
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
	for(Uint32 i = 1; i < MAX_VBM_BONEWEIGHTS; i++)
		m_tempVertexes[m_numTempVertexes].boneweights[i] = 0;
	m_numTempVertexes++;

	m_tempVertexes[m_numTempVertexes].origin = bboxpoints[0];
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
	for(Uint32 i = 1; i < MAX_VBM_BONEWEIGHTS; i++)
		m_tempVertexes[m_numTempVertexes].boneweights[i] = 0;
	m_numTempVertexes++;

	m_tempVertexes[m_numTempVertexes].origin = bboxpoints[4];
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
	for(Uint32 i = 1; i < MAX_VBM_BONEWEIGHTS; i++)
		m_tempVertexes[m_numTempVertexes].boneweights[i] = 0;
	m_numTempVertexes++;

	m_tempVertexes[m_numTempVertexes].origin = bboxpoints[0];
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
	for(Uint32 i = 1; i < MAX_VBM_BONEWEIGHTS; i++)
		m_tempVertexes[m_numTempVertexes].boneweights[i] = 0;
	m_numTempVertexes++;

	m_tempVertexes[m_numTempVertexes].origin = bboxpoints[4];
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
	for(Uint32 i = 1; i < MAX_VBM_BONEWEIGHTS; i++)
		m_tempVertexes[m_numTempVertexes].boneweights[i] = 0;
	m_numTempVertexes++;

	m_tempVertexes[m_numTempVertexes].origin = bboxpoints[2];
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
	for(Uint32 i = 1; i < MAX_VBM_BONEWEIGHTS; i++)
		m_tempVertexes[m_numTempVertexes].boneweights[i] = 0;
	m_numTempVertexes++;

	m_tempVertexes[m_numTempVertexes].origin = bboxpoints[1];
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
	for(Uint32 i = 1; i < MAX_VBM_BONEWEIGHTS; i++)
		m_tempVertexes[m_numTempVertexes].boneweights[i] = 0;
	m_numTempVertexes++;

	m_tempVertexes[m_numTempVertexes].origin = bboxpoints[7];
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
	for(Uint32 i = 1; i < MAX_VBM_BONEWEIGHTS; i++)
		m_tempVertexes[m_numTempVertexes].boneweights[i] = 0;
	m_numTempVertexes++;

	m_tempVertexes[m_numTempVertexes].origin = bboxpoints[3];
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
	for(Uint32 i = 1; i < MAX_VBM_BONEWEIGHTS; i++)
		m_tempVertexes[m_numTempVertexes].boneweights[i] = 0;
	m_numTempVertexes++;

	m_tempVertexes[m_numTempVertexes].origin = bboxpoints[7];
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
	for(Uint32 i = 1; i < MAX_VBM_BONEWEIGHTS; i++)
		m_tempVertexes[m_numTempVertexes].boneweights[i] = 0;
	m_numTempVertexes++;

	m_tempVertexes[m_numTempVertexes].origin = bboxpoints[3];
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
	for(Uint32 i = 1; i < MAX_VBM_BONEWEIGHTS; i++)
		m_tempVertexes[m_numTempVertexes].boneweights[i] = 0;
	m_numTempVertexes++;

	m_tempVertexes[m_numTempVertexes].origin = bboxpoints[5];
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
	for(Uint32 i = 1; i < MAX_VBM_BONEWEIGHTS; i++)
		m_tempVertexes[m_numTempVertexes].boneweights[i] = 0;
	m_numTempVertexes++;

	// Draw the planes
	m_pVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferOffset, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);
	glDrawArrays(GL_TRIANGLES, m_drawBufferOffset, m_numTempVertexes);
}

//=============================================
//
//
//=============================================
bool CBasicVBMRenderer::DrawBones( void )
{
	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_flexes, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, true))
		return false;

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glLineWidth(2.0);
	glPointSize(5.0);

	// Draw the points
	for(Int32 i = 0; i < m_pStudioHeader->numbones; i++)
	{
		const mstudiobone_t* pbone = m_pStudioHeader->getBone(i);
		if(pbone->parent == -1)
			continue;

		// Set bone
		m_pShader->SetUniform4fv(m_attribs.boneindexes[0], (Float *)m_boneTransform[pbone->parent], 3);

		// Set color
		m_pShader->SetUniform4f(m_attribs.u_color, BONE_LINES_COLOR[0], BONE_LINES_COLOR[1], BONE_LINES_COLOR[2], 1.0);

		// Begin compiling the vertex data
		m_numTempVertexes = 0;

		Vector worldOrigin(m_boneTransform[i][0][3], m_boneTransform[i][1][3], m_boneTransform[i][2][3]);
		Vector parentOrigin(m_boneTransform[pbone->parent][0][3], m_boneTransform[pbone->parent][1][3], m_boneTransform[pbone->parent][2][3]);

		Vector temp;
		Math::VectorSubtract(worldOrigin, parentOrigin, temp);
		Math::VectorInverseRotate(temp, m_boneTransform[pbone->parent], worldOrigin);

		Math::VectorClear(m_tempVertexes[m_numTempVertexes].origin);
		m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
		m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
		m_tempVertexes[m_numTempVertexes].normal.Clear();
		for(Uint32 j = 1; j < MAX_VBM_BONEWEIGHTS; j++)
			m_tempVertexes[m_numTempVertexes].boneweights[j] = 0;
		m_numTempVertexes++;

		Math::VectorCopy(worldOrigin, m_tempVertexes[m_numTempVertexes].origin);
		m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
		m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
		m_tempVertexes[m_numTempVertexes].normal.Clear();
		for(Uint32 j = 1; j < MAX_VBM_BONEWEIGHTS; j++)
			m_tempVertexes[m_numTempVertexes].boneweights[j] = 0;
		m_numTempVertexes++;

		m_pVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferOffset, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);
		glDrawArrays(GL_LINES, m_drawBufferOffset, m_numTempVertexes);

		// Draw point
		m_numTempVertexes = 0;
		Math::VectorCopy(worldOrigin, m_tempVertexes[m_numTempVertexes].origin);
		m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
		m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
		m_tempVertexes[m_numTempVertexes].normal.Clear();
		for(Uint32 j = 1; j < MAX_VBM_BONEWEIGHTS; j++)
			m_tempVertexes[m_numTempVertexes].boneweights[j] = 0;
		m_numTempVertexes++;

		m_pShader->SetUniform4f(m_attribs.u_color, BONE_ORIGIN_COLOR[0], BONE_ORIGIN_COLOR[1], BONE_ORIGIN_COLOR[2], 1.0);

		m_pVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferOffset, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);
		glDrawArrays(GL_POINTS, m_drawBufferOffset, m_numTempVertexes);
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glLineWidth(1.0);
	glPointSize(1.0);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	return true;
}

//=============================================
// @brief Draws hitboxes
//
//=============================================
bool CBasicVBMRenderer::DrawHitBoxes( void )
{
	Vector bboxpoints[8];

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_flexes, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, true))
		return false;

	glDisable (GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for(Int32 i = 0; i < m_pStudioHeader->numhitboxes; i++)
	{
		const mstudiobbox_t *pbbox = m_pStudioHeader->getHitBox(i);

		// Set bone transform
		m_pShader->SetUniform4fv(m_attribs.boneindexes[0], (Float *)m_boneTransform[pbbox->bone], 3);

		// Set color
		const Vector& pcolor = RANDOM_COLOR_ARRAY[pbbox->group%NB_RANDOM_COLORS];
		m_pShader->SetUniform4f(m_attribs.u_color, pcolor[0], pcolor[1], pcolor[2], 0.5);

		DrawBox(pbbox->bbmin, pbbox->bbmax);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	return true;
}

//=============================================
// @brief Draws attachemnts
//
//=============================================
bool CBasicVBMRenderer::DrawAttachments( void )
{
	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_flexes, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, true))
		return false;

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glLineWidth(2.0);

	// Draw the points
	for(Int32 i = 0; i < m_pStudioHeader->numattachments; i++)
	{
		const mstudioattachment_t* pattachment = m_pStudioHeader->getAttachment(i);
		if(!pattachment)
			break;

		const mstudiobone_t* pbone = m_pStudioHeader->getBone(pattachment->bone);

		// Set bone
		Float matrix[3][4];
		matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.0;
		matrix[0][1] = matrix[0][2] = matrix[1][0] = matrix[1][2] = matrix[2][0] = matrix[2][1] = 0.0;
		matrix[0][3] = matrix[1][3] = matrix[2][3] = 0.0;

		m_pShader->SetUniform4fv(m_attribs.boneindexes[0], (Float *)matrix, 3);

		// Set color
		m_pShader->SetUniform4f(m_attribs.u_color, ATTACHMENT_LINES_COLOR[0], ATTACHMENT_LINES_COLOR[1], ATTACHMENT_LINES_COLOR[2], 1.0);

		// Begin compiling the vertex data
		m_numTempVertexes = 0;

		Vector worldOrigin(m_boneTransform[pattachment->bone][0][3], m_boneTransform[pattachment->bone][1][3], m_boneTransform[pattachment->bone][2][3]);
		Vector attachmentOrigin;
		Math::VectorTransform(pattachment->org, m_boneTransform[pattachment->bone], attachmentOrigin);

		Math::VectorCopy(worldOrigin, m_tempVertexes[m_numTempVertexes].origin);
		m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
		m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
		m_tempVertexes[m_numTempVertexes].normal.Clear();
		for(Uint32 j = 1; j < MAX_VBM_BONEWEIGHTS; j++)
			m_tempVertexes[m_numTempVertexes].boneweights[j] = 0;
		m_numTempVertexes++;

		Math::VectorCopy(attachmentOrigin, m_tempVertexes[m_numTempVertexes].origin);
		m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
		m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
		m_tempVertexes[m_numTempVertexes].normal.Clear();
		for(Uint32 j = 1; j < MAX_VBM_BONEWEIGHTS; j++)
			m_tempVertexes[m_numTempVertexes].boneweights[j] = 0;
		m_numTempVertexes++;

		m_pVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferOffset, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);

		// Draw the line
		glDrawArrays(GL_LINES, m_drawBufferOffset, m_numTempVertexes);

		// Draw the point
		glPointSize(5);
		m_pShader->SetUniform4f(m_attribs.u_color, ATTACHMENT_ORIGIN_COLOR[0], ATTACHMENT_ORIGIN_COLOR[1], ATTACHMENT_ORIGIN_COLOR[2], 1.0);

		glDrawArrays(GL_POINTS, m_drawBufferOffset+1, 1);
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glLineWidth(1.0);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	glPointSize(1);
	return true;
}

//=============================================
// @brief Draws bounding box
//
//=============================================
bool CBasicVBMRenderer::DrawBoundingBox( void )
{
	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_flexes, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, true))
		return false;

	glDisable (GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Float matrix[3][4];
	matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.0;
	matrix[0][1] = matrix[0][2] = matrix[1][0] = matrix[1][2] = matrix[2][0] = matrix[2][1] = 0.0;
	matrix[0][3] = matrix[1][3] = matrix[2][3] = 0.0;

	// Set bone transform to identity
	m_pShader->SetUniform4fv(m_attribs.boneindexes[0], (Float *)matrix, 3);

	Vector mins, maxs;
	GetSequenceBBox(mins, maxs);

	const Vector& pcolor = RANDOM_COLOR_ARRAY[0];
	m_pShader->SetUniform4f(m_attribs.u_color, pcolor[0], pcolor[1], pcolor[2], 0.5);

	DrawBox(mins, maxs);

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	return true;
}

//=============================================
// @brief Calculates flexes
//
//=============================================
void CBasicVBMRenderer::CalculateFlexes ( void )
{
	static vec4_t flexTexels[VBM_FLEXTEXTURE_SIZE*VBM_FLEXTEXTURE_SIZE];

	if( !m_pVBMSubmodel || m_pVBMSubmodel->flexinfoindex == -1 || !m_pFlexState || !m_pFlexManager )
		return;

	if(m_scriptPlayback)
		m_pFlexManager->UpdateValues( m_time, 0, 0, m_pFlexState, false );

	if(m_pfnSetFlexValues)
		m_pfnSetFlexValues();

	const vbmflexinfo_t* pflexinfo = m_pVBMHeader->getFlexInfo(m_pVBMSubmodel->flexinfoindex);
	const vbmflexvertex_t* pflexverts = pflexinfo->getFlexVertexes(m_pVBMHeader);
	const vbmflexvertinfo_t* pflexvertinfos = pflexinfo->getFlexVertexInfos(m_pVBMHeader);
	const vbmflexcontroller_t* pflexcontrollers = m_pVBMHeader->getFlexControllers();
	const byte* pflexcontrollerindexes = pflexinfo->getFlexControllerIndexes(m_pVBMHeader);

	Int32 height = 0;

	// Reset all the values first
	for(Int32 i = 0; i < pflexinfo->numflexvertinfo; i++)
	{
		vec4_t* originoffset = flexTexels + pflexverts[pflexvertinfos[i].vertinfoindexes[0]].offset;
		vec4_t* normoffset = flexTexels + pflexverts[pflexvertinfos[i].vertinfoindexes[0]].offset + 1;

		if( pflexverts[pflexvertinfos[i].vertinfoindexes[0]].offset / VBM_FLEXTEXTURE_SIZE > height )
			height = (pflexverts[pflexvertinfos[i].vertinfoindexes[0]].offset / VBM_FLEXTEXTURE_SIZE);

		for(Int32 j = 0; j < 3; j++)
		{
			(*originoffset)[j] = 0;
			(*normoffset)[j] = 0;
		}

		for(Int32 j = 1; j < pflexinfo->numflexes; j++)
		{
			if(pflexvertinfos[i].vertinfoindexes[j] == -1)
				continue;

			Int32 controller_idx = pflexcontrollerindexes[j];
			Int32 script_idx = m_pFlexState->indexmap[controller_idx];
			if(script_idx == -1)
				continue;

			Float value = m_pFlexState->values[script_idx];
			value *= (pflexcontrollers[controller_idx].maxvalue - pflexcontrollers[controller_idx].minvalue);
			value += pflexcontrollers[controller_idx].minvalue;

			for(int k = 0; k < 3; k++)
			{
				(*originoffset)[k] += pflexverts[pflexvertinfos[i].vertinfoindexes[j]].originoffset[k]*value;
				(*normoffset)[k] = pflexverts[pflexvertinfos[i].vertinfoindexes[j]].normaloffset[k]*value;
			}
		}
	}

	// Now upload the texture
	m_glExtF.glActiveTexture(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D, m_pFlexTexture->gl_index);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VBM_FLEXTEXTURE_SIZE, height + 1, GL_RGBA, GL_FLOAT, flexTexels);
}

//=============================================
// @brief Renders the model
//
//=============================================
bool CBasicVBMRenderer::DrawModel( CMatrix& modelview, CMatrix& projection )
{
	if(!m_pVBO || !m_pShader)
		return true;

	m_triangleCounter = 0;

	if(!m_pStudioHeader || !m_pVBMHeader || !m_pVBMHeader->numbodyparts)
		return true;

	switch(m_renderMode)
	{
	case RENDER_WIREFRAME:
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
		}
		break;
	case RENDER_SMOOTHSHADED:
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
		}
		break;
	case RENDER_TEXTURED:
	default:
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
		}
		break;
	}

	m_pVBO->Bind();
	if(!m_pShader->EnableShader())
	{
		m_pVBO->UnBind();
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_boneindexes);
	m_pShader->EnableAttribute(m_attribs.a_boneweights);
	m_pShader->EnableAttribute(m_attribs.a_normal);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);
	m_pShader->EnableAttribute(m_attribs.a_tangent);
	m_pShader->EnableAttribute(m_attribs.a_flexcoord);

	modelview.PushMatrix();
	modelview.Translate(m_origin[0], m_origin[1], m_origin[2]);
    modelview.Rotate(m_angles[1],  0, 0, 1);
    modelview.Rotate(m_angles[0],  0, 1, 0);
    modelview.Rotate(m_angles[2],  1, 0, 0);

	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, modelview.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_normalmatrix, modelview.GetInverse());

	modelview.PopMatrix();

	m_pShader->SetUniform1i(m_attribs.u_texture0, 0);
	m_pShader->SetUniform1i(m_attribs.u_flextexture, 1);
	m_pShader->SetUniform1i(m_attribs.u_flextexturesize, VBM_FLEXTEXTURE_SIZE);

	// Set for chrome
	Vector right;
	Math::AngleVectors(m_viewAngles, nullptr, &right);

	m_pShader->SetUniform3f(m_attribs.u_vorigin, m_viewOrigin[0], m_viewOrigin[1], m_viewOrigin[2]);
	m_pShader->SetUniform3f(m_attribs.u_vright, right[0], right[1], right[2]);

	Vector vtransformed;
	CMatrix pmatrix(modelview.GetInverse());
	Math::MatMult(pmatrix.Transpose(), m_lightDirection, &vtransformed);

	SetupLighting();

	Float lightStrength = (Float)m_ambientlight/255.0f;
	Vector ambientLightColor = m_lightColor * lightStrength;
	Vector directLightColor = m_lightColor * (1.0 - lightStrength);

	m_pShader->SetUniform3f(m_attribs.u_sky_dir, vtransformed[0], vtransformed[1], vtransformed[2]);
	m_pShader->SetUniform3f(m_attribs.u_sky_ambient, ambientLightColor[0], ambientLightColor[1], ambientLightColor[2]);
	m_pShader->SetUniform3f(m_attribs.u_sky_diffuse, directLightColor[0], directLightColor[1], directLightColor[2]);

	if(!m_pShader->SetDeterminator(m_attribs.d_chrome, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false))
		return false;

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, m_transparency);

	SetUpBones();

	// Play any animation events
	CalculateAttachments();
	PlayEvents();

	// Set this after running events
	m_prevFrameSequence = m_sequence;
	m_prevFrameFrame = m_currentFrame;

	if(!m_showMeshDivisions)
	{
		Int32 i = 0;
		for(; i < m_pVBMHeader->numtextures; i++)
		{
			const vbmtexture_t* ptexture = m_pVBMHeader->getTexture(i);
			en_material_t* pmaterial = CTextureManager::GetInstance()->FindMaterialScriptByIndex(ptexture->index);
			if(!pmaterial)
				continue;

			if(pmaterial->flags & TX_FL_SCOPE)
				break;
		}

		if(i != m_pVBMHeader->numtextures)
		{
			glEnable(GL_TEXTURE_RECTANGLE);
			if(!m_pScreenTexture || m_screenTextureWidth != m_screenWidth || m_screenTextureHeight != m_screenHeight)
			{
				if(!m_pScreenTexture)
					m_pScreenTexture = CTextureManager::GetInstance()->GenTextureIndex(RS_WINDOW_LEVEL);

				glBindTexture(GL_TEXTURE_RECTANGLE, m_pScreenTexture->gl_index);
				glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, m_screenWidth, m_screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

				m_screenTextureWidth = m_screenWidth;
				m_screenTextureHeight = m_screenHeight;
			}
			else

			// Grab the screen texture
			glBindTexture(GL_TEXTURE_RECTANGLE, m_pScreenTexture->gl_index);
			glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, m_screenTextureWidth, m_screenTextureHeight, 0);
		
			// Unbind texture
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
			glDisable(GL_TEXTURE_RECTANGLE);
		}

		if(m_transparency < 1)
		{
			glEnable(GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		for(Int32 j = 0; j < m_pVBMHeader->numbodyparts; j++)
		{
			SetupModel(j);

			if(m_pVBMSubmodel->numlods > 0)
				m_pVBMSubmodel = GetIdealLOD(m_pVBMSubmodel, VBM_LOD_DISTANCE);

			if(!DrawSubmodel())
			{
				m_pShader->DisableShader();
				m_pVBO->UnBind();
				return false;
			}
		}

		if(m_transparency < 1.0)
			glDisable(GL_BLEND);
	}
	else
	{
		glLineWidth(3.0);
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

		for(Int32 i = 0; i < m_pVBMHeader->numbodyparts; i++)
		{
			SetupModel(i);

			if(m_pVBMSubmodel->numlods > 0)
				m_pVBMSubmodel = GetIdealLOD(m_pVBMSubmodel, VBM_LOD_DISTANCE);

			if(!DrawSubmodelSolid(true))
			{
				m_pShader->DisableShader();
				m_pVBO->UnBind();
				return false;
			}
		}

		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glLineWidth(1.0);
	}

	if( m_wireframeOverlay && m_renderMode != RENDER_WIREFRAME)
	{
		glLineWidth(2.0);
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

		for(Int32 i = 0; i < m_pVBMHeader->numbodyparts; i++)
		{
			SetupModel(i);

			if(m_pVBMSubmodel->numlods > 0)
				m_pVBMSubmodel = GetIdealLOD(m_pVBMSubmodel, VBM_LOD_DISTANCE);

			if(!DrawSubmodelSolid(true))
			{
				m_pShader->DisableShader();
				m_pVBO->UnBind();
				return false;
			}
		}

		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glLineWidth(1.0);
	}

	// draw bones
	if(m_showBones)
	{
		if(!DrawBones())
		{
			m_pShader->DisableShader();
			m_pVBO->UnBind();
			return false;
		}
	}

	if(m_showAttachments)
	{
		if(!DrawAttachments())
		{
			m_pShader->DisableShader();
			m_pVBO->UnBind();
			return false;
		}
	}

	if(m_showHitBoxes)
	{
		if(!DrawHitBoxes())
		{
			m_pShader->DisableShader();
			m_pVBO->UnBind();
			return false;
		}
	}
		
	m_pShader->DisableShader();
	m_pVBO->UnBind();

	return true;
}

//=============================================
// @brief Sets up lighting
//
//=============================================
void CBasicVBMRenderer::SetupLighting ( void )
{
	m_ambientlight = 100;
	m_lightDirection = Vector(0, 0, -1.0);
}

//=============================================
// @brief Returns mouth value
//
//=============================================
void CBasicVBMRenderer::SetMouthOpen( Float value )
{
	m_mouthOpen = value;
}

//=============================================
// @brief Sets mouth value
//
//=============================================
Float CBasicVBMRenderer::SetMouthController( Float value )
{
	if (!m_pStudioHeader)
		return value;

	// find first controller that matches the index
	const mstudiobonecontroller_t *pbonecontroller = nullptr;
	for (Int32 i = 0; i < m_pStudioHeader->numbonecontrollers; i++)
	{
		const mstudiobonecontroller_t *pcontroller = m_pStudioHeader->getBoneController(i);
		if (pcontroller->index == MOUTH_CONTROLLER_INDEX)
		{
			pbonecontroller = pcontroller;
			break;
		}
	}

	if (!pbonecontroller)
		return value;

	// wrap 0..360 if it's a rotational controller
	if (pbonecontroller->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pbonecontroller->end < pbonecontroller->start)
			value = -value;

		// does the controller not wrap?
		if (pbonecontroller->start + 359.0 >= pbonecontroller->end)
		{
			if (value > ((pbonecontroller->start + pbonecontroller->end) / 2.0) + 180)
				value = value - 360;
			if (value < ((pbonecontroller->start + pbonecontroller->end) / 2.0) - 180)
				value = value + 360;
		}
		else
		{
			if (value > 360)
				value = value - (value / 360.0) * 360.0;
			else if (value < 0)
				value = value + ((value / -360.0) + 1) * 360.0;
		}
	}

	Float setting = (64 * (value - pbonecontroller->start) / (pbonecontroller->end - pbonecontroller->start));
	setting = clamp(setting, 0, 64);
	m_mouth = setting;

	return setting * (1.0 / 64.0) * (pbonecontroller->end - pbonecontroller->start) + pbonecontroller->start;
}

//=============================================
// @brief Draws meshes for a submodel
//
//=============================================
bool CBasicVBMRenderer::DrawSubmodel ( void )
{
	if( m_pVBMSubmodel->flexinfoindex != -1 )
	{
		CalculateFlexes();
		Viewer_Bind2DTexture(GL_TEXTURE1_ARB, m_pFlexTexture->gl_index);
		m_pShader->SetDeterminator(m_attribs.d_flexes, true, false);
	}
	else
	{
		m_pShader->SetDeterminator(m_attribs.d_flexes, false, false);
	}

	m_pShader->SetUniform4f(m_attribs.u_color, GL_ONE, GL_ONE, GL_ONE, m_transparency);

	const Int16 *pskinref = m_pVBMHeader->getSkinFamilies();
	if(m_skinNumber != 0 && m_skinNumber < m_pVBMHeader->numskinfamilies)
		pskinref += (m_skinNumber * m_pVBMHeader->numskinref);

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	for (Int32 i = 0; i < m_pVBMSubmodel->nummeshes; i++) 
	{
		const vbmmesh_t *pmesh = m_pVBMSubmodel->getMesh(m_pVBMHeader, i);
		const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

		en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
		if(!pmaterial)
			continue;

		if(pmaterial->flags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE))
			continue;

		if(!DrawMesh(pmaterial, pmesh, false))
			return false;

		m_triangleCounter += pmesh->num_indexes/3;
	}

	for (Int32 i = 0; i < m_pVBMSubmodel->nummeshes; i++) 
	{
		const vbmmesh_t *pmesh = m_pVBMSubmodel->getMesh(m_pVBMHeader, i);
		const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

		en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
		if(!pmaterial)
			continue;

		if(!(pmaterial->flags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE|TX_FL_EYEGLINT)))
			continue;

		if(m_transparency >= 1.0)
			glEnable(GL_BLEND);

		glDepthMask(GL_FALSE);

		if(pmaterial->flags & (TX_FL_ADDITIVE|TX_FL_EYEGLINT))
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			m_pShader->SetUniform4f(m_attribs.u_color, GL_ONE, GL_ONE, GL_ONE, m_transparency);
		}
		else if(pmaterial->flags & TX_FL_ALPHABLEND)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			m_pShader->SetUniform4f(m_attribs.u_color, GL_ONE, GL_ONE, GL_ONE, m_transparency*pmaterial->alpha);
		}

		if(!DrawMesh(pmaterial, pmesh, true))
		{
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
			return false;
		}

		m_triangleCounter += pmesh->num_indexes/3;

		glDepthMask(GL_TRUE);
		if(m_transparency >= 1.0)
			glDisable(GL_BLEND);
	}

	// Reset this
	m_pShader->SetUniform4f(m_attribs.u_color, GL_ONE, GL_ONE, GL_ONE, m_transparency);

	if( m_pVBMSubmodel->flexinfoindex != -1 )
	{
		m_glExtF.glActiveTexture(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_glExtF.glActiveTexture(GL_TEXTURE0_ARB);

		m_pShader->SetDeterminator(m_attribs.d_flexes, false, false);
	}

	return true;
}

//=============================================
// @brief Draws a single mesh
//
//=============================================
bool CBasicVBMRenderer::DrawMesh( en_material_t *pmaterial, const vbmmesh_t *pmesh, bool drawBlended )
{
	// Set the determinator states
	if(!m_pShader->SetDeterminator(m_attribs.d_chrome, ((pmaterial->flags & (TX_FL_CHROME) || pmaterial->flags & (TX_FL_EYEGLINT) && drawBlended)) ? TRUE : FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_alphatest, ((pmaterial->flags & TX_FL_ALPHATEST) && !(pmaterial->flags & (TX_FL_SCOPE|TX_FL_CHROME|TX_FL_EYEGLINT))) ? true : false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, (pmaterial->ptextures[MT_TX_SPECULAR]) && !(pmaterial->flags & TX_FL_FULLBRIGHT), false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, (pmaterial->ptextures[MT_TX_LUMINANCE]) && !(pmaterial->flags & TX_FL_FULLBRIGHT), false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, (pmaterial->ptextures[MT_TX_NORMALMAP]) && !(pmaterial->flags & TX_FL_FULLBRIGHT), false))
	return false;

	bool result = false;
	if(m_renderMode == RENDER_TEXTURED)
	{
		if(pmaterial->flags & TX_FL_SCOPE)
			result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_scope, false);
		else if(pmaterial->flags & TX_FL_FULLBRIGHT)
			result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texonly, false);
		else
			result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texture, false);
	}
	else
	{
		result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_notexture, false);
	}
	// Verify the settings
	if(!result || !m_pShader->VerifyDeterminators())
		return false;

	if(pmaterial->flags & TX_FL_SCOPE)
	{
		m_glExtF.glActiveTexture(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_RECTANGLE);
		glBindTexture(GL_TEXTURE_RECTANGLE, m_pScreenTexture->gl_index);

		Float flscale = pmaterial->alpha/Viewer_GetFOV();
		m_pShader->SetUniform1f(m_attribs.u_scope_scale, flscale);
		m_pShader->SetUniform2f(m_attribs.u_scope_scrsize, m_screenWidth, m_screenHeight);
		m_pShader->SetUniform1i(m_attribs.u_rectangle, 1);
	}

	if(pmaterial->ptextures[MT_TX_SPECULAR])
	{
		m_pShader->SetUniform1f(m_attribs.u_phong_exponent, pmaterial->phong_exp*PHONG_EXPONENT_VALUE);
		m_pShader->SetUniform1f(m_attribs.u_specularfactor, pmaterial->spec_factor);

		m_pShader->SetUniform1i(m_attribs.u_spectexture, 2);
		Viewer_Bind2DTexture(GL_TEXTURE2, pmaterial->ptextures[MT_TX_SPECULAR]->palloc->gl_index);
	}

	if(pmaterial->ptextures[MT_TX_LUMINANCE])
	{
		m_pShader->SetUniform1i(m_attribs.u_lumtexture, 3);
		Viewer_Bind2DTexture(GL_TEXTURE3, pmaterial->ptextures[MT_TX_LUMINANCE]->palloc->gl_index);
	}

	if(pmaterial->ptextures[MT_TX_NORMALMAP])
	{
		m_pShader->SetUniform1i(m_attribs.u_normalmap, 4);
		Viewer_Bind2DTexture(GL_TEXTURE4, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);
	}

	if(pmaterial->scrollu || pmaterial->scrollv)
	{
		Float scrollu = pmaterial->scrollu ? (m_time * pmaterial->scrollu) : 0;
		Float scrollv = pmaterial->scrollv ? (m_time * pmaterial->scrollv) : 0;

		m_pShader->SetUniform2f(m_attribs.u_scroll, scrollu, scrollv);
	}
	else
	{
		// No scrolling
		m_pShader->SetUniform2f(m_attribs.u_scroll, 0, 0);
	}

	if(pmesh->numbones)
	{
		const byte *pboneindexes = pmesh->getBones(m_pVBMHeader);
		for(Int32 i = 0; i < pmesh->numbones; i++)
			m_pShader->SetUniform4fv(m_attribs.boneindexes[i], (Float *)m_weightBoneTransform[pboneindexes[i]], 3);
	}
	
	if(drawBlended && pmaterial->flags & TX_FL_EYEGLINT)
		Viewer_Bind2DTexture(GL_TEXTURE0, m_pGlintTexture->palloc->gl_index);
	else
		Viewer_Bind2DTexture(GL_TEXTURE0, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);

	if(pmaterial->flags & TX_FL_NO_CULLING)
		glDisable(GL_CULL_FACE);

	glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));

	if(pmaterial->flags & TX_FL_NO_CULLING)
		glEnable(GL_CULL_FACE);
	
	if(pmaterial->flags & TX_FL_SCOPE)
	{
		m_glExtF.glActiveTexture(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_RECTANGLE);

		m_glExtF.glActiveTexture(GL_TEXTURE0_ARB);
	}

	return true;
}

//=============================================
// @brief Draws solid meshes for a submodel
//
//=============================================
bool CBasicVBMRenderer::DrawSubmodelSolid( bool setFlexes )
{
	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, true))
		return false;

	if( m_pVBMSubmodel->flexinfoindex != -1 )
	{
		CalculateFlexes();
		Viewer_Bind2DTexture(GL_TEXTURE1_ARB, m_pFlexTexture->gl_index);
		m_pShader->SetDeterminator(m_attribs.d_flexes, true, false);
	}
	else
	{
		m_pShader->SetDeterminator(m_attribs.d_flexes, false, false);
	}

	const Int16 *pskinref = m_pVBMHeader->getSkinFamilies();
	if(m_skinNumber != 0 && m_skinNumber < m_pVBMHeader->numskinfamilies)
		pskinref += (m_skinNumber * m_pVBMHeader->numskinref);

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	for (Int32 i = 0; i < m_pVBMSubmodel->nummeshes; i++) 
	{
		const vbmmesh_t *pmesh = m_pVBMSubmodel->getMesh(m_pVBMHeader, i);
		const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

		en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
		if(!pmaterial)
			continue;

		if(pmaterial->flags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE))
			continue;

		const Vector& color = RANDOM_COLOR_ARRAY[i % NB_RANDOM_COLORS];
		m_pShader->SetUniform4f(m_attribs.u_color, color.x, color.y, color.z, m_transparency);

		if(pmesh->numbones)
		{
			const byte *pboneindexes = pmesh->getBones(m_pVBMHeader);
			for(Int32 j = 0; j < pmesh->numbones; j++)
				m_pShader->SetUniform4fv(m_attribs.boneindexes[j], (Float *)m_weightBoneTransform[pboneindexes[j]], 3);
		}

		glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));

		m_triangleCounter += pmesh->num_indexes/3;
	}

	if( m_pVBMSubmodel->flexinfoindex != -1 )
	{
		m_glExtF.glActiveTexture(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_glExtF.glActiveTexture(GL_TEXTURE0_ARB);
	}

	glEnable(GL_CULL_FACE);
	return true;
}

//=============================================
// @brief Returns the shader's error message
//
// @return Error string pointer
//=============================================
const Char* CBasicVBMRenderer::GetShaderError( void ) const
{
	if(!m_pShader)
		return "";
	else
		return m_pShader->GetError();
}

//========================================
// StudioModel :: Chrome
//
//========================================
void CBasicVBMRenderer::Chrome( const Vector& viewOrigin, const Vector& viewRight, const byte* pboneindexes, const vbmvertex_t* pvertex, Float *pchromecoords )
{
	Vector normal, origin, tmp;
	for(int i = 0; i < MAX_VBM_BONEWEIGHTS; i++)
	{
		if(!pvertex->boneweights[i])
			continue;

		byte boneindex = pboneindexes[(byte)(pvertex->boneindexes[i]/3)];
		Math::VectorTransform(pvertex->origin, m_weightBoneTransform[boneindex], tmp);
		Math::VectorMA(origin, ((float)pvertex->boneweights[i])/255.0f, tmp, origin);

		Math::VectorRotate(pvertex->normal, m_weightBoneTransform[boneindex], tmp);
		Math::VectorMA(normal, ((float)pvertex->boneweights[i])/255.0f, tmp, normal);
	}

	Math::VectorScale( viewOrigin, -1, tmp );
	tmp[0] += origin[0];
	tmp[1] += origin[1];
	tmp[2] += origin[2];

	Vector chromeupvec;
	Vector chromerightvec;
	Math::VectorNormalize( tmp );
	Math::CrossProduct( tmp, viewRight, chromeupvec );
	Math::VectorNormalize( chromeupvec );
	Math::CrossProduct( tmp, chromeupvec, chromerightvec );
	Math::VectorNormalize( chromerightvec );
	
	// calc s coord
	Float n = Math::DotProduct( normal, chromerightvec );
	pchromecoords[0] = (n + 0.5);

	// calc t coord
	n = Math::DotProduct( normal, chromeupvec );
	pchromecoords[1] = (n + 0.5);
}

//=============================================
// @brief Calculates attachment positions
//
//=============================================
void CBasicVBMRenderer::CalculateAttachments( void )
{
	if(!m_pfnVBMEvent)
		return;

	if(m_attachmentsArray.size() != m_pStudioHeader->numattachments)
		m_attachmentsArray.resize(m_pStudioHeader->numattachments);

	for(Int32 i = 0; i < m_pStudioHeader->numattachments; i++)
	{
		const mstudioattachment_t* pattachment = m_pStudioHeader->getAttachment((Uint32)i);
		Math::VectorTransform(pattachment->org, m_boneTransform[pattachment->bone], m_attachmentsArray[i]);
	}
}

//=============================================
// @brief Plays back animation events
//
//=============================================
void CBasicVBMRenderer::PlayEvents( void )
{
	if(!m_pfnVBMEvent)
		return;

	// Get current sequence info
	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_sequence);

	// Nothing to do here
	if(!pseqdesc->numevents)
		return;

	// Fixes first-frame event bug
	if(!m_currentFrame) 
		m_lastEventFrame = -0.01f;

	if(m_currentFrame == m_lastEventFrame) 
		return;

	if (m_currentFrame < m_lastEventFrame)
	{
		if(m_prevFrameSequence == m_sequence 
			&& m_prevFrameFrame > m_currentFrame)
		{
			for (Int32 i = 0; i < pseqdesc->numevents; i++)
			{
				const mstudioevent_t *pevent = pseqdesc->getEvent(m_pStudioHeader, i);
				if(pevent->frame <= m_lastEventFrame)
					continue;
				
				m_pfnVBMEvent(pevent, &m_attachmentsArray);
			}

			// Necessary to get the next loop working
			m_lastEventFrame = -0.01;
		}
		else
			m_lastEventFrame = -0.01;
	}

	for (Int32 i = 0; i < pseqdesc->numevents; i++)
	{
		const mstudioevent_t *pevent = pseqdesc->getEvent(m_pStudioHeader, i);
		if((pevent->frame > m_lastEventFrame && pevent->frame <= m_currentFrame))
			m_pfnVBMEvent(pevent, &m_attachmentsArray);
	}

	m_lastEventFrame = m_currentFrame;
}

//=============================================
// @brief Tells if the shader has an error
//
// @return TRUE if shader has an error, FALSE otherwise
//=============================================
bool CBasicVBMRenderer::HasError( void ) const
{
	if(!m_pShader)
		return false;
	else
		return m_pShader->HasError();
}

//=============================================
// @brief Sets the "SetFlexValues" function pointer
//
//=============================================
void CBasicVBMRenderer::SetSetFlexValuesFunctionPointer( pfnSetFlexValues_t pfnSetFlexValues )
{
	m_pfnSetFlexValues = pfnSetFlexValues;
}

//=============================================
// @brief Sets the VBMEvent function pointer
//
//=============================================
void CBasicVBMRenderer::SetVBMEventFunctionPointer( pfnVBMEvent_t pfnVBMEvent )
{
	m_pfnVBMEvent = pfnVBMEvent;
}

//=============================================
// @brief Sets the render mode
//
//=============================================
void CBasicVBMRenderer::SetRenderMode( rendermode_t rendermode )
{
	m_renderMode = rendermode;
}

//=============================================
// @brief Creates an instance of this class
//
// @return Created instance
//=============================================
CBasicVBMRenderer* CBasicVBMRenderer::CreateInstance( CGLExtF& glExtF, const file_interface_t& fileFuncs, pfnErrorPopup_t pfnErrorPopupFn )
{
	if(!g_pInstance)
		g_pInstance = new CBasicVBMRenderer(glExtF, fileFuncs, pfnErrorPopupFn);

	return g_pInstance;
}

//=============================================
// @brief Returns the current instance of this class
//
// @return Current instance, or nullptr
//=============================================
CBasicVBMRenderer* CBasicVBMRenderer::GetInstance( void )
{
	return g_pInstance;
}

//=============================================
// @brief Deletes the current instance of this class
//
//=============================================
void CBasicVBMRenderer::DeleteInstance( void )
{
	if(!g_pInstance)
		return;

	delete g_pInstance;
	g_pInstance = nullptr;
}