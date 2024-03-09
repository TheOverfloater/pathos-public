/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef UIEXITWINDOW_H
#define UIEXITWINDOW_H

class CUIExitWindow;

/*
=================================
CUIExitWindow

=================================
*/
class CUIExitWindow : public CUIWindow
{
public:
	// Window description file
	static const Char WINDOW_DESC_FILE[];
	// Window object name
	static const Char WINDOW_OBJ_NAME[];
	// Cancel button object name
	static const Char CANCEL_BUTTON_OBJ_NAME[];
	// Save and Quit button object name
	static const Char SAVE_AND_QUIT_BUTTON_OBJ_NAME[];
	// Quit button object name
	static const Char QUIT_BUTTON_OBJ_NAME[];
	// Quit game text object name
	static const Char QUIT_TEXT_OBJ_NAME[];
	// In-game quit game text object name
	static const Char INGAME_QUIT_TEXT_OBJ_NAME[];

public:
	enum buttonId_t
	{
		EXIT_BTN_CANCEL = 0,
		EXIT_BTN_QUIT,
		EXIT_BTN_SAVE_AND_QUIT
	};

private:
	CUIExitWindow( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
public:
	~CUIExitWindow( void );

public:
	// Creates an instance of the console window
	static CUIExitWindow* CreateInstance( bool isIngame );
	// Returns the current instance of the console window
	static CUIExitWindow* GetInstance( void );
	// Destroys the current instance
	static void DestroyInstance( void );

public:
	// Loads the schema, and creates the sub-elements
	virtual bool init( const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pWindowObject, bool isIngame );
	// The settings window cannot be resized
	virtual bool isResizable( void ) { return false; }
	// Manages a button press event
	virtual void buttonPressed( Uint32 buttonId );

public:
	// Current instance
	static CUIExitWindow* m_pInstance;
};

/*
=================================
CUIExitWindowEvent

=================================
*/
class CUIExitWindowEvent : public CUICallbackEvent
{
public:
	CUIExitWindowEvent( CUIExitWindow* pWindow, Uint32 buttonId ):
		m_buttonId(buttonId),
		m_pWindow(pWindow)
	{ };
	virtual ~CUIExitWindowEvent( void ) { };

public:
	// Performs the action
	virtual void PerformAction( Float param ) { m_pWindow->buttonPressed( m_buttonId ); }

protected:
	// Button id
	Uint32 m_buttonId;
	// Window that created this
	CUIExitWindow* m_pWindow;
};

#endif //UIEXITWINDOW_H