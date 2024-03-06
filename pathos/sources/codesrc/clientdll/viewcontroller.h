/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef VIEWCONTROLLER_H
#define VIEWCONTROLLER_H

struct ref_params_t;

#include "vcontroller_shared.h"
#include "cl_entity.h"

/*
====================
CViewController

====================
*/
class CViewController
{
public:
	CViewController( void );
	~CViewController( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes for game
	bool InitGame( void );
	// Clears game states
	void ClearGame( void );

	// Calculates view
	void CalcRefDef( ref_params_t& params );
	// Tells if view controller is active
	bool IsActive( void ) const;

	// Draws view controller objects
	bool Draw( void );

public:
	// Processes a view controller message
	void ProcessMessage( const byte* pdata, Uint32 msgsize );
	// Gets the view origin and angles
	void GetViewInfo( Vector& origin, Vector& angles );

private:
	// Calculates view for camera model
	void CalcView_CameraModel( ref_params_t& params );

	// Gets duration of a sequence
	Float GetSequenceTime( const cache_model_t *pmodel, Int32 sequence );
	// Estimates current frame
	Float EstimateFrame( cl_entity_t *pentity, Int32 sequence, Float flTime ) const;
	// Looks up a sequence by name
	static Int32 LookupSequence( const cache_model_t *pmodel, const Char *szname );

	// Gets the entity angles and origin from attachments
	static void GetViewInfoFromEntity( cl_entity_t* pentity, Vector& origin, Vector& angles );

private:
	// engine entity we're attached to
	cl_entity_t *m_pControllerEntity; 
	// for clientside animations
	cl_entity_t m_clientEntity; 
	// view entity
	cl_entity_t m_viewEntity;

	// View hands model
	const cache_model_t *m_pViewHands;

private:
	viewcontroller_modes_t			m_mode;
	viewcontroller_blend_modes_t	m_blendMode;

	bool							m_blend;
	Double							m_blendTime;
	Double							m_blendDelta;
	
	Float							m_lerpDelay;

	Vector							m_blendOrigin;
	Vector							m_blendAngles;

private:
	bool							m_cameraReset;
	bool							m_alwaysDraw;
};
extern CViewController gViewController;
#endif