/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CINPUT_H
#define CINPUT_H

class CCVar;

/*
=================================
CInput

=================================
*/
class CInput
{
private:
	// Inputs file path
	static const Char KEYNAMES_FILE_PATH[];
	// Mouse wheel names
	static const CString MOUSE_WHEEL_NAMES[];

private:
	enum event_type_t
	{
		EVENT_UNDEFINED = -1,
		EVENT_KEYBOARD_KEY = 0,
		EVENT_MOUSE_BUTTON,
		EVENT_MOUSE_WHEEL
	};

	struct keyinfo_t
	{
		keyinfo_t():
			nbRepeats(0),
			isDown(false),
			color(255, 255, 255, 255)
		{}

		Uint32 nbRepeats;
		bool isDown;

		CString binding;
		CString name;
		color32_t color;
	};

	struct keyevent_t
	{
		keyevent_t():
			type(EVENT_UNDEFINED),
			button(0),
			mod(KMOD_NONE),
			isDown(false),
			scroll(0)
		{}

		event_type_t type;
		Int32 button;
		Int16 mod;
		bool isDown;

		Int32 scroll;
	};

public:
	CInput( void );
	~CInput( void );

public:
	// Initializes input class
	bool Init( void );
	// Loads the key names list
	bool LoadKeyNames( void );

	// Adds extra events before polling
	void AddExtraEvents( void );
	// Handles all inputs
	void HandleInput( void );
	// Processes a keyboard event
	void KeyEvent( Int32 button, Int16 mod, bool keyDown );
	// Processes a mouse button event
	void MouseButtonEvent( Int32 button, bool keyDown );
	// Handles a mouse wheel event
	void MouseWheelEvent( Int32 button, bool keyDown, Int32 scroll );

	// Handles an SDL eent
	void HandleSDLEvent( const SDL_Event& sdlEvent );

	// Shows the mouse
	void ShowMouse( void );
	// Hides the mouse
	void HideMouse( void );
	// Resets the game cursor position
	void ResetMouse( void );
	// Returns the mouse X position
	static void GetMousePosition( Int32& x, Int32& y );
	// Processes mouse movement
	void GetMouseDelta( Int32 &deltaX, Int32 &deltaY );
	// Updates mouse positoons
	void UpdateMousePositions( bool clearReset = true );

	// Returns the key name for a bind
	const Char* GetKeynameForBind( const Char* pstrBind );
	// Returns the key name for a bind
	const Char* GetKeynameForScancode( Int32 scancodeIdx );
	// Returns the mouse button name for a bind
	const Char* GetMouseButtonName( Int32 button );

	// Tells if the cursor is visible
	inline bool IsMouseVisible( void ) const { return m_isCursorVisible; }

public:
	// Command functions
	void CmdBindKey( void );
	void CmdUnbindKey( void );
	void CmdUnbindAll( void );

private:
	// Array of key bindings
	CArray<keyinfo_t> m_keyInfoArray;
	// Key input buffer array
	CArray<keyevent_t> m_keyInputBuffer;
	// Number of cached key inputs
	Uint32 m_numKeyInputs;

	// TRUE if the system mouse is visible
	bool m_isCursorVisible;
	// TRUE if shift is being pressed
	bool m_isShiftDown;
	// TRUE if we should ignore delta on the next frame
	bool m_resetMouse;

	// Mouse positions on previous frame
	Int32 m_oldMousePosition[2];
	// Current mouse positions
	Int32 m_mousePosition[2];
};
extern CInput gInput;
#endif // CINPUT_H