/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "system.h"
#include "window.h"
#include "vid.h"
#include "enginestate.h"
#include "texturemanager.h"
#include "r_glextf.h"
#include "r_main.h"

#include "r_vbo.h"
#include "r_glsl.h"
#include "r_text.h"
#include "r_basicdraw.h"
#include "r_rttcache.h"
#include "r_glsl.h"

#include "uimanager.h"
#include "uielements.h"
#include "input.h"
#include "r_menu.h"
#include "r_menuparticles.h"
#include "commands.h"

#include "edict.h"
#include "sv_main.h"
#include "cl_entity.h"
#include "cl_main.h"
#include "console.h"
#include "cl_snd.h"
#include "file.h"

extern CCVar* g_pCvarTimeGraph;

//=============================================
//=============================================
void Cmd_VID_Restart( void ) 
{ 
	VID_Restart(); 
}

//=============================================
//=============================================
void Cmd_VID_SetFullscreen( void ) 
{ 
	ens.requestWMode = WM_FULLSCREEN; 
}

//=============================================
//=============================================
void Cmd_VID_SetWindowed( void ) 
{ 
	ens.requestWMode = WM_WINDOWED; 
}

//=============================================
//=============================================
void Cmd_VID_SetDisplayDevice( void )
{
	if(gCommands.Cmd_Argc() == 2) 
	{
		const Char *pstrDisplayIdx = gCommands.Cmd_Argv(1);
		if(!Common::IsNumber(pstrDisplayIdx))
		{
			Con_EPrintf("%s: Invalid value '%s' set.\n", gCommands.Cmd_Argv(0), pstrDisplayIdx);
			return;
		}

		ens.requestedDisplayDevice = SDL_atoi(pstrDisplayIdx);
	}
	else
		Con_EPrintf("%s: invalid number of parameters.\n", gCommands.Cmd_Argv(0));
}

//=============================================
//=============================================
void Cmd_VID_SetWidth( void ) 
{ 
	if(gCommands.Cmd_Argc() == 2) 
	{
		const Char *pstrWidth = gCommands.Cmd_Argv(1);
		if(!Common::IsNumber(pstrWidth))
		{
			Con_EPrintf("%s: Invalid value '%s' set.\n", gCommands.Cmd_Argv(0), pstrWidth);
			return;
		}

		ens.requestedScrWidth = SDL_atoi(pstrWidth);
	}
	else
		Con_EPrintf("%s: invalid number of parameters.\n", gCommands.Cmd_Argv(0));
}

//=============================================
//=============================================
void Cmd_VID_SetHeight( void ) 
{ 
	if(gCommands.Cmd_Argc() == 2) 
	{
		const Char *pstrHeight = gCommands.Cmd_Argv(1);
		if(!Common::IsNumber(pstrHeight))
		{
			Con_EPrintf("%s: Invalid value '%s' set.\n", gCommands.Cmd_Argv(0), pstrHeight);
			return;
		}

		ens.requestedScrHeight = SDL_atoi(pstrHeight);
	}
	else
		Con_EPrintf("%s: invalid number of parameters.\n", gCommands.Cmd_Argv(0));
}

//=============================================
//=============================================
void Cmd_VID_SetMSAA( void ) 
{ 
	if(gCommands.Cmd_Argc() == 2) 
	{
		const Char *pstrMSAASetting = gCommands.Cmd_Argv(1);
		if(!Common::IsNumber(pstrMSAASetting))
		{
			Con_EPrintf("%s: Invalid value '%s' set.\n", gCommands.Cmd_Argv(0), pstrMSAASetting);
			return;
		}

		ens.requestedMSAASetting = SDL_atoi(pstrMSAASetting);
	}
	else
		Con_EPrintf("%s: invalid number of parameters.\n", gCommands.Cmd_Argv(0));
}

//=============================================
//=============================================
void Cmd_VID_SetVSync( void ) 
{ 
	if(gCommands.Cmd_Argc() == 2) 
	{
		const Char *pstrSetting = gCommands.Cmd_Argv(1);
		if(!Common::IsNumber(pstrSetting))
		{
			Con_EPrintf("%s: Invalid value '%s' set.\n", gCommands.Cmd_Argv(0), pstrSetting);
			return;
		}

		ens.requestedVSyncSetting = SDL_atoi(pstrSetting);
	}
	else
		Con_EPrintf("%s: invalid number of parameters.\n", gCommands.Cmd_Argv(0));
}

//=============================================
//=============================================
void Cmd_VID_SetFBOEnabled(void)
{
	if (gCommands.Cmd_Argc() == 2)
	{
		const Char* pstrSetting = gCommands.Cmd_Argv(1);
		if (!Common::IsNumber(pstrSetting))
		{
			Con_EPrintf("%s: Invalid value '%s' set.\n", gCommands.Cmd_Argv(0), pstrSetting);
			return;
		}

		ens.requestedFBOSetting = SDL_atoi(pstrSetting);
	}
	else
		Con_EPrintf("%s: invalid number of parameters.\n", gCommands.Cmd_Argv(0));
}

//=============================================
//=============================================
void Cmd_VID_SetHDREnabled(void)
{
	if (gCommands.Cmd_Argc() == 2)
	{
		const Char* pstrSetting = gCommands.Cmd_Argv(1);
		if (!Common::IsNumber(pstrSetting))
		{
			Con_EPrintf("%s: Invalid value '%s' set.\n", gCommands.Cmd_Argv(0), pstrSetting);
			return;
		}

		ens.requestedHDRSetting = SDL_atoi(pstrSetting);
	}
	else
		Con_EPrintf("%s: invalid number of parameters.\n", gCommands.Cmd_Argv(0));
}

//=============================================
//=============================================
void VID_InitCommands( void )
{
	gCommands.CreateCommand("_vid_restart", &Cmd_VID_Restart, "Restarts the window, reloads all GL resources", CMD_FL_HIDDEN);
	gCommands.CreateCommand("_vid_setwidth", &Cmd_VID_SetWidth, "Sets the desired screen width. Call '_vid_restart' to apply", CMD_FL_HIDDEN);
	gCommands.CreateCommand("_vid_setheight", &Cmd_VID_SetHeight, "Sets the desired screen height. Call '_vid_restart' to apply", CMD_FL_HIDDEN);
	gCommands.CreateCommand("_vid_setfullscreen", &Cmd_VID_SetFullscreen, "Sets the desired window mode to fullscreen. Call '_vid_restart' to apply", CMD_FL_HIDDEN);
	gCommands.CreateCommand("_vid_setwindowed", &Cmd_VID_SetWindowed, "Sets the desired window mode to windowed. Call '_vid_restart' to apply", CMD_FL_HIDDEN);
	gCommands.CreateCommand("_vid_setdisplay", &Cmd_VID_SetDisplayDevice, "Sets the desired display device. Call '_vid_restart' to apply", CMD_FL_HIDDEN);
	gCommands.CreateCommand("_vid_setmsaa", &Cmd_VID_SetMSAA, "Sets the desired MSAA setting. Call '_vid_restart' to apply", CMD_FL_HIDDEN);
	gCommands.CreateCommand("_vid_setvsync", &Cmd_VID_SetVSync, "Sets the desired vertical sync setting. Call '_vid_restart' to apply", CMD_FL_HIDDEN);
	gCommands.CreateCommand("_vid_setfboenabled", &Cmd_VID_SetFBOEnabled, "Sets the desired framebuffer object setting. Call '_vid_restart' to apply", CMD_FL_HIDDEN);
	gCommands.CreateCommand("_vid_sethdrenabled", &Cmd_VID_SetHDREnabled, "Sets the desired high dynamic range setting. Call '_vid_restart' to apply", CMD_FL_HIDDEN);
}

//=============================================
//=============================================
void GLAPIENTRY VID_GLMessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
	if(!ens.pgllogfile)
		return;

	// Determien source
	CString srcName;
	switch (source) 
	{
	case GL_DEBUG_SOURCE_API:
		srcName = "API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		srcName = "WINDOW SYSTEM";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		srcName = "SHADER COMPILER";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		srcName = "THIRD PARTY";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		srcName = "APPLICATION";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		srcName = "UNKNOWN";
		break;
	default:
		srcName = "UNKNOWN";
		break;
	}

	// Determine type
	CString typeName;
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		typeName = "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		typeName = "DEPRECATED BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		typeName = "UDEFINED BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		typeName = "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		typeName = "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		typeName = "OTHER";
		break;
	case GL_DEBUG_TYPE_MARKER:
		typeName = "MARKER";
		break;
	default:
		typeName = "UNKNOWN";
		break;
	}

	// Severity
	CString severityStr;
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		severityStr = "HIGH";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		severityStr = "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		severityStr = "LOW";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		severityStr = "NOTIFICATION";
		break;
	default:
		severityStr = "UNKNOWN";
		break;
	}

	CString msg;
	msg << "[OPENGL " << typeName << "]"
		<< " - Source: " << srcName
		<< ", severity: " << severityStr
		<< " - Message: " << message << NEWLINE;

	ens.pgllogfile->Write(msg.c_str());
}

//=============================================
//=============================================
bool VID_Init( void )
{
	// Clear time counters
	CGLSLShader::ClearTimeCounters();

	// Initialize the GL window
	if(!gWindow.Init())
	{
		Con_EPrintf("Failed to initialize SDL window.\n");
		return false;
	}

	// Load OpenGL functions
	if(!gGLExtF.GetFunctions())
	{
		CString str;
		str << "Failed to load necessary OpenGL functions: " << gGLExtF.GetError();
		Sys_ErrorPopup(str.c_str());
		return false;
	}

	// During init, enable debug output
	if(ens.pgllogfile)
	{
		if(gGLExtF.glDebugMessageCallback)
		{
			glEnable(GL_DEBUG_OUTPUT);
			gGLExtF.glDebugMessageCallback( VID_GLMessageCallback, 0 );
		}
		else
		{
			// Make sure we handle it not being supported
			ens.pgllogfile->Write("OpenGL debug messages are not supported by your drivers.\n");
		}
	}

	// Reset the mouse
	gInput.ResetMouse();

	// Perform GL resource initialization here
	CTextureManager* pTextureManager = CTextureManager::CreateInstance(FL_GetInterface(), Con_Printf, Con_EPrintf, gGLExtF, false);
	if(!pTextureManager)
	{
		R_Shutdown();
		return false;
	}
	
	// Initialize the class
	pTextureManager->Init();

	// Init renderer
	if(!R_InitGL())
	{
		R_Shutdown();
		return false;
	}

	// Set the screen width/height
	rns.screenwidth = gWindow.GetWidth();
	rns.screenheight = gWindow.GetHeight();

	Con_Printf("OpenGL info:\n");
	const Char* pstrGLInfo = reinterpret_cast<const Char*>(glGetString(GL_RENDERER));
	if(pstrGLInfo)
	{
		Con_Printf(" - Renderer: %s.\n", pstrGLInfo);
		if(ens.pgllogfile)
			ens.pgllogfile->Printf(" - Renderer: %s.\n", pstrGLInfo);
	}
	else
	{
		Con_Printf(" - Couldn't get renderer information.\n");
		if(ens.pgllogfile)
			ens.pgllogfile->Printf(" - Couldn't get renderer information.\n");
	}

	pstrGLInfo = reinterpret_cast<const Char*>(glGetString(GL_VERSION));
	if(pstrGLInfo)
	{
		Con_Printf(" - Version: %s.\n", pstrGLInfo);
		if(ens.pgllogfile)
			ens.pgllogfile->Printf(" - Version: %s.\n", pstrGLInfo);
	}
	else
	{
		Con_Printf(" - Couldn't get version information.\n");
		if(ens.pgllogfile)
			ens.pgllogfile->Printf(" - Couldn't get version information.\n");
	}

	pstrGLInfo = reinterpret_cast<const Char*>(glGetString(GL_VENDOR));
	if(pstrGLInfo)
	{
		Con_Printf(" - Vendor: %s.\n", pstrGLInfo);
		if(ens.pgllogfile)
			ens.pgllogfile->Printf(" - Vendor: %s.\n", pstrGLInfo);
	}
	else
	{
		Con_Printf(" - Couldn't get vendor information.\n");
		if(ens.pgllogfile)
			ens.pgllogfile->Printf(" - Couldn't get vendor information.\n");
	}

	pstrGLInfo = reinterpret_cast<const Char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	if(pstrGLInfo)
	{
		Con_Printf(" - Shading language version: %s.\n", pstrGLInfo);
		if(ens.pgllogfile)
			ens.pgllogfile->Printf(" - Shading language version: %s.\n", pstrGLInfo);
	}
	else
	{
		Con_Printf(" - Couldn't get shading language version.\n");
		if(ens.pgllogfile)
			ens.pgllogfile->Printf(" - Couldn't get shading language version.\n");
	}

	Int32 majorVersion, minorVersion;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &majorVersion);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minorVersion);

	Con_Printf(" - OpenGL context version: %d.%d.\n", majorVersion, minorVersion);

	// Print statistics from GLSL shaders
	if(ens.pgllogfile)
	{
		ens.pgllogfile->Printf("Shader statistics:\n");
		ens.pgllogfile->Printf(" - Total vertex shader compile time: %.4f seconds.\n", CGLSLShader::GetTotalVertexShaderCompileTime());
		ens.pgllogfile->Printf(" - Total vertex shader status get call time: %.4f seconds.\n", CGLSLShader::GetTotalVertexShaderGetStatusCallTime());
		ens.pgllogfile->Printf(" - Total fragment shader compile time: %.4f seconds.\n", CGLSLShader::GetTotalFragmentShaderCompileTime());
		ens.pgllogfile->Printf(" - Total fragment shader status get call time: %.4f seconds.\n", CGLSLShader::GetTotalFragmentShaderGetStatusCallTime());
		ens.pgllogfile->Printf(" - Total shader link time: %.4f seconds.\n", CGLSLShader::GetTotalShaderLinkTime());
		ens.pgllogfile->Printf(" - Total shader link get status call time: %.4f seconds.\n", CGLSLShader::GetTotalShaderLinkGetStatusCallTime());
		ens.pgllogfile->Printf(" - Number of shader programs linked: %d.\n", CGLSLShader::GetNbTotalShaderProgramsLinked());
	}

	return true;
}

//=============================================
// @brief Called when the window is respawned after 
// changing mode, resolutions, etc
//
//=============================================
void VID_OnReload( void )
{
	Con_DPrintf("Reloading resources.\n");

	// Resize UI windows
	gUIManager.OnGLInitialization();

	// Reposition any UI windows
	gUIManager.RepositionWindows();

	// Restore cursor
	if(gMenu.IsActive())
		gInput.ShowMouse();
}

//=============================================
// @brief Called to respawn the window for resizing, etc
//
//=============================================
void VID_Restart( void )
{
	Con_DPrintf("Restarting Window.\n");

	if(gMenu.IsActive())
		gMenu.PauseMenuMusic();

	// Free objects, destroy window
	VID_Shutdown();

	// Respawn the window
	if(!VID_Init())
	{
		Sys_ErrorPopup("Failed to initialize OpenGL. Check log for details.");
		Sys_Exit();
	}

	// We're done loading, so finish drawing screen
	VID_EndLoading();

	// Resize/reposition windows
	VID_OnReload();

	if(gMenu.IsActive())
		gMenu.ResumeMenuMusic();

	Con_DPrintf("Window reloaded.\n");
}

//=============================================
// @brief This function is called to clean up OpenGL
// Usually this happens when the resolution is changed, or we're exiting
//=============================================
void VID_Shutdown( void )
{
	// Perform OpenGL cleanup here
	// IMPORTANT: Only free GL resources, don't free resource data!
	// This is because we might only be changing resolutions, or window mode
	// and we want to know what we've loaded, so we can reload it

	// Release renderer classes too
	R_ShutdownGL();

	// Delete all binds for texture, and non-texture file binds
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	pTextureManager->DeleteTextures(RS_GAME_LEVEL);
	pTextureManager->DeleteTextures(RS_WINDOW_LEVEL);

	// Destroy the GL window
	gWindow.DestroyWindow();
}

//=============================================
// @brief 
//
//=============================================
void VID_BeginLoading( bool drawUI )
{
	if(ens.isloading)
	{
		Con_Printf("%s - Called while draw state was already set.\n", __FUNCTION__);
		return;
	}

	// Kill menu sounds
	gMenu.PauseMenuMusic();

	// Hide mouse
	gInput.HideMouse();

	// Pause all sounds
	gSoundEngine.SetPauseAllSounds(true);

	ens.isloading = true;
	rns.drawuiwhileloading = drawUI;

	if(rns.drawuiwhileloading)
	{
		// Set window filter flags to draw only the console
		gUIManager.SetDrawFilter(CUIWindow::UIW_FL_CONSOLEWINDOW|CUIWindow::UIW_FL_DOWNLOADWINDOW);
	}

	// Set loading texture
	if(!CL_IsGameActive() || gMenu.IsActive())
	{
		en_texture_t* ptexture = gMenu.GetLoadingTexture();
		rns.ploadbackground = ptexture->palloc;

		// This is not a rectangle texture
		rns.isbgrectangletexture = (ptexture->flags & TX_FL_RECTANGLE) ? true : false;
	}
	else
	{
		// Draw the scene once more, and grab screen contents
		VID_DrawSceneOnly();

		// Set BG texture
		rns.pbgrtttexture = gRTTCache.Alloc(rns.screenwidth, rns.screenheight, true, GL_RGBA, RS_WINDOW_LEVEL);
		rns.ploadbackground = rns.pbgrtttexture->palloc;

		// Copy screen contents
		R_BindRectangleTexture(GL_TEXTURE0_ARB, rns.ploadbackground->gl_index);
		glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);
		R_BindRectangleTexture(GL_TEXTURE0_ARB, 0);

		// This is a rectangle texture
		rns.isbgrectangletexture = true;
	}

	// Draw the loading screen
	VID_DrawLoadingScreen();
}

//=============================================
// @brief 
//
//=============================================
void VID_EndLoading( void )
{
	if(rns.drawuiwhileloading)
	{
		// Remove filter for console window
		gUIManager.RemoveDrawFilter(CUIWindow::UIW_FL_CONSOLEWINDOW|CUIWindow::UIW_FL_DOWNLOADWINDOW);
	}

	// Reset these
	ens.isloading = false;
	rns.drawuiwhileloading = false;

	if(rns.pbgrtttexture)
	{
		gRTTCache.Delete(rns.pbgrtttexture);
		rns.pbgrtttexture = nullptr;
	}

	rns.ploadbackground = nullptr;

	// If loading failed, go back to menu
	if(cls.cl_state != CLIENT_ACTIVE)
	{
		if(!gMenu.IsActive())
			gMenu.ShowMenu();
		else
			gMenu.ResumeMenuMusic();

		gInput.ShowMouse();
	}

	// Unpause all sounds
	gSoundEngine.SetPauseAllSounds(false);
}

//=============================================
//=============================================
void VID_Draw( void )
{
	if(rns.numskipframes > 0)
	{
		rns.numskipframes--;
		return;
	}

	if(ens.isloading)
	{
		// Draw loading screen only if we're loading
		VID_DrawLoadingScreen();
		return;
	}

	// Set this here, so we can do fast checks later
	// on with the boolean
	rns.validateshaders = (g_pCvarGLSLValidate->GetValue() < 1) ? false : true;

	// For time graph
	Double time1;
	
	if(g_pCvarTimeGraph->GetValue() >= 1.0f)
		time1 = Sys_FloatTime();

	// Bind FBO if we use HDR
	if (rns.fboused && rns.usehdr)
		R_BindMainScreenFBO();

	glViewport(0, 0, rns.screenwidth, rns.screenheight);
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Draw the scene
	if(cls.cl_state == CLIENT_ACTIVE && rns.isgameready)
	{
		if(!R_DrawScene())
		{
			Con_Printf("%s - Fatal error while rendering scene.\n", __FUNCTION__);
			CL_Disconnect();
			ens.exit = true;
			return;
		}
	}

	// Draw HUD objects
	if(cls.cl_state == CLIENT_ACTIVE)
	{
		if(!R_DrawHUD())
		{
			Con_Printf("%s - Fatal error while drawing HUD.\n", __FUNCTION__);
			CL_Disconnect();
			ens.exit = true;
			return;
		}
	}

	// Blit from main FBO to back buffer
	if (rns.fboused && rns.usehdr)
		R_PerformMainScreenBlit();

	// Draw any menu/UI elements
	if(!R_DrawInterface())
	{
		Con_Printf("%s - Fatal error while drawing HUD.\n", __FUNCTION__);
		CL_Disconnect();
		ens.exit = true;
		return;
	}

	// Draw paused logo if needed
	if(cls.paused && ens.gamestate != GAME_LOADING && !gMenu.IsActive())
	{
		if(!R_DrawPausedLogo())
		{
			Con_Printf("%s - Fatal error while drawing paused logo.\n", __FUNCTION__);
			CL_Disconnect();
			ens.exit = true;
			return;
		}
	}

	// Print counters
	if(!R_PrintCounters())
	{
		CBasicDraw* pDraw = CBasicDraw::GetInstance();
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		Con_Printf("%s - Fatal error while drawing counters.\n", __FUNCTION__);
		CL_Disconnect();
		ens.exit = true;
		return;
	}

	// Draw console debug prints
	if(!gConsole.Draw())
	{
		CBasicDraw* pDraw = CBasicDraw::GetInstance();
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		Con_Printf("%s - Fatal error while drawing console history.\n", __FUNCTION__);
		CL_Disconnect();
		ens.exit = true;
		return;
	}

	// For time graph
	if(g_pCvarTimeGraph->GetValue() >= 1)
	{
		Double time2 = Sys_FloatTime();
		if(!R_DrawTimeGraph(time1, time2))
		{
			CBasicDraw* pDraw = CBasicDraw::GetInstance();
			Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
			Con_Printf("%s - Fatal error while drawing console history.\n", __FUNCTION__);
			CL_Disconnect();
			ens.exit = true;
			return;
		}
	}

	// Load any pending shaders
	if(!R_PerformPendingShaderLoads())
	{
		Con_Printf("%s - Fatal error while compiling a pending shader.\n", __FUNCTION__);
		CL_Disconnect();
		ens.exit = true;
		return;
	}

	// Swap the OGL buffer
	gWindow.SwapWindow();

	// Increment frame counter
	rns.framecount++;
}

//=============================================
//=============================================
void VID_DrawLoadingScreen( const Char* pstrText )
{
	if(rns.isdrawingloadingscreen)
		return;

	// Set this here, so we can do fast checks later
	// on with the boolean
	rns.validateshaders = (g_pCvarGLSLValidate->GetValue() < 1) ? false : true;

	// Make sure we don't go into a recursive infinite loop
	rns.isdrawingloadingscreen = true;

	if(!ens.isloading)
	{
		Con_Printf("%s - Called while not in loading state.\n", __FUNCTION__);
		rns.isdrawingloadingscreen = false;
		return;
	}

	// Bind FBO if we use HDR
	if (rns.fboused && rns.usehdr)
		R_BindMainScreenFBO();

	glViewport(0, 0, rns.screenwidth, rns.screenheight);
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Draw only load screen elements
	if(!R_DrawLoadingScreen(pstrText))
	{
		Con_Printf("%s - Fatal error while drawing loading screen.\n", __FUNCTION__);
		ens.exit = true;
	}

	// Draw console debug prints
	if(!gConsole.Draw())
	{
		Con_Printf("%s - Fatal error while drawing console history.\n", __FUNCTION__);
		ens.exit = true;
	}

	// Blit from main FBO to back buffer
	if (rns.fboused && rns.usehdr)
		R_PerformMainScreenBlit();

	// Swap the OGL buffer
	gWindow.SwapWindow();

	// Reset this
	rns.isdrawingloadingscreen = false;
}

//=============================================
//=============================================
void VID_DrawSceneOnly( void )
{
	// Set this here, so we can do fast checks later
	// on with the boolean
	rns.validateshaders = (g_pCvarGLSLValidate->GetValue() < 1) ? false : true;

	// Bind FBO if we use HDR
	if (rns.fboused && rns.usehdr)
		R_BindMainScreenFBO();

	glViewport(0, 0, rns.screenwidth, rns.screenheight);
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Draw an the scene
	if(cls.cl_state == CLIENT_ACTIVE && rns.isgameready)
	{
		if(!R_DrawScene())
		{
			Con_Printf("%s - Fatal error while rendering scene.\n", __FUNCTION__);
			CL_Disconnect();
			ens.exit = true;
			return;
		}

		if(!R_DrawHUD(true))
		{
			Con_Printf("%s - Fatal error while drawing HUD.\n", __FUNCTION__);
			CL_Disconnect();
			ens.exit = true;
			return;
		}
	}

	// Blit from main FBO to back buffer
	if (rns.fboused && rns.usehdr)
		R_PerformMainScreenBlit();

	// Increment frame counter
	rns.framecount++;
}

//=============================================
//=============================================
void VID_GetScreenContents( Uint32& width, Uint32& height, Uint32& bpp, byte** pdata )
{
	width = rns.screenwidth;
	height = rns.screenheight;
	bpp = 4;

	(*pdata) = new byte[width*height*bpp];

	glFinish();

	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (*pdata));
}

//=============================================
// @brief Callback function for shader compile progress
//
//=============================================
void VID_ShaderCompileCallback( const Char* pstrShaderName, Uint32 totalCount, Uint32 completedCount, bool buildingCache )
{
	Float progressPercentage = (static_cast<Float>(completedCount)/ static_cast<Float>(totalCount))*100;

	CString strProgressText;
	if (!buildingCache)
		strProgressText << "Loading shader '" << pstrShaderName << "'";
	else
		strProgressText << "Loading shader '" << pstrShaderName << "' and building cache";

	strProgressText << " - " << static_cast<Int32>(progressPercentage) << "% ";
	strProgressText << "(" << completedCount << "/" << totalCount << ")"; 

	VID_DrawLoadingScreen(strProgressText.c_str());
}