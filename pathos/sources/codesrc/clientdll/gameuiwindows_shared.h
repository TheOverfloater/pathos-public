/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMEUIWINDOWS_SHARED_H
#define GAMEUIWINDOWS_SHARED_H

// Base width for game ui windows
static const Uint32 BASE_GAMEUIWINDOW_WIDTH  = 1920;
// Base height for game ui windows
static const Uint32 BASE_GAMEUIWINDOW_HEIGHT  = 1080;

// Thickness of the bars making up the frame
static const Uint32 GAMEUIWINDOW_BAR_THICKNESS = 16;
// Y origin of horizontal bars relative to positioning
static const Uint32 GAMEUIWINDOW_H_BAR_Y_ORIGIN = 50;
// X origin of vertical bars relative to positioning
static const Uint32 GAMEUIWINDOW_V_BAR_X_ORIGIN = 200;
// Color of bars for this window
static const color32_t GAMEUIWINDOW_BARS_COLOR = color32_t(220, 220, 220, 255);
// Edge thickness for borders
static const Uint32 GAMEUIWINDOW_EDGE_THICKNESS = 2;
// Color of bars for this window
static const color32_t GAMEUIWINDOW_MAIN_TAB_COLOR = color32_t(100, 100, 250, 255);
// Color of background for this window
static const color32_t GAMEUIWINDOW_MAIN_TAB_BG_COLOR = color32_t(0, 0, 0, 150);
// Default text color
static const color32_t GAMEUIWINDOW_DEFAULT_TEXT_COLOR = color32_t(255, 255, 255, 255);
// Color of highlighted buttons for this window
static const color32_t GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR = color32_t(220, 220, 220, 100);
// Main tab max width
static const Uint32 GAMEUIWINDOW_MAIN_TAB_MAX_WIDTH = 800;
// Main tab max height
static const Uint32 GAMEUIWINDOW_MAIN_TAB_MAX_HEIGHT = 800;
// Default button width
static const Uint32 DEFAULT_BUTTON_WIDTH = 200;
// Default button height
static const Uint32 DEFAULT_BUTTON_HEIGHT = 60;
// Tab top inset
static const Uint32 GAMEUIWINDOW_TAB_TOP_INSET = 50;
// Tab side inset
static const Uint32 GAMEUIWINDOW_TAB_SIDE_INSET = 25;
// Tab bottom inset
static const Uint32 GAMEUIWINDOW_TAB_BOTTOM_INSET = 100;
// Game UI ok sound
static const Char GAMEUI_OK_SOUND[] = "gameui/gameui_ok.wav";
// Game UI fail sound
static const Char GAMEUI_FAIL_SOUND[] = "gameui/gameui_fail.wav";
// Game UI blip sound
static const Char GAMEUI_BLIP_SOUND[] = "gameui/gameui_blip.wav";
// Delay until a UI window is deleted
static const Double GAMEUIWINDOW_REMOVE_DELAY_TIME = 1;

#endif //GAMEUIWINDOWS_SHARED_H