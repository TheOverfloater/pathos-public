//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           CGLWindow.cpp
// last modified:  May 04 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.2
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//

#include <mx/mx.h>
#include <mx/gl.h>
#include <mx/mximage.h>
#include <mx/mxtga.h>
#include <GL/glu.h>

#include "includes.h"

#include "glwindow.h"
#include "mdlviewer.h"
#include "controlpanel.h"
#include "viewerstate.h"
#include "config.h"

#include "r_basicdraw.h"
#include "r_vbmbasic.h"

// Height fraction compared to window size
const Float CGLWindow::FLEX_VISUALIZER_HEIGHT_FRACTION = 0.1;
// Inset fraction compared to window size
const Float CGLWindow::FLEX_VISUALIZER_INSET_FRACTION = 0.1;
// Scale height compared to visualizer height
const Float CGLWindow::FLEX_VISUALIZER_SCALE_HEIGHT = 0.8;
// Time window on clicking for deletion/binding
const Float CGLWindow::FLEX_MOUSECLICK_TIME_WINDOW = 0.25;
// Reference texture width
const Uint32 CGLWindow::REFERENCE_TEXTURE_WIDTH = 256;
// Reference texture height
const Uint32 CGLWindow::REFERENCE_TEXTURE_HEIGHT = 256;
// Ground alpha value
const Float CGLWindow::GROUND_ALPHA_VALUE = 0.7f;
// Skybox size - should be lower than FAR_Z_DISTANCE
const Float CGLWindow::SKYBOX_SIZE = 512;
// Number of curve segments
const Uint32 CGLWindow::CURVE_SEGMENTS = 10;
// Skybox texture postfixes
const Char* CGLWindow::SKY_POSTFIXES[NUM_SKYBOX_TEXTURES] = {"lf", "bk", "rt", "ft", "dn", "up"};

// Current instance of this class
CGLWindow* CGLWindow::g_pInstance = nullptr;

//=============================================
// @brief Constructor
//
//=============================================
CGLWindow::CGLWindow( mxWindow *parent, Int32 x, Int32 y, Int32 w, Int32 h, const Char *label, Int32 style ): 
	mxGlWindow( parent, x, y, w, h, label, style ),
	m_oldRotateX(0),
	m_oldRotateY(0),
	m_oldTranslateZ(50),
	m_oldTranslateX(0),
	m_oldTranslateY(0),
	m_oldX(0),
	m_oldY(0),
	m_clickInVisualizer(false),
	m_lastActionClickTime(0),
	m_lastButtonClicked(0),
	m_pGroundTexture(nullptr)
{
	for(Uint32 i = 0; i < NUM_SKYBOX_TEXTURES; i++)
		m_pSkyboxTextures[i] = nullptr;

	if (!parent)
		setVisible(true);
	else
		mx::setIdleWindow(this);
}

//=============================================
// @brief Destructor
//
//=============================================
CGLWindow::~CGLWindow ()
{
	// mx deletes these windows before we 
	// get a chance to do it via DeleteInstance()
	g_pInstance = nullptr;

	Shutdown();
}

//=============================================
// @brief Initializes the class
//
//=============================================
void CGLWindow::Init( void )
{
}

//=============================================
// @brief Shuts down the class
//
//=============================================
void CGLWindow::Shutdown( void )
{
	mx::setIdleWindow(nullptr);
}

//=============================================
// @brief Tells if the cursor is inside the flex visualizer bounds
//
//=============================================
bool CGLWindow::IsInVisualizerBounds( Float x, Float y )
{
	// Use proper XY coords
	Float real_y = h2() - y;

	// Determine dimensions of the visualizer
	Float visualizerHeight = FLEX_VISUALIZER_HEIGHT_FRACTION*h2();

	Float scaleYBegin = visualizerHeight*FLEX_VISUALIZER_INSET_FRACTION;
	Float scaleHeight = visualizerHeight*FLEX_VISUALIZER_SCALE_HEIGHT;

	Float scaleXBegin = w2()*FLEX_VISUALIZER_INSET_FRACTION;
	Float scaleWidth = w2() - scaleXBegin*2;

	// Determine bounds
	Float scaleXEnd = scaleXBegin + scaleWidth;
	Float scaleYEnd = scaleYBegin + scaleHeight;

	if(x > scaleXBegin && x < scaleXEnd)
	{
		if( real_y > scaleYBegin && real_y < scaleYEnd )
			return true;
	}

	return false;
}

//=============================================
// @brief Handles an mx event
//
//=============================================
Int32 CGLWindow::handleEvent( mxEvent *pEvent )
{
	switch (pEvent->event)
	{
	case mxEvent::Idle:
		{
			Update();

			if (!vs.pause)
				redraw();

			return 1;
		}
		break;
	case mxEvent::MouseUp:
		{
			// Reset
			m_clickInVisualizer = false;
			return 1;
		}
		break;
	case mxEvent::MouseDown:
		{
			if(vs.flexscripting && vs.scripttimelength > 0
				&& IsInVisualizerBounds( pEvent->x, pEvent->y ))
			{
				if( pEvent->buttons & mxEvent::MouseLeftButton )
				{
					// Select position
					Float scaleXBegin = w2()*FLEX_VISUALIZER_INSET_FRACTION;
					Float scaleWidth = w2() - scaleXBegin*2;

					Float fractPosition = (pEvent->x - scaleXBegin)/scaleWidth;
					CControlPanel::GetInstance()->SetTimePosition(fractPosition, true);
				}
				else if( pEvent->buttons & (mxEvent::MouseMiddleButton|mxEvent::MouseRightButton) )
				{
					// True if the button was double-clicked
					bool shouldPerformAction = false;
					Float buttonInterval = Viewer_FloatTime() - m_lastActionClickTime;
					if( buttonInterval <= FLEX_MOUSECLICK_TIME_WINDOW
						&& pEvent->buttons == m_lastButtonClicked )
						shouldPerformAction = true;

					if( pEvent->buttons & mxEvent::MouseRightButton )
					{
						// Select position on X
						Float visualizerHeight = FLEX_VISUALIZER_HEIGHT_FRACTION*h2();
						Float scaleXBegin = w2()*FLEX_VISUALIZER_INSET_FRACTION;
						Float scaleWidth = w2() - scaleXBegin*2;
						Float fractXPosition = (pEvent->x - scaleXBegin)/scaleWidth;
					
						CControlPanel* pControlPanel = CControlPanel::GetInstance();
						pControlPanel->SetTimePosition(fractXPosition, true);
						if(shouldPerformAction)
						{
							// Use proper XY coords
							Float real_y = h2() - pEvent->y;

							Float scaleYBegin = visualizerHeight*FLEX_VISUALIZER_INSET_FRACTION;
							Float scaleHeight = visualizerHeight*FLEX_VISUALIZER_SCALE_HEIGHT;
							Float scaleYEnd = scaleYBegin + scaleHeight;
							Float fractYPosition = (real_y - scaleYBegin)/scaleHeight;

							pControlPanel->BindFlex(fractXPosition, fractYPosition);
							m_lastActionClickTime = 0;
							m_lastButtonClicked = 0;
						}
					}
					else if( pEvent->buttons & mxEvent::MouseMiddleButton )
					{
						// Select position
						Float scaleXBegin = w2()*FLEX_VISUALIZER_INSET_FRACTION;
						Float scaleWidth = w2() - scaleXBegin*2;
						Float fractPosition = (pEvent->x - scaleXBegin)/scaleWidth;
					
						CControlPanel* pControlPanel = CControlPanel::GetInstance();
						pControlPanel->SetTimePosition(fractPosition, true);

						if(shouldPerformAction)
						{
							pControlPanel->DeleteBind(fractPosition);
							m_lastActionClickTime = 0;
							m_lastButtonClicked = 0;
						}
					}

					// Record the last time we clicked either of these
					m_lastActionClickTime = Viewer_FloatTime();
					m_lastButtonClicked = pEvent->buttons;
				}

				m_clickInVisualizer = true;
				return 1;
			}
			else if (!vs.showtexture)
			{
				m_oldRotateX = vs.v_rotation[0];
				m_oldRotateY = vs.v_rotation[1];
				m_oldTranslateX = vs.v_translation[0];
				m_oldTranslateY = vs.v_translation[1];
				m_oldTranslateZ = vs.v_translation[2];
			}
			else
			{
				m_oldTranslateX = vs.t_translate[0];
				m_oldTranslateY = vs.t_translate[1];
			}

			m_oldX = pEvent->x;
			m_oldY = pEvent->y;
			vs.pause = false;

			return 1;
		}
		break;
	case mxEvent::MouseDrag:
	{
		if(m_clickInVisualizer)
		{
			// Determine dimensions of the visualizer
			Float visualizerHeight = FLEX_VISUALIZER_HEIGHT_FRACTION*h2();
			Float scaleXBegin = w2()*FLEX_VISUALIZER_INSET_FRACTION;
			Float scaleWidth = w2() - scaleXBegin*2;

			Float fractPosition = (pEvent->x - scaleXBegin)/scaleWidth;
			if(fractPosition < 0)
				fractPosition = 0;
			else if(fractPosition > 1)
				fractPosition = 1;

			CControlPanel::GetInstance()->SetTimePosition(fractPosition, true);
			return 1;
		}
		else
		{
			if (pEvent->buttons & mxEvent::MouseLeftButton)
			{
				if (vs.showtexture)
				{
					if(vs.pvbmheader)
					{
						Float w = ((Float) REFERENCE_TEXTURE_WIDTH * vs.texturescale)*4;
						Float h = ((Float) REFERENCE_TEXTURE_HEIGHT * vs.texturescale)*4;

						vs.t_translate[0] = m_oldTranslateX - (Float) (pEvent->x - m_oldX);
						vs.t_translate[0] = clamp(vs.t_translate[0], -w, w);

						vs.t_translate[1] = m_oldTranslateY + (Float) (pEvent->y - m_oldY);
						vs.t_translate[1] = clamp(vs.t_translate[1], -h, h);
					}
				}
				else
				{
					if (pEvent->modifiers & mxEvent::KeyShift)
					{
						vs.v_translation[0] = m_oldTranslateX - (Float) (pEvent->x - m_oldX);
						vs.v_translation[1] = m_oldTranslateY + (Float) (pEvent->y - m_oldY);
					}
					else
					{
						vs.v_rotation[0] = m_oldRotateX + (Float) (pEvent->y - m_oldY);
						vs.v_rotation[1] = m_oldRotateY + (Float) (pEvent->x - m_oldX);
					}
				}
			}
			else if (pEvent->buttons & mxEvent::MouseRightButton)
			{
				vs.v_translation[2] = m_oldTranslateZ + (Float) (pEvent->y - m_oldY);
			}
		}
		redraw();

		return 1;
	}
	break;
	case mxEvent::KeyDown:
		{
			switch (pEvent->key)
			{
			case 32:
				{
					CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
					if(pVBMRenderer)
					{
						Int32 iSeq = pVBMRenderer->GetSequence();
						if (iSeq == pVBMRenderer->SetSequence(iSeq + 1))
							pVBMRenderer->SetSequence (0);
					}
				}
				break;
			case 27:
				{
					mx::quit ();
				}
				break;
			case 'g':
				{
					vs.showground = !vs.showground;
					if (!vs.showground)
						vs.mirror = false;
				}
				break;
			case 'h':
				{
					vs.showhitboxes = !vs.showhitboxes;
				}
				break;
			case 'o':
				{
					vs.showbones = !vs.showbones;
				}
				break;
			case '5':
				{
					vs.transparency -= 0.05f;
					if (vs.transparency < 0.0f)
						vs.transparency = 0.0f;
				}
				break;
			case '6':
				{
					vs.transparency += 0.05f;
					if (vs.transparency > 1.0f)
						vs.transparency = 1.0f;
				}
				break;
			case 'b':
				{
					vs.showskybox = !vs.showskybox;
				}
				break;
			case 's':
				{
					vs.usestencil = !vs.usestencil;
				}
				break;
			case 'm':
				{
					vs.mirror = !vs.mirror;
					if (vs.mirror)
						vs.showground = true;
				}
				break;
			case '1':
			case '2':
			case '3':
				{
					Int32 renderModeIndex = pEvent->key - '1';
					mv_rendermodes_t renderMode;
					switch(renderModeIndex)
					{
					case 0:
						renderMode = RM_WIREFRAME;
						break;
					case 1:
						renderMode = RM_SMOOTHSHADED;
						break;
					case 2:
					default:
						renderMode = RM_TEXTURED;
						break;
					}

					vs.rendermode = renderMode;
				}
				break;
			case '-':
				{
					vs.speedscale -= 0.1f;
					if (vs.speedscale < 0.0f)
						vs.speedscale = 0.0f;
				}
				break;
			case '+':
				{
					vs.speedscale += 0.1f;
					if (vs.speedscale > 5.0f)
						vs.speedscale = 5.0f;
				}
				break;
			}
		}
		break;
	case mxEvent::MouseMove:
		break;
	default:
		break;
	}

	return 1;
}

//=============================================
// @brief Draws the floor
//
//=============================================
void CGLWindow::DrawFloor ( CBasicDraw* pBasicDraw )
{
	pBasicDraw->SetProjection(vs.projection.GetMatrix());
	pBasicDraw->SetModelview(vs.modelview.GetMatrix());

	pBasicDraw->Begin(GL_TRIANGLES);
	// Triangle 1
	pBasicDraw->TexCoord2f(0.0f + vs.grounddist, 1.0f);
	pBasicDraw->Vertex3f(-vs.groundsize, -vs.groundsize, 0.0f);

	pBasicDraw->TexCoord2f(1.0f + vs.grounddist, 0.0f);
	pBasicDraw->Vertex3f(vs.groundsize, vs.groundsize, 0.0f);

	pBasicDraw->TexCoord2f(0.0f + vs.grounddist, 0.0f);
	pBasicDraw->Vertex3f(-vs.groundsize, vs.groundsize, 0.0f);

	// Triangle 2
	pBasicDraw->TexCoord2f(1.0f + vs.grounddist, 0.0f);
	pBasicDraw->Vertex3f(vs.groundsize, vs.groundsize, 0.0f);

	pBasicDraw->TexCoord2f(0.0f + vs.grounddist, 1.0f);
	pBasicDraw->Vertex3f(-vs.groundsize, -vs.groundsize, 0.0f);

	pBasicDraw->TexCoord2f(1.0f + vs.grounddist, 1.0f);
	pBasicDraw->Vertex3f(vs.groundsize, -vs.groundsize, 0.0f);

	pBasicDraw->End();
}

//=============================================
// @brief Draws UVs
//
//=============================================
bool CGLWindow::DrawUVCoords( CBasicDraw* pBasicDraw, Int32 xOrigin, Int32 yOrigin, Int32 height, Int32 width )
{
	if(!vs.pvbmheader)
		return true;

	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();

	vbmtexture_t* ptexture = vs.pvbmheader->getTexture(vs.texture);
	en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
	if(!pmaterial)
		return true;

	if(!pBasicDraw->DisableTexture())
		return false;

	glLineWidth(1.0f);
	pBasicDraw->Color4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	glEnable(GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Vector origin, forward, right, up;
	Math::AngleVectors(vs.v_rotation, &forward, &right, &up);
	Math::VectorCopy(vs.v_translation, origin);

	if(pmaterial->flags & TX_FL_CHROME)
		pVBMRenderer->SetUpBones();

	const Uint32* pindexes = vs.pvbmheader->getIndexes();
	const vbmvertex_t* pvertexes = vs.pvbmheader->getVertexes();

	pBasicDraw->Begin(GL_LINES);

	for(Int32 i = 0; i < vs.pvbmheader->numbodyparts; i++)
	{
		const vbmbodypart_t* pvbmbodypart = vs.pvbmheader->getBodyPart(i);

		for(Int32 j = 0; j < pvbmbodypart->numsubmodels; j++)
		{
			const vbmsubmodel_t* psubmodel = pvbmbodypart->getSubmodel(vs.pvbmheader, j);

			const byte* pboneindexes = nullptr;
			for(Int32 k = 0; k < psubmodel->nummeshes; k++)
			{
				const vbmmesh_t* pmesh = psubmodel->getMesh(vs.pvbmheader, k);

				// Remember the last bone mesh
				if(pmesh->boneoffset)
					pboneindexes = pmesh->getBones(vs.pvbmheader);

				if(pmesh->skinref % vs.pvbmheader->numskinref != vs.texture)
					continue;
				
				for(Int32 l = 0; l < pmesh->num_indexes; l += 3)
				{
					for(Int32 m = 0; m < 3; m++)
					{
						Int32 triidx1 = pmesh->start_index + l + m;
						Int32 triidx2 = pmesh->start_index + l + m + 1;
						if(triidx2 > pmesh->start_index + l + 2)
							triidx2 = pmesh->start_index + l;

						const vbmvertex_t* pvert1 = &pvertexes[pindexes[triidx1]];
						const vbmvertex_t* pvert2 = &pvertexes[pindexes[triidx2]];

						Float tc1[2];
						Float tc2[2];
						if(!(pmaterial->flags & TX_FL_CHROME))
						{
							tc1[0] = pvert1->texcoord[0] * width;
							tc1[1] = pvert1->texcoord[1] * height;
							tc2[0] = pvert2->texcoord[0] * width;
							tc2[1] = pvert2->texcoord[1] * height;
						}
						else
						{
							pVBMRenderer->Chrome(vs.v_translation, right, pboneindexes, pvert1, tc1); 
							pVBMRenderer->Chrome(vs.v_translation, right, pboneindexes, pvert2, tc2);

							tc1[0] *= width; 
							tc2[0] *= width;
							tc1[1] *= height; 
							tc2[1] *= height;
						}

						
						pBasicDraw->Vertex3f(xOrigin + tc1[0], yOrigin + tc1[1], 0);
						pBasicDraw->Vertex3f(xOrigin + tc2[0], yOrigin + tc2[1], 0);
						
					}
				}
			}
		}
	}

	pBasicDraw->End();

	if(!pBasicDraw->EnableTexture())
		return false;

	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);

	return true;
}

//=============================================
// @brief Draws the texture
//
//=============================================
bool CGLWindow::DrawTexture( CBasicDraw* pBasicDraw )
{
	if(!vs.pvbmheader)
		return true;

	vs.projection.PushMatrix();
	vs.projection.LoadIdentity();
	vs.projection.Ortho(0.0f, w2(), h2(), 0.0f, 1.0f, -1.0f);

	Float width = (Float)REFERENCE_TEXTURE_WIDTH*2 * vs.texturescale;
	Float height = (Float)REFERENCE_TEXTURE_HEIGHT*2 * vs.texturescale;

	vs.modelview.PushMatrix();
	vs.modelview.LoadIdentity();

	vs.modelview.Translate(-vs.t_translate[0], vs.t_translate[1], 0);

	pBasicDraw->SetProjection(vs.projection.GetMatrix());
	pBasicDraw->SetModelview(vs.modelview.GetMatrix());

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	Float x = ((Float) w2 () - width) / 2;
	Float y = ((Float) h2 () - height) / 2;

	if(!pBasicDraw->DisableTexture())
		return false;

	pBasicDraw->Color4f(GL_ONE, GL_ZERO, GL_ZERO, GL_ONE);

	Float x1 = x - width - 2;
	Float y1 = y - height - 2;
	Float x2 = x  + width*2 + 2;
	Float y2 = y + height*2 + 2;

	pBasicDraw->Begin(GL_TRIANGLES);
	pBasicDraw->Vertex3f(x1, y1, 0);
	pBasicDraw->Vertex3f(x2, y1, 0);
	pBasicDraw->Vertex3f(x2, y2, 0);
	pBasicDraw->Vertex3f(x1, y1, 0);
	pBasicDraw->Vertex3f(x2, y2, 0);
	pBasicDraw->Vertex3f(x1, y2, 0);
	pBasicDraw->End();

	if(!pBasicDraw->EnableTexture())
		return false;

	pBasicDraw->Color4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	vbmtexture_t* ptexture = vs.pvbmheader->getTexture(vs.texture);
	en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
	if(!pmaterial)
		return false;

	Viewer_Bind2DTexture(GL_TEXTURE0_ARB, pmaterial->getdiffuse()->palloc->gl_index);

	Int32 positions[][2] = { 
		{ -1, 1 }, { 0, 1 }, { 1, 1 }, // Top rows
		{ -1, 0 }, { 0, 0 }, { 1, 0 }, // Middle rows
		{ -1, -1 }, { 0, -1 }, { 1, -1 } }; // Bottom rows

	for(Int32 i = 0; i < 9; i++)
	{
		pBasicDraw->Begin(GL_TRIANGLES);
		
		// Triangle 1
		pBasicDraw->TexCoord2f(0, 0);
		pBasicDraw->Vertex3f(x + positions[i][0]*width, y + positions[i][1]*height, 0);

		pBasicDraw->TexCoord2f(1, 0);
		pBasicDraw->Vertex3f(x + positions[i][0]*width + width, y + positions[i][1]*height, 0);

		pBasicDraw->TexCoord2f(0, 1);
		pBasicDraw->Vertex3f(x + positions[i][0]*width, y + positions[i][1]*height + height, 0);

		// Triangle 2
		pBasicDraw->TexCoord2f(1, 0);
		pBasicDraw->Vertex3f(x + positions[i][0]*width + width, y + positions[i][1]*height, 0);

		pBasicDraw->TexCoord2f(0, 1);
		pBasicDraw->Vertex3f(x + positions[i][0]*width, y + positions[i][1]*height + height, 0);

		pBasicDraw->TexCoord2f(1, 1);
		pBasicDraw->Vertex3f(x + positions[i][0]*width + width, y + positions[i][1]*height + height, 0);
		pBasicDraw->End();
	}

	// Draw texcoords
	if(!DrawUVCoords(pBasicDraw, x, y, height, width))
		return false;

	vs.modelview.PopMatrix();

	glClear(GL_DEPTH_BUFFER_BIT);
	return true;
}

//=============================================
// @brief Draws mirrored scene
//
//=============================================
bool CGLWindow::DrawMirrored( CBasicDraw* pBasicDraw )
{
	if(!vs.usestencil || !vs.mirror || vs.viewmodel)
		return true;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	/* Don't update color or depth. */
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	/* Draw 1 into the stencil buffer. */
	glEnable( GL_STENCIL_TEST );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
	glStencilFunc( GL_ALWAYS, 1, 0xffffffff );

	/* Now render floor; floor pixels just get their stencil set to 1. */
	DrawFloor(pBasicDraw);

	/* Re-enable update of color and depth. */ 
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	/* Now, only render where stencil is set to 1. */
	glStencilFunc( GL_EQUAL, 1, 0xffffffff );  /* draw if ==1 */
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );

	const Double clipPlane[] = { 
		0.0, 0.0, 1.0, 0.0 
	};

	glEnable( GL_CLIP_PLANE0 );
	glClipPlane( GL_CLIP_PLANE0, clipPlane );

	if(!DrawSkybox(pBasicDraw, true))
	{
		CString error;
		error << "BasicDraw error: " << pBasicDraw->GetShaderError();
		Viewer_ErrorPopup(error.c_str());
		return false;
	}

	vs.modelview.PushMatrix();
	vs.modelview.Scale(1, 1, -1);

	glCullFace(GL_BACK);

	// Disable for model rendering
	pBasicDraw->Disable();

	if(!DrawModel())
		return false;

	// Re-enable after model rendering
	if(!pBasicDraw->Enable())
	{
		CString error;
		error << "BasicDraw error: " << pBasicDraw->GetShaderError();
		Viewer_ErrorPopup(error.c_str());
		return false;
	}

	vs.modelview.PopMatrix();

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CLIP_PLANE0);

	return true;
}

//=============================================
// @brief Draws the skybox
//
//=============================================
bool CGLWindow::DrawSkybox ( CBasicDraw* pBasicDraw, bool mirror )
{
	if (!vs.showskybox || !m_pSkyboxTextures[0]  || vs.showtexture)
		return true;

	Float skyboxVerts[][3] = 
	{
		{-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE}, {SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE}, {SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE}, {-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE},
		{-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE}, {SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE}, {SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE}, {-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE}
	};

	Uint32 skyboxIndexes[6][4] = {{1, 2, 6, 5}, {2, 3, 7, 6}, {3, 0, 4, 7}, {0, 1, 5, 4}, {2, 1, 0, 3}, {7, 4, 5, 6}};

	if(!mirror)
		glCullFace(GL_FRONT);
	else
		glCullFace(GL_BACK);

	glDepthMask(GL_FALSE);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	pBasicDraw->Color4f(1.0, 1.0, 1.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	vs.modelview.PushMatrix();
	vs.modelview.LoadIdentity();
	vs.modelview.Rotate(vs.v_rotation[0], 1.0f, 0.0f, 0.0f);
	vs.modelview.Rotate(vs.v_rotation[1], 0.0f, 0.0f, 1.0f);

	if(mirror)
		vs.modelview.Scale(1.0, 1.0, -1.0);

	pBasicDraw->SetProjection(vs.projection.GetMatrix());
	pBasicDraw->SetModelview(vs.modelview.GetMatrix());

	if(!pBasicDraw->EnableTexture())
		return false;

	for (Int32 i = 0; i < 6; i++)
	{
		Viewer_Bind2DTexture(GL_TEXTURE0_ARB, m_pSkyboxTextures[i]->palloc->gl_index);

		pBasicDraw->Begin(GL_TRIANGLES);

		// Triangle 1
		pBasicDraw->TexCoord2f(0, 1);
		pBasicDraw->Vertex3fv(skyboxVerts[skyboxIndexes[i][0]]);

		pBasicDraw->TexCoord2f(1, 1);
		pBasicDraw->Vertex3fv(skyboxVerts[skyboxIndexes[i][1]]);

		pBasicDraw->TexCoord2f(1, 0);
		pBasicDraw->Vertex3fv(skyboxVerts[skyboxIndexes[i][2]]);

		// Triangle 2
		pBasicDraw->TexCoord2f(0, 1);
		pBasicDraw->Vertex3fv(skyboxVerts[skyboxIndexes[i][0]]);

		pBasicDraw->TexCoord2f(1, 0);
		pBasicDraw->Vertex3fv(skyboxVerts[skyboxIndexes[i][2]]);

		pBasicDraw->TexCoord2f(0, 0);
		pBasicDraw->Vertex3fv(skyboxVerts[skyboxIndexes[i][3]]);
		pBasicDraw->End();
	}

	vs.modelview.PopMatrix();

	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);
	return true;
}

//=============================================
// @brief Draws the ground
//
//=============================================
bool CGLWindow::DrawGround( CBasicDraw* pBasicDraw )
{
	if(!vs.showground)
		return true;

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);

	if(vs.mirror)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	glEnable(GL_BLEND);
	if(!m_pGroundTexture)
	{
		if(!pBasicDraw->DisableTexture())
			return false;

		pBasicDraw->Color4f(vs.groundcolor[0], vs.groundcolor[1], vs.groundcolor[2], GROUND_ALPHA_VALUE);
	}
	else
	{
		if(!pBasicDraw->EnableTexture())
			return false;

		pBasicDraw->Color4f(GL_ONE, GL_ONE, GL_ONE, GROUND_ALPHA_VALUE);
		Viewer_Bind2DTexture(GL_TEXTURE0_ARB, m_pGroundTexture->palloc->gl_index);
	}

	pBasicDraw->SetProjection(vs.projection.GetMatrix());
	pBasicDraw->SetModelview(vs.modelview.GetMatrix());

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
	DrawFloor(pBasicDraw);

	glDisable (GL_BLEND);

	if( vs.mirror )
	{
		glCullFace(GL_BACK);
		if(!pBasicDraw->DisableTexture())
			return false;

		pBasicDraw->Color4f(0.1f, 0.1f, 0.1f, 1.0);
		DrawFloor(pBasicDraw);

		glFrontFace( GL_CCW );
	}
	else
	{
		glEnable( GL_CULL_FACE );
	}

	return true;
}

//=============================================
// @brief Draws the ground
//
//=============================================
bool CGLWindow::DrawModel( void )
{
	if(!vs.pvbmheader || !vs.pstudioheader)
		return true;

	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
	if(!pVBMRenderer)
		return false;

	pVBMRenderer->SetDrawWireframeOverlay(vs.wireframeoverlay);
	pVBMRenderer->SetFOVValue(Viewer_GetFOV());
	pVBMRenderer->SetFrameTime(vs.frametime);
	pVBMRenderer->SetTime(vs.time);
	pVBMRenderer->SetScreenSize(GetWidth(), GetHeight());
	pVBMRenderer->SetScriptPlayback(vs.scriptplayback);
	pVBMRenderer->SetShowAttachments(vs.showattachments);
	pVBMRenderer->SetShowBones(vs.showbones);
	pVBMRenderer->SetShowHitBoxes(vs.showhitboxes);
	pVBMRenderer->SetShowMeshDivisions(vs.showmeshes);
	pVBMRenderer->SetTransparency(vs.transparency);
	pVBMRenderer->SetViewAngles(vs.v_rotation);
	pVBMRenderer->SetViewOrigin(vs.v_translation);
	pVBMRenderer->SetLightColor(vs.lightcolor);
	
	switch(vs.rendermode)
	{
	case RM_WIREFRAME:
		pVBMRenderer->SetRenderMode(CBasicVBMRenderer::RENDER_WIREFRAME);
		break;
	case RM_SMOOTHSHADED:
		pVBMRenderer->SetRenderMode(CBasicVBMRenderer::RENDER_SMOOTHSHADED);
		break;
	case RM_TEXTURED:
		pVBMRenderer->SetRenderMode(CBasicVBMRenderer::RENDER_TEXTURED);
		break;
	}

	if(!pVBMRenderer->DrawModel(vs.modelview, vs.projection))
	{
		Viewer_ErrorPopup(pVBMRenderer->GetShaderError());
		return false;
	}

	return true;
}

//=============================================
// @brief Updates renderer states
//
//=============================================
void CGLWindow::Update( void )
{
	// Set frametime
	Float curtime = Viewer_FloatTime();
	vs.frametime = curtime - vs.time;
	vs.time = curtime;

	CControlPanel* pControlPanel = CControlPanel::GetInstance();

	// Update script playback
	if(vs.scriptplayback)
	{
		if(!vs.scriptpaused)
		{
			// Update playback position in time
			vs.scriptplaybackposition = curtime - vs.playbacktime;

			if(vs.scriptplaybackposition >= vs.scripttimelength)
				pControlPanel->ScriptPlaybackStop();
		}
	}

	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();

	// Update ground position
	if(vs.groundsize)
	{
		Float groundSpeed = pVBMRenderer->GetGroundSpeed();
		if(groundSpeed)
			vs.grounddist += ( groundSpeed / vs.groundsize ) * vs.speedscale * vs.frametime;
	}

	// Set blending operators
	pVBMRenderer->SetBlending (0, 0.0);
	pVBMRenderer->SetBlending (1, 0.0);

	// Advance frame if not paused
	if (!vs.stopplaying)
		pVBMRenderer->AdvanceFrame (vs.frametime * vs.speedscale);

	if(vs.wavplayback)
		Viewer_SetMouthPlayback(vs.wavinfo, vs.playbacktime);

	pControlPanel->Update();
}

//=============================================
// @brief Draws the screen contents
//
//=============================================
void CGLWindow::draw( void )
{
	glClearColor(vs.backgroundcolor[0], vs.backgroundcolor[1], vs.backgroundcolor[2], 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	Float xOrigin = 0;
	Float yOrigin = (vs.flexscripting && vs.scripttimelength) ? FLEX_VISUALIZER_HEIGHT_FRACTION*h2() : 0;

	glViewport(xOrigin, yOrigin, GetWidth(), GetHeight());

	CBasicDraw* pBasicDraw = CBasicDraw::GetInstance();
	if(!pBasicDraw->Enable())
	{
		CString error;
		error << "BasicDraw error: " << pBasicDraw->GetShaderError();
		Viewer_ErrorPopup(error.c_str());
		mx::quit();
		return;
	}

	//
	// show textures
	//
	if (vs.showtexture)
	{
		if(!DrawTexture(pBasicDraw))
		{
			CString error;
			error << "BasicDraw error: " << pBasicDraw->GetShaderError();
			Viewer_ErrorPopup(error.c_str());
			mx::quit();
			return;
		}

		pBasicDraw->Disable();
		return;
	}

	// Set projection
	vs.projection.LoadIdentity();
	Viewer_SetProjectionMatrix(GetWidth(), GetHeight(), Viewer_GetFOV());

	// Set modelview
	vs.modelview.PushMatrix();
	vs.modelview.LoadIdentity();

	if(!vs.viewmodel)
	{
		vs.modelview.Translate(-vs.v_translation[0], -vs.v_translation[1], -vs.v_translation[2]);
		vs.modelview.Rotate(vs.v_rotation[0], 1.0f, 0.0f, 0.0f);
		vs.modelview.Rotate(vs.v_rotation[1], 0.0f, 0.0f, 1.0f);
	}
	else
	{
		vs.modelview.Translate(0, 0, 1);
		vs.modelview.Rotate(-90, 1.0f, 0.0f, 0.0f);
		vs.modelview.Rotate(90, 0.0f, 0.0f, 1.0f);
	}

	// Update matrix states in shader
	pBasicDraw->SetProjection(vs.projection.GetMatrix());
	pBasicDraw->SetModelview(vs.modelview.GetMatrix());

	//
	// Draw normal skybox
	//
	if(!DrawSkybox(pBasicDraw))
	{
		CString error;
		error << "BasicDraw error: " << pBasicDraw->GetShaderError();
		Viewer_ErrorPopup(error.c_str());
		mx::quit();
		return;
	}

	//
	// Draw mirrored model and skybox
	//
	if(!DrawMirrored(pBasicDraw))
	{
		mx::quit();
		return;
	}

	// Disable before rendering model
	pBasicDraw->Disable();

	//
	// Draw normal model
	//
	if(!DrawModel())
	{
		mx::quit();
		return;
	}

	if(!pBasicDraw->Enable())
	{
		CString error;
		error << "BasicDraw error: " << pBasicDraw->GetShaderError();
		Viewer_ErrorPopup(error.c_str());
		mx::quit();
		return;
	}

	//
	// draw ground
	//
	if(!DrawGround(pBasicDraw))
	{
		CString error;
		error << "BasicDraw error: " << pBasicDraw->GetShaderError();
		Viewer_ErrorPopup(error.c_str());
		mx::quit();
		return;
	}

	vs.modelview.PopMatrix();

	// Draw visualizer last
	if(!DrawFlexVisualizer(pBasicDraw))
	{
		CString error;
		error << "BasicDraw error: " << pBasicDraw->GetShaderError();
		Viewer_ErrorPopup(error.c_str());
		mx::quit();
		return;
	}

	pBasicDraw->Disable();
}

//=============================================
// @brief Draws a curved line
//
//=============================================
void CGLWindow::DrawCurvedLine( CBasicDraw* pBasicDraw, Float c1x, Float c1y, Float c2x, Float c2y )
{
	for(Uint32 i = 0; i < CURVE_SEGMENTS; i++)
	{
		Float frac1 = i / (Float)CURVE_SEGMENTS;
		Float frac2 = (i + 1) / (Float)CURVE_SEGMENTS;

		Float f_c1x = c1x + frac1 * ( c2x - c1x );
		Float f_c2x = c1x + frac2 * ( c2x - c1x );

		Float f_c1y = c1y + Common::SplineFraction(frac1, 1.0) * ( c2y - c1y );
		Float f_c2y = c1y + Common::SplineFraction(frac2, 1.0) * ( c2y - c1y );

		pBasicDraw->Vertex3f(f_c1x, f_c1y, 0.0);
		pBasicDraw->Vertex3f(f_c2x, f_c2y, 0.0);
	}
}

//=============================================
// @brief Draws the flex visualizer display
//
//=============================================
bool CGLWindow::DrawFlexVisualizer( CBasicDraw* pBasicDraw )
{
	if(!vs.scripttimelength || !vs.flexscripting)
		return true;
	
	Float totalWidth = w2();
	Float totalHeight = FLEX_VISUALIZER_HEIGHT_FRACTION*h2();
	glViewport(0, 0, totalWidth, totalHeight);

	vs.modelview.PushMatrix();
	vs.modelview.LoadIdentity();

	vs.projection.PushMatrix();
	vs.projection.LoadIdentity();
	vs.projection.Ortho(0.0f, (Float) w2 (), 0.0f, totalHeight, 1.0f, -1.0f);

	pBasicDraw->SetProjection(vs.projection.GetMatrix());
	pBasicDraw->SetModelview(vs.modelview.GetMatrix());

	if(!pBasicDraw->DisableTexture())
		return false;

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	pBasicDraw->Color4f(0.0, 0.0, 0.0, 1.0);

	pBasicDraw->Begin(GL_TRIANGLES);
	pBasicDraw->Vertex3f(0.0, 0.0, 0.0);
	pBasicDraw->Vertex3f(totalWidth, 0.0, 0.0);
	pBasicDraw->Vertex3f(totalWidth, totalHeight, 0.0);
	pBasicDraw->Vertex3f(0.0, 0.0, 0.0);
	pBasicDraw->Vertex3f(totalWidth, totalHeight, 0.0);
	pBasicDraw->Vertex3f(0, totalHeight, 0.0);
	pBasicDraw->End();

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(2);
	pBasicDraw->Color4f(0.0, 0.0, 1.0, 1.0);
	pBasicDraw->Begin(GL_LINES);

	// Draw the scale
	Float inset = w2()*FLEX_VISUALIZER_INSET_FRACTION;
	Float scaleWidth = w2() - inset*2;
	Float lineHeight = totalHeight*FLEX_VISUALIZER_INSET_FRACTION;

	pBasicDraw->Vertex3f(inset, lineHeight, 0);
	pBasicDraw->Vertex3f(inset + scaleWidth, lineHeight, 0);

	// Draw the vertical lines
	for(Float flPos = 0; flPos < vs.scripttimelength; flPos += 0.1)
	{
		Float height = (floor((flPos*10))/10 == floor(flPos)) ? 16 : 8;
		Float xpos = inset + (flPos/vs.scripttimelength)*scaleWidth;

		pBasicDraw->Vertex3f(xpos, lineHeight, 0);
		pBasicDraw->Vertex3f(xpos, lineHeight+height, 0);
	}

	pBasicDraw->End();

	Float selectionSize = (vs.scripttimelength/10.0f);
	flexstate_t* pstate = &vs.flexstate;
	
	// Draw any points and/or lines
	for(Uint32 k = 0; k < vs.flexscript.controllers.size(); k++)
	{
		const flexcontroller_t* pcontroller = &vs.flexscript.controllers[k];
		bool bSelected = (pstate->indexmap[vs.flexindex] == pcontroller->index) ? true : false;

		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if(!pcontroller->binds.empty())
		{
			Float scaleHeight = totalHeight*FLEX_VISUALIZER_SCALE_HEIGHT - totalHeight*FLEX_VISUALIZER_INSET_FRACTION;
			Float selectionTimePos = vs.timeposition*vs.scripttimelength;

			if(pcontroller->binds.size() > 1)	
			{
				// Draw the interpolated lines
				glLineWidth( bSelected ? 6.0f : 3.0f );
				pBasicDraw->Color4f(1.0, 0.0, 0.0, bSelected ? 1.0 : 0.25);

				pBasicDraw->Begin(GL_LINES);
				for(Uint32 i = 1; i < pcontroller->binds.size(); i++)
				{
					// Calculate coords for keyframe 1
					Float offset1 = pcontroller->binds[i-1].time/vs.scripttimelength;
					Float strength1 = Common::SplineFraction(pcontroller->binds[i-1].strength, 1.0/1.0);

					Float xcoord1 = inset + offset1*scaleWidth;
					Float ycoord1 = lineHeight + 4 + strength1*scaleHeight;

					// Calculate coords for keyframe 2
					Float offset2 = pcontroller->binds[i].time/vs.scripttimelength;
					Float strength2 = Common::SplineFraction(pcontroller->binds[i].strength, 1.0/1.0);

					Float xcoord2 = inset + offset2*scaleWidth;
					Float ycoord2 = lineHeight + 4 + strength2*scaleHeight;

					DrawCurvedLine(pBasicDraw, xcoord1, ycoord1, xcoord2, ycoord2);
				}
				pBasicDraw->End();
			}

			// Draw bind points
			glPointSize( bSelected ? 8.0f : 4.0f );

			pBasicDraw->Begin(GL_POINTS);
			for(Int32 i = 0; i < pcontroller->binds.size(); i++)
			{
				if(SDL_fabs(selectionTimePos - pcontroller->binds[i].time) <= CControlPanel::FLEX_DELETION_SENSITIVITY*selectionSize && bSelected)
					pBasicDraw->Color4f(0.0, 1.0, 0.0, bSelected ? 1.0 : 0.25);
				else
					pBasicDraw->Color4f(1.0, 0.0, 0.0, bSelected ? 1.0 : 0.25);

				Float offset = pcontroller->binds[i].time/vs.scripttimelength;
				Float strength = Common::SplineFraction(pcontroller->binds[i].strength, 1.0/1.0);

				Float xcoord = inset + offset*scaleWidth;
				Float ycoord = lineHeight + 4 + strength*scaleHeight;

				pBasicDraw->Vertex3f(xcoord, ycoord, 0.0);
			}
			pBasicDraw->End();
		}
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_POINT_SMOOTH);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	pBasicDraw->Color4f(0.0f, 1.0f, 0.0f, 0.5f);

	Float selectionWidth = ((CControlPanel::FLEX_DELETION_SENSITIVITY*selectionSize)/vs.scripttimelength)*scaleWidth;

	Float xOffs = inset + vs.timeposition*scaleWidth;

	Float x1 = xOffs - selectionWidth;
	Float y1 = lineHeight;
	Float x2 = xOffs + selectionWidth;
	Float y2 = lineHeight + totalHeight*FLEX_VISUALIZER_SCALE_HEIGHT;

	pBasicDraw->Begin(GL_TRIANGLES);
	pBasicDraw->Vertex3f(x1, y1, 0);
	pBasicDraw->Vertex3f(x2, y1, 0);
	pBasicDraw->Vertex3f(x2, y2, 0);
	pBasicDraw->Vertex3f(x1, y1, 0);
	pBasicDraw->Vertex3f(x2, y2, 0);
	pBasicDraw->Vertex3f(x1, y2, 0);
	pBasicDraw->End();

	if(vs.scriptplayback)
	{
		pBasicDraw->Color4f(1.0f, 0.0f, 0.0f, 0.5f);

		Float xOffs = inset + (vs.scriptplaybackposition/vs.scripttimelength)*scaleWidth;

		x1 = xOffs - (selectionWidth/2.0f);
		y1 = lineHeight;
		x2 = xOffs + selectionWidth;
		y2 = lineHeight + totalHeight*FLEX_VISUALIZER_SCALE_HEIGHT;

		pBasicDraw->Begin(GL_TRIANGLES);
		pBasicDraw->Vertex3f(x1, y1, 0);
		pBasicDraw->Vertex3f(x2, y1, 0);
		pBasicDraw->Vertex3f(x2, y2, 0);
		pBasicDraw->Vertex3f(x1, y1, 0);
		pBasicDraw->Vertex3f(x2, y2, 0);
		pBasicDraw->Vertex3f(x1, y2, 0);
		pBasicDraw->End();
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	vs.projection.PopMatrix();
	vs.modelview.PopMatrix();

	return true;
}

//=============================================
// @brief Loads the ground texture
//
//=============================================
bool CGLWindow::LoadGroundTexture( const Char* pstrFilename )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	if(!pstrFilename)
	{
		pTextureManager->DeleteTexture(m_pGroundTexture);
		m_pGroundTexture = nullptr;
		return true;
	}

	// Load the texture in
	en_texture_t* ptexture = pTextureManager->LoadTexture(pstrFilename, RS_WINDOW_LEVEL);
	if(!ptexture)
	{
		if(m_pGroundTexture)
			pTextureManager->DeleteTexture(m_pGroundTexture);

		m_pGroundTexture = nullptr;
		return false;
	}

	// Update in config
	gConfig.SetOption(CMDLViewer::VIEWER_GROUND_TEX_FILE_PATH, pstrFilename);
	gConfig.SaveOptions();

	m_pGroundTexture = ptexture;
	return true;
}

//=============================================
// @brief Loads the skybox textures
//
//=============================================
bool CGLWindow::LoadSkyboxTextures( const Char* pstrFilename )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	if(!pstrFilename || !strlen (pstrFilename))
	{
		for(Uint32 i = 0; i < NUM_SKYBOX_TEXTURES; i++)
		{
			if (!m_pSkyboxTextures[i])
				continue;

			pTextureManager->DeleteTexture(m_pSkyboxTextures[i]);
			m_pSkyboxTextures[i] = nullptr;
		}

		return false;
	}

	// copy the path and lowercase it
	CString filepath(pstrFilename);
	filepath.tolower();
	filepath.replaceslashes();

	Int32 offset = filepath.find(0, TEXTURE_BASE_DIRECTORY_PATH);
	if(offset != -1)
	{
		Int32 length = qstrlen(TEXTURE_BASE_DIRECTORY_PATH);
		filepath.erase(0, offset + length);
	}

	// Get the base path
	CString basepath;

	// Look up the postfix
	for(Uint32 i = 0; i < NUM_SKYBOX_TEXTURES; i++)
	{
		CString pstrSuffix;
		pstrSuffix << SKY_POSTFIXES[i] << ".";

		Int32 postfixPos = filepath.find(0, pstrSuffix.c_str());
		if(postfixPos != -1)
		{
			basepath.assign(filepath.c_str(), postfixPos);
			break;
		}
	}

	if(basepath.empty())
		return false;

	// Get extension from filename
	CString extension;
	CString strfilename(pstrFilename);
	Int32 dotpos = strfilename.find(0, ".");
	if(dotpos != -1)
	{
		Uint32 nbChars = strfilename.length() - (dotpos + 1);
		const Char* pstrBegin = strfilename.c_str() + dotpos + 1;
		extension.assign(pstrBegin, nbChars);
	}
	else
	{
		// Just use TGA
		extension = "tga";
	}

	for(Uint32 i = 0; i < NUM_SKYBOX_TEXTURES; i++)
	{
		CString filepath;
		filepath << basepath << SKY_POSTFIXES[i] << "." << extension;

		en_texture_t* ptexture = pTextureManager->LoadTexture(filepath.c_str(), RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS|TX_FL_CLAMP_S|TX_FL_CLAMP_T);
		if(!ptexture)
		{
			for(Uint32 i = 0; i < NUM_SKYBOX_TEXTURES; i++)
			{
				if (!m_pSkyboxTextures[i])
					continue;

				pTextureManager->DeleteTexture(m_pSkyboxTextures[i]);
				m_pSkyboxTextures[i] = nullptr;
			}

			return false;
		}

		m_pSkyboxTextures[i] = ptexture;
	}

	// Update in config
	gConfig.SetOption(CMDLViewer::VIEWER_ENV_TEX_FILE_PATH, pstrFilename);
	gConfig.SaveOptions();

	return true;
}

//=============================================
// @brief Dumps viewport contents into a screenshot
//
//=============================================
void CGLWindow::DumpViewport( const Char *filename )
{
#ifdef WIN32
	redraw ();
	int w = w2 ();
	int h = h2 ();

	mxImage *image = new mxImage ();
	if (image->create (w, h, 24))
	{
#if 0
		glReadBuffer (GL_FRONT);
		glReadPixels (0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, image->data);
#else
		HDC hdc = GetDC ((HWND) getHandle ());
		byte *data = (byte *) image->data;
		int i = 0;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				COLORREF cref = GetPixel (hdc, x, y);
				data[i++] = (byte) ((cref >> 0)& 0xff);
				data[i++] = (byte) ((cref >> 8) & 0xff);
				data[i++] = (byte) ((cref >> 16) & 0xff);
			}
		}
		ReleaseDC ((HWND) getHandle (), hdc);
#endif
		if (!mxTgaWrite (filename, image))
			mxMessageBox (this, "Error writing screenshot.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);

		delete image;
	}
#endif	
}

//=============================================
// @brief Returns the width of the window
//
//=============================================
Uint32 CGLWindow::GetWidth( void )
{
	return w2();
}

//=============================================
// @brief Returns the height of the window
//
//=============================================
Uint32 CGLWindow::GetHeight( void )
{
	if(vs.flexscripting && vs.scripttimelength)
		return h2() - FLEX_VISUALIZER_HEIGHT_FRACTION*h2();
	else
		return h2();
}

//=============================================
// @brief Creates an instance of this class
//
// @return Created instance
//=============================================
CGLWindow* CGLWindow::CreateInstance( mxWindow* pParent )
{
	if(!g_pInstance)
		g_pInstance = new CGLWindow(pParent, 0, 0, 0, 0, "", mxWindow::Normal);

	return g_pInstance;
}

//=============================================
// @brief Returns the current instance of this class
//
// @return Current instance, or nullptr
//=============================================
CGLWindow* CGLWindow::GetInstance( void )
{
	return g_pInstance;
}

//=============================================
// @brief Deletes the current instance of this class
//
//=============================================
void CGLWindow::DeleteInstance( void )
{
	if(!g_pInstance)
		return;

	delete g_pInstance;
	g_pInstance = nullptr;
}