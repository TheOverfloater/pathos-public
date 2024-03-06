//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           mdlviewer.cpp
// last modified:  Jun 03 1999, Mete Ciragan
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mx/mx.h>
#include <mx/gl.h>
#include <mx/mxTga.h>

#include "mdlviewer.h"
#include "glwindow.h"
#include "controlpanel.h"
#include "r_vbmbasic.h"
#include "fileassociation.h"
#include "optionswindow.h"
#include "config.h"
#include "compilerwindow.h"
#include "viewerstate.h"
#include "folderviewer.h"

// Current instance of this class
CMDLViewer* CMDLViewer::g_pInstance = nullptr;

// Application title
const Char CMDLViewer::VIEWER_APP_TITLE[] = "Pathos Model Viewer v1.0";
// Model load path last used
const Char CMDLViewer::VIEWER_MDL_LOAD_PATH[] = "MDL_LOAD_PATH";
// Model decompile path last used
const Char CMDLViewer::VIEWER_MDL_DECOMPILE_PATH[] = "MDL_DECOMPILE_PATH";
// Skybox texture path last used
const Char CMDLViewer::VIEWER_ENV_TEX_PATH[] = "ENV_TEXTURE_PATH";
// Ground texture path last used
const Char CMDLViewer::VIEWER_GROUND_TEX_PATH[] = "GROUND_TEXTURE_PATH";
// Skybox texture file path last used
const Char CMDLViewer::VIEWER_ENV_TEX_FILE_PATH[] = "ENV_TEXTURE_FILE_PATH";
// Ground texture file path last used
const Char CMDLViewer::VIEWER_GROUND_TEX_FILE_PATH[] = "GROUND_TEXTURE_FILE_PATH";
// Mod folder path last used
const Char CMDLViewer::VIEWER_MOD_FOLDER_PATH[] = "MOD_FOLDER_PATH";
// Background color
const Char CMDLViewer::VIEWER_COLOR_BACKGROUND[] = "COLOR_SETTING_BG";
// Ground color
const Char CMDLViewer::VIEWER_COLOR_GROUND[] = "COLOR_SETTING_GROUND";
// Light color
const Char CMDLViewer::VIEWER_COLOR_LIGHT[] = "COLOR_SETTING_LIGHTCOLOR";
// Window default x origin
const Uint32 CMDLViewer::WINDOW_DEFAULT_X_ORIGIN = 20;
// Window default y origin
const Uint32 CMDLViewer::WINDOW_DEFAULT_Y_ORIGIN = 20;
// Window default width
const Uint32 CMDLViewer::WINDOW_DEFAULT_WIDTH = 1024;
// Window default height
const Uint32 CMDLViewer::WINDOW_DEFAULT_HEIGHT = 768;
// Recent MDL file option prefix
const Char CMDLViewer::RECENT_MDL_FILE_PREFIX[] = "RECENT_MDL_FILE";
// Recent folder option prefix
const Char CMDLViewer::RECENT_FOLDER_PREFIX[] = "RECENT_FOLDER";

//=============================================
// @brief Constructor
//
//=============================================
CMDLViewer::CMDLViewer( void ): 
	mxWindow(0, 0, 0, 0, 0, VIEWER_APP_TITLE, mxWindow::Normal),
	m_pMenuBar(nullptr)
{
	// create menu stuff
	m_pMenuBar = new mxMenuBar(this);
	mxMenu *menuFile = new mxMenu();
	mxMenu *menuOptions = new mxMenu();
	mxMenu *menuView = new mxMenu();
	mxMenu *menuHelp = new mxMenu();

	m_pMenuBar->addMenu("File", menuFile);
	m_pMenuBar->addMenu("Options", menuOptions);
	m_pMenuBar->addMenu("View", menuView);
	m_pMenuBar->addMenu("Help", menuHelp);

	mxMenu *menuRecentModels = new mxMenu();
	menuRecentModels->add("(empty)", IDC_FILE_RECENTMODELS1);
	menuRecentModels->add("(empty)", IDC_FILE_RECENTMODELS2);
	menuRecentModels->add("(empty)", IDC_FILE_RECENTMODELS3);
	menuRecentModels->add("(empty)", IDC_FILE_RECENTMODELS4);

	mxMenu *menuRecentFolders = new mxMenu();
	menuRecentFolders->add("(empty)", IDC_FILE_RECENTFOLDERS1);
	menuRecentFolders->add("(empty)", IDC_FILE_RECENTFOLDERS2);
	menuRecentFolders->add("(empty)", IDC_FILE_RECENTFOLDERS3);
	menuRecentFolders->add("(empty)", IDC_FILE_RECENTFOLDERS4);

	menuFile->add("Load Model...", IDC_FILE_LOADMODEL);
	menuFile->addSeparator ();
	menuFile->add("Compile QC...", IDC_FILE_COMPILEMODEL);
	menuFile->add("Decompile Model...", IDC_FILE_DECOMPILEMODEL);
	menuFile->addSeparator();
	menuFile->add("Load Skybox Texture...", IDC_FILE_LOADBACKGROUNDTEX);
	menuFile->add("Load Ground Texture...", IDC_FILE_LOADGROUNDTEX);
	menuFile->addSeparator ();
	menuFile->add("Unload Ground Texture", IDC_FILE_UNLOADGROUNDTEX);
	menuFile->addSeparator ();
	menuFile->add ("Open folder...", IDC_FILE_OPENFOLDER);
	menuFile->add ("Close folder", IDC_FILE_CLOSEFOLDER);
	menuFile->addSeparator();
	menuFile->addMenu("Recent Models", menuRecentModels);
	menuFile->addMenu("Recent Folders", menuRecentFolders);
	menuFile->addSeparator();
	menuFile->add("Exit", IDC_FILE_EXIT);

	menuOptions->add("Background Color...", IDC_OPTIONS_COLORBACKGROUND);
	menuOptions->add("Ground Color...", IDC_OPTIONS_COLORGROUND);
	menuOptions->add("Light Color...", IDC_OPTIONS_COLORLIGHT);
	menuOptions->addSeparator();
	menuOptions->add("Center View", IDC_OPTIONS_CENTERVIEW);

	menuOptions->addSeparator();
	menuOptions->add ("Make Screenshot...", IDC_OPTIONS_MAKESCREENSHOT);

	menuOptions->add("Settings", IDC_OPTIONS_OPTIONSWINDOW);
	menuView->add("File Associations...", IDC_VIEW_FILEASSOCIATIONS);

	menuHelp->add("About...", IDC_HELP_ABOUT);
}

//=============================================
// @brief Destructor
//
//=============================================
CMDLViewer::~CMDLViewer( void )
{
	// mx deletes these windows before we 
	// get a chance to do it via DeleteInstance()
	g_pInstance = nullptr;

	SaveRecentFiles();
	SaveRecentFolders();
}

//=============================================
// @brief Initializes the GL window
//
//=============================================
void CMDLViewer::InitGLWindow( CGLWindow* pWindow )
{
	SetWindowLong((HWND)pWindow->getHandle(), GWL_EXSTYLE, WS_EX_CLIENTEDGE);
}

//=============================================
// @brief Initializes the viewer
//
//=============================================
void CMDLViewer::InitViewer( void )
{
	LoadRecentFiles();
	InitRecentFiles();
	LoadRecentFolders();
	InitRecentFolders();	
	InitPreviousTextures();

	setBounds(WINDOW_DEFAULT_X_ORIGIN, WINDOW_DEFAULT_Y_ORIGIN, 
		WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);

	setVisible(true);
}

//=============================================
// @brief Initializes recent files list
//
//=============================================
void CMDLViewer::InitRecentFiles( void )
{
	for (Uint32 i = 0; i < MAX_RECENT_FILES; i++)
	{
		if (!m_recentMDLFilesArray[i].empty())
		{
			m_pMenuBar->modify(IDC_FILE_RECENTMODELS1 + i, IDC_FILE_RECENTMODELS1 + i, m_recentMDLFilesArray[i].c_str());
		}
		else
		{
			m_pMenuBar->modify(IDC_FILE_RECENTMODELS1 + i, IDC_FILE_RECENTMODELS1 + i, "(empty)");
			m_pMenuBar->setEnabled(IDC_FILE_RECENTMODELS1 + i, false);
		}
	}
}

//=============================================
// @brief Loads recent files list
//
//=============================================
void CMDLViewer::LoadRecentFiles( void )
{
	for(Uint32 i = 0; i < MAX_RECENT_FILES; i++)
	{
		CString optionName;
		optionName << RECENT_MDL_FILE_PREFIX << i;

		const Char* pstrValue = gConfig.GetOptionValue(optionName.c_str());
		if(!pstrValue && !m_recentMDLFilesArray[i].empty())
			m_recentMDLFilesArray[i].clear();
		else
			m_recentMDLFilesArray[i] = pstrValue;
	}
}

//=============================================
// @brief Initializes recent folders list
//
//=============================================
void CMDLViewer::InitRecentFolders( void )
{
	for (Uint32 i = 0; i < MAX_RECENT_FILES; i++)
	{
		if (!m_recentFoldersArray[i].empty())
		{
			m_pMenuBar->modify(IDC_FILE_RECENTFOLDERS1 + i, IDC_FILE_RECENTFOLDERS1 + i, m_recentFoldersArray[i].c_str());
		}
		else
		{
			m_pMenuBar->modify(IDC_FILE_RECENTFOLDERS1 + i, IDC_FILE_RECENTFOLDERS1 + i, "(empty)");
			m_pMenuBar->setEnabled(IDC_FILE_RECENTFOLDERS1 + i, false);
		}
	}
}

//=============================================
// @brief Loads recent folders list
//
//=============================================
void CMDLViewer::LoadRecentFolders( void )
{
	for(Uint32 i = 0; i < MAX_RECENT_FILES; i++)
	{
		CString optionName;
		optionName << RECENT_FOLDER_PREFIX << i;

		const Char* pstrValue = gConfig.GetOptionValue(optionName.c_str());
		if(!pstrValue && !m_recentFoldersArray[i].empty())
			m_recentFoldersArray[i].clear();
		else
			m_recentFoldersArray[i] = pstrValue;
	}
}

//=============================================
// @brief Initializes recent textures
//
//=============================================
void CMDLViewer::InitPreviousTextures( void )
{
	// TODO Move this to CGLWindow!
	CGLWindow* pGLWindow = CGLWindow::GetInstance();

	const Char* pstrValue = gConfig.GetOptionValue(VIEWER_ENV_TEX_FILE_PATH);
	if(pstrValue)
		pGLWindow->LoadSkyboxTextures(pstrValue);

	pstrValue = gConfig.GetOptionValue(VIEWER_GROUND_TEX_FILE_PATH);
	if(pstrValue)
		pGLWindow->LoadGroundTexture(pstrValue);
}

//=============================================
// @brief Saves recent files list
//
//=============================================
void CMDLViewer::SaveRecentFiles( void )
{
	for(int i = 0; i < MAX_RECENT_FILES; i++)
	{
		if(m_recentMDLFilesArray[i].empty())
			break;

		CString optionName;
		optionName << RECENT_MDL_FILE_PREFIX << i;
		gConfig.SetOption(optionName.c_str(), m_recentMDLFilesArray[i].c_str());
	}

	gConfig.SaveOptions();
}

//=============================================
// @brief Saves recent folders list
//
//=============================================
void CMDLViewer::SaveRecentFolders( void )
{
	for(int i = 0; i < MAX_RECENT_FILES; i++)
	{
		if(m_recentFoldersArray[i].empty())
			break;

		CString optionName;
		optionName << RECENT_FOLDER_PREFIX << i;
		gConfig.SetOption(optionName.c_str(), m_recentFoldersArray[i].c_str());
	}

	gConfig.SaveOptions();
}

//=============================================
// @brief Handles an mx event
//
//=============================================
Int32 CMDLViewer::handleEvent( mxEvent *pEvent )
{
	switch (pEvent->event)
	{
	case mxEvent::KeyDown:
		{
			return CGLWindow::GetInstance()->handleEvent(pEvent);
		}
		break;
	case mxEvent::KeyUp:
		{
			return CGLWindow::GetInstance()->handleEvent(pEvent);
		}
	break;
	case mxEvent::MouseWheeled:
		{
			return CGLWindow::GetInstance()->handleEvent(pEvent);
		}
	break;
	case mxEvent::Action:
		{
			switch (pEvent->action)
			{
			case IDC_FILE_LOADMODEL:
				{
					const Char *pstrLastLoadPath = gConfig.GetOptionValue(VIEWER_MDL_LOAD_PATH);
					const Char *pstrFilePath = mxGetOpenFileName (this, pstrLastLoadPath, "*.mdl");
					if (pstrFilePath)
					{
						CString dirpath;
						Viewer_GetDirectoryPath(pstrFilePath, dirpath);
						gConfig.SetOption(VIEWER_MDL_LOAD_PATH, dirpath.c_str());

						CControlPanel* pControlPanel = CControlPanel::GetInstance();
						pControlPanel->LoadModel(pstrFilePath);

						Uint32 i = 0;
						for(; i < MAX_RECENT_FILES; i++)
						{
							if(!qstrcicmp(m_recentMDLFilesArray[i], pstrFilePath))
								break;
						}

						if(i != MAX_RECENT_FILES)
						{
							CString tmp = m_recentMDLFilesArray[0];
							m_recentMDLFilesArray[0] = m_recentMDLFilesArray[i];
							m_recentMDLFilesArray[i] = tmp;
						}
						else
						{
							for(i = MAX_RECENT_FILES-1; i > 0; i--)
								m_recentMDLFilesArray[i] = m_recentMDLFilesArray[i - 1];

							m_recentMDLFilesArray[0] = pstrFilePath;
						}

						InitRecentFiles();
					}
				}
				break;
			case IDC_FILE_COMPILEMODEL:
				{
					CCompilerWindow* pCompilerWindow = CCompilerWindow::GetInstance();
					pCompilerWindow->GetHistory();
					pCompilerWindow->setVisible(true);
				}
				break;
			case IDC_FILE_DECOMPILEMODEL:
				{
					const Char* pstrDecompilerPath = gConfig.GetOptionValue(COptionsWindow::OW_DECOMPILER_PATH);
					if(!pstrDecompilerPath)
					{
						mxMessageBox (this, "Decompiler not specified.", VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
					}
					else
					{
						const Char *pstrLastDecompilePath = gConfig.GetOptionValue(VIEWER_MDL_DECOMPILE_PATH);
						const Char *pstrFilePath = mxGetOpenFileName(this, pstrLastDecompilePath, "*.mdl");
						if(pstrFilePath)
						{
							// Get dir path
							CString directoryPath;
							Viewer_GetDirectoryPath(pstrFilePath, directoryPath);
							gConfig.SetOption(VIEWER_MDL_DECOMPILE_PATH, directoryPath.c_str());

							CString cmd;
							cmd << pstrDecompilerPath << " " << pstrFilePath;
							system( cmd.c_str() );
						}
					}
				}
				break;
			case IDC_FILE_LOADBACKGROUNDTEX:
				{
					const Char *pstrLastEnvTexPath = gConfig.GetOptionValue(VIEWER_ENV_TEX_PATH);
					const Char *pstrFilePath = mxGetOpenFileName(this, pstrLastEnvTexPath, "*.*");
					if (pstrFilePath)
					{
						// Get dir path
						CString directoryPath;
						Viewer_GetDirectoryPath(pstrFilePath, directoryPath);
						gConfig.SetOption(VIEWER_ENV_TEX_PATH, directoryPath.c_str());

						CGLWindow* pGLWindow = CGLWindow::GetInstance();
						CControlPanel* pControlPanel = CControlPanel::GetInstance();

						if(pGLWindow->LoadSkyboxTextures(pstrFilePath))
						{
							vs.skyboxtexfile = pstrFilePath;
							pControlPanel->SetShowSkybox(true);
						}
						else
						{
							mxMessageBox(this, "Error loading texture.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
							vs.skyboxtexfile.clear();
							pControlPanel->SetShowSkybox(false);
						}
					}
				}
				break;
			case IDC_FILE_LOADGROUNDTEX:
				{
					const Char *pstrLastGroundTexPath = gConfig.GetOptionValue(VIEWER_GROUND_TEX_PATH);
					const Char *pstrFilePath = mxGetOpenFileName (this, pstrLastGroundTexPath, "*.*");
					if (pstrFilePath)
					{
						// Get dir path
						CString directoryPath;
						Viewer_GetDirectoryPath(pstrFilePath, directoryPath);
						gConfig.SetOption(VIEWER_GROUND_TEX_PATH, directoryPath.c_str());

						CGLWindow* pGLWindow = CGLWindow::GetInstance();
						CControlPanel* pControlPanel = CControlPanel::GetInstance();

						if(pGLWindow->LoadGroundTexture(pstrFilePath))
						{
							vs.groundtexfile = pstrFilePath;
							pControlPanel->SetShowGround(true);
						}
						else
						{
							vs.groundtexfile.clear();
							pControlPanel->SetShowGround(false);
						}
					}
				}
				break;
			case IDC_FILE_UNLOADGROUNDTEX:
				{
					CGLWindow::GetInstance()->LoadGroundTexture(nullptr);
					CControlPanel::GetInstance()->SetShowGround(false);
					gConfig.EraseOption(VIEWER_GROUND_TEX_FILE_PATH);
				}
				break;
			case IDC_FILE_RECENTMODELS1:
			case IDC_FILE_RECENTMODELS2:
			case IDC_FILE_RECENTMODELS3:
			case IDC_FILE_RECENTMODELS4:
				{
					Int32 i = pEvent->action - IDC_FILE_RECENTMODELS1;
					CControlPanel::GetInstance()->LoadModel(m_recentMDLFilesArray[i].c_str());

					CString tmp = m_recentMDLFilesArray[0];
					m_recentMDLFilesArray[0] = m_recentMDLFilesArray[i];
					m_recentMDLFilesArray[i] = tmp;
					InitRecentFiles();
					redraw();
				}
				break;
			case IDC_FILE_RECENTFOLDERS1:
			case IDC_FILE_RECENTFOLDERS2:
			case IDC_FILE_RECENTFOLDERS3:
			case IDC_FILE_RECENTFOLDERS4:
				{
					Int32 i = pEvent->action - IDC_FILE_RECENTFOLDERS1;

					CFolderViewer* pFolderViewer = CFolderViewer::GetInstance();
					pFolderViewer->OpenFolder(m_recentFoldersArray[i].c_str());

					CString tmp = m_recentFoldersArray[0];
					m_recentFoldersArray[0] = m_recentFoldersArray[i];
					m_recentFoldersArray[i] = tmp;
					InitRecentFolders();
					redraw();
				}
				break;
			case IDC_FILE_EXIT:
				{
					Viewer_Shutdown();
					mx::quit();
				}
				break;
			case IDC_OPTIONS_COLORBACKGROUND:
			case IDC_OPTIONS_COLORGROUND:
			case IDC_OPTIONS_COLORLIGHT:
				{
					Vector* pColor = nullptr;
					switch(pEvent->action)
					{
					case IDC_OPTIONS_COLORBACKGROUND:
						pColor = &vs.backgroundcolor;
						break;
					case IDC_OPTIONS_COLORGROUND:
						pColor = &vs.groundcolor;
						break;
					case IDC_OPTIONS_COLORLIGHT:
						pColor = &vs.lightcolor;
						break;
					}

					if(pColor)
					{
						Int32 r = (Int32)((*pColor)[0] * 255.0f);
						Int32 g = (Int32)((*pColor)[1] * 255.0f);
						Int32 b = (Int32)((*pColor)[2] * 255.0f);

						r = clamp(r, 0, 255);
						g = clamp(g, 0, 255);
						b = clamp(b, 0, 255);

						if (mxChooseColor (this, &r, &g, &b))
						{
							(*pColor)[0] = (Float)r / 255.0f;
							(*pColor)[1] = (Float)g / 255.0f;
							(*pColor)[2] = (Float)b / 255.0f;
						}

						for(Uint32 i = 0; i < 3; i++)
							(*pColor)[i] = clamp((*pColor)[i], 0, 255);

						CString colorOptionName;
						switch(pEvent->action)
						{
						case IDC_OPTIONS_COLORBACKGROUND:
							colorOptionName = VIEWER_COLOR_BACKGROUND;
							break;
						case IDC_OPTIONS_COLORGROUND:
							colorOptionName = VIEWER_COLOR_GROUND;
							break;
						case IDC_OPTIONS_COLORLIGHT:
							colorOptionName = VIEWER_COLOR_LIGHT;
							break;
						}

						CString value;
						value << (*pColor)[0] << " " << (*pColor)[1] << " " << (*pColor)[2];
						gConfig.SetOption(colorOptionName.c_str(), value.c_str());
					}
				}
				break;
			case IDC_OPTIONS_CENTERVIEW:
				{
					CControlPanel::GetInstance()->CenterView();
				}
				break;
			case IDC_OPTIONS_MAKESCREENSHOT:
				{
					CString filename = mxGetSaveFileName(this, "", "*.tga");
					filename.tolower();

					if(!filename.empty())
					{
						if(filename.find(0, ".tga") == -1)
							filename << ".tga";

						CGLWindow::GetInstance()->DumpViewport(filename.c_str());
					}
				}
				break;
			case IDC_VIEW_FILEASSOCIATIONS:
				{
					CFileAssociation* pAssocWindow = CFileAssociation::GetInstance();
					pAssocWindow->SetAssociation(0);
					pAssocWindow->setVisible(true);
				}
				break;
			case IDC_OPTIONS_OPTIONSWINDOW:
				{
					COptionsWindow::GetInstance()->setVisible(true);
				}
				break;
			case IDC_HELP_ABOUT:
				mxMessageBox (this,
					"Pathos Model Viewer v1.0 (c) 2021 by Andrew Lucas, based on HLMV by Mete Ciragan\n\n"
					"Left-drag to rotate.\n"
					"Right-drag to zoom.\n"
					"Shift-left-drag to x-y-pan.\n\n"
					"Build:\t" __DATE__ ".\n"
					"Email: doommusic666@hotmail.com\n", "About Pathos Model Viewer",
					MX_MB_OK | MX_MB_INFORMATION);
				break;
				case IDC_FILE_OPENFOLDER:
				{
					const Char *pstrFolderPath = mxGetSelectFolder(this);
					if(pstrFolderPath)
					{
						CFolderViewer* pFolderViewer = CFolderViewer::GetInstance();
						pFolderViewer->OpenFolder(pstrFolderPath);

						Uint32 i = 0;
						for(; i < MAX_RECENT_FILES; i++)
						{
							if(!qstrcicmp(m_recentFoldersArray[i], pstrFolderPath))
								break;
						}

						if(i != MAX_RECENT_FILES)
						{
							CString tmp = m_recentFoldersArray[0];
							m_recentFoldersArray[0] = m_recentFoldersArray[i];
							m_recentFoldersArray[i] = tmp;
						}
						else
						{
							for(i = MAX_RECENT_FILES-1; i > 0; i--)
								m_recentFoldersArray[i] = m_recentFoldersArray[i - 1];

							m_recentFoldersArray[0] = pstrFolderPath;
						}

						InitRecentFolders();
						redraw();
					}
				}
				break;
				case IDC_FILE_CLOSEFOLDER:
				{
					CFolderViewer* pFolderViewer = CFolderViewer::GetInstance();
					pFolderViewer->CloseFolder();
					redraw ();
				}
				break;
			}
		}
		break;
		case mxEvent::Size:
		{
			int w = pEvent->width;
			int h = pEvent->height;
			int y = m_pMenuBar->getHeight ();
#ifdef WIN32
#define	HEIGHT 120
#else
#define HEIGHT 140
			h -= 40;
#endif
			CFolderViewer* pFolderViewer = CFolderViewer::GetInstance();
			if(pFolderViewer && pFolderViewer->isVisible())
			{
				w -= 170;
				pFolderViewer->setBounds(w, y, 170, h);
			}

			CGLWindow::GetInstance()->setBounds(0, y, w, h - HEIGHT);
			CControlPanel::GetInstance()->setBounds(0, y + h - HEIGHT, w, HEIGHT);
		}
		break;
	}

	return 1;
}

//=============================================
// @brief Redraws the object
//
//=============================================
void CMDLViewer::redraw ()
{
	mxEvent event;
	event.event = mxEvent::Size;
	event.width = w2();
	event.height = h2();

	handleEvent (&event);
}

//=============================================
// @brief Creates an instance of this class
//
// @return Created instance
//=============================================
CMDLViewer* CMDLViewer::CreateInstance( void )
{
	if(!g_pInstance)
		g_pInstance = new CMDLViewer();

	return g_pInstance;
}

//=============================================
// @brief Returns the current instance of this class
//
// @return Current instance, or nullptr
//=============================================
CMDLViewer* CMDLViewer::GetInstance( void )
{
	return g_pInstance;
}

//=============================================
// @brief Deletes the current instance of this class
//
//=============================================
void CMDLViewer::DeleteInstance( void )
{
	if(!g_pInstance)
		return;

	// TODO
	//delete g_pInstance;
	g_pInstance = nullptr;
}

