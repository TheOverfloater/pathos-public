/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <SDL.h>

#include "includes.h"
#include "window.h"
#include "system.h"
#include "config.h"
#include "enginestate.h"
#include "input.h"
#include "texturemanager.h"

// Default is 60 frames per sec
#define DEFAULT_REFRESH_RATE 60

// Minimum screen width
const Uint32 CWindow::MIN_SCREEN_WIDTH = 640;
// Minimum screen height
const Uint32 CWindow::MIN_SCREEN_HEIGHT = 480;
// Maximum MSAA value
const Uint32 CWindow::MAX_MSAA_VALUE = 8;

//CWindow class definition
CWindow gWindow;

//=============================================
// Class: CWindow
// Function: CWindow
//=============================================
CWindow::CWindow( void ):
	m_iDisplayDevice(0),
	m_bFullScreen(false),
	m_bVerticalSync(false),
	m_bIsMSAAEnabled(false),
	m_bWindowActive(false),
	m_bWindowInitialized(false),
	m_pSDLWindow(nullptr),
	m_sdlContext(nullptr),
	m_pActiveDevice(nullptr),
	m_pCurrentRes(nullptr)
{
}

//=============================================
// Class: CWindow
// Function: ~CWindow
//=============================================
CWindow::~CWindow( void )
{
	DestroyWindow();

	// Delete lists
	if(!m_devicesArray.empty())
	{
		for(Uint32 i = 0; i < m_devicesArray.size(); i++)
		{
			m_devicesArray[i]->resolutions.clear();
			delete m_devicesArray[i];
		}
	
		m_devicesArray.clear();
	}
}

//=============================================
// Class: CWindow
// Function: Init
//=============================================
Int32 CWindow::GetMaxMultiSample( void )
{
	// Create the temporary window
	SDL_Window* pTempWindow = SDL_CreateWindow(ens.gametitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		4, 4, (SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN));

	if(!pTempWindow)
		return 0;

	SDL_GLContext tempContext = SDL_GL_CreateContext(pTempWindow);
	const Char* pstrError = SDL_GetError();
	if(pstrError[0] != '\0')
	{
		SDL_DestroyWindow(pTempWindow);
		return 0;
	}

	Int32 maxSamples;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

	SDL_GL_DeleteContext(tempContext);
	SDL_DestroyWindow(pTempWindow);

	// Seems GL doesn't tolerate more
	// than 12x, and above 8x menu items
	// look artifacted
	if(maxSamples > MAX_MSAA_VALUE)
		maxSamples = MAX_MSAA_VALUE;

	return maxSamples;
}

//=============================================
// Class: CWindow
// Function: Init
//=============================================
bool CWindow::Init( void )
{
	// Get active devices
	if(SDL_VideoInit(nullptr))
	{
		Con_EPrintf("%s failed: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}
	       
	// Build the list of display devices
	if(!BuildDeviceList())
		return false;

	// Get active resolution
	if(!SetDisplayProperties())
	{
		Con_EPrintf("Failed to set display properties.\n");
		return false;
	}

	// Set the OpenGL version
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, SDL_TRUE);

#ifdef _DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_RELEASE_BEHAVIOR, SDL_GL_CONTEXT_RELEASE_BEHAVIOR_NONE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_RELEASE_BEHAVIOR, SDL_GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_FALSE);
#endif

	// Get depth buffer bit count
	Int32 nbDepthBits = gConfig.GetInt(GetConfigGroup(), "DepthBits");
	if(gConfig.GetStatus() != CONF_ERR_NONE || nbDepthBits != 24)
	{
		nbDepthBits = 24;
		gConfig.SetValue(GetConfigGroup(), "DepthBits", (Int32)nbDepthBits, true);
	}

	// Get stencil bits count
	Int32 nbStencilBits = gConfig.GetInt(GetConfigGroup(), "StencilBits");
	if(gConfig.GetStatus() != CONF_ERR_NONE)
	{
		nbStencilBits = 0;
		gConfig.SetValue(GetConfigGroup(), "StencilBits", (Int32)nbStencilBits, true);
	}

	// Turn on double buffering with a 24bit z buffer
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, nbDepthBits);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, nbStencilBits);

	if(Sys_CheckLaunchArgs("-lowcolorwidth") == NO_POSITION)
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 10);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 10);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 10);
	}

	Int32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	if(m_bFullScreen)
		windowFlags |= SDL_WINDOW_FULLSCREEN;

	if(Sys_CheckLaunchArgs("-borderless") != NO_POSITION)
		windowFlags |= SDL_WINDOW_BORDERLESS;

	Int32 xPos;
	Int32 yPos;
	if(m_bFullScreen)
	{
		xPos = m_pActiveDevice->xOffset;
		yPos = m_pActiveDevice->yOffset;
	}
	else
	{
		xPos = SDL_WINDOWPOS_CENTERED;
		yPos = SDL_WINDOWPOS_CENTERED;
	}

	if(SDL_Init(SDL_INIT_VIDEO))
	{
		Sys_ErrorPopup("SDL_Init for SDL_INIT_VIDEO returned an error: %s", SDL_GetError());
		return false;
	}

	// Set default(off) value for multisample
	if(!m_multiSampleSettingsArray.empty())
		m_multiSampleSettingsArray.clear();

	m_multiSampleSettingsArray.push_back(0);

	// Set up multisampling
	Int32 maxMultiSample = GetMaxMultiSample();
	for(Int32 i = 2; i <= maxMultiSample; i += 2)
		m_multiSampleSettingsArray.push_back(i);

	// Get current MSAA setting
	Int32 currentMSAASetting = GetCurrentMSAASetting();
	Int32 msaaSettingValue;
	if(ens.requestedMSAASetting != -1 && ens.requestedMSAASetting != currentMSAASetting)
	{
		msaaSettingValue = m_multiSampleSettingsArray[ens.requestedMSAASetting];
		gConfig.SetValue(GetConfigGroup(), "MSAASetting", msaaSettingValue, true);
	}
	else
	{
		msaaSettingValue = m_multiSampleSettingsArray[currentMSAASetting];
	}

	ens.requestedMSAASetting = -1;

	if(msaaSettingValue != 0)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaaSettingValue);
		m_bIsMSAAEnabled = true;
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		m_bIsMSAAEnabled = false;
	}

	// Create the window
	m_pSDLWindow = SDL_CreateWindow(ens.gametitle.c_str(), xPos, yPos, 
		m_pCurrentRes->width, m_pCurrentRes->height, windowFlags);

	if(!m_pSDLWindow)
	{
		Con_EPrintf("Failed to create window. SDL: %s.", SDL_GetError());
		return false;
	}

	m_sdlContext = SDL_GL_CreateContext(m_pSDLWindow);
	const Char* pstrError = SDL_GetError();
	if(pstrError[0] != '\0')
	{
		Con_EPrintf("Failed to create GL context. SDL: %s.", pstrError);
		return false;
	}

	if(msaaSettingValue != 0)
		glEnable(GL_MULTISAMPLE);
	else
		glDisable(GL_MULTISAMPLE);

	// Set vertical sync
	SDL_GL_SetSwapInterval( m_bVerticalSync ? 1 : 0 );

	// Set as active(default)
	m_bWindowActive = true;
	m_bWindowInitialized = true;

	return true;
}

//=============================================
// Class: CWindow
// Function: DestroyWindow
//=============================================
void CWindow::DestroyWindow( void )
{
	SDL_GL_DeleteContext(m_sdlContext);

	if(m_pSDLWindow)
	{
		SDL_DestroyWindow(m_pSDLWindow);
		m_pSDLWindow = nullptr;
	}

	SDL_VideoQuit();

	m_bWindowActive = false;
	m_bWindowInitialized = false;

	m_pCurrentRes = nullptr;
}

//=============================================
// Class: CWindow
// Function: BuildDeviceList
//=============================================
bool CWindow::BuildDeviceList( void )
{
	// Rebuild the list
	if(!m_devicesArray.empty())
	{
		for(Uint32 i = 0; i < m_devicesArray.size(); i++)
		{
			m_devicesArray[i]->resolutions.clear();
			delete m_devicesArray[i];
		}

		m_devicesArray.clear();
	}

	Int32 numDisplays = SDL_GetNumVideoDisplays();
	for( Uint32 i = 0; i < (Uint32)numDisplays; i++ ) 
	{
		ddevice_t *newDevice = new ddevice_t;
		CString deviceName;
		deviceName << (Int32)(i+1) << " - " << SDL_GetDisplayName(i);

		newDevice->name = deviceName;
		newDevice->index = i;

		SDL_Rect monRect;
		SDL_GetDisplayBounds(i, &monRect);
		newDevice->xOffset = monRect.x;
		newDevice->yOffset = monRect.y;

		if(!FetchResolutions(newDevice))
		{
			delete newDevice;
			continue;
		}

		m_devicesArray.push_back(newDevice);
	}

	if(!numDisplays)
	{
		Con_EPrintf("No valid display devices found.\n");
		return false;
	}

	return true;
}

//=============================================
// Class: CWindow
// Function: FetchResolutions
//=============================================
bool CWindow::FetchResolutions( ddevice_t* pdevice )
{
	Int32 nbResolutions = SDL_GetNumDisplayModes(pdevice->index);
	if(!nbResolutions)
	{
		Con_EPrintf("Couldn't get display mode number for device %s.\n", pdevice->name.c_str());
		return false;
	}

	pdevice->resolutions.reserve(nbResolutions);

	Uint32 resindex = 0;
	for(Uint32 i = 0; i < (Uint32)nbResolutions; i++) 
	{
		SDL_DisplayMode dMode;
		dMode.driverdata = nullptr;

		if(SDL_GetDisplayMode(pdevice->index, i, &dMode))
		{
			Con_EPrintf("Couldn't get display information: %s.\n", SDL_GetError());
			return false;
		}

		if(dMode.w < MIN_SCREEN_WIDTH || dMode.h < MIN_SCREEN_HEIGHT)
			continue;

		Uint32 j = 0;
		for(; j < pdevice->resolutions.size(); j++)
		{
			if(pdevice->resolutions[j].width == (Uint32)dMode.w 
				&& pdevice->resolutions[j].height == (Uint32)dMode.h)
				break;
		}

		if(j != pdevice->resolutions.size())
			continue;

		resolution_t newRes;
		newRes.index = resindex;
		newRes.width = dMode.w;
		newRes.height = dMode.h;
		newRes.rate = dMode.refresh_rate;

		pdevice->resolutions.push_back(newRes);
		resindex++;
	}

	pdevice->resolutions.resize(pdevice->resolutions.size());

	if(pdevice->resolutions.empty())
	{
		Con_EPrintf("No valid resolutions fetched for device '%s'.\n", pdevice->name.c_str());
		return false;
	}

	return true;
}

//=============================================
// Class: CWindow
// Function: FindResolution
//=============================================
CWindow::resolution_t* CWindow::FindResolution( ddevice_t* pdevice, Uint32 width, Uint32 height )
{
	for(Uint32 i = 0; i < pdevice->resolutions.size(); i++)
	{
		resolution_t *pResolution = &pdevice->resolutions[i];

		if(pResolution->width == width && pResolution->height == height)
			return pResolution;
	}

	return nullptr;
}

//=============================================
// Class: CWindow
// Function: SetDisplayProperties
//=============================================
bool CWindow::SetDisplayProperties( void )
{
	CString deviceName = gConfig.GetString(GetConfigGroup(), "Device");
	if(gConfig.GetStatus() == CONF_ERR_NONE)
	{
		for(Uint32 i = 0; i < m_devicesArray.size(); i++)
		{
			if(!qstrcmp(m_devicesArray[i]->name, deviceName))
			{
				m_pActiveDevice = m_devicesArray[i];
				break;
			}
		}
	}

	// Set the desired device
	bool setDisplayDeviceConfig = false;
	if(ens.requestedDisplayDevice != -1)
	{
		if(ens.requestedDisplayDevice < 0 || (Uint32)ens.requestedDisplayDevice >= m_devicesArray.size())
			Con_EPrintf("Invalid display device requested. Option discarded.\n");
		else
		{
			m_pActiveDevice = m_devicesArray[ens.requestedDisplayDevice];
			setDisplayDeviceConfig = true;
		}

		ens.requestedDisplayDevice = -1;
	}

	if(!m_pActiveDevice)
	{
		// Set the default device
		m_pActiveDevice = m_devicesArray[0];
		setDisplayDeviceConfig = true;
	}

	if(setDisplayDeviceConfig)
		gConfig.SetValue(GetConfigGroup(), "Device", m_pActiveDevice->name.c_str(), true);

	// Check override from launch args
	if(ens.requestedScrWidth && ens.requestedScrHeight)
	{
		m_pCurrentRes = FindResolution(m_pActiveDevice, ens.requestedScrWidth, ens.requestedScrHeight);
		if(m_pCurrentRes)
		{
			gConfig.SetValue(GetConfigGroup(), "ScreenWidth", (Int32)ens.requestedScrWidth, true);
			gConfig.SetValue(GetConfigGroup(), "ScreenHeight", (Int32)ens.requestedScrHeight, true);
		}
		else
			Con_EPrintf("Invalid width/height set, discarding launch args.\n");

		// Reset this
		ens.requestedScrWidth = 0;
		ens.requestedScrHeight = 0;
	}

	if(!m_pCurrentRes)
	{
		Int32 scrnWidth = gConfig.GetInt(GetConfigGroup(), "ScreenWidth");
		Int32 scrnHeight = gConfig.GetInt(GetConfigGroup(), "ScreenHeight");

		if(scrnWidth && scrnHeight)
			m_pCurrentRes = FindResolution(m_pActiveDevice, scrnWidth, scrnHeight);
	}

	// See if we're fullscreen
	// See if we got an override from the launch params
	if(ens.requestWMode == WM_NONE)
	{
		m_bFullScreen = gConfig.GetInt(GetConfigGroup(), "Fullscreen") ? true : false;
		if(gConfig.GetStatus() != CONF_ERR_NONE)
		{
			m_bFullScreen = false;
			gConfig.SetValue(GetConfigGroup(), "Fullscreen", m_bFullScreen, true);
		}
	}
	else
	{
		if(ens.requestWMode == WM_FULLSCREEN)
			m_bFullScreen = true;
		else
			m_bFullScreen = false;

		gConfig.SetValue(GetConfigGroup(), "Fullscreen", m_bFullScreen, true);
	}

	if(!m_pCurrentRes)
	{
		// No config set, use native resolution
		SDL_DisplayMode dMode;
		if(SDL_GetCurrentDisplayMode(m_pActiveDevice->index, &dMode))
		{
			Con_EPrintf("Failed to get desktop size.\n");
			return false;
		}

		// Find it in the config list
		m_pCurrentRes = FindResolution(m_pActiveDevice, dMode.w, dMode.h);

		// Shouldn't happen
		if(!m_pCurrentRes)
		{
			m_pCurrentRes = &m_pActiveDevice->resolutions[0];

			gConfig.SetValue(GetConfigGroup(), "ScreenWidth", (Int32)m_pCurrentRes->width, true);
			gConfig.SetValue(GetConfigGroup(), "ScreenHeight", (Int32)m_pCurrentRes->height, true);
		}
	}

	// See if we're fullscreen
	m_bVerticalSync = gConfig.GetInt(GetConfigGroup(), "VerticalSync") ? true : false;
	if(gConfig.GetStatus() != CONF_ERR_NONE)
	{
		m_bVerticalSync = false;
		gConfig.SetValue(GetConfigGroup(), "VerticalSync", m_bVerticalSync, true);
	}

	if(ens.requestedVSyncSetting != -1)
	{
		bool isEnabled = ens.requestedVSyncSetting == 0 ? false : true;
		if(m_bVerticalSync != isEnabled)
		{
			gConfig.SetValue(GetConfigGroup(), "VerticalSync", isEnabled ? 1 : 0, true);
			m_bVerticalSync = isEnabled;
		}

		ens.requestedVSyncSetting = -1;
	}

	return true;
}

//=============================================
// Class: CWindow
// Function: SwapWindow
//=============================================
void CWindow::SwapWindow( void )
{
	SDL_GL_SwapWindow(m_pSDLWindow);
}

//=============================================
// Class: CWindow
// Function: SetActive
//=============================================
void CWindow::SetActive ( bool bActive )
{
	m_bWindowActive = bActive;

	// Reset mouse position
	if(m_bWindowActive)
		gInput.ResetMouse();
}

//=============================================
// Class: CWindow
// Function: IsActive
//=============================================
bool CWindow::IsActive ( void ) const
{
	return m_bWindowActive;
}

//=============================================
// Class: CWindow
// Function: GetWidth
//=============================================
Uint32 CWindow::GetWidth ( void ) const
{
	if(!m_pCurrentRes)
		return 0;

	return m_pCurrentRes->width;
}

//=============================================
// Class: CWindow
// Function: GetHeight
//=============================================
Uint32 CWindow::GetHeight ( void ) const
{
	if(!m_pCurrentRes)
		return 0;

	return m_pCurrentRes->height;
}

//=============================================
// Class: CWindow
// Function: GetHeight
//=============================================
Uint32 CWindow::GetRefreshRate ( void ) const
{
	if(!m_pCurrentRes)
		return 0;

	return m_pCurrentRes->rate;
}

//=============================================
// Class: CWindow
// Function: IsInitialized
//=============================================
bool CWindow::IsInitialized( void ) const
{
	return m_bWindowInitialized;
}

//=============================================
// Class: CWindow
// Function: GetRefreshRate
//=============================================
conf_group_t* CWindow::GetConfigGroup( void )
{
	CString groupName("Display");
	conf_group_t* pgroup = gConfig.FindGroup(groupName.c_str());
	if(!pgroup)
		pgroup = gConfig.CreateGroup(groupName.c_str(), CConfig::SYSTEM_CONFIG_FILENAME, CONF_GRP_SYSTEM);

	return pgroup;
}

//=============================================
// Class: CWindow
// Function: GetNbResolutions
//=============================================
Int32 CWindow::GetNbResolutions( Int32 deviceIndex ) const
{
	assert(deviceIndex >= 0 && deviceIndex < (Int32)m_devicesArray.size());
	return m_devicesArray[deviceIndex]->resolutions.size();
}

//=============================================
// Class: CWindow
// Function: GetResolutionInfo
//=============================================
void CWindow::GetResolutionInfo( Int32 deviceIndex, Int32 index, Uint32& width, Uint32& height ) const
{
	assert(deviceIndex >= 0 && deviceIndex < (Int32)m_devicesArray.size());
	assert(index >= 0 && index < (Int32)m_devicesArray[deviceIndex]->resolutions.size());

	width = m_devicesArray[deviceIndex]->resolutions[index].width;
	height = m_devicesArray[deviceIndex]->resolutions[index].height;
}

//=============================================
// Class: CWindow
// Function: GetCurrentDeviceIndex
//=============================================
Int32 CWindow::GetCurrentResolutionIndex( void ) const
{
	if(!m_pCurrentRes)
		return 0;

	return m_pCurrentRes->index;
}

//=============================================
// Class: CWindow
// Function: GetNbDisplayDevices
//=============================================
Int32 CWindow::GetNbDisplayDevices( void ) const
{
	return (Int32)m_devicesArray.size();
}

//=============================================
// Class: CWindow
// Function: GetResolution
//=============================================
const Char* CWindow::GetDisplayDeviceName( Int32 index ) const
{
	assert(index >= 0 && index < (Int32)m_devicesArray.size());
	return m_devicesArray[index]->name.c_str();
}

//=============================================
// Class: CWindow
// Function: GetCurrentDeviceIndex
//=============================================
Int32 CWindow::GetCurrentDeviceIndex( void ) const
{
	if(!m_pActiveDevice)
		return -1;

	return m_pActiveDevice->index;
}

//=============================================
// Class: CWindow
// Function: GetCenterX
//=============================================
Int32 CWindow::GetCenterX( void ) const
{
	return ((Int32)m_pCurrentRes->width/2);
}

//=============================================
// Class: CWindow
// Function: GetCenterY
//=============================================
Int32 CWindow::GetCenterY( void ) const
{
	return ((Int32)m_pCurrentRes->height/2);
}

//=============================================
// Class: CWindow
// Function: GetCurrentMSAASetting
//=============================================
Int32 CWindow::GetCurrentMSAASetting( void )
{
	Int32 msaaSettingValue = gConfig.GetInt(GetConfigGroup(), "MSAASetting");
	for(Uint32 i = 0; i < m_multiSampleSettingsArray.size(); i++)
	{
		if(m_multiSampleSettingsArray[i] == msaaSettingValue)
			return i;
	}

	return 0;
}

//=============================================
// Class: CWindow
// Function: GetNbMSAASettings
//=============================================
Uint32 CWindow::GetNbMSAASettings( void )
{
	return m_multiSampleSettingsArray.size();
}

//=============================================
// Class: CWindow
// Function: GetMSAASetting
//=============================================
Int32 CWindow::GetMSAASetting( Int32 index )
{
	return m_multiSampleSettingsArray[index];
}
