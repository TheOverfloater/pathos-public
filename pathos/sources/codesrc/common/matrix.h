/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MATRIX_H
#define MATRIX_H

struct matrix_t
{
	matrix_t():
		next(nullptr)
	{
		for(Uint32 i = 0; i < 16; i++)
			matrix[i] = 0;
	}

	Float matrix[16];
	matrix_t *next;
};

/*
====================
CMatrix

====================
*/
class CMatrix
{
public:
	CMatrix ( void );
	explicit CMatrix ( const Float *pMatrix );
	virtual ~CMatrix( void );
	CMatrix( const CMatrix& src );

public:
	const Float *GetMatrix( void );
	const Float *GetInverse( void );
	const Float *Transpose( void );
	void SetMatrix( const Float *pMatrix );

	void LoadIdentity( void );
	void PushMatrix( void );
	void PopMatrix( void );

	void MultMatrix ( const Float *pMatrix );
	void Translate ( Float x, Float y, Float z );
	void Rotate ( Float angle, Float x, Float y, Float z );
	void Scale ( Float x, Float y, Float z );

	void SetFrustum( Float l, Float r, Float b, Float t, Float n, Float f );
	void Ortho(Float l, Float r, Float b, Float t, Float n, Float f);
	void LookAt( Float eyex, Float eyey, Float eyez, Float centerx, Float centery, Float centerz, Float upx, Float upy, Float upz );

public:
	inline void operator=( const CMatrix& src );

private:
	// working matrix
	Float	m_workingMatrix[16];

	// output matrix
	Float	m_outMatrix[16];

	// multiply matrix
	Float	m_multMatrix[16];

	// rotation matrix
	Float	m_rotationMatrix[16];

	// inverse transpose matrix
	Float	m_matrixTranspose[16];

private:
	// pointer to current
	Float *m_pCurrentMatrix;

	// matrix stack
	matrix_t *m_pMatrixHeader;

};
#endif