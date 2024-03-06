/*
===============================================
Pathos Engine - Created by Andrew "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "mdlviewer.h"
#include "viewerstate.h"
#include "controlpanel.h"

//===============================================
// _tmain
//
//===============================================
Int32 _tmain(Int32 argc, Char* argv[])
{
	//
	// make sure, we start in the right directory
	//
	CString appPath = mx::getApplicationPath();
	mx_setcwd(appPath.c_str());

	CString cmdline;
	if (argc > 1)
	{
		cmdline << argv[1];

		for(Uint32 i = 2; i < argc; i++)
			cmdline << " " << argv[i];
	}

	if(!Viewer_Init(argc, argv))
	{
		Viewer_ErrorPopup("Error initializing model viewer.");
		return -1;
	}

	if(cmdline.find(0, ".mdl") != -1 || cmdline.find(0, ".vbm") != -1)
		CControlPanel::GetInstance()->LoadModel(cmdline.c_str());

	// Run main loop
	Int32 ret = mx::run();
	Viewer_Shutdown();

	return ret;
}
