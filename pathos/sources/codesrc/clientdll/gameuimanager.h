/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMEUIMANAGER_H
#define GAMEUIMANAGER_H

class CGameUIWindow;
class CCVar;
struct font_set_t;

#include "gameui_shared.h"

/*
=================================
CUIManager

=================================
*/
class CGameUIManager
{
public:
	// Blur time for background
	static const Float BACKGROUND_BLUR_TIME;
	// Default font schema of the game UI
	static const Char DEFAULT_TEXT_SCHEMA[];

public:
	CGameUIManager( void );
	~CGameUIManager( void );

public:
	// Initializes the class
	bool Init( void );
	// Clears the class
	void Shutdown( void );

	// Initializes game objects
	bool InitGame( void );
	// Clears game objects
	void ClearGame( void );

	// Initializes opengl objects
	bool InitGL( void );
	// Clears opengl objects
	void ClearGL( void );

public:
	// Performs think functions
	void Think( void );
	// Performs rendering functions
	bool Draw( void );

	// TRUE if a window is active
	bool HasActiveWindows( void ) const;
	// Spawns a window based on type
	CGameUIWindow* SpawnWindow( gameui_windows_t windowtype );
	// Destroys a window
	void DestroyActiveWindow( void );
	// Recreates the current window
	void RespawnWindow( void );

	// Returns the server UI message id
	Uint32 GetServerUIMessageId( void ) const { return m_uiServerUserMsgId; }
	// Returns the default font set
	const font_set_t* GetDefaultFontSet( void ) const { return m_pFontSet; }

	// Returns the active window
	const CGameUIWindow* GetActiveWindow( void );

public:
	// Manages a mouse wheel event
	bool MouseWheelEvent( Int32 button, bool keyDown, Int32 scroll );
	// Manages a mouse button event
	bool MouseButtonEvent( Int32 button, bool keyDown );
	// Manages a key event
	bool KeyEvent( Int32 button, Int16 mod, bool keyDown );

public:
	// Currently active window
	CGameUIWindow* m_pActiveWindow;
	// Next windows to be shown
	CLinkedList<CGameUIWindow*> m_pNextWindowList;

	// Blur fade time
	Double m_blurFadeTime;
	// Blur active state
	bool m_isBlurActive;

	// UI server usermsg id
	Uint32 m_uiServerUserMsgId;

	// CVar to toggle borders
	CCVar* m_pCvarBorders;

	// Default font set used by game UI
	const font_set_t* m_pFontSet;
};
extern CGameUIManager gGameUIManager;
#endif //GAMEUIMANAGER_H