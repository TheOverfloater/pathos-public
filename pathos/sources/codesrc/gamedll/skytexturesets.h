/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SKYTEXTURESETS_H
#define SKYTEXTURESETS_H

//=============================================
//
//=============================================
class CSkyTextureSets
{
public:
	// Max skybox sets
	static const Uint32 MAX_SKYBOX_TEXTURE_SETS;

public:
	CSkyTextureSets( void );
	virtual ~CSkyTextureSets( void );

public:
	void Reset( void );
	Int32 RegisterSkyTextureSet( const Char* pstrSkyTextureName );
	void RegisterSets( edict_t* pPlayer );

private:
	// Array of sky texture names
	CArray<CString> m_skyTextureNamesArray;
};
extern CSkyTextureSets gSkyTextureSets;
#endif //SKYTEXTURESETS_H