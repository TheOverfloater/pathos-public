/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "window.h"
#include "system.h"
#include "config.h"
#include "enginestate.h"
#include "input.h"

#include "texturemanager.h"
#include "r_menu.h"

#include "r_vbo.h"
#include "r_glsl.h"
#include "r_basicdraw.h"
#include "r_main.h"
#include "r_text.h"
#include "r_common.h"
#include "r_menuparticles.h"

#include "uielements.h"
#include "uimanager.h"

#include "uiconsolewindow.h"
#include "uisettingswindow.h"
#include "uiexitwindow.h"
#include "uisaveloadwindow.h"
#include "uinewgamewindow.h"
#include "cl_main.h"

#include "console.h"
#include "edict.h"
#include "sv_main.h"
#include "cl_snd.h"
#include "saverestore.h"
#include "commands.h"
#include "textschemas.h"

// Brightness values for the menu button
const Float CMenuButton::DEFAULT_BRIGHTNESS = 0.75;
const Float CMenuButton::HOVER_BRIGHTNESS = 1.0;
const Float CMenuButton::DISABLED_BRIGHTNESS = 0.25;

// Alpha values for the menu buttons
const Float CMenuButton::DEFAULT_ALPHA = 0.75;
const Float CMenuButton::HOVER_ALPHA = 1.0;

// Time it takes for the button to fade to a new color
const Float CMenuButton::FADE_TIME = 0.75;

// How many pixels to offset by when clicked
const Float CMenuButton::CLICK_OFFSET = 4;

// Menu button position relative to 1024x768 resolution
const Int32 CMenu::MENU_TITLE_XPOS = 50;
const Int32 CMenu::MENU_TITLE_YPOS = 50;

// Menu title position relative to 1024x768 resolution
const Int32 CMenu::MENU_BUTTONS_XPOS = 80;
const Int32 CMenu::MENU_BUTTONS_YPOS = 270;
const Int32 CMenu::MENU_BUTTONS_GAP = 30;

// Menu title position relative to 1024x768 resolution
const Int32 CMenu::MENU_BASE_WIDTH = 1024;
const Int32 CMenu::MENU_BASE_HEIGHT = 768;

// Menu font size relative to 1024x768 resolution
const Int32 CMenu::MENU_BUTTON_FONTSIZE = 36;

// Blend time for background texture
const Float CMenu::MENU_BLEND_TIME = 0.25f;
// Post-init blend time for menu
const Float CMenu::MENU_POST_INIT_BLEND_TIME = 2;

// Menu sounds
const Char CMenu::MENU_HOVER_SOUND[] = "menu/button_glow.wav";
const Char CMenu::MENU_CLICK_SOUND[] = "menu/button_click.wav";
const Char CMenu::MENU_MUSIC_FILE_STARTUP[] = "music/gamestartup.ogg";
const Char CMenu::MENU_MUSIC_FILE_INGAME[] = "music/menumusic_game.ogg";

// Menu button font schema
const Char CMenu::MENU_BUTTON_TEXT_SCHEMA[] = "menubuttons";

// Menu object
CMenu gMenu;

//=============================================
// Class: CMenu
// Function: CMenu
//=============================================
CMenu::CMenu( void ):
	m_isActive(false),
	m_pTitleLogoTexture(nullptr),
	m_pBackgroundTexture(nullptr),
	m_pBgBlurredTexture(nullptr),
	m_pButtonFont(nullptr),
	m_pTitleFont(nullptr),
	m_pCurrentBackgroundTexture(nullptr),
	m_pBlendFromTexture(nullptr),
	m_pBlendToTexture(nullptr),
	m_pNextBlendTexture(nullptr),
	m_flBlendBeginTime(0),
	m_shouldHideMouse(true),
	m_postInitBlendBeginTime(0),
	m_isMenuMusicPlaying(false),
	m_pLatestSaveFileBgTexture(nullptr),
	m_playingMusicType(MENU_MUSIC_UNDEFINED)
{
	CString menuBtnNames[NB_MENU_BTN] = {
		"Console",
		"Resume Game",
		"Continue",
		"New Game",
		"Load Game",
		"Save/Load Game",
		"Settings",
		"Quit"
	};

	SDL_Scancode scanCodes[NB_MENU_BTN] = {
		SDL_SCANCODE_C,
		SDL_SCANCODE_R,
		SDL_SCANCODE_W,
		SDL_SCANCODE_N,
		SDL_SCANCODE_L,
		SDL_SCANCODE_G,
		SDL_SCANCODE_S,
		SDL_SCANCODE_Q
	};

	// Create the buttons
	m_buttonsArray.resize(NB_MENU_BTN);
	for(Uint32 i = 0; i < NB_MENU_BTN; i++)
		m_buttonsArray[i] = new CMenuButton(this, (mbutton_t)(MENU_BTN_FIRST+i), menuBtnNames[i].c_str(), scanCodes[i]);

	// Disabled save game by default
	m_buttonsArray[MENU_BTN_RESUMEGAME]->SetEnabled(false);
	m_buttonsArray[MENU_BTN_RESUMEGAME]->SetHidden(true);
	m_buttonsArray[MENU_BTN_SAVELOADGAME]->SetEnabled(false);
	m_buttonsArray[MENU_BTN_SAVELOADGAME]->SetHidden(true);
}

//=============================================
// Class: CMenu
// Function: ~CMenu
//=============================================
CMenu::~CMenu( void )
{
	for(Uint32 i = 0; i < NB_MENU_BTN; i++)
		delete m_buttonsArray[i];
}

//=============================================
// Class: CMenu
// Function: Init
//=============================================
bool CMenu::Init( void )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Remove the previous textures
	if(m_pBackgroundTexture && m_pBackgroundTexture != pTextureManager->GetDummyTexture())
		pTextureManager->DeleteTexture(m_pBackgroundTexture->filepath.c_str());

	if(m_pBgBlurredTexture && m_pBgBlurredTexture != pTextureManager->GetDummyTexture())
		pTextureManager->DeleteTexture(m_pBgBlurredTexture->filepath.c_str());

	// Determine which texture to use
	Float aspectRatio = (Float)gWindow.GetWidth()/(Float)gWindow.GetHeight();
	if(aspectRatio > 1.5)
	{
		m_pBackgroundTexture = pTextureManager->LoadTexture("menu/background_widescreen.dds", RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS);
		m_pBgBlurredTexture = pTextureManager->LoadTexture("menu/background_widescreen_blur.dds", RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS);
	}
	else
	{
		m_pBackgroundTexture = pTextureManager->LoadTexture("menu/background.dds", RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS);
		m_pBgBlurredTexture = pTextureManager->LoadTexture("menu/background_blur.dds", RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS);
	}

	if(!m_pBackgroundTexture)
	{
		m_pBackgroundTexture = pTextureManager->GetDummyTexture();
		Con_EPrintf("Failed to load background texture.\n");
	}

	if(!m_pBgBlurredTexture)
	{
		m_pBgBlurredTexture = pTextureManager->GetDummyTexture();
		Con_EPrintf("Failed to load background texture.\n");
	}

	// Load the title texture
	if(!m_pTitleLogoTexture)
	{
		m_pTitleLogoTexture = pTextureManager->LoadTexture("menu/title_logo.tga", RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS);

		if(!m_pTitleLogoTexture)
		{
			m_pTitleLogoTexture = pTextureManager->GetDummyTexture();
			Con_EPrintf("Failed to load title logo texture.\n");
		}
	}

	// Determine button font to use
	m_pButtonFont = gTextSchemas.GetResolutionSchemaFontSet(MENU_BUTTON_TEXT_SCHEMA, gWindow.GetHeight());
	if(!m_pButtonFont)
	{
		Int32 idealFontSize = (Uint32)R_GetRelativeY(MENU_BUTTON_FONTSIZE, MENU_BASE_HEIGHT, gWindow.GetHeight());
		m_pButtonFont = gText.LoadFont("albertus.ttf", idealFontSize, true, nullptr, 2);
	}

	if(!m_pButtonFont)
	{
		Con_EPrintf("Failed to load menu button font.\n");
		return false;
	}

	// Set bg texture ptr
	m_pCurrentBackgroundTexture = m_pBackgroundTexture;

	// Precache the sounds
	gSoundEngine.PrecacheSound(MENU_CLICK_SOUND, -1, RS_APP_LEVEL, false);
	gSoundEngine.PrecacheSound(MENU_HOVER_SOUND, -1, RS_APP_LEVEL, false);

	// Precache music files
	CString menuMusicPath;
	menuMusicPath << SOUND_FOLDER_BASE_PATH << MENU_MUSIC_FILE_STARTUP;
	gSoundEngine.PrecacheOgg(menuMusicPath.c_str(), RS_APP_LEVEL);

	menuMusicPath.clear();
	menuMusicPath << SOUND_FOLDER_BASE_PATH << MENU_MUSIC_FILE_INGAME;
	gSoundEngine.PrecacheOgg(menuMusicPath.c_str(), RS_APP_LEVEL);

	return true;
}

//=============================================
// Class: CMenu
// Function: ClearGL
//=============================================
bool CMenu::InitGL( void )
{
	if(!IsActive())
		return true;

	UpdateContineButton();
	return true;
}

//=============================================
// Class: CMenu
// Function: ClearGL
//=============================================
void CMenu::ClearGL( void )
{
	FreeSaveBackgroundTextures();
}

//=============================================
// Class: CMenu
// Function: PlayMenuSound
//=============================================
void CMenu::PlayMenuSound( const Char* pstrSample )
{
	gSoundEngine.PlaySound(pstrSample, nullptr, SND_FL_MENU, SND_CHAN_AUTO);
}

//=============================================
// Class: CMenu
// Function: MouseButtonEvent
//=============================================
void CMenu::MouseButtonEvent( Int32 button, bool keyDown )
{
	if(!IsActive())
		return;

	// Fix: Do not allow mouse events while loading
	if(ens.gamestate == GAME_LOADING)
		return;

	// Check which button was clicked on
	for(Uint32 i = 0; i < NB_MENU_BTN; i++)
	{
		if(m_buttonsArray[i]->MouseButtonEvent(button, keyDown))
			return;
	}
}

//=============================================
// Class: CMenu
// Function: KeyEvent
//=============================================
bool CMenu::KeyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(!IsActive())
		return false;

	// Fix: Do not allow key events while loading
	if(ens.gamestate == GAME_LOADING)
		return false;

	// Only allow toggling the menu when we're ingame
	if(ens.gamestate == GAME_RUNNING && keyDown)
	{
		if(button == SDL_SCANCODE_ESCAPE)
		{
			HideMenu();
			return true;
		}
	}

	// See if we have any buttons mapped
	for(Uint32 i = 0; i < NB_MENU_BTN; i++)
	{
		if(m_buttonsArray[i]->KeyEvent(button, mod, keyDown))
			return true;
	}

	return false;
}

//=============================================
// Class: CMenu
// Function: CreateButtons
//=============================================
void CMenu::SetButtons( void )
{
	// Get base positions
	Int32 xPos = R_GetRelativeX(MENU_BUTTONS_XPOS, MENU_BASE_WIDTH, gWindow.GetWidth());
	Int32 yPos = R_GetRelativeY(MENU_BUTTONS_YPOS, MENU_BASE_HEIGHT, gWindow.GetHeight());
	Int32 btnGap = R_GetRelativeY(MENU_BUTTONS_GAP, MENU_BASE_HEIGHT, gWindow.GetHeight());

	// Hide/show buttons
	if(ens.gamestate != GAME_RUNNING)
	{
		m_buttonsArray[MENU_BTN_RESUMEGAME]->SetEnabled(false);
		m_buttonsArray[MENU_BTN_RESUMEGAME]->SetHidden(true);
		m_buttonsArray[MENU_BTN_SAVELOADGAME]->SetEnabled(false);
		m_buttonsArray[MENU_BTN_SAVELOADGAME]->SetHidden(true);
		m_buttonsArray[MENU_BTN_LOADGAME]->SetEnabled(true);
		m_buttonsArray[MENU_BTN_LOADGAME]->SetHidden(false);
	}
	else
	{
		m_buttonsArray[MENU_BTN_RESUMEGAME]->SetEnabled(true);
		m_buttonsArray[MENU_BTN_RESUMEGAME]->SetHidden(false);
		m_buttonsArray[MENU_BTN_SAVELOADGAME]->SetEnabled(true);
		m_buttonsArray[MENU_BTN_SAVELOADGAME]->SetHidden(false);
		m_buttonsArray[MENU_BTN_LOADGAME]->SetEnabled(false);
		m_buttonsArray[MENU_BTN_LOADGAME]->SetHidden(true);
		m_buttonsArray[MENU_BTN_CONTINUE]->SetEnabled(false);
		m_buttonsArray[MENU_BTN_CONTINUE]->SetHidden(true);
	}

	for(Uint32 i = 0; i < NB_MENU_BTN; i++)
	{
		CMenuButton* pButton = m_buttonsArray[i];
		if(pButton->IsHidden())
			continue;

		Int32 ymin;
		Uint32 width, height;
		gText.GetStringSize(m_pButtonFont, pButton->GetString(), &width, &height, &ymin);

		pButton->SetProperties(xPos, yPos, width, height);
		yPos += height + btnGap;
	}
}

//=============================================
// Class: CMenu
// Function: Draw
//=============================================
CMenu::rendercode_t CMenu::Draw( void )
{
	if(!m_isActive)
		return RC_OK;

	CBasicDraw* pDraw = CBasicDraw::GetInstance();

	// Draw the main background
	if(!DrawMenuBackground(pDraw))
		return RC_BASICDRAW_FAIL;

	// Draw menu particles
	pDraw->Disable();
	if(!gMenuParticles.Draw())
		return RC_MENUPARTICLES_FAIL;

	// Re-enable basic draw
	if(!pDraw->Enable())
		return RC_BASICDRAW_FAIL;

	// Draw the various menu elements
	rendercode_t result = DrawMenuElements(pDraw);
	return result;
}

//=============================================
// Class: CMenu
// Function: DrawMenuBackground
//=============================================
bool CMenu::DrawMenuBackground( CBasicDraw* pDraw )
{
	assert(pDraw->IsActive());

	// Set matrices
	rns.view.modelview.LoadIdentity();

	rns.view.projection.LoadIdentity();
	rns.view.projection.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, (Float)0.1, 100);

	pDraw->SetModelview(rns.view.modelview.GetMatrix());
	pDraw->SetProjection(rns.view.projection.GetMatrix());

	if(!pDraw->EnableTexture() || !pDraw->DisableRectangleTexture())
		return false;

	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	// Draw the background
	if(m_flBlendBeginTime && m_pBlendFromTexture && m_pBlendToTexture)
	{
		// Adjust for rectangle textures if needed
		Float tc1ymod, tc1scalex, tc1scaley;
		if(m_pBlendFromTexture->flags & TX_FL_RECTANGLE)
		{
			tc1ymod = 1.0;
			tc1scalex = m_pBlendFromTexture->width;
			tc1scaley = m_pBlendFromTexture->height;

			R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pBlendFromTexture->palloc->gl_index);
			if(!pDraw->EnableRectangleTexture())
				return false;
		}
		else
		{
			tc1ymod = 0.0;
			tc1scalex = 1.0;
			tc1scaley = 1.0;

			R_Bind2DTexture(GL_TEXTURE0_ARB, m_pBlendFromTexture->palloc->gl_index);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Draw the blend-from texture
		Float alpha = 1.0 - ((ens.time - m_flBlendBeginTime) / MENU_BLEND_TIME);
		alpha = clamp(alpha, 0.0, 1.0);
		
		pDraw->Color4f(GL_ONE, GL_ONE, GL_ONE, alpha);

		R_ValidateShader(pDraw);

		pDraw->Begin(GL_TRIANGLES);
		pDraw->TexCoord2f(0.0, tc1ymod*tc1scaley);
		pDraw->Vertex3f(0.0, 0.0, -1.0);

		pDraw->TexCoord2f(1.0*tc1scalex, tc1ymod*tc1scaley);
		pDraw->Vertex3f(1.0, 0.0, -1.0);

		pDraw->TexCoord2f(0.0, (1.0-tc1ymod)*tc1scaley);
		pDraw->Vertex3f(0.0, 1.0, -1.0);

		pDraw->TexCoord2f(1.0*tc1scalex, tc1ymod*tc1scaley);
		pDraw->Vertex3f(1.0, 0.0, -1.0);

		pDraw->TexCoord2f(0.0, (1.0-tc1ymod)*tc1scaley);
		pDraw->Vertex3f(0.0, 1.0, -1.0);

		pDraw->TexCoord2f(1.0*tc1scalex, (1.0-tc1ymod)*tc1scaley);
		pDraw->Vertex3f(1.0, 1.0, -1.0);
		pDraw->End();

		if(m_pBlendFromTexture->flags & TX_FL_RECTANGLE)
		{
			if(!pDraw->DisableRectangleTexture())
				return false;
		}

		// Bind the blend-to texture
		Float tc2ymod, tc2scalex, tc2scaley;
		if(m_pBlendToTexture->flags & TX_FL_RECTANGLE)
		{
			tc2ymod = 1.0;
			tc2scalex = m_pBlendToTexture->width;
			tc2scaley = m_pBlendToTexture->height;

			R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pBlendToTexture->palloc->gl_index);
			if(!pDraw->EnableRectangleTexture())
				return false;
		}
		else
		{
			tc2ymod = 0.0;
			tc2scalex = 1.0;
			tc2scaley = 1.0;
			
			R_Bind2DTexture(GL_TEXTURE0_ARB, m_pBlendToTexture->palloc->gl_index);
		}

		// Do not recalculate this, insteadj ust use 1.0 - alpha
		pDraw->Color4f(GL_ONE, GL_ONE, GL_ONE, (1.0 - alpha));

		R_ValidateShader(pDraw);

		pDraw->Begin(GL_TRIANGLES);
		pDraw->TexCoord2f(0.0, tc2ymod*tc2scaley);
		pDraw->Vertex3f(0.0, 0.0, -1.0);

		pDraw->TexCoord2f(1.0*tc2scalex, tc2ymod*tc2scaley);
		pDraw->Vertex3f(1.0, 0.0, -1.0);

		pDraw->TexCoord2f(0.0, (1.0-tc2ymod)*tc2scaley);
		pDraw->Vertex3f(0.0, 1.0, -1.0);

		pDraw->TexCoord2f(1.0*tc2scalex, tc2ymod*tc2scaley);
		pDraw->Vertex3f(1.0, 0.0, -1.0);

		pDraw->TexCoord2f(0.0, (1.0-tc2ymod)*tc2scaley);
		pDraw->Vertex3f(0.0, 1.0, -1.0);

		pDraw->TexCoord2f(1.0*tc2scalex, (1.0-tc2ymod)*tc2scaley);
		pDraw->Vertex3f(1.0, 1.0, -1.0);
		pDraw->End();

		glDisable(GL_BLEND);

		if(m_pBlendToTexture->flags & TX_FL_RECTANGLE)
		{
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
			if(!pDraw->DisableRectangleTexture())
				return false;
		}
	}
	else
	{
		// Adjust for rectangle textures if needed
		Float tcymod, tcscalex, tcscaley;
		if(m_pCurrentBackgroundTexture->flags & TX_FL_RECTANGLE)
		{
			tcymod = 1.0;
			tcscalex = m_pCurrentBackgroundTexture->width;
			tcscaley = m_pCurrentBackgroundTexture->height;

			R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pCurrentBackgroundTexture->palloc->gl_index);
			if(!pDraw->EnableRectangleTexture())
				return false;
		}
		else
		{
			tcymod = 0.0;
			tcscalex = 1.0;
			tcscaley = 1.0;

			// Just bind the current texture
			R_Bind2DTexture(GL_TEXTURE0_ARB, m_pCurrentBackgroundTexture->palloc->gl_index);
		}

		pDraw->Color4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

		R_ValidateShader(pDraw);

		pDraw->Begin(GL_TRIANGLES);
		pDraw->TexCoord2f(0.0, tcymod*tcscaley);
		pDraw->Vertex3f(0.0, 0.0, -1.0);

		pDraw->TexCoord2f(1.0*tcscalex, tcymod*tcscaley);
		pDraw->Vertex3f(1.0, 0.0, -1.0);

		pDraw->TexCoord2f(0.0, (1.0-tcymod)*tcscaley);
		pDraw->Vertex3f(0.0, 1.0, -1.0);

		pDraw->TexCoord2f(1.0*tcscalex, tcymod*tcscaley);
		pDraw->Vertex3f(1.0, 0.0, -1.0);

		pDraw->TexCoord2f(0.0, (1.0-tcymod)*tcscaley);
		pDraw->Vertex3f(0.0, 1.0, -1.0);

		pDraw->TexCoord2f(1.0*tcscalex, (1.0-tcymod)*tcscaley);
		pDraw->Vertex3f(1.0, 1.0, -1.0);
		pDraw->End();

		if(m_pCurrentBackgroundTexture->flags & TX_FL_RECTANGLE)
		{
			glBindTexture(GL_TEXTURE_RECTANGLE, 0);
			if(!pDraw->DisableRectangleTexture())
				return false;
		}
	}

	// Clear any binds
	R_ClearBinds();

	return true;
}

//=============================================
// Class: CMenu
// Function: DrawMenuElements
//=============================================
CMenu::rendercode_t CMenu::DrawMenuElements( CBasicDraw* pDraw )
{
	assert(pDraw->IsActive());

	// Set matrices
	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Scale(1.0/(Float)gWindow.GetWidth(), 1.0/(Float)gWindow.GetHeight(), 1.0);

	rns.view.projection.LoadIdentity();
	rns.view.projection.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, (Float)0.1, 100);

	pDraw->SetModelview(rns.view.modelview.GetMatrix());
	pDraw->SetProjection(rns.view.projection.GetMatrix());

	if(!pDraw->EnableTexture() || !pDraw->DisableRectangleTexture())
		return RC_BASICDRAW_FAIL;

	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	pDraw->Color4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	// Draw the title logo
	R_Bind2DTexture(GL_TEXTURE0_ARB, m_pTitleLogoTexture->palloc->gl_index);

	// Determine origin
	Float titleX = R_GetRelativeX(MENU_TITLE_XPOS, MENU_BASE_WIDTH, gWindow.GetWidth());
	Float titleY = R_GetRelativeY(MENU_TITLE_YPOS, MENU_BASE_HEIGHT, gWindow.GetHeight());

	// Determine size
	Float idealWidth = R_GetRelativeX(m_pTitleLogoTexture->width, MENU_BASE_WIDTH, gWindow.GetWidth());
	Float idealHeight = R_GetRelativeX(m_pTitleLogoTexture->height, MENU_BASE_WIDTH, gWindow.GetWidth());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	R_ValidateShader(pDraw);

	pDraw->Begin(GL_TRIANGLES);
	pDraw->TexCoord2f(0.0, 0.0);
	pDraw->Vertex3f(titleX, titleY, -1.0);

	pDraw->TexCoord2f(1.0, 0.0);
	pDraw->Vertex3f(titleX+idealWidth, titleY, -1.0);

	pDraw->TexCoord2f(0.0, 1.0);
	pDraw->Vertex3f(titleX, titleY+idealHeight, -1.0);

	pDraw->TexCoord2f(1.0, 0.0);
	pDraw->Vertex3f(titleX+idealWidth, titleY, -1.0);

	pDraw->TexCoord2f(0.0, 1.0);
	pDraw->Vertex3f(titleX, titleY+idealHeight, -1.0);

	pDraw->TexCoord2f(1.0, 1.0);
	pDraw->Vertex3f(titleX+idealWidth, titleY+idealHeight, -1.0);
	pDraw->End();

	glDisable(GL_BLEND);

	if(!pDraw->DisableTexture())
		return RC_BASICDRAW_FAIL;

	pDraw->Disable();

	// Draw the title
	if(!gText.Prepare())
		return RC_TEXT_FAIL;

	if(!gText.BindSet(m_pButtonFont))
		return RC_TEXT_FAIL;

	// Now draw the buttons
	bool result = true;
	for(Uint32 i = 0; i < NB_MENU_BTN; i++)
	{
		result = m_buttonsArray[i]->Draw(m_pButtonFont);
		if(!result)
			break;
	}

	// Restore basic draw
	gText.UnBind(m_pButtonFont);
	gText.Reset();

	if(!result)
		return RC_TEXT_FAIL;

	if(!pDraw->Enable())
		return RC_BASICDRAW_FAIL;

	// Clear any binds
	R_ClearBinds();

	return RC_OK;
}

//=============================================
// Class: CMenu
// Function: DrawMenuFade
//=============================================
bool CMenu::DrawMenuFade( void )
{
	if(!m_isActive)
		return true;

	if(!m_postInitBlendBeginTime)
		return true;

	if(m_postInitBlendBeginTime + MENU_POST_INIT_BLEND_TIME < ens.time)
	{
		m_postInitBlendBeginTime = 0;
		return true;
	}

	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	assert(pDraw->IsActive());

	// Set matrices
	rns.view.modelview.LoadIdentity();

	rns.view.projection.LoadIdentity();
	rns.view.projection.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, (Float)0.1, 100);

	pDraw->SetModelview(rns.view.modelview.GetMatrix());
	pDraw->SetProjection(rns.view.projection.GetMatrix());

	if(!pDraw->EnableTexture() || !pDraw->DisableRectangleTexture())
		return false;

	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	R_Bind2DTexture(GL_TEXTURE0_ARB, m_pBgBlurredTexture->palloc->gl_index);

	Float alpha = (ens.time - m_postInitBlendBeginTime)/MENU_POST_INIT_BLEND_TIME;
	alpha = 1.0 - alpha;

	pDraw->Color4f(GL_ONE, GL_ONE, GL_ONE, alpha);

	R_ValidateShader(pDraw);

	Float tcymod = 0.0;
	Float tcscalex = 1.0;
	Float tcscaley = 1.0;

	pDraw->Begin(GL_TRIANGLES);
	pDraw->TexCoord2f(0.0, tcymod*tcscaley);
	pDraw->Vertex3f(0.0, 0.0, -1.0);

	pDraw->TexCoord2f(1.0*tcscalex, tcymod*tcscaley);
	pDraw->Vertex3f(1.0, 0.0, -1.0);

	pDraw->TexCoord2f(0.0, (1.0-tcymod)*tcscaley);
	pDraw->Vertex3f(0.0, 1.0, -1.0);

	pDraw->TexCoord2f(1.0*tcscalex, tcymod*tcscaley);
	pDraw->Vertex3f(1.0, 0.0, -1.0);

	pDraw->TexCoord2f(0.0, (1.0-tcymod)*tcscaley);
	pDraw->Vertex3f(0.0, 1.0, -1.0);

	pDraw->TexCoord2f(1.0*tcscalex, (1.0-tcymod)*tcscaley);
	pDraw->Vertex3f(1.0, 1.0, -1.0);
	pDraw->End();

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	// Clear any binds
	R_ClearBinds();

	return true;
}

//=============================================
// Class: CMenu
// Function: SetBlendTargetTexture
//=============================================
void CMenu::SetBlendTargetTexture( en_texture_t* ptexture )
{
	if(!m_flBlendBeginTime 
		&& (!ptexture && m_pCurrentBackgroundTexture == m_pBackgroundTexture 
		|| ptexture && m_pCurrentBackgroundTexture == ptexture))
	{
		m_pBlendFromTexture = nullptr;
		m_pBlendToTexture = nullptr;
		m_flBlendBeginTime = 0;
		return;
	}

	if(m_flBlendBeginTime && (ens.time - m_flBlendBeginTime <= MENU_BLEND_TIME) && m_flBlendBeginTime != ens.time)
	{
		if(ptexture)
			m_pNextBlendTexture = ptexture;
		else
			m_pNextBlendTexture = m_pBackgroundTexture;

		return;
	}

	m_pBlendFromTexture = m_pCurrentBackgroundTexture;
	if(ptexture)
		m_pBlendToTexture = ptexture;
	else
		m_pBlendToTexture = m_pBackgroundTexture;

	m_flBlendBeginTime = ens.time;
	m_pNextBlendTexture = nullptr;
}

//=============================================
// Class: CMenu
// Function: GetLoadingTexture
//=============================================
en_texture_t* CMenu::GetLoadingTexture( void )
{
	if(!m_pBlendToTexture && m_pCurrentBackgroundTexture == m_pBackgroundTexture)
		return m_pBgBlurredTexture;
	else if(m_pBlendToTexture)
		return m_pBlendToTexture;
	else
		return m_pCurrentBackgroundTexture;
}

//=============================================
// Class: CMenu
// Function: Think
//=============================================
void CMenu::Think( void )
{
	if(!IsActive())
		return;

	// Do not think when loading
	if(ens.gamestate != GAME_LOADING)
	{
		// Reposition the buttons
		SetButtons();

		// Perform think functions for the buttons
		for(Uint32 i = 0; i < m_buttonsArray.size(); i++)
			m_buttonsArray[i]->Think();
	}

	// Clear blending if requested
	if(m_flBlendBeginTime && (m_flBlendBeginTime+MENU_BLEND_TIME) <= ens.time)
	{
		m_pCurrentBackgroundTexture = m_pBlendToTexture;
		m_pBlendFromTexture = nullptr;
		m_pBlendToTexture = nullptr;
		m_flBlendBeginTime = 0;
	}

	// If there is another texture queued up, load it
	if(m_pNextBlendTexture && !m_flBlendBeginTime)
	{
		m_pBlendFromTexture = m_pCurrentBackgroundTexture;
		m_pBlendToTexture = m_pNextBlendTexture;

		m_flBlendBeginTime = ens.time;
		m_pNextBlendTexture = nullptr;
	}
}

//=============================================
// Class: CMenu
// Function: ButtonAction
//=============================================
void CMenu::ButtonAction( mbutton_t buttonId )
{
	switch(buttonId)
	{
	case MENU_BTN_CONSOLE:
		gConsole.ShowConsole();
		break;
	case MENU_BTN_RESUMEGAME:
		if(ens.gamestate == GAME_RUNNING)
			HideMenu();
		break;
	case MENU_BTN_CONTINUE:
		{
			CString savefile;
			if(gSaveRestore.GetMostRecentSave(&savefile))
			{
				CString command;
				command << "load " << savefile;
				gCommands.AddCommand(command.c_str());
			}
		}
		break;
	case MENU_BTN_NEWGAME:
		{
			CUINewGameWindow* pNewGameWindow = CUINewGameWindow::GetInstance();
			if(!pNewGameWindow)
			{
				// Create the console window
				bool isIngame = (ens.gamestate == GAME_RUNNING) ? true : false;
				pNewGameWindow = CUINewGameWindow::CreateInstance(isIngame);
				if(!pNewGameWindow)
					Con_EPrintf("Failed to create exit window.\n");
			}
			else
			{
				// Change focus to this window
				gUIManager.SetFocusWindow(pNewGameWindow);
			}
		}
		break;
	case MENU_BTN_SAVELOADGAME:
	case MENU_BTN_LOADGAME:
		{
			CUISaveLoadWindow* pSaveLoadWindow = CUISaveLoadWindow::GetInstance();
			if(!pSaveLoadWindow)
			{
				// Create the console window
				bool isIngame = (ens.gamestate == GAME_RUNNING) ? true : false;
				pSaveLoadWindow = CUISaveLoadWindow::CreateInstance(isIngame);
				if(!pSaveLoadWindow)
					Con_EPrintf("Failed to create exit window.\n");
			}
			else
			{
				// Change focus to this window
				gUIManager.SetFocusWindow(pSaveLoadWindow);
			}
		}
		break;
	case MENU_BTN_SETTINGS:
		{
			CUISettingsWindow* pSettingsWindow = CUISettingsWindow::GetInstance();
			if(!pSettingsWindow)
			{
				// Create the console window
				pSettingsWindow = CUISettingsWindow::CreateInstance();
				if(!pSettingsWindow)
					Con_EPrintf("Failed to create settings window.\n");
			}
			else
			{
				if(!pSettingsWindow->isVisible())
					pSettingsWindow->setVisible(true);

				// Change focus to this window
				gUIManager.SetFocusWindow(pSettingsWindow);
			}
		}
		break;
	case MENU_BTN_QUIT:
		{
			CUIExitWindow* pExitWindow = CUIExitWindow::GetInstance();
			if(!pExitWindow)
			{
				// Create the console window
				bool isIngame = (ens.gamestate == GAME_RUNNING) ? true : false;
				pExitWindow = CUIExitWindow::CreateInstance(isIngame);
				if(!pExitWindow)
					Con_EPrintf("Failed to create exit window.\n");
			}
			else
			{
				// Change focus to this window
				gUIManager.SetFocusWindow(pExitWindow);
			}
		}
		break;
	}
}

//=============================================
// Class: CMenu
// Function: ShowMenu
//=============================================
void CMenu::ShowMenu( void )
{
	// Reposition the cursor
	gInput.ResetMouse();

	// Set active state
	m_isActive = true;

	// Update contine button
	UpdateContineButton();

	// Show menu windows and hide non-menu ones
	gUIManager.HideWindows(~CUIWindow::UIW_FL_MENUWINDOW);
	gUIManager.ShowWindows(CUIWindow::UIW_FL_MENUWINDOW);

	if(!gInput.IsMouseVisible())
	{
		m_shouldHideMouse = true;
		gInput.ShowMouse();
	}
	else if(CL_IsGameActive())
	{
		// We're in a GUI menu or saytext, so don't hide it
		m_shouldHideMouse = false;
	}

	// Pause the game if we're in the menu
	if(ens.gamestate == GAME_RUNNING && svs.maxclients <= 1 && !svs.pauseovveride )
		Sys_SetPaused(true, false);

	if(ens.spawnconsole)
	{
		ens.spawnconsole = false;
		gConsole.ShowConsole();
	}

	// Refresh saves list
	CUISaveLoadWindow* pSaveLoadWindow = CUISaveLoadWindow::GetInstance();
	if(pSaveLoadWindow)
		pSaveLoadWindow->MarkRecheckSaves();

	// Ensure that this is set
	gConsole.UpdateTextHistory();

	// Begin spawning menu particles
	gMenuParticles.StartParticles();

	// Pause music playback
	ResumeMenuMusic();
}

//=============================================
// Class: CMenu
// Function: ShowMenu
//=============================================
void CMenu::HideMenu( void )
{
	if(ens.gamestate != GAME_RUNNING)
		return;

	// Set active state
	m_isActive = false;

	// Hide menu windows and show non-menu ones
	gUIManager.ShowWindows(~CUIWindow::UIW_FL_MENUWINDOW);
	gUIManager.HideWindows(CUIWindow::UIW_FL_MENUWINDOW);

	// Reset menu bg
	ResetMenuBackground();
	// Release bg textures
	FreeSaveBackgroundTextures();

	// Reposition the cursor
	gInput.ResetMouse();

	if(m_shouldHideMouse && gInput.IsMouseVisible())
		gInput.HideMouse();

	// Un-pause the game if we're exiting the menu
	if(ens.gamestate == GAME_RUNNING && svs.maxclients <= 1 && !svs.pauseovveride)
		Sys_SetPaused(false, false);

	// Kill all menu particles
	gMenuParticles.KillParticles();

	// Resume music playback
	PauseMenuMusic();
}

//=============================================
// Class: CMenu
// Function: FreeSaveBackgroundTextures
//=============================================
void CMenu::FreeSaveBackgroundTextures( void )
{
	if(!m_saveBgTexturesList.empty())
	{
		CTextureManager* pTextureManager = CTextureManager::GetInstance();

		m_saveBgTexturesList.begin();
		while(!m_saveBgTexturesList.end())
		{
			pTextureManager->DeleteTexture(m_saveBgTexturesList.get());
			m_saveBgTexturesList.next();
		}

		m_saveBgTexturesList.clear();
	}

	// Reset any blending
	m_pBlendFromTexture = nullptr;
	m_pBlendToTexture = nullptr;
	m_pNextBlendTexture = nullptr;
	m_pLatestSaveFileBgTexture = nullptr;

	m_pCurrentBackgroundTexture = m_pBackgroundTexture;

	m_flBlendBeginTime = 0;
}

//=============================================
// Class: CMenu
// Function: FreeSaveBackgroundTextures
//=============================================
void CMenu::AddSaveBackgroundTexture( en_texture_t* ptexture )
{
	if(!m_saveBgTexturesList.empty())
	{
		m_saveBgTexturesList.begin();
		while(!m_saveBgTexturesList.end())
		{
			if(m_saveBgTexturesList.get() == ptexture)
				return;

			m_saveBgTexturesList.next();
		}
	}

	m_saveBgTexturesList.add(ptexture);
}

//=============================================
// Class: CMenu
// Function: InitialStartup
//=============================================
void CMenu::SetCurrentBgTexture( en_texture_t* ptexture ) 
{ 
	m_pCurrentBackgroundTexture = ptexture; 
}

//=============================================
// Class: CMenu
// Function: InitialStartup
//=============================================
en_texture_t* CMenu::GetCurrentBgTexture( void ) 
{ 
	return m_pCurrentBackgroundTexture; 
}

//=============================================
// Class: CMenu
// Function: InitialStartup
//=============================================
void CMenu::OnBgTextureDeleted( en_texture_t* ptexture )
{
	if(!m_saveBgTexturesList.empty())
	{
		m_saveBgTexturesList.begin();
		while(!m_saveBgTexturesList.end())
		{
			if(m_saveBgTexturesList.get() == ptexture)
			{
				m_saveBgTexturesList.remove(ptexture);
				continue;
			}

			m_saveBgTexturesList.next();
		}
	}

	if(m_pCurrentBackgroundTexture == ptexture)
		m_pCurrentBackgroundTexture = m_pBackgroundTexture;

	if(m_pBlendFromTexture == ptexture
		|| m_pBlendToTexture == ptexture
		|| m_pNextBlendTexture == ptexture)
	{
		m_flBlendBeginTime = 0;
		m_pBlendFromTexture = nullptr;
		m_pBlendToTexture = nullptr;
		m_pNextBlendTexture = nullptr;
	}
}

//=============================================
// Class: CMenu
// Function: InitialStartup
//=============================================
void CMenu::InitialStartup( void )
{
	// Blend the screen
	m_postInitBlendBeginTime = ens.time;

	// Play music for the menu
	gSoundEngine.PlayOgg(MENU_MUSIC_FILE_STARTUP, MUSIC_CHANNEL_MENU, 0, (OGG_FL_LOOP|OGG_FL_FADE_IN|OGG_FL_MENU), 1);
	m_isMenuMusicPlaying = true;

	m_playingMusicType = MENU_MUSIC_STARTUP;
}

//=============================================
// Class: CMenu
// Function: KillMenuMusic
//=============================================
void CMenu::PauseMenuMusic( void )
{
	if(!m_isMenuMusicPlaying)
		return;

	gSoundEngine.PauseOggChannel(MUSIC_CHANNEL_MENU);
	m_isMenuMusicPlaying = false;
}

//=============================================
// Class: CMenu
// Function: KillMenuMusic
//=============================================
void CMenu::ResumeMenuMusic( void )
{
	// Determine what type of music we want to play
	menu_music_type_t requestedMusicType = MENU_MUSIC_UNDEFINED;
	if(ens.gamestate == GAME_RUNNING)
		requestedMusicType = MENU_MUSIC_INGAME;
	else
		requestedMusicType = MENU_MUSIC_STARTUP;

	if(requestedMusicType != m_playingMusicType)
	{
		// Switch to desired music
		CString menuMusicFile;
		if(requestedMusicType == MENU_MUSIC_INGAME)
			menuMusicFile = MENU_MUSIC_FILE_INGAME;
		else
			menuMusicFile = MENU_MUSIC_FILE_STARTUP;

		gSoundEngine.PlayOgg(menuMusicFile.c_str(), MUSIC_CHANNEL_MENU, 0, (OGG_FL_LOOP|OGG_FL_FADE_IN|OGG_FL_MENU), 1);

		// Set current type playing
		m_playingMusicType = requestedMusicType;
		m_isMenuMusicPlaying = true;
	}
	else
	{
		if(m_isMenuMusicPlaying)
			return;

		gSoundEngine.UnPauseOggChannel(MUSIC_CHANNEL_MENU, 0.5);
		m_isMenuMusicPlaying = true;
	}
}

//=============================================
// Class: CMenu
// Function: CursorOverButton
//=============================================
void CMenu::CursorOverButton( mbutton_t buttonId )
{
	switch(buttonId)
	{
	case MENU_BTN_CONTINUE:
		{
			if(m_pLatestSaveFileBgTexture)
				SetBlendTargetTexture(m_pLatestSaveFileBgTexture);
		}
		break;
	case MENU_BTN_CONSOLE:
	case MENU_BTN_RESUMEGAME:
	case MENU_BTN_NEWGAME:
	case MENU_BTN_SAVELOADGAME:
	case MENU_BTN_LOADGAME:
	case MENU_BTN_SETTINGS:
	case MENU_BTN_QUIT:
		break;
	}
}

//=============================================
// Class: CMenu
// Function: CursorLeaveButton
//=============================================
void CMenu::CursorLeaveButton( mbutton_t buttonId )
{
	switch(buttonId)
	{
	case MENU_BTN_CONTINUE:
		{
			if(m_pLatestSaveFileBgTexture)
				SetBlendTargetTexture(nullptr);
		}
		break;
	case MENU_BTN_CONSOLE:
	case MENU_BTN_RESUMEGAME:
	case MENU_BTN_NEWGAME:
	case MENU_BTN_SAVELOADGAME:
	case MENU_BTN_LOADGAME:
	case MENU_BTN_SETTINGS:
	case MENU_BTN_QUIT:
		break;
	}
}

//=============================================
// Class: CMenu
// Function: ResetMenuBackground
//=============================================
void CMenu::ResetMenuBackground( void )
{
	m_pBlendFromTexture = nullptr;
	m_pBlendToTexture = nullptr;
	m_flBlendBeginTime = 0;

	m_pCurrentBackgroundTexture = m_pBackgroundTexture;
}

//=============================================
// Class: CMenu
// Function: UpdateContineButton
//=============================================
void CMenu::UpdateContineButton( void )
{
	// Hide/show "Continue Game" button depending on gamestate
	// and availability of save files
	if(ens.gamestate == GAME_RUNNING || !gSaveRestore.GetMostRecentSave(&m_latestSaveFileName))
	{
		ClearLatestSaveFile();
		return;
	}

	// Retrieve the screenshot from the file
	const byte* pfile = gSaveRestore.LoadSaveFile(m_latestSaveFileName.c_str());
	if(!pfile)
		return;

	const save_header_t* pheader = reinterpret_cast<const save_header_t*>(pfile);
	if(!pheader->screenshotdatasize 
		|| !pheader->screenshotheight
		|| !pheader->screenshotwidth
		|| !pheader->screenshotoffset)
	{
		gSaveRestore.FreeSaveFile(pfile);
		ClearLatestSaveFile();
		return;
	}

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Bind the texture to GL
	const byte* pimagedata = pfile + pheader->screenshotoffset;
	m_pLatestSaveFileBgTexture = pTextureManager->LoadFromMemory(m_latestSaveFileName.c_str(), RS_WINDOW_LEVEL, (TX_FL_DXT1|TX_FL_NOMIPMAPS|TX_FL_CLAMP_S|TX_FL_CLAMP_T), pimagedata, pheader->screenshotwidth, pheader->screenshotheight, pheader->screenshotbpp, pheader->screenshotdatasize);
	if(!m_pLatestSaveFileBgTexture)
	{
		gSaveRestore.FreeSaveFile(pfile);
		return;
	}

	gSaveRestore.FreeSaveFile(pfile);

	m_buttonsArray[MENU_BTN_CONTINUE]->SetEnabled(true);
	m_buttonsArray[MENU_BTN_CONTINUE]->SetHidden(false);
}

//=============================================
// Class: CMenu
// Function: ClearLatestSaveFile
//=============================================
void CMenu::ClearLatestSaveFile( void )
{
	// Hide the button
	m_buttonsArray[MENU_BTN_CONTINUE]->SetEnabled(false);
	m_buttonsArray[MENU_BTN_CONTINUE]->SetHidden(true);

	if(!m_latestSaveFileName.empty())
		m_latestSaveFileName.clear();

	if(m_pLatestSaveFileBgTexture)
	{
		CTextureManager* pTextureManager = CTextureManager::GetInstance();
		pTextureManager->DeleteTexture(m_pLatestSaveFileBgTexture);
		m_pLatestSaveFileBgTexture = nullptr;
	}
}

//=============================================
// Class: CMenuButton
// Function: Constructor
//=============================================
CMenuButton::CMenuButton( CMenu* pMenu, CMenu::mbutton_t buttonId, const Char* pstrText, SDL_Scancode scancode ):
	m_text(pstrText),
	m_brightness(CMenuButton::DEFAULT_BRIGHTNESS),
	m_alpha(CMenuButton::DEFAULT_ALPHA),
	m_originX(0),
	m_originY(0),
	m_sizeX(0),
	m_sizeY(0),
	m_isClicked(false),
	m_isEnabled(true),
	m_isHidden(false),
	m_glowSoundPlayed(false),
	m_isCursorOver(false),
	m_scancode(scancode),
	m_pMenu(pMenu),
	m_buttonId(buttonId)
{
}

//=============================================
// Class: CMenuButton
// Function: Destructor
//=============================================
CMenuButton::~CMenuButton( void )
{
}

//=============================================
// Class: CMenuButton
// Function: Draw
//=============================================
bool CMenuButton::Draw( const font_set_t* pFont )
{
	if(m_isHidden)
		return true;

	// Draw, offsetting by height
	Int32 originX = m_originX;
	Int32 originY = m_originY + m_sizeY;

	if(m_isClicked)
	{
		// Set alpha at 25% for the outline
		gText.SetColor((Uint32)(255*m_brightness),
			(Uint32)(255*m_brightness),
			(Uint32)(255*m_brightness),
			(Uint32)(255*m_alpha*0.25));

		// Draw a pale outline
		if(!gText.DrawSimpleString(pFont, m_text.c_str(), originX, originY))
			return false;
	}

	// Set brightness and alpha
	gText.SetColor((Uint32)(255*m_brightness),
		(Uint32)(255*m_brightness),
		(Uint32)(255*m_brightness),
		(Uint32)(255*m_alpha));

	if(m_isClicked)
	{
		originX -= CLICK_OFFSET;
		originY += CLICK_OFFSET;
	}

	if(!gText.DrawSimpleString(pFont, m_text.c_str(), originX, originY))
		return false;
	else
		return true;
}

//=============================================
// Class: CMenuButton
// Function: Think
//=============================================
void CMenuButton::Think( void )
{
	if(m_isHidden)
		return;

	Int32 cursorX, cursorY;
	gInput.GetMousePosition(cursorX, cursorY);

	// Modulate alpha
	if(IsCursorOver(cursorX, cursorY))
	{
		if(m_isEnabled)
		{
			if(m_brightness < HOVER_BRIGHTNESS)
				m_brightness += 1.0 * (1.0/FADE_TIME)*ens.frametime;
			else if(m_brightness > HOVER_BRIGHTNESS)
				m_brightness = HOVER_BRIGHTNESS;

			if(!m_glowSoundPlayed)
			{
				m_pMenu->PlayMenuSound(CMenu::MENU_HOVER_SOUND);
				m_glowSoundPlayed = true;
			}
		}

		if(m_alpha < HOVER_ALPHA)
			m_alpha += 1.0 * (1.0/FADE_TIME)*ens.frametime;
		else if(m_alpha > HOVER_ALPHA)
			m_alpha = HOVER_ALPHA;

		if(!m_isCursorOver)
		{
			m_pMenu->CursorOverButton(m_buttonId);
			m_isCursorOver = true;
		}
	}
	else
	{
		if(m_isEnabled)
		{
			if(m_brightness > DEFAULT_BRIGHTNESS)
				m_brightness -= 1.0 * (1.0/FADE_TIME)*ens.frametime;
			else if(m_brightness < DEFAULT_BRIGHTNESS)
				m_brightness = DEFAULT_BRIGHTNESS;
		}

		if(m_alpha > DEFAULT_ALPHA)
			m_alpha -= 1.0 * (1.0/FADE_TIME)*ens.frametime;
		else if(m_alpha < DEFAULT_ALPHA)
			m_alpha = DEFAULT_ALPHA;

		if(m_glowSoundPlayed)
			m_glowSoundPlayed = false;

		if(m_isCursorOver)
		{
			m_pMenu->CursorLeaveButton(m_buttonId);
			m_isCursorOver = false;
		}
	}

	if(!m_isEnabled)
		m_brightness = DISABLED_BRIGHTNESS;

	// Clear this if the button was released
	if(m_isClicked)
	{
		gInput.GetMousePosition(cursorX, cursorY);

		// See if the cursor wandered away
		if(!IsCursorOver(cursorX, cursorY))
			m_isClicked = false;
	}
}

//=============================================
// Class: CMenuButton
// Function: IsCursorOver
//=============================================
bool CMenuButton::IsCursorOver( Int32 cursorX, Int32 cursorY ) const
{
	if(gUIManager.IsMouseOverAnyWindow())
		return false;

	if(m_originX > cursorX)
		return false;
	if(m_originX+(Int32)m_sizeX < cursorX)
		return false;
	if(m_originY > cursorY)
		return false;
	if(m_originY+(Int32)m_sizeY < cursorY)
		return false;

	return true;
}

//=============================================
// Class: CMenuButton
// Function: SetProperties
//=============================================
void CMenuButton::SetProperties( Int32 xPos, Int32 yPos, Uint32 width, Uint32 height )
{
	m_originX = xPos;
	m_originY = yPos;

	m_sizeX = width;
	m_sizeY = height;
}

//=============================================
// Class: CMenuButton
// Function: MouseButtonEvent
//=============================================
bool CMenuButton::MouseButtonEvent( Int32 button, bool keyDown )
{
	if(!m_isEnabled || m_isHidden)
		return false;

	// Only support this for now
	if(button != SDL_BUTTON_LEFT)
		return false;

	Int32 cursorX, cursorY;
	gInput.GetMousePosition(cursorX, cursorY);

	if(IsCursorOver(cursorX, cursorY))
	{
		if(keyDown)
		{
			m_alpha = HOVER_ALPHA;
			m_brightness = HOVER_BRIGHTNESS;
		}

		if(keyDown && !m_isClicked)
		{
			m_pMenu->PlayMenuSound(CMenu::MENU_CLICK_SOUND);
			m_isClicked = true;
		}
		else if(m_isClicked)
		{
			m_pMenu->ButtonAction(m_buttonId);
			m_isClicked = false;
		}

		return true;
	}

	return false;
}

//=============================================
// Class: CMenuButton
// Function: KeyEvent
//=============================================
bool CMenuButton::KeyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(!m_isEnabled || m_isHidden)
		return false;

	if(!keyDown || button != m_scancode)
		return false;
	
	// Play click sound
	m_pMenu->PlayMenuSound(CMenu::MENU_CLICK_SOUND);

	m_alpha = HOVER_ALPHA;
	m_brightness = HOVER_BRIGHTNESS;
	m_isClicked = true;

	m_pMenu->ButtonAction(m_buttonId);
	return true;
}