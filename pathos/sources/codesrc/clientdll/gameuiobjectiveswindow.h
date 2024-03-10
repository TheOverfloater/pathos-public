/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMEUIOBJECTIVESWINDOW_H
#define GAMEUIOBJECTIVESWINDOW_H

#include "gameuielements.h"

enum objectivesbuttonevent_t
{
	OBJ_BUTTON_1 = 0,
	OBJ_BUTTON_2,
	OBJ_BUTTON_3,
	OBJ_BUTTON_4,
	OBJ_BUTTON_5,
	OBJ_BUTTON_6,
	OBJ_BUTTON_EXIT
};

/*
====================
CGameUIObjectivesWindow

====================
*/
class CGameUIObjectivesWindow : public CGameUIWindow
{
public:
	// Max number of objective buttons
	static const Uint32 MAX_NB_OBJECTIVE_BUTTONS;

public:
	struct button_t
	{
		button_t():
			pButton(nullptr),
			pDescription(nullptr)
			{
			}

		// Objective name
		CString objectiveName;

		// Pointer to button
		CGameUIButton* pButton;
		// Pointer to description
		CGameUITextTab* pDescription;
	};

public:
	// Object x inset for login window
	static const Uint32 OBJECTIVESWINDOW_TAB_X_INSET;
	// Object y inset for login window
	static const Uint32 OBJECTIVESWINDOW_TAB_Y_INSET;
	// Object y spacing for login window
	static const Uint32 OBJECTIVESWINDOW_TAB_X_SPACING;
	// Object x spacing for login window
	static const Uint32 OBJECTIVESWINDOW_TAB_Y_SPACING;
	// Text inset for objectives window
	static const Uint32 OBJECTIVESWINDOW_TAB_TEXT_INSET;
	// Default text color
	static const color32_t OBJECTIVESWINDOW_TEXT_COLOR;
	// Height of the title surface
	static const Uint32 OBJECTIVESWINDOW_TITLE_SURFACE_HEIGHT;
	// Height of the button surface
	static const Uint32 OBJECTIVESWINDOW_BUTTON_SURFACE_HEIGHT;
	// Height of the info surface
	static const Uint32 OBJECTIVESWINDOW_INFO_SURFACE_HEIGHT;
	// Height of the exit button surface
	static const Uint32 OBJECTIVESWINDOW_EXIT_BUTTON_SURFACE_HEIGHT;
	// Script subfolder name
	static const Char OBJECTIVESWINDOW_SCRIPT_SUBFOLDER_NAME[];
	// Base script name
	static const Char OBJECTIVESWINDOW_SCRIPT_NAME[];
	// Color of highlighted buttons for this window
	static const color32_t OBJECTIVESWINDOW_BUTTON_NEW_COLOR;
	// Title text default schema set name
	static const Char OBJECTIVESWINDOW_TITLE_TEXTSCHEMA_NAME[];
	// Text default font schema name
	static const Char OBJECTIVESWINDOW_TEXTSCHEMA_NAME[];

public:
	CGameUIObjectivesWindow( Int32 flags, Int32 originX, Int32 originY, Uint32 width, Uint32 height );
	~CGameUIObjectivesWindow();

public:
	// Initializes the window
	void init( void ) override;
	// Think function for window
	void think( void ) override;

	// Initializes the data
	bool initData( const CArray<CString>& objectivesArray, const Char* pstrActiveObjectiveName, Int16& newObjectivesBitflags );
	// Returns the current window information
	void getInformation( CArray<CString>& objectivesArray, CString& activeObjectiveName, Int16& newObjectivesBitflags ) const;
	// Returns the type of the window
	virtual gameui_windows_t getWindowType( void ) const override { return GAMEUI_OBJECTIVESWINDOW; }

public:
	// Manages an event
	void ManageEvent( objectivesbuttonevent_t event );

private:
	// Title text
	CGameUIText* m_pWindowTitleText;
	// Default description
	CGameUIText* m_pDefaultDescription;
	// Exit button
	CGameUIButton* m_pExitButton;
	// Array of buttons
	CArray<button_t> m_buttonsArray;
	// Currently selected objective button
	button_t* m_pCurrentObjective;
	// Bitflags marking which entries are new
	Int16 m_newObjectivesBitflags;
};

/*
=================================
CGameUIObjectivesWindowExitCallbackEvent

=================================
*/
class CGameUIObjectivesWindowCallbackEvent : public CGameUICallbackEvent
{
public:
	CGameUIObjectivesWindowCallbackEvent( CGameUIObjectivesWindow* pLoginWindow, Uint32 type ):
		m_pObjectivesWindow(pLoginWindow),
		m_eventType((objectivesbuttonevent_t)type)
	{ 
	};
	virtual ~CGameUIObjectivesWindowCallbackEvent( void ) { };

public:
	// Performs the action
	virtual void PerformAction( Float param ) override;
	// Handles a special key event
	virtual bool KeyEvent( Int32 button, Int16 mod, bool keyDown ) override { return false; }
	// Handles a mouse button event
	virtual bool MouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override { return false; }

private:
	// Text window object
	CGameUIObjectivesWindow* m_pObjectivesWindow;
	// Event type
	objectivesbuttonevent_t m_eventType;
};

#endif //GAMEUIOBJECTIVESWINDOW_H