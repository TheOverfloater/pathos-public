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
#include "hud.h"

#include "gameuidocumentswindow.h"
#include "gameuiwindows_shared.h"
#include "snd_shared.h"
#include "gameui_shared.h"
#include "gameuitextwindow.h"

// Object x inset for login window
const Uint32 CGameUIDocumentsWindow::DOCUMENTSWINDOW_TAB_X_INSET = 30;
// Object y inset for login window
const Uint32 CGameUIDocumentsWindow::DOCUMENTSWINDOW_TAB_Y_INSET = 60;
// Object y spacing for login window
const Uint32 CGameUIDocumentsWindow::DOCUMENTSWINDOW_TAB_Y_SPACING = 20;
// Object x spacing for login window
const Uint32 CGameUIDocumentsWindow::DOCUMENTSWINDOW_TAB_X_SPACING = 80;
// Text inset for objectives window
const Uint32 CGameUIDocumentsWindow::DOCUMENTSWINDOW_TAB_TEXT_INSET = 10;
// Default text color
const color32_t CGameUIDocumentsWindow::DOCUMENTSWINDOW_TEXT_COLOR = color32_t(255, 255, 255, 255);
// Height of the title surface
const Uint32 CGameUIDocumentsWindow::DOCUMENTSWINDOW_TITLE_SURFACE_HEIGHT = 50;
// Height of the main surface
const Uint32 CGameUIDocumentsWindow::DOCUMENTSWINDOW_MAIN_SURFACE_HEIGHT = 550;
// Height of the bottom button surface
const Uint32 CGameUIDocumentsWindow::DOCUMENTSWINDOW_BOTTOM_BUTTON_SURFACE_HEIGHT = 100;
// Base script name
const Char CGameUIDocumentsWindow::DOCUMENTSWINDOW_SCRIPT_NAME[] = "defaults.txt";
// Color of highlighted buttons for this window
const color32_t CGameUIDocumentsWindow::DOCUMENTSWINDOW_BUTTON_NEW_COLOR = color32_t(0, 255, 0, 100);
// Title text default schema set name
const Char CGameUIDocumentsWindow::DOCUMENTSWINDOW_TITLE_TEXTSCHEMA_NAME[] = "documentstitle";
// Text default font schema name
const Char CGameUIDocumentsWindow::DOCUMENTSWINDOW_TEXTSCHEMA_NAME[] = "documentstext";

//====================================
//
//====================================
CGameUIDocumentsWindow::CGameUIDocumentsWindow( Int32 flags, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	CGameUIWindow(flags, originX, originY, width, height),
	m_pExitButton(nullptr),
	m_pCurrentDocument(nullptr),
	m_usedEdgeThickness(0),
	m_buttonXPos(0),
	m_buttonYPos(0),
	m_buttonWidth(0),
	m_buttonHeight(0),
	m_textInset(0),
	m_tabYSpacing(0),
	m_tabWidth(0),
	m_pReaderTitleText(nullptr),
	m_pReaderTextTab(nullptr),
	m_pFontSet(nullptr)
{
}

//====================================
//
//====================================
CGameUIDocumentsWindow::~CGameUIDocumentsWindow( void )
{
	if(!m_documentsArray.empty())
	{
		for(Uint32 i = 0; i < m_documentsArray.size(); i++)
			delete m_documentsArray[i];

		m_documentsArray.clear();
	}
}

//====================================
//
//====================================
void CGameUIDocumentsWindow::init( void )
{
	// Init basic window elements
	Uint32 verticalbarheight, middlebarwidth, barThickness;
	CGameUIWindow::initBackground(verticalbarheight, middlebarwidth, barThickness);

	Uint32 tabTopInset = gHUDDraw.ScaleX(GAMEUIWINDOW_TAB_TOP_INSET);
	Uint32 hBarYOrigin = gHUDDraw.ScaleY(GAMEUIWINDOW_H_BAR_Y_ORIGIN);
	Uint32 tabSideInset = gHUDDraw.ScaleX(GAMEUIWINDOW_TAB_SIDE_INSET);
	Uint32 mainTabMaxWidth = gHUDDraw.ScaleX(GAMEUIWINDOW_MAIN_TAB_MAX_WIDTH);
	Uint32 titleSurfaceHeight = gHUDDraw.ScaleY(DOCUMENTSWINDOW_TITLE_SURFACE_HEIGHT);
	Uint32 mainSurfaceHeight = gHUDDraw.ScaleY(DOCUMENTSWINDOW_MAIN_SURFACE_HEIGHT);
	Uint32 bottomButtonSurfaceHeight = gHUDDraw.ScaleY(DOCUMENTSWINDOW_BOTTOM_BUTTON_SURFACE_HEIGHT);
	Uint32 tabXSpacing = gHUDDraw.ScaleX(DOCUMENTSWINDOW_TAB_X_SPACING);
	Uint32 textInset = gHUDDraw.ScaleY(CGameUITextWindow::TEXTWINDOW_TEXT_TAB_TEXT_INSET);

	m_usedEdgeThickness = gHUDDraw.ScaleX(GAMEUIWINDOW_EDGE_THICKNESS);
	m_tabYSpacing = gHUDDraw.ScaleY(DOCUMENTSWINDOW_TAB_Y_SPACING);
	m_textInset = gHUDDraw.ScaleY(DOCUMENTSWINDOW_TAB_TEXT_INSET);

	//
	// Get the font sets used
	//

	// Get the title font
	Uint32 screenWidth, screenHeight;
	cl_renderfuncs.pfnGetScreenSize(screenWidth, screenHeight);

	// Get the title font used
	const font_set_t* pTitleFont = cl_engfuncs.pfnGetResolutionSchemaFontSet(DOCUMENTSWINDOW_TITLE_TEXTSCHEMA_NAME, screenHeight);
	if(!pTitleFont)
		pTitleFont = gGameUIManager.GetDefaultFontSet();
	
	// Get the font set used for the listings
	m_pFontSet = cl_engfuncs.pfnGetResolutionSchemaFontSet(DOCUMENTSWINDOW_TEXTSCHEMA_NAME, screenHeight);
	if (!m_pFontSet)
		m_pFontSet = gGameUIManager.GetDefaultFontSet();

	//
	// Create the main tab objects
	//
	m_tabWidth = middlebarwidth - barThickness*2 - tabSideInset*2;
	if(m_tabWidth > mainTabMaxWidth)
		m_tabWidth = mainTabMaxWidth;

	Int32 tabOriginX = m_width / 2.0 - m_tabWidth / 2.0;
	Int32 tabOriginY = hBarYOrigin + barThickness + tabTopInset;

	// Create the title tab
	CGameUISurface* pTitleTab = new CGameUISurface(CGameUIObject::FL_NO_BOTTOM_BORDER,
		m_usedEdgeThickness,
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		tabOriginX,
		tabOriginY,
		m_tabWidth,
		titleSurfaceHeight);
	pTitleTab->setParent(this);

	// Create the buttons tab
	tabOriginY += titleSurfaceHeight;
	m_pButtonsTab = new CGameUIScrollableSurface(CGameUIObject::FL_NO_BOTTOM_BORDER,
		m_usedEdgeThickness,
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		m_pFontSet,
		tabOriginX,
		tabOriginY,
		m_tabWidth,
		mainSurfaceHeight,
		m_tabYSpacing);
	m_pButtonsTab->setParent(this);
	m_pListingObjectsArray.push_back(m_pButtonsTab);

	//
	// Create the text tab
	//
	m_pReaderTextTab = new CGameUITextTab(CGameUIObject::FL_NO_BOTTOM_BORDER, 
		m_pFontSet,
		textInset,
		m_usedEdgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		GAMEUIWINDOW_DEFAULT_TEXT_COLOR,
		tabOriginX,
		tabOriginY,
		m_tabWidth,
		mainSurfaceHeight);
	m_pReaderTextTab->setParent(this);
	m_pTextReaderObjectsArray.push_back(m_pReaderTextTab);

	// Create the bottom button tab
	tabOriginY += mainSurfaceHeight;
	CGameUISurface* pBottomButtonTab = new CGameUISurface(CGameUIObject::FL_NONE,
		m_usedEdgeThickness,
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		tabOriginX,
		tabOriginY,
		m_tabWidth,
		bottomButtonSurfaceHeight);
	pBottomButtonTab->setParent(this);

	// Create window title
	CGameUIText* pWindowTitleText = new CGameUIText(CGameUIObject::FL_ALIGN_CH|CGameUIObject::FL_ALIGN_CV,
		DOCUMENTSWINDOW_TEXT_COLOR,
		m_pFontSet,
		0,
		0);

	pWindowTitleText->setText("Documents");
	pWindowTitleText->setParent(pTitleTab);
	m_pListingObjectsArray.push_back(pWindowTitleText);

	// Create reader title text
	m_pReaderTitleText = new CGameUIText(CGameUIObject::FL_ALIGN_CH|CGameUIObject::FL_ALIGN_CV,
		DOCUMENTSWINDOW_TEXT_COLOR,
		m_pFontSet,
		0,
		0);
	m_pReaderTitleText->setParent(pTitleTab);
	m_pTextReaderObjectsArray.push_back(m_pReaderTitleText);

	// Create the buttons
	Uint32 buttonWidth = m_buttonWidth = m_tabWidth - 2*tabXSpacing;
	Uint32 buttonHeight = bottomButtonSurfaceHeight - m_tabYSpacing * 2;

	m_buttonHeight = (mainSurfaceHeight - m_tabYSpacing * 7)/6;

	Int32 buttonXPos = m_buttonXPos = tabXSpacing;
	Int32 buttonYPos = m_buttonYPos = m_tabYSpacing;

	// Create the exit button
	CGameUIDocumentsWindowButtonCallbackEvent* pExitEvent = new CGameUIDocumentsWindowButtonCallbackEvent(this,  DOC_BUTTON_EXIT);
	CGameUIButton* pExitButton = new CGameUIButton(CGameUIObject::FL_NONE, 
		pExitEvent,
		SDLK_ESCAPE,
		m_usedEdgeThickness,
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		buttonXPos,
		buttonYPos,
		buttonWidth,
		buttonHeight);
	pExitButton->setParent(pBottomButtonTab);
	pExitButton->setText("Exit");
	m_pListingObjectsArray.push_back(pExitButton);

	// Create the back button
	CGameUIDocumentsWindowButtonCallbackEvent* pBackEvent = new CGameUIDocumentsWindowButtonCallbackEvent(this,  DOC_BUTTON_BACK);
	CGameUIButton* pBackButton = new CGameUIButton(CGameUIObject::FL_NONE, 
		pBackEvent,
		SDLK_BACKSPACE,
		m_usedEdgeThickness,
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		buttonXPos,
		buttonYPos,
		buttonWidth,
		buttonHeight);
	pBackButton->setParent(pBottomButtonTab);
	pBackButton->setText("Back");
	m_pTextReaderObjectsArray.push_back(pBackButton);
}

//====================================
//
//====================================
bool CGameUIDocumentsWindow::initData( const CArray<CString>& textFilesArray, const Char* pstrActiveFileName )
{
	Int32 buttonXPos = m_buttonXPos;
	Int32 buttonYPos = m_buttonYPos;

	// Track separately to avoid issues if a file is missing/bad
	Uint32 nbAdded = 0;
	for(Uint32 i = 0; i < textFilesArray.size(); i++)
	{
		CString textFileEntry = textFilesArray[i];

		CString textFilePath;
		CString textFileCode;

		// Check for the code
		Int32 semicolonPosition = textFileEntry.find(0, ";");
		if(semicolonPosition != -1)
		{
			Uint32 codeLength = textFileEntry.length() - semicolonPosition - 1;
			textFileCode.assign(textFileEntry.c_str() + semicolonPosition + 1, codeLength);
			textFilePath.assign(textFileEntry.c_str(), semicolonPosition);
		}
		else
		{
			// Just straight up use the entry
			textFilePath = textFileEntry;
		}

		// See if we already have this loaded
		textdocumentinfo_t* pDocument = nullptr;
		for(Uint32 j = 0; j < m_documentsArray.size(); j++)
		{
			textdocumentinfo_t* pCheck = m_documentsArray[j];
			if(!qstrcmp(pCheck->filePath, textFilePath))
			{
				pDocument = pCheck;
				break;
			}
		}

		// If not, load it in
		if(!pDocument)
		{
			// Title text
			CString titletext;
			// Font set name
			CString titletextschema = CGameUITextWindow::TEXTWINDOW_TITLE_TEXTSCHEMA_NAME;
			// Text color
			color32_t titletextcolor = GAMEUIWINDOW_DEFAULT_TEXT_COLOR;
			// Font set name
			CString textschema = CGameUITextWindow::TEXTWINDOW_TEXTSCHEMA_NAME;
			// Text color
			color32_t textcolor = GAMEUIWINDOW_DEFAULT_TEXT_COLOR;
			// Contents of the document
			CString documentContents;

			// Now load and process the file
			if(!CGameUITextWindow::ProcessTextFile(textFilePath.c_str(), textFileCode, titletext, titletextschema, titletextcolor, textschema, textcolor, documentContents))
			{
				cl_engfuncs.pfnCon_Printf("%s - Failed to process document '%s'.\n", __FUNCTION__, textFilePath.c_str());
				continue;
			}

			pDocument = new textdocumentinfo_t;
			m_documentsArray.push_back(pDocument);

			// Set file path and code
			pDocument->documentEntry = textFileEntry;
			pDocument->filePath = textFilePath;

			// Set document specifics
			pDocument->documentTitle = titletext;
			pDocument->titleTextSchema = titletextschema;
			pDocument->titleTextColor = titletextcolor;
			pDocument->textSchema = textschema;
			pDocument->textColor = textcolor;
			pDocument->documentText = documentContents;
		}

		// Assign button contents and such
		Uint32 addIndex = nbAdded;
		m_buttonsArray.resize(nbAdded + 1);
		nbAdded++;

		// Assign data read from the document
		button_t& newButton = m_buttonsArray[addIndex];
		newButton.pDocumentInfo = pDocument;

		// Create the button object
		CGameUIDocumentsWindowButtonCallbackEvent* pEvent = new CGameUIDocumentsWindowButtonCallbackEvent(this, DOC_BUTTON_DOCUMENT_0 + addIndex);
		newButton.pButton = new CGameUIButton(CGameUIObject::FL_NONE,
			pEvent,
			m_usedEdgeThickness,
			GAMEUIWINDOW_MAIN_TAB_COLOR,
			GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
			GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
			buttonXPos,
			buttonYPos,
			m_buttonWidth,
			m_buttonHeight);
		newButton.pButton->setParent(m_pButtonsTab);
		newButton.pButton->setVisible(false);
		m_pListingObjectsArray.push_back(newButton.pButton);

		// Set button properties
		buttonYPos += m_buttonHeight + m_tabYSpacing;
		newButton.pButton->setText(pDocument->documentTitle.c_str());
	}

	if(pstrActiveFileName)
	{
		for(Uint32 i = 0; i < m_documentsArray.size(); i++)
		{
			textdocumentinfo_t* pInfo = m_documentsArray[i];
			if(!qstrcmp(pInfo->filePath, pstrActiveFileName))
			{
				SetActiveDocument(pInfo);
				break;
			}
		}
	}

	// Enable the front layer if nothing got selected
	if(!m_pCurrentDocument)
		SetActiveDocument(nullptr);

	// Clear "new objective" flag
	gHUD.SetNewObjective(false);

	return true;
}

//====================================
//
//====================================
void CGameUIDocumentsWindow::getInformation( CArray<CString>& textFilesArray, CString& pstrActiveFileName ) const
{
	// The buttons array will contain all of the original document list
	for(Uint32 i = 0; i < m_buttonsArray.size(); i++)
		textFilesArray.push_back(m_buttonsArray[i].pDocumentInfo->documentEntry);

	if(m_pCurrentDocument)
		pstrActiveFileName = m_pCurrentDocument->filePath;
}

//====================================
//
//====================================
void CGameUIDocumentsWindow::ManageEvent( documentsbuttonevent_t event )
{
	if (event >= DOC_BUTTON_DOCUMENT_0)
	{
		button_t& button = m_buttonsArray[event - DOC_BUTTON_DOCUMENT_0];
		if (!button.pButton->isVisible())
			return;

		// Set this as the active document
		SetActiveDocument(button.pDocumentInfo);
	}
	else if(event == DOC_BUTTON_EXIT)
	{
		// Remove window
		m_windowFlags |= CGameUIWindow::FL_WINDOW_KILLME;
	}
	else if(event == DOC_BUTTON_BACK)
	{
		// Close current document and unhide the main tab
		SetActiveDocument(nullptr);
	}
	else
	{
		cl_engfuncs.pfnCon_Printf("%s - Unknown button event %d.\n", __FUNCTION__, event);
	}
}

//====================================
//
//====================================
void CGameUIDocumentsWindow::SetActiveDocument( textdocumentinfo_t* pDocument )
{
	if(!pDocument)
	{
		for(Uint32 i = 0; i < m_pListingObjectsArray.size(); i++)
		{
			m_pListingObjectsArray[i]->setVisible(true);
			m_pListingObjectsArray[i]->setDisabled(false);
		}

		for(Uint32 i = 0; i < m_pTextReaderObjectsArray.size(); i++)
		{
			m_pTextReaderObjectsArray[i]->setVisible(false);
			m_pTextReaderObjectsArray[i]->setDisabled(true);
		}
	}
	else
	{
		for(Uint32 i = 0; i < m_pListingObjectsArray.size(); i++)
		{
			m_pListingObjectsArray[i]->setVisible(false);
			m_pListingObjectsArray[i]->setDisabled(true);
		}

		for(Uint32 i = 0; i < m_pTextReaderObjectsArray.size(); i++)
		{
			m_pTextReaderObjectsArray[i]->setVisible(true);
			m_pTextReaderObjectsArray[i]->setDisabled(false);
		}

		CString textschema = pDocument->textSchema;
		CString titletextschema = pDocument->titleTextSchema;

		Uint32 screenWidth, screenHeight;
		cl_renderfuncs.pfnGetScreenSize(screenWidth, screenHeight);

		// Load font if not default
		const font_set_t* pfontset = nullptr;
		if(!textschema.empty())
			pfontset = cl_engfuncs.pfnGetResolutionSchemaFontSet(textschema.c_str(), screenHeight);

		if(!pfontset)
			pfontset = gGameUIManager.GetDefaultFontSet();

		const font_set_t* ptitlefontset = nullptr;
		if(!titletextschema.empty())
			ptitlefontset = cl_engfuncs.pfnGetResolutionSchemaFontSet(titletextschema.c_str(), screenHeight);

		if(!pfontset)
			ptitlefontset = gGameUIManager.GetDefaultFontSet();

		// Assign the contents
		m_pReaderTextTab->setTextColor(pDocument->textColor);
		m_pReaderTextTab->setFontSet(pfontset);
		m_pReaderTextTab->initData(reinterpret_cast<const byte*>(pDocument->documentText.c_str()), pDocument->documentText.length());

		if(!pDocument->documentTitle.empty())
		{
			m_pReaderTitleText->setFontSet(ptitlefontset);
			m_pReaderTitleText->setColor(pDocument->titleTextColor);
			m_pReaderTitleText->setText(pDocument->documentTitle.c_str());
		}
	}

	// Remember this
	m_pCurrentDocument = pDocument;
}

//====================================
//
//====================================
void CGameUIDocumentsWindowButtonCallbackEvent::PerformAction( Float param )
{
	if(!m_pWindow)
		return;

	m_pWindow->ManageEvent(m_eventType);
}