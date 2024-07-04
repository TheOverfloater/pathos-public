//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           ViewerSettings.cpp
// last modified:  May 29 1999, Mete Ciragan
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

#include <mx/mx.h>

#include "includes.h"
#include "viewerstate.h"
#include "constants.h"

#include "file_interface.h"
#include "vbm_shared.h"

#include "config.h"
#include "optionswindow.h"
#include "mdlviewer.h"

#include "file.h"
#include "controlpanel.h"
#include "glwindow.h"
#include "fileassociation.h"
#include "compilerwindow.h"
#include "controlpanel.h"
#include "folderviewer.h"

#include "r_glextf.h"
#include "r_vbmbasic.h"
#include "r_basicdraw.h"
#include "animevent.h"
#include "constants.h"

// Average samples taken for mouth movements
static const Uint32 AVERAGE_SAMPLES = 1024;
// Default rotation on axis 0
static const Vector DEFAULT_ROTATION = Vector(-90.0f, 0, 0);
// Default translation on axis 3
static const Vector DEFAULT_POSITION = Vector(0, 0, -90.0f);
// Default ground color
static const Vector DEFAULT_BGCOLOR = Vector(0.0, 0.0, 0.0);
// Default ground color
static const Vector DEFAULT_GROUNDCOLOR = Vector(0.85, 0.85, 0.7);
// Default light color
static const Vector DEFAULT_LIGHTCOLOR = Vector(1.0, 1.0, 1.0);
// Texture limit
static const Uint32 TEXTURE_LIMIT = 256;
// Ground size
static const Uint32 GROUND_SIZE = 128;
// Near Z distance
static const Float NEAR_Z_DISTANCE = 0.1f;
// Far Z distance
static const Float FAR_Z_DISTANCE = 16384;
// Size of buffer for message prints
static const Uint32 PRINT_MSG_BUFFER_SIZE = 16384;

// Definition of viewer state object
viewerstate_t vs;

// GLEXT object declaration
CGLExtF g_glExtF;

static file_interface_t FILE_INTERFACE = 
{
	FL_LoadFile,						//pfnLoadFile
	FL_WriteFile,						//pfnWriteFile
	nullptr,							//pfnWriteLogFile
	FL_FreeFile,						//pfnFreeFile
	FL_FileExists,						//pfnFileExists
	FL_CreateDirectory,					//pfnCreateDirectory
	FL_DeleteFile,						//pfnDeleteFile
	FL_GetFileDate,						//pfnGetFileDate
	FL_CompareFileDates,				//pfnCompareFileDates
};

static file_interface_t FILE_INTERFACE_MOD_DIR = 
{
	FL_LoadFile_GameDirectory,			//pfnLoadFile
	FL_WriteFile_GameDirectory,			//pfnWriteFile
	nullptr,							//pfnWriteLogFile
	FL_FreeFile,						//pfnFreeFile
	FL_FileExists_GameDirectory,		//pfnFileExists
	FL_CreateDirectory_GameDirectory,	//pfnCreateDirectory
	FL_DeleteFile_GameDirectory,		//pfnDeleteFile
	FL_GetFileDate_GameDirectory,		//pfnGetFileDate
	FL_CompareFileDates,				//pfnCompareFileDates
};

//=============================================
// @brief Initializes viewer states
//
//=============================================
bool Viewer_InitStates( void )
{
	// Set basic values
	vs.v_rotation = DEFAULT_ROTATION;
	vs.v_translation = DEFAULT_POSITION;
	vs.rendermode = RM_TEXTURED;
	vs.transparency = 1.0f;

	vs.speedscale = 1.0f;
	vs.texturelimit = TEXTURE_LIMIT;
	vs.texturescale = 1.0f;

	vs.groundsize = GROUND_SIZE;
	vs.scriptzoom = 1.0;

	// Set bg color
	const Char* pstrValue = gConfig.GetOptionValue(CMDLViewer::VIEWER_COLOR_BACKGROUND);
	if(pstrValue)
		Common::StringToVector(pstrValue, vs.backgroundcolor);
	else
		vs.backgroundcolor = DEFAULT_BGCOLOR;

	// Set ground color
	pstrValue = gConfig.GetOptionValue(CMDLViewer::VIEWER_COLOR_GROUND);
	if(pstrValue)
		Common::StringToVector(pstrValue, vs.groundcolor);
	else
		vs.groundcolor = DEFAULT_GROUNDCOLOR;

	// Set light color
	pstrValue = gConfig.GetOptionValue(CMDLViewer::VIEWER_COLOR_LIGHT);
	if(pstrValue)
		Common::StringToVector(pstrValue, vs.lightcolor);
	else
		vs.lightcolor = DEFAULT_LIGHTCOLOR;

	while(true)
	{
		// Set the mod folder path
		const Char* pstrModFolderPath = gConfig.GetOptionValue(COptionsWindow::OW_MOD_FOLDER_PATH);

		CString errorMsg;
		if(pstrModFolderPath)
		{
			// Set the value
			vs.moddir_path = pstrModFolderPath;

			CString shaderPath;
			shaderPath << "scripts/shaders/" << CBasicVBMRenderer::SHADER_FILE_NAME;
			if(!FL_FileExists_GameDirectory(shaderPath.c_str()))
			{
				// Mod folder is missing the shader script
				errorMsg << " Unable to find shader script '" << shaderPath << "', please specify a valid mod folder.";
				mxMessageBox(CMDLViewer::GetInstance(), errorMsg.c_str(), CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
			}
			else
			{
				// We have a valid folder
				break;
			}
		}
		else
		{
			// No mod folder specified at all
			errorMsg << "No mod folder set. Please specify the mod folder.";
			mxMessageBox(CMDLViewer::GetInstance(), errorMsg.c_str(), CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		}

		const Char *pstrFolderPath = mxGetSelectFolder(CMDLViewer::GetInstance());
		if(pstrFolderPath)
		{
			// Set moddir in settings
			gConfig.SetOption(COptionsWindow::OW_MOD_FOLDER_PATH, pstrFolderPath);
			gConfig.SaveOptions();
			vs.moddir_path = pstrFolderPath;
		}
	}

	// Set script pointer
	vs.flexstate.pscript = &vs.flexscript;

	return true;
}

//=============================================
// @brief Initializes viewer states
//
//=============================================
bool Viewer_LoadStudioModel( const Char* pstrFilePath )
{
	Uint32 fileSize = 0;
	const byte* pFile = FL_LoadFile(pstrFilePath, &fileSize);
	if(!pFile)
		return false;

	const studiohdr_t* pstudiohdr = reinterpret_cast<const studiohdr_t*>(pFile);
	if(pstudiohdr->id != IDSTUDIOHEADER)
	{
		Viewer_ErrorPopup("%s - '%s' is not a valid studio model.\n", __FUNCTION__, pstrFilePath);
		return nullptr;
	}

	if(pstudiohdr->version != STUDIO_VERSION)
	{
		Viewer_ErrorPopup("%s - '%s' has wrong version(%d).\n", __FUNCTION__, pstrFilePath, pstudiohdr->version);
		return nullptr;
	}

	// Copy the data
	Uint32 studiodatasize = 0;
	if(pstudiohdr->texturedataindex)
		studiodatasize = pstudiohdr->texturedataindex;
	else
		studiodatasize = fileSize;

	byte* pdata = new byte[studiodatasize];
	memcpy(pdata, pFile, sizeof(byte)*studiodatasize);
	FL_FreeFile(pFile);

	vs.pstudioheader = reinterpret_cast<studiohdr_t*>(pdata);

	// Remove this if present
	if(vs.pstudioheader->flags & STUDIO_MF_DEMOLOCK)
	{
		vs.pstudioheader->bodypartindex -= qstrlen(vs.pstudioheader->name)*32;
		vs.pstudioheader->seqindex -= qstrlen(vs.pstudioheader->name)*16;
		vs.pstudioheader->seqgroupindex -= qstrlen(vs.pstudioheader->name)*8;
		vs.pstudioheader->flags &= ~STUDIO_MF_DEMOLOCK;
	}

	return true;
}

//=============================================
// @brief Initializes viewer states
//
//=============================================
bool Viewer_LoadVBMModel( const Char* pstrFilePath )
{
	Uint32 fileSize = 0;
	const byte* pFile = FL_LoadFile(pstrFilePath, &fileSize);
	if(!pFile)
		return false;

	const vbmheader_t* pvbmheader = reinterpret_cast<const vbmheader_t*>(pFile);
	if(pvbmheader->id != VBM_HEADER)
	{
		Viewer_ErrorPopup("%s - '%s' is not a valid studio model.\n", __FUNCTION__, pstrFilePath);
		return nullptr;
	}

	byte* pdata = new byte[fileSize];
	memcpy(pdata, pFile, sizeof(byte)*fileSize);
	FL_FreeFile(pFile);

	vs.pvbmheader = reinterpret_cast<vbmheader_t*>(pdata);

	return true;
}

//=============================================
// @brief Initializes viewer states
//
//=============================================
void Viewer_LoadTextures( void )
{
	// set up textures
	CString modelname;
	Common::Basename(vs.pvbmheader->name, modelname);
	modelname.tolower();

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	for(Int32 i = 0; i < vs.pvbmheader->numtextures; i++)
	{
		vbmtexture_t *ptexture = vs.pvbmheader->getTexture((Uint32)i);

		CString textureName;
		Common::Basename(ptexture->name, textureName); 
		textureName.tolower();

		// Create and assign the group
		CString materialscriptpath;
		materialscriptpath << MODEL_MATERIALS_BASE_PATH << modelname << PATH_SLASH_CHAR << textureName.c_str() << PMF_FORMAT_EXTENSION;

		// Retreive material name
		en_material_t* pmaterial = pTextureManager->LoadMaterialScript(materialscriptpath.c_str(), RS_GAME_LEVEL);
		if(!pmaterial)
			pmaterial = pTextureManager->GetDummyMaterial();

		if(pmaterial->flags & (TX_FL_ADDITIVE|TX_FL_ALPHABLEND))
			ptexture->flags |= FL_VBM_TEXTURE_BLEND;

		if(!pmaterial->containername.empty())
		{
			Viewer_ErrorPopup("%s - Container name specified for non-world material script '%s'. Texture will be null.\n", __FUNCTION__, materialscriptpath.c_str());
			pmaterial->ptextures[MT_TX_DIFFUSE] = pTextureManager->GetDummyTexture();
		}

		ptexture->index = pmaterial->index;
	}

	VBM_PostLoadVBMCheck(vs.pvbmheader, Viewer_FindMaterialScriptByIndex);
}

//=============================================
// @brief Initializes viewer states
//
//=============================================
bool Viewer_LoadModel( const Char* pstrFilePath )
{
	// Release previous model
	Viewer_ReleaseModel();

	CString filePath(pstrFilePath);
	Uint32 begin = filePath.find(0, ".");
	if(begin != -1)
	{
		Uint32 numErase = filePath.length() - begin;
		filePath.erase(begin, numErase);
	}

	// Load studiomdl file
	CString mdlFilePath(filePath);
	mdlFilePath << ".mdl";

	if(!Viewer_LoadStudioModel(mdlFilePath.c_str()))
	{
		Viewer_ReleaseModel();
		return false;
	}

	// Load studiomdl file
	CString vbmFilePath(filePath);
	vbmFilePath << ".vbm";

	if(!Viewer_LoadVBMModel(vbmFilePath.c_str()))
	{
		Viewer_ReleaseModel();
		return false;
	}

	// Load textures for the model
	Viewer_LoadTextures();
	// Make sure meshes are updated accordingingly with textures
	VBM_PostLoadVBMCheck(vs.pvbmheader, Viewer_FindMaterialScriptByIndex);

	// Set pointers in vbm renderer
	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
	if(pVBMRenderer)
	{
		pVBMRenderer->SetStudioHeader(vs.pstudioheader);
		pVBMRenderer->SetVBMHeader(vs.pvbmheader);

		pVBMRenderer->InitVBM();
		pVBMRenderer->SetSequence(0);
		for(Uint32 i = 0; i < MAXSTUDIOCONTROLLERS; i++)
			pVBMRenderer->SetController(i, 0);
		pVBMRenderer->SetMouthOpen(0);
		for(Uint32 i = 0; i < vs.pvbmheader->numbodyparts; i++)
			pVBMRenderer->SetBodyGroup(i, 0);
		pVBMRenderer->SetSkin(0);
	}

	// Allocate flex values array
	vs.pflexvalues = new Float[NB_FIXED_FLEXES + vs.pvbmheader->numflexcontrollers];
	memset(vs.pflexvalues, 0, sizeof(Float)*(NB_FIXED_FLEXES + vs.pvbmheader->numflexcontrollers));

	// Set flex index mappings
	vs.pflexmanager->SetFlexMappings(vs.pvbmheader, &vs.flexstate);

	// Set up viewer states for model
	CControlPanel::GetInstance()->SetupViewerForModel(pstrFilePath);

	// Load any sounds used by model
	Viewer_LoadSounds();

	return true;
}

//=============================================
// @brief Initializes viewer states
//
//=============================================
void Viewer_ReleaseModel( void )
{
	// Clear studio header
	if(vs.pstudioheader)
	{
		delete[] reinterpret_cast<byte*>(vs.pstudioheader);
		vs.pstudioheader = nullptr;
	}

	// Clear vbm header
	if(vs.pvbmheader)
	{
		delete[] reinterpret_cast<byte*>(vs.pvbmheader);
		vs.pvbmheader = nullptr;
	}

	// Clear textures
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	if(pTextureManager)
	{
		pTextureManager->DeleteTextures(RS_GAME_LEVEL, false);
		pTextureManager->DeleteMaterials(RS_GAME_LEVEL);
	}

	// Clear any flexes also
	if(vs.pflexmanager)
		vs.pflexmanager->Clear();

	// Clear any model states
	vs.flexscript = flexscript_t();
	vs.flexstate = flexstate_t();
	vs.flexstate.pscript = &vs.flexscript;
	vs.timeposition = 0;
	vs.modelfile.clear();
	vs.playbacktime = 0;
	vs.scriptplayback = false;
	vs.scriptzoom = 0;
	vs.stopplaying = false;
	vs.wavplayback = false;

	if(vs.pflexvalues)
	{
		delete[] vs.pflexvalues;
		vs.pflexvalues = nullptr;
	}

	// Clear flex names
	CControlPanel* pControlPanel = CControlPanel::GetInstance();
	if(pControlPanel)
		pControlPanel->ClearFlexNames();

	// Destroy VBO object
	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
	if(pVBMRenderer)
		pVBMRenderer->DestroyVBM();

	// Delete any sounds loaded
	Viewer_ReleaseSounds();
}

//=============================================
// @brief Initializes viewer states
//
//=============================================
void Viewer_LoadSounds( void )
{
	if(!vs.pstudioheader)
		return;
	
	// TRUE if we need footstep sounds
	bool hasFootsteps = false;

	// Look through all animation events
	for(Uint32 i = 0; i < vs.pstudioheader->numseq; i++)
	{
		const mstudioseqdesc_t *pseqdesc = vs.pstudioheader->getSequence(i);
		if(!pseqdesc)
			break;

		if(!pseqdesc->numevents)
			continue;

		for(Uint32 j = 0; j < pseqdesc->numevents; j++)
		{
			const mstudioevent_t* pevent = pseqdesc->getEvent(vs.pstudioheader, j);
			if(!pevent)
				break;

			switch(pevent->event)
			{
			case EVENT_SCRIPT_SOUND:
			case EVENT_CLIENTDLL_PLAYSOUND_A1:
			case EVENT_CLIENTDLL_PLAYSOUND_A1_PLAYER:
				{
					CString filepath;
					filepath << SOUND_FOLDER_BASE_PATH << pevent->options;
					Viewer_LoadSound(filepath.c_str());
				}
				break;
			case EVENT_CLIENTDLL_NPC_STEP_SOUND:
				{
					// Mark as needed
					hasFootsteps = true;
				}
				break;
			}
		}
	}

	// Load footsteps if needed
	if(hasFootsteps)
	{
		Uint32 filesize = 0;
		const byte* pfile = FL_LoadFile_GameDirectory(FOOTSTEP_SCRIPT_FILE, &filesize);
		if(pfile)
		{
			// Allocate object
			vs.pstepsound = new CStepSound();

			// Initialize
			if(!vs.pstepsound->Init(reinterpret_cast<const Char*>(pfile), filesize))
			{
				Viewer_ErrorPopup("Error loading '%s' - %s.\n", FOOTSTEP_SCRIPT_FILE, vs.pstepsound->GetInfoString().c_str());

				delete vs.pstepsound;
				vs.pstepsound = nullptr;
			}
			else
			{
				const CString& infoString = vs.pstepsound->GetInfoString();
				if(!infoString.empty())
					Viewer_ErrorPopup("Step sound: %s.\n", vs.pstepsound->GetInfoString().c_str());

				CArray<CString> soundsList;
				vs.pstepsound->GetPrecacheList(soundsList);

				for(Uint32 i = 0; i < soundsList.size(); i++)
				{
					CString filepath;
					filepath << SOUND_FOLDER_BASE_PATH << soundsList[i];
					Viewer_LoadSound(filepath.c_str());
				}

				CControlPanel* pControlPanel = CControlPanel::GetInstance();
				if(pControlPanel)
					pControlPanel->InitMaterialsList(vs.pstepsound);
			}
		}

		// Reset this
		vs.stepleft = false;
	}
}

//=============================================
// @brief Initializes viewer states
//
//=============================================
void Viewer_ReleaseSounds( void )
{
	if(!vs.wavfileslist.empty())
	{
		vs.wavfileslist.begin();
		while(!vs.wavfileslist.end())
		{
			delete vs.wavfileslist.get();
			vs.wavfileslist.next();
		}

		vs.wavfileslist.clear();
	}

	if(vs.pstepsound)
	{
		delete vs.pstepsound;
		vs.pstepsound = nullptr;

		CControlPanel* pControlPanel = CControlPanel::GetInstance();
		if(pControlPanel)
			pControlPanel->ClearMaterialsList();
	}
}

//=============================================
// @brief Initializes viewer states
//
//=============================================
const sound_data_t* Viewer_LoadSound( const Char* pstrPath )
{
	vs.wavfileslist.begin();
	while(!vs.wavfileslist.end())
	{
		CLinkedList<sound_data_t*>::link_t* plink = vs.wavfileslist.get_link();
		if(!qstrcmp(plink->_val->name, pstrPath))
			return plink->_val;

		vs.wavfileslist.next();
	}

	Uint32 size = 0;
	const byte* pfile = ::FL_LoadFile_GameDirectory(pstrPath, &size);
	if(pfile)
	{
		sound_data_t* pnew = new sound_data_t;
		pnew->name = pstrPath;
		pnew->pwavdata = new byte[size];
		memcpy(pnew->pwavdata, pfile, sizeof(byte)*size);
		vs.wavfileslist.add(pnew);
		return pnew;
	}

	return nullptr;
}

//=============================================
// @brief Returns general file interface
//
//=============================================
const file_interface_t& Viewer_GetGeneralFileInterface( void )
{
	return FILE_INTERFACE;
}

//=============================================
// @brief Returns file interface using mod directory
//
//=============================================
const file_interface_t& Viewer_GetModDirFileInterface( void )
{
	return FILE_INTERFACE_MOD_DIR;
}

//=============================================
// @brief Binds a 2D texture
//
//=============================================
void Viewer_Bind2DTexture( Int32 texture, Uint32 id )
{
	g_glExtF.glActiveTexture( texture );
	glBindTexture( GL_TEXTURE_2D, id );
}

//=============================================
// @brief Binds a 2D texture
//
//=============================================
void Viewer_BindCubemapTexture( Int32 texture, Uint32 id )
{
	g_glExtF.glActiveTexture( texture );
	glBindTexture( GL_TEXTURE_CUBE_MAP, id );
}

//=============================================
// @brief Returns the FOV value to use
//
//=============================================
Float Viewer_GetFOV( void )
{
	return (vs.largefov) ? REFERENCE_FOV_VALUE : DEFAULT_FOV_VALUE;
}

//=============================================
// @brief Finds the material by it's index - ugly hack
//
//=============================================
en_material_t* Viewer_FindMaterialScriptByIndex( Int32 index )
{
	return CTextureManager::GetInstance()->FindMaterialScriptByIndex(index);
}

//=============================================
// @brief Sets flex values
//
//=============================================
void Viewer_SetFlexValues ( void )
{
	if(!vs.pvbmheader || !vs.pflexvalues)
		return;

	if(!vs.flexscripting)
	{
		for (Int32 i = 0; i < vs.pvbmheader->numflexcontrollers; i++)
		{
			Int32 flexcontrollerindex = vs.flexstate.indexmap[i];
			if (flexcontrollerindex == -1)
				continue;

			assert(flexcontrollerindex >= 0 && flexcontrollerindex < 64);
			vs.flexstate.values[flexcontrollerindex] = Common::SplineFraction(vs.pflexvalues[flexcontrollerindex], 1.0 / 1.0);
		}
	}
	else if(vs.scriptplayback)
	{
		CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
		if(pVBMRenderer)
			vs.flexstate.values[FLEX_MOUTH_OPEN] += pVBMRenderer->GetMouthOpen()/30.0f;
	}
	else
	{
		CControlPanel* pPanel = CControlPanel::GetInstance();
		if(pPanel)
		{
			const Char* pname = pPanel->GetFlexLabel(vs.flexindex);
			if(pname)
			{
				Int32 index = vs.pflexmanager->GetControllerIndex(&vs.flexscript, pname);
				if(index != -1)
					vs.flexstate.values[index] = vs.flexvalue;
			}
		}
	}
}

//=============================================
// @brief Sets mouth value
//
//=============================================
void Viewer_SetMouthPlayback( sound_info_t& wavInfo, Double playbacktime )
{
	if(!wavInfo.length)
		return;

	if(wavInfo.numchannels > 1)
		return;

	Float timeDelta = Viewer_FloatTime() - playbacktime;
	Int32 bytepersec = wavInfo.numchannels * (wavInfo.samplerate) * (wavInfo.bitspersample>>3);
	Int32 datapos = timeDelta*bytepersec;

	if(datapos >= wavInfo.length)
	{
		vs.wavplayback = false;
		return;
	}

	Int32 i = 0;
	Int32 mouthavg = 0;
	for(; i < AVERAGE_SAMPLES; i++)
	{
		if((datapos+i) > vs.wavinfo.length)
			break;

		byte *pdata = vs.wavinfo.pdata+datapos+i;
		mouthavg += SDL_abs((*pdata)-127);
	}

	mouthavg = (mouthavg-10)/(Float)i;
	mouthavg = clamp(mouthavg, 0, 255);

	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
	if(pVBMRenderer)
	{
		Float lastval = pVBMRenderer->GetMouthOpen();
		Float value = ((lastval+(Float)mouthavg)/2.0f);

		pVBMRenderer->SetMouthOpen(value);
		pVBMRenderer->SetMouthController(value);
	}
}

//=============================================
// @brief // TODO: Move to r_common
//
//=============================================
Float Viewer_GetXFOVFromY( Float fovY, Float ratio )
{
	Float halfradians = fovY * (0.5f * M_PI / 180.0f);
	Float t = SDL_tan(halfradians) * ratio;

	Float fovX = ((180.0f / M_PI) * SDL_atan(t)) * 2.0f;
	return fovX;
}

//====================================
//
//====================================
void Viewer_SetProjectionMatrix( Float screenWidth, Float screenHeight, Float fovY )
{
	Float ratio = screenWidth/screenHeight;
	Float fovX = Viewer_GetXFOVFromY(fovY, ratio * 0.75f);

	Float width = 2*NEAR_Z_DISTANCE*SDL_tan(fovX*M_PI/360.0f);
	Float height = width/ratio;

	static Float matrix[16];
	matrix[0] = 2*NEAR_Z_DISTANCE/width;
	matrix[1] = matrix[2] = matrix[3] = matrix[4] = 0.0;
	matrix[5] = 2*NEAR_Z_DISTANCE/height;
	matrix[6] = matrix[7] = matrix[8] = matrix[9] = 0.0;
	matrix[10] = FAR_Z_DISTANCE/(NEAR_Z_DISTANCE-FAR_Z_DISTANCE);
	matrix[11] = -1;
	matrix[12] = matrix[13] = 0.0;
	matrix[14] = NEAR_Z_DISTANCE*FAR_Z_DISTANCE/(NEAR_Z_DISTANCE-FAR_Z_DISTANCE);
	matrix[15] = 0.0;

	vs.projection.SetMatrix(matrix);
}

//=============================================
// @brief Prints a formatted string as a message box
//
// @param fmt String describing the format
// @param ... Additional format input parameters
//=============================================
void Viewer_ErrorPopup( const Char *fmt, ... )
{
	va_list	vArgPtr;
	Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	mxMessageBox(nullptr, cMsg, CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
}

//=============================================
// @brief Plays back an animation event
//
// @param pevent Pointer to event structure
// @param pAttachmentsVector Array of attachment positions
//=============================================
void Viewer_VBMEvent( const mstudioevent_t* pevent, const CArray<Vector>* pAttachmentsVector )
{
	switch(pevent->event)
	{
	case EVENT_SCRIPT_SOUND:
	case EVENT_CLIENTDLL_PLAYSOUND_A1:
	case EVENT_CLIENTDLL_PLAYSOUND_A1_PLAYER:
		{
			CString filepath;
			filepath << SOUND_FOLDER_BASE_PATH << pevent->options;

			const sound_data_t* pwavdata = Viewer_LoadSound(filepath.c_str());
			if(pwavdata)
				PlaySound((LPCSTR)pwavdata->pwavdata, NULL, (SND_FILENAME | SND_ASYNC | SND_MEMORY));
		}
		break;
	case EVENT_CLIENTDLL_NPC_STEP_SOUND:
		{
			CStepSound::foot_t foot = vs.stepleft ? CStepSound::FOOT_LEFT : CStepSound::FOOT_RIGHT;
			vs.stepleft = !vs.stepleft;

			// Retrieve material name
			CString materialName;
			CControlPanel* pControlPanel = CControlPanel::GetInstance();
			if(pControlPanel)
				materialName = pControlPanel->GetCurrentMaterialName();

			if(materialName.empty())
				materialName = "concrete";

			if(!Common::IsNumber(pevent->options))
				return;

			Int32 optionvalue = SDL_atoi(pevent->options);
			if(optionvalue < 0 || optionvalue >= NB_NPC_LEGACY_STEPTYPES)
				return;

			CString steptype = NPC_LEGACY_STEPTYPE_NAMES[optionvalue];

			const CArray<CString>* pSoundsArray = vs.pstepsound->GetFootSoundList(foot, materialName.c_str(), steptype.c_str());
			if(!pSoundsArray || pSoundsArray->empty())
				return;

			Uint32 irand = Common::RandomLong(0, pSoundsArray->size()-1);
			const CString& sound = (*pSoundsArray)[irand];

			CString filepath;
			filepath << SOUND_FOLDER_BASE_PATH << sound;

			const sound_data_t* pwavdata = Viewer_LoadSound(filepath.c_str());
			if(pwavdata)
				PlaySound((LPCSTR)pwavdata->pwavdata, NULL, (SND_FILENAME | SND_ASYNC | SND_MEMORY));
		}
		break;
	default:
		{
		}
		break;
	}
}

//=============================================
// @brief Initializes the viewer
//
// @return Success result
//=============================================
bool Viewer_Init( Int32 argc, Char* argv[] )
{
	// Start SDL
	if(SDL_Init( SDL_INIT_EVERYTHING ))
	{
		Viewer_ErrorPopup("SDL_Init returned an error: %s", SDL_GetError());
		return false;
	}

	if(!mx::init (argc, argv))
		return false;

	// Create model viewer instance
	CMDLViewer* pViewer = CMDLViewer::CreateInstance();
	if(!pViewer)
		return false;

	if(!Viewer_InitStates())
		return false;

	// Create GL Window instance
	CGLWindow* pGLWindow = CGLWindow::CreateInstance(pViewer);
	pViewer->InitGLWindow(pGLWindow);

	if(!g_glExtF.GetFunctions())
	{
		mxMessageBox(nullptr, "Failed to load EXT GL functions.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return false;
	}

	// Create control panel instance
	CControlPanel* pControlPanel = CControlPanel::CreateInstance(pViewer);
	if(!pControlPanel)
		return false;

	// Create file association instance
	CFileAssociation* pFileAssociation = CFileAssociation::CreateInstance();
	if(!pFileAssociation)
		return false;

	// Create options window instance
	COptionsWindow* pOptionsWindow = COptionsWindow::CreateInstance();
	if(!pOptionsWindow)
		return false;

	// Create compiler window instance
	CCompilerWindow *pCompilerWindow = CCompilerWindow::CreateInstance();
	if(!pCompilerWindow)
		return false;

	// Create instance of texture manager
	CTextureManager* pTextureManager = CTextureManager::CreateInstance(FILE_INTERFACE_MOD_DIR, Viewer_ErrorPopup, Viewer_ErrorPopup, g_glExtF, false);
	if(!pTextureManager)
		return false;

	// Create folder viewer instance
	CFolderViewer *pFolderViewer = CFolderViewer::CreateInstance(pViewer);
	if(!pFolderViewer)
		return false;

	pTextureManager->Init();

	// Create basic draw instance
	CBasicDraw* pBasicDraw = CBasicDraw::CreateInstance();
	if(!pBasicDraw)
		return false;

	if(!pBasicDraw->InitGL(g_glExtF, FILE_INTERFACE_MOD_DIR, Viewer_ErrorPopup))
	{
		Viewer_ErrorPopup(pBasicDraw->GetShaderError());
		return false;
	}

	// Create model renderer instance
	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::CreateInstance(g_glExtF, FILE_INTERFACE_MOD_DIR, Viewer_ErrorPopup);
	if(!pVBMRenderer)
		return false;

	if(!pVBMRenderer->Init())
	{
		Viewer_ErrorPopup(pVBMRenderer->GetShaderError());
		return false;
	}

	// Set the function pointer
	pVBMRenderer->SetSetFlexValuesFunctionPointer(Viewer_SetFlexValues);
	pVBMRenderer->SetVBMEventFunctionPointer(Viewer_VBMEvent);

	// Allocate flex manager
	vs.pflexmanager = new CFlexManager(FILE_INTERFACE);
	if(!vs.pflexmanager)
		return false;

	pVBMRenderer->SetFlexManager(vs.pflexmanager);
	pVBMRenderer->SetFlexStatePointer(&vs.flexstate);
	pVBMRenderer->SetSetFlexValuesFunctionPointer(Viewer_SetFlexValues);

	pViewer->InitViewer();
	return true;
}

//=============================================
// @brief Initializes viewer states
//
//=============================================
void Viewer_Shutdown( void )
{
	if(vs.pflexmanager)
	{
		delete vs.pflexmanager;
		vs.pflexmanager = nullptr;
	}

	CTextureManager::DeleteInstance();

	if(vs.pflexvalues)
	{
		delete[] vs.pflexvalues;
		vs.pflexvalues = nullptr;
	}

	CBasicVBMRenderer* pVBMRenderer = CBasicVBMRenderer::GetInstance();
	if(pVBMRenderer)
		pVBMRenderer->Shutdown();

	CFolderViewer::DeleteInstance();
	CMDLViewer::DeleteInstance();
	CControlPanel::DeleteInstance();
	CFileAssociation::DeleteInstance();
	COptionsWindow::DeleteInstance();
	CCompilerWindow::DeleteInstance();
	CBasicVBMRenderer::DeleteInstance();
	CBasicDraw::DeleteInstance();

	Viewer_ReleaseModel();
	SDL_Quit();
}

//=============================================
// @brief Returns the time elapsed since the function was last called
// 
// @return Time in seconds
//=============================================
Double Viewer_FloatTime( void )
{
	static Uint32 lastTime = 0;
	Uint32 curTime = SDL_GetTicks();

	if(!lastTime)
	{
		lastTime = curTime;
		return 0;
	}

	return (curTime - lastTime)*MILLISECONDS_TO_SECONDS;
}

//=============================================
// @brief Writes data to a specified file
//
// @param pstr Pointer to string
//=============================================
void Viewer_GetDirectoryPath( const Char* strIn, CString& out )
{
	Int32 i = qstrlen(strIn)-1;
	while(i > 0 && strIn[i] != '\\' && strIn[i] != '/')
		i--;

	out.assign(strIn, i);
}
