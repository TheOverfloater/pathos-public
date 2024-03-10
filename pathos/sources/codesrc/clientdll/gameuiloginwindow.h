/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMEUILOGINWINDOW_H
#define GAMEUILOGINWINDOW_H

#include "gameuielements.h"

enum loginwindowevent_t
{
	LOGINWINDOW_EVENT_USERNAMETAB = 0,
	LOGINWINDOW_EVENT_PASSWORDTAB,
	LOGINWINDOW_EVENT_LOGIN_BTN,
	LOGINWINDOW_EVENT_CANCEL_BTN
};

/*
====================
CGameUILoginWindow

====================
*/
class CGameUILoginWindow : public CGameUIWindow
{
public:
	// Object x inset for login window
	static const Uint32 LOGINWINDOW_TAB_X_INSET;
	// Object y inset for login window
	static const Uint32 LOGINWINDOW_TAB_Y_INSET;
	// Object y spacing for login window
	static const Uint32 LOGINWINDOW_TAB_X_SPACING;
	// Object x spacing for login window
	static const Uint32 LOGINWINDOW_TAB_Y_SPACING;
	// Space before info tab on y axis for login window
	static const Uint32 LOGINWINDOW_INFO_TAB_Y_SPACING;
	// Input tab width
	static const Uint32 LOGINWINDOW_INPUT_TAB_WIDTH;
	// Input tab height
	static const Uint32 LOGINWINDOW_INPUT_TAB_HEIGHT;
	// Input tab text inset
	static const Uint32 LOGINWINDOW_INPUT_TAB_TEXT_INSET;
	// Default text color
	static const color32_t LOGINWINDOW_TEXT_COLOR;
	// Default text color
	static const color32_t LOGINWINDOW_NOTES_INFO_TEXT_COLOR;
	// Default text color
	static const color32_t LOGINWINDOW_PROMPT_FAIL_TEXT_COLOR;
	// Default text color
	static const color32_t LOGINWINDOW_PROMPT_SUCCESS_TEXT_COLOR;
	// Button y spacing for login window
	static const Uint32 LOGINWINDOW_BUTTON_X_SPACING;
	// Prompt text lifetime
	static const Double LOGINWINDOW_PROMPT_LIFETIME;
	// Title text default schema set name
	static const Char LOGINWINDOW_TITLE_TEXTSCHEMA_NAME[];
	// Text default font schema name
	static const Char LOGINWINDOW_TEXTSCHEMA_NAME[];

public:
	CGameUILoginWindow( Int32 flags, Int32 originX, Int32 originY, Uint32 width, Uint32 height );
	~CGameUILoginWindow();

public:
	// Initializes the window
	void init( void ) override;
	// Initializes the data
	bool initData( const Char* pstrUsername, const Char* pstrPassword, const Char* pstrUsernameInput, const Char* pstrPasswordInput, bool stayTillNext );

	// Think function for window
	void think( void ) override;
	// Called when window is removed
	void onRemove( void ) override;
	// Mark for delayed removal
	void setDelayedRemoval( Double delay ) override;

	// Manages a key event
	virtual bool keyEvent( Int32 button, Int16 mod, bool keyDown ) override;

public:
	// Manages an event
	void ManageEvent( loginwindowevent_t event );

public:
	// Returns the current window information
	void getInformation( CString& username, CString& password, CString& usernameInput, CString& passwordInput, bool& stayTillNext );
	// Returns the type of the window
	virtual gameui_windows_t getWindowType( void ) const override { return GAMEUI_LOGINWINDOW; }

private:
	// Username field
	CGameUITextInputTab* m_pUsernameField;
	// Password field
	CGameUITextInputTab* m_pPasswordField;

	// Prompt text
	CGameUIText* m_pPromptText;
	// Prompt text disappear time
	Double m_promptTextTime;
	// Time at which window dies
	Double m_loginWindowTime;

	// Username text label
	CGameUIText* m_pInfoLabelUsername;
	// Username text label
	CGameUIText* m_pTextUsername;
	// Password text label
	CGameUIText* m_pInfoLabelPassword;
	// Username text label
	CGameUIText* m_pTextPassword;

	// Username
	CString m_username;
	// Password
	CString m_password;
	// TRUE if login was successful
	bool m_loginSuccessful;
	// TRUE if we should stay till next window spawn
	bool m_stayTillNextWindow;
	// Message send time
	Double m_messageSendTime;
};

/*
=================================
CGameUILoginWindowExitCallbackEvent

=================================
*/
class CGameUILoginWindowCallbackEvent : public CGameUICallbackEvent
{
public:
	CGameUILoginWindowCallbackEvent( CGameUILoginWindow* pLoginWindow, loginwindowevent_t type ):
		m_pLoginWindow(pLoginWindow),
		m_eventType(type)
	{ 
	};
	virtual ~CGameUILoginWindowCallbackEvent( void ) { };

public:
	// Performs the action
	virtual void PerformAction( Float param ) override;
	// Handles a special key event
	virtual bool KeyEvent( Int32 button, Int16 mod, bool keyDown ) override { return false; }
	// Handles a mouse button event
	virtual bool MouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override { return false; }

private:
	// Text window object
	CGameUILoginWindow* m_pLoginWindow;
	// Event type
	loginwindowevent_t m_eventType;
};

#endif //GAMEUILOGINWINDOW_H