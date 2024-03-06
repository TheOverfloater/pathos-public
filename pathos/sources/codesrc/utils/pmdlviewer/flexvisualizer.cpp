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

#include "GlWindow.h"
#include "FlexVisualizer.h"

FlexVisualizer* g_FlexVisualizer = 0;

FlexVisualizer::FlexVisualizer ()
: mxWindow (0, 0, 0, 0, 0, "Flex Visualizer", mxWindow::Normal)
{
	// create the OpenGL window
	m_GlWindow = new GlWindow (this, 0, 0, 0, 0, "", mxWindow::Normal);
#ifdef WIN32
	SetWindowLong ((HWND) m_GlWindow->getHandle (), GWL_EXSTYLE, WS_EX_CLIENTEDGE);
#endif

	setBounds (1100, 20, 700, 700);
	setVisible (true);
}

FlexVisualizer::~FlexVisualizer ()
{
}
