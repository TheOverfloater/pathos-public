/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_MENU_H
#define R_MENU_H

class CMenuButton;
class CUIWindow;

struct font_set_t;
struct en_texture_t;

/*
=================================
-Class: CMenu
-Description:

=================================
*/
class CMenu
{
public:
	static const Int32 MENU_TITLE_XPOS;
	static const Int32 MENU_TITLE_YPOS;

	static const Int32 MENU_BUTTONS_XPOS;
	static const Int32 MENU_BUTTONS_YPOS;
	static const Int32 MENU_BUTTONS_GAP;

	static const Int32 MENU_BUTTON_FONTSIZE;

	static const Int32 MENU_BASE_WIDTH;
	static const Int32 MENU_BASE_HEIGHT;

	static const Float MENU_BLEND_TIME;
	static const Float MENU_POST_INIT_BLEND_TIME;

	static const Char MENU_HOVER_SOUND[];
	static const Char MENU_CLICK_SOUND[];
	static const Char MENU_MUSIC_FILE_STARTUP[];
	static const Char MENU_MUSIC_FILE_INGAME[];

	static const Char MENU_BUTTON_TEXT_SCHEMA[];

public:
	// Menu button actions
	enum mbutton_t
	{
		MENU_BTN_FIRST = 0,
		MENU_BTN_CONSOLE = MENU_BTN_FIRST,
		MENU_BTN_RESUMEGAME,
		MENU_BTN_CONTINUE,
		MENU_BTN_NEWGAME,
		MENU_BTN_LOADGAME,
		MENU_BTN_SAVELOADGAME,
		MENU_BTN_SETTINGS,
		MENU_BTN_QUIT,
		NB_MENU_BTN
	};

	// Renderer result codes returned by rendering
	enum rendercode_t
	{
		RC_OK = 0,
		RC_BASICDRAW_FAIL,
		RC_MENUPARTICLES_FAIL,
		RC_TEXT_FAIL
	};

	enum menu_music_type_t
	{
		MENU_MUSIC_UNDEFINED = 0,
		MENU_MUSIC_STARTUP,
		MENU_MUSIC_INGAME
	};

public:
	CMenu( void );
	~CMenu( void );

public:
	// Loads the required fonts and texture
	bool Init( void );

	// Called when initializing OpenGL
	bool InitGL( void );
	// Called on OpenGL cleanup
	void ClearGL( void );

	// Draws the menu
	rendercode_t Draw( void );
	// Draws the menu boot-up fade
	bool DrawMenuFade( void );
	// Think function
	void Think( void );

	// Brings up the menu
	void ShowMenu( void );
	// Hides the menu
	void HideMenu( void );
	// Tells if the menu is up
	bool IsActive( void ) const { return m_isActive; }

	// Performs a button action
	void ButtonAction( mbutton_t buttonId );
	// Handles an input event
	void MouseButtonEvent( Int32 button, bool keyDown );
	// Manages a key event
	bool KeyEvent( Int32 button, Int16 mod, bool keyDown );
	// Manages a mouse wheel event
	static bool MouseWheelEvent( Int32 button, bool keyDown, Int32 scroll ) { return false; };

	// Called when the cursor just hovered over a button
	void CursorOverButton( mbutton_t buttonId );
	// Called when the cursor just hovered away from a button
	void CursorLeaveButton( mbutton_t buttonId );

	// Updates state of the contine button
	void UpdateContineButton( void );
	// Clears latest save file info
	void ClearLatestSaveFile( void );

	// Sets blending target texture
	void SetBlendTargetTexture( en_texture_t* ptexture );
	// Returns the loading texture to use if menu is active
	en_texture_t* GetLoadingTexture( void );
	// Resets menu background blending
	void ResetMenuBackground( void );

	// Plays a menu sound
	static void PlayMenuSound( const Char* pstrSample );

	// Sets whether the mouse should be hidden on return to game
	void SetShouldHideMouse( bool shouldHide ) { m_shouldHideMouse = shouldHide; }
	
	// Releases save background textures
	void FreeSaveBackgroundTextures( void );
	// Add a save background texture to the list
	void AddSaveBackgroundTexture( en_texture_t* ptexture );
	// Calls to make sure a deleted texture is not used
	void OnBgTextureDeleted( en_texture_t* ptexture );

	// Sets current bg texture
	void SetCurrentBgTexture( en_texture_t* ptexture );
	// Returns current bg texture
	en_texture_t* GetCurrentBgTexture( void );

	// Tells menu to blend the bootup screen
	void InitialStartup( void );
	// Pauses menu music playback
	void PauseMenuMusic( void );
	// Resumes menu music playback
	void ResumeMenuMusic( void );

private:
	// Positions the menu buttons
	void SetButtons( void );

	// Draws the menu background
	bool DrawMenuBackground( class CBasicDraw* pDraw );
	// Draws menu elements
	rendercode_t DrawMenuElements( class CBasicDraw* pDraw );

private:
	// TRUE if menu should be displayed
	bool m_isActive;
	// Menu title logo texture
	en_texture_t* m_pTitleLogoTexture;
	// Menu background texture
	en_texture_t* m_pBackgroundTexture;
	// Menu background texture
	en_texture_t* m_pBgBlurredTexture;
	// Font used by the menu for buttons
	const font_set_t* m_pButtonFont;
	// Font used by the menu for the title text
	const font_set_t* m_pTitleFont;

private:
	// Current background texture
	en_texture_t* m_pCurrentBackgroundTexture;
	// Blend from texture
	en_texture_t* m_pBlendFromTexture;
	// Blend-to texture
	en_texture_t* m_pBlendToTexture;
	// Texture to blend to after current blend finishes
	en_texture_t* m_pNextBlendTexture;

	// Blend begin time
	Double m_flBlendBeginTime;

	// TRUE if we should hide them mouse on return to game
	bool m_shouldHideMouse;

	// List of save background textures
	CLinkedList<en_texture_t*> m_saveBgTexturesList;

	// Post-initialization blend time
	Double m_postInitBlendBeginTime;
	// TRUE if menu music is playing
	bool m_isMenuMusicPlaying;

private:
	// Latest save file name
	CString m_latestSaveFileName;
	// Latest save file bg texture
	en_texture_t* m_pLatestSaveFileBgTexture;

	// Type of menu music playing
	menu_music_type_t m_playingMusicType;

private:
	// Menu button array
	CArray<CMenuButton*> m_buttonsArray;
};
extern CMenu gMenu;

/*
=================================
-Class: CMenuButton
-Description:

=================================
*/
class CMenuButton
{
private:
	static const Float DEFAULT_BRIGHTNESS;
	static const Float HOVER_BRIGHTNESS;
	static const Float DISABLED_BRIGHTNESS;

	static const Float DEFAULT_ALPHA;
	static const Float HOVER_ALPHA;
	
	static const Float FADE_TIME;
	static const Float CLICK_OFFSET;

public:
	CMenuButton( CMenu *pMenu, CMenu::mbutton_t buttonId, const Char* pstrText, SDL_Scancode scancode );
	~CMenuButton( void );

public:
	// Performs thinkinking code
	void Think( void );
	// Draws the string
	bool Draw( const font_set_t* pFont );

	// Determines if the cursor is over the button
	bool IsCursorOver( Int32 cursorX, Int32 cursorY ) const;
	// Tells if the button is hidden
	bool IsHidden( void ) const { return m_isHidden; }
	// Tells if the button is enabled
	bool IsEnabled( void ) const { return m_isEnabled; }

	// Performs the button's action
	bool MouseButtonEvent( Int32 button, bool keyDown );
	// Manages a key event
	bool KeyEvent( Int32 button, Int16 mod, bool keyDown );

	// Sets the properties of the button
	void SetProperties( Int32 xPos, Int32 yPos, Uint32 width, Uint32 height );
	// Returns the text used
	const Char* GetString( void ) const { return m_text.c_str(); }
	// Returns the height of the button
	Uint32 GetHeight( void ) const { return m_sizeY; }

	// Sets enabled state
	void SetEnabled( bool enabled ) { m_isEnabled = enabled; }
	// Sets hidden staye
	void SetHidden( bool hidden ) { m_isHidden = hidden; }

private:
	// Text to display
	CString m_text;
	// Brightness value
	Float m_brightness;
	// Alpha value
	Float m_alpha;

	// Origin
	Int32 m_originX;
	Int32 m_originY;

	// Size
	Uint32 m_sizeX;
	Uint32 m_sizeY;

	// TRUE if the button was clicked on
	bool m_isClicked;
	// Determines if the button is available
	bool m_isEnabled;
	// Determines if the button is hidden
	bool m_isHidden;
	// Tells if the glow sound was played
	bool m_glowSoundPlayed;
	// TRUE if cursor is over us
	bool m_isCursorOver;

	// Scancode tied to this button
	SDL_Scancode m_scancode;

	// Menu object
	CMenu* m_pMenu;
	// Menu button id
	CMenu::mbutton_t m_buttonId;
};
#endif //R_MENU_H