/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ANIMATIONSMDPARSER_H
#define ANIMATIONSMDPARSER_H

#include "smdparser.h"

/*
=======================
CAnimationSMDParser

=======================
*/
class CAnimationSMDParser : public CSMDParser
{
public:
	CAnimationSMDParser( CStudioModelCompiler& compiler, smdl::animation_t* panimation, Float zrotation );
	~CAnimationSMDParser( void );

public:
	// Processes a file
	virtual bool ProcessFile( const Char* pstrFilename ) override;

private:
	// Parses animation from the SMD file loaded
	bool ParseAnimation( void );

private:
	// Z rotation value
	Float m_zRotation;
	// Pointer to submodel to load for
	smdl::animation_t* m_pAnimation;
};
#endif // ANIMATIONSMDPARSER_H