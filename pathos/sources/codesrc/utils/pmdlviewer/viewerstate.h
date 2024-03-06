//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           ViewerSettings.h
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

#ifndef VIEWERSTATE_H
#define VIEWERSTATE_H

#include "studio.h"
#include "vbmformat.h"
#include "flex_shared.h"
#include "matrix.h"
#include "file_interface.h"
#include "stepsound.h"

class CFlexManager;
class CControlPanel;
class CGLExtF;

struct en_material_t;

// Default phong exponent value
static const Float PHONG_EXPONENT_VALUE = 1;
// WinExec error code max value
static const Uint32 WINEXEC_ERROR_CODE_MAX = 32;

enum mv_rendermodes_t
{
	RM_WIREFRAME = 0,
	RM_SMOOTHSHADED,
	RM_TEXTURED
};

struct sound_info_t
{
	sound_info_t():
		numchannels(0),
		samplerate(0),
		bitspersample(0),
		length(0),
		dataoffs(0),
		pdata(nullptr)
	{
	}

	CString filepath;

	Int32 numchannels;
	Int32 samplerate;
	Int32 bitspersample;
	Int32 length;
	Int32 dataoffs;

	byte* pdata;
};

struct sound_data_t
{
	sound_data_t():
		pwavdata(nullptr)
		{}

	~sound_data_t()
	{
		if(pwavdata)
			delete[] pwavdata;
	}

	CString name;
	byte* pwavdata;
};

struct viewerstate_t
{
	viewerstate_t():
		rendermode(RM_TEXTURED),
		transparency(0),
		showskybox(false),
		showground(false),
		showhitboxes(false),
		showbones(false),
		showtexture(false),
		showattachments(false),
		showmeshes(false),
		wireframeoverlay(false),
		viewmodel(false),
		flexscripting(false),
		filtertextures(false),
		largefov(false),
		texture(0),
		texturescale(0),
		skin(0),
		mirror(false),
		usestencil(false),
		sequence(0),
		speedscale(0),
		frametime(0),
		time(0),
		curframe(0),
		texturelimit(0),
		pause(false),
		pflexvalues(nullptr),
		flexvalue(0),
		flexindex(0),
		timeposition(0),
		scripttimelength(0),
		scriptplaybackposition(0),
		scriptzoom(0),
		playbacktime(0),
		wavplayback(false),
		scriptplayback(false),
		scriptpaused(false),
		loopscript(false),
		scriptstay(false),
		stopplaying(false),
		grounddist(0),
		groundsize(0),
		pvbmheader(nullptr),
		pstudioheader(nullptr),
		pflexmanager(nullptr),
		pstepsound(nullptr),
		stepleft(false)
	{
		memset(t_translate, 0, sizeof(t_translate));
		memset(submodels, 0, sizeof(submodels));
		memset(controllers, 0, sizeof(controllers));
	}

	// model
	Vector v_rotation;
	Vector v_translation;

	// Texture window
	Float t_translate[2];

	// render
	enum mv_rendermodes_t rendermode;
	Float transparency;

	bool showskybox;
	bool showground;
	bool showhitboxes;
	bool showbones;
	bool showtexture;
	bool showattachments;
	bool showmeshes;
	bool wireframeoverlay;
	bool viewmodel;
	bool flexscripting;
	bool filtertextures;
	bool largefov;

	Int32 texture;
	Float texturescale;
	Int32 skin;
	bool mirror;
	bool usestencil;	// if 3dfx fullscreen set false

	// animation
	Int32 sequence;
	Float speedscale;

	// bodyparts and bonecontrollers
	Int32 submodels[MAXSTUDIOMODELS];
	Float controllers[MAXSTUDIOCONTROLLERS];

	// frametime related
	Double frametime;
	Double time;

	// Current frame
	Float curframe;

	// colors
	Vector backgroundcolor;
	Vector lightcolor;
	Vector groundcolor;

	// misc
	Int32 texturelimit;
	bool pause;

	Float *pflexvalues;

	// flex scripting
	Float flexvalue;
	Int32 flexindex;
	Float timeposition;

	Float scripttimelength;
	Float scriptplaybackposition;
	Float scriptzoom;

	Double playbacktime;
	bool wavplayback;
	bool scriptplayback;
	bool scriptpaused;

	bool loopscript;
	bool scriptstay;
	bool stopplaying;

	// Info for the WAV file
	sound_info_t wavinfo;

	// Ground related
	Float grounddist;
	Float groundsize;

	CString modelfile;
	CString skyboxtexfile;
	CString groundtexfile;
	CString texturescriptpath;
	CString texturedirectory;

	// Currently loaded VBM file
	vbmheader_t* pvbmheader;
	// Studiomodel file loaded
	studiohdr_t* pstudioheader;

	// Flex script
	flexscript_t flexscript;
	// Flex manager
	CFlexManager* pflexmanager;
	// Flex state
	flexstate_t flexstate;

	// Mod directory used
	CString moddir_path;

	// Modelview matrix
	CMatrix modelview;
	// Projection matrix
	CMatrix projection;

	// Loaded WAV file list
	CLinkedList<sound_data_t*> wavfileslist;
	// Stepsound object
	CStepSound* pstepsound;
	// TRUE if left stepsound
	bool stepleft;
};
// Viewer state object
extern viewerstate_t vs;
// GLEXT object
extern CGLExtF g_glExtF;

extern bool Viewer_Init( Int32 argc, Char* argv[] );
extern void Viewer_Shutdown( void );
extern bool Viewer_LoadModel( const Char* pstrFilePath );
extern bool Viewer_LoadStudioModel( const Char* pstrFilePath );
extern bool Viewer_LoadVBMModel( const Char* pstrFilePath );
extern void Viewer_ReleaseModel( void );
extern const file_interface_t& Viewer_GetGeneralFileInterface( void );
extern const file_interface_t& Viewer_GetModDirFileInterface( void );
extern void Viewer_Bind2DTexture( Int32 texture, Uint32 id );
extern void Viewer_BindCubemapTexture( Int32 texture, Uint32 id );
extern Float Viewer_GetFOV( void );
extern en_material_t* Viewer_FindMaterialScriptByIndex( Int32 index );
extern void Viewer_LoadTextures( void );
extern void Viewer_SetFlexValues ( void );
extern void Viewer_SetMouthPlayback( sound_info_t& wavInfo, Double playbacktime );
extern void Viewer_SetProjectionMatrix( Float screenWidth, Float screenHeight, Float fovY );
extern void Viewer_ErrorPopup( const Char *fmt, ... );
extern bool Viewer_Init( void );
extern Double Viewer_FloatTime( void );
extern void Viewer_GetDirectoryPath( const Char* strIn, CString& out );
extern void Viewer_LoadSounds( void );
extern void Viewer_ReleaseSounds( void );
extern const sound_data_t* Viewer_LoadSound( const Char* pstrPath );
#endif // VIEWERSTATE_H