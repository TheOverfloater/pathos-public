/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cl_main.h"
#include "cl_resource.h"
#include "system.h"
#include "networking.h"
#include "texturemanager.h"
#include "cl_snd.h"
#include "file.h"
#include "cl_msg.h"
#include "modelcache.h"
#include "vid.h"
#include "commands.h"

#include "uielements.h"
#include "uimanager.h"
#include "uidownloadwindow.h"

//=============================================
//
//=============================================
cl_resource_t* CL_GetNextMissingResource( void )
{
	cls.netinfo.resourcestlist.begin();
	while(!cls.netinfo.resourcestlist.end())
	{
		cl_resource_t& resource = cls.netinfo.resourcestlist.get();
		if(resource.missing)
			return &resource;

		cls.netinfo.resourcestlist.next();
	}

	return nullptr;
}

//=============================================
//
//=============================================
void CL_ClearDownload( void )
{
	cls.netinfo.download.chunkbits.clear();
	
	// Delete the chunks
	cls.netinfo.download.filechunks.begin();
	while(!cls.netinfo.download.filechunks.end())
	{
		byte* pdata = reinterpret_cast<byte*>(cls.netinfo.download.filechunks.get());
		delete[] pdata;

		cls.netinfo.download.filechunks.next();
	}
	
	cls.netinfo.download.filechunks.clear();
	cls.netinfo.download.presource = nullptr;
}

//=============================================
//
//=============================================
void CL_ClearResources( void )
{
	// Clear current download
	if(cls.netinfo.download.presource != nullptr)
		CL_ClearDownload();

	// Clear resources list
	if(!cls.netinfo.resourcestlist.empty())
		cls.netinfo.resourcestlist.clear();

	cls.netinfo.nummissingresources = 0;
	cls.netinfo.numdownloadedresources = 0;

	// Delete the window
	CUIDownloadWindow::DestroyInstance();
}

//=============================================
//
//=============================================
bool CL_CancelDownload( void )
{
	CL_ClearDownload();

	if(cls.netinfo.nummissingresources == cls.netinfo.numdownloadedresources)
		CL_ClearResources();

	if(!cls.hasallresources)
	{
		Con_Printf("Download cancelled.\n");
		CL_Disconnect();
		return false;
	}

	return true;
}

//=============================================
//
//=============================================
const CString CL_GetResourceAbsolutePath( cl_resource_t* presource )
{
	// Build absolute file path first
	CString filepath;
	switch(presource->type)
	{
	case RS_TYPE_MATERIAL_SCRIPT:
	case RS_TYPE_TEXTURE:
		filepath << TEXTURE_BASE_DIRECTORY_PATH << presource->filepath;
		break;
	case RS_TYPE_SOUND:
		filepath << SOUND_FOLDER_BASE_PATH << presource->filepath;
		break;
	case RS_TYPE_MODEL:
	case RS_TYPE_GENERIC:
		filepath << presource->filepath;
		break;
	case RS_TYPE_UNDEFINED:
	default:
		Con_EPrintf("%s - Invalid resource type %d for %s.\n", __FUNCTION__, (Int32)presource->type, presource->filepath.c_str());
		return "";
		break;
	}

	return filepath;
}

//=============================================
//
//=============================================
bool CL_BeginResourceDownload( cl_resource_t* presource )
{
	// Build full path
	CString filepath = CL_GetResourceAbsolutePath(presource);
	if(filepath.empty())
		return false;

	// Tell the server we're downloading this file
	cls.netinfo.pnet->CLS_MessageBegin(cls_resources);
		cls.netinfo.pnet->WriteByte(SV_RESOURCE_DOWNLOAD_BEGIN);
		cls.netinfo.pnet->WriteBuffer(reinterpret_cast<const byte*>(filepath.c_str()), filepath.length()+1);
		cls.netinfo.pnet->WriteUint16(presource->fileid);
	cls.netinfo.pnet->CLS_MessageEnd();

	// Set file as current download
	cls.netinfo.download.presource = presource;

	return true;
}

//=============================================
//
//=============================================
bool CL_BeginFilesDownload( void )
{
	// Notify the client
	if(cls.netinfo.nummissingresources > 1)
		Con_Printf("Beginning download of %d files.\n", (Int32)cls.netinfo.nummissingresources);
	else
		Con_Printf("Beginning download of %d file.\n", (Int32)cls.netinfo.nummissingresources);

	// Get the first file
	cl_resource_t* pfirst = CL_GetNextMissingResource();
	if(!pfirst)
	{
		Con_EPrintf("%s - Download initiated with no resources to load!.\n", __FUNCTION__);
		return false;
	}

	// Spawn the download progress bar
	CUIDownloadWindow* pDownloadWindow = CUIDownloadWindow::GetInstance();
	if(!pDownloadWindow)
	{
		// Create the window
		pDownloadWindow = CUIDownloadWindow::CreateInstance();
		if(!pDownloadWindow)
			Con_EPrintf("Failed to create exit window.\n");
	}
	else
	{
		// Change focus to this window
		gUIManager.SetFocusWindow(pDownloadWindow);
	}

	return CL_BeginResourceDownload(pfirst);
}

//=============================================
//
//=============================================
bool CL_ReadFileInfo( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	// Read filename and file id
	const Char* pstrFilename = reader.ReadString();
	Int32 fileid = reader.ReadUint16();
	Uint32 numchunks = reader.ReadInt32();

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	CString filepath = CL_GetResourceAbsolutePath(cls.netinfo.download.presource);

	// Make sure this is the file we want to download
	if(qstrcmp(pstrFilename, filepath) || fileid != cls.netinfo.download.presource->fileid)
	{
		Con_EPrintf("%s - File '%s' data received does not match with file '%s' on client.\n", __FUNCTION__, pstrFilename, cls.netinfo.download.presource->filepath.c_str());
		return false;
	}

	// Set bit counts for chunks
	cls.netinfo.download.chunkbits.resize(numchunks);

	// Retrive instance of download progress window
	CUIDownloadWindow* pWindow = CUIDownloadWindow::GetInstance();
	if(pWindow)
	{
		pWindow->SetDownloadFileName(pstrFilename);
		pWindow->SetFileProgressBar(0);
	}

	return true;
}

//=============================================
//
//=============================================
bool CL_LoadResources( void )
{
	// Check for errors
	if(cls.netinfo.resourcestlist.empty())
	{
		Con_EPrintf("%s - No resources were retreived from the server.\n", __FUNCTION__);
		return false;
	}

	// Load resources in order
	cls.netinfo.resourcestlist.begin();
	while(!cls.netinfo.resourcestlist.end())
	{
		cl_resource_t& resource = cls.netinfo.resourcestlist.get();
		if(resource.type != RS_TYPE_MODEL && resource.type != RS_TYPE_SOUND)
		{
			// These resources will be loaded by the appropriate classes
			cls.netinfo.resourcestlist.next();
			continue;
		}

		switch(resource.type)
		{
		case RS_TYPE_MODEL:
			{
				const Char* pstrModelName = resource.filepath.c_str();
				if(!gModelCache.LoadModel(pstrModelName))
				{
					Con_EPrintf("%s - Failed to precache %s.\n", __FUNCTION__, pstrModelName);
					return false;
				}

				// Redraw the loading screen
				VID_DrawLoadingScreen();
			}
			break;
		case RS_TYPE_SOUND:
			{
				const Char* pstrSoundName = resource.filepath.c_str();
				if(!gSoundEngine.PrecacheSound(pstrSoundName, resource.svindex, RS_GAME_LEVEL, false))
					Con_EPrintf("%s - Could not precache %s.\n", __FUNCTION__, pstrSoundName);

				// Redraw the loading screen
				VID_DrawLoadingScreen();
			}
			break;
		}

		cls.netinfo.resourcestlist.next();
	}

	// Tell the server that we have all the resources
	cls.netinfo.pnet->CLS_MessageBegin(cls_resources);
		cls.netinfo.pnet->WriteByte(SV_RESOURCES_OK);
	cls.netinfo.pnet->CLS_MessageEnd();

	// Mark that we have all resources
	cls.hasallresources = true;
	CL_ClearResources();

	// Check if we're ready
	if(!CL_CheckGameReady())
		return false;

	return true;
}

//=============================================
//
//=============================================
bool CL_FinishDownloadedFile( void )
{
	// Calculate the final file size
	Uint32 finalFileSize = 0;
	cls.netinfo.download.filechunks.begin();
	while(!cls.netinfo.download.filechunks.end())
	{
		filechunk_t* pchunk = cls.netinfo.download.filechunks.get();
		finalFileSize += pchunk->datasize;

		cls.netinfo.download.filechunks.next();
	}

	// Safety checks
	if(!finalFileSize)
	{
		Con_EPrintf("%s - Invalid filesize %d for '%s'.\n", __FUNCTION__, finalFileSize, cls.netinfo.download.presource->filepath.c_str());
		return false;
	}

	byte* pbuffer = new byte[finalFileSize];
	memset(pbuffer, 0, sizeof(byte)*finalFileSize);

	// Note: File chunks are supposed to be in order
	Uint32 insertOffset = 0;
	cls.netinfo.download.filechunks.begin();
	while(!cls.netinfo.download.filechunks.end())
	{
		filechunk_t* pchunk = cls.netinfo.download.filechunks.get();

		if(insertOffset >= finalFileSize)
		{
			Con_EPrintf("%s - Overindexing on file buffer(%d > %d) for %s.\n", __FUNCTION__, insertOffset, finalFileSize, cls.netinfo.download.presource->filepath.c_str());
			delete[] pbuffer;
			return false;
		}

		// Copy data to the buffer
		byte* pdatadest = pbuffer + insertOffset;
		byte* pdatasrc = (reinterpret_cast<byte*>(pchunk) + pchunk->dataoffset);
		memcpy(pdatadest, pdatasrc, sizeof(byte)*pchunk->datasize);

		// Increment and continue
		insertOffset += pchunk->datasize;
		cls.netinfo.download.filechunks.next();
	}

	// Write the file to the destination
	CString filepath = CL_GetResourceAbsolutePath(cls.netinfo.download.presource);
	if(filepath.empty())
	{
		delete[] pbuffer;
		return false;
	}

	// Fix slashes
	filepath = Common::FixSlashes(filepath.c_str());

	CString dirpath, token;
	const Char* pstr = filepath.c_str();
	while(pstr)
	{
		while(*pstr == '\\' || *pstr == '/')
			pstr++;

		pstr = Common::Parse(pstr, token, "/");
		if(token.empty())
			break;

		if(qstrstr(token.c_str(), "."))
			break;
		
		dirpath << token << PATH_SLASH_CHAR;
		FL_CreateDirectory(dirpath.c_str());

		// Skip any slashes
		while(pstr && *pstr == PATH_SLASH_CHAR)
			pstr++;
	}

	// Write the output
	if(!FL_WriteFile(pbuffer, finalFileSize, filepath.c_str()))
	{
		Con_EPrintf("%s - Failed to write file '%s'.\n", __FUNCTION__, filepath.c_str());
		delete[] pbuffer;
		return false;
	}

	Con_Printf("Downloaded '%s'.\n", filepath.c_str());

	// Mark as not missing
	cls.netinfo.download.presource->missing = false;
	cls.netinfo.numdownloadedresources++;

	// Retrive instance of download progress window
	CUIDownloadWindow* pWindow = CUIDownloadWindow::GetInstance();
	if(pWindow)
	{
		// Set the value of the total progress bar
		Float value = (Float)cls.netinfo.numdownloadedresources/(Float)cls.netinfo.nummissingresources;
		pWindow->SetTotalProgressBar(value);
	}

	delete[] pbuffer;
	return true;
}

//=============================================
//
//=============================================
bool CL_ReadFileChunk( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	if(!cls.netinfo.download.presource)
	{
		// Download may have been cancelled
		return true;
	}

	Uint32 fileid = reader.ReadUint16();
	Uint32 chunkindex = reader.ReadUint32();
	Uint32 chunksize = reader.ReadUint32();

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	// Make sure everything is alright
	if(fileid != (Uint32)cls.netinfo.download.presource->fileid)
	{
		Con_EPrintf("%s - Mismatch in file ID. Received: %d, expected: %d.\n", __FUNCTION__, fileid, cls.netinfo.download.presource->fileid);
		return false;
	}

	// Retreive data from the packet
	const byte* pdata = reader.ReadBuffer(chunksize);

	// Allocate the chunk's data
	Uint32 allocsize = chunksize + sizeof(filechunk_t);
	byte* pchunkdata = new byte[allocsize];
	filechunk_t* pheader = reinterpret_cast<filechunk_t*>(pchunkdata);

	// Set header info
	pheader->chunkindex = chunkindex;
	pheader->dataoffset = sizeof(filechunk_t);
	pheader->fileid = fileid;
	pheader->datasize = chunksize;

	// Copy the raw data
	byte* pcopydest = pchunkdata + sizeof(filechunk_t);
	memcpy(pcopydest, pdata, sizeof(byte)*chunksize);

	if(cls.netinfo.download.filechunks.empty())
	{
		// Don't bother about order if it's the first element
		cls.netinfo.download.filechunks.add(pheader);
	}
	else
	{
		// Insert in the proper order into the list
		cls.netinfo.download.filechunks.begin();
		while(!cls.netinfo.download.filechunks.end())
		{
			filechunk_t* pchunk = cls.netinfo.download.filechunks.get();
			if(pchunk->chunkindex > chunkindex)
			{
				CLinkedList<filechunk_t*>::link_t *plink = cls.netinfo.download.filechunks.get_link();
				cls.netinfo.download.filechunks.insert_before(plink, pheader);
				break;
			}

			cls.netinfo.download.filechunks.next();
		}

		
		if(cls.netinfo.download.filechunks.end())
		{
			// Add at the end if it wasn't inserted
			cls.netinfo.download.filechunks.radd(pheader);
		}
	}

	// Set bit and test
	cls.netinfo.download.chunkbits.set(chunkindex);

	// Set progress bar
	CUIDownloadWindow* pWindow = CUIDownloadWindow::GetInstance();
	if(pWindow)
	{
		// Set file progress bar
		Float count = cls.netinfo.download.chunkbits.count();
		Float size = cls.netinfo.download.chunkbits.size();

		Float fileValue = count/size;
		pWindow->SetFileProgressBar(fileValue);

		// Set total progress bar
		Float totalValue = (Float)cls.netinfo.numdownloadedresources/(Float)cls.netinfo.nummissingresources;
		totalValue += fileValue * 1.0f/(Float)cls.netinfo.nummissingresources;

		pWindow->SetTotalProgressBar(totalValue);
	}

	// Check if we have all of the bits set
	if(cls.netinfo.download.chunkbits.all())
	{
		// Write the file to the destination
		if(!CL_FinishDownloadedFile())
			return false;

		// Clear the download and start the next one
		CL_ClearDownload();

		// Retreive the next download
		cl_resource_t* presource = CL_GetNextMissingResource();
		if(!presource)
		{
			// Load in the resources
			if(!cls.hasallresources)
				CL_LoadResources();
			else
				CL_ClearResources();

			return true;
		}

		// Begin download
		CL_BeginResourceDownload(presource);
	}
	else
	{
		// Tell the server that we have the chunk
		cls.netinfo.pnet->CLS_MessageBegin(cls_resources);
			cls.netinfo.pnet->WriteByte(SV_RESOURCE_CHUNK_RECEIVED);
			cls.netinfo.pnet->WriteUint16(cls.netinfo.download.presource->fileid);
		cls.netinfo.pnet->CLS_MessageEnd();
	}

	return true;
}

//=============================================
//
//=============================================
bool CL_ReadResourceList( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	// Get the type
	Uint16 resourceType = reader.ReadByte();
	switch(resourceType)
	{
	case RS_LIST_MODELS:
		{
			// Read in model by model
			Uint16 nummodels = reader.ReadUint16();
			for(Uint32 i = 0; i < nummodels; i++)
			{
				cl_resource_t newRes;
				newRes.fileid = cls.netinfo.resourcestlist.size() + 1;
				newRes.filepath = reader.ReadString();
				newRes.svindex = -1; // No server index for models
				newRes.type = RS_TYPE_MODEL;
				newRes.missing = FL_FileExists(newRes.filepath.c_str()) ? false : true;

				cls.netinfo.resourcestlist.radd(newRes);

				if(newRes.missing)
					cls.netinfo.nummissingresources++;
			}

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}
		}
		break;
	case RS_LIST_SOUNDS:
		{
			// Read in the number of sounds
			Uint16 numsounds = reader.ReadUint16();
			for(Uint32 i = 0; i < numsounds; i++)
			{
				cl_resource_t newRes;
				newRes.fileid = cls.netinfo.resourcestlist.size() + 1;
				newRes.filepath = reader.ReadString();
				newRes.svindex = reader.ReadInt16();
				newRes.type = RS_TYPE_SOUND;

				CString filepath;
				filepath << SOUND_FOLDER_BASE_PATH << newRes.filepath;
				newRes.missing = FL_FileExists(filepath.c_str()) ? false : true;

				cls.netinfo.resourcestlist.radd(newRes);

				if(newRes.missing)
					cls.netinfo.nummissingresources++;
			}

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}
		}
		break;
	case RS_LIST_TEXTURES:
		{
			Uint16 numTextures = reader.ReadUint16();
			for(Uint32 i = 0; i < numTextures; i++)
			{
				cl_resource_t newRes;
				newRes.fileid = cls.netinfo.resourcestlist.size() + 1;
				newRes.filepath = reader.ReadString();
				newRes.svindex = -1; // No server index for textures
				newRes.type = RS_TYPE_TEXTURE;

				CString filepath;
				filepath << TEXTURE_BASE_DIRECTORY_PATH << newRes.filepath;
				newRes.missing = FL_FileExists(filepath.c_str()) ? false : true;

				cls.netinfo.resourcestlist.radd(newRes);

				if(newRes.missing)
					cls.netinfo.nummissingresources++;
			}

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}
		}
		break;
	case RS_LIST_MATERIAL_SCRIPTS:
		{
			Uint16 numMaterialScripts = reader.ReadUint16();
			for(Uint32 i = 0; i < numMaterialScripts; i++)
			{
				cl_resource_t newRes;
				newRes.fileid = cls.netinfo.resourcestlist.size() + 1;
				newRes.filepath = reader.ReadString();
				newRes.svindex = -1; // No server index for material scripts
				newRes.type = RS_TYPE_MATERIAL_SCRIPT;

				CString filepath;
				filepath << TEXTURE_BASE_DIRECTORY_PATH << newRes.filepath;
				newRes.missing = FL_FileExists(filepath.c_str()) ? false : true;

				cls.netinfo.resourcestlist.radd(newRes);

				if(newRes.missing)
					cls.netinfo.nummissingresources++;
			}
		}
		break;
	case RS_LIST_GENERIC:
		{
			Uint16 numFiles = reader.ReadUint16();
			for(Uint32 i = 0; i < numFiles; i++)
			{
				cl_resource_t newRes;
				newRes.fileid = cls.netinfo.resourcestlist.size() + 1;
				newRes.filepath = reader.ReadString();
				newRes.svindex = -1; // No server index for generic files
				newRes.type = RS_TYPE_GENERIC;
				newRes.missing = FL_FileExists(newRes.filepath.c_str()) ? false : true;

				cls.netinfo.resourcestlist.radd(newRes);

				if(newRes.missing)
					cls.netinfo.nummissingresources++;
			}

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}
		}
		break;
	case RS_LIST_PARTICLE_SCRIPTS:
		{
			Uint16 numFiles = reader.ReadUint16();
			for(Uint32 i = 0; i < numFiles; i++)
			{
				cl_resource_t newRes;
				newRes.fileid = cls.netinfo.resourcestlist.size() + 1;
				newRes.filepath = reader.ReadString();
				newRes.svindex = -1;
				newRes.type = RS_TYPE_PARTICLE_SCRIPT;

				CString filepath;
				filepath << PARTICLE_SCRIPT_PATH << newRes.filepath;
				newRes.missing = FL_FileExists(newRes.filepath.c_str()) ? false : true;

				cls.netinfo.resourcestlist.radd(newRes);

				if(newRes.missing)
					cls.netinfo.nummissingresources++;
			}

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}
		}
		break;
	case RS_LIST_DECALS:
		{
			if(!cls.netinfo.decalcache.empty())
				cls.netinfo.decalcache.clear();

			Uint16 numFiles = reader.ReadUint16();
			cls.netinfo.decalcache.reserve(numFiles);

			for(Uint32 i = 0; i < numFiles; i++)
			{
				decalcache_t newcache;
				newcache.name = reader.ReadString();
				newcache.type = (decalcache_type_t)reader.ReadByte();

				cls.netinfo.decalcache.push_back(newcache);
			}

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return false;
			}
		}
		break;
	case RS_FINISHED:
		{
			if(!cls.netinfo.nummissingresources)
			{
				// Check if we can progress
				if(!CL_LoadResources())
					return false;
			}
			else
			{
				// Begin download requests towards the server
				CL_BeginFilesDownload();
			}
		};
	}

	return true;
}

//=============================================
//
//=============================================
bool CL_ReadResourceMessage( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	// Read type
	Int32 type = reader.ReadByte();
	switch(type)
	{
	case CL_RESOURCE_LIST:
		return CL_ReadResourceList();
		break;
	case CL_RESOURCE_FILEINFO:
		return CL_ReadFileInfo();
		break;
	case CL_RESOURCE_FILECHUNK:
		return CL_ReadFileChunk();
		break;
	case CL_RESOURCE_UNAVAILABLE:
		return CL_CancelDownload();
		break;
	default:
		Con_Printf("%s - Unknown message type %d.\n", __FUNCTION__, type);
		return false;
		break;
	}
}
