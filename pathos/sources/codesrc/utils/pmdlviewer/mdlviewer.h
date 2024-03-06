//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           mdlviewer.h
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
#ifndef MDLVIEWER_H
#define MDLVIEWER_H

#include <mx/mx.h>

#include "includes.h"
#include "vector.h"
#include "common.h"

#include "vbmformat.h"
#include "studio.h"

#include "com_math.h"
#include "textures_shared.h"
#include "texturemanager.h"

#include "r_glextf.h"
#include "flex_shared.h"
#include "flexmanager.h"
#include "FlexVisualizer.h"

class mxMenuBar;
class CGLWindow;
class CControlPanel;
class CGLWindow;
class CFolderViewer;

/*
=================================
CMDLViewer

=================================
*/
class CMDLViewer : public mxWindow
{
public:
	// Application title
	static const Char VIEWER_APP_TITLE[];
	// Model load path last used
	static const Char VIEWER_MDL_LOAD_PATH[];
	// Model decompile path last used
	static const Char VIEWER_MDL_DECOMPILE_PATH[];
	// Skybox texture path last used
	static const Char VIEWER_ENV_TEX_PATH[];
	// Ground texture path last used
	static const Char VIEWER_GROUND_TEX_PATH[];
	// Skybox texture file path last used
	static const Char VIEWER_ENV_TEX_FILE_PATH[];
	// Ground texture file path last used
	static const Char VIEWER_GROUND_TEX_FILE_PATH[];
	// Mod folder path last used
	static const Char VIEWER_MOD_FOLDER_PATH[];
	// Background color
	static const Char VIEWER_COLOR_BACKGROUND[];
	// Ground color
	static const Char VIEWER_COLOR_GROUND[];
	// Light color
	static const Char VIEWER_COLOR_LIGHT[];
	// Window default x origin
	static const Uint32 WINDOW_DEFAULT_X_ORIGIN;
	// Window default y origin
	static const Uint32 WINDOW_DEFAULT_Y_ORIGIN;
	// Window default width
	static const Uint32 WINDOW_DEFAULT_WIDTH;
	// Window default height
	static const Uint32 WINDOW_DEFAULT_HEIGHT;
	// Recent MDL file option prefix
	static const Char RECENT_MDL_FILE_PREFIX[];
	// Recent folder option prefix
	static const Char RECENT_FOLDER_PREFIX[];

public:
	// Max recent files
	static const Uint32 MAX_RECENT_FILES = 4;

	enum
	{
		IDC_FILE_LOADMODEL = 1001,
		IDC_FILE_LOADBACKGROUNDTEX,
		IDC_FILE_LOADGROUNDTEX,
		IDC_FILE_OPENFOLDER,
		IDC_FILE_CLOSEFOLDER,
		IDC_FILE_UNLOADGROUNDTEX,
		IDC_FILE_RECENTMODELS1,
		IDC_FILE_RECENTMODELS2,
		IDC_FILE_RECENTMODELS3,
		IDC_FILE_RECENTMODELS4,
		IDC_FILE_RECENTFOLDERS1,
		IDC_FILE_RECENTFOLDERS2,
		IDC_FILE_RECENTFOLDERS3,
		IDC_FILE_RECENTFOLDERS4,
		IDC_FILE_EXIT,
		IDC_FILE_COMPILEMODEL,
		IDC_FILE_DECOMPILEMODEL,
		IDC_FILE_CLOSEFOLDER2,

		IDC_OPTIONS_COLORBACKGROUND = 1101,
		IDC_OPTIONS_COLORGROUND,
		IDC_OPTIONS_COLORLIGHT,
		IDC_OPTIONS_CENTERVIEW,
		IDC_OPTIONS_MAKESCREENSHOT,
		IDC_OPTIONS_DUMP,
		IDC_OPTIONS_OPTIONSWINDOW,
	
		IDC_VIEW_FILEASSOCIATIONS = 1201,
	
		IDC_HELP_ABOUT = 1301
	};

private:
	CMDLViewer( void );
	~CMDLViewer( void );

public:
	// Initializes the GL window
	void InitGLWindow( CGLWindow* pWindow );
	// Initializes the viewer
	void InitViewer( void );

	// Load recent files list
	void LoadRecentFiles( void );
	// Save recent files list
	void SaveRecentFiles( void );
	// Init recent files list
	void InitRecentFiles( void );
	// Initializes recent textures
	void InitPreviousTextures( void );

	// Load recent folders list
	void LoadRecentFolders( void );
	// Save recent files list
	void SaveRecentFolders( void );
	// Init recent files list
	void InitRecentFolders( void );

public:
	// Handles an mx event
	virtual Int32 handleEvent( mxEvent *pEvent ) override;
	// Redraws the object
	virtual void redraw( void ) override;

public:
	// Returns the menu bar
	mxMenuBar *GetMenuBar( void ) const { return m_pMenuBar; }

public:
	// Creates an instance of this class
	static CMDLViewer* CreateInstance( void );
	// Returns the current instance of this class
	static CMDLViewer* GetInstance( void );
	// Deletes the current instance of this class
	static void DeleteInstance( void );

private:
	// Menu bar object
	mxMenuBar* m_pMenuBar;

	// Recent MDL files array
	CString m_recentMDLFilesArray[MAX_RECENT_FILES];
	// Recent folders array
	CString m_recentFoldersArray[MAX_RECENT_FILES];

private:
	// Current instance of this class
	static CMDLViewer* g_pInstance;
};
#endif // MDLVIEWER_H