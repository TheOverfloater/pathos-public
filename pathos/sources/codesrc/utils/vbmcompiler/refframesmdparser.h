/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef REFFRAMESMDPARSER_H
#define REFFRAMESMDPARSER_H

#include "smdparser.h"

/*
=======================
CReferenceFrameSMDParser

=======================
*/
class CReferenceFrameSMDParser : public CSMDParser
{
public:
	CReferenceFrameSMDParser( CStudioModelCompiler& compiler, vbm::ref_frameinfo_t& referenceFrameInfo );
	~CReferenceFrameSMDParser( void );

public:
	// Processes a file
	virtual bool ProcessFile( const Char* pstrFilename ) override;

private:
	// Reference to the structure that holds this information
	vbm::ref_frameinfo_t& m_referenceFrameInfo;
};
#endif // REFFRAMESMDPARSER_H