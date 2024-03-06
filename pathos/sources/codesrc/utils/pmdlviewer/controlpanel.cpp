//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           CControlPanel.cpp
// last modified:  Oct 20 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.24
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mx/mx.h>
#include <mx/mxBmp.h>
#include <mx/mxmessagebox.h>

#include "includes.h"
#include "controlpanel.h"
#include "flexmanager.h"
#include "flex_shared.h"
#include "viewerstate.h"
#include "r_vbmbasic.h"
#include "glwindow.h"
#include "mdlviewer.h"
#include "config.h"
#include "stepsound.h"

// Last loaded WAV file option header
const char CControlPanel::CP_LAST_WAV_PATH[] = "LAST_WAV_PATH";
// Last loaded script file option header
const char CControlPanel::CP_LAST_SCRIPT_PATH[] = "LAST_SCRIPT_PATH";
// Last loaded skybox option header
const char CControlPanel::CP_LAST_ENV_TEX_PATH[] = "LAST_ENV_TEX_PATH";
// Last loaded ground texture option header
const char CControlPanel::CP_LAST_GROUND_TEX_PATH[] = "LAST_GROUND_TEX_PATH";
// Whether ground was enabled
const Char CControlPanel::CP_GROUND_ENABLED[] = "GROUND_RENDER_ENABLED";
// Whether mirroring was enabled
const Char CControlPanel::CP_MIRROR_ENABLED[] = "GROUND_MIRROR_ENABLED";
// Whether skybox was enabled
const Char CControlPanel::CP_SKYBOX_ENABLED[] = "SKYBOX_ENABLED";

// Deletion sensitivity
const float CControlPanel::FLEX_DELETION_SENSITIVITY	= 0.05;

// Current instance of this class
CControlPanel* CControlPanel::g_pInstance = nullptr;

//=============================================
// @brief Constructor
//
//=============================================
CControlPanel::CControlPanel( mxWindow *parent ): 
	mxWindow(parent, 0, 0, 0, 0, "Control Panel", mxWindow::Normal),
	m_pTab(nullptr),
	m_pChoiceRenderMode(nullptr),
	m_pSliderTransparency(nullptr),
	m_pCheckBoxGround(nullptr),
	m_pCheckBoxMirror(nullptr),
	m_pCheckBoxSkybox(nullptr),
	m_pChoiceSequence(nullptr),
	m_pSliderSpeedScale(nullptr),
	m_pToggleButtonStop(nullptr),
	m_pButtonPrevFrame(nullptr),
	m_pButtonNextFrame(nullptr),
	m_pButtonFlexPlay(nullptr),
	m_pChoiceMaterial(nullptr),
	m_pLineEditFrame(nullptr),
	m_pChoiceBodypart(nullptr),
	m_pChoiceController(nullptr),
	m_pChoiceSubmodel(nullptr),
	m_pSliderController(nullptr),
	m_pChoiceSkin(nullptr),
	m_pLabelModelInfo1(nullptr),
	m_pLabelModelInfo2(nullptr),
	m_pLabelModelInfo3(nullptr),
	m_pChoiceTextures(nullptr),
	m_pCheckBoxChrome(nullptr),
	m_pCheckBoxAdditive(nullptr),
	m_pCheckBoxAlphaTest(nullptr),
	m_pCheckBoxNoMipmaps(nullptr),
	m_pCheckBoxUnlit(nullptr),
	m_pCheckBoxScope(nullptr),
	m_pCheckBoxTransparent(nullptr),
	m_pCheckBoxEyeGlint(nullptr),
	m_pCheckBoxNoDecal(nullptr),
	m_pCheckBoxNoCulling(nullptr),
	m_pLabelTransSlider(nullptr),
	m_pSliderTransSlider(nullptr),
	m_pLabelTexSize(nullptr),
	m_pLabelTexScale(nullptr),
	m_pLineEditWidth(nullptr),
	m_pLineEditHeight(nullptr),
	m_pLabelSequenceIndex(nullptr),
	m_pLabelFrameCount(nullptr),
	m_pLabelFramerate(nullptr),
	m_pLabelFrame(nullptr),
	m_pChoiceFlex(nullptr),
	m_pSliderFlexStrength(nullptr),
	m_pLabelFlexTimeLabel(nullptr),
	m_pLabelWAVLengthLabel(nullptr),
	m_pLineEditLength(nullptr),
	m_pLabelLength(nullptr),
	m_pCheckBoxFlexStay(nullptr),
	m_pCheckBoxFlexLoop(nullptr),
	m_pLabelFlexNames(nullptr),
	m_pSliderFlexScalers(nullptr),
	m_numFlexes(0),
	m_pWindowFlexes(nullptr),
	m_pWindowFlexScripting(nullptr),
	m_pTextureWindow(nullptr)
{
	// create tabcontrol with subdialog windows
	m_pTab = new mxTab (this, 0, 0, 0, 0, IDC_TAB);
	SetWindowLong((HWND) m_pTab->getHandle(), GWL_EXSTYLE, WS_EX_CLIENTEDGE);

	// Init render tab
	InitRenderTab();

	// Sequence tab
	InitSequenceTab();

	// Body tab
	InitBodyTab();

	// Texture tab
	InitTextureTab();

	// Flexes tab
	InitFlexesTab();
	
	// Flex scripting tab
	InitFlexScriptingTab();
}

//=============================================
// @brief Destructor
//
//=============================================
CControlPanel::~CControlPanel( void )
{
	if(m_pLabelFlexNames)
	{
		delete[] m_pLabelFlexNames;
		m_pLabelFlexNames = nullptr;
	}

	if(m_pSliderFlexScalers)
	{
		delete[] m_pSliderFlexScalers;
		m_pSliderFlexScalers = nullptr;
	}

	// mx deletes these windows before we 
	// get a chance to do it via DeleteInstance()
	g_pInstance = nullptr;
}

//=============================================
// @brief Creates and populated the "Render" tab
//
//=============================================
void CControlPanel::InitRenderTab( void )
{
	// Render tab
	mxWindow *wRender = new mxWindow (this, 0, 0, 0, 0);
	m_pTab->add(wRender, "Render");
	m_pChoiceRenderMode = new mxChoice (wRender, 5, 5, 100, 22, IDC_RENDERMODE);
	m_pChoiceRenderMode->add ("Wireframe");
	m_pChoiceRenderMode->add ("Smoothshaded");
	m_pChoiceRenderMode->add ("Textured");
	m_pChoiceRenderMode->select (2);
	mxToolTip::add (m_pChoiceRenderMode, "Select Render Mode");

	m_pSliderTransparency = new mxSlider (wRender, 5, 28, 100, 18, IDC_TRANSPARENCY);
	m_pSliderTransparency->setValue (100);
	m_pSliderTransparency->setRange(100, 0);
	mxToolTip::add (m_pSliderTransparency, "Model Transparency");

	m_pCheckBoxGround = new mxCheckBox (wRender, 110, 5, 150, 20, "Ground", IDC_GROUND);
	const Char* pstrValue = gConfig.GetOptionValue(CP_GROUND_ENABLED);
	if(pstrValue && !qstrcmp(pstrValue, "1"))
	{
		m_pCheckBoxGround->setChecked(true);
		vs.showground = true;
	}
	else
	{
		m_pCheckBoxGround->setChecked(false);
		vs.showground = false;
	}

	m_pCheckBoxMirror = new mxCheckBox (wRender, 110, 25, 150, 20, "Mirror Model On Ground", IDC_MIRROR);
	pstrValue = gConfig.GetOptionValue(CP_MIRROR_ENABLED);
	if(pstrValue && !qstrcmp(pstrValue, "1"))
	{
		m_pCheckBoxMirror->setChecked(true);
		vs.mirror = true;
		vs.usestencil = true;
	}
	else
	{
		m_pCheckBoxMirror->setChecked(false);
		vs.mirror = false;
		vs.usestencil = false;
	}

	m_pCheckBoxSkybox = new mxCheckBox (wRender, 110, 45, 150, 20, "Skybox", IDC_SKYBOX);
	pstrValue = gConfig.GetOptionValue(CP_SKYBOX_ENABLED);
	if(pstrValue && !qstrcmp(pstrValue, "1"))
	{
		m_pCheckBoxSkybox->setChecked(true);
		vs.showskybox = true;
	}
	else
	{
		m_pCheckBoxSkybox->setChecked(false);
		vs.showskybox = false;
	}

	mxCheckBox *pCheckBoxHitBoxes = new mxCheckBox (wRender, 110, 65, 150, 20, "Hit Boxes", IDC_HITBOXES);
	mxCheckBox *pCheckBoxBones = new mxCheckBox (wRender, 5, 65, 100, 20, "Bones", IDC_BONES);
	mxCheckBox *pCheckBoxAttachments = new mxCheckBox (wRender, 5, 45, 100, 20, "Attachments", IDC_ATTACHMENTS);
	mxCheckBox *pCheckBoxMeshGroups = new mxCheckBox (wRender, 280, 5, 150, 20, "Mesh Division", IDC_MESHGROUPS);
	mxCheckBox *pCheckBoxWifreframe = new mxCheckBox (wRender, 280, 25, 150, 20, "Wireframe Overlay", IDC_WIREFRAMEOVERLAY);
	mxCheckBox *pCheckBoxViewModel = new mxCheckBox (wRender, 280, 45, 150, 20, "View Model", IDC_VIEWMODEL);
	mxCheckBox *pCheckBoxLargeFOV = new mxCheckBox (wRender, 280, 65, 150, 20, "Large FOV", IDC_LARGEFOV);
}

//=============================================
// @brief Creates and populated the "Render" tab
//
//=============================================
void CControlPanel::InitSequenceTab( void )
{
	mxWindow *pWindowSequence = new mxWindow(this, 0, 0, 0, 0);
	m_pTab->add (pWindowSequence, "Sequence");

	m_pChoiceSequence = new mxChoice (pWindowSequence, 5, 5, 200, 22, IDC_SEQUENCE);	
	mxToolTip::add (m_pChoiceSequence, "Select Sequence");

	m_pSliderSpeedScale = new mxSlider (pWindowSequence, 5, 32, 200, 18, IDC_SPEEDSCALE);
	m_pSliderSpeedScale->setRange (0, 200);
	m_pSliderSpeedScale->setValue (40);

	mxToolTip::add (m_pSliderSpeedScale, "Speed Scale");
	m_pToggleButtonStop = new mxToggleButton (pWindowSequence, 5, 55, 60, 22, "Stop", IDC_STOP);
	mxToolTip::add (m_pToggleButtonStop, "Stop Playing");

	m_pButtonPrevFrame = new mxButton (pWindowSequence, 70, 55, 30, 22, "<<", IDC_PREVFRAME);
	m_pButtonPrevFrame->setEnabled (false);
	mxToolTip::add (m_pButtonPrevFrame, "Prev Frame");

	m_pLineEditFrame = new mxLineEdit (pWindowSequence, 105, 55, 50, 22, "", IDC_FRAME); 
	m_pLineEditFrame->setEnabled (false);
	mxToolTip::add (m_pLineEditFrame, "Set Frame");

	m_pButtonNextFrame = new mxButton (pWindowSequence, 160, 55, 30, 22, ">>", IDC_NEXTFRAME);
	m_pButtonNextFrame->setEnabled (false);
	mxToolTip::add (m_pButtonNextFrame, "Next Frame");

	m_pChoiceMaterial = new mxChoice (pWindowSequence, 215, 5, 200, 22, IDC_MATERIAL);	
	mxToolTip::add (m_pChoiceSequence, "Select footstep material");

	Int32 labelsOffset = 435;
	m_pLabelSequenceIndex = new mxLabel(pWindowSequence, labelsOffset, 10, 100, 18, "Sequence index:");
	m_pLabelFrameCount = new mxLabel(pWindowSequence, labelsOffset, 28, 100, 18, "Frames:");
	m_pLabelFramerate = new mxLabel(pWindowSequence, labelsOffset, 44, 100, 18, "FPS:");
	m_pLabelFrame = new mxLabel(pWindowSequence, labelsOffset, 62, 100, 18, "Frame:");
}

//=============================================
// @brief Creates and populated the "Render" tab
//
//=============================================
void CControlPanel::InitBodyTab( void )
{
	mxWindow *pWinbowBody = new mxWindow(this, 0, 0, 0, 0);
	m_pTab->add (pWinbowBody, "Body");

	m_pChoiceBodypart = new mxChoice(pWinbowBody, 5, 5, 100, 22, IDC_BODYPART);
	mxToolTip::add(m_pChoiceBodypart, "Choose a bodypart");

	m_pChoiceSubmodel = new mxChoice(pWinbowBody, 110, 5, 100, 22, IDC_SUBMODEL);
	mxToolTip::add(m_pChoiceSubmodel, "Choose a submodel of current bodypart");

	m_pChoiceController = new mxChoice(pWinbowBody, 5, 30, 100, 22, IDC_CONTROLLER);	
	mxToolTip::add(m_pChoiceController, "Choose a bone controller");

	m_pSliderController = new mxSlider(pWinbowBody, 105, 32, 100, 18, IDC_CONTROLLERVALUE);
	m_pSliderController->setRange (0, 45);
	mxToolTip::add(m_pSliderController, "Change current bone controller value");

	m_pLabelModelInfo1 = new mxLabel(pWinbowBody, 220, 5, 120, 100, "No Model.");
	m_pLabelModelInfo2 = new mxLabel(pWinbowBody, 340, 5, 120, 100, "");
	m_pLabelModelInfo3 = new mxLabel(pWinbowBody, 460, 5, 120, 100, "");
	m_pChoiceSkin = new mxChoice(pWinbowBody, 5, 55, 100, 22, IDC_SKINS);

	mxToolTip::add(m_pChoiceSkin, "Choose a skin family");
}

//=============================================
// @brief Creates and populated the "Texture" tab
//
//=============================================
void CControlPanel::InitTextureTab( void )
{
	mxWindow *pWindowTexture = new mxWindow(this, 0, 0, 0, 0);

	m_pTab->add(pWindowTexture, "Texture");
	m_pChoiceTextures = new mxChoice(pWindowTexture, 5, 5, 150, 22, IDC_TEXTURES);
	mxToolTip::add(m_pChoiceTextures, "Choose a texture");

	m_pLabelTexSize = new mxLabel(pWindowTexture, 162, 70, 150, 18, "Width x Height");
	m_pLabelTexScale = new mxLabel(pWindowTexture, 5, 40, 150, 18, "Magnification: 1.0");
	mxSlider* ptexSlider = new mxSlider(pWindowTexture, 5, 57, 150, 18, IDC_TEXTURESCALE);
	mxToolTip::add(ptexSlider, "Scale texture size");
	
	ptexSlider->setRange (0, 100);
	ptexSlider->setValue (0);

	m_pCheckBoxChrome = new mxCheckBox(pWindowTexture, 250, 5, 150, 22, "Chrome", IDC_CHROME);
	m_pCheckBoxAdditive = new mxCheckBox(pWindowTexture, 250, 25, 150, 22, "Additive", IDC_ADDITIVE);
	m_pCheckBoxAlphaTest = new mxCheckBox(pWindowTexture, 400, 5, 150, 22, "Alphatest", IDC_ALPHATEST);
	m_pCheckBoxNoMipmaps = new mxCheckBox(pWindowTexture, 400, 25, 150, 22, "No Mipmaps", IDC_NOMIPMAPS);
	m_pCheckBoxUnlit = new mxCheckBox(pWindowTexture, 550, 5, 150, 22, "Unlit", IDC_UNLIT);
	m_pCheckBoxScope = new mxCheckBox(pWindowTexture, 550, 25, 150, 22, "Scope", IDC_SCOPE);
	m_pCheckBoxTransparent = new mxCheckBox(pWindowTexture, 700, 25, 150, 22, "Transparent", IDC_TRANSPARENT);
	m_pCheckBoxEyeGlint = new mxCheckBox(pWindowTexture, 700, 5, 150, 22, "Eye Glint", IDC_EYEGLINT);
	m_pCheckBoxNoDecal = new mxCheckBox(pWindowTexture, 850, 25, 150, 22, "No decals", IDC_NO_DECAL);
	m_pCheckBoxNoCulling = new mxCheckBox(pWindowTexture, 850, 5, 150, 22, "No face cull", IDC_NO_FACE_CULL);

	m_pLabelTransSlider = new mxLabel(pWindowTexture, 250, 57, 70, 18, "Transparency");
	m_pSliderTransSlider = new mxSlider(pWindowTexture, 320, 57, 150, 18, IDC_TRANSPARENCY_SCALE);
	mxToolTip::add(m_pSliderTransSlider, "Transparency");

	m_pSliderTransSlider->setRange(0, 100);
	m_pSliderTransSlider->setValue(0);
	m_pSliderTransSlider->setEnabled(false);
	m_pLabelTransSlider->setEnabled(false);
}

//=============================================
// @brief Creates and populated the "Render" tab
//
//=============================================
void CControlPanel::InitFlexesTab( void )
{
	m_pWindowFlexes = new mxWindow(this, 0, 0, 0, 0);
	m_pTab->add(m_pWindowFlexes, "Flexes");

	m_numFlexes = 0;
}

//=============================================
// @brief Creates and populated the "Render" tab
//
//=============================================
void CControlPanel::InitFlexScriptingTab( void )
{
	// Flex scriptinmxWindow *wFlexScriptingg tab
	m_pWindowFlexScripting = new mxWindow(this, 0, 0, 0, 0);
	m_pTab->add(m_pWindowFlexScripting, "Flex Scripting");

	m_pChoiceFlex = new mxChoice(m_pWindowFlexScripting, 5, 5, 200, 22, IDC_SCRIPTFLEX_CHOICE);
	m_pChoiceFlex->select(0);

	m_pSliderFlexStrength = new mxSlider(m_pWindowFlexScripting, 5, 32, 200, 30, IDC_SCRIPTFLEX_STRENGTH);
	m_pSliderFlexStrength->setRange(0, 1);
	m_pSliderFlexStrength->setValue(0);

	m_pLabelLength = new mxLabel(m_pWindowFlexScripting, 230, 5, 60, 18, "Duration:");
	m_pLineEditLength = new mxLineEdit(m_pWindowFlexScripting, 285, 5, 45, 22, "", IDC_FLEX_LENGTH);

	m_pButtonFlexPlay = new mxButton(m_pWindowFlexScripting, 425, 5, 75, 18, "Play", IDC_FLEX_PLAYBACK);
	new mxButton(m_pWindowFlexScripting, 340, 5, 75, 18, "Load WAV", IDC_FLEX_WAVBUTTON);
	new mxButton(m_pWindowFlexScripting, 510, 5, 75, 18, "Stop", IDC_FLEX_STOP);
	new mxButton(m_pWindowFlexScripting, 595, 5, 75, 18, "Bind", IDC_FLEX_BIND);
	new mxButton(m_pWindowFlexScripting, 340, 30, 75, 18, "Delete", IDC_FLEX_DELETE);
	new mxButton(m_pWindowFlexScripting, 425, 30, 75, 18, "Load script", IDC_FLEX_LOAD_SCRIPT);
	new mxButton(m_pWindowFlexScripting, 510, 30, 75, 18, "Save script", IDC_FLEX_SAVE_SCRIPT);
	new mxButton(m_pWindowFlexScripting, 595, 30, 75, 18, "Reset", IDC_FLEX_RESET_SCRIPT);

	m_pCheckBoxFlexLoop = new mxCheckBox(m_pWindowFlexScripting, 680, 5, 60, 20, "Loop", IDC_FLEX_LOOP);
	m_pCheckBoxFlexStay = new mxCheckBox(m_pWindowFlexScripting, 680, 30, 60, 20, "Stay", IDC_FLEX_STAY);

	m_pLabelWAVLengthLabel = new mxLabel(m_pWindowFlexScripting, 5, 70, 100, 18, "WAV Length: 0.00s");
	m_pLabelFlexTimeLabel = new mxLabel(m_pWindowFlexScripting, 110, 70, 100, 18, "Time: 0.00s");
}

//=============================================
// @brief Sets flex names based on vbm file contents
//
//=============================================
void CControlPanel::SetFlexNames( void )
{
	ClearFlexNames();

	const vbmheader_t* pvbm = vs.pvbmheader;
	if(!pvbm || !pvbm->numflexinfo || !pvbm->numflexcontrollers)
		return;

	static const Int32 FLEXES_X_BASE_ORIGIN = 5;
	static const Int32 FLEXES_Y_BASE_ORIGIN = 5;
	static const Int32 FLEXES_X_ROW_OFFSET = 100;
	static const Int32 FLEXES_Y_OFFSET = 18;

	Int32 x = FLEXES_X_BASE_ORIGIN;
	Int32 y = FLEXES_Y_BASE_ORIGIN;
	Int32 row = 0;

	m_pLabelFlexNames = new mxLabel*[pvbm->numflexcontrollers];
	m_pSliderFlexScalers = new mxSlider*[pvbm->numflexcontrollers];
	m_numFlexes = pvbm->numflexcontrollers;

	vbmflexcontroller_t* pflexcontrollers = (vbmflexcontroller_t*)((byte*)pvbm + pvbm->flexcontrolleroffset);
	for(Int32 i = 0; i < pvbm->numflexcontrollers; i++)
	{
		if( row > 1 )
		{
			row = 0;
			x += FLEXES_X_ROW_OFFSET;
			y = FLEXES_Y_BASE_ORIGIN;
		}

		m_pLabelFlexNames[i] = new mxLabel(m_pWindowFlexes, x, y, 100, 18, pflexcontrollers[i].name);
		y += FLEXES_Y_OFFSET;
		
		m_pSliderFlexScalers[i] = new mxSlider(m_pWindowFlexes, x, y, 100, 18, IDC_FLEXCONTROLLER_BASE + i);
		y += FLEXES_Y_OFFSET;

		m_pSliderFlexScalers[i]->setRange (0, 100);
		m_pSliderFlexScalers[i]->setValue (0);
		row++;

		m_pChoiceFlex->add( pflexcontrollers[i].name );
	}

	m_pChoiceFlex->select(0);
}

//=============================================
// @brief Clears flex labels and scalers
//
//=============================================
void CControlPanel::ClearFlexNames ( void )
{
	m_pChoiceFlex->removeAll();
	
	if(m_pLabelFlexNames)
	{
		for(Int32 i = 0; i < m_numFlexes; i++)
		{
			delete m_pLabelFlexNames[i];
			m_pLabelFlexNames[i] = nullptr;
		}

		delete[] m_pLabelFlexNames;
		m_pLabelFlexNames = nullptr;
	}

	if(m_pSliderFlexScalers)
	{
		for(Int32 i = 0; i < m_numFlexes; i++)
		{
			delete m_pSliderFlexScalers[i];
			m_pSliderFlexScalers[i] = nullptr;
		}

		delete[] m_pSliderFlexScalers;
		m_pSliderFlexScalers = nullptr;
	}
}

//=============================================
// @brief Returns a flex's label name
//
//=============================================
const Char* CControlPanel::GetFlexLabel( Int32 index )
{
	if(index < 0 || index >= m_numFlexes)
		return nullptr;

	return m_pLabelFlexNames[index]->getLabel();
}

//=============================================
// @brief Sets time position for flex editor
//
//=============================================
void CControlPanel::SetTimePosition( Float position, bool setCursor )
{
	if(!vs.pflexmanager)
		return;

	if(setCursor)
		vs.timeposition = position;

	if(!vs.pflexvalues)
		return;

	if(!m_pLabelFlexNames)
		return;

	flexstate_t* pflexState = &vs.flexstate;
	pflexState->time = 0;

	Float sliderTimePosition = vs.timeposition*vs.scripttimelength;

	vs.pflexmanager->UpdateValues( sliderTimePosition, 100, vs.pflexvalues[FLEX_MOUTH_OPEN], pflexState, true );
	m_pLabelFlexTimeLabel->setLabel("Time: %.2fs", sliderTimePosition);

	const Char* pstrcontrollername = m_pLabelFlexNames[vs.flexindex]->getLabel();
	Int32 index = vs.pflexmanager->GetControllerIndex(&vs.flexscript, pstrcontrollername);

	if(index != -1)
	{
		vs.flexvalue = pflexState->values[index];
		m_pSliderFlexStrength->setValue( vs.flexvalue );
	}
}

//=============================================
// @brief Stops script playback
//
//=============================================
void CControlPanel::ScriptPlaybackStop( void )
{
	vs.wavplayback = false;
	vs.scriptplayback = false;
	vs.scriptpaused = false;
	vs.scriptplaybackposition = 0;

	SetTimePosition(0, false);

	// Change label
	m_pButtonFlexPlay->setLabel("Play");
	// Shut up sounds
	PlaySound(nullptr, 0, 0);	
}

//=============================================
// @brief Set a texture flag
//
//=============================================
void CControlPanel::SetTextureFlag( mxCheckBox* pBox, Int32 texture, textureflags_t flag )
{
	vbmheader_t* pvbm = vs.pvbmheader;
	if(!pvbm)
		return;

	vbmtexture_t *ptexture = pvbm->getTexture(texture);
	if(!pvbm)
	{
		mxMessageBox(this, "Texture index was invalid.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return;
	}
	
	en_material_t* pmaterial = CTextureManager::GetInstance()->FindMaterialScriptByIndex(ptexture->index);
	if(!pmaterial)
	{
		mxMessageBox(this, "Material index was invalid.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return;
	}

	Int32 flagToCheck = 0;
	switch(flag)
	{
	case MDLV_TF_FULLBRIGHT:
		flagToCheck = TX_FL_FULLBRIGHT;
		break;
	case MDLV_TF_NODECAL:
		flagToCheck = TX_FL_NODECAL;
		break;
	case MDLV_TF_ALPHATEST:
		flagToCheck = TX_FL_ALPHATEST;
		break;
	case MDLV_TF_CLAMP:
		flagToCheck = (TX_FL_CLAMP_S|TX_FL_CLAMP_T);
		break;
	case MDLV_TF_EYEGLINT:
		flagToCheck = TX_FL_EYEGLINT;
		break;
	case MDLV_TF_CHROME:
		flagToCheck = TX_FL_CHROME;
		break;
	case MDLV_TF_ADDITIVE:
		flagToCheck = TX_FL_ADDITIVE;
		break;
	case MDLV_TF_SCOPE:
		flagToCheck = TX_FL_SCOPE;
		break;
	case MDLV_TF_NOCULL:
		flagToCheck = TX_FL_NO_CULLING;
		break;
	case MDLV_TF_ALPHABLEND:
		flagToCheck = TX_FL_ALPHABLEND;
		break;
	case MDLV_TF_NO_MIPMAP:
		flagToCheck = TX_FL_NOMIPMAPS;
		break;
	case MDLV_TF_NO_DECAL:
		flagToCheck = TX_FL_NODECAL;
		break;
	}

	if(pBox->isChecked())
		pmaterial->flags |= flagToCheck;
	else
		pmaterial->flags &= ~flagToCheck;

	if(flag == MDLV_TF_ADDITIVE)
	{
		if(pBox->isChecked())
		{
			m_pSliderTransSlider->setEnabled(true);
			m_pLabelTransSlider->setEnabled(true);
			m_pSliderTransSlider->setValue(pmaterial->alpha*100);
		}
		else
		{
			m_pSliderTransSlider->setValue(0);
			m_pSliderTransSlider->setEnabled(false);
			m_pSliderTransSlider->setEnabled(false);
		}
	}
}

//=============================================
// @brief Handles an input/window event
//
//=============================================
int CControlPanel::handleEvent( mxEvent *pEvent )
{
	// Change in size
	if (pEvent->event == mxEvent::Size)
	{
		m_pTab->setBounds (0, 0, pEvent->width, pEvent->height);
		return 1;
	}

	// Flex sliders are a special case
	if(pEvent->action >= IDC_FLEXCONTROLLER_BASE && pEvent->action < IDC_FLEXCONTROLLER_BASE + MAX_VBM_FLEXES)
	{
		if(!vs.pflexmanager)
			return 1;

		flexstate_t& flexstate = vs.flexstate;
		if(!flexstate.pscript)
			return 1;
		
		Int32 labelidx = pEvent->action - IDC_FLEXCONTROLLER_BASE;
		const Char *pstrname = m_pLabelFlexNames[labelidx]->getLabel();
		
		// This is done for assigning indexes to vbm flex controllers
		Int32 index = vs.pflexmanager->GetControllerIndex(flexstate.pscript, pstrname);
		if(index == -1)
		{
			vs.flexscript.controllers.resize(vs.flexscript.controllers.size()+1);
			flexcontroller_t* pcontroller = &vs.flexscript.controllers[vs.flexscript.controllers.size()-1];

			// Use the internal function to set the name
			vs.pflexmanager->SetControllerName(&vs.flexscript, pcontroller, pstrname);
			index = pcontroller->index;

			if(vs.pvbmheader)
				vs.pflexmanager->SetFlexMappings(vs.pvbmheader, &vs.flexstate);
		}

		// Set value if index is valid
		if(index != -1)
		{
			Float value = ((mxSlider*)pEvent->widget)->getValue();
			vs.pflexvalues[index] = value / 100.0f;
		}

		return 1;
	}

	switch (pEvent->action)
	{
	case IDC_TAB:
		{
			Int32 index = m_pTab->getSelectedIndex();
			switch(index)
			{
			case TAB_INDEX_RENDER:
			case TAB_INDEX_SEQUENCE:
			case TAB_INDEX_BODY:
			case TAB_INDEX_FLEXES:
				{
					vs.showtexture = false;
					vs.flexscripting = false;
				}
				break;
			case TAB_INDEX_TEXTURE:
				{
					vs.showtexture = true;
					vs.flexscripting = false;
				}
				break;
			case TAB_INDEX_FLEX_SCRIPTING:
				{
					vs.flexscripting = true;
					vs.showtexture = false;

					SetTimePosition(vs.timeposition, true);
				}
				break;
			}
		}
		break;
	case IDC_RENDERMODE:
		{
			Int32 index = m_pChoiceRenderMode->getSelectedIndex();
			switch(index)
			{
			case 0:
				SetRenderMode(RM_WIREFRAME);
				break;
			case 1:
				SetRenderMode(RM_SMOOTHSHADED);
				break;
			case 2:
				SetRenderMode(RM_TEXTURED);
				break;
			}
		}
		break;
	case IDC_TRANSPARENCY:
		{
			Float value = m_pSliderTransparency->getValue();
			vs.transparency = value/100.0f; 
		}
		break;
	case IDC_GROUND:
		{
			mxCheckBox* pCheckBox = reinterpret_cast<mxCheckBox*>(pEvent->widget);
			SetShowGround(pCheckBox->isChecked() ? true : false);

			CString optionValue = pCheckBox->isChecked() ? "1" : "0";
			gConfig.SetOption(CP_GROUND_ENABLED, optionValue.c_str());
			gConfig.SaveOptions();
		}
		break;
	case IDC_MIRROR:
		{
			mxCheckBox* pCheckBox = reinterpret_cast<mxCheckBox*>(pEvent->widget);
			SetMirror(pCheckBox->isChecked() ? true : false);

			CString optionValue = pCheckBox->isChecked() ? "1" : "0";
			gConfig.SetOption(CP_MIRROR_ENABLED, optionValue.c_str());
			gConfig.SaveOptions();
		}
		break;
	case IDC_SKYBOX:
		{
			mxCheckBox* pCheckBox = reinterpret_cast<mxCheckBox*>(pEvent->widget);
			SetShowSkybox(pCheckBox->isChecked() ? true : false);

			CString optionValue = pCheckBox->isChecked() ? "1" : "0";
			gConfig.SetOption(CP_SKYBOX_ENABLED, optionValue.c_str());
			gConfig.SaveOptions();
		}
		break;

	case IDC_HITBOXES:
		{
			mxCheckBox* pCheckBox = reinterpret_cast<mxCheckBox*>(pEvent->widget);
			vs.showhitboxes = pCheckBox->isChecked() ? true : false;
		}
		break;
	case IDC_BONES:
		{
			mxCheckBox* pCheckBox = reinterpret_cast<mxCheckBox*>(pEvent->widget);
			vs.showbones = pCheckBox->isChecked() ? true : false;
		}
		break;
	case IDC_ATTACHMENTS:
		{
			mxCheckBox* pCheckBox = reinterpret_cast<mxCheckBox*>(pEvent->widget);
			vs.showattachments = pCheckBox->isChecked() ? true : false;
		}
		break;
	case IDC_MESHGROUPS:
		{
			mxCheckBox* pCheckBox = reinterpret_cast<mxCheckBox*>(pEvent->widget);
			vs.showmeshes = pCheckBox->isChecked() ? true : false;
		}
		break;
	case IDC_WIREFRAMEOVERLAY:
		{
			mxCheckBox* pCheckBox = reinterpret_cast<mxCheckBox*>(pEvent->widget);
			vs.wireframeoverlay = pCheckBox->isChecked() ? true : false;
		}
		break;
	case IDC_VIEWMODEL:
		{
			mxCheckBox* pCheckBox = reinterpret_cast<mxCheckBox*>(pEvent->widget);
			vs.viewmodel = pCheckBox->isChecked() ? true : false;
		}
		break;
	case IDC_LARGEFOV:
		{
			mxCheckBox* pCheckBox = reinterpret_cast<mxCheckBox*>(pEvent->widget);
			vs.largefov = pCheckBox->isChecked() ? true : false;
		}
		break;
	case IDC_FLEX_LOOP:
		{
			mxCheckBox* pCheckBox = reinterpret_cast<mxCheckBox*>(pEvent->widget);
			if(pCheckBox->isChecked())
				vs.flexscript.flags |= FLEX_FLAG_LOOP;
			else
				vs.flexscript.flags &= ~FLEX_FLAG_LOOP;
		}
		break;
	case IDC_FLEX_STAY:
		{
			mxCheckBox* pCheckBox = reinterpret_cast<mxCheckBox*>(pEvent->widget);
			if(pCheckBox->isChecked())
				vs.flexscript.flags |= FLEX_FLAG_STAY;
			else
				vs.flexscript.flags &= ~FLEX_FLAG_STAY;
		}
		break;
	case IDC_SEQUENCE:
		{
			Int32 index = m_pChoiceSequence->getSelectedIndex();
			if(index >= 0)
				SetSequence(index);
		}
		break;
	case IDC_SPEEDSCALE:
		{
			mxSlider* pSlider = reinterpret_cast<mxSlider*>(pEvent->widget);
			vs.speedscale = (float) (pSlider->getValue() * 5) / 200.0f;
		}
		break;
	case IDC_STOP:
		{
			if (m_pToggleButtonStop->isChecked ())
			{
				m_pToggleButtonStop->setLabel ("Play");
				vs.stopplaying = true;

				CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
				vs.curframe = pVBMRenderer->SetFrame(-1);

				CString str;
				str << (Int32)vs.curframe;
				m_pLineEditFrame->setLabel(str.c_str());

				m_pButtonPrevFrame->setEnabled (true);
				m_pLineEditFrame->setEnabled (true);
				m_pButtonNextFrame->setEnabled (true);
			}
			else
			{
				m_pToggleButtonStop->setLabel ("Stop");
				vs.stopplaying = false;
				m_pButtonPrevFrame->setEnabled (false);
				m_pLineEditFrame->setEnabled (false);
				m_pButtonNextFrame->setEnabled (false);
			}
		}
		break;
	case IDC_PREVFRAME:
		{
			CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
			vs.curframe = pVBMRenderer->SetFrame(vs.curframe - 1);

			CString str;
			str << (Int32)vs.curframe;
			m_pLineEditFrame->setLabel(str.c_str());
		}
		break;
	case IDC_FRAME:
		{
			vs.curframe = SDL_atoi(m_pLineEditFrame->getLabel());
			CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
			vs.curframe = pVBMRenderer->SetFrame(vs.curframe);
		}
		break;
	case IDC_NEXTFRAME:
		{
			CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
			vs.curframe = pVBMRenderer->SetFrame (vs.curframe + 1);

			CString str;
			str << (Int32)vs.curframe;
			m_pLineEditFrame->setLabel(str.c_str());
		}
		break;
	case IDC_MATERIAL:
		{
			m_currentMaterialIndex = m_pChoiceMaterial->getSelectedIndex();
			if(m_currentMaterialIndex >= m_materialNamesArray.size())
				m_currentMaterialIndex = 0;
		}
		break;
	case IDC_BODYPART:
		{
			Int32 index = m_pChoiceBodypart->getSelectedIndex();
			if (index >= 0)
				SetBodypart(index);
		}
		break;
	case IDC_SUBMODEL:
		{
			Int32 index = m_pChoiceSubmodel->getSelectedIndex();
			if (index >= 0)
				SetSubmodel(index);
		}
		break;
	case IDC_CONTROLLER:
		{
			Int32 index = m_pChoiceController->getSelectedIndex();
			if (index >= 0)
				SetBoneController(index);
		}
		break;
	case IDC_CONTROLLERVALUE:
		{
			Int32 index = m_pChoiceController->getSelectedIndex ();
			if (index >= 0)
				SetBoneControllerValue(index, m_pSliderController->getValue());
		}
		break;
	case IDC_SKINS:
		{
			Int32 index = m_pChoiceSkin->getSelectedIndex ();
			if (index >= 0)
			{
				CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
				if(pVBMRenderer)
				{
					vs.skin = index;
					pVBMRenderer->SetSkin(vs.skin);
					
					CGLWindow* pGLWindow = CGLWindow::GetInstance();
					if(pGLWindow)
						pGLWindow->redraw();
				}
			}
		}
		break;
	case IDC_TEXTURES:
		{
			Int32 index = m_pChoiceTextures->getSelectedIndex();
			if (index >= 0)
			{
				// Set texture index
				vs.texture = index;

				if(vs.pvbmheader)
				{
					const vbmtexture_t* ptexture = vs.pvbmheader->getTexture(index);
					InitTextureFlags(ptexture);
				}

				CGLWindow* pGLWindow = CGLWindow::GetInstance();
				if(pGLWindow)
					pGLWindow->redraw();
			}
		}
	break;
	case IDC_CHROME:
			SetTextureFlag( m_pCheckBoxChrome, vs.texture, MDLV_TF_CHROME );
		break;
	case IDC_ADDITIVE:
			SetTextureFlag( m_pCheckBoxAdditive, vs.texture, MDLV_TF_ADDITIVE );
		break;
	case IDC_ALPHATEST:
			SetTextureFlag( m_pCheckBoxAlphaTest, vs.texture, MDLV_TF_ALPHATEST );
		break;
	case IDC_UNLIT:
			SetTextureFlag( m_pCheckBoxUnlit, vs.texture, MDLV_TF_FULLBRIGHT );
		break;
	case IDC_NOMIPMAPS:
			SetTextureFlag( m_pCheckBoxNoMipmaps, vs.texture, MDLV_TF_NO_MIPMAP );
		break;
	case IDC_TRANSPARENT:
			SetTextureFlag( m_pCheckBoxTransparent, vs.texture, MDLV_TF_ALPHABLEND );
		break;
	case IDC_SCOPE:
			SetTextureFlag( m_pCheckBoxScope, vs.texture, MDLV_TF_SCOPE );
		break;
	case IDC_EYEGLINT:
			SetTextureFlag( m_pCheckBoxEyeGlint, vs.texture, MDLV_TF_EYEGLINT );
		break;
	case IDC_NO_DECAL:
			SetTextureFlag( m_pCheckBoxNoDecal, vs.texture, MDLV_TF_NO_DECAL );
		break;
	case IDC_NO_FACE_CULL:
			SetTextureFlag( m_pCheckBoxNoCulling, vs.texture, MDLV_TF_NOCULL );
		break;
	case IDC_TRANSPARENCY_SCALE:
		{
			if (vs.pvbmheader)
			{
				mxSlider* pSlider = reinterpret_cast<mxSlider*>(pEvent->widget);
				vbmtexture_t* ptexture = vs.pvbmheader->getTexture(vs.texture);
				en_material_t* pmaterial = CTextureManager::GetInstance()->FindMaterialScriptByIndex(ptexture->index);
				if(pmaterial)
					pmaterial->alpha = pSlider->getValue() / 100.0f;
			}
		}
		break;
	case IDC_TEXTURESCALE:
		{
			mxSlider* pSlider = reinterpret_cast<mxSlider*>(pEvent->widget);
			vs.texturescale =  1.0f + pSlider->getValue() * 4.0f / 100.0f;

			CString text;
			text << "Magnification: " << vs.texturescale;

			m_pLabelTexScale->setLabel(text.c_str());

			CGLWindow* pGLWindow = CGLWindow::GetInstance();
			if(pGLWindow)
				pGLWindow->redraw();
		}
		break;
	case IDC_SCRIPTFLEX_CHOICE:
		{
			mxChoice* pChoice = reinterpret_cast<mxChoice*>(pEvent->widget);
			vs.flexindex = pChoice->getSelectedIndex();
		}
		break;
	case IDC_SCRIPTFLEX_STRENGTH:
		{
			mxSlider* pSlider = reinterpret_cast<mxSlider*>(pEvent->widget);
			vs.flexvalue = pSlider->getValue();
		}
		break;
	case IDC_FLEX_WAVBUTTON:
		{
			LoadWAVFile();
		}
		break;
	case IDC_FLEX_PLAYBACK:
	{
			if(vs.wavinfo.length)
			{
				vs.wavplayback = true;
				PlaySound(vs.wavinfo.filepath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
			}

			vs.scriptplayback = true;
			vs.playbacktime = Viewer_FloatTime();
			vs.flexstate.time = Viewer_FloatTime();
		}
		break;
	case IDC_FLEX_STOP:
		{
			ScriptPlaybackStop();
		}
		break;
	case IDC_FLEX_RESET_SCRIPT:
		{
			ResetScript();
		}
		break;
	case IDC_FLEX_BIND:
		{
			BindFlex(vs.timeposition, vs.flexvalue);
		}
		break;
	case IDC_FLEX_LENGTH:
		{
			mxLineEdit* pLineEdit = reinterpret_cast<mxLineEdit*>(pEvent->widget);
			vs.scripttimelength = SDL_atof(pLineEdit->getLabel());
		}
		break;
	case IDC_FLEX_DELETE:
		{
			DeleteBind( vs.timeposition );
		}
		break;
	case IDC_FLEX_LOAD_SCRIPT:
		{
			LoadFlexScript();
		}
		break;
	case IDC_FLEX_SAVE_SCRIPT:
		{
			SaveFlexScript();
		}
		break;
	default:
		{
			return 1;
		}
		break;
	}

	return 0;
}

//=============================================
// @brief Loads a WAV file
//
//=============================================
void CControlPanel::LoadWAVFile( void )
{
	const Char *pstrLastWavPath = gConfig.GetOptionValue(CP_LAST_WAV_PATH);
	const Char *pstrFilePath = mxGetOpenFileName(this, pstrLastWavPath, "*.wav");
	if (!pstrFilePath)
		return;

	// Get dir path
	CString dirPath;
	Viewer_GetDirectoryPath(pstrFilePath, dirPath);
	gConfig.SetOption(CP_LAST_WAV_PATH, dirPath.c_str());

	if(vs.wavinfo.pdata)
	{
		delete [] vs.wavinfo.pdata;
		memset(&vs.wavinfo, 0, sizeof(sound_info_t));
	}

	Uint32 fileSize = 0;
	const byte* pFile = FL_LoadFile(pstrFilePath, &fileSize);
	if(!pFile)
	{
		mxMessageBox(this, "Error loading wave file.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return;
	}

	// Set WAV Path
	vs.wavinfo.filepath = pstrFilePath;

	const byte *pbegin = reinterpret_cast<const byte*>(pFile + 12);
	const byte *pend = reinterpret_cast<const byte*>(pFile + fileSize);

	while(1)
	{
		if(pbegin >= pend)
			break;

		DWORD ilength = Common::ByteToInt32(pbegin+4);
		Common::ScaleByte(&ilength);

		if(!strncmp((const char*)pbegin, "fmt ", 4))
		{
			vs.wavinfo.numchannels = Common::ByteToUint16(pbegin+10);
			vs.wavinfo.samplerate = Common::ByteToInt32(pbegin+12);
			vs.wavinfo.bitspersample = Common::ByteToUint16(pbegin+22);
		}

		if(!strncmp((const char*)pbegin, "data", 4))
		{
			vs.wavinfo.dataoffs = (pbegin+8)-pFile;
			vs.wavinfo.length = Common::ByteToInt32(pbegin+4);
		}

		pbegin = pbegin + 8 + ilength;
	}

	if(!vs.wavinfo.length || !vs.wavinfo.dataoffs)
	{
		mxMessageBox(this, "Error loading wave file.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		vs.wavinfo = sound_info_t();
		FL_FreeFile(pFile);
		return;
	}

	vs.wavinfo.pdata = new byte[vs.wavinfo.length];

	const byte *psrcdata = pFile + vs.wavinfo.dataoffs;
	memcpy(vs.wavinfo.pdata, psrcdata, vs.wavinfo.length);

	// Estimate the duration
	Int32 bytepersec = vs.wavinfo.numchannels * (vs.wavinfo.samplerate) * (vs.wavinfo.bitspersample>>3);
	Float length = (float)vs.wavinfo.length/(float)bytepersec;

	vs.timeposition = 0;

	m_pLabelWAVLengthLabel->setLabel("Length: %.2fs", length);
	m_pLabelFlexTimeLabel->setLabel("Time: %.2fs", vs.timeposition);

	if(SDL_atof(m_pLineEditLength->getLabel()) < length)
	{
		m_pLineEditLength->setLabel("%.2f", length);
		vs.scripttimelength = length;
	}

	FL_FreeFile(pFile);
}

//=============================================
// @brief Loads a flex script
//
//=============================================
void CControlPanel::LoadFlexScript( void )
{
	const Char *pstrLastScriptPath = gConfig.GetOptionValue(CP_LAST_SCRIPT_PATH);
	const Char *pstrFilePath = mxGetOpenFileName(this, pstrLastScriptPath, "*.ccs");
	if (!pstrFilePath)
	{
		mxMessageBox(this, "Error loading ccs file.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return;
	}

	// Remember previous path used
	CString path;
	Viewer_GetDirectoryPath(pstrFilePath, path);
	gConfig.SetOption(CP_LAST_SCRIPT_PATH, path.c_str());

	// Load the script
	const flexscript_t* pscript = vs.pflexmanager->LoadScript(pstrFilePath);
	if(!pscript)
		return;

	// Clear previous flex states
	if(!vs.flexscript.controllers.empty())
		vs.flexscript.controllers.clear();

	// Copy script information
	if(!pscript->controllers.empty())
	{
		vs.flexscript.controllers.resize(pscript->controllers.size());

		for(Uint32 i = 0; i < pscript->controllers.size(); i++)
			vs.flexscript.controllers[i] = pscript->controllers[i];

		vs.flexscript.flags = pscript->flags;
		vs.flexscript.filename = pscript->filename;
	}

	vs.flexscript.flags = pscript->flags;

	// Estimate the length of the script
	Float totalTime = 0;
	for(Uint32 i = 0; i < vs.flexscript.controllers.size(); i++)
	{
		for(Uint32 j = 0; j < vs.flexscript.controllers[i].binds.size(); j++)
		{
			if(vs.flexscript.controllers[i].binds[j].time > totalTime)
				totalTime = vs.flexscript.controllers[i].binds[j].time;
		}
	}

	if(vs.flexscript.flags & FLEX_FLAG_STAY)
		m_pCheckBoxFlexStay->setChecked(true);
	else
		m_pCheckBoxFlexStay->setChecked(false);

	if(vs.flexscript.flags & FLEX_FLAG_LOOP)
		m_pCheckBoxFlexLoop->setChecked(true);
	else
		m_pCheckBoxFlexLoop->setChecked(false);

	// Set flexstate pointer
	vs.flexstate.pscript = &vs.flexscript;

	if(vs.pvbmheader)
		vs.pflexmanager->SetFlexMappings(vs.pvbmheader, &vs.flexstate);

	if(vs.scripttimelength < totalTime)
		vs.scripttimelength = totalTime;

	m_pLineEditLength->setLabel("%.2f", vs.scripttimelength);
}

//=============================================
// @brief Saves flex information to a file
//
//=============================================
void CControlPanel::InitTextureFlags( const vbmtexture_t* pvbmtexture )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(pvbmtexture->index);
	if(!pmaterial)
		return;

	const en_texture_t* ptexture = pmaterial->getdiffuse();
	if(!ptexture)
		return;

	CString size;
	size << "Size: " << ptexture->width << " x " << ptexture->height;
	m_pLabelTexSize->setLabel(size.c_str());

	m_pCheckBoxChrome->setChecked((pmaterial->flags & TX_FL_CHROME) ? true : false);
	m_pCheckBoxAdditive->setChecked((pmaterial->flags & TX_FL_ADDITIVE) ? true : false);
	m_pCheckBoxUnlit->setChecked((pmaterial->flags & TX_FL_FULLBRIGHT) ? true : false);
	m_pCheckBoxNoMipmaps->setChecked((pmaterial->flags & TX_FL_NOMIPMAPS) ? true : false);
	m_pCheckBoxAlphaTest->setChecked((pmaterial->flags & TX_FL_ALPHATEST) ? true : false);
	m_pCheckBoxScope->setChecked((pmaterial->flags & TX_FL_SCOPE) ? true : false);
	m_pCheckBoxTransparent->setChecked((pmaterial->flags & TX_FL_ALPHABLEND) ? true : false);
	m_pCheckBoxEyeGlint->setChecked((pmaterial->flags & TX_FL_EYEGLINT) ? true : false);
	m_pCheckBoxNoDecal->setChecked((pmaterial->flags & TX_FL_NODECAL) ? true : false);
	m_pCheckBoxNoCulling->setChecked((pmaterial->flags & TX_FL_NO_CULLING) ? true : false);

	if(pmaterial->flags & TX_FL_ALPHABLEND)
	{
		m_pSliderTransSlider->setEnabled(true);
		m_pLabelTransSlider->setEnabled(true);
		m_pSliderTransSlider->setValue(pmaterial->alpha*100);
	}
	else
	{
		m_pSliderTransSlider->setEnabled(false);
		m_pLabelTransSlider->setEnabled(false);
		m_pSliderTransSlider->setValue(0);
	}
}

//=============================================
// @brief Saves flex information to a file
//
//=============================================
void CControlPanel::SaveFlexScript( void )
{
	const Char *pstrLastScriptPath = gConfig.GetOptionValue(CP_LAST_SCRIPT_PATH);
	const Char *pstrFilePath = mxGetSaveFileName(this, pstrLastScriptPath, "*.ccs");
	if (!pstrFilePath)
		return;

	// Remember previous path used
	CString path;
	Viewer_GetDirectoryPath(pstrFilePath, path);
	gConfig.SetOption(CP_LAST_SCRIPT_PATH, path.c_str());

	if(path.find(0, ".ccs") == -1)
		path << ".ccs";

	FILE *pf = fopen(pstrFilePath, "w");
	if(!pf)
	{
		mxMessageBox (0, "Error saving script file.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return;
	}

	fprintf(pf, "{\n" );
	if(vs.flexscript.flags)
	{
		fprintf(pf, "\tflags");

		if(vs.flexscript.flags & FLEX_FLAG_LOOP)
			fprintf(pf, " looped");

		if(vs.flexscript.flags & FLEX_FLAG_STAY)
			fprintf(pf, " stay");

		fprintf(pf, "\n");
	}

	for(Uint32 i = 0; i < vs.flexscript.controllers.size(); i++)
	{
		const flexcontroller_t* pcontroller = &vs.flexscript.controllers[i];
		if(pcontroller->binds.empty())
			continue;

		fprintf(pf, "\t{\n" );
		fprintf(pf, "\t\tname %s\n",  pcontroller->name.c_str());

		for(Uint32 j = 0; j < pcontroller->binds.size(); j++)
		{
			fprintf(pf, "\t\t{\n" );
			fprintf(pf, "\t\t\ttime %f\n", pcontroller->binds[j].time);
			fprintf(pf, "\t\t\tstrength %f\n", pcontroller->binds[j].strength);
			fprintf(pf, "\t\t}\n" );
		}
		fprintf(pf, "\t}\n" );
	}
	fprintf(pf, "}\n" );
	fclose(pf);
}

//=============================================
// @brief Deletes a flex bind
//
//=============================================
void CControlPanel::DeleteBind( Float position )
{
	if(!vs.scripttimelength)
		return;

	Int32 flexIndex = vs.flexindex;

	flexcontroller_t *pcontroller = NULL;
	Uint32 i = 0;
	for(; i < vs.flexscript.controllers.size(); i++)
	{
		if(vs.flexscript.controllers[i].index == flexIndex)
		{
			pcontroller = &vs.flexscript.controllers[i];
			break;
		}
	}

	if(!pcontroller)
		return;

	Float selectionSize = (vs.scripttimelength/10.0f);
	Float bindTime = position*vs.scripttimelength;

	for(Uint32 i = 0; i < pcontroller->binds.size(); i++)
	{
		Float secondsDist = abs(bindTime - pcontroller->binds[i].time);
		if(secondsDist <= FLEX_DELETION_SENSITIVITY*selectionSize)
		{
			pcontroller->binds.erase(i);
			return;
		}
	}
}

//=============================================
// @brief Resets a script completely
//
//=============================================
void CControlPanel::ResetScript( void )
{
	Int32 confirmResult = mxMessageBox(this, "This will erase all your binds. Confirm?", "Reset Script", MX_MB_YESNO);
	if(confirmResult)
		return;

	if(!vs.flexscript.controllers.empty())
		vs.flexscript.controllers.clear();

	vs.flexscript.duration = 0;
	vs.flexscript.filename.clear();
	vs.flexscript.flags = 0;
}

//=============================================
// @brief Binds a flex keyvalue
//
//=============================================
void CControlPanel::BindFlex( Float position, Float strength )
{
	if(!vs.scripttimelength || !vs.pvbmheader)
		return;

	Int32 flexIndex = vs.flexindex;
	const Char* pstrname = m_pLabelFlexNames[flexIndex]->getLabel();

	flexcontroller_t *pcontroller = nullptr;

	Uint32 i = 0;
	for(; i < vs.flexscript.controllers.size(); i++)
	{
		if(!qstrcmp(vs.flexscript.controllers[i].name, pstrname))
		{
			pcontroller = &vs.flexscript.controllers[i];
			break;
		}
	}

	if(!pcontroller)
	{
		vs.flexscript.controllers.resize(vs.flexscript.controllers.size()+1);
		pcontroller = &vs.flexscript.controllers[vs.flexscript.controllers.size()-1];

		vs.pflexmanager->SetControllerName(&vs.flexscript, pcontroller, pstrname);
		vs.pflexmanager->SetFlexMappings(vs.pvbmheader, &vs.flexstate);
	}

	// TODO: Make this custom specifiable
	Float selectionSize = (vs.scripttimelength/10.0f);
	Float bindTime = position*vs.scripttimelength;

	// Try binding over
	i = 0;
	flexbind_t* pbind = nullptr;
	for(; i < pcontroller->binds.size(); i++)
	{
		Float secondsDist = SDL_fabs(bindTime - pcontroller->binds[i].time);
		if(secondsDist <= FLEX_DELETION_SENSITIVITY*selectionSize)
		{
			pcontroller->binds[i].strength = strength;
			pcontroller->binds[i].time = bindTime;
			return;
		}
	}

	// Check if we need to resize
	i = 0;
	for(;i < pcontroller->binds.size(); i++)
	{
		if(pcontroller->binds[i].time > bindTime)
			break;
	}

	Uint32 prevsize = pcontroller->binds.size();
	pcontroller->binds.resize(prevsize+1);

	if(i != prevsize)
	{
		for(Uint32 j = prevsize; j > i; j--)
			pcontroller->binds[j] = pcontroller->binds[j-1];

		pbind = &pcontroller->binds[i];
	}
	else
		pbind = &pcontroller->binds[prevsize];

	pbind->strength = strength;
	pbind->time = bindTime;
}

//=============================================
// @brief Dumps model info to a text window
//
//=============================================
void CControlPanel::DumpModelInfo( void )
{
#if 0
	studiohdr_t *hdr = g_studioModel.getStudioHeader ();
	if (hdr)
	{
		DeleteFile ("midump.txt");
		FILE *file = fopen ("midump.txt", "wt");
		if (file)
		{
			byte *phdr = (byte *) hdr;
			int i;

			fprintf (file, "id: %c%c%c%c\n", phdr[0], phdr[1], phdr[2], phdr[3]);
			fprintf (file, "version: %d\n", hdr->version);
			fprintf (file, "name: \"%s\"\n", hdr->name);
			fprintf (file, "length: %d\n\n", hdr->length);

			fprintf (file, "eyeposition: %f %f %f\n", hdr->eyeposition[0], hdr->eyeposition[1], hdr->eyeposition[2]);
			fprintf (file, "min: %f %f %f\n", hdr->min[0], hdr->min[1], hdr->min[2]);
			fprintf (file, "max: %f %f %f\n", hdr->max[0], hdr->max[1], hdr->max[2]);
			fprintf (file, "bbmin: %f %f %f\n", hdr->bbmin[0], hdr->bbmin[1], hdr->bbmin[2]);
			fprintf (file, "bbmax: %f %f %f\n", hdr->bbmax[0], hdr->bbmax[1], hdr->bbmax[2]);
			
			fprintf (file, "flags: %d\n\n", hdr->flags);

			fprintf (file, "numbones: %d\n", hdr->numbones);
			for (i = 0; i < hdr->numbones; i++)
			{
				mstudiobone_t *pbones = (mstudiobone_t *) (phdr + hdr->boneindex);
				fprintf (file, "\nbone %d.name: \"%s\"\n", i + 1, pbones[i].name);
				fprintf (file, "bone %d.parent: %d\n", i + 1, pbones[i].parent);
				fprintf (file, "bone %d.flags: %d\n", i + 1, pbones[i].flags);
				fprintf (file, "bone %d.bonecontroller: %d %d %d %d %d %d\n", i + 1, pbones[i].bonecontroller[0], pbones[i].bonecontroller[1], pbones[i].bonecontroller[2], pbones[i].bonecontroller[3], pbones[i].bonecontroller[4], pbones[i].bonecontroller[5]);
				fprintf (file, "bone %d.value: %f %f %f %f %f %f\n", i + 1, pbones[i].value[0], pbones[i].value[1], pbones[i].value[2], pbones[i].value[3], pbones[i].value[4], pbones[i].value[5]);
				fprintf (file, "bone %d.scale: %f %f %f %f %f %f\n", i + 1, pbones[i].scale[0], pbones[i].scale[1], pbones[i].scale[2], pbones[i].scale[3], pbones[i].scale[4], pbones[i].scale[5]);
			}

			fprintf (file, "\nnumbonecontrollers: %d\n", hdr->numbonecontrollers);
			for (i = 0; i < hdr->numbonecontrollers; i++)
			{
				mstudiobonecontroller_t *pbonecontrollers = (mstudiobonecontroller_t *) (phdr + hdr->bonecontrollerindex);
				fprintf (file, "\nbonecontroller %d.bone: %d\n", i + 1, pbonecontrollers[i].bone);
				fprintf (file, "bonecontroller %d.type: %d\n", i + 1, pbonecontrollers[i].type);
				fprintf (file, "bonecontroller %d.start: %f\n", i + 1, pbonecontrollers[i].start);
				fprintf (file, "bonecontroller %d.end: %f\n", i + 1, pbonecontrollers[i].end);
				fprintf (file, "bonecontroller %d.rest: %d\n", i + 1, pbonecontrollers[i].rest);
				fprintf (file, "bonecontroller %d.index: %d\n", i + 1, pbonecontrollers[i].index);
			}

			fprintf (file, "\nnumhitboxes: %d\n", hdr->numhitboxes);
			for (i = 0; i < hdr->numhitboxes; i++)
			{
				mstudiobbox_t *pbboxes = (mstudiobbox_t *) (phdr + hdr->hitboxindex);
				fprintf (file, "\nhitbox %d.bone: %d\n", i + 1, pbboxes[i].bone);
				fprintf (file, "hitbox %d.group: %d\n", i + 1, pbboxes[i].group);
				fprintf (file, "hitbox %d.bbmin: %f %f %f\n", i + 1, pbboxes[i].bbmin[0], pbboxes[i].bbmin[1], pbboxes[i].bbmin[2]);
				fprintf (file, "hitbox %d.bbmax: %f %f %f\n", i + 1, pbboxes[i].bbmax[0], pbboxes[i].bbmax[1], pbboxes[i].bbmax[2]);
			}

			fprintf (file, "\nnumseq: %d\n", hdr->numseq);
			for (i = 0; i < hdr->numseq; i++)
			{
				mstudioseqdesc_t *pseqdescs = (mstudioseqdesc_t *) (phdr + hdr->seqindex);
				fprintf (file, "\nseqdesc %d.label: \"%s\"\n", i + 1, pseqdescs[i].label);
				fprintf (file, "seqdesc %d.fps: %f\n", i + 1, pseqdescs[i].fps);
				fprintf (file, "seqdesc %d.flags: %d\n", i + 1, pseqdescs[i].flags);
				fprintf (file, "<...>\n");
			}
/*
			fprintf (file, "\nnumseqgroups: %d\n", hdr->numseqgroups);
			for (i = 0; i < hdr->numseqgroups; i++)
			{
				mstudioseqgroup_t *pseqgroups = (mstudioseqgroup_t *) (phdr + hdr->seqgroupindex);
				fprintf (file, "\nseqgroup %d.label: \"%s\"\n", i + 1, pseqgroups[i].label);
				fprintf (file, "\nseqgroup %d.namel: \"%s\"\n", i + 1, pseqgroups[i].name);
				fprintf (file, "\nseqgroup %d.data: %d\n", i + 1, pseqgroups[i].data);
			}
*/
			hdr = g_studioModel.getTextureHeader ();
			fprintf (file, "\nnumtextures: %d\n", hdr->numtextures);
			fprintf (file, "textureindex: %d\n", hdr->textureindex);
			fprintf (file, "texturedataindex: %d\n", hdr->texturedataindex);
			for (i = 0; i < hdr->numtextures; i++)
			{
				mstudiotexture_t *ptextures = (mstudiotexture_t *) ((byte *) hdr + hdr->textureindex);
				fprintf (file, "\ntexture %d.name: \"%s\"\n", i + 1, ptextures[i].name);
				fprintf (file, "texture %d.flags: %d\n", i + 1, ptextures[i].flags);
				fprintf (file, "texture %d.width: %d\n", i + 1, ptextures[i].width);
				fprintf (file, "texture %d.height: %d\n", i + 1, ptextures[i].height);
				fprintf (file, "texture %d.index: %d\n", i + 1, ptextures[i].index);
			}

			hdr = g_studioModel.getStudioHeader ();
			fprintf (file, "\nnumskinref: %d\n", hdr->numskinref);
			fprintf (file, "numskinfamilies: %d\n", hdr->numskinfamilies);

			fprintf (file, "\nnumbodyparts: %d\n", hdr->numbodyparts);
			for (i = 0; i < hdr->numbodyparts; i++)
			{
				mstudiobodyparts_t *pbodyparts = (mstudiobodyparts_t *) ((byte *) hdr + hdr->bodypartindex);
				fprintf (file, "\nbodypart %d.name: \"%s\"\n", i + 1, pbodyparts[i].name);
				fprintf (file, "bodypart %d.nummodels: %d\n", i + 1, pbodyparts[i].nummodels);
				fprintf (file, "bodypart %d.base: %d\n", i + 1, pbodyparts[i].base);
				fprintf (file, "bodypart %d.modelindex: %d\n", i + 1, pbodyparts[i].modelindex);
			}

			fprintf (file, "\nnumattachments: %d\n", hdr->numattachments);
			for (i = 0; i < hdr->numattachments; i++)
			{
				mstudioattachment_t *pattachments = (mstudioattachment_t *) ((byte *) hdr + hdr->attachmentindex);
				fprintf (file, "attachment %d.name: \"%s\"\n", i + 1, pattachments[i].name);
			}

			fclose (file);

			ShellExecute ((HWND) getHandle (), "open", "midump.txt", 0, 0, SW_SHOW);
		}
	}
#endif
}

//=============================================
// @brief Sets the viewer up with the model info loaded
//
//=============================================
void CControlPanel::SetupViewerForModel( const Char* pstrFilename )
{
	SetFlexNames();
	InitSequences();
	InitBodyparts();
	InitBoneControllers();
	InitSkins();
	InitTextures();
	CenterView();

	vs.modelfile = pstrFilename;

	vs.sequence = 0;
	vs.speedscale = 1.0f;
	m_pSliderSpeedScale->setValue (40);

	// Set Submodels
	for (Int32 i = 0; i < MAXSTUDIOMODELS; i++)
		vs.submodels[i] = 0;
	
	// Set controllers
	for (Int32 i = 0; i < MAXSTUDIOCONTROLLERS; i++)
		vs.controllers[i] = 0;

	// Set path for mx
	mx_setcwd(mx_getpath(pstrFilename));
}

//=============================================
// @brief Loads a VBM model
//
//=============================================
void CControlPanel::LoadModel( const Char* pstrFilename )
{
	// Load the model itself
	if(!Viewer_LoadModel(pstrFilename))
	{
		Viewer_ReleaseModel();
		return;
	}

	SetModelInfo();
}

//=============================================
// @brief Sets render mode
//
//=============================================
void CControlPanel::SetRenderMode( mv_rendermodes_t mode )
{
	vs.rendermode = mode;

	CGLWindow* pGLWindow = CGLWindow::GetInstance();
	if(pGLWindow)
		pGLWindow->redraw();
}

//=============================================
// @brief Sets wherhet ground should be drawn
//
//=============================================
void CControlPanel::SetShowGround( bool showGround )
{
	vs.showground = showGround;
	m_pCheckBoxGround->setChecked(showGround);
	
	if (!showGround)
	{
		m_pCheckBoxMirror->setChecked(showGround);
		vs.mirror = showGround;
	}
}

//=============================================
// @brief Sets whether model should be mirrored
//
//=============================================
void CControlPanel::SetMirror( bool showMirror )
{
	vs.usestencil = showMirror;
	vs.mirror = showMirror;

	m_pCheckBoxMirror->setChecked (showMirror);

	if(showMirror)
	{
		m_pCheckBoxGround->setChecked(showMirror);
		vs.showground = showMirror;
	}
}

//=============================================
// @brief Sets whether skybox should be shown
//
//=============================================
void CControlPanel::SetShowSkybox( bool showSkybox )
{
	vs.showskybox = showSkybox;
	m_pCheckBoxSkybox->setChecked(showSkybox);
}

//=============================================
// @brief Initializes sequences list
//
//=============================================
void CControlPanel::InitSequences( void )
{
	if (!vs.pstudioheader)
		return;

	m_pChoiceSequence->removeAll();

	for (Int32 i = 0; i < vs.pstudioheader->numseq; i++)
	{
		const mstudioseqdesc_t* pseqdesc = vs.pstudioheader->getSequence(i);
		CString name;
		name << i << " - " << pseqdesc->label;

		m_pChoiceSequence->add(name.c_str());
	}

	SetSequence (0);
}

//=============================================
// @brief Sets current sequence and it's information
//
//=============================================
void CControlPanel::SetSequence( Int32 index )
{
	m_pChoiceSequence->select (index);
	vs.sequence = index;

	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
	if(!pVBMRenderer)
		return;

	pVBMRenderer->SetSequence(index);

	Float framerate;
	Int32 numFrames;
	Float groundSpeed;
	pVBMRenderer->GetSequenceInfo(framerate, numFrames, groundSpeed);

	m_pLabelSequenceIndex->setLabel( "Sequence index: %d", index );
	m_pLabelFrameCount->setLabel( "Frames: %d", numFrames );
	m_pLabelFramerate->setLabel( "FPS: %.3f", framerate );
	m_pLabelFrame->setLabel( "Frame: 0.00" );
}

//=============================================
// @brief Initializes bodyparts selector
//
//=============================================
void CControlPanel::InitBodyparts( void )
{
	if (!vs.pvbmheader)
		return;

	m_pChoiceBodypart->removeAll();
	if (!vs.pvbmheader->numbodyparts)
		return;

	// Fill bodyparts selector
	for (Int32 i = 0; i < vs.pvbmheader->numbodyparts; i++)
	{
		const vbmbodypart_t* pbodypart = vs.pvbmheader->getBodyPart(i);

		CString name;
		name << i << " - " << pbodypart->name;

		m_pChoiceBodypart->add(pbodypart->name);
	}

	m_pChoiceBodypart->select(0);

	// Fill submodel selector
	m_pChoiceSubmodel->removeAll();

	const vbmbodypart_t* firstpbodypart = vs.pvbmheader->getBodyPart(0);
	for (Int32 i = 0; i < firstpbodypart->numsubmodels; i++)
	{
		const vbmsubmodel_t* psubmodel = firstpbodypart->getSubmodel(vs.pvbmheader, i);

		CString submodelName;
		submodelName << i << " - " << psubmodel->name;

		m_pChoiceSubmodel->add(submodelName.c_str());
	}
	m_pChoiceSubmodel->select(0);
}

//=============================================
// @brief Sets current bodypart
//
//=============================================
void CControlPanel::SetBodypart( Int32 index )
{
	if (!vs.pvbmheader)
		return;

	m_pChoiceBodypart->select(index);

	// Remove all previous ones
	m_pChoiceSubmodel->removeAll();

	if (index < vs.pvbmheader->numbodyparts)
	{
		const vbmbodypart_t* firstpbodypart = vs.pvbmheader->getBodyPart(index);
		for (Int32 i = 0; i < firstpbodypart->numsubmodels; i++)
		{
			const vbmsubmodel_t* psubmodel = firstpbodypart->getSubmodel(vs.pvbmheader, i);

			CString submodelName;
			submodelName << i << " - ";
			if(!qstrlen(psubmodel->name))
				submodelName << "Blank";
			else
				submodelName << psubmodel->name;

			m_pChoiceSubmodel->add(submodelName.c_str());
		}

		m_pChoiceSubmodel->select(0);
	}

	SetModelInfo();
}

//=============================================
// @brief Sets current submodel
//
//=============================================
void CControlPanel::SetSubmodel( Int32 index )
{
	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
	if(!pVBMRenderer)
		return;

	Int32 selectIndex = m_pChoiceBodypart->getSelectedIndex();
	pVBMRenderer->SetBodyGroup(selectIndex, index);
	vs.submodels[selectIndex] = index;

	SetModelInfo();
}

//=============================================
// @brief Initializes bone controllers
//
//=============================================
void CControlPanel::InitBoneControllers( void )
{
	if(!vs.pstudioheader)
		return;

	if(vs.pstudioheader > 0)
	{
		m_pChoiceController->setEnabled(true);
		m_pSliderController->setEnabled(true);	
	}
	else
	{
		m_pChoiceController->setEnabled(false);
		m_pSliderController->setEnabled(false);
	}

	m_pChoiceController->removeAll();

	for(Int32 i = 0; i < vs.pstudioheader->numbonecontrollers; i++)
	{
		const mstudiobonecontroller_t* pcontroller = vs.pstudioheader->getBoneController(i);

		CString name;
		if (pcontroller->index == 4)
		{
			// Mouth is special
			name = "4 - Mouth";
		}
		else
		{
			const mstudiobone_t* pbone = vs.pstudioheader->getBone(pcontroller->bone);
			name << i << " - " << pbone->name;
		}

		m_pChoiceController->add(name.c_str());
	}

	if (vs.pstudioheader->numbonecontrollers > 0)
	{
		const mstudiobonecontroller_t* pcontroller = vs.pstudioheader->getBoneController(0);

		m_pChoiceController->select(0);
		m_pSliderController->setRange(pcontroller->start, pcontroller->end);
		m_pSliderController->setValue(0);
	}
}

//=============================================
// @brief Sets the current bone controller
//
//=============================================
void CControlPanel::SetBoneController( Int32 index )
{
	if(!vs.pstudioheader)
		return;

	const mstudiobonecontroller_t *pbonecontroller = vs.pstudioheader->getBoneController(index);
	if(!pbonecontroller)
		return;

	m_pSliderController->setRange(pbonecontroller->start, pbonecontroller->end);
	m_pSliderController->setValue(0);
}

//=============================================
// @brief Sets a bone controller's value
//
//=============================================
void CControlPanel::SetBoneControllerValue( Int32 index, Float value )
{
	if(!vs.pstudioheader)
		return;

	vs.controllers[index] = value;

	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
	if(!pVBMRenderer)
		return;

	const mstudiobonecontroller_t *pbonecontroller = vs.pstudioheader->getBoneController(index);
	if(!pbonecontroller)
		return;

	if (pbonecontroller->index == 4)
		pVBMRenderer->SetMouthOpen(value);
	else
		pVBMRenderer->SetController(pbonecontroller->index, value);
}

//=============================================
// @brief Initializes skin selector
//
//=============================================
void CControlPanel::InitSkins( void )
{
	if(!vs.pvbmheader)
		return;

	if(vs.pvbmheader->numskinfamilies > 0)
		m_pChoiceSkin->setEnabled(true);
	else
		m_pChoiceSkin->setEnabled(false);

	m_pChoiceSkin->removeAll();

	for (Int32 i = 0; i < vs.pvbmheader->numskinfamilies; i++)
	{
		CString name;
		name << "Skin " << i;
		m_pChoiceSkin->add(name.c_str());
	}

	m_pChoiceSkin->select(0);
	vs.skin = 0;

	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
	if(!pVBMRenderer)
		return;

	pVBMRenderer->SetSkin(0);
}

//=============================================
// @brief Sets model info labels
//
//=============================================
void CControlPanel::SetModelInfo( void )
{
	if(!vs.pstudioheader || !vs.pvbmheader)
		return;

	CString desc;
	desc << "Bones: " << vs.pstudioheader->numbones << "\n"
		<< "Bone Controllers: " << vs.pstudioheader->numbonecontrollers << "\n"
		<< "Hit Boxes: " << vs.pstudioheader->numhitboxes << "\n"
		<< "Sequences: " << vs.pstudioheader->numseq << "\n";

	m_pLabelModelInfo1->setLabel(desc.c_str());

	desc.clear();
	desc << "Textures: " << vs.pvbmheader->numtextures << "\n"
		<< "Skin Families: " << vs.pvbmheader->numskinfamilies << "\n"
		<< "Bodyparts: " << vs.pvbmheader->numbodyparts << "\n"
		<< "Attachments: " << vs.pstudioheader->numattachments << "\n"
		<< "Transitions: " << vs.pstudioheader->numtransitions << "\n";

	m_pLabelModelInfo2->setLabel(desc.c_str());

	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
	if(!pVBMRenderer)
		return;

	desc.clear(); // TODO: add Uint64 case for CString
	desc << "Body value: " << (Uint32)pVBMRenderer->GetBodyNum() << "\n";

	m_pLabelModelInfo3->setLabel (desc.c_str());
}

//==============================================
// Initializes textures tab
//
//==============================================
void CControlPanel::InitTextures( void )
{
	if(!vs.pvbmheader)
		return;

	m_pChoiceTextures->removeAll();

	for (Int32 i = 0; i < vs.pvbmheader->numtextures; i++)
	{
		vbmtexture_t* ptexture = vs.pvbmheader->getTexture(i);
		m_pChoiceTextures->add(ptexture->name);
	}

	m_pChoiceTextures->select (0);
	vs.texture = 0;

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// TODO: check all texflags for selected texture
	//if (hdr->numtextures > 0)
	//	m_cbChrome->setChecked ((ptextures[0].flags & STUDIO_NF_CHROME) == STUDIO_NF_CHROME);
}

//=============================================
// @brief Centers the view
//
//=============================================
void CControlPanel::CenterView( void )
{
	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
	if(!pVBMRenderer)
		return;

	Vector mins;
	Vector maxs;
	pVBMRenderer->GetSequenceBBox(mins, maxs);

	Float dx = maxs[0] - mins[0];
	Float dy = maxs[1] - mins[1];
	Float dz = maxs[2] - mins[2];
	Float d = dx;

	if (dy > d)
		d = dy;
	if (dz > d)
		d = dz;

	vs.v_translation[0] = 0;
	vs.v_translation[1] = mins[2] + dz / 2;
	vs.v_translation[2] = d * 1.0f;
	vs.v_rotation[0] = -90.0f;
	vs.v_rotation[1] = -90.0f;
	vs.v_rotation[2] = 0.0f;

	CGLWindow* pGLWindow = CGLWindow::GetInstance();
	if(pGLWindow)
		pGLWindow->redraw();
}

//=============================================
// @brief Updates specific labels each frame
//
//=============================================
void CControlPanel::Update( void )
{
	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
	if(!pVBMRenderer)
		return;

	m_pLabelFrame->setLabel("Frame: %.2f", pVBMRenderer->GetFrame());
}

//=============================================
// @brief Initializes the materials list
//
//=============================================
void CControlPanel::InitMaterialsList( CStepSound* pstepsound )
{
	ClearMaterialsList();

	Uint32 nbMaterials = pstepsound->GetNbMaterials();
	for(Uint32 i = 0; i < nbMaterials; i++)
	{
		CString materialName = pstepsound->GetMaterialByIndex(i)->materialname;
		Uint32 j = 0;
		for(; j < m_materialNamesArray.size(); j++)
		{
			if(!qstrcmp(m_materialNamesArray[j], materialName))
				break;
		}

		if(j == m_materialNamesArray.size())
		{
			m_materialNamesArray.push_back(materialName);
			m_pChoiceMaterial->add(materialName.c_str());
		}
	}

	m_currentMaterialIndex = 0;
	m_pChoiceMaterial->select(0);
}

//=============================================
// @brief Clears materials list
//
//=============================================
void CControlPanel::ClearMaterialsList( void )
{
	if(m_materialNamesArray.empty())
		return;

	m_materialNamesArray.clear();
	m_currentMaterialIndex = 0;

	m_pChoiceMaterial->removeAll();
	m_pChoiceMaterial->select(0);
}

//=============================================
// @brief Returns the currently selected material name
//
//=============================================
const Char* CControlPanel::GetCurrentMaterialName( void )
{
	if(m_currentMaterialIndex >= m_materialNamesArray.size())
		return nullptr;
	else
		return m_materialNamesArray[m_currentMaterialIndex].c_str();
}

//=============================================
// @brief Creates an instance of this class
//
// @return Created instance
//=============================================
CControlPanel* CControlPanel::CreateInstance( mxWindow* pParent )
{
	if(!g_pInstance)
		g_pInstance = new CControlPanel(pParent);

	return g_pInstance;
}

//=============================================
// @brief Returns the current instance of this class
//
// @return Current instance, or nullptr
//=============================================
CControlPanel* CControlPanel::GetInstance( void )
{
	return g_pInstance;
}

//=============================================
// @brief Deletes the current instance of this class
//
//=============================================
void CControlPanel::DeleteInstance( void )
{
	if(!g_pInstance)
		return;

	// TODO
	//delete g_pInstance;
	g_pInstance = nullptr;
}