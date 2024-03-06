//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           mdlviewer.h
// last modified:  Apr 28 1999, Mete Ciragan
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
#ifndef INCLUDED_FLEXVISUALIZER
#define INCLUDED_FLEXVISUALIZER

#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif

class GlWindow;

class FlexVisualizer : public mxWindow
{
public:
	// CREATORS
	FlexVisualizer ();
	~FlexVisualizer ();

private:
	GlWindow	*m_GlWindow;
};
extern FlexVisualizer* g_FlexVisualizer;
#endif // INCLUDED_FLEXVISUALIZER