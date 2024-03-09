/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMEUIELEMENTS_H
#define GAMEUIELEMENTS_H

#include "gameuimanager.h"
#include "gameui_shared.h"

class CGameUIManager;
class CGameUIScroller;

struct font_set_t;

// Max length of an input buffer
#define GAMEUI_MAX_INPUT_LENGTH 1024

/*
=================================
CGameUIObject

=================================
*/
class CGameUIObject
{
public:
	enum gui_el_flags_t
	{
		FL_NONE							= 0,
		FL_ALIGN_LEFT					= (1<<0),
		FL_ALIGN_RIGHT					= (1<<1),
		FL_ALIGN_TOP					= (1<<2),
		FL_ALIGN_BOTTOM					= (1<<3),
		FL_ALIGN_CV						= (1<<4),
		FL_ALIGN_CH						= (1<<5),
		FL_SCROLL_REVERSE				= (1<<6),
		FL_DRAW_LINKED_HIGHLIGHT_ONLY	= (1<<7),
		FL_NO_BOTTOM_BORDER				= (1<<8)
	};

	enum gui_scroller_align_t
	{
		FL_SCROLL_V = 0,
		FL_SCROLL_H,
	};

public:
	CGameUIObject( Int32 flags, Int32 originX, Int32 originY, Uint32 width, Uint32 height );
	virtual ~CGameUIObject( void );

public:
	// Sets the parent of this object
	void setParent( CGameUIObject* pparent );
	// Retrieves the parent object
	CGameUIObject* getParent( void );
	// Adds a child object
	void addChild( CGameUIObject* pchild );

	// Sets the position of the object
	void setPosition( Int32 xcoord, Int32 ycoord );
	// Returns the position relative to parent
	void getPosition( Int32& xcoord, Int32& ycoord ) const;
	// Returns the absolute position
	void getAbsolutePosition( Int32& xcoord, Int32& ycoord );

	// Sets the size of the object
	void setSize( Uint32 width, Uint32 height );
	// Returns the size of the object
	void getSize( Uint32& width, Uint32& height ) const;
	// Retrieves the width of the object
	Uint32 getWidth( void ) const;
	// Retrieves the height of the object
	Uint32 getHeight( void ) const;

	// Sets visibility for the object
	void setVisible( bool visible );
	// Returns visibility state
	bool isVisible( void ) const;

	// Returns the object flags
	Int32 getFlags( void ) const;

	// Performs think functions
	virtual void think( void );
	// Performs rendering functions
	virtual bool draw( void );

	// Tells if the mouse is over the object
	virtual bool isMouseOver( Int32 mousex, Int32 mousey );
	// Tells if the mouse is over the object
	virtual bool isMouseOver( void );

	// Manages a mouse wheel event
	virtual bool mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll );
	// Manages a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown );
	// Manages a key event
	virtual bool keyEvent( Int32 button, Int16 mod, bool keyDown );

	// Re-adjusts the element's position
	virtual void adjustPosition( void );

	// Tells if the object is interactive
	virtual bool isInteractive( void ) { return true; }
	
	// Sets offset for objects that use it
	virtual void setOffsetValue( Float offset ) { };
	// Returns the full range(for scrollers)
	virtual Uint32 getFullRange( void ) { return 0; }

	// Sets the game ui object pointer
	static void SetGameUIManager( CGameUIManager* pGUIManager );

	// Sets the input focus object for parent window
	virtual void setInputFocusObject( CGameUIObject* pObject );
	// Tells if the object is the focus of inputs
	virtual bool isInputFocusObject( void );
	// Tells if the object is the focus of inputs
	virtual void setIsInputFocusObject( bool isFocusObject );

	// Sets the linked object
	virtual void setLinkedObject( CGameUIObject* pObject );
	// Returns the linked object
	virtual CGameUIObject* getLinkedObject( void );

	// Tells if the object is a button
	virtual bool isButton( void ) { return false; }
	// Disables the UI object
	virtual void setDisabled( bool isDisabled ) { }
	// Tells if the object is disabled
	virtual bool isDisabled( void ) { return false; }

	// Disables any buttons tied to this object
	virtual void setButtonsDisabled( bool isDisabled );

protected:
	// Object flags
	Int32 m_flags;
	// X coordinate of the object relative to parent
	Int32 m_originX;
	// Y coordinate of the object relative to parent
	Int32 m_originY;
	// Initial X coordinate of the object relative to parent
	Int32 m_baseOriginX;
	// Initial Y coordinate of the object relative to parent
	Int32 m_baseOriginY;

	// Width of the object
	Uint32 m_width;
	// Height of the object
	Uint32 m_height;

	// Parent of this object
	CGameUIObject* m_pParent;
	// Array of child objects
	CArray<CGameUIObject*> m_pChildrenArray;

	// Linked object
	CGameUIObject* m_pLinkedObject;

	// TRUE if visible
	bool m_isVisible;
	// TRUE if this is the input focus object of the parent window
	bool m_isInputFocusObject;

protected:
	// Pointer to game ui manager object
	static CGameUIManager* g_pGUIManager;
};

/*
=================================
CGameUIBar

=================================
*/
class CGameUIBar : public CGameUIObject
{
public:
	enum gui_bar_flags_t
	{
		FL_BAR_FADE_NONE			= 0,
		FL_BAR_FADE_LEFT			= (1<<0),
		FL_BAR_FADE_RIGHT			= (1<<1),
		FL_BAR_FADE_BOTTOM			= (1<<2),
		FL_BAR_FADE_TOP				= (1<<3)
	};

public:
	CGameUIBar( Int32 flags, const color32_t& color, byte minalpha, Int32 fadeflags, Int32 originX, Int32 originY, Uint32 width, Uint32 height );
	virtual ~CGameUIBar( void );

	// Tells if the object is interactive
	bool isInteractive( void ) override { return false; }

	// Performs rendering functions
	bool draw( void ) override;

private:
	// Fade flags
	Int32 m_fadeFlags;
	// Bar color
	color32_t m_color;
	// Minimum fade alpha
	byte m_minAlpha;
};

/*
=================================
CGameUISurface

=================================
*/
class CGameUISurface : public CGameUIObject
{
public:
	CGameUISurface( Int32 flags, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, Int32 originX, Int32 originY, Uint32 width, Uint32 height );
	virtual ~CGameUISurface( void );

public:
	// Performs rendering functions
	bool draw( void ) override;

protected:
	// Surface color
	color32_t m_color;
	// Background color
	color32_t m_backgroundColor;
	// Edge thickness
	Uint32 m_edgeThickness;
};

/*
=================================
CGameUICallbackEvent

=================================
*/
class CGameUICallbackEvent
{
public:
	CGameUICallbackEvent( void ) { };
	virtual ~CGameUICallbackEvent( void ) { };

public:
	// Performs the action
	virtual void PerformAction( Float param ) = 0;
	// Handles a special key event
	virtual bool KeyEvent( Int32 button, Int16 mod, bool keyDown ) { return false; }
	// Handles a mouse button event
	virtual bool MouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) { return false; }
};

/*
=================================
CGameUIText

=================================
*/
class CGameUIText : public CGameUIObject
{
public:
	CGameUIText( Int32 flags, const color32_t& color, const font_set_t* pfontset, const Char* pstrText, Int32 originX, Int32 originY );
	CGameUIText( Int32 flags, const color32_t& color, const font_set_t* pfontset, Int32 originX, Int32 originY );
	CGameUIText( Int32 flags, const color32_t& color, const font_set_t* pfontset, const Char* pstrText, Int32 originX, Int32 originY, Uint32 maxwidth, Uint32 maxheight, Uint32 textInset );
	virtual ~CGameUIText( void );

public:
	// Performs rendering functions
	virtual bool draw( void ) override;

	// Sets the text to use
	virtual void setText( const Char* pstrText );
	// Sets the text to use
	virtual void setFontSet( const font_set_t* pFontSet );
	// Sets the color to use
	virtual void setColor( const color32_t& fontcolor );

private:
	// Adjusts the size based on text contents
	void adjustSize( void );

private:
	// Render color
	color32_t m_color;
	// Text to display
	CString m_text;
	// Font set used
	const font_set_t* m_pFontSet;
	// Text inset
	Uint32 m_textInset;
	// Maximum width
	Uint32 m_maxWidth;
	// Maximum height
	Uint32 m_maxHeight;
};

/*
=================================
CGameUIButton

=================================
*/
class CGameUIButton : public CGameUIObject
{
public:
	CGameUIButton( Int32 flags, CGameUICallbackEvent* pEvent, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, Int32 originX, Int32 originY, Uint32 width, Uint32 height );
	CGameUIButton( Int32 flags, CGameUICallbackEvent* pEvent, SDL_Keycode keycode, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, Int32 originX, Int32 originY, Uint32 width, Uint32 height );
	CGameUIButton( Int32 flags, CGameUICallbackEvent* pEvent, const CArray<SDL_Keycode>& keycodesArray, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, Int32 originX, Int32 originY, Uint32 width, Uint32 height );
	virtual ~CGameUIButton( void );

public:
	// Performs rendering functions
	virtual bool draw( void ) override;

	// Manages a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;
	// Manages a key event
	virtual bool keyEvent( Int32 button, Int16 mod, bool keyDown ) override;

	// Performs think functions
	virtual void think( void ) override;
	// Sets text for the button
	virtual void setText( const Char* pstrText );

	// Sets object enabled state
	virtual void setDisabled( bool isDisabled ) override;
	// Gets the object enabled state
	virtual bool isDisabled( void ) override;
	// Tells if the object is a button
	virtual bool isButton( void ) override { return true; }

public:
	// Sets object enabled state
	void setHighlighted( bool isHighlighted );
	// Sets bg color on the button
	void setBgColor( const color32_t& color );

protected:
	// TRUE if the button is clicked on
	bool m_isClickedOn;
	// TRUE if the button is disabled
	bool m_isDisabled;
	// TRUE if button is highlighted
	bool m_isHighlighted;
	// Callback event on click
	CGameUICallbackEvent *m_pEvent;
	// Key tied to this button
	CArray<SDL_Keycode> m_buttonKeysArray;

	// Surface color
	color32_t m_color;
	// Background color
	color32_t m_backgroundColor;
	// Clicked-on color
	color32_t m_highlightColor;
	// Edge thickness
	Uint32 m_edgeThickness;

	// Text to display if any
	CGameUIText* m_pButtonText;
};

/*
=================================
CGameUITextTab

=================================
*/
class CGameUITextTab : public CGameUISurface
{
public:
	CGameUITextTab( Int32 flags, const font_set_t* pfontset, Uint32 textinset, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, const color32_t& textcolor, Int32 originX, Int32 originY, Uint32 width, Uint32 height );
	virtual ~CGameUITextTab( void );

public:
	// Performs rendering functions
	virtual bool draw( void ) override;
	// Initializes the data
	virtual void initData( const byte* pdata, Uint32 datasize = 0 );

	// Sets offset for objects that use it
	virtual void setOffsetValue( Float offset ) override;
	// Updates range size
	virtual void updateRangeSize( void );

	// Sets the font set
	virtual void setFontSet( const font_set_t* pfontset );
	// Sets the text color
	virtual void setTextColor( const color32_t& textcolor );

	// Handles a mouse wheel event
	virtual bool mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll ) override;
	// Manages a key event
	virtual bool keyEvent( Int32 button, Int16 mod, bool keyDown ) override;

private:
	// Text inset
	Uint32 m_textInset;
	// Text data
	CString m_displayText;
	// Font set used
	const font_set_t* m_pFontSet;
	// Text offset
	Uint32 m_textOffset;
	// Text color
	color32_t m_textColor;

	// Scroller object
	CGameUIScroller* m_pScroller;
};

/*
=================================
CGameUIWindow

=================================
*/
class CGameUIWindow : public CGameUIObject
{
public:
	enum gui_win_flags_t
	{
		FL_WINDOW_NONE				= 0,
		FL_WINDOW_KILLME			= (1<<0),
		FL_WINDOW_WAIT_TILL_NEXT	= (1<<1),
		FL_WINDOW_DELAY_REMOVE		= (1<<2),
		FL_WINDOW_NO_BGBORDERS		= (1<<3)
	};

public:
	CGameUIWindow( Int32 flags, Int32 originX, Int32 originY, Uint32 width, Uint32 height );
	virtual ~CGameUIWindow( void );

public:
	// Initializes the window
	virtual void init( void ) = 0;
	// Initializes the window
	virtual void initBackground( Uint32& verticalbarheight, Uint32& middlebarwidth, Uint32& barThickness );
	// Initializes the data
	virtual void initData( const byte* pdata, Uint32 datasize = 0 ) {};
	// Returns the window flags
	virtual Int32 getWindowFlags() { return m_windowFlags; }

	// Sets the input focus object
	virtual void setInputFocusObject( CGameUIObject* pObject ) override;

	// Manages a mouse wheel event
	virtual bool mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll ) override;
	// Manages a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;
	// Manages a key event
	virtual bool keyEvent( Int32 button, Int16 mod, bool keyDown ) override;

	// Called when the window is removed
	virtual void onRemove( void ) {};
	// Returns the time at which the window is to be removed
	virtual Double getWindowRemoveTime() { return m_windowRemoveTime; }
	// Mark for delayed removal
	virtual void setDelayedRemoval( Double delay );
	// Returns the type of the window
	virtual gameui_windows_t getWindowType( void ) const { return GAMEUI_WINDOW_NONE; }

protected:
	// Window flags
	Int32 m_windowFlags;
	// Input focus object
	CGameUIObject* m_pInputFocusObject;
	// Delay until window close
	Double m_windowRemoveTime;
};

/*
=================================
CGameUIDragButton

=================================
*/
class CGameUIDragButton : public CGameUIButton
{
public:
	CGameUIDragButton( Int32 flags, gui_scroller_align_t alignment, CGameUICallbackEvent* pEvent, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CGameUIDragButton( void );

public:
	// Sets min/max offset from parent
	virtual void setBounds( Int32 start, Int32 end );
	// Sets min/max offset from parent
	virtual void getBounds( Int32& start, Int32& end );

	// Handles a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;
	// Performs think functions
	virtual void think( void ) override;

	// Sets the position of the button
	virtual void setPosition( Float position );
	// Gets the position of the button
	virtual Float getPosition( void );
	// Adjusts the position of the button
	virtual bool adjPosition( Int32 adjAmt, bool isMouseDrag, bool callEvent = true );

	// Sets the length of the button
	virtual void setLength( Uint32 length );
	// Gets the length of the button
	virtual Uint32 getLength( void );

	// Repositions the element according to alignment after the parent is set
	virtual void adjustPosition( void ) override;

protected:
	// Alignment of the scroller
	gui_scroller_align_t m_alignment;

	// Previous mouse x position while clicked on
	Int32 m_lastMouseX;
	// Previous mouse y position while clicked on
	Int32 m_lastMouseY;

	// Starting offset from parent
	Int32 m_startInset;
	// Offset from parent's other end
	Int32 m_endInset;

	// Current position of the slider
	Float m_position;
	// Last parent length the position was set on
	Uint32 m_lastParentLength;
};

/*
=================================
CGameUIScroller

=================================
*/
class CGameUIScroller : public CGameUISurface
{
public:
	// Dragger button action
	class CGameUIScrollerDragBtnAction : public CGameUICallbackEvent
	{
		public:
			explicit CGameUIScrollerDragBtnAction( CGameUIScroller* pScroller ):
				m_pScroller(pScroller)
			{ };
			virtual ~CGameUIScrollerDragBtnAction( void ) { };
			
		public:
			virtual void PerformAction( Float param ) override;

		private:
			// Window that created this
			CGameUIScroller* m_pScroller;
	};
public:
	CGameUIScroller( Int32 flags, gui_scroller_align_t alignment, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, Uint32 unitSize, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CGameUIScroller( void );

public:
	// Queries vif the element is interactive
	virtual bool isInteractive( void ) override { return true; }

	// Sets the full range's size
	virtual void setFullRange( Int32 fullRangeSize );
	// Returns the full range's size
	virtual Uint32 getFullRange( void ) override { return m_fullRangeSize; }
	// Readjusts the scroll drag button's position and size
	virtual void readjustDragButton( void );
	// Sets the unit size
	virtual void setUnitSize( Uint32 unitsize );

	// Returns the dragger button
	CGameUIDragButton* getDragButton( void ) { return m_pDragButton; }

	// Retreives the alignment
	virtual gui_scroller_align_t getAlignment( void ) { return m_alignment; }
	// Moves the scroller
	virtual bool moveScroller( Int32 button, Int32 scrollAmount );

protected:
	// Drag button for the scroller
	CGameUIDragButton* m_pDragButton;

	// Alignment
	gui_scroller_align_t m_alignment;

	// Total area to consider
	Int32 m_fullRangeSize;
	// Previous range we considered
	Int32 m_prevFullRangeSize;
	// Size of a single jump unit
	Uint32 m_unitSize;
};
/*
=================================
CGameUITextInputTab

=================================
*/
class CGameUITextInputTab : public CGameUIObject
{
public:
	CGameUITextInputTab( Int32 flags, CGameUICallbackEvent* pAction, Uint32 textinset, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, const font_set_t* pFont, Int32 originx, Int32 originy, Uint32 width, Uint32 height );
	virtual ~CGameUITextInputTab( void );

public:
	// Handles a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;
	// Handles a keyboard input event
	virtual bool keyEvent( Int32 button, Int16 mod, bool keyDown ) override;
	// Draws the text on the UI object
	virtual bool draw( void ) override;
	// Peforms think functions
	virtual void think( void ) override;

	// Specifies the text inset
	virtual void setTextInset( Uint32 inset ) { m_inset = inset; }
	// Returns the buffer's contents
	virtual const Char* getText( void ) { return m_szBuffer; }
	// Sets the buffer's contents
	virtual void setText( const Char* pstrText );
	// Clears the buffer
	virtual void clearText( void );

protected:
	// Font set used
	const font_set_t* m_pFont;
	// Next marker blink time
	Float m_nextBlinkTime;
	// Boolean for market
	bool m_drawMarker;
	// Text inset on x axis
	Uint32 m_inset;

	// Surface color
	color32_t m_color;
	// Background color
	color32_t m_backgroundColor;
	// Clicked-on color
	color32_t m_highlightColor;
	// Edge thickness
	Uint32 m_edgeThickness;

	// Current input string
	Char m_szBuffer[GAMEUI_MAX_INPUT_LENGTH+1];
	// Marker position
	Uint32 m_inputPosition;

	// Callback handler
	CGameUICallbackEvent* m_pAction;
};
#endif //GAMEUIELEMENTS_H