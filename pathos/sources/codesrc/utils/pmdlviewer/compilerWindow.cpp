//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           CompilerOptions.cpp
// last modified:  May 04 1999, Mete Ciragan
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

#include "compilerwindow.h"
#include "optionswindow.h"
#include "config.h"
#include "mdlviewer.h"
#include "fileassociation.h"
#include "file.h"
#include "viewerstate.h"

// Option header for last qc path
const Char CCompilerWindow::LAST_QC_PATH_HEADER[] = "LAST_QC_PATH";
// Option header for last copy path
const Char CCompilerWindow::LAST_COPY_PATH_HEADER[] = "LAST_COPY_PATH";
// Compiler window title
const Char CCompilerWindow::WINDOW_TITLE[] = "QC Compile";
// Default program for editing
const Char CCompilerWindow::DEFAULT_EDITOR[] = "notepad";
// Max history of qc files
const Uint32 CCompilerWindow::MAX_COMPILER_HISTORY = 8;
// Compiler window x origin
const Uint32 CCompilerWindow::WINDOW_X_ORIGIN = 100;
// Compiler window y origin
const Uint32 CCompilerWindow::WINDOW_Y_ORIGIN = 100;
// Compiler window width
const Uint32 CCompilerWindow::WINDOW_WIDTH = 460;
// Compiler window height
const Uint32 CCompilerWindow::WINDOW_HEIGHT = 280;

// Current instance of this class
CCompilerWindow* CCompilerWindow::g_pInstance = nullptr;

//=============================================
// @brief Constructor
//
//=============================================
CCompilerWindow::CCompilerWindow( void ): 
	mxWindow(nullptr, WINDOW_X_ORIGIN, WINDOW_Y_ORIGIN, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, mxWindow::Dialog),
	m_pChoiceQcPaths(nullptr),
	m_pButtonQcPathButton(nullptr),
	m_pButtonQcCompileButton(nullptr),
	m_pButtonQcEditButton(nullptr),
	m_pChoiceCopyPaths(nullptr),
	m_pButtonCopyPathButton(nullptr),
	m_pButtonCopyButton(nullptr)
{
	mxGroupBox* compileGroup = new mxGroupBox (this, 5, 5, 440, 100, "Compile");

	new mxLabel (this, 15, 28, 150, 18, "QC path:");
	m_pChoiceQcPaths = new mxChoice (this, 15, 45, 400, 22, IDCW_FILE_PATH);
	m_pButtonQcPathButton = new mxButton (this, 415, 45, 22, 22, "...", IDCW_FILE_BUTTON);
	m_pButtonQcCompileButton = new mxButton (this, 15, 70, 60, 22, "Compile", IDCW_COMPILE_BUTTON);
	m_pButtonQcEditButton = new mxButton (this, 85, 70, 60, 22, "Edit", IDCW_EDIT_BUTTON);

	mxGroupBox* copyGroup = new mxGroupBox (this, 5, 115, 440, 100, "Copy");

	new mxLabel (this, 15, 138, 150, 18, "Target directory:");
	m_pChoiceCopyPaths = new mxChoice (this, 15, 155, 400, 22, IDCW_COPY_PATH);
	m_pButtonCopyPathButton = new mxButton (this, 415, 155, 22, 22, "...", IDCW_COPY_PATH_BUTTON);
	m_pButtonCopyButton = new mxButton (this, 15, 180, 60, 22, "Copy", IDCW_COPY_BUTTON);

	new mxButton (this, 5, 225, 60, 22, "Exit", IDCW_EXIT_BUTTON);
}

//=============================================
// @brief Destructor
//
//=============================================
CCompilerWindow::~CCompilerWindow( void )
{
	// mx deletes these windows before we 
	// get a chance to do it via DeleteInstance()
	g_pInstance = nullptr;
}

//=============================================
// @brief Opens a QC file for editing
//
//=============================================
void CCompilerWindow::EditQC( void )
{
	Int32 qcIndex = m_pChoiceQcPaths->getSelectedIndex();
	if(qcIndex < 0 || qcIndex >= MAX_COMPILER_HISTORY || qcIndex >= m_qcFileHistoryArray.size())
	{
		mxMessageBox(this, "QC index was invalid.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return;
	}

	CFileAssociation *pFileAssoc = CFileAssociation::GetInstance();
	if(!pFileAssoc)
	{
		mxMessageBox(this, "Couldn't get file assoc window.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return;
	}

	// Get the associated program for QCs
	CString programToUse;
	const Char* pstrAssoc = pFileAssoc->GetProgram( "qc" );
	Int32 openMode = pFileAssoc->GetMode( "qc" );
	if(pstrAssoc && openMode == 0)
		programToUse = pstrAssoc;
	else
		programToUse = DEFAULT_EDITOR;

	CString cmd;
	cmd << programToUse << " \"" << m_qcFileHistoryArray[qcIndex] << "\"";

	if(WinExec(cmd.c_str(), SW_SHOW) <= WINEXEC_ERROR_CODE_MAX)
		mxMessageBox(this, "Error executing specified program.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);

	SetFocus((HWND)this->getHandle());
}

//=============================================
// @brief Compiles a QC file
//
//=============================================
void CCompilerWindow::CompileQC( void )
{
	const Char *pstrCompilerPath = gConfig.GetOptionValue(COptionsWindow::OW_COMPILER_PATH);
	if(!pstrCompilerPath)
	{
		mxMessageBox(this, "Compiler not specified.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return;
	}

	// Get the QC we want to compile
	Int32 qcIndex = m_pChoiceQcPaths->getSelectedIndex();
	if(qcIndex < 0 || qcIndex >= MAX_COMPILER_HISTORY || qcIndex >= m_qcFileHistoryArray.size())
	{
		mxMessageBox(this, "QC index was invalid.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return;
	}

	// Retrieve the QC path
	const CString& qcPath = m_qcFileHistoryArray[qcIndex];

	// Retrieve the folder
	Uint32 length = qcPath.length() - 1;
	while(qcPath[length] != '\\' && qcPath[length] != '/')
		length--;

	// Assign final contents
	CString path(qcPath.c_str(), length);

	// Build command
	CString cmd;
	cmd << "cd \"" << path << "\" & ";

	// Get command args
	const Char *pstrCmdArgs = gConfig.GetOptionValue(COptionsWindow::OW_COMPILER_ARGS);
	if(pstrCmdArgs)
		cmd << "\"" << pstrCompilerPath << "\"" <<" " << pstrCmdArgs << " -w " << " \"" << qcPath << "\"";

	system(cmd.c_str());

	// Restore focus to compiler window
	SetFocus((HWND)this->getHandle());
}

//=============================================
// @brief Copies files to the destination folder
//
//=============================================
void CCompilerWindow::CopyFiles( void )
{
	Int32 qcIndex = m_pChoiceQcPaths->getSelectedIndex();
	if(qcIndex < 0 || qcIndex >= MAX_COMPILER_HISTORY || qcIndex >= m_qcFileHistoryArray.size())
	{
		mxMessageBox(this, "QC index was invalid.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return;
	}

	// Retrieve the QC path
	const CString& qcPath = m_qcFileHistoryArray[qcIndex];
	const byte* pFile = FL_LoadFile(qcPath.c_str(), nullptr);
	if(!pFile)
	{
		mxMessageBox (this, "File not found.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return;
	}

	CString basename;
	Char token[MAX_PARSE_LENGTH];

	CString line;
	const Char* pstr = reinterpret_cast<const Char*>(pFile);
	while(pstr)
	{
		pstr = Common::ReadLine(pstr, line);
		if(line.empty())
			continue;

		const Char* plstr = Common::Parse(line.c_str(), token);
		if(!plstr)
			continue;

		if(!qstrcmp(token, "$modelname"))
		{
			Common::Parse(plstr, token);
			Common::Basename(token, basename);
			break;
		}
	}

	FL_FreeFile(pFile);

	// Check if we got a basename
	if(basename.empty())
	{
		mxMessageBox(this, "$modelname not found in Qc file.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return;
	}

	// Copy the MDL file
	bool copiedMDL = CopyFile(basename.c_str(), "mdl");
	// Copy the VBM file
	bool copiedVBM = CopyFile(basename.c_str(), "vbm");

	// Copy the .mdl and .vbm files
	CString msg;
	if(copiedMDL && copiedVBM)
	{
		Int32 outDirIndex = m_pChoiceCopyPaths->getSelectedIndex();
		if(outDirIndex < 0 || outDirIndex >= MAX_COMPILER_HISTORY || outDirIndex >= m_copyPathHistoryArray.size())
		{
			mxMessageBox(this, "Output directory index was invalid.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
			return;
		}

		msg << basename << ".mdl and " << basename << ".vbm successfully copied to " << m_copyPathHistoryArray[outDirIndex];
	}
	else if(copiedMDL)
	{
		Int32 outDirIndex = m_pChoiceCopyPaths->getSelectedIndex();
		if(outDirIndex < 0 || outDirIndex >= MAX_COMPILER_HISTORY || outDirIndex >= m_copyPathHistoryArray.size())
		{
			mxMessageBox(this, "Output directory index was invalid.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
			return;
		}

		msg << basename << ".mdl successfully copied to " << m_copyPathHistoryArray[outDirIndex];
	}
	else
	{
		msg = "No files were copied.";
	}

	mxMessageBox(this, msg.c_str(), CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_INFORMATION);
}

//=============================================
// @brief Copies specified file to the destination folder
//
//=============================================
bool CCompilerWindow::CopyFile( const Char* pstrBaseName, const Char* pstrExtension )
{
	Int32 qcIndex = m_pChoiceQcPaths->getSelectedIndex();
	if(qcIndex < 0 || qcIndex >= MAX_COMPILER_HISTORY || qcIndex >= m_qcFileHistoryArray.size())
	{
		mxMessageBox(this, "QC index was invalid.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return false;
	}

	Int32 outDirIndex = m_pChoiceCopyPaths->getSelectedIndex();
	if(outDirIndex < 0 || outDirIndex >= MAX_COMPILER_HISTORY || outDirIndex >= m_copyPathHistoryArray.size())
	{
		mxMessageBox(this, "Output directory index was invalid.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return false;
	}

	// Retrieve the QC path
	const CString& qcPath = m_qcFileHistoryArray[qcIndex];

	// Retrieve the folder
	Uint32 length = qcPath.length() - 1;
	while(qcPath[length] != '\\' && qcPath[length] != '/')
		length--;

	// Assign final contents
	CString path(qcPath.c_str(), length);
	path << PATH_SLASH_CHAR << pstrBaseName << "." << pstrExtension;

	Uint32 fileSize = 0;
	const byte* pFile = FL_LoadFile(path.c_str(), &fileSize);
	if(!pFile)
	{
		CString msg;
		msg << "Couldn't open " << path << " for reading.";
		mxMessageBox(this, msg.c_str(), CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
		return false;
	}

	CString outputPath;
	outputPath << m_copyPathHistoryArray[outDirIndex] << PATH_SLASH_CHAR << pstrBaseName << "." << pstrExtension;

	bool result = FL_WriteFile(pFile, fileSize, outputPath.c_str());
	FL_FreeFile(pFile);

	if(!result)
	{
		CString msg;
		msg << "Failed to open " << outputPath << " for writing.";
		mxMessageBox(this, msg.c_str(), CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
	}
	
	return result;	
}

//=============================================
// @brief Handles an event
//
//=============================================
Int32 CCompilerWindow::handleEvent( mxEvent* pEvent )
{
	if (pEvent->event != mxEvent::Action)
		return 0;

	switch (pEvent->action)
	{
		case IDCW_COMPILE_BUTTON:
			{
				CompileQC();
			}
			break;
		case IDCW_EXIT_BUTTON:
			{
				gConfig.SaveOptions();
				this->setVisible(false);
				SaveHistory();
			}
			break;
		case IDCW_FILE_BUTTON:
			{
				CString ssOption0;
				ssOption0 << LAST_QC_PATH_HEADER << "_0";

				const Char *pstrLastQcPath = gConfig.GetOptionValue(ssOption0.c_str());
				const Char *pstrFilePath = mxGetOpenFileName(this, pstrLastQcPath, "*.qc");
				if (pstrFilePath)
				{
					// Get dir path
					CString path;
					Viewer_GetDirectoryPath(pstrFilePath, path);

					Uint32 i;
					for (i = 0; i < m_qcFileHistoryArray.size(); i++)
					{
						if(!qstrcicmp(m_qcFileHistoryArray[i], path))
							break;
					}

					// swap existing recent file
					if (i != m_qcFileHistoryArray.size())
					{
						CString tmp = m_qcFileHistoryArray[0];
						m_qcFileHistoryArray[0] = m_qcFileHistoryArray[i];
						m_qcFileHistoryArray[i] = tmp;
					}
					else
					{
						if(m_qcFileHistoryArray.size() < MAX_COMPILER_HISTORY)
							m_qcFileHistoryArray.resize(m_qcFileHistoryArray.size()+1);

						for (i = m_qcFileHistoryArray.size()-1; i > 0; i--)
							m_qcFileHistoryArray[i] = m_qcFileHistoryArray[i-1];

						m_qcFileHistoryArray[0] = pstrFilePath;
					}
				}

				// Reset these
				InitCopyPathHistory();
				InitQcHistory();
				SaveHistory();

				// Redraw the window
				redraw();
			}
			break;
		case IDCW_COPY_PATH_BUTTON:
			{
				const char *pstrFolder = mxGetSelectFolder(this);
				if (pstrFolder)
				{
					Uint32 i;
					for (i = 0; i < m_copyPathHistoryArray.size(); i++)
					{
						if(!qstrcicmp(m_copyPathHistoryArray[i], pstrFolder))
							break;
					}

					// swap existing recent file
					if (i != m_copyPathHistoryArray.size())
					{
						CString tmp = m_copyPathHistoryArray[0];
						m_copyPathHistoryArray[0] = m_copyPathHistoryArray[i];
						m_copyPathHistoryArray[i] = tmp;
					}
					else
					{
						if(m_copyPathHistoryArray.size() < MAX_COMPILER_HISTORY)
							m_copyPathHistoryArray.resize(m_copyPathHistoryArray.size()+1);

						for(i = m_copyPathHistoryArray.size()-1; i > 0; i--)
							m_copyPathHistoryArray[i] = m_copyPathHistoryArray[i-1];

						m_copyPathHistoryArray[0] = pstrFolder;
					}
				}
				// Re-initialize these
				InitCopyPathHistory();
				SaveHistory();

				// Redraw window
				redraw();
			}
			break;
		case IDCW_COPY_PATH:
			{
				Int32 selectIdx = m_pChoiceCopyPaths->getSelectedIndex();
				m_pChoiceCopyPaths->select(0);

				if(selectIdx < 0 || selectIdx >= MAX_COMPILER_HISTORY || selectIdx >= m_copyPathHistoryArray.size())
				{
					mxMessageBox(this, "Output directory index was invalid.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
					return 0;
				}

				CString tmp = m_copyPathHistoryArray[0];
				m_copyPathHistoryArray[0] = m_copyPathHistoryArray[selectIdx];
				m_copyPathHistoryArray[selectIdx] = tmp;

				SaveHistory();
				InitCopyPathHistory();
			}
			break;
		case IDCW_FILE_PATH:
			{
				Int32 selectIdx = m_pChoiceQcPaths->getSelectedIndex();
				m_pChoiceQcPaths->select(0);

				if(selectIdx < 0 || selectIdx >= MAX_COMPILER_HISTORY || selectIdx >= m_qcFileHistoryArray.size())
				{
					mxMessageBox(this, "Output directory index was invalid.", CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
					return 0;
				}

				CString tmp = m_qcFileHistoryArray[0];
				m_qcFileHistoryArray[0] = m_qcFileHistoryArray[selectIdx];
				m_qcFileHistoryArray[selectIdx] = tmp;

				SaveHistory();
				InitQcHistory();
			}
			break;
		case IDCW_EDIT_BUTTON:
			{
				EditQC();
			}
			break;
		case IDCW_COPY_BUTTON:
			{
				CopyFiles();
			}
			break;
	}

	return 1;
}

//=============================================
// @brief Loads history information
//
//=============================================
void CCompilerWindow::GetHistory( void )
{
	m_pChoiceQcPaths->removeAll();
	for(Uint32 i = 0; i < MAX_COMPILER_HISTORY; i++)
	{
		CString optionName;
		optionName << LAST_QC_PATH_HEADER << "_" << i;

		const Char* pstrValue = gConfig.GetOptionValue(optionName.c_str());
		if(!pstrValue)
			break;

		if(m_qcFileHistoryArray.size() <= i)
			m_qcFileHistoryArray.resize(i+1);

		m_qcFileHistoryArray[i] = pstrValue;
		m_pChoiceQcPaths->add(pstrValue);
	}
	m_pChoiceQcPaths->select(0);

	m_pChoiceCopyPaths->removeAll();
	for(Uint32 i = 0; i < MAX_COMPILER_HISTORY; i++)
	{
		CString optionName;
		optionName << LAST_COPY_PATH_HEADER << "_" << i;
		const Char* pstrValue = gConfig.GetOptionValue(optionName.c_str());
		if(!pstrValue)
			break;

		if(m_copyPathHistoryArray.size() <= i)
			m_copyPathHistoryArray.resize(i+1);

		m_copyPathHistoryArray[i] = pstrValue;
		m_pChoiceCopyPaths->add(pstrValue);
	}
	m_pChoiceCopyPaths->select(0);
}

//=============================================
// @brief Saves history information
//
//=============================================
void CCompilerWindow::SaveHistory( void )
{
	for(Uint32 i = 0; i < m_qcFileHistoryArray.size(); i++)
	{
		CString optionName;
		optionName << LAST_QC_PATH_HEADER << "_" << i;
		gConfig.SetOption(optionName.c_str(), m_qcFileHistoryArray[i].c_str());
	}

	for(Uint32 i = 0; i < m_copyPathHistoryArray.size(); i++)
	{
		CString optionName;
		optionName << LAST_COPY_PATH_HEADER << "_" << i;
		gConfig.SetOption(optionName.c_str(), m_copyPathHistoryArray[i].c_str());
	}
}

//=============================================
// @brief Initializes QC history
//
//=============================================
void CCompilerWindow::InitQcHistory( void )
{
	m_pChoiceQcPaths->removeAll();
	for (Uint32 i = 0; i < m_qcFileHistoryArray.size(); i++)
		m_pChoiceQcPaths->add(m_qcFileHistoryArray[i].c_str());

	m_pChoiceQcPaths->select(0);
}

//=============================================
// @brief Initializes copy path history
//
//=============================================
void CCompilerWindow::InitCopyPathHistory( void )
{
	m_pChoiceCopyPaths->removeAll();
	for (Uint32 i = 0; i < m_copyPathHistoryArray.size(); i++)
		m_pChoiceCopyPaths->add(m_copyPathHistoryArray[i].c_str());

	m_pChoiceCopyPaths->select(0);
}

//=============================================
// @brief Creates an instance of this class
//
// @return Created instance
//=============================================
CCompilerWindow* CCompilerWindow::CreateInstance( void )
{
	if(!g_pInstance)
		g_pInstance = new CCompilerWindow();

	return g_pInstance;
}

//=============================================
// @brief Returns the current instance of this class
//
// @return Current instance, or nullptr
//=============================================
CCompilerWindow* CCompilerWindow::GetInstance( void )
{
	return g_pInstance;
}

//=============================================
// @brief Deletes the current instance of this class
//
//=============================================
void CCompilerWindow::DeleteInstance( void )
{
	if(!g_pInstance)
		return;

	g_pInstance = nullptr;
}