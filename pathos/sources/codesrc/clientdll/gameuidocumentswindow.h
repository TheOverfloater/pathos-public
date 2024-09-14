/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMEUIDOCUMENTSWINDOW_H
#define GAMEUIDOCUMENTSWINDOW_H

#include "gameuielements.h"

enum documentsbuttonevent_t
{
	DOC_BUTTON_EXIT = 0,
	DOC_BUTTON_BACK,
	DOC_BUTTON_DOCUMENT_0
};

/*
====================
CGameUIDocumentsWindow

====================
*/
class CGameUIDocumentsWindow : public CGameUIWindow
{
public:
	struct textdocumentinfo_t
	{
		// File path
		CString filePath;
		// Document file path and optionally the code
		CString documentEntry;

		// Document title
		CString documentTitle;
		// Text file contents
		CString documentText;

		// Font set name
		CString titleTextSchema;
		// Text color
		color32_t titleTextColor;
		// Font set name
		CString textSchema;
		// Text color
		color32_t textColor;
	};

	struct button_t
	{
		button_t():
			pButton(nullptr),
			pDocumentInfo(nullptr)
			{
			}

		// Pointer to button
		CGameUIButton* pButton;

		// Info about the document
		textdocumentinfo_t* pDocumentInfo;
	};

public:
	// Object x inset for login window
	static const Uint32 DOCUMENTSWINDOW_TAB_X_INSET;
	// Object y inset for login window
	static const Uint32 DOCUMENTSWINDOW_TAB_Y_INSET;
	// Object y spacing for login window
	static const Uint32 DOCUMENTSWINDOW_TAB_X_SPACING;
	// Object x spacing for login window
	static const Uint32 DOCUMENTSWINDOW_TAB_Y_SPACING;
	// Text inset for documents window
	static const Uint32 DOCUMENTSWINDOW_TAB_TEXT_INSET;
	// Default text color
	static const color32_t DOCUMENTSWINDOW_TEXT_COLOR;
	// Height of the title surface
	static const Uint32 DOCUMENTSWINDOW_TITLE_SURFACE_HEIGHT;
	// Height of the main surface
	static const Uint32 DOCUMENTSWINDOW_MAIN_SURFACE_HEIGHT;
	// Height of the bottom button surface
	static const Uint32 DOCUMENTSWINDOW_BOTTOM_BUTTON_SURFACE_HEIGHT;
	// Script subfolder name
	static const Char DOCUMENTSWINDOW_SCRIPT_SUBFOLDER_NAME[];
	// Base script name
	static const Char DOCUMENTSWINDOW_SCRIPT_NAME[];
	// Color of highlighted buttons for this window
	static const color32_t DOCUMENTSWINDOW_BUTTON_NEW_COLOR;
	// Title text default schema set name
	static const Char DOCUMENTSWINDOW_TITLE_TEXTSCHEMA_NAME[];
	// Text default font schema name
	static const Char DOCUMENTSWINDOW_TEXTSCHEMA_NAME[];

public:
	CGameUIDocumentsWindow( Int32 flags, Int32 originX, Int32 originY, Uint32 width, Uint32 height );
	~CGameUIDocumentsWindow();

public:
	// Initializes the window
	void init( void ) override;

	// Initializes the data
	bool initData( const CArray<CString>& textFilesArray, const Char* pstrActiveFileName );
	// Returns the current window information
	void getInformation( CArray<CString>& textFilesArray, CString& pstrActiveFileName ) const;

	// Returns the type of the window
	virtual gameui_windows_t getWindowType( void ) const override { return GAMEUI_DOCUMENTSWINDOW; }

public:
	// Manages an event
	void ManageEvent( documentsbuttonevent_t event );
	// Sets the current active document
	void SetActiveDocument( textdocumentinfo_t* pDocument );

private:
	// Exit button
	CGameUIButton* m_pExitButton;
	// Array of buttons
	CArray<button_t> m_buttonsArray;

	// Edge thickness used
	Float m_usedEdgeThickness;
	// Button base X position used
	Uint32 m_buttonXPos;
	// Button base Y position used
	Uint32 m_buttonYPos;

	// Button width used
	Uint32 m_buttonWidth;
	// Button height used
	Uint32 m_buttonHeight;

	// Text inset used
	Uint32 m_textInset;
	// Tab Y inset used
	Uint32 m_tabYSpacing;
	// Tab width
	Uint32 m_tabWidth;

	// Title text object
	CGameUIText* m_pReaderTitleText;
	// Text tab object
	CGameUITextTab* m_pReaderTextTab;
	// Buttons tab object
	CGameUIScrollableSurface* m_pButtonsTab;

	// Array containing listing objects
	CArray<CGameUIObject*> m_pListingObjectsArray;
	// Array containing text reader objects
	CArray<CGameUIObject*> m_pTextReaderObjectsArray;

	// Array of text documents
	CArray<textdocumentinfo_t*> m_documentsArray;
	// Current document being read
	textdocumentinfo_t* m_pCurrentDocument;

	// Font set used
	const font_set_t* m_pFontSet;
};

/*
=================================
CGameUIDocumentsWindowButtonCallbackEvent

=================================
*/
class CGameUIDocumentsWindowButtonCallbackEvent : public CGameUICallbackEvent
{
public:
	CGameUIDocumentsWindowButtonCallbackEvent( CGameUIDocumentsWindow* pWindow, Uint32 type ):
		m_pWindow(pWindow),
		m_eventType((documentsbuttonevent_t)type)
	{ 
	};
	virtual ~CGameUIDocumentsWindowButtonCallbackEvent( void ) { };

public:
	// Performs the action
	virtual void PerformAction( Float param ) override;
	// Handles a special key event
	virtual bool KeyEvent( Int32 button, Int16 mod, bool keyDown ) override { return false; }
	// Handles a mouse button event
	virtual bool MouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override { return false; }

private:
	// Text window object
	CGameUIDocumentsWindow* m_pWindow;
	// Event type
	documentsbuttonevent_t m_eventType;
};

#endif //GAMEUIDOCUMENTSWINDOW_H