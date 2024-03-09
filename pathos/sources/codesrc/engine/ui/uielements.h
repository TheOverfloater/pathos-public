/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef UIELEMENTS_H
#define UIELEMENTS_H

// Max length of an input buffer
#define MAX_INPUT_LENGTH 1024

struct ui_schemeinfo_t;
struct ui_schemeobject_t;
struct font_set_t;

class CUIObject;
class CUISurface;
class CUIWindow;
class CUIText;
class CUIDragger;
class CUIButton;
class CUIScroller;
class CUITabBody;
class CUIListHeader;
class CUIListRow;
class CUIDropDownList;
class CUICallbackEvent;
class CUIListSeparator;

/*
=================================
CUICallbackEvent

=================================
*/
class CUICallbackEvent
{
public:
	CUICallbackEvent( void ) { };
	virtual ~CUICallbackEvent( void ) { };

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
CUIObject

=================================
*/
class CUIObject
{
	friend class CUISurface;

public:
	enum ui_elem_flags_t
	{
		UIEL_FL_NONE			= 0,		// No special flags
		UIEL_FL_FIXED_W			= (1<<0),	// Width of the element is fixed
		UIEL_FL_FIXED_H			= (1<<1),	// Height of the element is fixed
		UIEL_FL_ALIGN_L			= (1<<2),	// Align to the left edge of the parent
		UIEL_FL_ALIGN_R			= (1<<3),	// Align to the right edge of the parent
		UIEL_FL_ALIGN_T			= (1<<4),	// Align to the top edge of the parent
		UIEL_FL_ALIGN_B			= (1<<5),	// Align to the bottom edge of the parent
		UIEL_FL_ALIGN_CH		= (1<<6),	// Align to center of parent horizontally
		UIEL_FL_ALIGN_CV		= (1<<7),	// Align to center of parent vertically
		UIEL_FL_WRAP_WORD		= (1<<8),	// Breaks long lines into multiple lines
		UIEL_FL_FIXED_XPOS		= (1<<9),	// Relative X position is fixed in offset
		UIEL_FL_FIXED_YPOS		= (1<<10),	// Relative Y position is fixed in offset
		UIEL_FL_ONTOP			= (1<<11),	// Always draw last
		UIEL_FL_SCR_REVERSE		= (1<<12),  // Always auto-position scroller to the end when contents are changed
		UIEL_FL_EXPAND_W		= (1<<13),  // Width can be expanded by child elements
		UIEL_FL_EXPAND_H		= (1<<14),  // Height can be expanded by child elements
		UIEL_FL_NO_HEADER		= (1<<15),  // CUIList - No header used
		UIEL_FL_HOVER_HIGHLIGHT	= (1<<16)   // CUIList - Highlight elements the mouse hovers over
	};
public:
	enum ui_scroller_align_t
	{
		UIEL_SCROLL_V = 0,
		UIEL_SCROLL_H,
	};

public:
	CUIObject( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUIObject( void );

public:
	// Adds a child to this object
	void addChild( CUIObject* pChild );
	// Removes a child object from this object
	void removeChild( CUIObject* pChild ) { m_childrenArray.erase(pChild); }

	// Returns the number of children
	Uint32 getNbChildren( void ) const { return m_childrenArray.size(); }
	// Gets a child by it's index
	CUIObject* getChildByIndex( Uint32 idx );

	// Sets the parent, and also adds the element to the parent's children list
	void setParent( CUIObject* pParent );
	// Returns a pointer to the parent
	CUIObject* getParent( void ) { return m_pParent; }
	// Gets the size of the element
	virtual void getSize( Uint32& width, Uint32& height );

	// Sets the input focus object on the parent window
	virtual void setWindowInputFocusObject( CUIObject* pFocusObject );

	// Sets the width of the element
	virtual void setWidth( Int32 width ) { m_width = _min(0, width); }
	// Gets the width of the element
	virtual Uint32 getWidth( void ) { return m_width; }

	// Sets the height of the element
	virtual void setHeight( Int32 height ) { m_height = _min(0, height); }
	// Gets the height of the element
	virtual Uint32 getHeight( void ) { return m_height; }

	// Sets the position of the element relative to parent's
	virtual void setPosition( Int32 xpos, Int32 ypos, bool setBase = false );
	// Returns the position of the element relative to the parent's
	virtual void getPosition( Int32& xpos, Int32& ypos );
	// Returns the absolute origin on screen
	virtual void getAbsPosition( Int32& xpos, Int32& ypos );

	// Draws the element and it's children
	virtual bool draw( void );
	// Think function for state watching
	virtual void think( void );
	// Think function for state watching
	virtual void postThink( void );
	// Sets alpha value for the element
	virtual void setAlpha( Int32 alpha, bool recursive = false );
	// Sets the color for the element
	virtual void setColor( Uint32 r, Uint32 g, Uint32 b, Int32 a = -1, bool recursive = false );

	// Sets the visibility of the element
	virtual void setVisible( bool visible ) { m_isVisible = visible; }
	// Queries visibility of the element
	virtual bool isVisible( void ) { return m_isVisible; }

	// Queries vif the element is interactive
	virtual bool isInteractive( void ) { return false; }
	// Queries if the element is resizable
	virtual bool isResizable( void ) { return false; }

	// Handles a mouse button event
	virtual bool mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll );
	// Handles a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown );
	// Handles a keyboard input event
	virtual bool keyEvent( Int32 button, Int16 mod, bool keyDown ) { return false; }
	// Determines if the cursor is touching this object
	virtual bool isMouseOver( Int32 xPos, Int32 yPos );
	// Tells if the element is being clicked on
	virtual bool isClickedOn( void ) { return false; }

	// Tells if the element is disabled
	virtual void setDisabled( bool disabled ) { }
	// Tells if the element is disabled
	virtual bool isDisabled( void ) { return false; }

	// Moves the element by x and y amount
	virtual void move( Int32 x, Int32 y ) { if(m_pParent) m_pParent->move(x, y); };
	// Adjusts the element's size by x and y amount
	virtual bool adjustSize( Int32 x, Int32 y );
	// Repositions the element according to alignment after the parent is set
	virtual void adjustPosition( void );

	// Sets the scroller offset value(only for text tabs/lists
	virtual void setOffsetValue( Float offset ) { };

	// Tells if the proposed parent size is valid for us
	virtual bool isParentSizeValid( Uint32 testWidth, Uint32 testHeight, Int32 adjX, Int32 adjY );
	// Tells if a child element should be queried on size checks
	virtual bool considerElementOnResize( CUIObject* pObject ) { return true; }

	// Returns the flag values for the element
	virtual Int32 getFlags() const { return m_flags; }
	// Sets a flag
	virtual void setFlag( Int32 flag ) { m_flags |= flag; }
	// Removes a flag
	virtual void removeFlag( Int32 flag ) { m_flags &= ~flag; }
	// Retrieves the focus state
	virtual bool getFocusState( void ) const { return false; }
	// Returns the full range's size
	virtual Uint32 getFullRange( void ) { return 0; }

	// Updates text on text tabs
	virtual void updateRangeSize( void ) { };
	// For releasing pressed states on buttons
	virtual void releaseClickStates( void );
	// Requests that other tab objects be closed
	virtual void closeOtherTabs( CUIObject* pCaller );

	// Returns window owner of this element
	virtual CUIObject* getParentWindow( void );
	// TRUE if parent is window
	virtual bool isWindow( void ) { return false; }
	// TRUE if window is in focus
	virtual bool isInFocus( void ) { return false; }

	// Sets object name
	void setObjectName( const Char* pstrName ) { m_objName = pstrName; }
	// Retreives the object's name
	const Char* getObjectName( void ) const { return m_objName.c_str(); }

protected:
	// Sets the sizes of the element
	// This should only be called on background base elements
	virtual void setSize( Int32 width, Int32 height );

public:
	// Sets the render function pointers
	static void SetRenderInterface( const struct ui_engine_interface_t& renderfuncs );

protected:
	// Object name
	CString m_objName;
	// Flags
	Int32 m_flags;
	// Parent of the element
	CUIObject* m_pParent;
	// List containing all children
	CArray<CUIObject*> m_childrenArray;

	// Width of the element
	Uint32 m_width;
	// Original width
	Uint32 m_baseWidth;
	// Height of the element
	Uint32 m_height;
	// Original height
	Uint32 m_baseHeight;

	// X position relative to parent's position
	Int32 m_originX;
	// Original x position
	Int32 m_baseOriginX;
	// Y position relative to parent's position
	Int32 m_originY;
	// Original Y position
	Int32 m_baseOriginY;

	// Color and alpha values
	color32_t m_color;

	// TRUE if visible
	bool m_isVisible;

	// UI engine function pointers
	static struct ui_engine_interface_t g_engineFuncs;
};

/*
=================================
CUIInteractiveObject

=================================
*/
class CUIInteractiveObject : public CUIObject
{
public:
	CUIInteractiveObject( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUIInteractiveObject( void );

public:
	// Queries vif the element is interactive
	virtual bool isInteractive( void ) override { return true; }
	// Determines if the cursor is touching this object
	virtual bool isMouseOver( Int32 xPos, Int32 yPos ) override;
};

/*
=================================
CUITexturedObject

=================================
*/
class CUITexturedObject : public CUIObject
{
public:
	CUITexturedObject( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUITexturedObject( void );

	// Sets the default texture to use
	virtual void setDefaultTexture( const en_texture_t* ptexture ) { m_pDefaultTexture = ptexture; }
	// Sets the texture to use when the mouse is over the element
	virtual void setFocusTexture( const en_texture_t* ptexture ) { m_pFocusTexture = ptexture; }
	// Sets the texture to use when the element is clicked
	virtual void setClickedTexture( const en_texture_t* ptexture ) { m_pClickedTexture = ptexture; }
	// Sets the texture to use when the element is disabled
	virtual void setDisabledTexture( const en_texture_t* ptexture ) { m_pDisabledTexture = ptexture; }

	// Draws the element and it's children
	virtual bool draw( void ) override;

protected:
	// Textures of the element
	const en_texture_t* m_pDefaultTexture;
	const en_texture_t* m_pFocusTexture;
	const en_texture_t* m_pClickedTexture;
	const en_texture_t* m_pDisabledTexture;
};

/*
=================================
CUISurface

=================================
*/
class CUISurface : public CUIObject
{
public:
	CUISurface( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUISurface( void );

public:
	// Loads the schema, and creates the sub-elements
	virtual bool init( const Char* pstrSchemaName );
	// Sets the bottom border's elements
	virtual bool initBottomElements( void );
	// Sets the top border's elements
	virtual bool initTopElements( void );
	// Draws the surface, and all children
	virtual bool draw( void ) override;

	// Sets alpha value for the element
	virtual void setAlpha( Int32 alpha, bool recursive = false ) override;

	// Queries if the element is resizable
	virtual bool isResizable( void ) override { return true; }
	// Tells if the proposed parent size is valid for us
	virtual bool isParentSizeValid( Uint32 testWidth, Uint32 testHeight, Int32 adjX, Int32 adjY ) override;
	// Tells if a child element should be queried on size checks
	virtual bool considerElementOnResize( CUIObject* pObject ) override;

	// Adjusts the surface elements
	virtual void adjustBgElements( void );
	// Adjusts the top border
	virtual void adjustTopBorder( void );
	// Adjusts the element's size by x and y amount
	virtual bool adjustSize( Int32 x, Int32 y ) override;
	// Repositions the element according to alignment after the parent is set
	virtual void adjustPosition( void ) override;

protected:
	// Creates a single schema sub-object
	virtual CUITexturedObject* createObject( const ui_schemeinfo_t* pscheme, const Char* objectName );

protected:
	// Background UI element
	CUITexturedObject* m_pBackground;

	// Bottom left corner element
	CUITexturedObject* m_pBtmLeftCorner;
	// Bottom right corner element
	CUITexturedObject* m_pBtmRightCorner;
	// Top left corner element
	CUITexturedObject* m_pTopLeftCorner;
	// Top right corner element
	CUITexturedObject* m_pTopRightCorner;

	// Left border element
	CUITexturedObject* m_pLeftBorder;
	// Right border element
	CUITexturedObject* m_pRightBorder;
	// Bottom border element
	CUITexturedObject* m_pBottomBorder;
	// Top border element
	CUITexturedObject* m_pTopBorder;

	// Scheme object
	const ui_schemeinfo_t* m_pScheme;
};

/*
=================================
CUIWindow

=================================
*/
class CUIWindow : public CUISurface
{
public:
	enum ui_win_flags
	{
		UIW_FL_NONE				= 0,
		UIW_FL_KILLME			= (1<<1),
		UIW_FL_MENUWINDOW		= (1<<2),
		UIW_FL_CONSOLEWINDOW	= (1<<3),
		UIW_FL_DOWNLOADWINDOW	= (1<<4),
		UIW_FL_GAMEWINDOW		= (1<<5)
	};

public:
	// Close action object
	class CUIWindowCloseAction : public CUICallbackEvent
	{
		public:
			explicit CUIWindowCloseAction( CUIWindow* pWindow ):
				m_pWindow(pWindow)
			{ };
			virtual ~CUIWindowCloseAction( void ) { };
			
		public:
			virtual void PerformAction( Float param ) override;

		private:
			// Window that created this
			CUIWindow* m_pWindow;
	};

public:
	CUIWindow( Int32 winFlags, Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUIWindow( void );

	// Loads the schema, and creates the sub-elements
	virtual bool init( const Char* pstrSchemaName ) override;
	// Sets the title text
	void setTitle( const Char* pstrTitle, const font_set_t* pTitleFont, Uint32 insetX = 0, Uint32 insetY = 0 );
	// Moves the element by x and y amount
	virtual void move( Int32 x, Int32 y ) override;

	// Sets the input focus object
	virtual void setWindowInputFocusObject( CUIObject* pFocus ) override { m_pInputFocusObject = pFocus; }
	// Gets the input focus object
	virtual CUIObject* getInputFocusObject( void ) { return m_pInputFocusObject; }

	// Sets the focus index
	virtual void setFocusIndex( Uint32 focusIndex ) { m_focusIndex = focusIndex; }
	// Retrieves the focus index of this window
	virtual Uint32 getFocusIndex( void ) { return m_focusIndex; }
	// Sets the focus state
	virtual void setFocusState( bool inFocus ) { m_inFocus = inFocus; }
	// Retrieves the focus state
	virtual bool getFocusState( void ) const override { return m_inFocus; }

	// Handles a keyboard input event
	virtual bool keyEvent( Int32 button, Int16 mod, bool keyDown ) override;
	// Handles a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;
	// Handles a mouse button event
	virtual bool mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll ) override;

	// Set a flag on the window
	virtual void setWindowFlags( Int32 flags ) { m_winFlags |= flags; }
	// Returns the flags for the window
	virtual Int32 getWindowFlags( void ) { return m_winFlags; }
	// TRUE if parent is window
	virtual bool isWindow( void ) override { return true; }
	// TRUE if window is in focus
	virtual bool isInFocus( void ) override { return m_inFocus; }

	// Called on GL initialization
	virtual void onGLInitialization( void ) { };

protected:
	// Window flags
	Int32 m_winFlags;
	// Title object
	CUIText* m_pTitle;
	// Current object in input focus
	CUIObject* m_pInputFocusObject;
	// Click index on which this window achieved focus
	Uint32 m_focusIndex;
	// True if we're in focus
	bool m_inFocus;
};

/*
=================================
CUIText

=================================
*/
class CUIText : public CUIObject
{
public:
	CUIText( Int32 flags, const font_set_t* pFont, const Char* pstrText, Int32 originx, Int32 originy, Uint32 maxWidth = 0 );
	virtual ~CUIText( void );

public:
	// Sets the text to display
	void setText( const Char* pstrText );
	// Returns the text displayed
	const Char* getText( void ) const { return m_string.c_str(); }
	// Sets the font for the text to be used
	void setFont( struct font_set_t* pfont ) { m_pFont = pfont; }

	// Sets the displayed text
	void setDisplayText( const Char* pstrText );

public:
	// Draws the element
	bool draw( void ) override;
	// Repositions the element according to alignment after the parent is set
	void adjustPosition( void ) override;

protected:
	// String to display
	CString m_string;
	// Displayed text
	CString m_displayText;
	// Font set used
	const font_set_t* m_pFont;
	// Max width of the text object
	Uint32 m_maxTextWidth;
};

/*
=================================
CUIDragger

=================================
*/
class CUIDragger : public CUIObject
{
public:
	CUIDragger( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUIDragger( void );

public:
	// Handles a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;
	// Performs think functions
	virtual void think( void ) override;
	// Performs the adjustment required
	virtual bool performAdj( Int32 x, Int32 y );

	// Queries vif the element is interactive
	virtual bool isInteractive( void ) override { return true; }
	// Queries vif the element is interactive
	virtual bool isResizable( void ) override { return true; }
	// Tells if the proposed parent size is valid for us
	virtual bool isParentSizeValid( Uint32 testWidth, Uint32 testHeight, Int32 adjX, Int32 adjY ) override;

	// Tells if the element is being clicked on
	virtual bool isClickedOn( void ) override { return m_isClickedOn; }
	// For releasing pressed states on buttons
	virtual void releaseClickStates( void ) override { m_isClickedOn = false; };

protected:
	// Previous mouse x position while clicked on
	Int32 m_lastMouseX;
	// Previous mouse y position while clicked on
	Int32 m_lastMouseY;

	// True if we are clicked on
	bool m_isClickedOn;
};

/*
=================================
CUIResizer

=================================
*/
class CUIResizer : public CUIDragger
{
public:
	CUIResizer( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUIResizer( void );

public:
	// Performs the adjustment required
	virtual bool performAdj( Int32 x, Int32 y ) override;
};

/*
=================================
CUIButton

=================================
*/
class CUIButton : public CUISurface
{
public:
	CUIButton( Int32 flags, const Char* pstrText, const font_set_t* pFont, CUICallbackEvent* pAction, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	CUIButton( Int32 flags, const ui_schemeobject_t* pScheme, CUICallbackEvent* pAction, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	CUIButton( Int32 flags, CUICallbackEvent* pAction, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUIButton( void );

public:
	// Handles a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;
	// Performs think functions
	virtual void think( void ) override;

	// Queries vif the element is interactive
	virtual bool isInteractive( void ) override { return true; }
	// Queries vif the element is interactive
	virtual bool isResizable( void ) override { return false; }
	// Tells if the element is being clicked on
	virtual bool isClickedOn( void ) override { return m_isClickedOn; }
	// For releasing pressed states on buttons
	virtual void releaseClickStates( void ) override { m_isClickedOn = false; };

	// Tells if the element is disabled
	virtual void setDisabled( bool disabled ) override { m_isDisabled = disabled; }
	// Tells if the element is disabled
	virtual bool isDisabled( void ) { return m_isDisabled; }

protected:
	// Can be text or an icon
	CUIObject* m_pDisplay;
	// TRUE if we're clicked
	bool m_isClickedOn;
	// Button event class
	CUICallbackEvent* m_pAction;
	// TRUE if the button is disabled
	bool m_isDisabled;
};

/*
=================================
CUITextTab

=================================
*/
class CUITextTab : public CUISurface
{
public:
	CUITextTab( Int32 flags, const font_set_t* pFont, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUITextTab( void );

public:
	// Sets the source text array to draw from
	virtual void setText( const Char* pstrText );
	// Draws the text on the UI object
	virtual bool draw( void ) override;
	// Loads the schema, and creates the sub-elements
	virtual bool init( const Char* pstrSchemaName ) override;

	// Queries vif the element is interactive
	virtual bool isInteractive( void ) override { return true; }
	// Sets the scroller offset value(only for text tabs/lists
	virtual void setOffsetValue( Float offset ) override;
	// Specifies the text inset
	virtual void setTextInset( Uint32 inset ) { m_inset = inset; }

	// Handles a mouse button event
	virtual bool mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll ) override;

	// Updates text on text tabs
	virtual void updateRangeSize( void ) override;

protected:
	// Text to display
	const Char* m_pText;
	// Font set used
	const font_set_t* m_pFont;
	// Scroller object
	CUIScroller* m_pScroller;
	// Text offset
	Uint32 m_textOffset;
	// Text inset
	Uint32 m_inset;
};

/*
=================================
CUITextInputTab

=================================
*/
class CUITextInputTab : public CUISurface
{
public:
	CUITextInputTab( Int32 flags, CUICallbackEvent* pAction, const font_set_t* pFont, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUITextInputTab( void );

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

	// Current input string
	Char m_szBuffer[MAX_INPUT_LENGTH+1];
	// Marker position
	Uint32 m_inputPosition;

	// Callback handler
	CUICallbackEvent* m_pAction;
};

/*
=================================
CUIDragButton

=================================
*/
class CUIDragButton : public CUIButton
{
public:
	CUIDragButton( Int32 flags, ui_scroller_align_t alignment, CUICallbackEvent* pAction, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUIDragButton( void );

public:
	// Sets min/max offset from parent
	virtual void setBounds( Int32 start, Int32 end );
	// Sets min/max offset from parent
	virtual void getBounds( Int32& start, Int32& end );

	// Handles a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;
	// Performs think functions
	virtual void think( void ) override;

	// Queries if the element is resizable
	virtual bool isResizable( void ) override { return false; }

	// Sets the position of the button
	virtual void setPosition( Float position );
	// Gets the position of the button
	virtual Double getPosition( void );
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
	ui_scroller_align_t m_alignment;

	// Previous mouse x position while clicked on
	Int32 m_lastMouseX;
	// Previous mouse y position while clicked on
	Int32 m_lastMouseY;

	// Starting offset from parent
	Int32 m_startInset;
	// Offset from parent's other end
	Int32 m_endInset;

	// Current position of the slider
	Double m_position;
	// Last parent length the position was set on
	Uint32 m_lastParentLength;
};

/*
=================================
CUIScroller

=================================
*/
class CUIScroller : public CUITexturedObject
{
public:
	// Arrow button action object
	class CUIScrollerArrowBtnAction : public CUICallbackEvent
	{
		public:
			CUIScrollerArrowBtnAction( CUIScroller* pScroller, Int32 direction, Uint32 jumpSize ):
				m_pScroller(pScroller),
				m_direction(direction),
				m_jumpSize(jumpSize)
			{ };
			virtual ~CUIScrollerArrowBtnAction( void ) { };
			
		public:
			virtual void PerformAction( Float param ) override;

		private:
			// Window that created this
			CUIScroller* m_pScroller;
			// Direction sign
			Int32 m_direction;
			// Jump size
			Uint32 m_jumpSize;
	};
	// Dragger button action
	class CUIScrollerDragBtnAction : public CUICallbackEvent
	{
		public:
			explicit CUIScrollerDragBtnAction( CUIScroller* pScroller ):
				m_pScroller(pScroller)
			{ };
			virtual ~CUIScrollerDragBtnAction( void ) { };
			
		public:
			virtual void PerformAction( Float param ) override;

		private:
			// Window that created this
			CUIScroller* m_pScroller;
	};
public:
	CUIScroller( Int32 flags, ui_scroller_align_t alignment, Uint32 unitSize, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUIScroller( void );

public:
	// Loads the schema, and creates the sub-elements
	virtual bool init( const Char* pstrSchemaName );
	// Tells if the proposed parent size is valid for us
	virtual bool isParentSizeValid( Uint32 testWidth, Uint32 testHeight, Int32 adjX, Int32 adjY ) override;
	// Tells if a child element should be queried on size checks
	virtual bool considerElementOnResize( CUIObject* pObject ) override;

	// Queries if the element is resizable
	virtual bool isResizable( void ) override { return true; }
	// Queries vif the element is interactive
	virtual bool isInteractive( void ) override { return true; }

	// Sets the full range's size
	virtual void setFullRange( Uint32 fullRangeSize );
	// Returns the full range's size
	virtual Uint32 getFullRange( void ) override { return m_fullRangeSize; }
	// Readjusts the scroll drag button's position and size
	virtual void readjustDragButton( void );

	// Adjusts the element's size by x and y amount
	virtual bool adjustSize( Int32 x, Int32 y ) override;
	// Returns the dragger button
	CUIDragButton* getDragButton( void ) { return m_pDragButton; }

	// Retreives the alignment
	virtual ui_scroller_align_t getAlignment( void ) { return m_alignment; }
	// Moves the scroller
	virtual bool moveScroller( Int32 button, Int32 scrollAmount );

protected:
	// Button at beginning of the scroller
	CUIButton* m_pButtonStart;
	// Button at end of the scroller
	CUIButton* m_pButtonEnd;
	// Drag button for the scroller
	CUIDragButton* m_pDragButton;

	// Alignment
	ui_scroller_align_t m_alignment;

	// Total area to consider
	Uint32 m_fullRangeSize;
	// Previous range we considered
	Uint32 m_prevFullRangeSize;
	// Size of a single jump unit
	Uint32 m_unitSize;

	// Scheme object
	const ui_schemeinfo_t* m_pScheme;
};

/*
=================================
CUITabLabel

=================================
*/
class CUITabLabel : public CUIButton
{
public:
	// Width of a tab label
	static const Uint32 TAB_LABEL_HEIGHT;
	// Height of a tab label
	static const Uint32 TAB_LABEL_WIDTH;

public:
	CUITabLabel( Int32 flags, const Char* pstrLabelTitle, const font_set_t* pFont, CUICallbackEvent* pAction, Int32 originx, Int32 originy );
	~CUITabLabel( void );

public:
	// Sets the top border's elements
	virtual bool initBottomElements( void ) override { return true; };
	// Sets the tab body
	virtual void setTabBody( CUITabBody* pTabBody ) { m_pTabBody = pTabBody; }

private:
	// Tab body tied to us
	CUITabBody* m_pTabBody;
};

/*
=================================
CUITabBody

=================================
*/
class CUITabBody : public CUISurface
{
public:
	CUITabBody( Int32 flags, CUITabLabel* pTabLabel, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	~CUITabBody( void );

	// Queries if the element is interactive
	virtual bool isInteractive( void ) override { return true; }
	// Queries if the element is resizable
	virtual bool isResizable( void ) override { return false; }

	// Sets the top border's elements
	virtual bool initTopElements( void ) override;
	// Adjusts the top border
	virtual void adjustTopBorder( void ) override;

	// Sets alpha value for the element
	virtual void setAlpha( Int32 alpha, bool recursive = false ) override;
	// Tells if a child element should be queried on size checks
	virtual bool considerElementOnResize( CUIObject* pObject ) override;

protected:
	// Tab label object's pointer
	CUITabLabel* m_pTabLabel;

	// Top inner left corner element
	CUITexturedObject* m_pTopLeftInnerCorner;
	// Top inner right corner element
	CUITexturedObject* m_pTopRightInnerCorner;
	// Top left border bit
	CUITexturedObject* m_pTopLeftBorderBit;
	// Top right border bit
	CUITexturedObject* m_pTopRightBorderBit;
};

/*
=================================
CUITabList

=================================
*/
class CUITabList : public CUIObject
{
private:
	// Tab body-label pairs
	struct tab_t
	{
		tab_t():
			plabel(nullptr),
			pbody(nullptr)
		{}

		CUITabLabel* plabel;
		CUITabBody* pbody;
	};
	// Dragger button action
	class CUITabSelectCallback : public CUICallbackEvent
	{
		public:
			CUITabSelectCallback( CUITabList* pList, Uint32 tabIndex ):
				m_tabIndex(tabIndex),
				m_pTabListObject(pList)
			{ };
			virtual ~CUITabSelectCallback( void ) { };
			
		public:
			virtual void PerformAction( Float param ) override { m_pTabListObject->showTab(m_tabIndex); }

		private:
			// Index of tab to activate
			Uint32 m_tabIndex;
			// Window that created this
			CUITabList* m_pTabListObject;
	};

public:
	CUITabList( Int32 flags, CUICallbackEvent* pSelectEvent, const font_set_t* pFont, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	~CUITabList( void );

public:
	// Creates a tab object
	CUITabBody* createTab( const Char* pstrName );

	// Queries if the element is interactive
	virtual bool isInteractive( void ) override { return true; }
	// Queries if the element is resizable
	virtual bool isResizable( void ) override { return false; }

	// Returns the number of tabs
	virtual Uint32 getNbTabs( void ) { return m_pTabsArray.size(); }

private:
	// Switches to the tab and hides the rest
	virtual void showTab( Uint32 tabIndex );

public:
	// Array of tab body-label pairs
	CArray<tab_t> m_pTabsArray;
	// Optional event to call when selecting a tab
	CUICallbackEvent* m_pTabSelectEvent;
	// Font set
	const font_set_t* m_pFont;
};

/*
=================================
CUIScrollableSurface

=================================
*/
class CUIScrollableSurface : public CUIObject
{
public:
	CUIScrollableSurface( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUIScrollableSurface( void );

public:
	// Loads the schema, and creates the sub-elements
	virtual bool init( const Char* pstrSchemaName );

	// Queries if the element is interactive
	virtual bool isInteractive( void ) override { return true; }
	// Queries if the element is resizable
	virtual bool isResizable( void ) override { return true; }

	// Sets the scroller offset value(only for text tabs/lists
	virtual void setOffsetValue( Float offset ) override;
	// Adjusts the element's size by x and y amount
	virtual bool adjustSize( Int32 x, Int32 y ) override;

	// Tells if a child element should be queried on size checks
	virtual bool considerElementOnResize( CUIObject* pObject ) override { return false; };
	// Handles a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;
	// Handles a mouse wheel event
	virtual bool mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll ) override;

	// Draws the text on the UI object
	virtual bool draw( void ) override;
	// Performs think functions
	virtual void think( void ) override;

	// Determines if the cursor is touching this object
	virtual bool isMouseOver( Int32 xPos, Int32 yPos ) override;
	// Tells if an element is visible
	virtual bool isChildVisible( CUIObject* pChild, Int32 offset );
	// Tells if an element should be offset
	virtual bool shouldShiftChild( CUIObject* pChild );

protected:
	// Background surface
	CUISurface* m_pSurface;
	// Scroller object
	CUIScroller* m_pScroller;
	// Offset value
	Float m_scrollOffset;
	// Base Y offset
	Uint32 m_baseYOffset;
	// Font set
	const font_set_t* m_pFont;
};

/*
=================================
CUIList

=================================
*/
class CUIList : public CUIScrollableSurface
{
public:
	CUIList( Int32 flags, const font_set_t* pFont, Uint32 rowHeight, Uint32 nbColumns, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUIList( void );

public:
	// Initializes the element
	virtual bool init( const Char* pstrSchemaName ) override;

	// Tells if an element should be offset
	virtual bool shouldShiftChild( CUIObject* pChild ) override;

	// Sets the text for a header column
	virtual void setHeaderColumnName( Uint32 index, const Char* pstrName );
	// Creates a row element and returns it's pointer
	virtual CUIListRow* createNewRow( CUICallbackEvent* pEvent, Uint32 textInset );
	// Creates a row element and returns it's pointer
	virtual void createSeparator( Uint32 textInset, const Char* pstrName, Uint32 height );

	// Sets highlighting on a row
	virtual void setHighlightOnRow( Uint32 index, bool isHighlighted );
	// Sets highlighting on the column of a row
	virtual void setHighlightOnRowColumn( Int32 columnIndex );
	// Gets the text on the column of a row
	virtual CUIObject* getRowColumnObject( Uint32 rowIndex, Uint32 columnIndex );
	// Returns the number of rows in the list
	virtual Int32 getNbRows( void ) { return (Int32)m_rowsArray.size(); }

	// Clears the list
	virtual void clearList( void );
	// Clears highlight
	virtual void clearHighlight( void );

protected:
	// Number of columns in a row
	Uint32 m_nbColumns;
	// Height of a row
	Uint32 m_rowHeight;
	// Optional header element
	CUIListHeader* m_pHeader;
	
	// Array of rows
	CArray<CUIListRow*> m_rowsArray;
	// Array of separators
	CArray<CUIListSeparator*> m_separatorsArray;

	// Combined height of list elements
	Uint32 m_listHeight;
	// Last highlighted element
	CUIListRow* m_pHighlightedRow;
	// Font set
	const font_set_t* m_pListFont;
};

/*
=================================
CUIListHeader

=================================
*/
class CUIListHeader : public CUIObject
{
public:
	struct hdr_column_t
	{
		hdr_column_t():
			psurface(nullptr),
			ptext(nullptr)
		{}

		CString name;
		CUISurface* psurface;
		CUIText* ptext;
	};

public:
	CUIListHeader( Int32 flags, const font_set_t* pFont, Uint32 numColumns, Uint32 width, Uint32 height );
	~CUIListHeader( void );

public:
	// Loads the schema, and creates the sub-elements
	virtual bool init( const Char* pstrSchemaName );
	// Sets the name of a column
	virtual void setColumnName( Uint32 index, const Char* pstrName );
	// Queries if the element is interactive
	virtual bool isInteractive( void ) override { return false; }

protected:
	// Array of columns
	CArray<hdr_column_t> m_columns;
	// Font set
	const font_set_t* m_pFont;
};

/*
=================================
CUIListRow

=================================
*/
class CUIListRow : public CUIObject
{
public:
	CUIListRow( Int32 flags, Uint32 rowIndex, Uint32 textInset, CUIListHeader* pHeader, CUICallbackEvent* pEvent, Uint32 numColumns, Uint32 rowOffs, Uint32 width, Uint32 height );
	~CUIListRow( void );

public:
	// Queries if the element is interactive
	virtual bool isInteractive( void ) override { return true; }
	// Sets the text on a column
	virtual void setColumnContents( Uint32 index, CUIObject* pObject );
	// Gets the text on a column
	virtual CUIObject* getColumnContents( Uint32 index );
	// Sets the text color
	virtual void setColumnColor( byte r, byte g, byte b );

	// Sets the highlight status on the element
	virtual void setHighlight( bool highlighted ) { m_isHighlighted = highlighted; }
	// Sets the highlight status on the element's column
	virtual void setHighlightedColumn( Int32 columnIdx ) { m_highlightedColumnIdx = columnIdx; }

	// Draws the text on the UI object
	virtual bool draw( void ) override;

	// Returns the row index
	virtual Uint32 getIndex( void ) { return m_rowIndex; }
	// Handles a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;

public:
	// Index of this row object
	Uint32 m_rowIndex;
	// List header object
	CUIListHeader* m_pHeader;
	// Texts for the columns
	CArray<CUIObject*> m_columns;
	// Column contents
	CArray<CUIObject*> m_columnContents;
	// Callback event tied to this row
	CUICallbackEvent* m_pEvent;

	// TRUE if we're being highlighted
	bool m_isHighlighted;
	// Index of the highlighted column
	Int32 m_highlightedColumnIdx;
};

/*
=================================
CUIListSeparator

=================================
*/
class CUIListSeparator : public CUIListRow
{
public:
	CUIListSeparator( Int32 flags, Uint32 textInset, Uint32 rowOffs, Uint32 width, Uint32 height );
	~CUIListSeparator( void );

public:
	// Queries if the element is interactive
	virtual bool isInteractive( void ) override { return true; }
	// Draws the text on the UI object
	virtual bool draw( void ) override;
};

/*
=================================
CUIDropDownList

=================================
*/
class CUIDropDownList : public CUISurface
{
private:
	// Dropbown button event object
	class CUIDropDownButtonEvent : public CUICallbackEvent
	{
		public:
			explicit CUIDropDownButtonEvent( CUIDropDownList* pList ):
				m_pList(pList)
			{ };
			virtual ~CUIDropDownButtonEvent( void ) { };
			
		public:
			virtual void PerformAction( Float param ) override;

		private:
			// Window that created this
			CUIDropDownList* m_pList;
	};
	// Dropbown button event object
	class CUIDropDownRowClickEvent : public CUICallbackEvent
	{
		public:
			CUIDropDownRowClickEvent( CUIDropDownList* pList, Uint32 rowIndex ):
				m_rowIndex(rowIndex),
				m_pList(pList),
				m_wasClickedOn(false)
			{ };
			virtual ~CUIDropDownRowClickEvent( void ) { };
			
		public:
			virtual void PerformAction( Float param ) override { };
			virtual bool MouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;

		private:
			// Index of the row tied to this event
			Uint32 m_rowIndex;
			// Window that created this
			CUIDropDownList* m_pList;
			// TRUE if we got clicked on
			bool m_wasClickedOn;
	};

public:
	// Default width for the dropdown list
	static const Uint32 DROPDOWN_LIST_WIDTH;
	// Default width for the dropdown list
	static const Uint32 DROPDOWN_LIST_HEIGHT;

public:
	CUIDropDownList( Int32 flags, CUICallbackEvent* pEvent, CUICallbackEvent* pListToggleEvent, const font_set_t* pFont, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	~CUIDropDownList( void );

public:
	// Queries if the element is interactive
	virtual bool isInteractive( void ) override { return true; }
	// Queries if the element is resizable
	virtual bool isResizable( void ) override { return false; }
	// Loads the schema, and creates the sub-elements
	virtual bool init( const Char* pstrSchemaName ) override;
	// Handles a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;
	// Requests that other tab objects be closed
	virtual void closeOtherTabs( CUIObject* pCaller ) override;
	// Tells if the mouse is over this object
	virtual bool isMouseOver( Int32 xPos, Int32 yPos ) override;

public:
	// Toggles the list
	virtual void toggleList( void );
	// Clears the list
	virtual void clearList( void ) { m_pDropdownList->clearList(); }
	// Returns the list size
	virtual Uint32 getListSize( void ) { return m_pDropdownList->getNbRows(); }

	// Sets the current value
	virtual void setSelection( Int32 rowIndex );
	// Adds a choice to the list
	virtual void addChoice( const Char* pstrText );
	// Manages a selection event
	virtual void manageSelectionEvent( Uint32 rowIndex );

protected:
	// Text displaying the current value
	CUIText* m_pCurrentValue;
	// Dropdown button
	CUIButton* m_pDropdownButton;
	// The list object, normally hidden
	CUIList* m_pDropdownList;

	// Current selection index
	Int32 m_selectionIdx;
	// Action event to call when selecting
	CUICallbackEvent* m_pEvent;
	// Font set used
	const font_set_t* m_pFont;

	// Event pointer for Toggle
	CUICallbackEvent* m_pListToggleEvent;
};

/*
=================================
CUITickBox

=================================
*/
class CUITickBox : public CUIButton
{
public:
	CUITickBox( Int32 flags, CUICallbackEvent* pAction, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUITickBox( void );

public:
	// Sets the text
	virtual void setText( const Char* pstrText, const font_set_t* pFont, Uint32 textOffset );
	// Loads the schema, and creates the sub-elements
	virtual bool init( const Char* pstrSchemaName ) override;
	// Sets the state of the tick box
	virtual void setState( bool isEnabled );
	// Handles a mouse button event
	virtual bool mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown ) override;

protected:
	// Can be text or an icon
	CUIObject* m_pTickFlagObject;
	// Flag for toggling state
	bool m_isEnabled;
};

/*
=================================
CUISlider

=================================
*/
class CUISlider : public CUIObject
{
public:
	// Dragger button action
	class CUISliderDragBtnAction : public CUICallbackEvent
	{
		public:
			explicit CUISliderDragBtnAction( CUISlider* pSlider ):
				m_pSlider(pSlider)
			{ };
			virtual ~CUISliderDragBtnAction( void ) { };
			
		public:
			virtual void PerformAction( Float param ) override;

		private:
			// Window that created this
			CUISlider* m_pSlider;
	};

public:
	CUISlider( Int32 flags, CUICallbackEvent* pEvent, Uint32 width, Uint32 height, Int32 originx, Int32 originy, Float minValue, Float maxValue, Float markerdistance );
	virtual ~CUISlider( void );

public:
	// Draws the text on the UI object
	virtual bool draw( void ) override;
	// Queries if the element is interactive
	virtual bool isInteractive( void ) override { return true; }
	// Loads the schema, and creates the sub-elements
	virtual bool init( const Char* pstrSchemaName );
	
	// Retreives the drag button object
	virtual CUIDragButton* getSliderButton( void ) { return m_pSliderButton; }
	// Sets the value of the slider based on the position
	virtual void setValueFromPosition( Double position );
	// Sets position and value based on 0-1 range
	virtual void setValue( Float value );

private:
	// Minimum value
	Float m_minValue;
	// Maximum value
	Float m_maxValue;
	// Distance between markers
	Float m_markerDistance;
	// Current value
	Float m_value;

private:
	// Callback event
	CUICallbackEvent* m_pEvent;
	// Slider button object
	CUIDragButton* m_pSliderButton;
	// Slider left object
	CUITexturedObject* m_pLeftObject;
	// Slider body object
	CUITexturedObject* m_pBodyObject;
	// Slider right object
	CUITexturedObject* m_pRightObject;
};

/*
=================================
CUIProgressBar

=================================
*/
class CUIProgressBar : public CUISurface
{
public:
	CUIProgressBar( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
	virtual ~CUIProgressBar( void );

public:
	// Draws the text on the UI object
	virtual bool draw( void ) override;
	// Sets the meter value
	virtual void setValue( Float value );

private:
	// Meter value
	Float m_meterValue;
};

#endif //UIELEMENTS_H