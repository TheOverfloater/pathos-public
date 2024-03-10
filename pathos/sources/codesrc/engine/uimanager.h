/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CUISCHEME_H
#define CUISCHEME_H

#include "ui_shared.h"

class CUIObject;
class CUIWindow;
struct font_set_t;

/*
=================================
CUIManager

=================================
*/
class CUIManager
{
public:
	// Default font schema of the game UI
	static const Char DEFAULT_TEXT_SCHEMA[];

public:
	CUIManager( void );
	~CUIManager( void );

public:
	// Initializes interface
	void Init( void );
	// Performs shut down operations
	void Shutdown( void );
	// Draws any active windows
	bool Draw( void );
	// Performs think functions
	void Think( void );
	// Performs think functions after all commands have been run
	void PostThink( void );

	// Handles an input event
	bool MouseButtonEvent( Int32 button, bool keyDown );
	// Handles a keyboard input event
	bool KeyEvent( Int32 button, Int16 mod, bool keyDown );
	// Handles a mouse wheel event
	bool MouseWheelEvent( Int32 button, bool keyDown, Int32 scroll );

	// Set a window as the focus window
	void SetFocusWindow( CUIWindow* pWindow );
	// Returns the window currently in focus
	CUIWindow* GetFocusWindow( void ) { return m_pFocusWindow; }

	// Hides specific types of windows
	void HideWindows( Int32 windowFlags );
	// Shows specific types of windows
	void ShowWindows( Int32 windowFlags );

	// Sets the filter for which windows to draw
	void SetDrawFilter( Int32 windowFlags );
	// Sets the filter for which windows to draw
	void RemoveDrawFilter( Int32 windowFlags );

	// Tells if mouse is over any active window
	bool IsMouseOverAnyWindow( void );

public:
	// Adds a window to the list
	void AddWindow( CUIWindow* pWindow );
	// Repositions windows after a resolution change
	void RepositionWindows( void );
	// Initializes interface
	void OnGLInitialization( void );

	// Tells if any windows are active
	bool HasActiveWindows( void );

	// Loads in a schema file
	ui_schemeinfo_t* LoadSchemaFile( const Char* pstrFilename );
	// Loads in a schema file
	ui_windowdescription_t* LoadWindowDescriptionFile( const Char* pstrWindowName, const Char* pstrFilename );

	// Returns the default font set
	const font_set_t* GetDefaultFontSet( void ) const { return m_pFontSet; }

private:
	// Sorts windows by focus index
	void ReorderWindows( void );
	// Destroys a UI window
	void DestroyWindow( CUIWindow* pWindow );

private:
	// Array of schemas loaded
	CArray<ui_schemeinfo_t*> m_tabSchemeArray;
	// Array of window descriptions
	CArray<ui_windowdescription_t*> m_windowDescriptionArray;
	// List of UI windows in memory
	CLinkedList<CUIWindow*> m_windowList;

	// Window in focus
	CUIWindow* m_pFocusWindow;
	// Last click index
	Uint32 m_currentFocusIndex;
	// Window filter flags
	Int32 m_windowFilterFlags;

	// Default font set
	const font_set_t* m_pFontSet;
};
extern CUIManager gUIManager;
#endif //CUISCHEME_H