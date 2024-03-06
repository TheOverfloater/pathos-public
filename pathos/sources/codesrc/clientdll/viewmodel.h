/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef VIEWMODEL_H
#define VIEWMODEL_H

#include "cl_entity.h"

struct cache_model_t;
struct mstudioseqdesc_t;
class CCVar;

/*
====================
CViewModel

====================
*/
class CViewModel
{
public:
	// View model default FOV value
	static const Uint32 VIEWMODEL_DEFAULT_FOV_VALUE;

public:
	CViewModel( void );
	~CViewModel( void );

public:
	// Initializes the class
	bool Init ( void );
	// Initializes for game
	bool InitGame ( void );
	// Clears game info
	void ClearGame( void );

	// Performs think functions
	void Think ( void );
	// Draws the view model
	bool Draw( void );

	// Returns a pointer to the view model entity
	cl_entity_t *GetViewModel ( void );
	// Sets the sequence to play
	void SetSequence( Int32 sequence, Int64 body, Int32 skin, bool blendsequence = true );
	// Reads viewmodel emssages
	void ProcessMessage( const byte* pdata, Uint32 msgsize );

public:
	// Tells if offsets can be applied
	bool CanApplyOffsets( void ) const { return m_applyOffsets; }

private:
	// Retreives the model to use
	cache_model_t *GetModel ( char *szname );
	// Retreives the sequence information
	const mstudioseqdesc_t* GetSequenceInfo ( Int32 sequence );

private:
	// View model render toggle cvar
	CCVar*					m_pCvarDrawViewModel;

	// View model entity itself
	cl_entity_t				m_viewModelEntity;

	// Pointer to view model
	const cache_model_t*	m_pModel;
	// Ideal viewmodel to use
	CString					m_idealModel;

	// Ideal sequence to use
	Int32					m_iIdealSequence;
	// TRUE if we should blend sequences
	bool					m_bBlendSequence;
	// TRUE if sequence needs to be updated
	bool					m_bUpdateSequence;

	// Don't blend if vmodel got changed
	bool					m_bForceDontBlend;
	// Don't apply offsets(in case of scope)
	bool					m_applyOffsets;


private:
	// Default FOV cvar
	CCVar					*m_pCvarReferenceFOV;
	// Viewmodel FOV cvar
	CCVar					*m_pCvarViewModelFOV;
};
extern CViewModel gViewModel;
#endif
