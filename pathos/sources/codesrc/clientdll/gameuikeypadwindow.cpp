/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "huddraw.h"
#include "r_interface.h"
#include "clientdll.h"
#include "fontset.h"
#include "common.h"

#include "gameuikeypadwindow.h"
#include "gameuiwindows_shared.h"
#include "snd_shared.h"
#include "r_common.h"

// Width of the keypad window tab
const Uint32 CGameUIKeypadWindow::KEYPADWINDOW_TAB_WIDTH = 400;
// Height of the keypad window tab
const Uint32 CGameUIKeypadWindow::KEYPADWINDOW_TAB_HEIGHT = 500;
// Width of the keypad window tab
const Uint32 CGameUIKeypadWindow::KEYPADWINDOW_TAB_DISPLAY_HEIGHT = 60;
// Spacing between the buttons and digits of the keypad window
const Uint32 CGameUIKeypadWindow::KEYPADWINDOW_ELEMENT_SPACING = 10;
// Height of the keypad window notes tab
const Uint32 CGameUIKeypadWindow::KEYPADWINDOW_NOTES_TAB_HEIGHT = 100;
// Default text color
const color32_t CGameUIKeypadWindow::KEYPADWINDOW_TEXT_COLOR = color32_t(255, 255, 255, 255);
// Default text color
const color32_t CGameUIKeypadWindow::KEYPADWINDOW_NOTES_INFO_TEXT_COLOR = color32_t(30, 30, 255, 255);
// Button y spacing for login window
const Uint32 CGameUIKeypadWindow::KEYPADWINDOW_BUTTON_Y_SPACING = 20;
// Prompt text lifetime
const Double CGameUIKeypadWindow::KEYPADWINDOW_PROMPT_LIFETIME = 1;
// Default text color
const color32_t CGameUIKeypadWindow::KEYPADWINDOW_PROMPT_FAIL_TEXT_COLOR = color32_t(255, 30, 30, 255);
// Default text color
const color32_t CGameUIKeypadWindow::KEYPADWINDOW_PROMPT_SUCCESS_TEXT_COLOR = color32_t(30, 255, 30, 255);
// Keypad window button height
const Uint32 CGameUIKeypadWindow::KEYPADWINDOW_BUTTON_HEIGHT = 100;
// Title text default schema set name
const Char CGameUIKeypadWindow::KEYPADWINDOW_TITLE_TEXTSCHEMA_NAME[] = "keypadtitle";
// Text default font schema name
const Char CGameUIKeypadWindow::KEYPADWINDOW_TEXTSCHEMA_NAME[] = "keypadtext";

//====================================
//
//====================================
CGameUIKeypadWindow::CGameUIKeypadWindow( Int32 flags, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	CGameUIWindow(flags, originX, originY, width, height),
	m_pKeypadDisplayText(nullptr),
	m_pNotesPasscodeLabel(nullptr),
	m_pNotesPasscode(nullptr),
	m_codeAccepted(false),
	m_stayTillNextWindow(false),
	m_promptTextTime(0),
	m_messageSendTime(0),
	m_pInfoLabelPasscode(nullptr),
	m_pTextPasscode(nullptr)
{
}

//====================================
//
//====================================
CGameUIKeypadWindow::~CGameUIKeypadWindow( void )
{
}

//====================================
//
//====================================
void CGameUIKeypadWindow::init( void )
{
	// Init basic window elements
	Uint32 verticalbarheight, middlebarwidth, barThickness;
	CGameUIWindow::initBackground(verticalbarheight, middlebarwidth, barThickness);

	// Scale elements by relative sizes
	Uint32 tabTopInset = gHUDDraw.ScaleY(GAMEUIWINDOW_TAB_TOP_INSET);
	Uint32 hBarYOrigin = gHUDDraw.ScaleY(GAMEUIWINDOW_H_BAR_Y_ORIGIN);
	Uint32 keypadTabWidth = gHUDDraw.ScaleX(KEYPADWINDOW_TAB_WIDTH);
	Uint32 keypadTabHeight = gHUDDraw.ScaleY(KEYPADWINDOW_TAB_HEIGHT);
	Uint32 edgeThickness = gHUDDraw.ScaleX(GAMEUIWINDOW_EDGE_THICKNESS);
	Uint32 notesTabHeight = gHUDDraw.ScaleY(KEYPADWINDOW_NOTES_TAB_HEIGHT);
	Uint32 elementSpacing = gHUDDraw.ScaleX(KEYPADWINDOW_ELEMENT_SPACING);
	Uint32 displayTabHeight = gHUDDraw.ScaleX(KEYPADWINDOW_TAB_DISPLAY_HEIGHT);
	Uint32 keypadButtonHeight = gHUDDraw.ScaleY(KEYPADWINDOW_BUTTON_HEIGHT);

	//
	// Create the title text object
	//
	Uint32 screenWidth, screenHeight;
	cl_renderfuncs.pfnGetScreenSize(screenWidth, screenHeight);

	const font_set_t* pTitleFont = cl_engfuncs.pfnGetResolutionSchemaFontSet(KEYPADWINDOW_TITLE_TEXTSCHEMA_NAME, screenHeight);
	if(!pTitleFont)
		pTitleFont = gGameUIManager.GetDefaultFontSet();

	Uint32 textYOrigin = hBarYOrigin + tabTopInset/2.0f;
	CGameUIText *pTitleText = new CGameUIText(CGameUIObject::FL_ALIGN_CH, GAMEUIWINDOW_DEFAULT_TEXT_COLOR, pTitleFont, 0, textYOrigin);
	pTitleText->setParent(this);

	// Set default text
	pTitleText->setText("Enter keycode");

	//
	// Create the tab objects
	//
	Int32 keypadTabXPos = m_width / 2 - keypadTabWidth / 2;
	Int32 keypadTabYPos = hBarYOrigin + barThickness + tabTopInset;

	CGameUISurface* pMainTab = new CGameUISurface(CGameUIObject::FL_NONE,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		keypadTabXPos,
		keypadTabYPos,
		keypadTabWidth,
		keypadTabHeight);
	pMainTab->setParent(this);

	Int32 notesTabYPos = keypadTabYPos + keypadTabHeight + elementSpacing;
	CGameUISurface* pNotesTab = new CGameUISurface(CGameUIObject::FL_NONE,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		keypadTabXPos,
		notesTabYPos,
		keypadTabWidth,
		notesTabHeight);
	pNotesTab->setParent(this);

	// Create the display tab surface
	Uint32 displayWidth = keypadTabWidth - elementSpacing*2;
	CGameUISurface* pDisplaySurface = new CGameUISurface(CGameUIObject::FL_NONE,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		elementSpacing,
		elementSpacing,
		displayWidth,
		displayTabHeight);
	pDisplaySurface->setParent(pMainTab);

	// Create the text object
	const font_set_t* pFontSet = cl_engfuncs.pfnGetResolutionSchemaFontSet(KEYPADWINDOW_TEXTSCHEMA_NAME, screenHeight);
	if(!pFontSet)
		pFontSet = gGameUIManager.GetDefaultFontSet();

	m_pKeypadDisplayText = new CGameUIText(CGameUIObject::FL_ALIGN_CH|CGameUIObject::FL_ALIGN_CV,
		KEYPADWINDOW_TEXT_COLOR,
		pFontSet,
		0,
		0);
	m_pKeypadDisplayText->setParent(pDisplaySurface);

	// Button size is a third of the space available for buttons
	Uint32 buttonWidth = (keypadTabWidth - elementSpacing * 4) / 3;
	Uint32 buttonHeight = (keypadTabHeight - displayTabHeight - elementSpacing * 7 - keypadButtonHeight)/4;

	// Create buttons 1 to 9
	for(Uint32 i = 0; i < 9; i++)
	{
		Int32 buttonXPos = elementSpacing + (i%3) * (buttonWidth+elementSpacing);
		Int32 buttonYPos = elementSpacing + displayTabHeight + elementSpacing + (i/3)*(buttonHeight+elementSpacing);

		CString buttonCharacter;
		buttonCharacter << (Int32)(i+1);

		CArray<SDL_Keycode> codesArray;
		codesArray.push_back(SDLK_1+i);
		codesArray.push_back(SDLK_KP_1+i);

		// Create the event and button
		CGameUIKeypadWindowDigitCallbackEvent* pEvent = new CGameUIKeypadWindowDigitCallbackEvent(this, buttonCharacter[0]);
		CGameUIButton* pButton = new CGameUIButton(CGameUIObject::FL_NONE, 
			pEvent, 
			codesArray,
			edgeThickness, 
			GAMEUIWINDOW_MAIN_TAB_COLOR,
			GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
			GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
			buttonXPos,
			buttonYPos,
			buttonWidth,
			buttonHeight);

		pButton->setParent(pMainTab);
		pButton->setText(buttonCharacter.c_str());
	}

	// Create the zero button
	Int32 buttonXPos = elementSpacing + (buttonWidth + elementSpacing);
	Int32 buttonYPos = elementSpacing + displayTabHeight + elementSpacing + 3*(buttonHeight+elementSpacing);

	CArray<SDL_Keycode> zeroKeyCodes;
	zeroKeyCodes.push_back(SDLK_0);
	zeroKeyCodes.push_back(SDLK_KP_0);

	// Create the event
	CGameUIKeypadWindowDigitCallbackEvent* pDigitEvent = new CGameUIKeypadWindowDigitCallbackEvent(this, '0');
	CGameUIButton* pDigitButton = new CGameUIButton(CGameUIObject::FL_NONE, 
		pDigitEvent, 
		zeroKeyCodes,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		buttonXPos,
		buttonYPos,
		buttonWidth,
		buttonHeight);

	pDigitButton->setParent(pMainTab);
	pDigitButton->setText("0");

	// Create the "Ok" button
	buttonXPos += elementSpacing + buttonWidth;

	CArray<SDL_Keycode> okKeyCodes;
	okKeyCodes.push_back(SDLK_RETURN);
	okKeyCodes.push_back(SDLK_KP_ENTER);

	CGameUIKeypadWindowButtonCallbackEvent* pOkEvent = new CGameUIKeypadWindowButtonCallbackEvent(this, KEYPADWINDOW_BUTTON_OK);
	CGameUIButton* pOkButton = new CGameUIButton(CGameUIObject::FL_NONE, 
		pOkEvent,
		okKeyCodes,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		buttonXPos,
		buttonYPos,
		buttonWidth,
		buttonHeight);
	pOkButton->setParent(pMainTab);
	pOkButton->setText("Ok");

	// Create the "Cancel" button
	buttonXPos = elementSpacing;
	buttonYPos += buttonHeight + elementSpacing;

	// Create the event and button
	CGameUIKeypadWindowButtonCallbackEvent* pCancelEvent = new CGameUIKeypadWindowButtonCallbackEvent(this, KEYPADWINDOW_BUTTON_CANCEL);
	CGameUIButton* pCancelButton = new CGameUIButton(CGameUIObject::FL_NONE, 
		pCancelEvent,
		SDLK_ESCAPE,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		buttonXPos,
		buttonYPos,
		buttonWidth,
		buttonHeight);
	pCancelButton->setParent(pMainTab);
	pCancelButton->setText("Cancel");

	// Create the "Delete" button
	buttonXPos = elementSpacing;
	buttonXPos += buttonWidth + elementSpacing;

	CGameUIKeypadWindowButtonCallbackEvent* pDeleteEvent = new CGameUIKeypadWindowButtonCallbackEvent(this, KEYPADWINDOW_BUTTON_DELETE);
	CGameUIButton* pDeleteButton = new CGameUIButton(CGameUIObject::FL_NONE, 
		pDeleteEvent,
		SDLK_BACKSPACE,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		buttonXPos,
		buttonYPos,
		buttonWidth,
		buttonHeight);
	pDeleteButton->setParent(pMainTab);
	pDeleteButton->setText("Delete");

	// Create the "Clear" button
	buttonXPos += buttonWidth + elementSpacing;

	CGameUIKeypadWindowButtonCallbackEvent* pClearEvent = new CGameUIKeypadWindowButtonCallbackEvent(this, KEYPADWINDOW_BUTTON_CLEAR);
	CGameUIButton* pClearButton = new CGameUIButton(CGameUIObject::FL_NONE, 
		pClearEvent,
		SDLK_DELETE,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		buttonXPos,
		buttonYPos,
		buttonWidth,
		buttonHeight);
	pClearButton->setParent(pMainTab);
	pClearButton->setText("Clear");

	//
	// Set the "Notes" tab objects
	//
	
	// Create notes title text
	CGameUIText* pNotesText = new CGameUIText(CGameUIObject::FL_ALIGN_CH, 
		KEYPADWINDOW_TEXT_COLOR, 
		pFontSet, 
		0, 
		pFontSet->fontsize);
	pNotesText->setParent(pNotesTab);
	pNotesText->setText("Notes");

	// Create the "username" field label
	Int32 labelXPos = elementSpacing;
	Int32 labelYPos = elementSpacing + pFontSet->fontsize + pNotesText->getHeight();

	m_pInfoLabelPasscode = new CGameUIText(CGameUIObject::FL_NONE, 
		KEYPADWINDOW_TEXT_COLOR,
		pFontSet, 
		labelXPos,
		labelYPos);
	m_pInfoLabelPasscode->setParent(pNotesTab);
	m_pInfoLabelPasscode->setText("Passcode: ");
	m_pInfoLabelPasscode->setVisible(false);

	Int32 fieldXPos = labelXPos + m_pInfoLabelPasscode->getWidth();

	// Create "username" data field
	m_pTextPasscode = new CGameUIText(CGameUIObject::FL_NONE,
		KEYPADWINDOW_NOTES_INFO_TEXT_COLOR,
		pFontSet, fieldXPos, labelYPos);
	m_pTextPasscode->setParent(pNotesTab);
	m_pTextPasscode->setVisible(false);
}

//====================================
//
//====================================
bool CGameUIKeypadWindow::initData( const Char* pstrPasscode, const Char* pstrInput, bool stayTillNext )
{
	if(pstrPasscode && qstrlen(pstrPasscode) > 0)
	{
		// Set the passcode
		m_passcode = pstrPasscode;

		// Verify that it's a valid digit
		if(Common::IsNumber(pstrPasscode))
		{
			m_pInfoLabelPasscode->setVisible(true);
			m_pTextPasscode->setText(pstrPasscode);
			m_pTextPasscode->setVisible(true);
		}
	}

	// If this is available, set input code
	if(pstrInput)
	{
		m_inputCode = pstrInput;
		m_pKeypadDisplayText->setText(pstrInput);
	}

	m_stayTillNextWindow = stayTillNext;
	if(m_stayTillNextWindow)
		m_windowFlags |= FL_WINDOW_WAIT_TILL_NEXT;

	return true;
}

//====================================
//
//====================================
void CGameUIKeypadWindow::getInformation( CString& passcode, CString& input, bool& stayTillNext ) const
{
	passcode = m_passcode;
	input = m_inputCode;
	stayTillNext = m_stayTillNextWindow;
}

//====================================
//
//====================================
void CGameUIKeypadWindow::ManageButtonEvent( keypadbutton_t event )
{
	switch(event)
	{
	case KEYPADWINDOW_BUTTON_CLEAR:
		{
			if(!m_inputCode.empty())
			{
				m_inputCode.clear();
				m_pKeypadDisplayText->setText("");
			}

			// Play the blip sound
			cl_engfuncs.pfnPlayAmbientSound(0, ZERO_VECTOR, SND_CHAN_AUTO, GAMEUI_BLIP_SOUND, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_2D, 0);
		}
		break;
	case KEYPADWINDOW_BUTTON_DELETE:
		{
			if(!m_inputCode.empty())
			{
				m_inputCode.erase(m_inputCode.length()-1, 1);
				m_pKeypadDisplayText->setText(m_inputCode.c_str());
			}

			// Play the blip sound
			cl_engfuncs.pfnPlayAmbientSound(0, ZERO_VECTOR, SND_CHAN_AUTO, GAMEUI_BLIP_SOUND, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_2D, 0);
		}
		break;
	case KEYPADWINDOW_BUTTON_CANCEL:
		{
			// Play the blip sound
			cl_engfuncs.pfnPlayAmbientSound(0, ZERO_VECTOR, SND_CHAN_AUTO, GAMEUI_BLIP_SOUND, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_2D, 0);
			m_windowFlags |= CGameUIWindow::FL_WINDOW_KILLME;
		}
		break;
	case KEYPADWINDOW_BUTTON_OK:
		{
			if(!m_passcode.empty() && !qstrcmp(m_inputCode, m_passcode))
			{
				cl_engfuncs.pfnPlayAmbientSound(0, ZERO_VECTOR, SND_CHAN_AUTO, GAMEUI_OK_SOUND, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_2D, 0);
				setDelayedRemoval(GAMEUIWINDOW_REMOVE_DELAY_TIME);

				m_pKeypadDisplayText->setText("Code accepted");
				m_pKeypadDisplayText->setColor(KEYPADWINDOW_PROMPT_SUCCESS_TEXT_COLOR);
				m_codeAccepted = true;
			}
			else
			{
				cl_engfuncs.pfnPlayAmbientSound(0, ZERO_VECTOR, SND_CHAN_AUTO, GAMEUI_FAIL_SOUND, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_2D, 0);
			
				m_pKeypadDisplayText->setText("Invalid code");
				m_pKeypadDisplayText->setColor(KEYPADWINDOW_PROMPT_FAIL_TEXT_COLOR);
				m_promptTextTime = cl_engfuncs.pfnGetClientTime() + KEYPADWINDOW_PROMPT_LIFETIME;

				m_inputCode.clear();
			}
		}
		break;
	}
}

//====================================
//
//====================================
void CGameUIKeypadWindow::ManageDigitButtonEvent( Char digit )
{
	// Reset text color
	if(m_promptTextTime)
	{
		m_promptTextTime = 0;
		m_pKeypadDisplayText->setColor(KEYPADWINDOW_TEXT_COLOR);
	}

	// Set the digit
	m_inputCode << digit;
	m_pKeypadDisplayText->setText(m_inputCode.c_str());

	// Play the blip sound
	cl_engfuncs.pfnPlayAmbientSound(0, ZERO_VECTOR, SND_CHAN_AUTO, GAMEUI_BLIP_SOUND, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_2D, 0);
}

//====================================
//
//====================================
void CGameUIKeypadWindow::think( void )
{
	if(m_promptTextTime && m_promptTextTime <= cl_engfuncs.pfnGetClientTime())
	{
		m_pKeypadDisplayText->setText("");
		m_pKeypadDisplayText->setColor(KEYPADWINDOW_TEXT_COLOR);
		m_promptTextTime = 0;
	}

	if(m_messageSendTime && m_messageSendTime <= cl_engfuncs.pfnGetClientTime())
	{
		onRemove();
		m_messageSendTime = 0;
	}

	// Call base class to manage think functions
	CGameUIWindow::think();
}

//====================================
//
//====================================
void CGameUIKeypadWindow::setDelayedRemoval( Double delay )
{
	if(m_windowFlags & FL_WINDOW_WAIT_TILL_NEXT)
		m_messageSendTime = cl_engfuncs.pfnGetClientTime() + delay;

	CGameUIWindow::setDelayedRemoval(delay);
}

//====================================
//
//====================================
void CGameUIKeypadWindow::onRemove( void ) 
{
	if(!m_codeAccepted)
		return;

	// Send message to server
	Uint32 msgid = g_pGUIManager->GetServerUIMessageId();
	if(!msgid)
	{
		cl_engfuncs.pfnCon_Printf("%s - Message 'GameUIMessage' not registered on client.\n", __FUNCTION__);
		return;
	}

	// Just tell them which window sent it
	cl_engfuncs.pfnClientUserMessageBegin(msgid);
		cl_engfuncs.pfnMsgWriteByte(GAMEUIEVENT_CODE_MATCHES);
		cl_engfuncs.pfnMsgWriteByte(GAMEUI_KEYPADWINDOW);
	cl_engfuncs.pfnClientUserMessageEnd();
}

//====================================
//
//====================================
void CGameUIKeypadWindowButtonCallbackEvent::PerformAction( Float param )
{
	if(!m_pKeypadWindow)
		return;

	m_pKeypadWindow->ManageButtonEvent(m_button);
}

//====================================
//
//====================================
void CGameUIKeypadWindowDigitCallbackEvent::PerformAction( Float param )
{
	if(!m_pKeypadWindow)
		return;

	m_pKeypadWindow->ManageDigitButtonEvent(m_digit);
}