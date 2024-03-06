//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           GlWindow.h
// last modified:  Apr 28 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.2
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//
#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <mx/mx.h>
#include <mx/mxglwindow.h>

#include "includes.h"

struct en_texture_t;

class CBasicDraw;
class CBasicVBMRenderer;

/*
=================================
CGLWindow

=================================
*/
class CGLWindow : public mxGlWindow
{
public:
	// Number of skybox textures
	static const Uint32 NUM_SKYBOX_TEXTURES = 6;

public:
	// Flex visualizer height
	static const Float FLEX_VISUALIZER_HEIGHT_FRACTION;
	// Flex visualizer inset
	static const Float FLEX_VISUALIZER_INSET_FRACTION;
	// Flex visualize scale height
	static const Float FLEX_VISUALIZER_SCALE_HEIGHT;
	// Flex visualizer time window for double click
	static const Float FLEX_MOUSECLICK_TIME_WINDOW;
	// Reference texture width
	static const Uint32 REFERENCE_TEXTURE_WIDTH;
	// Reference texture height
	static const Uint32 REFERENCE_TEXTURE_HEIGHT;
	// Ground alpha value
	static const Float GROUND_ALPHA_VALUE;
	// Skybox size
	static const Float SKYBOX_SIZE;
	// Number of curve segments
	static const Uint32 CURVE_SEGMENTS;
	// Skybox texture postfixes
	static const Char* SKY_POSTFIXES[NUM_SKYBOX_TEXTURES];

private:
	CGLWindow( mxWindow *parent, Int32 x, Int32 y, Int32 w, Int32 h, const Char *label, Int32 style );
	~CGLWindow( void );

public:
	// Initializes the class
	void Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Updates frame-based states
	void Update( void );
	// Draws the skybox
	bool DrawSkybox( CBasicDraw* pBasicDraw, bool mirror = false );
	// Draws the mirror
	bool DrawMirrored( CBasicDraw* pBasicDraw );
	// Draws the flex visualizer
	bool DrawFlexVisualizer( CBasicDraw* pBasicDraw );
	// Draws a curved line
	void DrawCurvedLine( CBasicDraw* pBasicDraw, Float c1x, Float c1y, Float c2x, Float c2y );
	// Draws the texture tab contents
	bool DrawTexture( CBasicDraw* pBasicDraw );
	// Draws UV coords
	bool DrawUVCoords( CBasicDraw* pBasicDraw, Int32 xOrigin, Int32 yOrigin, Int32 height, Int32 width );
	// Draws the floor
	void DrawFloor( CBasicDraw* pBasicDraw );
	// Tells if the cursor is inside the flex visualizer bounds
	bool IsInVisualizerBounds( Float x, Float y );
	// Draws the model itself
	bool DrawModel( void );
	// Draws the ground
	bool DrawGround( CBasicDraw* pBasicDraw );

	// Loads the ground texture
	bool LoadGroundTexture( const Char* pstrFilename );
	// Loads skybox textures
	bool LoadSkyboxTextures( const Char* pstrFilename );
	// Dumps viewport
	void DumpViewport( const Char *filename );

	// Returns the width of the window
	Uint32 GetWidth( void );
	// Returns the height of the window
	Uint32 GetHeight( void );

public:
	// Draws window contents
	virtual void draw( void ) override;
	// Handles an mx event
	virtual Int32 handleEvent( mxEvent *pEvent ) override;

public:
	// Creates an instance of this class
	static CGLWindow* CreateInstance( mxWindow* pParent );
	// Returns the current instance of this class
	static CGLWindow* GetInstance( void );
	// Deletes the current instance of this class
	static void DeleteInstance( void );

private:
	Float m_oldRotateX;
	Float m_oldRotateY;
	Float m_oldTranslateZ;
	Float m_oldTranslateX;
	Float m_oldTranslateY;
	Int32 m_oldX;
	Int32 m_oldY;

	bool m_clickInVisualizer;
	Double m_lastActionClickTime;
	Int32 m_lastButtonClicked;

	en_texture_t* m_pGroundTexture;
	en_texture_t* m_pSkyboxTextures[NUM_SKYBOX_TEXTURES];

private:
	// Current instance of this class
	static CGLWindow* g_pInstance;
};
#endif // GLWINDOW_H
