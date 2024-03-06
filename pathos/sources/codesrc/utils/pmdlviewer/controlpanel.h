//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           ControlPanel.h
// last modified:  Oct 20 programs and associated files contained in this
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

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif

#include "includes.h"
#include "file.h"

class mxTab;
class mxChoice;
class mxCheckBox;
class mxSlider;
class mxLineEdit;
class mxLabel;
class mxButton;
class mxToggleButton;
class GlWindow;
class TextureWindow;

struct vbmtexture_t;

/*
=================================
CControlPanel

=================================
*/
class CControlPanel : public mxWindow
{
public:
	enum
	{
		IDC_TAB = 1901,
		IDC_RENDERMODE = 2001,
		IDC_TRANSPARENCY,
		IDC_GROUND,
		IDC_MIRROR,
		IDC_SKYBOX,
		IDC_HITBOXES,
		IDC_BONES,
		IDC_ATTACHMENTS,
		IDC_MESHGROUPS,
		IDC_WIREFRAMEOVERLAY,
		IDC_VIEWMODEL,
		IDC_LARGEFOV,

		IDC_SEQUENCE = 3001,
		IDC_SPEEDSCALE,
		IDC_STOP,
		IDC_PREVFRAME,
		IDC_FRAME,
		IDC_NEXTFRAME,
		IDC_MATERIAL,

		IDC_BODYPART = 4001,
		IDC_SUBMODEL,
		IDC_CONTROLLER,
		IDC_CONTROLLERVALUE,
		IDC_SKINS,

		IDC_TEXTURES = 5001,
		IDC_TEXTURESCALE,
		IDC_CHROME,
		IDC_ADDITIVE,
		IDC_ALPHATEST,
		IDC_UNLIT,
		IDC_TRANSPARENT,
		IDC_SCOPE,
		IDC_NOMIPMAPS,
		IDC_EYEGLINT,
		IDC_TRANSPARENCY_SCALE,
		IDC_NO_FACE_CULL,
		IDC_NO_DECAL,

		IDC_FLEXCONTROLLER_BASE = 6001,

		IDC_SCRIPTFLEX_CHOICE = 7001,
		IDC_SCRIPTFLEX_STRENGTH	,
		IDC_FLEX_WAVBUTTON,
		IDC_FLEX_PLAYBACK,
		IDC_FLEX_STOP,
		IDC_FLEX_BIND,
		IDC_FLEX_DELETE,
		IDC_FLEX_LOAD_SCRIPT,
		IDC_FLEX_SAVE_SCRIPT,
		IDC_FLEX_RESET_SCRIPT,
		IDC_FLEX_LENGTH,
		IDC_FLEX_LOOP,
		IDC_FLEX_STAY
	};

	enum textureflags_t
	{
		MDLV_TF_FULLBRIGHT = 0,
		MDLV_TF_NODECAL,
		MDLV_TF_ALPHATEST,
		MDLV_TF_CLAMP,
		MDLV_TF_EYEGLINT,
		MDLV_TF_CHROME,
		MDLV_TF_ADDITIVE,
		MDLV_TF_SCOPE,
		MDLV_TF_NOCULL,
		MDLV_TF_ALPHABLEND,
		MDLV_TF_NO_MIPMAP,
		MDLV_TF_NO_DECAL
	};

public:
	// Last WAV file path
	static const Char CP_LAST_WAV_PATH[];
	// Last script path
	static const Char CP_LAST_SCRIPT_PATH[];
	// Last env texture path
	static const Char CP_LAST_ENV_TEX_PATH[];
	// Last ground texture path
	static const Char CP_LAST_GROUND_TEX_PATH[];
	// Whether ground was enabled
	static const Char CP_GROUND_ENABLED[];
	// Whether mirroring was enabled
	static const Char CP_MIRROR_ENABLED[];
	// Whether skybox was enabled
	static const Char CP_SKYBOX_ENABLED[];

public:
	// Tab indexes
	enum tabindexes_t
	{
		TAB_INDEX_RENDER = 0,
		TAB_INDEX_SEQUENCE,
		TAB_INDEX_BODY,
		TAB_INDEX_TEXTURE,
		TAB_INDEX_FLEXES,
		TAB_INDEX_FLEX_SCRIPTING
	};

public:
	// Deletion sensitivity
	static const float FLEX_DELETION_SENSITIVITY;

private:
	CControlPanel( mxWindow *parent );
	virtual ~CControlPanel( void );

public:
	// Dumps info about the model to a text window
	void DumpModelInfo( void );
	// Loads a model
	void LoadModel( const Char *pstrFilename );

	// Sets the render mode
	void SetRenderMode( enum mv_rendermodes_t mode );
	// Sets whether ground should be shown
	void SetShowGround( bool showGround );
	// Sets whether the mirror should be shown
	void SetMirror( bool showMirror );
	// Sets whether the sky should be shown
	void SetShowSkybox( bool showSkybox );

	// Initializes the sequences tab
	void InitSequences( void );
	// Sets a sequence by index
	void SetSequence( Int32 index );

	// Initialzies bodyparts window
	void InitBodyparts( void );
	// Sets a body part
	void SetBodypart( Int32 index );
	// Sets a submodel
	void SetSubmodel( Int32 index );

	// Deletes a flex bind
	void DeleteBind( Float position );
	// Binds a flex at a given position
	void BindFlex( Float position, Float strength ); 
	// Resets the flex script
	void ResetScript( void );

	// Initializes bone controllers
	void InitBoneControllers( void );
	// Sets the current bone controller
	void SetBoneController( Int32 index );
	// Sets the bone controller's value
	void SetBoneControllerValue( Int32 index, Float value );

	// Initializes skins
	void InitSkins( void );
	// Sets a texture flag
	void SetTextureFlag( mxCheckBox* pBox, Int32 texture, textureflags_t flag );
	// Inits texture flags
	void InitTextureFlags( const vbmtexture_t* pvbmtexture );

	// Sets model info
	void SetModelInfo( void );
	// Initializes the textures tab
	void InitTextures( void );

	// Centers the view
	void CenterView( void );
	// Loads a WAV file
	void LoadWAVFile( void );

	// Loads a flex script
	void LoadFlexScript( void );
	// Saves the flex script to file
	void SaveFlexScript( void );

	// Sets the time position
	void SetTimePosition( float position, bool setCursor );
	// Stops script playback
	void ScriptPlaybackStop( void );
	// Updates the control panel
	void Update( void );

	// Sets flex names
	void SetFlexNames( void );
	// Clears flex names
	void ClearFlexNames( void );

	// Retrieves the flex label
	const Char* GetFlexLabel( Int32 index );

	// Sets the viewer up with the model info loaded
	void SetupViewerForModel( const Char* pstrFilename );

	// Initializes the materials list
	void InitMaterialsList( class CStepSound* pstepsound );
	// Clears materials list
	void ClearMaterialsList( void );
	// Returns the currently selected material name
	const Char* GetCurrentMaterialName( void );

public:
	// Handles an mx event
	virtual int handleEvent( mxEvent *pEvent ) override;

public:
	// Creates an instance of this class
	static CControlPanel* CreateInstance( mxWindow* pParent );
	// Returns the current instance of this class
	static CControlPanel* GetInstance( void );
	// Deletes the current instance of this class
	static void DeleteInstance( void );

private:
	// Initializes render tab
	void InitRenderTab( void );
	// Initializes sequence tab
	void InitSequenceTab( void );
	// Initializes body tab
	void InitBodyTab( void );
	// Initializes texture tab
	void InitTextureTab( void );
	// Initializes flexes tab
	void InitFlexesTab( void );
	// Initialzies flex scripting tab
	void InitFlexScriptingTab( void );

private:
	mxTab*			m_pTab;
	mxChoice*		m_pChoiceRenderMode;
	mxSlider*		m_pSliderTransparency;
	mxCheckBox*		m_pCheckBoxGround;
	mxCheckBox*		m_pCheckBoxMirror;
	mxCheckBox*		m_pCheckBoxSkybox;

	mxChoice*		m_pChoiceSequence;
	mxSlider*		m_pSliderSpeedScale;
	mxToggleButton*	m_pToggleButtonStop;
	mxButton*		m_pButtonPrevFrame;
	mxButton*		m_pButtonNextFrame;
	mxButton*		m_pButtonFlexPlay;
	mxChoice*		m_pChoiceMaterial;

	mxLineEdit*		m_pLineEditFrame;
	mxChoice*		m_pChoiceBodypart;
	mxChoice*		m_pChoiceController;
	mxChoice*		m_pChoiceSubmodel;

	mxSlider*		m_pSliderController;
	mxChoice*		m_pChoiceSkin;
	mxLabel*		m_pLabelModelInfo1;
	mxLabel*		m_pLabelModelInfo2;
	mxLabel*		m_pLabelModelInfo3;

	mxChoice*		m_pChoiceTextures;

	mxCheckBox*		m_pCheckBoxChrome;
	mxCheckBox*		m_pCheckBoxAdditive;
	mxCheckBox*		m_pCheckBoxAlphaTest;
	mxCheckBox*		m_pCheckBoxNoMipmaps;
	mxCheckBox*		m_pCheckBoxUnlit;
	mxCheckBox*		m_pCheckBoxScope;
	mxCheckBox*		m_pCheckBoxTransparent;
	mxCheckBox*		m_pCheckBoxEyeGlint;
	mxCheckBox*		m_pCheckBoxNoDecal;
	mxCheckBox*		m_pCheckBoxNoCulling;

	mxLabel*		m_pLabelTransSlider;
	mxSlider*		m_pSliderTransSlider;

	mxLabel*		m_pLabelTexSize;
	mxLabel*		m_pLabelTexScale;
	mxLineEdit*		m_pLineEditWidth;
	mxLineEdit*		m_pLineEditHeight;

	mxLabel*		m_pLabelSequenceIndex;
	mxLabel*		m_pLabelFrameCount;
	mxLabel*		m_pLabelFramerate;
	mxLabel*		m_pLabelFrame;

	mxChoice*		m_pChoiceFlex;
	mxSlider*		m_pSliderFlexStrength;
	mxLabel*		m_pLabelFlexTimeLabel;
	mxLabel*		m_pLabelWAVLengthLabel;
	mxLineEdit*		m_pLineEditLength;
	mxLabel*		m_pLabelLength;

	mxCheckBox*		m_pCheckBoxFlexStay;
	mxCheckBox*		m_pCheckBoxFlexLoop;

	mxLabel**		m_pLabelFlexNames;
	mxSlider**		m_pSliderFlexScalers;
	Uint32			m_numFlexes;

	mxWindow*		m_pWindowFlexes;
	mxWindow*		m_pWindowFlexScripting;
	TextureWindow*	m_pTextureWindow;

	// Currently selected material index
	Uint32			m_currentMaterialIndex;
	// List of material names
	CArray<CString>	m_materialNamesArray;

private:
	// Current instance of this class
	static CControlPanel* g_pInstance;
};
#endif // CONTROLPANEL_H