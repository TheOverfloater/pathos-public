/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef WINDOW_H
#define WINDOW_H

#include <SDL_video.h>

/*
=================================
-Class: CWindow
-Description:

=================================
*/
class CWindow
{
public:
	// Minimum screen width
	static const Uint32 MIN_SCREEN_WIDTH;
	// Minimum screen height
	static const Uint32 MIN_SCREEN_HEIGHT;
	// Maximum MSAA value
	static const Uint32 MAX_MSAA_VALUE;

public:
	struct resolution_t
	{
		resolution_t():
			index(0),
			width(0),
			height(0),
			rate(0)
		{}

		Uint32 index;
		Uint32 width;
		Uint32 height;
		Uint32 rate;
	};

	struct ddevice_t
	{
		ddevice_t():
			index(0),
			xOffset(0),
			yOffset(0)
		{}

		CString name;
		Int32 index;

		Int32 xOffset;
		Int32 yOffset;

		CArray<resolution_t> resolutions;
	};

public:
	CWindow( void );
	~CWindow();

public:
	bool Init( void );
	void DestroyWindow( void );

	void SwapWindow( void );

public:
	Uint32 GetWidth( void ) const;
	Uint32 GetHeight( void ) const;
	Uint32 GetRefreshRate( void ) const;

	Int32 GetCenterX( void ) const;
	Int32 GetCenterY( void ) const;

	SDL_Window* GetWindow() { return m_pSDLWindow; }

	Int32 GetNbResolutions( Int32 deviceIndex ) const;
	void GetResolutionInfo( Int32 deviceIndex, Int32 index, Uint32& width, Uint32& height ) const;
	Int32 GetCurrentResolutionIndex( void ) const;

	Int32 GetNbDisplayDevices( void ) const;
	const Char* GetDisplayDeviceName( Int32 index ) const;
	Int32 GetCurrentDeviceIndex( void ) const;

	bool GetOpenGLInfo( Int32& maxMSAA, bool& fboSupported );

	bool IsFullScreen( void ) const { return m_bFullScreen; }
	bool IsVerticalSyncEnabled( void ) { return m_bVerticalSync; }

	Int32 GetCurrentMSAASetting( void );
	Uint32 GetNbMSAASettings( void );
	Int32 GetMSAASetting( Int32 index );

private:
	bool BuildDeviceList( void );
	static bool FetchResolutions( ddevice_t* pdevice );

	bool SetDisplayProperties( void );
	struct conf_group_t* GetConfigGroup( void );

	resolution_t* FindResolution( ddevice_t* pdevice, Uint32 width, Uint32 height );

public:
	void SetActive( bool bActive );
	bool IsActive( void ) const;
	bool IsInitialized( void ) const;
	bool IsMSAAEnabled( void ) const { return m_bIsMSAAEnabled; }
	bool AreFBOsSupported(void) const { return m_areFBOsSupported; }
	bool AreFBOsEnabled(void) const { return m_areFBOsEnabled; }

private:
	Int32			m_iDisplayDevice;
	bool			m_bFullScreen;
	bool			m_bVerticalSync;
	bool			m_bIsMSAAEnabled;
	bool			m_areFBOsSupported;
	bool			m_areFBOsEnabled;

private:
	// TRUE if the window is in focus
	bool			m_bWindowActive;
	// TRUE if the window is initialized
	bool			m_bWindowInitialized;

private:
	// SDL Window object
	SDL_Window* m_pSDLWindow;
	// SDL Context object
	SDL_GLContext m_sdlContext;

	// Device list
	CArray<ddevice_t*> m_devicesArray;

	// Currently active device
	ddevice_t* m_pActiveDevice;
	// Pointer to active resolution struct
	resolution_t* m_pCurrentRes;

	// Multisample settings array
	CArray<Int32> m_multiSampleSettingsArray;
};
extern CWindow gWindow;
#endif
