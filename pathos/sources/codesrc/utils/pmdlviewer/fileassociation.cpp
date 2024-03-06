//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           FileAssociation.cpp
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

#include <mx/mx.h>

#include "fileassociation.h"
#include "config.h"
#include "mdlviewer.h"

// Window title
const Char CFileAssociation::WINDOW_TITLE[] = "File Associations";
// Compiler window x origin
const Uint32 CFileAssociation::WINDOW_X_ORIGIN = 100;
// Compiler window y origin
const Uint32 CFileAssociation::WINDOW_Y_ORIGIN = 100;
// Compiler window width
const Uint32 CFileAssociation::WINDOW_WIDTH = 400;
// Compiler window height
const Uint32 CFileAssociation::WINDOW_HEIGHT = 210;
// Setting prefix
const Char CFileAssociation::SETTING_PREFIX[] = "ASSOC_";

// Current instance of this class
CFileAssociation* CFileAssociation::g_pInstance = nullptr;

//=============================================
// @brief Constructor
//
//=============================================
CFileAssociation::CFileAssociation( void ): 
	mxWindow(nullptr, WINDOW_X_ORIGIN, WINDOW_Y_ORIGIN, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, mxWindow::Dialog),
	m_pChoiceExtension(nullptr),
	m_pLineEditProgram(nullptr),
	m_pButtonChooseProgram(nullptr)
{
	m_pChoiceExtension = new mxChoice (this, 5, 5, 220, 22, IDC_EXTENSION);

	new mxGroupBox (this, 5, 30, 380, 115, "Assocations");

	m_pRadioButtonAction[ACTION_BTN_PROGRAM] = new mxRadioButton(this, 10, 50, 120, 22, "program", IDC_ACTION1, true);
	m_pRadioButtonAction[ACTION_BTN_ASSOCIATED_PROGRAM] = new mxRadioButton(this, 10, 72, 120, 22, "associated program", IDC_ACTION2);
	m_pRadioButtonAction[ACTION_BTN_DEFAULT] = new mxRadioButton(this, 10, 94, 120, 22, "PMV default", IDC_ACTION3);
	m_pRadioButtonAction[ACTION_BTN_NONE] = new mxRadioButton(this, 10, 116, 120, 22, "none", IDC_ACTION4);

	m_pLineEditProgram = new mxLineEdit (this, 130, 50, 220, 22, "", IDC_PROGRAM);
	m_pLineEditProgram->setEnabled (false);
	m_pButtonChooseProgram = new mxButton(this, 352, 50, 22, 22, ">>", IDC_CHOOSEPROGRAM);
	m_pButtonChooseProgram->setEnabled(false);

	m_pRadioButtonAction[ACTION_BTN_PROGRAM]->setChecked(false);
	m_pRadioButtonAction[ACTION_BTN_ASSOCIATED_PROGRAM]->setChecked(true);

	new mxButton(this, 110, 155, 75, 22, "Ok", IDC_OK);
	new mxButton(this, 215, 155, 75, 22, "Cancel", IDC_CANCEL);

	InitAssociations();
}

//=============================================
// @brief Destructor
//
//=============================================
CFileAssociation::~CFileAssociation( void )
{
	// mx deletes these windows before we 
	// get a chance to do it via DeleteInstance()
	g_pInstance = nullptr;
}

//=============================================
// @brief Handles an mv event
//
//=============================================
Int32 CFileAssociation::handleEvent( mxEvent* pEvent )
{
	if (pEvent->event != mxEvent::Action)
		return 0;

	switch (pEvent->action)
	{
	case IDC_EXTENSION:
		{
			Int32 index = m_pChoiceExtension->getSelectedIndex();
			if (index >= 0)
				SetAssociation(index);
		}
		break;
	case IDC_ACTION1:
	case IDC_ACTION2:
	case IDC_ACTION3:
	case IDC_ACTION4:
		{
			if(m_pRadioButtonAction[ACTION_BTN_PROGRAM]->isChecked())
			{
				m_pLineEditProgram->setEnabled(true);
				m_pButtonChooseProgram->setEnabled(true);
			}
			else
			{
				m_pLineEditProgram->setEnabled(false);
				m_pButtonChooseProgram->setEnabled(false);
			}

			Int32 index = m_pChoiceExtension->getSelectedIndex();
			if (index >= 0 && index < m_associationsArray.size())
				m_associationsArray[index].association = pEvent->action - IDC_ACTION1;
		
		}
		break;
	case IDC_PROGRAM:
		{
			Int32 index = m_pChoiceExtension->getSelectedIndex();
			if (index >= 0 && index < m_associationsArray.size())
				m_associationsArray[index].program = m_pLineEditProgram->getLabel();
		}
		break;
	case IDC_CHOOSEPROGRAM:
		{
			const Char *pstrPath = mxGetOpenFileName(this, nullptr, "*.exe");
			if (pstrPath)
			{
				m_pLineEditProgram->setLabel(pstrPath);

				Int32 index = m_pChoiceExtension->getSelectedIndex();
				if (index >= 0 && index < m_associationsArray.size())
					m_associationsArray[index].program = m_pLineEditProgram->getLabel();
			}
		}
		break;
	case IDC_OK:
		{
			SaveAssociations();
			setVisible(false);
		}
		break;
	case IDC_CANCEL:
		{
			setVisible(false);
		}
		break;
	}

	return 1;
}

//=============================================
// @brief Initializes associations
//
//=============================================
void CFileAssociation::InitAssociations( void )
{
	m_pChoiceExtension->removeAll();

	Uint32 assocIndex = 0;
	while(true)
	{
		CString optionName;
		optionName << SETTING_PREFIX << assocIndex;
		assocIndex++;

		CString value = gConfig.GetOptionValue(optionName.c_str());
		if(value.empty())
			break;

		// Extract the extension name
		Int32 lastSemicolonOffset = 0;
		Int32 semicolonOffset = value.find(0, ";");
		if(semicolonOffset == -1)
		{
			CString error;
			error << "Invalid option string: " << value;

			mxMessageBox(nullptr, error.c_str(), CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
			continue;
		}

		CString formatExtension(value.c_str(), semicolonOffset);
		lastSemicolonOffset = semicolonOffset;

		// Extract the program path
		semicolonOffset = value.find(lastSemicolonOffset + 1, ";");
		if(semicolonOffset == -1)
		{
			CString error;
			error << "Invalid option string: " << value;

			mxMessageBox(nullptr, error.c_str(), CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
			continue;
		}

		const Char* pstrBegin = value.c_str() + lastSemicolonOffset + 1;
		CString programPath(pstrBegin, semicolonOffset - lastSemicolonOffset - 1);
		lastSemicolonOffset = semicolonOffset;

		// Extract the last token
		CString association;
		pstrBegin = value.c_str() + lastSemicolonOffset + 1;
		Common::Parse(pstrBegin, association);

		if(association.empty())
		{
			CString error;
			error << "Invalid option string: " << value;

			mxMessageBox(nullptr, error.c_str(), CMDLViewer::VIEWER_APP_TITLE, MX_MB_OK | MX_MB_ERROR);
			continue;
		}

		association_t newAssociation;
		newAssociation.association = SDL_atoi(association.c_str());
		newAssociation.extension = formatExtension;
		newAssociation.program = programPath;

		m_associationsArray.push_back(newAssociation);
		m_pChoiceExtension->add(formatExtension.c_str());
	}

	if(!m_associationsArray.empty())
		SetAssociation(0);
}

//=============================================
// @brief Sets an association
//
//=============================================
void CFileAssociation::SetAssociation( Int32 index )
{
	if(m_associationsArray.empty())
		return;

	if(index >= m_associationsArray.size())
		return;

	association_t& assoc = m_associationsArray[index];

	m_pChoiceExtension->select(index);
	m_pLineEditProgram->setLabel(assoc.program.c_str());

	for (Uint32 i = 0; i < NB_ACTION_BTNS; i++)
		m_pRadioButtonAction[i]->setChecked((i == assoc.association) ? true : false);

	if(assoc.association == 0)
	{
		m_pLineEditProgram->setEnabled(true);
		m_pButtonChooseProgram->setEnabled(true);
	}
	else
	{
		m_pLineEditProgram->setEnabled(false);
		m_pButtonChooseProgram->setEnabled(false);
	}

	if(!qstrcicmp(assoc.extension, "mdl") 
		|| !qstrcicmp(assoc.extension, "tga") 
		|| !qstrcicmp(assoc.extension, "wav"))
		m_pRadioButtonAction[ACTION_BTN_DEFAULT]->setEnabled(true);
	else
		m_pRadioButtonAction[ACTION_BTN_DEFAULT]->setEnabled(false);
}

//=============================================
// @brief Saves associations to config file
//
//=============================================
void CFileAssociation::SaveAssociations( void )
{
	if(m_associationsArray.empty())
		return;

	for(Uint32 i = 0; i < m_associationsArray.size(); i++)
	{
		association_t& assoc = m_associationsArray[i];

		CString saveName;
		saveName << SETTING_PREFIX << i;

		CString value;
		value << assoc.extension << ";" << assoc.program << ";" << assoc.association;
		gConfig.SetOption(saveName.c_str(), value.c_str());
	}
}

//=============================================
// @brief Returns the mode for a file extension
//
//=============================================
Int32 CFileAssociation::GetMode( const Char* pstrExtension )
{
	for(Uint32 i = 0; i < m_associationsArray.size(); i++)
	{
		association_t& assoc = m_associationsArray[i];
		if(!qstrcicmp(assoc.extension, pstrExtension))
			return assoc.association;
	}

	return ACTION_BTN_NULL;
}

//=============================================
// @brief Returns the program to use for an extension
//
//=============================================
const Char *CFileAssociation::GetProgram( const Char *pstrExtension )
{
	for(Uint32 i = 0; i < m_associationsArray.size(); i++)
	{
		association_t& assoc = m_associationsArray[i];
		if(!qstrcicmp(assoc.extension, pstrExtension))
			return assoc.program.c_str();
	}

	return nullptr;
}

//=============================================
// @brief Creates an instance of this class
//
// @return Created instance
//=============================================
CFileAssociation* CFileAssociation::CreateInstance( void )
{
	if(!g_pInstance)
		g_pInstance = new CFileAssociation();

	return g_pInstance;
}

//=============================================
// @brief Returns the current instance of this class
//
// @return Current instance, or nullptr
//=============================================
CFileAssociation* CFileAssociation::GetInstance( void )
{
	return g_pInstance;
}

//=============================================
// @brief Deletes the current instance of this class
//
//=============================================
void CFileAssociation::DeleteInstance( void )
{
	if(!g_pInstance)
		return;

	// TODO
	//delete g_pInstance;
	g_pInstance = nullptr;
}
