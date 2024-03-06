/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef VIEW_H
#define VIEW_H

struct ref_params_t;
struct cache_model_t;

class CCVar;

extern Vector g_viewOrigin;

/*
====================
CNormalSmoothing

====================
*/
class CDefaultView
{
public:
	// Flashlight fade speed
	static const Float FLASHLIGHT_FADE_SPEED;
	// Flashlight sprite file
	static const Char FLASHLIGHT_SPRITE_FILE[];

	// Values view leaning calculations
	static const Float VIEWMODEL_LAG_MULT;
	static const Float VIEWMODEL_LAG_SPEED;

	// Values for view bob calculations
	static const Float BOB_CYCLE_MIN;
	static const Float BOB_CYCLE_MAX;
	static const Float BOB;
	static const Float BOB_UP;

	// Values for stair step smoothing
	static const Float V_SM_REF_VEL;
	static const Float V_SM_BLEND_VEL;

public:
	CDefaultView();
	~CDefaultView();

public:
	// Initializes the class
	bool Init( void );

	// Initializes for game
	bool InitGame( void );
	// Clears game data
	void ClearGame( void );

	// Calculates view position
	void CalcRefDef( ref_params_t& params );

	// Returns the lean offset
	Vector GetLeanOffset( void ) const;

	// Resets view roll
	void ResetViewRoll( void );
	// Resets view idling
	void ResetViewIdle( void );

	// Calculates the view model angle
	void CalcViewModelAngle( cl_entity_t* pviewmodel, const ref_params_t& params );

	// Sets the FOV value
	void SetFOV( Float newFOV );
	// Sets the FOV override value
	void SetFOVOverride( Float overrideFOV );
	// Sets the fov zoom
	void SetFOVZoom( Float desiredFOV, Float blenddelta );

	// Blends field of view values
	void CalculateFOV( ref_params_t& params );
	// Returns the current FOV value
	Float GetFOV( void ) const;

	// Sets up all player flashlight types
	void SetupFlashlights( const ref_params_t& params );

	// Sets the view entity
	void SetViewEntity( entindex_t viewentity );
	// Returns the view entity
	entindex_t GetViewEntity( void ) const;

	// Updates autoaim
	void SetAutoAim( Float autoAimX, Float autoAimY );

private:
	// Calculates the view bob
	void CalcBob( cl_entity_t* pplayer, ref_params_t& params );
	// Calculates view roll
	static Float CalcRoll( const Vector& angles, const Vector& velocity, Float rollangle, Float rollspeed );
	// Calculates interpolated rolling
	static Float CalcSmoothRolling( Float cur_roll, Float target_roll, Float speed, Double frametime );
	// Calculates view rolling
	void CalcViewRoll( cl_entity_t* pplayer, ref_params_t& params );
	// Calculates leaning
	void CalcLeaning( cl_entity_t* pplayer, cl_entity_t *pviewmodel, ref_params_t& params );

	// Estimates footstep time
	static Float EstimateStepTime( cl_entity_t* pplayer, ref_params_t& params );
	// Smooths out up-down steps
	void SmoothSteps( cl_entity_t* pplayer, cl_entity_t* pviewmodel, ref_params_t& params );

	// Adds viewmodel bob
	void AddViewModelBob( cl_entity_t* pviewmodel, ref_params_t& params );
	// Calculates view model lag
	void CalcViewModelLag( const ref_params_t& params, Vector& origin, const Vector& angles, const Vector& orig_angles );

	// Adds idle breathing movement
	void AddIdle( cl_entity_t* pplayer, ref_params_t& params );
	// Calculates swim floating
	void CalcSwimFloat( cl_entity_t* pplayer, ref_params_t& params );

	// Sets up player flashlights
	void SetupFlashlightForType( const ref_params_t& params, Float* pstrengths, Int32 effectbit );

private:
	// Bob time
	Double m_bobTime;
	// Last bob time
	Double m_lastBobTime;
	// Vertical bob amount
	Float m_verticalBob;
	// Lateral bob amount
	Float m_lateralBob;

	// Last facing of view model
	Vector m_viewModelLastFacing;
	// Last viewmodel lag time
	Double m_flLastViewModelLagTime;

	// Time we started breathing
	Double m_breathingTime;

	// Current view roll
	Float m_currentViewRoll;

	// Last lean time
	Double m_leanTime;
	// Previous buttons when calculating leaning
	Int32 m_prevLeanButtons;
	// Previous player flags
	Int64 m_prevPlayerFlags;
	// Leaning state
	bool m_leaningState;

	// Lean angles
	Vector m_prevLeanAngles;
	Vector m_idealLeanAngles;
	Vector m_curLeanAngles;

	// Lean offset
	Vector m_prevLeanOffset;
	Vector m_idealLeanOffset;
	Vector m_curLeanOffset;

	// Last time we calculated step smoothing
	Double m_lastStepSmoothTime;
	// Last Z value used for step smoothing
	Float m_prevStepSmoothZ;
	// Stair-step smoothing speed
	Float m_stepSmoothSpeed;
	// Last origin at step smoothing
	Vector m_lastStepSmoothOrigin;
	// Last view offset used in step smoothing
	Vector m_lastStepSmoothViewOffset;

	// Last swim float time
	Double m_lastSwimFloatTime;

private:
	// Current field of view value
	Float m_fovValue;
	// FOV override value
	Float m_fovOverrideValue;

	// FOV previous value
	Float m_prevFOVValue;
	// FOV desired value
	Float m_desiredFOVValue;

	// Blend duration for FOV
	Float m_fovBlendDelta;
	// FOV blend time
	Double m_fovBlendTime;

private:
	// Ideal autoaim vector
	Vector m_idealAutoAimVector;
	// Previous autoaim vector
	Vector m_currentAutoAimVector;

private:
	// Tactical flashlight strengths
	Float m_tacticalLightStrengths[MAX_PLAYERS];
	// Shoulder flashlight strengths
	Float m_shoulderLightStrengths[MAX_PLAYERS];
	// Local player's dimlight strength
	Float m_localPlayerDimLightStrength;

	// Flashlight sprite model
	const cache_model_t* m_pFlashlightSprite;

private:
	// View entity
	entindex_t m_viewEntity;

private:
	// View roll angle cvar
	CCVar* m_pCvarRollAngle;
	// View roll speed cvar
	CCVar* m_pCvarRollSpeed;
	// Default FOV cvar ptr
	CCVar* m_pCvarReferenceFOV;
	// View bob cvar
	CCVar* m_pCvarViewBob;
};
extern CDefaultView gDefaultView;

extern cl_entity_t* V_GetViewModel( void );
extern void V_CalcRefDef( ref_params_t& params );
extern void V_GetViewInfo( Vector& vOrigin, Vector& vAngles );
#endif //VIEW_H