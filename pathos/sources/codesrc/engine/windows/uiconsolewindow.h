/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef UICONSOLEWINDOW_H
#define UICONSOLEWINDOW_H

class CUIConsoleWindow;

/*
=================================
CUIConsoleExecuteCallback

=================================
*/
class CUIConsoleExecuteCallback : public CUICallbackEvent
{
public:
	explicit CUIConsoleExecuteCallback( CUIConsoleWindow* pWindow ):
	  m_pWindow(pWindow)
	{ };
	virtual ~CUIConsoleExecuteCallback( void ) { };

public:
	// Performs the action
	virtual void PerformAction( Float param );

private:
	// Console window pointer
	CUIConsoleWindow* m_pWindow;
};

/*
=================================
CUIConsoleKeyEventCallback

=================================
*/
class CUIConsoleKeyEventCallback : public CUICallbackEvent
{
public:
	explicit CUIConsoleKeyEventCallback( CUIConsoleWindow* pWindow ):
		m_pWindow(pWindow),
		m_lastButton(0)
	{ };
	virtual ~CUIConsoleKeyEventCallback( void ) { };

public:
	// Performs the action
	virtual void PerformAction( Float param ) { };
	// Handles a special key event
	virtual bool KeyEvent( Int32 button, Int16 mod, bool keyDown );

private:
	// Console window pointer
	CUIConsoleWindow* m_pWindow;
	// Last key that was pressed
	Int32 m_lastButton;
};

/*
=================================
CUIConsoleWindow

=================================
*/
class CUIConsoleWindow : public CUIWindow
{
private:
	// Window description file
	static const Char WINDOW_DESC_FILE[];
	// Window object name
	static const Char WINDOW_OBJ_NAME[];
	// Execute button object name
	static const Char EXECUTE_BUTTON_OBJ_NAME[];
	// Input tab object name
	static const Char INPUT_TAB_OBJ_NAME[];
	// History tab object name
	static const Char HISTORY_TAB_OBJ_NAME[];

private:
	CUIConsoleWindow( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
public:
	~CUIConsoleWindow( void );

public:
	// Creates an instance of the console window
	static CUIConsoleWindow* CreateInstance( void );
	// Returns the current instance of the console window
	static CUIConsoleWindow* GetInstance( void );
	// Destroys the current instance
	static void DestroyInstance( void );

public:
	// Executes the currently typed text
	virtual void Execute( void );

	// Sets the history text
	virtual void SetHistoryText( const Char* pstrText );
	// Saves the currently present input
	virtual void SaveInputText( void );
	// Restores the input text from the container
	virtual bool RestoreInputText( void );
	// Clears the input text from the container
	virtual void ClearInputText( void );
	// Sets the input text for the container
	virtual void SetInputText( const Char* pstrText );

	// Retrieves the best input choice from the console list
	virtual void GetNextBestInputChoice( void );
	// Resets the tabbing filter string
	virtual void ResetTabFilter( void );

	// Called on GL initialization
	virtual void onGLInitialization( void ) override;

public:
	// Loads the schema, and creates the sub-elements
	virtual bool init( const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pWindowObject );

public:
	// History text tab
	CUITextTab* m_pHistoryTab;
	// Input text tab
	CUITextInputTab * m_pInputTab;

	// Saved input text
	CString m_savedInput;
	// TRUE if input was saved
	bool m_inputWasSaved;

	// Tabbing filter string
	CString m_tabbingFilter;
	// TRUE if tabbing filter was set
	bool m_tabbingFilterSet;

	// Previous resolution width
	Uint32 m_prevResolutionWidth;
	// Previous resolution height
	Uint32 m_prevResolutionHeight;

	// Current instance
	static CUIConsoleWindow* m_pInstance;
};

#endif //UICONSOLEWINDOW_H