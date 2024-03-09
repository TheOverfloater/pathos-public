/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef UINEWGAMEWINDOW_H
#define UINEWGAMEWINDOW_H

/*
=================================
CUINewGameWindow

=================================
*/
class CUINewGameWindow : public CUIWindow
{
public:
	// Window description file
	static const Char WINDOW_DESC_FILE[];
	// Window object name
	static const Char WINDOW_OBJ_NAME[];
	// Cancel button object name
	static const Char CANCEL_BUTTON_OBJ_NAME[];
	// 'Easy' button object name
	static const Char NEW_GAME_EASY_BUTTON_OBJ_NAME[];
	// 'Normal' button object name
	static const Char NEW_GAME_NORMAL_BUTTON_OBJ_NAME[];
	// 'Hard' button object name
	static const Char NEW_GAME_HARD_BUTTON_OBJ_NAME[];

public:
	enum buttonId_t
	{
		NEWGAME_BTN_CANCEL = 0,
		NEWGAME_BTN_EASY,
		NEWGAME_BTN_NORMAL,
		NEWGAME_BTN_HARD
	};

private:
	CUINewGameWindow( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
public:
	~CUINewGameWindow( void );

public:
	// Creates an instance of the console window
	static CUINewGameWindow* CreateInstance( bool isIngame );
	// Returns the current instance of the console window
	static CUINewGameWindow* GetInstance( void );
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
	static CUINewGameWindow* m_pInstance;
};

/*
=================================
CUINewGameWindowButtonEvent

=================================
*/
class CUINewGameWindowButtonEvent : public CUICallbackEvent
{
public:
	CUINewGameWindowButtonEvent( CUINewGameWindow* pWindow, Uint32 buttonId ):
		m_buttonId(buttonId),
		m_pWindow(pWindow)
	{ };
	virtual ~CUINewGameWindowButtonEvent( void ) { };

public:
	// Performs the action
	virtual void PerformAction( Float param ) { m_pWindow->buttonPressed( m_buttonId ); }

protected:
	// Button id
	Uint32 m_buttonId;
	// Window that created this
	CUINewGameWindow* m_pWindow;
};

#endif //UINEWGAMEWINDOW_H