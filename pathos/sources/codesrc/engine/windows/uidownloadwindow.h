/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef UIDOWNLOADWINDOW_H
#define UIDOWNLOADWINDOW_H

/*
=================================
CUIDownloadWindow

=================================
*/
class CUIDownloadWindow : public CUIWindow
{
public:
	// Window description file
	static const Char WINDOW_DESC_FILE[];
	// Window object name
	static const Char WINDOW_OBJ_NAME[];
	// Cancel button object name
	static const Char CANCEL_BUTTON_OBJ_NAME[];
	// Save and Quit button object name
	static const Char FILE_DOWNLOAD_LABEL_OBJ_NAME[];
	// Quit button object name
	static const Char FILE_DOWNLOAD_PROGRESSBAR_OBJ_NAME[];
	// Save and Quit button object name
	static const Char TOTAL_PROGRESS_LABEL_OBJ_NAME[];
	// Quit button object name
	static const Char TOTAL_PROGRESS_PROGRESSBAR_OBJ_NAME[];

private:
	CUIDownloadWindow( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy );
public:
	~CUIDownloadWindow( void );

public:
	// Creates an instance of the console window
	static CUIDownloadWindow* CreateInstance( void );
	// Returns the current instance of the console window
	static CUIDownloadWindow* GetInstance( void );
	// Destroys the current instance
	static void DestroyInstance( void );

public:
	// Loads the schema, and creates the sub-elements
	virtual bool init( const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pWindowObject );
	// The settings window cannot be resized
	virtual bool isResizable( void ) override { return false; }

public:
	// Manages a button press event
	void CancelDownload( void );
	// Sets file progress meter
	void SetFileProgressBar( Float value );
	// Sets total progress bar
	void SetTotalProgressBar( Float value );
	// Sets the downloaded file name
	void SetDownloadFileName( const Char* pstr );

private:
	// Base text for download bar
	CString m_downloadBarTextBase;
	// Text for the downloaded file
	CUIText* m_pDownloadFileText;
	// Progress bar for file download
	CUIProgressBar* m_pFileProgressBar;
	// Progress bar for total progress
	CUIProgressBar* m_pTotalProgressBar;

public:
	// Current instance
	static CUIDownloadWindow* m_pInstance;
};

/*
=================================
CUIDownloadWindowCancelEvent

=================================
*/
class CUIDownloadWindowCancelEvent : public CUICallbackEvent
{
public:
	explicit CUIDownloadWindowCancelEvent( CUIDownloadWindow* pWindow ):
		m_pWindow(pWindow)
	{ };
	virtual ~CUIDownloadWindowCancelEvent( void ) { };

public:
	// Performs the action
	virtual void PerformAction( Float param ) override { m_pWindow->CancelDownload(); }

protected:
	// Window that created this
	CUIDownloadWindow* m_pWindow;
};

#endif //UIDOWNLOADWINDOW_H