/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef UISAVELOADWINDOW_H
#define UISAVELOADWINDOW_H

class CUISaveLoadWindow;

#include "savefile.h"

enum slwindow_btn_id
{
	SL_BUTTON_LOAD_GAME = 0,
	SL_BUTTON_SAVE_GAME,
	SL_BUTTON_CANCEL,
	SL_BUTTON_DELETE
};

/*
=================================
CUISaveLoadWindow

=================================
*/
class CUISaveLoadWindow : public CUIWindow
{
public:
	struct save_file_t
	{
		save_file_t():
			gametime(0),
			type(SAVE_UNDEFINED)
			{}

		Char filepath[MAX_PARSE_LENGTH];
		Char savetitle[SAVE_FILE_HEADER_MAX_LENGTH];
		
		Float gametime;
		CString datestring;
		savefile_type_t type;

		file_dateinfo_t date;
	};

public:
	// Window description file
	static const Char WINDOW_DESC_FILE[];
	// Window object name
	static const Char WINDOW_OBJ_NAME[];
	// Save list object name
	static const Char SAVELIST_OBJ_NAME[];
	// Cancel button object name
	static const Char CANCEL_BUTTON_OBJ_NAME[];
	// Load game button object name
	static const Char LOAD_GAME_BUTTON_OBJ_NAME[];
	// Save game button object name
	static const Char SAVE_GAME_BUTTON_OBJ_NAME[];
	// Delete save button object name
	static const Char DELETE_SAVE_BUTTON_OBJ_NAME[];

private:
	CUISaveLoadWindow( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
public:
	~CUISaveLoadWindow( void );

public:
	// Creates an instance of the console window
	static CUISaveLoadWindow* CreateInstance( bool isIngame );
	// Returns the current instance of the console window
	static CUISaveLoadWindow* GetInstance( void );
	// Destroys the current instance
	static void DestroyInstance( void );

	// Sets the focus state
	void setFocusState( bool inFocus );
	// Reacts to a key event
	bool keyEvent( Int32 button, Int16 mod, bool keyDown );
	// Handles post-think functionalities
	void postThink( void );

public:
	// Set focus on a specific row
	void SetFocusOnRow( Int32 rowIndex, Int32 fileIndex );
	// Set focus on a specific row
	void RowDoubleClickEvent( Int32 rowIndex );
	// Called to delete a save file
	void DeleteSave( const Char* pstrSavePath );

	// Reacts to a button being pressed
	void ButtonEvent( slwindow_btn_id btn );
	// Loads a game file
	void LoadSave( const Char* pstrSavePath );
	// Loads save files from the save folder
	void LoadSaves( bool isIngame );
	// Marks for a save re-check
	void MarkRecheckSaves( void ) { m_bRecheckSaves = true; }

private:
	// Retrieves save file info
	void AddSaveFileInfo( const CString& filePath, FILETIME& fileTime );
	// Sets background texture
	void SetBackgroundTexture( save_file_t* psave );
	// Creates a save file row
	void CreateSaveFileRow( save_file_t& saveFile, Uint32 fileindex );

	// Updates buttons
	void UpdateButtons( bool selectFirst );

public:
	// Loads the schema, and creates the sub-elements
	virtual bool init( const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pWindowObject, bool isIngame );
	// The settings window cannot be resized
	virtual bool isResizable( void ) { return false; }

private:
	// Save file list
	CUIList* m_pSaveList;
	// Array of save file infos
	CArray<save_file_t> m_saveFilesArray;
	// Last selected save file
	Int32 m_lastSelectedSave;

	// TRUE if we need to re-check the save dir
	bool m_bRecheckSaves;
	// Text inset for rows
	Uint32 m_rowTextInset;
	// List info object
	const ui_objectinfo_t* m_pListObjectInfo;
	// TRUE if we're in an active game
	bool m_bIsIngame;

	// Save button
	CUIButton* m_pSaveButton;
	// Load button
	CUIButton* m_pLoadButton;
	// Delete button
	CUIButton* m_pDeleteButton;

public:
	// Current instance
	static CUISaveLoadWindow* m_pInstance;
};

/*
=================================
CUISaveLoadWindowRowEvent

=================================
*/
class CUISaveLoadWindowRowEvent : public CUICallbackEvent
{
public:
	CUISaveLoadWindowRowEvent( CUISaveLoadWindow* pWindow, Int32 rowIndex, Int32 fileIndex ):
		m_lastClickTime(0),
		m_rowIndex(rowIndex),
		m_fileIndex(fileIndex),
		m_pWindow(pWindow)
	{ };
	virtual ~CUISaveLoadWindowRowEvent( void ) { };

public:
	// Performs the action
	virtual void PerformAction( Float param ) { }
	// Handles a mouse button event
	virtual bool MouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown );

protected:
	// Last click time
	Double m_lastClickTime;
	// Row index
	Int32 m_rowIndex;
	// File index
	Int32 m_fileIndex;
	// Window that created this
	CUISaveLoadWindow* m_pWindow;
};

/*
=================================
CUISaveLoadWindowButtonEvent

=================================
*/
class CUISaveLoadWindowButtonEvent : public CUICallbackEvent
{
public:
	CUISaveLoadWindowButtonEvent( CUISaveLoadWindow* pWindow, slwindow_btn_id btnId ):
		m_buttonId(btnId),
		m_pWindow(pWindow)
	{ };
	virtual ~CUISaveLoadWindowButtonEvent( void ) { };

public:
	// Performs the action
	virtual void PerformAction( Float param );

protected:
	// Row index
	slwindow_btn_id m_buttonId;
	// Window that created this
	CUISaveLoadWindow* m_pWindow;
};
#endif //UISAVELOADWINDOW_H